#!/bin/bash

## source first
source scripts/common_variables.sh

## command line options
outdir=${1:-"plots/ntuples_v4/checks_v3/DEG_test/varwgts"}
docleanup=${2:-"true"}

## text
orig="_orig"
wgt="_wgt"

## varmaps
empty="empty"
map="_map"

## make tmp directory for configs
tmpdir="tmp"
crtosrtmpdir="${crtosrconfigdir}/${tmpdir}"
varwgttmpdir="${varwgtconfigdir}/${tmpdir}"

function makeTreePlot() 
{
    ## passed parameters
    local plot=${1}
    local input=${2}
    local varwgtmap=${3}
    local text=${4}

    echo ${!input} | while read -r label infile insigfile sel
    do
	## output filename
	local outfile="${plot}_${label}${text}"

	echo "Creating input plot for: ${outfile}"
	
	## determine which misc file to use
	local misc=$(GetMisc ${input} ${plot})
	
	## make the plot
	./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${varwgtmap}.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${miscconfigdir}/${misc}.${inTextExt}" "${outfile}" "${outdir}/${plot}"
    done
}

function makeCRtoSRPlot()
{
    ## passed parameters
    local plot=${1}
    local crinput=${2}
    local srinput=${3}
    local text=${4}
    
    ## derived parameters
    local crlabel=$( echo ${!crinput} | cut -d " " -f 1 )
    local srlabel=$( echo ${!srinput} | cut -d " " -f 1 )

    ## make config
    local crtosrconfig="${crtosrtmpdir}/${crinput}_${plot}${text}.${inTextExt}"
    > "${crtosrconfig}"
     
    ## fill config
    echo "sample=${crinput}" >> "${crtosrconfig}"
    echo "cr_file=${plot}_${crlabel}${text}.root" >> "${crtosrconfig}"
    echo "sr_file=${plot}_${srlabel}.root" >> "${crtosrconfig}"
    echo "${plotconfigdir}=${plotconfigdir}/${plot}.${inTextExt}" >> "${crtosrconfig}"
    echo "draw_scaled=1" >> "${crtosrconfig}"
    
    ## output filename
    local outfile="${plot}_${crinput}${text}_CRtoSR"
    
    echo "Creating CRtoSR plot for: ${outfile}"

     ## make the plots
    ./scripts/runCRtoSRPlotter.sh "${crtosrconfig}" "${outfile}" "${outdir}/${plot}"
}

function makeVarWeights()
{
    ## passed parameters
    local plot=${1}
    local var=${2}
    local crinput=${3}
    local srinput=${4}
    local text=${5}

    ## derived parameters
    local crlabel=$( echo ${!crinput} | cut -d " " -f 1 )
    local srlabel=$( echo ${!srinput} | cut -d " " -f 1 )
    local skim=$( echo ${!crinput} | cut -d " " -f 2 )

    ## make tmp config
    local varwgtconfig="${varwgttmpdir}/${crinput}_${plot}.${inTextExt}"
    > "${varwgtconfig}"

    ## fill config
    echo "sample=${crinput}" >> "${varwgtconfig}"
    echo "var=${var}" >> "${varwgtconfig}"
    echo "${plotconfigdir}=${plotconfigdir}/${plot}.${inTextExt}" >> "${varwgtconfig}"
    echo "cr_file=${plot}_${crlabel}${text}.root" >> "${varwgtconfig}"
    echo "sr_file=${plot}_${srlabel}.root" >> "${varwgtconfig}"
    echo "skim_file=skims/${skim}.root" >> "${varwgtconfig}"

    echo "Making weights for: ${plot}, ${crinput}"

    ## make the weights
    ./scripts/runVarWeighter.sh "${varwgtconfig}"

    ## make map needed for next plotter
    local varwgtmap="${varwgttmpdir}/${crinput}_${var}${map}.${inTextExt}"
    > "${varwgtmap}"

    ## fill config
    echo "Data ${var}_wgt" >> "${varwgtmap}"
    echo "${crinput} ${var}_wgt" >> "${varwgtmap}"
}

## make tmp dirs
mkdir -p "${crtosrtmpdir}"
mkdir -p "${varwgttmpdir}"

## main call: loop over plots
while IFS='' read -r plot_info || [[ -n "${plot_info}" ]]
do
    if [[ ${plot_info} != "" ]]
    then
	plot=$( echo ${plot_info} | cut -d " " -f 1 )
	var=$( echo ${plot_info} | cut -d " " -f 2 )

	echo "Working on plot: ${plot}"

	## make input SR plot first (unchanged by weights)
	makeTreePlot "${plot}" "Signal" "${varwgtconfigdir}/${empty}" ""

	for CR in GJets QCD
	do
	    ## make input plots
	    makeTreePlot "${plot}" "${CR}" "${varwgtconfigdir}/${empty}" "${orig}"
	    
	    ## make CRtoSR plots
	    makeCRtoSRPlot "${plot}" "${CR}" "Signal" "${orig}"
	
	    ## make var weights
	    makeVarWeights "${plot}" "${var}" "${CR}" "Signal" "${orig}"
	
            ## make input plots (with weights applied)
	    makeTreePlot "${plot}" "${CR}" "${varwgttmpdir}/${CR}_${var}${map}" "${wgt}"
	
 	    ## make CRtoSR plots (final comparison)
	    makeCRtoSRPlot "${plot}" "${CR}" "Signal" "${wgt}"
	done ## end loop over control regions

    fi ## end check over empty plot
done < "${varwgtconfigdir}/plot_info.${inTextExt}" ## end loop over input plot configs

## clear up tmpdirs
if [[ "${docleanup}" == "true" ]]; then
    echo "Removing tmp dirs: ${crtosrtmpdir}, ${varwgttmpdir}"
    rm -r "${crtosrtmpdir}"
    rm -r "${varwgttmpdir}"
fi

## Final message
echo "Finishing MakingWgtsAndPlots"
