#!/usr/local/bin/bash

input=${1}
indir=${2}

indir=$( echo "${indir}" | sed 's|\_/|\\\_|g' )
indir=$( echo "${indir}" | sed 's|\/|\\\/|g' )

gsed -i 's/GMSB\_L//g' "${input}"
gsed -i 's/\_CTau/ /g' "${input}"
gsed -i 's|'"${indir}"'||g' "${input}"
gsed -i 's/x\_//g' "${input}"
gsed -i 's/\_y\_/ /g' "${input}"
