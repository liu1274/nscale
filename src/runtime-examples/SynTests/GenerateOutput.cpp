/*
 * GenerateOutput.cpp
 *
 *  Created on: Jun 18, 2012
 *      Author: tcpan
 */

#include "GenerateOutput.h"
#include "Debug.h"
#include "opencv2/opencv.hpp"
#include "CVImage.h"
#include "FileUtils.h"
#include <string>
#include "TypeUtils.h"
#include "SCIOHistologicalEntities.h"
#include <unistd.h>
#include "CmdlineParser.h"

namespace cci {
namespace rt {
namespace syntest {

bool GenerateOutput::initParams() {

	params.add_options()
		("compute_time_min,d", boost::program_options::value< double >(), "min compute time.  default 1")
		("compute_time_max,D", boost::program_options::value< double >(), "max compute time.  default 1")
			;
	return true;
}

boost::program_options::options_description GenerateOutput::params("Compute Options");
bool GenerateOutput::param_init = GenerateOutput::initParams();


GenerateOutput::GenerateOutput(MPI_Comm const * _parent_comm, int const _gid,
		DataBuffer *_input, DataBuffer *_output,
		boost::program_options::variables_map &_vm,
		cci::common::LogSession *_logsession) :
				Action_I(_parent_comm, _gid, _input, _output, _logsession), output_count(0), min(0), duration(0) {
	output_dim = cci::rt::CmdlineParser::getParamValueByName<int>(_vm, cci::rt::CmdlineParser::PARAM_MAXIMGSIZE);
	compress = cci::rt::CmdlineParser::getParamValueByName<bool>(_vm, cci::rt::DataBuffer::PARAM_COMPRESSION);

	if (_vm.count("compute_time_min")) {
		min = cci::rt::CmdlineParser::getParamValueByName< double >(_vm, "compute_time_min");
	}
	if (_vm.count("compute_time_max")) {
		duration = cci::rt::CmdlineParser::getParamValueByName< double >(_vm, "compute_time_max") - min;
	}

	if (min < 0) min = 0;
	if (duration < 0) duration = 0;
}

GenerateOutput::~GenerateOutput() {
	//cci::common::Debug::print("%s destructor called.  %d generated\n", getClassName(), output_count[6~);
}

int GenerateOutput::compute(int const &input_size , void * const &input,
			int &output_size, void * &output) {
	if (input_size == 0 || input == NULL) return -1;

	long long t1, t2;

	t1 = ::cci::common::event::timestampInUS();

	std::string fn = std::string((char const *)input);
	//cci::common::Debug::print("%s READING %s\n", getClassName(), fn.c_str());


	// parse the input string
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

	//cv::Mat im = cv::imread(fn, -1);
	//cv::Mat im = cv::Mat::zeros(4096, 4096, CV_8UC4);
	// simulate computation
	//sleep(rand() % 3 + 1);
//	t2 = ::cci::common::event::timestampInUS();
	char len[21];  // max length of uint64 is 20 digits
//	memset(len, 0, 21);
//	sprintf(len, "%lu", (long)(im.dataend) - (long)(im.datastart));
//	if (logsession != NULL) logsession->log(cci::common::event(0, std::string("read"), t1, t2, std::string(len), ::cci::common::event::FILE_I));
//
//
//	if (!im.data) {
//		im.release();
//		return -1;
//	}
//
//	t1 = ::cci::common::event::timestampInUS();

	// real computation:
	int status = ::nscale::SCIOHistologicalEntities::SUCCESS;
	cv::Mat mask = cv::Mat::zeros(output_dim, output_dim, CV_32SC1);

	int slept = (int)((rand() % 1000000) * duration + min * 1000000);
	//printf("sleeping for %d\n", slept);
	usleep(slept);

	t2 = ::cci::common::event::timestampInUS();
	if (logsession != NULL) logsession->log(cci::common::event(90, std::string("compute"), t1, t2, std::string("1"), ::cci::common::event::COMPUTE));

		t1 = ::cci::common::event::timestampInUS();
		cci::rt::adios::CVImage *img = new cci::rt::adios::CVImage(mask, imagename, fn, tilex, tiley);
//		CVImage *img = new CVImage(im, imagename, fn, tilex, tiley);
		if (compress) img->serialize(output_size, output, cci::rt::adios::CVImage::ENCODE_Z);
		else img->serialize(output_size, output);
		// clean up
		delete img;

		t2 = ::cci::common::event::timestampInUS();
		memset(len, 0, 21);
		sprintf(len, "%lu", (long)output_size);
		if (logsession != NULL) logsession->log(cci::common::event(90, std::string("serialize"), t1, t2, std::string(len), ::cci::common::event::MEM_IO));

//	im.release();

	mask.release();
	return status;
}

int GenerateOutput::run() {

	if (this->inputBuf->isFinished()) {
		//cci::common::Debug::print("%s input DONE.  input = %d, output = %d\n", getClassName(), call_count, output_count);
		this->outputBuf->stop();

		return Communicator_I::DONE;
	} else if (this->outputBuf->isStopped()) {
		//cci::common::Debug::print("%s output DONE.  input = %d, output = %d\n", getClassName(), call_count, output_count);
		this->inputBuf->stop();

		if (!this->inputBuf->isFinished()) cci::common::Debug::print("WARNING: %s input buffer is not empty.\n", getClassName());
		return Communicator_I::DONE;
	} else if (!this->inputBuf->canPop() || !this->outputBuf->canPush()) {
		return Communicator_I::WAIT;
	}

	DataBuffer::DataType data;
	int output_size, input_size;
	void *output = NULL, *input = NULL;


	int bstat = this->inputBuf->pop(data);
	if (bstat == DataBuffer::EMPTY) {
		return Communicator_I::WAIT;
	}
	input_size = data.first;
	input = data.second;

//		cci::common::Debug::print("%s READY and getting input:  call count= %d\n", getClassName(), call_count);

	int result = compute(input_size, input, output_size, output);
	call_count++;


	if (result == ::nscale::SCIOHistologicalEntities::SUCCESS) {
//			cci::common::Debug::print("%s bufferring output:  call count= %d\n", getClassName(), call_count);
		++output_count;
		bstat = this->outputBuf->push(std::make_pair(output_size, output));

		if (bstat == DataBuffer::STOP) {
			cci::common::Debug::print("ERROR: %s can't push into buffer.  status STOP.  Should have caught this earlier. \n", getClassName());
			this->inputBuf->push(data);
			this->inputBuf->stop();
			free(output);
			return Communicator_I::DONE;
		} else if (bstat == DataBuffer::FULL) {
			cci::common::Debug::print("WARNING: %s can't push into buffer.  status FULL.  Should have caught this earlier.\n", getClassName());
			this->inputBuf->push(data);
			free(output);
			return Communicator_I::WAIT;
		} else {
			if (input != NULL) {
				free(input);
				input = NULL;
			}
			return Communicator_I::READY;
		}
	} else {
		if (input != NULL) {
			free(input);
			input = NULL;
		}
		return Communicator_I::READY;
	}



}

}
} /* namespace rt */
} /* namespace cci */
