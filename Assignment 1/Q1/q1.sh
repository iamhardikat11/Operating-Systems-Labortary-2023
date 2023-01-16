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
for num in $(rev $1); do
gcd $ans $num
let "ans=(ans*num)/$?"
done
echo $ans