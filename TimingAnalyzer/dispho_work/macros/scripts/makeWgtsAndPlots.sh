## command line options
outdir=${1:-"plots/ntuples_v4/checks_v3/varwgts"}

## input configs
GJets="gjets signals_gjets always_true cr_gjets_DEG"
QCD="qcd signals_qcd cuts_v3/invertiso0_v0 cr_qcd_DEG"
SR="sr signals_sr always_true sr_SPH"

## text
orig="_orig"
wgt="_wgt"

## varmaps
empty=""
map="_map"

## input dirs
crtosrdir="crtosr_config"
varwgtdir="varwgt_config"

## make tmp directory for configs
tmpdir="tmp"
crtosrtmpdir="${crtosrdir}/${tmpdir}"
varwgttmpdir="${varwgtdir}/${tmpdir}"
mkdir -p "${crtosrtmpdir}"
mkdir -p "${varwgttmpdir}"

function makeTreePlot() 
{
    ## passed parameters
    plot=${1}
    input=${2}
    varwgtmap=${3}
    text=${4}

    echo ${!input} | while read -r infile insigfile sel label
    do
	## output filename
	outfile="${plot}_${label}${text}"

	echo "Creating input plot for: ${outfile}"
	
	## determine which misc file to use
	misc="misc"
	if [[ ${input} == "SR" ]] 
	then
	    if [[ ${plot} == *"met"* ]] || [[ ${plot} == "phoseedtime_0" ]] 
	    then
		misc="misc_blind"
	    fi
	fi
	
	 ## make the plot
	./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insigfile}.root" "cut_config/${sel}.txt" "${varwgtmap}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${outfile}" "${outdir}/${plot}"
    done
}

function makeCRtoSRPlot()
{
    ## passed parameters
    plot=${1}
    crinput=${2}
    srinput=${3}
    text=${4}
    
    ## derived parameters
    crlabel=$( echo ${!crinput} | cut -d " " -f 4 )
    srlabel=$( echo ${!srinput} | cut -d " " -f 4 )

    ## make config
    crtosrconfig="${crtosrtmpdir}/${crinput}_${plot}${text}.txt"
    echo "sample=${crinput}" >> "${crtosrconfig}"
    echo "cr_file=${plot}_${crlabel}${text}.root" >> "${crtosrconfig}"
    echo "sr_file=${plot}_${srlabel}.root" >> "${crtosrconfig}"
    echo "plot_config=plot_config/${plot}.txt" >> "${crtosrconfig}"
    echo "draw_scaled=1" >> "${crtosrconfig}"
    
    ## output filename
    outfile="${plot}_${crinput}${text}_CRtoSR"
    
    echo "Creating CRtoSR plot for: ${outfile}"

     ## make the plots
    ./scripts/runCRtoSRPlotter.sh "${crtosrconfig}" "${outfile}" "${outdir}/${plot}"
}

function makeVarWeights()
{
    ## passed parameters
    plot=${1}
    var=${2}
    crinput=${3}
    srinput=${4}
    text=${5}

    ## derived parameters
    crlabel=$( echo ${!crinput} | cut -d " " -f 4 )
    srlabel=$( echo ${!srinput} | cut -d " " -f 4 )
    skim=$( echo ${!crinput} | cut -d " " -f 1 )

    ## make tmp config
    varwgtconfig="${varwgttmpdir}/${crinput}_${plot}.txt"
    echo "sample=${crinput}" >> "${varwgtconfig}"
    echo "var=${var}" >> "${varwgtconfig}"
    echo "plot_config=plot_config/${plot}.txt" >> "${varwgtconfig}"
    echo "cr_file=${plot}_${crlabel}${text}.root" >> "${varwgtconfig}"
    echo "sr_file=${plot}_${srlabel}.root" >> "${varwgtconfig}"
    echo "skim_file=skims/${skim}.root" >> "${varwgtconfig}"

    echo "Making weights for: ${plot}, ${crinput}"

    ## make the weights
    ./scripts/runVarWeighter.sh "${varwgtconfig}"

    ## make map needed for next plotter
    varwgtmap="${varwgttmpdir}/${crinput}_${var}${map}.txt"
    echo "Data ${var}_wgt" >> "${varwgtmap}"
    echo "${crinput} ${var}_wgt" >> "${varwgtmap}"
}

## main call: loop over plots
while IFS='' read -r plot_info || [[ -n "${plot_info}" ]]
do
    if [[ ${plot_info} != "" ]]
    then
	plot=$( echo ${plot_info} | cut -d " " -f 1 )
	var=$( echo ${plot_info} | cut -d " " -f 2 )

	echo "Working on plot: ${plot}"

        ## make input plots
	makeTreePlot "${plot}" "GJets" "${varwgtdir}/${empty}" "${orig}"
	makeTreePlot "${plot}" "QCD" "${varwgtdir}/${empty}" "${orig}"
	makeTreePlot "${plot}" "SR" "${varwgtdir}/${empty}" ""
	
	## make CRtoSR plots
	makeCRtoSRPlot "${plot}" "GJets" "SR" "${orig}"
	makeCRtoSRPlot "${plot}" "QCD" "SR" "${orig}"
	
	## make var weights
	makeVarWeights "${plot}" "${var}" "GJets" "SR" "${orig}"
	makeVarWeights "${plot}" "${var}" "QCD" "SR" "${orig}"
	
        ## make input plots
	makeTreePlot "${plot}" "GJets" "${varwgttmpdir}/GJets_${var}${map}" "${wgt}"
	makeTreePlot "${plot}" "QCD" "${varwgttmpdir}/QCD_${var}${map}" "${wgt}"
	
	## make CRtoSR plots
	makeCRtoSRPlot "${plot}" "GJets" "SR" "${wgt}"
	makeCRtoSRPlot "${plot}" "QCD" "SR" "${wgt}"
	
    fi ## end check over empty plot
done < varwgt_config/plot_info.txt ## end loop over input plot configs

## clear up tmpdirs
echo "Removing tmp dirs: ${crtosrtmpdir}, ${varwgttmpdir}"
rm -r "${crtosrtmpdir}"
rm -r "${varwgttmpdir}"

## Final message
echo "Finishing MakingWgtsAndPlots"