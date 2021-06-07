#!/bin/bash

srcs=$1
for omit in $2
do
    srcs=$(grep -v $(basename $omit) <(echo $srcs | tr ' ' '\n') )
done

echo $srcs