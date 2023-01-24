#!/bin/bash
# Name of the script: Assgn1_8_<groupno>.sh
# Check if main.csv exists, if not create it
if [ ! -f main.csv ]; then
  `touch main.csv`
  echo "Date,Category,Amount,Name" > main.csv
fi

# Helper function to insert a new record into main.csv
insert_record() {
  echo "Hi"
  echo "$1,$2,$3,$4" >> main.csv
  echo "Inserted $1,$2,$3,$4 in main.csv"
  echo
}

# Helper function to calculate total amount spent for a category
calculate_category_total() {
  category_total=$(awk -F, -v category="$1" '$2 == category { sum += $3 } END { print sum }' main.csv)
  echo "Total amount spent on $1: $category_total"
  echo 
}

# Helper function to calculate total amount spent by a person
calculate_name_total() {
  name_total=$(awk -F, -v name="$1" '$4 == name { sum += $3 } END { print sum }' main.csv)
  echo "Total amount spent by $1: $name_total"
  echo
}

# Helper function to sort the csv by column
sort_csv() {
  k=1
  str1="Date"
  str2="Category"
  str3="Amount"
  str4="Name"
#   if [[ "$1" == "$str1" ]]; then
#    awk -i inplace -F, '{split($1,a,"-"); print a[3]a[2]a[1], $0}' main.csv | sort | cut -d " " -f 2-
#     k=1
#   fi  
#   if [[ "$1" == "$str2" ]]; then
#     k=2
#   fi 
#   if [[ "$1" == "$str3" ]]; then
#     k=3
#   fi 
#   if [[ "$1" == "$str4" ]]; then
#     k=4
#   fi 
    # case $1 in
    # Date)
    # ;;
    # Category)
    # ;;
    # Amount)
    # ;;
    
  sort -t, -k$1 main.csv -o main.csv
  echo $k
  echo "main.csv sorted by $1 column"
}
show_help() {
  echo "Utility Name: Expense Tracker"
  echo "Usage: sh Assgn1_8_11.sh [-c category] [-n name] [-s column] record [record...]"
  echo "Description: This script is used to track expenses. It accepts records in the form of 'dd-mm-yyyy category amount name' and inserts them into main.csv. 
        It also accepts flags '-c' to show total amount spent on a category, '-n' to show total amount spent by a name, 
        and '-s' to sort the csv by a column. By default, the csv is sorted by the 'Date' column."
}
cnt=0
while getopts ":c:n:s:h" opt; do
  case $opt in
    c)
      CATEGORY="$2"
      shift 2
      ;;
    n)
      NAME="$2"
      shift 2
      ;;
    s)
      SORT="$2"
      shift 2
      ;;
    h)
        show_help
        shift 1
        echo
      ;;
    \?)
      echo "Invalid option: -$OPTARG"
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument."
      exit 1
      ;;
  esac
done

insert_record "$1" "$2" "$3" "$4"
#sort_csv
if [ -n "$CATEGORY" ]; then
  calculate_category_total "$CATEGORY"
fi
if [ -n "$NAME" ]; then
  calculate_name_total "$NAME"
fi
if [ -n "$SORT" ]; then
  sort_csv "$SORT"
fi
# Debug Sort the CSV 
