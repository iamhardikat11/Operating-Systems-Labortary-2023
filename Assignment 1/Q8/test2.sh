script_name="Assgn1_8_11.sh"
if [ ! -f "main.csv" ]; then
  echo "Date (dd-mm-yy),Category,Amount,Name" >> main.csv
fi

# Check for flags and perform corresponding actions
while getopts ":c:n:s:h" opt; do
  case $opt in
    c)
    category="$OPTARG"
      awk -F "," -v cat="$category" '$2 == cat { total += $3 } END { print "Total spent on " cat ": " total }' main.csv
      ;;
    n)
      name="$OPTARG"
      awk -F "," -v nm="$name" '$4 == nm { total += $3 } END { print "Total spent by " nm ": " total }' main.csv
      ;;
    s)
      column="$OPTARG"
      sort -t "," -k"$column" main.csv
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
shift $((OPTIND-1))
echo "$1,$2,$3,$4" >> main.csv
sort -t, -k1,1n -o main.csv main.csv