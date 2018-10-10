#!/usr/bin/env bash

echo "usage:"
echo "put [name] [surname] [phone] [email] [age] -- (other arguments will be ignored)"
echo "get_all"
echo "get [surname]"
echo "del [surname]"

DONE=false
until $DONE ;do
    read || DONE=true
    echo "$REPLY" > /dev/lkm_example
    cat /dev/lkm_example 
done
