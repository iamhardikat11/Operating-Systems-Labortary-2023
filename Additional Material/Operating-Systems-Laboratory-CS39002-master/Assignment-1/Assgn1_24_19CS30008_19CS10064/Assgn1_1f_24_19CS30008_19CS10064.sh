awk '{print tolower($'$2')}' $1|sort|uniq -c|awk '{print $2,$1}'|sort -nrk2>1f_output_$2_column.freq