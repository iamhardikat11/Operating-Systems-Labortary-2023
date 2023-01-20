#!/bin/bash
mkdir -p "$2"
for file in $(find "$1" -type f -name "*.txt"); do
    for letter in {a..z}; do
        grep "^[$letter$(echo "$letter" | tr [a-z] [A-Z])]" $file | sort > "$2/$letter".txt
    done
done

