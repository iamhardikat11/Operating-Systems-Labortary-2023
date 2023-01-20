#!/bin/bash
for dir in $(find "$1" -type f -name "*.sh"); do
        if [[ "${dir}" =~ ^Assignment\ 1/Q ]]; then
            echo "Hi"
        fi
done