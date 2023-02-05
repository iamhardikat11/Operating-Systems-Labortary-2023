#!/bin/bash
if [[ $2 -lt 1||$2 -gt 4||!(-f $1) ]];then
	echo "Please_check_that_column_number_is_between_1_and_4_and_Given_file_is_present_in_current_directory"
	exit 
fi

awk -v F="$2" '{$F=tolower($F);print}' $1>temp.txt
cat temp.txt>$1
awk -v F="$2" '{print$F}' $1>temp.txt
sort temp.txt|uniq -c|sort -r|awk '{print$2,$1}'>1c_output_${2}_column.freq
rm temp.txt
