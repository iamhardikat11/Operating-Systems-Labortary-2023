#!/bin/bash

folder=$1
for file in $(find $folder -name "*.py"); do
    echo $file
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
    /^"""/ {
        if(in_multiline_comment) {
            printf("Line %d-%d: %s\n", start_line, NR, multiline_comment);
            in_multiline_comment = 0;
            multiline_comment = "";
        } else {
            in_multiline_comment = 1;
            start_line = NR;
            multiline_comment = $0 "\n";
        }
    }
    ' $file
done
