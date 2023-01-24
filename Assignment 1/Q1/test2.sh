#!/bin/bash
# Read numbers from file
numbers=($(grep -o '[0-9]*' lcm.txt))
# Reverse the numbers
reversed_numbers=($(echo ${numbers[@]} | sed 's/ /\n/g' | awk '{print reverse}'))
for i in reversed_numbers
# Find LCM
lcm=$(echo "scale=0; lcm=${reversed_numbers[0]}; for (i=1; i<${#reversed_numbers[@]}; i++) { lcm=lcm*${reversed_numbers[i]}/gcd(lcm, ${reversed_numbers[i]}); } print lcm" | bc)

# echo "The LCM of the reversed numbers in the file is: $lcm"