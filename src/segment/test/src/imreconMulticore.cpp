/*
 * test.cpp
 *
 *  Created on: Jun 28, 2011
 *      Author: tcpan
 */
#include "opencv2/opencv.hpp"
#include <iostream>
#include <dirent.h>
#include <vector>
#include <errno.h>
#include <time.h>
#include "MorphologicOperations.h"
#include "utils.h"
#include <stdio.h>
#include <omp.h>

#include "opencv2/gpu/gpu.hpp"



using namespace cv;
using namespace cv::gpu;
using namespace std;

int main (int argc, char **argv){
	if(argc != 3){
		std::cout << "Usage: ./imreconMulticore <marker-img> <mask-img>" <<std::endl;
		exit(1);
	}
	Mat marker = imread(argv[1], -1);
	Mat mask = imread(argv[2], -1);

	Mat recon1 = nscale::imreconstruct<unsigned char>(marker, mask, 8);
//	imwrite("out-recon8.ppm", recon1);

	Mat marker_border(marker.size() + Size(2,2), marker.type());
	copyMakeBorder(marker, marker_border, 1, 1, 1, 1, BORDER_CONSTANT, 0);
	Mat mask_border(mask.size() + Size(2,2), mask.type());
	copyMakeBorder(mask, mask_border, 1, 1, 1, 1, BORDER_CONSTANT, 0);

	Mat marker_copy(marker_border, Rect(1,1,marker_border.cols-2,marker_border.rows-2));

	Mat mask_copy(mask_border, Rect(1,1,mask_border.cols-2,mask_border.rows-2));
//	marker.copyTo(marker_copy);
//	mask.copyTo(mask_copy);

	// Warmup
//	Mat roiMarker(marker_copy, Rect(0,0, 10,10 ));
//	GpuMat mat(roiMarker);
//	mat.release();

//#pragma omp parallel for
//	for(int i = 0; i < 10; i++){
//		printf("hi");
//	}
	Mat recon, recon2;
	uint64_t t1, t2;

	int tileWidth=1024;
	int tileHeight=1024;
	int nTilesX=marker.cols/tileWidth;
	int nTilesY=marker.rows/tileHeight;
	
	uint64_t t1_tiled = cciutils::ClockGetTime();
	omp_set_num_threads(8);

//#pragma omp parallel for
	for(int tileY=0; tileY < nTilesY; tileY++){
//#pragma omp parallel for
		for(int tileX=0; tileX < nTilesX; tileX++){
//			std::cout <<"Rect("<< tileX*tileWidth << "," << tileY*tileHeight <<","<< tileWidth <<","<< tileHeight<< ");"<<std::endl;
			if(tileX==0 && tileY==0){

				std::cout << "NumberThreads="<< omp_get_max_threads()<<std::endl;
			}

			Mat roiMarker(marker_copy, Rect(tileX*tileWidth, tileY*tileHeight , tileWidth, tileHeight ));
			Mat roiMask(mask_copy, Rect(tileX*tileWidth, tileY*tileHeight , tileWidth, tileHeight));
		
//			Stream stream;
//			GpuMat g_marker, g_mask, g_recon;
//			stream.enqueueUpload(roiMarker, g_marker);
//			stream.enqueueUpload(roiMask, g_mask);
//			stream.waitForCompletion();
//
			t1 = cciutils::ClockGetTime();
//
//			g_recon = nscale::gpu::imreconstructQueueSpeedup<unsigned char>(g_marker, g_mask, 8, 1,stream);
//			std::cout << "Done up to here" <<std::endl;
//			stream.waitForCompletion();
//			Mat reconTile;
//			g_recon.download(reconTile);
//			stream.waitForCompletion();
			Mat reconTile = nscale::imreconstruct<unsigned char>(roiMarker, roiMask, 8);
			reconTile.copyTo(roiMarker);
			t2 = cciutils::ClockGetTime();

			std::cout << " Tile took " << t2-t1 << "ms" << std::endl;
		}
	}
	uint64_t t2_tiled = cciutils::ClockGetTime();
	std::cout << " Tile total took " << t2_tiled-t1_tiled << "ms" << std::endl;

	t1 = cciutils::ClockGetTime();
//	Mat reconCopy = nscale::imreconstructFixTilingEffects<unsigned char>(marker_copy, mask, 8, 0, 0, tileWidth, true);

	Mat reconCopy = nscale::imreconstructFixTilingEffects<unsigned char>(marker_border, mask_border, 8, 0, 0, tileWidth, true);
	t2 = cciutils::ClockGetTime();
	std::cout << "fix tiling recon8 took " << t2-t1 << "ms" << std::endl;



//	imwrite("out-recon8tile.ppm", marker_copy);
//
//	Mat reconOpenMP = nscale::imreconstructOpenMP<unsigned char>(marker, mask, 8, 512);
//
	Mat comp = recon1 != reconCopy;
//	Mat openMpDiff = recon != reconOpenMP;
//
	std::cout << "comp reconCopy= "<<countNonZero(comp) << std::endl;
//	std::cout << "comp openmp= "<<countNonZero(openMpDiff) << std::endl;
//	
//	imwrite("diff.ppm", openMpDiff);
	return 0;
}

