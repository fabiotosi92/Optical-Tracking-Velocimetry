#!/bin/bash

VIDEO=$1
LOCATION=$2
FILE=$3
RESOLUTION=$4
JUMP=$5
FILECOUNT=0
QUARTER=Q
HALF=H
FULL=F

#extract fps from video
FPS=$(ffprobe -v error -select_streams v -of default=noprint_wrappers=1:nokey=1 -show_entries stream=r_frame_rate $VIDEO)

#extract the resolution
WIDTH=$(ffmpeg -i $VIDEO 2>&1 | grep Video: | grep -Po '\d{3,5}x\d{3,5}' | cut -d'x' -f1)
HEIGHT=$(ffmpeg -i $VIDEO 2>&1 | grep Video: | grep -Po '\d{3,5}x\d{3,5}' | cut -d'x' -f2)

		echo "--Extract frames from video: $VIDEO"
		if [[ ($RESOLUTION == $QUARTER) ]]; then
			WIDTH=$((WIDTH/4))
			HEIGHT=$((HEIGHT/4))
			time ffmpeg -loglevel panic -i $VIDEO -s "$((WIDTH))x$((HEIGHT))" -qscale:v 2 "$LOCATION/img%d.jpeg"
		elif [[($RESOLUTION == $HALF) ]]; then
			WIDTH=$((WIDTH/2))
			HEIGHT=$((HEIGHT/2))
			time ffmpeg -loglevel panic -i $VIDEO -s "$((WIDTH))x$((HEIGHT))" -qscale:v 2 "$LOCATION/img%d.jpeg"
		elif [[($RESOLUTION == $FULL) ]]; then
			time ffmpeg -loglevel panic -i $VIDEO -qscale:v 2 "$LOCATION/img%d.jpeg"
		fi

#count frames
for item in $LOCATION/*
do
if [ -f "$item" ]
    then
         FILECOUNT=$[$FILECOUNT+1]
fi
done

#delete video
#rm $VIDEO

#printing value 
truncate -s 0 $FILE
echo -e "FPS" $((FPS)) >> $FILE 
echo -e "Width" $WIDTH >> $FILE 
echo -e "Height" $HEIGHT >> $FILE 
echo -e "FileCount" $FILECOUNT >> $FILE 
echo -e "DirectoryFrame" $LOCATION >> $FILE 
echo -e "Jump" $JUMP >> $FILE




