/*
 * MorphologicOperation.h
 *
 *  Created on: Jul 7, 2011
 */

#ifndef MORPHOLOGICOPERATION_H_
#define MORPHOLOGICOPERATION_H_


#ifdef _MSC_VER
#define DllExport __declspec(dllexport)
#else
#define DllExport //nothing 
#endif

#include "opencv/cv.hpp"

#ifdef WITH_CUDA
#include "opencv2/gpu/gpu.hpp"
#endif

using namespace cv;
#ifdef WITH_CUDA
using namespace cv::gpu;
#endif


namespace nscale {

// DOES NOT WORK WITH MULTICHANNEL.
template <typename T>
cv::Mat imreconstruct(const cv::Mat& seeds, const cv::Mat& image, int connectivity);

template <typename T>
std::vector<cv::Mat> imreconstruct3D(const std::vector<cv::Mat>& seeds, const std::vector<cv::Mat>& image, int connectivity);

template <typename T>
std::vector<cv::Mat> imhmax3D(const std::vector<cv::Mat>& input, T h, int connectivity);

template <typename T>
cv::Mat imreconstructParallelTile(const cv::Mat& seeds, const cv::Mat& image, int connectivity, int tileSize, int nThreads = 0);

template <typename T>
Mat imreconstructFixTilingEffects(const Mat& seeds, const Mat& image, int connectivity, int tileIdX, int tileIdY, int tileSize, bool withBorder = false);

template <typename T>
Mat imreconstructFixTilingEffectsParallel(const Mat& seeds, const Mat& image, int connectivity, int tileSize, bool withBorder = false);

template <typename T>
Mat imreconstructParallelQueue(const Mat& seeds, const Mat& image, int connectivity, bool withBorder = false, int nThreads = 0);


//template <typename T>
//cv::Mat imreconstructGeorge(const cv::Mat& seeds, const cv::Mat& image, int connectivity);

// downhill version...
DllExport cv::Mat imreconstructUChar(const cv::Mat& seeds, const cv::Mat& image, int connectivity);

template <typename T>
cv::Mat imreconstructBinary(const cv::Mat& seeds, const cv::Mat& binaryImage, int connectivity);


template <typename T>
cv::Mat imfill(const cv::Mat& image, const cv::Mat& seeds, bool binary, int connectivity);

template <typename T>
cv::Mat imfillHoles(const cv::Mat& image, bool binary, int connectivity);



template <typename T>
cv::Mat bwselect(const cv::Mat& binaryImage, const cv::Mat& seeds, int connectivity);

DllExport cv::Mat_<int> bwlabel(const cv::Mat& binaryImage, bool contourOnly, int connectivity, bool bbox, std::vector<Vec4i> &boundingBoxes);
DllExport cv::Mat_<int> bwlabel2(const cv::Mat& binaryImage, int connectivity, bool relab);

// incorporates a filter for the contours.
template <typename T>
cv::Mat bwlabelFiltered(const cv::Mat& binaryImage, bool binaryOutput,
		bool (*contourFilter)(const std::vector<std::vector<cv::Point> >&, const std::vector<cv::Vec4i>&, int),
		bool contourOnly, int connectivity);

// inclusive min, exclusive max
DllExport bool contourAreaFilter(const std::vector<std::vector<cv::Point> >& contours, const std::vector<cv::Vec4i>& hierarchy, int idx, int minArea, int maxArea);

// inclusive min, exclusive max.
template <typename T>
cv::Mat bwareaopen(const cv::Mat& binaryImage, int minSize, int maxSize, int connectivity, int& count);

DllExport cv::Mat bwareaopen2(const cv::Mat& binaryImage, bool labeled, bool flatten, int minSize, int maxSize, int connectivity, int& count);
DllExport cv::Mat bwareaopen3(const cv::Mat& binaryImage, bool flatten, int minSize, int maxSize, int connectivity, int& count);

template <typename T>
cv::Mat imhmin(const cv::Mat& image, T h, int connectivity);



DllExport cv::Mat_<int> watershed(const cv::Mat& origImage, const cv::Mat_<float>& image, int connectivity);
DllExport cv::Mat_<int> watershed2(const cv::Mat& origImage, const cv::Mat_<float>& image, int connectivity);
DllExport cv::Mat_<int> watershed(const cv::Mat& image, int connectivity);
DllExport cv::Mat_<int> watershedCC(const Mat& image, int connectivity);

template <typename T>
cv::Mat_<unsigned char> localMaxima(const cv::Mat& image, int connectivity);
template <typename T>
cv::Mat_<unsigned char> localMinima(const cv::Mat& image, int connectivity);

template <typename T>
cv::Mat morphOpen(const cv::Mat& image, const cv::Mat& kernel);


DllExport cv::Mat distanceTransform(const cv::Mat& mask, bool calcDist = true);
DllExport cv::Mat distanceTransformParallelTile(const cv::Mat& mask, int tileSize, int nThreads, bool calcDist = true);
DllExport cv::Mat distTransformFixTilingEffects(cv::Mat& nearestNeighbor, int tileSize, bool calcDist = true);

#ifdef WITH_CUDA
namespace gpu {
// GPU versions of the same functions.

// DOES NOT WORK WITH MULTICHANNEL.

template <typename T>
cv::gpu::GpuMat imreconstruct(const cv::gpu::GpuMat& seeds, const cv::gpu::GpuMat& image, int connectivity, cv::gpu::Stream& stream, unsigned int& iter);

template <typename T> 
cv::gpu::GpuMat imreconstructQueue(const cv::gpu::GpuMat& seeds, const cv::gpu::GpuMat& image, int connectivity, cv::gpu::Stream& stream);

template <typename T>
vector<cv::gpu::GpuMat> imreconstructQueueThroughput(vector<GpuMat> & seeds, vector<GpuMat> & image, int connectivity, int nItFirstPass, Stream& stream);

template <typename T>
cv::gpu::GpuMat imreconstructQueueSpeedup(GpuMat& seeds, GpuMat& image, int connectivity, int nItFirstPass, Stream& stream, int nBlocks = 14, bool binary = false);


DllExport cv::gpu::GpuMat imreconstructQueueSpeedupFloat(GpuMat &seeds, GpuMat &image, int connectivity, int nItFirstPass, Stream& stream, int nBlocks = 14);


template <typename T>
cv::gpu::GpuMat imreconstruct(const cv::gpu::GpuMat& seeds, const cv::gpu::GpuMat& image, int connectivity, cv::gpu::Stream& stream) {
	unsigned int iter;
	return imreconstruct<T>(seeds, image, connectivity, stream, iter);
};

template <typename T>
cv::gpu::GpuMat imreconstructBinary(const cv::gpu::GpuMat& seeds, const cv::gpu::GpuMat& binaryImage, int connectivity, cv::gpu::Stream& stream, unsigned int& iter);

template <typename T>
cv::gpu::GpuMat imreconstructBinary(const cv::gpu::GpuMat& seeds, const cv::gpu::GpuMat& binaryImage, int connectivity, cv::gpu::Stream& stream){
	unsigned int iter;
	return imreconstructBinary<T>(seeds, binaryImage,connectivity, stream, iter);
};

template <typename T>
cv::gpu::GpuMat imreconstructQ(const cv::gpu::GpuMat& seeds, const cv::gpu::GpuMat& image, int connectivity, cv::gpu::Stream& stream, unsigned int& iter);

template <typename T>
cv::gpu::GpuMat imreconstructQ(const cv::gpu::GpuMat& seeds, const cv::gpu::GpuMat& image, int connectivity, cv::gpu::Stream& stream) {
	unsigned int iter;
	return imreconstructQ<T>(seeds, image, connectivity, stream, iter);
};


//template <typename T>
//cv::Mat imfill(const cv::Mat& image, const cv::Mat& seeds, bool binary, int connectivity);
//
template <typename T>
cv::gpu::GpuMat imfillHoles(const cv::gpu::GpuMat& image, bool binary, int connectivity, cv::gpu::Stream& stream);



template <typename T>
cv::gpu::GpuMat bwselect(const cv::gpu::GpuMat& binaryImage, const cv::gpu::GpuMat& seeds, int connectivity, cv::gpu::Stream& stream);

DllExport cv::gpu::GpuMat bwlabel(const cv::gpu::GpuMat& binaryImage, int connectivity, bool relab, cv::gpu::Stream& stream);
// incorporates a filter for the contours.
//template <typename T>
//cv::Mat bwlabelFiltered(const cv::Mat& binaryImage, bool binaryOutput,
//		bool (*contourFilter)(const std::vector<std::vector<cv::Point> >&, const std::vector<cv::Vec4i>&, int),
//		bool contourOnly, int connectivity);
//
//// inclusive min, exclusive max
//bool contourAreaFilter(const std::vector<std::vector<cv::Point> >& contours, const std::vector<cv::Vec4i>& hierarchy, int idx, int minArea, int maxArea);
//// inclusive min, exclusive max
//bool contourAreaFilter2(const std::vector<std::vector<cv::Point> >& contours, const std::vector<cv::Vec4i>& hierarchy, int idx, int minArea, int maxArea);
//
// inclusive min, exclusive max.
DllExport cv::gpu::GpuMat bwareaopen(const cv::gpu::GpuMat& binaryImage, bool labeled, bool flatten, int minSize, int maxSize, int connectivity, int& count, Stream& stream);
DllExport cv::gpu::GpuMat bwareaopen2(const cv::gpu::GpuMat& binaryImage, bool labeled, bool flatten, int minSize, int maxSize, int connectivity, int& count, Stream& stream);
//
template <typename T>
cv::gpu::GpuMat imhmin(const cv::gpu::GpuMat& image, T h, int connectivity, cv::gpu::Stream& stream);
//
//cv::gpu::GpuMat watershedCA(const cv::gpu::GpuMat& origImage, const cv::gpu::GpuMat& image, int connectivity, cv::gpu::Stream& stream);
DllExport cv::gpu::GpuMat watershedDW(const cv::gpu::GpuMat& maskImage, const cv::gpu::GpuMat& image, int background, int connectivity, cv::gpu::Stream& stream);
//
//template <typename T>
//cv::Mat_<unsigned char> localMaxima(const cv::Mat& image, int connectivity);
//template <typename T>
//cv::Mat_<unsigned char> localMinima(const cv::Mat& image, int connectivity);


template <typename T>
cv::gpu::GpuMat morphOpen(const cv::gpu::GpuMat& image, const cv::Mat& kernel, cv::gpu::Stream& stream);
template <typename T>
cv::gpu::GpuMat morphErode(const cv::gpu::GpuMat& image, const cv::Mat& kernel, cv::gpu::Stream& stream);
template <typename T>
cv::gpu::GpuMat morphDilate(const cv::gpu::GpuMat& image, const cv::Mat& kernel, cv::gpu::Stream& stream);

DllExport cv::gpu::GpuMat distanceTransform(const cv::gpu::GpuMat& mask, cv::gpu::Stream& stream, bool calcDist = true, int tIdX = 0, int tIdY = 0, int tileSize = 0, int imgCols = 0);

}
#endif


}


#endif /* MORPHOLOGICOPERATION_H_ */
