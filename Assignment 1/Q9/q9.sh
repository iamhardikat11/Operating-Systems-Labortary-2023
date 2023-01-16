#!/bin/bash
# awk '{print $2}' < $1 | uniq -c | sort -k
# awk '{a[$2]++} END{for(s in a){print s" "a[s]}}'
awk '{ print $2 }' $1 | uniq -c | sort -k 2 -t, -rn
echo
awk '{count[$1]++} END {for ( i in count ) {if(count[i] >= 2) {print i} if(count[i] == 1) {cnt++} } {print cnt} }' $1
# awk '{if(count[$1]++ >= max) max = count[$1]} END {for ( i in count ) if(max == count[i]) print i, count[i] }' $1
