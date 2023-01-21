#!/bin/bash
limit=1000000
declare -a spf
Sieve_of_Eratosthene() {
    for((i=1; i<=limit; i++))
    do
        spf[i]=$i
    done
    for((i=4; i<=limit; i=i+2))
    do
        spf[i]=2
    done
    for((i=3; i*i<=limit; i++))
    do 
        if [[ ${spf[i]} -eq $i ]]; then
            for((j=i*i; j<=limit; j+=i))
            do
                if [ ${sp[j]} -eq $j ]; then
                    spf[j]=$i
                fi
            done   
        fi 
    done
}
Sieve_of_Eratosthene
while read -r x
do
    while [[ $x -ne 1 ]]
    do 
        echo -n "${spf[x]} "
        x=$((x/spf[x]))
    done
    echo
done < $1

