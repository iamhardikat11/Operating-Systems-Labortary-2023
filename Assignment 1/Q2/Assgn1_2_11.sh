#!/bin/bash
while read username;
do
    if [[ $username =~ ^.{5,20}$ ]] && [[ $username =~ ^[a-zA-Z][a-zA-Z0-9]*[0-9][a-zA-Z0-9]*$ ]] <<< "$username" && ! grep -iqFf "fruits.txt" <<< "$username"
    then
        echo "YES"
    else
        echo "NO"
    fi
done < $1
