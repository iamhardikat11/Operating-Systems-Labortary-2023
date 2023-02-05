length=16
if [ $# -eq 1 ];then
    length=$1
fi
if [ $length -lt 4 ];then
    echo "Password_length_should_be_atleast_4"
    exit
fi
cat /dev/urandom|tr -cd '_''A-Z''a-z''0-9'|head -c$length