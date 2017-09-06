#! /bin/sh

#echo is safegaurd check
#find . -mindepth 1 -type d -exec echo cp index.php {} \;

#cp index into all subs
find . -mindepth 1 -type d -exec cp index.php {} \;

#ln soft php into all subs
#find . -mindepth 1 -type d -exec ln -s index.php {} \;
