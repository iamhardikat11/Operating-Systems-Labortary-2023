#!/bin/bash
lcm=1
for num in $(rev $1); do 
    a=$lcm
    b=$num
    while [ $b -ne 0 ]
    do
        t=$b
        b=$(($a % $b))
        a=$t
    done
    lcm=$(($lcm*$(($num/$a))))
done
echo $lcm