#!/bin/bash
filename=$1
input_word=$2
while read line;do
found=0
echo $line | grep -q $input_word && found=1
if [ $found -eq 1 ];then
echo "$line" | tr '[:lower:]' '[:upper:]' | sed -e:a -e 's/\([A-Z][^a-zA-Z]*\)\([A-Z]\)/\1\l\2/;ta'
else
echo $line
fi
done < $filename