#!/bin/bash
declare -a spf

Sieve_of_Eratosthenes() {
    for((i=2; i<=1000000; i++))
    do
        spf[i]=$i
    done
    for((i=4; i<=1000000; i=i+2))
    do
        spf[i]=2
    done
    for((i=3; i*i<=1000000; i=i+2))
    do 
        if [[ ${spf[i]} -eq $i ]]; then
            for((j=i*i; j<=1000000; j+=i))
            do
                if [[ ${spf[j]} -eq $j ]]; then
                    spf[j]=$i
                fi
            done   
        fi 
    done
}

Sieve_of_Eratosthenes
while read -r x
do
    while [[ $x -ne 1 ]]
    do 
        echo -n "${spf[x]} "
        x=$((x/spf[x]))
    done
    echo
done < $1

echo 5 > input1.txt
echo > input1.txt
echo 15 > input1.txt
echo > input1.txt
echo 25 > input1.txt
echo > input1.txt
echo 22 > input1.txt