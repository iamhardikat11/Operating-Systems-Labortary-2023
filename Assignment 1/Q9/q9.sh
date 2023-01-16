#!/bin/bash
# awk '{print $2}' < $1 | uniq -c | sort -k
awk '{a[$2]++} END{for(s in a){print s" "a[s]}}' | sort $1