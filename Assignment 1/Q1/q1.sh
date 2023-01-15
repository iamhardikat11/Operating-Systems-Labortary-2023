#!/bin/bash
gcd() {
  if [ $2 -eq 0 ] 
   then 
    return $1
  else
    return $(gcd $2 `expr $1 % $2`)
  fi
}
ans=1
while read line; do
    let "num=$num | rev"
    gcd $ans $num
    let "ans=(ans*num)/$?"
done < $1
echo "$ans"