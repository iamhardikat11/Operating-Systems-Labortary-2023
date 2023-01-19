#!/bin/bash
limit=1000000
declare -a prime
Sieve_of_Eratosthene() {
    for((i=1; i<=limit; i++))
    do
        prime[i]=1
    done
    prime[1]=0
    for((p=2; p*p<=limit; p++))
    do 
        if [[ ${prime[p]} -eq 1 ]]; then
            for((i=p*p; i<=limit; i+=p))
            do
                prime[i]=0
            done   
        fi 
    done
}
Sieve_of_Eratosthene
while read -r n
do
    for((i=1; i<=n; i++))
    do
        if [[ ${prime[i]} -eq 1 ]]; then
            echo -n "$i "
        fi 
    done
    echo
done < $1
