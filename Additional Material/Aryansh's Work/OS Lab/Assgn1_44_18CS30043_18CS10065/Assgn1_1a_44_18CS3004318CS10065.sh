#!/bin/bash
IFS=',' read -a ARR<<<$1
if [[ ${#ARR[@]} -lt 2||${#ARR[@]} -gt 9 ]];then
	echo "Enter_more_than_1_number_and_less_than_10_numbers"
	exit
fi

GCD=${ARR[0]}


for i in ${ARR[@]};do
	if [[ $i -lt 0 ]];then
		i=$((0-$i))
	fi
	while [[ $i -ne 0 ]];do
		x=$GCD
		GCD=$i
		i=$(($x%$i));done;done
echo $GCD
