#!/bin/bash
awk '{count[$2]++} END {for ( i in count ) { print i " " count[i]} print "" }' $1 | sort -k2,2nr -k1,1
awk '{count[$1]++} END {for ( i in count ) {if(count[i] >= 2) {print i} if(count[i] == 1) {cnt++} } {print cnt} }' $1