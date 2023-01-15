#! /bin/bash
words=0
chars=0
while read -r l
do 
    echo "$l"
    # Couting Words
    word=$(echo -n "$l" | wc -w)
    let "words+=word"
    # Counting characters
    char=$(echo -n "$l" | wc -c)
    let "chars+=char"
done < "$1"


# Output
echo "Number of Words = $words"
echo "Number of Characters = $chars"
