#!/bin/bash
if [ ! -f main.csv ]; then
  `touch main.csv`
fi

insert_record() {
  echo "$1,$2,$3,$4" >> main.csv
  echo "Inserted $1,$2,$3,$4 in main.csv"
  echo
}
calculate_category_total() {
  category_total=$(awk -F, -v category="$1" '$2 == category { sum += $3 } END { print sum }' main.csv)
  echo "Total amount spent on $1: $category_total"
  echo 
}
calculate_name_total() {
  name_total=$(awk -F, -v name="$1" '$4 == name { sum += $3 } END { print sum }' main.csv)
  echo "Total amount spent by $1: $name_total"
  echo
}
# Helper function to sort the csv by column
sort_csv() {
  col=0
  case $1 in
    Date)sort -t, -k3.1,3.2 -k2 -k1n main.csv -o main.csv ;;
    Category)sort -t, -k2,2 -o main.csv main.csv ;;
    Amount)sort -t, -k3,3  -n -o main.csv main.csv ;;
    Name)sort -t, -k4,4 -o main.csv main.csv ;;
    \*)
      echo "INVALID!!"
      ;;
    esac
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
      CATEGORY="$OPTARG"
      shift 2
      ;;
    n)
      NAME="$OPTARG"
      shift 2
      ;;
    s)
      SORT="$OPTARG"
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
sort -t, -k3.1,3.2 -k2 -k1n main.csv -o main.csv 
# sort -t, -k2,2 --ignore-leading-tabs -n -o main.csv main.csv
if [ -n "$CATEGORY" ]; then
  calculate_category_total "$CATEGORY"
fi
if [ -n "$NAME" ]; then
  calculate_name_total "$NAME"
fi
if [ -n "$SORT" ]; then
  sort_csv "$SORT"
fi
