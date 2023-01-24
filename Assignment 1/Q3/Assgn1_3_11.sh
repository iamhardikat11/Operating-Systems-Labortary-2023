#!/bin/bash
# Assign the arguments to variables
json_dir=$1
csv_dir=$2
attributes=("$@")
# Remove the first two arguments from the attributes array
attributes=("${attributes[@]:2}")
# Loop through each file in the json_dir directory
for file in "$json_dir"/*
do
  # Get the base file name
  base_file_name=$(basename "$file" .jsonl)
  # Create the csv file path
  csv_file_path="$csv_dir/$base_file_name.csv"
  # Print the header row with the attribute names
  echo "${attributes[*]}" > "$csv_file_path"
  # Loop through each line in the json file
  while read -r line
  do
    # Initialize the csv row
    csv_row=""
    # Loop through each attribute
    for attribute in "${attributes[@]}"
    do
      # Extract the value of the attribute from the json object
      value=$(echo "$line" | jq -r ".$attribute")
      # Check if the value contains a comma
      if [[ $value == *","* ]]
      then
        # If it does, wrap the value in double quotes
        value="\"$value\""
      fi
      # Append the value to the csv row
      csv_row="$csv_row,$value"
    done
    # Print the csv row to the csv file
    echo "${csv_row:1}" >> "$csv_file_path"
  done < "$file"
done