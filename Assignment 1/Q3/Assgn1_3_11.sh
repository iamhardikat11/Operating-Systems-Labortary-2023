#!/bin/bash

# # Input JSONL file
# jsonl_file=d_3.jsonl

# # Output CSV file
# csv_file=data.csv

# # Initialize header row
# header=""

# # Extract keys from first line of JSONL file
# keys=$(head -n 1 $jsonl_file | sed 's/[{}"]//g' | sed 's/:/,/g')
# echo $keys
# # Loop through keys and create header row
# for key in $keys; do
#     header="$header,$key"
# done

# # Remove leading comma
# header=${header:1}
# echo $header
# # Write header row to CSV file
# echo $header > $csv_file

# # Replace curly braces and quotes with commas
# # and append each line to the CSV file
# sed 's/[{}"]//g' $jsonl_file | sed 's/:/,/g' >> $csv_file
#!/bin/bash

# Input JSON file
jsonl_file=d_3.jsonl

# Extract first line of JSON file
first_line=$(head -n 1 $jsonl_file)

# Initialize keys array
keys=()

# Loop through characters in first line
i=0
while [ $i -lt ${#first_line} ]; do
  # Check for opening quotation mark
  if [ "${first_line:$i:1}" == "\"" ]; then
    # Initialize key variable
    key=""
    # Increment index
    i=$((i+1))
    # Loop through characters in key
    while [ "${first_line:$i:1}" != "\"" ]; do
      key="$key${first_line:$i:1}"
      i=$((i+1))
    done
    # Add key to keys array
    keys+=("$key,")
  fi
  if [ "${first_line:$i:1}" == ":" ]; then
     while [ $i -lt ${#first_line} ] && [ "${first_line:$i:1}" != "," ]; do
      i=$((i+1))
     done
  fi
  i=$((i+1))
done

# Print keys array
echo ${keys[@]} > data.csv
keys=$(head -n 1 $jsonl_file | sed 's/[{}"]//g' | sed 's/:/,/g')
echo $keys
for key in $keys; do
    echo $key
done

# # Remove leading comma
# header=${header:1}
# echo $header
# # Write header row to CSV file
# echo $header > data.csv

# # Replace curly braces and quotes with commas
# # and append each line to the CSV file
# sed 's/[{}"]//g' $jsonl_file | sed 's/:/,/g' >> data.csv
