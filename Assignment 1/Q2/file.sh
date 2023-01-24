if cmp --silent -- "$1" "$2"; then
  echo "files contents are identical"
else
  echo "files differ"
fi
