#!/bin/bash

# source firts
source scripts/common_variables.sh

# read dir
indir=${1:-"madv2_v4p1/categories/full_chain"}

# derived
fulldir="${topdir}/${disphodir}/${indir}"

# move ex1 datacards and append
ex1phodir="${inlimitdir}/${ex1pho}"
mkdir -p "${ex1phodir}"

echo "Copying locally datacards for ${ex1pho}" 

for lambda in 100 150 200 250 300 350 400
do
    for ctau in 10
    do
	cp "${fulldir}/${ex1pho}/x_0.5_y_300/${incombdir}/${datacardname}_GMSB_L${lambda}_CTau${ctau}.${inTextExt}" "${ex1phodir}"
    done

    for ctau in 200 400 600 800 1000 1200 10000
    do
	cp "${fulldir}/${ex1pho}/x_1.5_y_200/${incombdir}/${datacardname}_GMSB_L${lambda}_CTau${ctau}.${inTextExt}" "${ex1phodir}"
    done
done

echo "Fixing datacards in ${ex1pho}" 

for file in "${ex1phodir}/"*".${inTextExt}"
do
    for bin in 1 2 3 4
    do
	sed -i 's/Bin'"${bin}"'/Bin'"${bin}"'\_ex1/g' "${file}"
    done

    sed -i 's/\_2017/\_ex1/g' "${file}"
done

# move in2 datacards
in2phodir="${inlimitdir}/${in2pho}"
mkdir -p "${in2phodir}"

echo "Copying locally datacards for ${in2pho}" 

for lambda in 100 150 200 250 300 350 400
do
    for ctau in 10
    do
	cp "${fulldir}/${in2pho}/x_0.5_y_150/${incombdir}/${datacardname}_GMSB_L${lambda}_CTau${ctau}.${inTextExt}" "${in2phodir}"
    done

    for ctau in 200 400 600 800 1000 1200 10000
    do
	cp "${fulldir}/${in2pho}/x_1.5_y_150/${incombdir}/${datacardname}_GMSB_L${lambda}_CTau${ctau}.${inTextExt}" "${in2phodir}"
    done
done

echo "Fixing datacards in ${in2pho}" 

for file in "${in2phodir}/"*".${inTextExt}"
do
    for bin in 1 2 3 4
    do
	sed -i 's/Bin'"${bin}"'/Bin'"${bin}"'\_in2/g' "${file}"
    done

    sed -i 's/\_2017/\_in2/g' "${file}"
done
