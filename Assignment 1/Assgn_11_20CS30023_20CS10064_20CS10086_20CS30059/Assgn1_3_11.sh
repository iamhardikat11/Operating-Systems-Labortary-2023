#!/bin/bash
json_dir=$1
csv_dir=$2
attributes=("$@")
attributes=("${attributes[@]:2}")
for file in "$json_dir"/*
do
  base_file_name=$(basename "$file" .jsonl)
  csv_file_path="$csv_dir/$base_file_name.csv"
  echo "${attributes[*]}" > "$csv_file_path"
  while read -r line
  do
    csv_row=""
    for attribute in "${attributes[@]}"
    do
      value=$(echo "$line" | jq -r ".$attribute")
      if [[ $value == *","* ]]
      then
        value="\"$value\""
      fi
      csv_row="$csv_row,$value"
    done
    echo "${csv_row:1}" >> "$csv_file_path"
  done < "$file"
done