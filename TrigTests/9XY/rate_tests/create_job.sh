#!/bin/bash

infile=$1

set -o nounset

echo "starting..."

jobindex=0
counter=0

filename=""

NFILESPERJOB=50

####################################################################
while read file
do
    
    let "counter=$counter+1"

    if [ $counter == 1 ]; then 
	let "jobindex=$jobindex+1"
    	cname="hlt_""$jobindex""_cfg.py"
	oname="hltbits_""$jobindex"".root"
    fi

    if [ $counter == 1 ]; then
        filename="'root://cms-xrd-global.cern.ch/""$file""'"
    else 
	filename="$filename, 'root://cms-xrd-global.cern.ch/""$file""'"
    fi


    if [ $counter == $NFILESPERJOB ]; then
	sed -e "s@INPUT_FILES@$filename@g" \
	    -e "s@OUTPUT_FILE@$oname@g" \
	    run_ntupleprod.py > $cname
    
	jname='myjob_'$jobindex
	echo $jname
	sed -e "s@INDX@$jobindex@g" \
	    -e "s@JOB_NAME@$cname@g" \
	    -e "s@FILE_PATH@$file@g" \
	    -e "s@OUTPUT_FILE@$oname@g"\
      submitjob.tmpl > $jname
	
	chmod 755 $jname
    
	echo "submitting job : "$jname
        #echo "bsub -q 1nd "$jname
	bsub -q 1nd $jname

	counter=0
	filename=""
    fi
    
done < $infile
####################################################################

if [ $counter != 0 ]; then
    sed -e "s@INPUT_FILES@$filename@g" \
	-e "s@OUTPUT_FILE@$oname@g" \
	run_ntupleprod.py > $cname
    
    jname='myjob_'$jobindex
    echo $jname
    sed -e "s@INDX@$jobindex@g" \
	-e "s@JOB_NAME@$cname@g" \
	-e "s@FILE_PATH@$file@g" \
	-e "s@OUTPUT_FILE@$oname@g"\
      submitjob.tmpl > $jname
    
    chmod 755 $jname
    
    echo "submitting job : "$jname
    #echo "bsub -q 1nd "$jname
    bsub -q 1nd $jname

fi
