#!/bin/bash

file=$1

echo -e "radius" $2 > $file
echo -e "maxLevel" $3 >> $file
echo -e "maxCount" $4 >> $file
echo -e "epsilon" $5 >> $file
echo -e "flags" $6 >> $file
echo -e "minEigThreshold" $7 >> $file
