/* -*- C -*-
 * cma-test.h -- definitions for the cma-test misc driver module
 *
 * Copyright (C) 2016 FNC
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files. No warranty
 * is attached; we cannot take responsibility for errors or
 * fitness for use.
 */

#ifndef _UAPI__LINUX_CMA_ALLOC_H__
#define _UAPI__LINUX_CMA_ALLOC_H__

#include <linux/types.h>
#include <asm/compat.h>         // for compat data types for 32/64bit support

/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef CMA_ALLOC_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "cma_alloc: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUG
#define PDEBUG(fmt, args...) /* nothing: it's a placeholder */


/*
 * Ioctl definitions
 */
#define CMA_ALLOC_IOC_MAGIC         'C'
#define CMA_ALLOC_IOW(num, dtype)	_IOW('C', num, dtype)
#define CMA_ALLOC_IOR(num, dtype)	_IOR('C', num, dtype)
#define CMA_ALLOC_IOWR(num, dtype)	_IOWR('C', num, dtype)
#define CMA_ALLOC_IO(num)		    _IO('C', num)

#define CMA_ALLOC_MEM		CMA_ALLOC_IOWR(1, int)
#define CMA_FREE_MEM		CMA_ALLOC_IOWR(2, int)
#define CMA_ALLOC_MEM_V2    CMA_ALLOC_IOWR(3, int)
#define CMA_FREE_MEM_V2   CMA_ALLOC_IOWR(4, int)

#define CMA_ALLOC_IOC_MAXNR 4

struct cma_user {
    uint64_t dma;
    void *virt;
    unsigned long size;
};

struct compat_cma_user {
    compat_u64 dma;
    compat_uptr_t virt;
    compat_ulong_t size;
};

#endif //_UAPI__LINUX_CMA_ALLOC_H__
