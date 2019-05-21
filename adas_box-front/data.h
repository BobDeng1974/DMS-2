#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>
#include <opencv2/opencv.hpp>

enum DataFormat {
	DF_UNKNOWN,
	DF_YUYV,
	DF_BGR,
};

struct Buffer {
        void *start;
        size_t length;
};

struct Data {
	int cam_width;
	int cam_height;
	uint32_t benchmark_time, frames;
        struct Buffer buffers[2][3];
        int buffer_index[2];
	int fd[2];
	DataFormat df;
	int c_speed;
	float fcw_distance;
	bool enable_ldw;
	bool enable_fcw;
	bool enable_dsm;
	int fcwCameraIndex;
	int dsmCameraIndex;
	cv::Mat fcwMat;
	cv::Mat dsmMat;
};
