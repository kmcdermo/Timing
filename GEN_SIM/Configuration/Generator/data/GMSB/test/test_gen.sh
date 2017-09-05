#!/bin/bash

lambda=100
ctau=0.1

root -l -b -q test/getCgrav.C\(${lambda},${ctau}\) > cgrav.txt
cgrav=$(grep "cgrav" cgrav.txt | cut -d ' ' -f 2) 
rm cgrav.txt

lamb=$((${lambda}*1000))
mess=$((${lamb}*2)) # messenger mass is set to 2 * lambda in SPS8

echo -e "temp.txt\ntemp.slha\n/\n2\n"${lamb}" "${mess}" 1 15 1 173.1 "${cgrav}"\n0\n/" | ./isasugra.x
rm temp.txt

width=$(grep "DECAY   1000022" temp.slha | cut -d ' ' -f6)

echo "Test lambda:" ${lambda} "test ctau:" ${ctau}
root -l -b -q test/getCtau.C\(${width}\)
rm temp.slha