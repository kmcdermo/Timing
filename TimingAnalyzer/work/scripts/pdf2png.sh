#!/bin/sh

#hack solution from interwebz to batch convert pdf to png
#top is the top level directory to go and convert everything

top=$1

if [ "$top" = "" ]; then
echo "Need a top level directory to execute script!"
exit 0
fi 

{ find $top -type d; echo; } |
awk 'index($0,prev"/")!=1 && NR!=1 {print prev}
     1 {sub(/\/$/,""); prev=$0}' > dirs.txt

while IFS='' read -r line || [[ -n "$line" ]]; do
    dir=$line
    for i in ${dir}/*.pdf; do 
	sips -s format png $i --out $i.png
    done
    rename .pdf.png .png ${dir}/*.pdf.png
done < dirs.txt

rm dirs.txt
