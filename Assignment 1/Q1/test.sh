ans=1
for num in $(rev $1); do
echo $num
a=$num; b=$ans;
while ((b)) ; do
    t=$b; 
    b=$(a%b);
    a=$t;
done; 
a=$((num/a))
ans=$((ans*a))
done
echo $ans