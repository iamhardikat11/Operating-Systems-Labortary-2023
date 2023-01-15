#!/bin/bash
# cnt=0
# while read -r l
# do 
#     if [[ $l == *"kharagpur"* ]] ; then
#         echo "$l@@"
#     else
#         echo "$l"
#     fi
# done < "$1"
# echo "$cnt"
word=$1
line='Just som3 rand0m text with spec!4L CHARS?”'
# while read line; do
# if echo $line | grep -q $word; then
    echo $line | tr '[a-z]' '[A-Z]' | tr '[A-Z]' '[a-z]'
# else
#     echo $line
# fi
