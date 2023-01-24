#!/bin/bash
awk '{c[$2]++} END {for (i in c) {print i " " c[i]} print ""}' $1 | sort -k2nr -k1
awk '{c[$1]++} END {for (i in c) {if(c[i]>=2) {print i} if(c[i]==1) {n++}} print n}' $1