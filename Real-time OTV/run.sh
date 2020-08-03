#!bin/bash

cd build 
make -j4 #4 core
cd ..

#params
path_file_params=../common/params.txt
pixel_dim=0.004965
partial_min_angle=-135
partial_max_angle=-45
final_min_angle=-110
final_max_angle=-80
final_min_distance=216
region_step=100
resolution=H #F,H,Q
max_features_to_track=15000
detector=FAST 
jump=2

#Lukas-Kanade params
path_file_lk=../common/lk.txt
radius=4
maxLevel=3
maxCount=20
epsilon=0.03
flags=0
minEigThreshold=0.001

#in/out
path_output=./output
path_video=../common/video.m4v
path_mask=../common/mask.png

#creation folder for frames
path_frames=./temp/frames
path_info_video=../common/info_video.txt
mkdir -p $path_frames


bash ./script/utility_video.sh $path_video $path_frames $path_info_video $resolution $jump
bash ./script/lk.sh $path_file_lk $radius $maxLevel $maxCount $epsilon $flags $minEigThreshold
bash ./script/calib.sh $path_file_params $pixel_dim $partial_min_angle $partial_max_angle $final_min_angle $final_max_angle $final_min_distance $max_features_to_track $resolution $region_step $detector
./build/river_flow_velocity_estimation $path_info_video $path_file_params $path_file_lk $path_output $path_mask

#delete frame's directory
rm -r $path_frames




