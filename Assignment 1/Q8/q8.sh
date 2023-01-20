# !/bin/bash
# if [ ! -f "main.csv" ]
# then
#     `touch main.csv`
#     echo "Date (dd-mm-yy),Category,Amount,Name" > main.csv
# fi
# if [ ! -z "$1" ] 
# then
#     if [ "$1" == "-c" ]; 
#     then
#         # cat=$2
#         echo "1"
#         # awk -F, '{sum = 0} $2 == "Food" { sum += $3 } END { print sum }' main.csv
#     elif [ "$1" == "-n" ]; 
#     then 
#         # name=$2
#         echo "2"
#         # awk -F, '{sum = 0} $4 == "Lakhan" { sum += $3 } END { print sum }' main.csv
#     elif [ $1 -eq "-s" ] 
#     then
#         echo "3"
#     elif [ $1 -eq "-h" ] 
#     then
#         echo "4"
#     else 
#         echo "$1,$2,$3,$4" >> main.csv
#         sort -t, -k1,1n -o main.csv main.csv
# fi

# # usage() { echo "$0 usage:" && grep " .)\ #" $0; exit 0; }
# # [ $# -eq 0 ] && usage
# # while getopts ":hs:p:" arg; do
# #   case $arg in
# #     p) # Specify p value.
# #       echo "p is ${OPTARG}"
# #       ;;
# #     s) # Specify strength, either 45 or 90.
# #       strength=${OPTARG}
# #       [ $strength -eq 45 -o $strength -eq 90 ] \
# #         && echo "Strength is $strength." \
# #         || echo "Strength needs to be either 45 or 90, $strength found instead."
# #       ;;
# #     h | *) # Display help.
# #       usage
# #       exit 0
# #       ;;
# #   esac
# # done
#!/bin/bash

# Assign script name
script_name="Assgn1_8_<groupno>.sh"
# Shift flags and remaining arguments to separate variables
shift $((OPTIND-1))
echo "$1,$2,$3,$4" >> main.csv
sort -t, -k1,1n -o main.csv main.csv
# Check if main.csv exists, create it if it doesn't
if [ ! -f "main.csv" ]; then
  echo "Date (dd-mm-yy),Category,Amount,Name" > main.csv
fi

# Check for flags and perform corresponding actions
while getopts ":c:n:s:h" opt; do
  case $opt in
    c)
    #   category="$OPTARG"
      awk -F "," -v cat="$OPTARG" '$2 == cat { total += $3 } END { print "Total spent on " cat ": " total }' main.csv
      ;;
    n)
    #   name="$OPTARG"
      awk -F "," -v nm="$OPTARG" '$4 == nm { total += $3 } END { print "Total spent by " nm ": " total }' main.csv
      ;;
    s)
    #   column="$OPTARG"
      sort -t "," -k"$OPTARG" main.csv
      ;;
    h)
      echo "Name of utility: $script_name"
      echo "Usage: sh $script_name [-c category] [-n name] [-s column] record [arguments]"
      echo "Description: A shell script to track expenses by inserting records into a CSV file and performing calculations and sorting based on flags provided."
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      ;;
  esac
done