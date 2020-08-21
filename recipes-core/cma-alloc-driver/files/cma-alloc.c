/*
 * kernel module helper for testing CMA
 *
 * Copyright (C) 2016 FNC
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/dma-contiguous.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/sizes.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/delay.h>

#include <asm/compat.h>         // for compat data types for 32/64bit support
#include <asm/uaccess.h>
#include "cma-alloc.h"

struct cma_allocation {
    struct list_head list;
    unsigned long size;
    dma_addr_t dma;
    void *virt;
};

static struct device *cma_dev;
static LIST_HEAD(cma_allocations);
static DEFINE_SPINLOCK(cma_lock);

/**
 *  dma_alloc_coherent_mask:
 *  @dev: pointer to struct device
 *  @gfp: gfp flags
 *  returns dma mask as unsigned long
 */
unsigned long dma_alloc_coherent_mask(struct device *dev, gfp_t gfp)
{
    unsigned long dma_mask = 0;
    
    dma_mask = dev->coherent_dma_mask;
    if (!dma_mask)
            dma_mask = (gfp & GFP_DMA) ? DMA_BIT_MASK(24) : DMA_BIT_MASK(32);
    
    return dma_mask;
}

/**
 *  dma_alloc_coherent_mask:
 *  @dev: pointer to struct device
 *  @size: size of requested dma memory to be allocated
 *  @dma_addr: pointer to dma phy address
 *  @flag: gfp flags
 *  returns pointer to dma virtual address
 */
void *dma_generic_alloc_coherent(struct device *dev, size_t size,
                                 dma_addr_t *dma_addr, gfp_t flag)
{
	unsigned long dma_mask;
	struct page *page;
	unsigned int count = PAGE_ALIGN(size) >> PAGE_SHIFT;
	dma_addr_t addr;

	dma_mask = dma_alloc_coherent_mask(dev, flag);

	flag |= __GFP_ZERO;
again:
	page = NULL;
	if (!(flag & GFP_ATOMIC))
		page = dma_alloc_from_contiguous(dev, count, get_order(size));
	if (!page)
		return NULL;

	addr = page_to_phys(page);
	if (addr + size > dma_mask) {
		__free_pages(page, get_order(size));

		if (dma_mask < DMA_BIT_MASK(32) && !(flag & GFP_DMA)) {
			flag = (flag & ~GFP_DMA32) | GFP_DMA;
			goto again;
		}

		return NULL;
	}

	*dma_addr = addr;
	return (page);
}

/**
 *  dma_generic_free_coherent:
 *  @dev: pointer to struct device
 *  @size: size of requested dma memory to be freed
 *  @vaddr: pointer to dma virt to be freed
 *  @dma_addr: gfp flags
 */
void dma_generic_free_coherent(struct device *dev, size_t size,
                               void *vaddr, dma_addr_t dma_addr)
{
	unsigned int count = PAGE_ALIGN(size) >> PAGE_SHIFT;
	struct page *page = vaddr;

	if (!dma_release_from_contiguous(dev, page, count))
		free_pages((unsigned long)vaddr, get_order(size));
}

/**
 *  allocate_dma_mem:
 *  @size: size of requested dma memory to be freed
 *  returns size of allocated memory or error value
 */
size_t allocate_dma_mem(size_t size)
{
    int ret = 0;
    struct cma_allocation *alloc;

    alloc = kmalloc(sizeof *alloc, GFP_KERNEL);
    if (!alloc)
        return -ENOMEM;

    size *= SZ_1K;
    alloc->virt = dma_generic_alloc_coherent(cma_dev, size,
                                             &alloc->dma, GFP_KERNEL);

    if (alloc->virt) {
        alloc->size = size;
        dev_info(cma_dev, "allocated CM at virtual address: 0x%p "
                          "dma address: 0x%llu size:%luKiB\n",
                 alloc->virt, alloc->dma, alloc->size / SZ_1K);

        spin_lock(&cma_lock);
        list_add_tail(&alloc->list, &cma_allocations);
        spin_unlock(&cma_lock);

        ret = alloc->size;
    } else {
        dev_err(cma_dev, "no mem in CMA area\n");
        kfree(alloc);
        ret = -ENOSPC;
    }

    return ret;
}

/**
 *  free_dma_mem:
 *  @cma: pointer to struct cma_user
 *  returns size of memory freed or error
 */
static int free_dma_mem(struct cma_user *cma)
{
    int ret = 0;
    struct cma_allocation *alloc = NULL;
    struct cma_allocation *tmp_alloc = NULL;

    if (!cma)
        return -EFAULT;

    spin_lock(&cma_lock);
    if (!list_empty(&cma_allocations)) {
        list_for_each_entry_safe(alloc, tmp_alloc, &cma_allocations, list) {
            if (alloc->dma == cma->dma)
                break;
        }
        list_del(&alloc->list);
    }
    spin_unlock(&cma_lock);

    if (alloc) {
        dev_info(cma_dev, "free CM at virtual address: 0x%p "
                         "dma address: 0x%llu size:%luKiB\n",
                  alloc->virt, alloc->dma, alloc->size / SZ_1K);

        dma_generic_free_coherent(cma_dev, alloc->size,
                                  alloc->virt, alloc->dma);
        kfree(alloc);
    } else {
        dev_err(cma_dev, "invalid alloc: %p\n", alloc);

        ret = -EFAULT;
    }

    return ret;
}

/*
 * This is just a placeholder for the call below incase we need
 * it.
 * cat /dev/cma_alloc
 */
static ssize_t
cma_alloc_read(struct file *file, char __user *buf,
               size_t count, loff_t *ppos)
{
    return 0;
}

/*
 * any write request will alloc a new coherent memory, eg.
 * echo 1024 > /dev/cma_alloc
 * will request 1024KiB by CMA
 */
static ssize_t
cma_alloc_write(struct file *file, const char __user *buf,
                size_t count, loff_t *ppos)
{
    unsigned long size;
    int ret;

    ret = kstrtoul_from_user(buf, count, 0, &size);
    if (ret)
        return ret;

    dev_info(cma_dev, "buffer to be allocated %lu, ret = %d\n", size, ret);

    if (!size)
        return -EINVAL;
    if (size > (ULONG_MAX << PAGE_SHIFT))
        return -EOVERFLOW;

    return allocate_dma_mem(size);
}

/*
 * The ioctl() implementation
 */
static long
cma_alloc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int err = 0, ret = 0;
    struct cma_allocation *alloc = NULL;
    struct cma_allocation *tmp_alloc = NULL;
    struct cma_user *cma = NULL;
    size_t size = 0;
    struct page *page = NULL;
    /* don't even decode wrong cmds: better returning  ENOTTY than EFAULT */
    if (_IOC_TYPE(cmd) != CMA_ALLOC_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > CMA_ALLOC_IOC_MAXNR) return -ENOTTY;
    /*
     * the type is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. Note that the type is user-oriented, while
     * verify_area is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
      err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
      err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    if (err)
        return -EFAULT;

    cma = (struct cma_user*)kmalloc(sizeof *cma, GFP_KERNEL);
    if (!cma)
        return -ENOMEM;

    memset(cma, 0, sizeof *cma);
    ret = copy_from_user(cma, (char*)arg, sizeof *cma);
    if (ret) {
        dev_err(cma_dev, "copy from user failed. ret= %d, arg= 0x%lx, cma= 0x%lx\n",
                ret, arg, cma);
    } else if (cma->size <= 0) {
        dev_err(cma_dev, "invalid size %lu\n", cma->size);
        ret = -EINVAL;
    } else if (size > (ULONG_MAX << PAGE_SHIFT)) {
        ret = -EOVERFLOW;
    }

    if(ret)
        goto error;

    switch(cmd) {
    case CMA_ALLOC_MEM:
        size = allocate_dma_mem(cma->size);
        if (size != (cma->size * SZ_1K)){
            ret = -EFAULT;
        } else {
            spin_lock(&cma_lock);
            if (!list_empty(&cma_allocations))
                alloc = list_last_entry(&cma_allocations,
                                         struct cma_allocation, list);
            spin_unlock(&cma_lock);
            if (alloc) {
                filp->private_data = (void *)alloc;
                cma->virt   = alloc->virt;
                cma->dma    = alloc->dma;
                cma->size   = alloc->size;
                dev_info(cma_dev, "allocated CM at virtual address: 0x%p "
                                  "dma address: 0x%llu size:%luKiB\n",
                         alloc->virt, alloc->dma, alloc->size / SZ_1K);
                ret = copy_to_user((void __user *)arg, cma, sizeof *cma);
                if (ret) {
                    dev_err(cma_dev, "copy to user failed %d\n", ret);
                    ret = -EFAULT;
                }
            } else 
                ret = -EFAULT;
        }
        break;
    case CMA_FREE_MEM:
        if (!cma->dma || !cma->size || !cma->virt)
            ret = -EFAULT;
        else
            ret = free_dma_mem(cma);
        break;
    case CMA_ALLOC_MEM_V2:
        alloc = kmalloc(sizeof *alloc, GFP_KERNEL);
        if (!alloc)
            return -ENOMEM;

        page = dma_alloc_from_contiguous(cma_dev, PAGE_ALIGN(cma->size) >> PAGE_SHIFT, 0);
        if (page) {
            alloc->dma = page_to_phys(page);
            alloc->virt = page;
            alloc->size = PAGE_ALIGN(cma->size);

            spin_lock(&cma_lock);
            list_add_tail(&alloc->list, &cma_allocations);
            spin_unlock(&cma_lock);

            filp->private_data = (void *)alloc;
            cma->virt   = alloc->virt;
            cma->dma    = alloc->dma;
            cma->size   = alloc->size;
            dev_info(cma_dev, "allocated CM at virtual address: 0x%p "
                              "dma address: 0x%llx size:%luB\n",
                     alloc->virt, alloc->dma, alloc->size);
            ret = copy_to_user((void __user *)arg, cma, sizeof *cma);
            if (ret) {
                dev_err(cma_dev, "copy to user failed %d\n", ret);
                ret = -EFAULT;
            }
        } else {
            dev_err(cma_dev, "no mem in CMA area\n");
            kfree(alloc);
            ret = -ENOSPC;
        }
        break;
    case CMA_FREE_MEM_V2:
        if (!cma->dma || !cma->size || !cma->virt)
            ret = -EFAULT;
        else {
            spin_lock(&cma_lock);
            if (!list_empty(&cma_allocations)) {
                list_for_each_entry_safe(alloc, tmp_alloc, &cma_allocations, list) {
                    if (alloc->dma == cma->dma)
                        break;
                }
                list_del(&alloc->list);
            }
            spin_unlock(&cma_lock);

            if (alloc) {
                dma_release_from_contiguous(cma_dev, alloc->virt, PAGE_ALIGN(alloc->size) >> PAGE_SHIFT);
                kfree(alloc);
            }
        }
        break;
    default:
        ret = -ENOTTY;
    }

error:
    kfree(cma);
    return ret;
}

/*
 * The compat_ioctl implementation.
 * This implementation allows compatibility between 32bit User applications
 * and 64bit kernel.  This ioctl is automatically called in that situation
 * in place of the unlocked_ioctl function cma_alloc_icoctl().
 * Refer to the file operations struct below.
 * This implementation is a direct copy of the existing cma_alloc_ioctl(),
 * but manages the user/kernel-space differences of the cma_user arg before
 * calling out to the same subfunctions.  Also manages those differences for
 * copying the results out to cma_user arg.
 */
static long
cma_alloc_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int err = 0, ret = 0;
    struct cma_allocation *alloc = NULL;
    struct cma_allocation *tmp_alloc = NULL;
    struct cma_user *cma = NULL;
    struct compat_cma_user *cma_frm32 = NULL;
    // struct compat_cma_user *cma_frm_arg = NULL;
    size_t size = 0;
    struct page *page = NULL;

    // dev_info(cma_dev, " compat IOCTL ");
    /* don't even decode wrong cmds: better returning  ENOTTY than EFAULT */
    if (_IOC_TYPE(cmd) != CMA_ALLOC_IOC_MAGIC) {
      dev_info(cma_dev, " wrong magic number");
      return -ENOTTY;
    }
    if (_IOC_NR(cmd) > CMA_ALLOC_IOC_MAXNR) {
      dev_info(cma_dev, " invalid command");
      return -ENOTTY;
    }
    /*
     * the type is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. Note that the type is user-oriented, while
     * verify_area is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
      err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
      err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    if (err)
        return -EFAULT;

    cma = (struct cma_user*)kmalloc(sizeof *cma, GFP_KERNEL);
    if (!cma)
        return -ENOMEM;

    cma_frm32 = (struct compat_cma_user*)kmalloc(sizeof *cma_frm32, GFP_KERNEL);
    if (!cma_frm32)
        return -ENOMEM;

    memset(cma, 0, sizeof *cma);
    memset(cma_frm32, 0, sizeof *cma_frm32);

    ret = copy_from_user(cma_frm32, compat_ptr(arg), sizeof *cma_frm32);

    /* debug prints to help understand 32/64bit handling */
    // dev_info(cma_dev, " arg ptr as lx  0x%lx", arg );
    // dev_info(cma_dev, " arg ptr as ptr 0x%p", arg );
    // dev_info(cma_dev, " arg ptr as compat_ptr 0x%p", compat_ptr(arg) );
    // dev_info(cma_dev, " sizeof cma_user %d", sizeof(struct cma_user));
    // dev_info(cma_dev, " sizeof compat_cma_user %d", sizeof(struct compat_cma_user));
    dev_info(cma_dev, " cma_frm32->size %d", cma_frm32->size);
    /* */

    if (ret) {
        dev_err(cma_dev, "copy from user failed. ret= %d, arg= 0x%lx, cma_frm32= 0x%lx\n",
                ret, arg, cma_frm32);
    } else if (cma_frm32->size <= 0) {
        dev_err(cma_dev, "invalid size %lu\n", cma_frm32->size);
        ret = -EINVAL;
    } else if (cma_frm32->size > (ULONG_MAX << PAGE_SHIFT)) {
        ret = -EOVERFLOW;
    }

    if(ret)
        goto error;

    // copy elements of user-space 32-bit version of cma_user
    // to our kernel version
    cma->dma  = (uint64_t)cma_frm32->dma;
    cma->virt = (void *)cma_frm32->virt;
    cma->size = (unsigned long)cma_frm32->size;

    switch(cmd) {
    case CMA_ALLOC_MEM:
        size = allocate_dma_mem(cma->size);
        if (size != (cma->size * SZ_1K)){
            ret = -EFAULT;
        } else {
            spin_lock(&cma_lock);
            if (!list_empty(&cma_allocations))
                alloc = list_last_entry(&cma_allocations,
                                         struct cma_allocation, list);
            spin_unlock(&cma_lock);
            if (alloc) {
                filp->private_data = (void *)alloc;
                cma->virt   = alloc->virt;
                cma->dma    = alloc->dma;
                cma->size   = alloc->size;
                dev_info(cma_dev, "allocated CM at virtual address: 0x%p "
                                  "dma address: 0x%llu size:%luKiB\n",
                         alloc->virt, alloc->dma, alloc->size / SZ_1K);

                // copy elements to user-space 32-bit version of cma_user
                // from our kernel version
                cma_frm32->dma  = (compat_u64)cma->dma;
                cma_frm32->virt = (compat_uptr_t)cma->virt;  // PBugni: unused
                cma_frm32->size = (compat_ulong_t)cma->size;

                ret = copy_to_user(compat_ptr(arg), cma_frm32, sizeof *cma_frm32);
                if (ret) {
                    dev_err(cma_dev, "copy to user failed %d\n", ret);
                    ret = -EFAULT;
                }
            } else
                ret = -EFAULT;
        }
        break;
    case CMA_FREE_MEM:
        if (!cma->dma || !cma->size || !cma->virt)
            ret = -EFAULT;
        else
            ret = free_dma_mem(cma);
        break;
    case CMA_ALLOC_MEM_V2:
        alloc = kmalloc(sizeof *alloc, GFP_KERNEL);
        if (!alloc)
            return -ENOMEM;

        page = dma_alloc_from_contiguous(cma_dev, PAGE_ALIGN(cma->size) >> PAGE_SHIFT, 0);
        if (page) {
            alloc->dma = page_to_phys(page);
            alloc->virt = page;
            alloc->size = PAGE_ALIGN(cma->size);

            spin_lock(&cma_lock);
            list_add_tail(&alloc->list, &cma_allocations);
            spin_unlock(&cma_lock);

            filp->private_data = (void *)alloc;
            cma->virt   = alloc->virt;
            cma->dma    = alloc->dma;
            cma->size   = alloc->size;
            dev_info(cma_dev, "allocated CM at virtual address: 0x%p "
                              "dma address: 0x%llx size:%luB\n",
                     alloc->virt, alloc->dma, alloc->size);

            // copy elements to user-space 32-bit version of cma_user
            // from our kernel version
            cma_frm32->dma  = (compat_u64)cma->dma;
            cma_frm32->virt = (compat_uptr_t)cma->virt;
            cma_frm32->size = (compat_ulong_t)cma->size;

            ret = copy_to_user(compat_ptr(arg), cma_frm32, sizeof *cma_frm32);
            if (ret) {
                dev_err(cma_dev, "copy to user failed %d\n", ret);
                ret = -EFAULT;
            }
        } else {
            dev_err(cma_dev, "no mem in CMA area\n");
            kfree(alloc);
            ret = -ENOSPC;
        }
        break;
    case CMA_FREE_MEM_V2:
        if (!cma->dma || !cma->size || !cma->virt)
            ret = -EFAULT;
        else {
            spin_lock(&cma_lock);
            if (!list_empty(&cma_allocations)) {
                list_for_each_entry_safe(alloc, tmp_alloc, &cma_allocations, list) {
                    if (alloc->dma == cma->dma)
                        break;
                }
                list_del(&alloc->list);
            }
            spin_unlock(&cma_lock);

            if (alloc) {
                dma_release_from_contiguous(cma_dev, alloc->virt, PAGE_ALIGN(alloc->size) >> PAGE_SHIFT);
                kfree(alloc);
            }
        }
        break;
    default:
        dev_info(cma_dev, " command default case ");
        ret = -ENOTTY;
    }

error:
    kfree(cma);
    return ret;
}

/*
 * cma_alloc_open
 * called when open is called on the driver
 */
static int cma_alloc_open(struct inode *i, struct file *f)
{
   dev_info(cma_dev, "Driver: open()\n");
   return 0;
} 

/*
 * cma_alloc_release
 * called when close is called on the driver
 */
static int cma_alloc_release(struct inode *i, struct file *f)
{
   dev_info(cma_dev, "Driver: closed()\n");
   return 0;
} 

/*
 * cma_alloc_mmap
 * called when mmap is called on the driver
 */
static int cma_alloc_mmap(struct file *filp, struct vm_area_struct *vma)
{
    struct cma_allocation *alloc = NULL;
    struct cma_allocation *tmp_alloc = (struct cma_allocation*)filp->private_data;
    if (!tmp_alloc)
        return -EFAULT;

    spin_lock(&cma_lock);
    if (!list_empty(&cma_allocations))
        list_for_each_entry(alloc, &cma_allocations, list) {
            if(alloc == tmp_alloc)
                break;
        }

    spin_unlock(&cma_lock);

    if (alloc) {
        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
        vma->vm_flags |= VM_IO;
        return vm_iomap_memory(vma, alloc->dma, vma->vm_end - vma->vm_start);
    } else
        return -EFAULT;
}

static const struct file_operations cma_alloc_fops = {
    .owner =    THIS_MODULE,
    .read  =    cma_alloc_read,
    .write =    cma_alloc_write,
    .open  =    cma_alloc_open,
    .release =  cma_alloc_release,
    .mmap   =   cma_alloc_mmap,
    .unlocked_ioctl =   cma_alloc_ioctl,
    .compat_ioctl =   cma_alloc_compat_ioctl,
};

static struct miscdevice cma_alloc_misc = {
    .name = "cma_alloc",
    .fops = &cma_alloc_fops,
};

static int __init cma_alloc_init(void)
{
    int ret = 0;

    ret = misc_register(&cma_alloc_misc);
    if (unlikely(ret)) {
        pr_err("failed to register cma-alloc misc device!\n");
        return -EINVAL;
    }
    cma_dev = cma_alloc_misc.this_device;
    cma_dev->coherent_dma_mask = ~0;
    _dev_info(cma_dev, "registered.\n");

    return 0;
}
module_init(cma_alloc_init);

static void __exit cma_alloc_exit(void)
{
    misc_deregister(&cma_alloc_misc);
}
module_exit(cma_alloc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Srinivas Pulukuru <srinivas.pulukuru@us.fujitsu.com>");
MODULE_DESCRIPTION("kernel module to help DMA CMA allocation");
MODULE_ALIAS("CMA Alloc");
