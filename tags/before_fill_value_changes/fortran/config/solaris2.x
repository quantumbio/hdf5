#							-*- shell-script -*-
#
# This file is part of the HDF5 build script.  It is processed shortly
# after configure starts and defines, among other things, flags for
# the various compile modes.
#
# See BlankForm in this directory for details

# The default compiler is `sunpro cc'
if test "X-" =  "X-$CC"; then
    CC=cc
    CC_BASENAME=cc
fi

# Try gcc compiler flags
. $srcdir/config/gnu-flags

# Try solaris native compiler flags
if test "X-" = "X-$cc_flags_set"; then
    CFLAGS="-erroff=%none"
    DEBUG_CFLAGS=-g
    DEBUG_CPPFLAGS=
    PROD_CFLAGS="-O -s"
    PROD_CPPFLAGS=
    PROFILE_CFLAGS=-xpg
    PROFILE_CPPFLAGS=
    cc_flags_set=yes
    # Turn off optimization flag for SUNpro compiler versions 4.x which
    # have an optimization bug.  Version 5.0 works.
    ($CC -V 2>&1) | grep -s 'cc: .* C 4\.' >/dev/null 2>&1 \
	&& PROD_CFLAGS="`echo $PROD_CFLAGS | sed -e 's/-O//'`"
fi

# The default Fortran 90 compiler

#
# HDF5 integers
#
# 	R_LARGE is the number of digits for the bigest integer supported.
#	R_INTEGER is the number of digits in INTEGER
#
# (for the Sparc Solaris architechture)
#
R_LARGE=18
R_INTEGER=9
HSIZE_T='SELECTED_INT_KIND(R_LARGE)'
HSSIZE_T='SELECTED_INT_KIND(R_LARGE)'
HID_T='SELECTED_INT_KIND(R_INTEGER)'
SIZE_T='SELECTED_INT_KIND(R_INTEGER)'
OBJECT_NAMELEN_DEFAULT_F=-1

if test "X-" = "X-$F9X"; then
    F9X=f90
fi

if test "X-" = "X-$f9x_flags_set"; then
    F9XSUFFIXFLAG=""
    FSEARCH_DIRS=""
    FFLAGS=""
    DEBUG_FFLAGS=""
    PROD_FFLAGS=""
    PROFILE_FFLAGS=""
    f9x_flags_set=yes
fi
