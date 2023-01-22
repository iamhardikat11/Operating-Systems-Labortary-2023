#!/bin/bash
mkdir -p "$2"
for letter in {a..z}; do
    grep -rihE "^[$letter]" $1 | sort > "$2/$letter".txt
done