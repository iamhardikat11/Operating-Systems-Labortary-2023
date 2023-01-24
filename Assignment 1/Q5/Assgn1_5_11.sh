#!/bin/bash

folder=$1

if [ -z "$folder" ]; then
    echo "Usage: $0 <folder>"
    exit 1
fi

if [ ! -d "$folder" ]; then
    echo "Error: $folder is not a directory"
    exit 1
fi

for file in $(find $folder -name "*.py"); do
    echo $file
   flag=0
    n=0
    output=""
    while read -r word; do
      
      if [[ $word == *'"""'* ]]; then
        if [ $flag -eq 0 ]; then
          flag=1
          output="$output $word"
          temp=0
            while IFS= read -r line; do
            temp=$((temp+1))
        if [ $temp -gt $n ]; then

  
        n=$((n+1))
        if [[ $line == *"$word"* ]]; then
              echo "line $n:"
              break
          fi
        fi  
        done < "$file"
        else
          flag=0
          output="$output $word"
          echo "$output"
          output=""
        fi
      elif [ $flag -eq 1 ]; then
        output="$output $word"
      else
        if [ -n "$output" ]; then
          echo "$output"
          output=""
        fi
      fi
    done < <(tr -s '[[:space:]]' '\n' < $file)
    if [ -n "$output" ]; then
      echo "$output "
    fi
awk '{if(/^[^'\''"]*#/) {gsub(/^[^#]+#/, "#", $0);print "Line " NR ": " $0;} else if(/#[^'\''"]*$/) {gsub(/^.*#/, "#", $0);print "Line " NR ": " $0;} else if(/^.*#[^'\''"]*$/) {gsub(/^.*#/, "#", $0);print "Line " NR ": "  $0;}}' $file
done
