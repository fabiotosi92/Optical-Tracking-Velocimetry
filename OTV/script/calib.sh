#!/bin/bash

file=$1

echo -e "pixel_to_real" $2 > $file
echo -e "partial_min_angle" $3 >> $file
echo -e "partial_max_angle" $4 >> $file
echo -e "final_min_angle" $5 >> $file
echo -e "final_max_angle" $6 >> $file
echo -e "final_min_distance" $7 >> $file
echo -e "max_features" $8 >> $file
echo -e "resolution" ${9} >> $file
echo -e "region_step" ${10} >> $file
echo -e "detector" ${11} >> $file
