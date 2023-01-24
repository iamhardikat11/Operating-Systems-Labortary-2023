#!/bin/bash
for item in `cat $1`; do
    flag=0
    if [[${#item} -ge 5 ]] && [[ ${#item} -le 20 ]] && [[ $item =~ ^[a-zA-Z][a-zA-Z0-9]*[0-9][a-zA-Z0-9]*$ ]] 
    then
        for f in `cat fruits.txt`; do
            if [[ $(echo -n "$item" | grep -iFc "$f") -ne 0 ]]; then
                flag=1
                break
            fi
        done
        if [[ $flag -eq 0 ]]
        then
            echo "YES"
            continue
        fi
    fi
    echo "NO"
done
