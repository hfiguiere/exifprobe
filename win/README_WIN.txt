How to build exifprobe with Microsoft Windows Visual Studio 2010
================================================================

The "win" directory contains a Visual Studio 2010 solution which builds
exifprobe under the Windows OS.

The Visual Studio solution has successfully been used to compile the original
exifprobe 2.0.1 sources developed 2002-2005 by Duane H. Hesser (who died in
2012) as well as the forked and improved version 2.1.0 from Hubert Figuiere
(this code, available at https://github.com/hfiguiere/exifprobe).

The original 2.0.1 source code archive can be found at
http://archive.ubuntu.com/ubuntu/pool/universe/e/exifprobe/exifprobe_2.0.1.orig.tar.gz

* The Visual Studio project file exifprobe.vcxproj contains the following
  custom settings:
  - Additional Include Directories: .
  - Preprocessor: u_int32_t=uint32_t  strncasecmp=_strnicmp

* The following files have been added in the "win" directory. They are only used
  when exifprobe is build under Windows.
  - a file byteorder.h with defines NATIVE_BYTEORDER_LITTLE_ENDIAN
  - a file comptime.h
  - an empty file as a replacement for unistd.h
  - a replacement for getopt.h and getopt.c (with BSD license) from
    http://www.rohitab.com/discuss/topic/39420-a-proper-port-of-getopt/
  - a file snprintf.c which wrapps the Windows _snprintf() function to work around its non
    standard negative return value, see http://stackoverflow.com/a/13067917/321013

* The following modifications had to be applied to the original 2.0.1 source code

  diff exifprobe-2.0.1.orig/main.c exifprobe-2.0.1/main.c
  86c86
  <         inptr = fopen(file,"r");
  ---
  >         inptr = fopen(file,"rb");
  
  diff exifprobe-2.0.1.orig/process.c exifprobe-2.0.1/process.c
  254c254,255
  <                         fseek(inptr,0L,current_offset);
  ---
  >                         fseek(inptr,current_offset,0);
   
  diff exifprobe-2.0.1.orig/readfile.c exifprobe-2.0.1/readfile.c
  25a26
  > #include <stdint.h>

* To generate the Windows executable open and build the solution exifprobe.sln
  in Visual Studio 2010 or later.
  A number of warnigns will be displayed (mixing unsigned and signed integers,
  use of deprecated posix functions).
  Finally the generated exifprobe.exe file can be found in the Debug directory.
