# Optical tracking velocimetry (OTV)

[Paper](https://www.mdpi.com/2072-4292/10/12/2010/pdf)

[YouTube Video](https://www.youtube.com/watch?v=KM8vZyz7GgU)

## Requirements

This C++ code is developed under Ubuntu 16.04. The following libraries are required:

 - gcc
 - cmake
 - OpenCV (2.4.x)
 
## Building from the command line

Create a build directory and run cmake:

 * mkdir build
 * cd build
 * cmake ..
 * make

## Usage

1) Create arguments [args.txt] 

```shell
bash ./script/calib.sh [path_file] [pixel_dim] [partial_min_angle] [partial_max_angle] [final_min_angle] [final_max_angle] [max_features_to_track] [resolution] [region_step] [detector]
```

where

 * [path_file] : the file will be stored in the specified path
 * [pixel_dim] : real-world dimension of a single pixel (from calibration) 
 * [partial_min_angle] :  trajectory-based partial filtering (min angle value)
 * [partial_max_angle] : trajectory-based partial filtering (max angle value)
 * [final_min_angle] : trajectory-based final filtering (min angle value)
 * [final_max_angle] : trajectory-based final filtering (max angle value)
 * [final_min_distance] : trajectory-based final filtering (min distance in pixel)
 * [max_features_to_track] : maximum number of trackable trajectories
 * [resolution] : Resolution of the input images (Full Resolution [F], Half Resolution [H], Quarter Resolution [Q])
 * [region step] : In order to compute subregions statistics, the image will be divided into N-regions of region_step pixels each
 * [detector] : [SIFT, GFTT, SURF, ORB, FAST]

2) Create Lukas-Kanade arguments [lk.txt]

```shell
bash ./script/LK.sh [path_file] [radius] [maxLevel] [maxCount] [epsilon] [flags] [minEigThreshold]
```

where

 * [path_file] : the file will be stored in the specified path
 * [radius] :  radius of the search window at each pyramid level
 * [maxLevel] : 0-based maximal pyramid level number; if set to 0, pyramids are not used (single level), if set to 1, two levels are used, and so on; if pyramids are passed to input then algorithm will use as many levels as pyramids have but no more than maxLevel.

For criteria parameter: parameter, specifying the termination criteria of the iterative search algorithm (after the specified maximum number of iterations  * [maxCount] or when the search window moves by less than [epsilon])
 * [flags] :

 - OPTFLOW_USE_INITIAL_FLOW uses initial estimations, stored in nextPts; if the flag is not set, then prevPts is copied to nextPts and is considered the initial estimate.
 
 - OPTFLOW_LK_GET_MIN_EIGENVALS use minimum eigen values as an error measure (see minEigThreshold description); if the flag is not set, then L1 distance between patches around the original and a moved point, divided by number of pixels in a window, is used as a error measure.

 * [minEigThreshold] : minEigThreshold – the algorithm calculates the minimum eigen value of a 2x2 normal matrix of optical flow equations (this matrix is called a spatial gradient matrix in [Bouguet00]), divided by number of pixels in a window; if this value is less than minEigThreshold, then a corresponding feature is filtered out and its flow is not processed, so it allows to remove bad points and get a performance boost.

3) Run 

```shell
./build/river_flow_velocity_estimation [path_video] [path_args_file] [path_lk_file] [output_path] [mask_path]
```
where

 * [path_video] : path to the video
 * [path_args_file] : path to the args.txt
 * [path_lk_file] : path to the lk.txt
 * [output_path] : output path where to save results/processed data
 * [mask_path] (optional): path to the binary (0/255) mask image in order to compute trajectories only in the specified regions
