#!/bin/bash

declare -a data=("B 190305_054958" "C 190305_055010" "D 190305_055022" "E 190305_055034" "F 190305_055046")
dir="/eos/cms/store/group/phys_exotica/displacedPhotons/nTuples/2017/analysis/unskimmed/DoubleEG"

output="files.txt"
> "${output}"

for datum in "${data[@]}"
do echo "${datum}" | while read -r era stamp
    do
	fulldir="${dir}/data_DoubleEG_Run2017${era}-31Mar2018-v1/${stamp}/0000"

	tmp_input="tmp.log"
	> "${tmp_input}"
	ls "${fulldir}" >> "${tmp_input}" 

	input="files.log"
	> "${input}"
	grep ".root" "${tmp_input}" >> "${input}"

	while IFS= read -r file
	do
	    echo "${fulldir}/${file}" >> "${output}"
	done < "${input}"
	
	rm "${tmp_input}" "${input}"
    done
done
