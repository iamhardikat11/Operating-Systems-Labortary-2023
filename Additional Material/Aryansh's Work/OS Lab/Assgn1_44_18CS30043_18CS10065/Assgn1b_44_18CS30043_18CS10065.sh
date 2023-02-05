dir2=./$1
if [ ! -d $dir2 ];then
  echo "Directory_doesn't_exist"
  exit
fi
mkdir 1.b.files.out
for file in $dir2/*;do
    filename=$(basename $file)
    sort -nr $file >./1.b.files.out/$filename;done
cat ./1.b.files.out/*|sort -nr >1.b.out.txt
