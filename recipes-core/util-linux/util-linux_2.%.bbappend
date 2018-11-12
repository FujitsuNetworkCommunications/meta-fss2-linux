# Add missing RHEL 6.4 kernel flag but only for native builds.
BUILD_CPPFLAGS += " -D__NR_setns=326 "

# Work around bug in util-linux Makefile fixed in 2.25.1.  Don't port
# this fix to that version.
# http://comments.gmane.org/gmane.linux.utilities.util-linux-ng/9799
inherit autotools-brokensep
SEPB = "${S}"

PR := "${PR}.1"
