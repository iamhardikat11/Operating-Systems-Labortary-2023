#!/bin/bash
file="fruits.txt"
while read item;
do
    flag=0
    if [[ $item =~ ^.{5,20}$ ]] && [[ $item =~ ^[a-zA-Z][a-zA-Z0-9]*[0-9][a-zA-Z0-9]*$ ]] 
    then
        echo "Hi"
        # while read f;
        # do
            if [[ $(echo -n "$item" | grep -iqrFc file) -ne 0 ]]; then
                flag=1
                break
            fi
        # done < "fruits.txt"
        # grep -iF "$item" "$file"
        #     echo $item 
        #     flag=1
        # fi
        # if [[ $flag -eq 0 ]]
        # then
        #     echo "YES"
        #     continue
        # fi
    fi
    if [[ $flag -eq 0 ]];
    then
        echo "NO"
    else
        echo "YES"
    fi
done < $1
