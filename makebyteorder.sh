#!/bin/sh
# @(#) $Id: makebyteorder.sh,v 1.1 2003/01/12 19:30:31 alex Exp $

if test -f /usr/include/machine/endian.h
then
    byteorder=`grep BYTE_ /usr/include/machine/endian.h`
elif test -f /usr/include/bits/endian.h
then
    byteorder=`grep BYTE_ /usr/include/bits/endian.h`
fi

case "${byteorder}" in
    *BIG*) echo "#define NATIVE_BYTEORDER_BIGENDIAN" > byteorder.h ;;
    *LITTLE*) echo "#define NATIVE_BYTEORDER_LITTLE_ENDIAN" > byteorder.h ;;
    *) echo "#define NATIVE_BYTEORDER_UNKNOWN /* assume little-endian */" > byteorder.h ;;
esac

exit 0

