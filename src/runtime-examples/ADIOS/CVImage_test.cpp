/*
 * CVImagetest.cpp
 *
 *  Created on: Jul 9, 2012
 *      Author: tcpan
 */

#include "CVImage.h"
#include <string>
#include <vector>
#include "FileUtils.h"

// from http://answers.opencv.org/question/213/comparing-two-images-whether-same-or-not/
bool cvMatEQ(const cv::Mat& data1, const cv::Mat& data2)
{
    bool success = true;
    // check if is multi dimensional
    if(data1.dims > 2 || data2.dims > 2)
    {
      if( data1.dims != data2.dims || data1.type() != data2.type() )
      {
        return false;
      }
      for(int dim = 0; dim < data1.dims; dim++){
        if(data1.size[dim] != data2.size[dim]){
          return false;
        }
      }
    }
    else
    {
      if(data1.size() != data2.size() || data1.channels() != data2.channels() || data1.type() != data2.type()){
        return false;
      }
    }
    int nrOfElements = data1.total()*data1.elemSize1();
    //bytewise comparison of data
    int cnt = 0;
    for(cnt = 0; cnt < nrOfElements && success; cnt++)
    {
      if(data1.data[cnt] != data2.data[cnt]){
        success = false;
      }
    }
    return success;
  }


int main (int argc, char **argv){

	int retcode = 0;

	char const *input = "/home/tcpan/PhD/path/Data/ValidationSet/20X_4096x4096_tiles/astroII.1/astroII.1.ndpi-0000028672-0000012288.tif";

	std::string fn = std::string(input);
	//cci::common::Debug::print("%s READING %s\n", getClassName(), fn.c_str());


	// parse the input std::string
	std::string filename = cci::common::FileUtils::getFile(const_cast<std::string&>(fn));
	// get the image name
	size_t pos = filename.rfind('.');
	if (pos == std::string::npos) printf("ERROR:  file %s does not have extension\n", fn.c_str());
	std::string prefix = filename.substr(0, pos);
	pos = prefix.rfind("-");
	if (pos == std::string::npos) printf("ERROR:  file %s does not have a properly formed x, y coords\n", fn.c_str());
	std::string ystr = prefix.substr(pos + 1);
	prefix = prefix.substr(0, pos);
	pos = prefix.rfind("-");
	if (pos == std::string::npos) printf("ERROR:  file %s does not have a properly formed x, y coords\n", fn.c_str());
	std::string xstr = prefix.substr(pos + 1);

	std::string imagename = prefix.substr(0, pos);
	int tilex = atoi(xstr.c_str());
	int tiley = atoi(ystr.c_str());


	cv::Mat im = cv::imread(fn, -1);

	cci::rt::adios::CVImage *src, *dest1, *dest2, *dest3;
	int data_pos = 0, mx_image_bytes = im.rows * im.cols * im.elemSize();
	int imageNames_pos = 0, mx_imagename_bytes = 256;
	int filenames_pos = 0, mx_filename_bytes = 1024;
	int data_size, img_name_size, src_fn_size;
	int iterations = 1;

	printf("max image bytes = %d\n", mx_image_bytes);

	const unsigned char* tile;
	const char* imageName;
	const char* sourceTileFile;

	// allocate
	unsigned char* tileB = (unsigned char*) malloc(mx_image_bytes * iterations);
	char* imageNameB = (char*)malloc(mx_imagename_bytes * iterations);
	char* sourceTileFileB = (char*)malloc(mx_filename_bytes * iterations);


	int dummy, dummy2;
	src = new cci::rt::adios::CVImage(im, imagename, fn, tilex, tiley);
	printf("orig image name %s, filename %s, data size %d\n", src->getImageName(dummy, dummy2), src->getSourceFileName(dummy, dummy2), src->getMetadata().info.data_size);

	cv::Mat im2 = src->getImage();
	printf("regenerated opencv image is same as original? %s, expect true\n", (cvMatEQ(im, im2) ? "true" : "false"));

	dest1 = new cci::rt::adios::CVImage();
	dest1->copy(*src);
	//	src3->deserialize(output_size, output);
	printf("copy orig to empty %s, filename %s, data size %d\n", dest1->getImageName(dummy, dummy2), dest1->getSourceFileName(dummy, dummy2), dest1->getMetadata().info.data_size);

	im2 = dest1->getImage();
	printf("regenerated copied opencv image is same as original? %s, expect true\n", (cvMatEQ(im, im2) ? "true" : "false"));

	cci::rt::adios::CVImage::MetadataType *meta = cci::rt::adios::CVImage::allocMetadata();

	dest2 = new ::cci::rt::adios::CVImage(meta,
					tileB + data_pos, mx_image_bytes,
					imageNameB + imageNames_pos, mx_imagename_bytes,
					sourceTileFileB + filenames_pos, mx_filename_bytes);

	dest2->copy(*src);
	printf("copy orig to preallocated buffer %s, filename %s, data size %d\n", dest2->getImageName(dummy, dummy2), dest2->getSourceFileName(dummy, dummy2), dest2->getMetadata().info.data_size);

	im2 = dest2->getImage();
	printf("regenerated prealloc copied opencv image is same as original? %s, expect true\n", (cvMatEQ(im, im2) ? "true" : "false"));


	int output_size = 0;
	void *output = NULL;

	src->serialize(output_size, output);

	dest3 = new cci::rt::adios::CVImage(output_size, output);
	printf("deserialized image name %s, filename %s, data size %d\n", dest3->getImageName(dummy, dummy2), dest3->getSourceFileName(dummy, dummy2), dest3->getMetadata().info.data_size);

	im2 = dest3->getImage();
	printf("regenerated copied serialized opencv image is same as original? %s, expect true\n", (cvMatEQ(im, im2) ? "true" : "false"));

	int output_size2 = 0;
	void *output2 = NULL;
	src->serialize(output_size2, output2, cci::rt::adios::CVImage::ENCODE_Z);

	::cci::rt::adios::CVImage *destz = new cci::rt::adios::CVImage(output_size2, output2, cci::rt::adios::CVImage::ENCODE_Z);
	printf("deserialized COMPRESSED image name %s, filename %s, data size %d\n", destz->getImageName(dummy, dummy2), destz->getSourceFileName(dummy, dummy2), destz->getMetadata().info.data_size);

	cv::Mat imz = destz->getImage();
	printf("regenerated copied serialized COMPRESSED opencv image is same as original? %s, expect true\n", (cvMatEQ(im, imz) ? "true" : "false"));



	dest3->copy(*src);
	printf("copy orig to serialized %s, filename %s, data size %d.  expect fail\n", dest3->getImageName(dummy, dummy2), dest3->getSourceFileName(dummy, dummy2), dest3->getMetadata().info.data_size);



	////////////////

	src->copy(*dest3);
	printf("copy serialized to orig %s, filename %s, data size %d. expect fail\n", src->getImageName(dummy, dummy2), src->getSourceFileName(dummy, dummy2), src->getMetadata().info.data_size);

	delete dest1;
	dest1 = new cci::rt::adios::CVImage();

	dest1->copy(*dest3);
	//	src3->deserialize(output_size, output);
	printf("copy serialized to empty %s, filename %s, data size %d\n", dest1->getImageName(dummy, dummy2), dest1->getSourceFileName(dummy, dummy2), dest1->getMetadata().info.data_size);

	cv::Mat im3 = dest3->getImage();
	printf("regenerated copied serialized opencv image is same as original? %s, expect true\n", (cvMatEQ(im2, im3) ? "true" : "false"));


	delete dest2;
	dest2 =
			new ::cci::rt::adios::CVImage(meta,
					tileB + data_pos, mx_image_bytes,
					imageNameB + imageNames_pos, mx_imagename_bytes,
					sourceTileFileB + filenames_pos, mx_filename_bytes);

	dest2->copy(*dest3);
	printf("copy serialized to preallocated buffer %s, filename %s, data size %d\n", dest2->getImageName(dummy, dummy2), dest2->getSourceFileName(dummy, dummy2), dest2->getMetadata().info.data_size);

	im3 = dest3->getImage();
	printf("regenerated copied serialized opencv image is same as original? %s, expect true\n", (cvMatEQ(im2, im3) ? "true" : "false"));

	///////////////

	delete dest1;
	dest1 = new cci::rt::adios::CVImage();

	src->copy(*dest1);
	printf("copy empty to orig %s, filename %s, data size %d.  expect fail\n", src->getImageName(dummy, dummy2), src->getSourceFileName(dummy, dummy2), src->getMetadata().info.data_size);

	delete dest2;
	dest2 =
			new ::cci::rt::adios::CVImage(meta,
					tileB + data_pos, mx_image_bytes,
					imageNameB + imageNames_pos, mx_imagename_bytes,
					sourceTileFileB + filenames_pos, mx_filename_bytes);
	dest2->copy(*dest1);
	printf("copy empty to preallocated buffer %s, filename %s, data size %d\n", dest2->getImageName(dummy, dummy2), dest2->getSourceFileName(dummy, dummy2), dest2->getMetadata().info.data_size);

	dest3->copy(*dest1);
	printf("copy empty to serialized %s, filename %s, data size %d.  expect fail\n", dest3->getImageName(dummy, dummy2), dest3->getSourceFileName(dummy, dummy2), dest3->getMetadata().info.data_size);


	//////////////////

	delete dest2;
	dest2 =
			new ::cci::rt::adios::CVImage(meta,
					tileB + data_pos, mx_image_bytes,
					imageNameB + imageNames_pos, mx_imagename_bytes,
					sourceTileFileB + filenames_pos, mx_filename_bytes);

	src->copy(*dest2);
	printf("copy preallocated to orig %s, filename %s, data size %d.  expect fail \n", src->getImageName(dummy, dummy2), src->getSourceFileName(dummy, dummy2), src->getMetadata().info.data_size);

	delete dest1;
	dest1 = new cci::rt::adios::CVImage();
	dest1->copy(*dest2);
	//	src3->deserialize(output_size, output);
	printf("copy preallocated to empty %s, filename %s, data size %d\n", dest1->getImageName(dummy, dummy2), dest1->getSourceFileName(dummy, dummy2), dest1->getMetadata().info.data_size);

	dest3->copy(*dest2);
	printf("copy prealloc to serialized %s, filename %s, data size %d.  expect fail\n", dest3->getImageName(dummy, dummy2), dest3->getSourceFileName(dummy, dummy2), dest3->getMetadata().info.data_size);



	free(tileB);
	free(imageNameB);
	free(sourceTileFileB);
	delete dest1;
	delete dest2;
	delete dest3;


	// test copying to preallocated buffer in iterations

	iterations = 5;

	// allocate
	tileB = (unsigned char*) malloc(mx_image_bytes * iterations);
	imageNameB = (char*)malloc(mx_imagename_bytes * iterations);
	sourceTileFileB = (char*)malloc(mx_filename_bytes * iterations);

	std::vector<cci::rt::adios::CVImage *> outs;

	for (int i = 0; i < iterations -1 ; ++i) {

		tile = src->getData(dummy, data_size);
		imageName = src->getImageName(dummy, img_name_size);
		sourceTileFile = src->getSourceFileName(dummy, src_fn_size);
		printf("input: data at %p, size %d, imname at %p, size %d, fn at %p, size %d\n",
				tile, data_size, imageName, img_name_size,
				sourceTileFile, src_fn_size);



		::cci::rt::adios::CVImage *out =
				new ::cci::rt::adios::CVImage(meta,
						tileB + data_pos, mx_image_bytes,
						imageNameB + imageNames_pos, mx_imagename_bytes,
						sourceTileFileB + filenames_pos, mx_filename_bytes);

		outs.push_back(out);

		tile = out->getData(dummy, data_size);
		imageName = out->getImageName(dummy, img_name_size);
		sourceTileFile = out->getSourceFileName(dummy, src_fn_size);

		printf("output allocated: data at %p, size %d, imname at %p, size %d, fn at %p, size %d\n",
				tileB + data_pos, data_size, imageNameB + imageNames_pos, img_name_size,
				sourceTileFileB + filenames_pos, src_fn_size);


		out->copy(*src);

		tile = out->getData(dummy, data_size);
		imageName = out->getImageName(dummy, img_name_size);
		sourceTileFile = out->getSourceFileName(dummy, src_fn_size);

		printf("output copied: data at %p, size %d, imname at %p, size %d, fn at %p, size %d\n",
				tileB + data_pos, data_size, imageNameB + imageNames_pos, img_name_size,
				sourceTileFileB + filenames_pos, src_fn_size);


		out->compact();
		int mx_in_size;
		int mx_sfn_size;

		tile = out->getData(dummy, data_size);
		imageName = out->getImageName(mx_in_size, img_name_size);
		sourceTileFile = out->getSourceFileName(mx_sfn_size, src_fn_size);

		printf("output compacted: data at %p, size %d, imname at %p, size %d, fn at %p, size %d\n",
		tileB + data_pos, data_size, imageNameB + imageNames_pos, img_name_size,
		sourceTileFileB + filenames_pos, src_fn_size);

		data_pos += data_size;
		imageNames_pos += mx_in_size;
		filenames_pos += mx_sfn_size;

	}

	for (std::vector<cci::rt::adios::CVImage *>::iterator it = outs.begin();
			it != outs.end(); ++it) {
		delete *it;
	}
	outs.clear();


	free(tileB);
	free(imageNameB);
	free(sourceTileFileB);
	// clean up
	delete src;
	cci::rt::adios::CVImage::freeMetadata(meta);
	cci::rt::adios::CVImage::freeSerializedData(output);
	im.release();

	return 0;


}
