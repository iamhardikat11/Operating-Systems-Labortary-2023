#!/bin/bash
if echo $line | grep -q $word; then
        echo $line | tr '[a-z]' '[A-Z]' | tr '[A-Z]' '[a-z]'
else
        echo $line

