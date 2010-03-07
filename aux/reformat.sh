#!/bin/sh

# Reformat the output of 'exifprobe -L' to remove the "." separators,
# which are not valid in shell variable names, and produce output
# which may be 'sourced' by a shell.  The value of the variable will
# be the last value present (the "interpreted" value, if available).
# Comments are stripped.

# This doesn't allow filenames on each line (exifprobe -n)

# Replace dots with underscores.
to_underscore()
{
    local nextarg

    nextarg=
    OIFS=${IFS}
    IFS="."
    set - $*
    echo -n $1
    shift
    # strip off asterisks
    for nextarg in $*
    do
        nextarg=${nextarg%[*]}
        echo -n _${nextarg%[*]}
    done
    IFS=${OIFS}
}

# Replace the dots and pick out the value, stripping comments.
reformat()
{
    local lastarg

    case $# in
        0) return ;;
    esac
    lastarg=
    endquote=
    to_underscore $1
    shift 
    # Pick out value after the last '=', quote it if not already quoted,
    # and strip off comments.
    while test $# -ge 1
    do
        case "$1" in
            \#*) break ;;
            \:*) break ;;
            =) lastarg=; shift ;;
           \') case "${lastarg}" in
                    "") lastarg="'" ;;
                     *) endquote="'"; break ;;
               esac
               ;;
           \'?*) lastarg="$1"; endquote=; shift ;;
            *) case "${lastarg}" in
                    "") lastarg="'$1"; endquote="'" ;;
                     *) lastarg="${lastarg} $1" ;;
               esac
               shift
               ;;
        esac
    done
    echo "=${lastarg}${endquote}" 
}

# Prepend value of 'MARK' to each line of output.
MARK=
case $# in
    0) ;;
    *) MARK="${1}:" ;;
esac

# expects output of 'exifprobe -L' (without '-n')
while read LINE
do
    case "${LINE}" in
        ?[*m\#*) continue ;;
        \#*) continue ;;
        "") continue ;;
    esac
    echo -n "${MARK}"
    reformat $LINE
done
