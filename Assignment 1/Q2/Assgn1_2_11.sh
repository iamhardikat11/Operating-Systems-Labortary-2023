#!/bin/bash
# declare -a not_allowed
# for inv in `cat fruits.txt`; do
#         not_allowed[inv]=1
#     done
cnt=0
for item in `cat $1`; do
    l=${#item}
    if [[ $l -lt 5 ]] || [[ $l -gt 20 ]]; then
        cnt=$((cnt + 1))
        continue
    fi
    if [[ $item =~ ^[A-Za-z][A-Za-z0-9]*[0-9][A-Za-z0-9]*$ ]]; then
        for inv in `cat fruits.txt`; do
            if [[ $(echo -n inv | grep -iqF item) -eq 0 ]]; then
                break 1
            fi
        done
        echo "$cnt : $l : $item"
        cnt=$((cnt + 1))
    fi
done
echo "$cnt"