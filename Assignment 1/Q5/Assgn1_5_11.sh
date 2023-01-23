#!/bin/bash

folder=$1
for file in $(find $folder -name "*.py"); do
    echo "-----------------"
    echo "-----------------"
    echo "Name of the file : $file"
        echo "-----------------"
    echo "-----------------"
    awk '
    /^[^#]/ {
        if(in_multiline_comment) {
            printf("Line %d-%d: %s\n", start_line, NR, multiline_comment);
            in_multiline_comment = 0;
            multiline_comment = "";
        }
    }
    /^#/ {
        if(in_multiline_comment) {
            multiline_comment = multiline_comment $0 "\n";
        } else {
            printf("Line %d: %s\n", NR, $0);
        }
    }
   
    ' $file
    echo "-----------------"
    echo "Multiline comments in the file"
    echo "-----------------"
    flag=0
    output=""
    while read -r word; do
      if [[ $word == *'"""'* ]]; then
        if [ $flag -eq 0 ]; then
          flag=1
          output="$output $word"
           while IFS= read -r line; do
        n=$((n+1))
        if [[ $line == *"$word"* ]]; then
              echo "line $n:"
              break
          fi
    
          done < "$file"
        else
          flag=0
          output="$output $word"
          n=0
        newline_count=0
     
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
    echo "-----------------"
   
done
