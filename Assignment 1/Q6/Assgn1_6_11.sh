#!/bin/bash
declare -a minPrimeFactor

Sieve_of_Eratosthenes() {
    for((i=2; i<=1000000; i++))
    do
        minPrimeFactor[i]=$i
    done
    for((i=4; i<=1000000; i=i+2))
    do
        minPrimeFactor[i]=2
    done
    for((i=3; i*i<=1000000; i=i+2))
    do 
        if [[ ${minPrimeFactor[i]} -eq $i ]]; then
            for((j=i*i; j<=1000000; j+=i))
            do
            if [ ${minPrimeFactor[j]} -eq $j ]; then
                minPrimeFactor[j]=$i
            fi
            done   
        fi 
    done
}

Sieve_of_Eratosthenes
while read -r x
do
    declare -a factors
    while [[ $x -ne 1 ]]
    do 
        factors[minPrimeFactor[x]]=1
        x=$((x/minPrimeFactor[x]))
    done
    for i in "${!factors[@]}"; do
        echo -n "$i " >> output.txt
    done
    echo >> output.txt
    factors=()
done < input.txt


