#include "util.h"

Mat genMat(const Data* dt, int index) {
	struct ReusableMat {
		Mat m;
		uint32_t frames;
	};
	static ReusableMat rm[2] = {
		{ Mat(), dt->frames + 1 },
		{ Mat(), dt->frames + 1 },
	};

	DataFormat df = dt->df;
	int w = dt->cam_width;
	int h = dt->cam_height;
	uint8_t* pixel = (uint8_t*) dt->buffers[index][dt->buffer_index[index]].start;
	Mat& m = rm[index].m;
	uint32_t& f = rm[index].frames;

	switch (df) {
	case DF_YUYV:
		if (f != dt->frames) {
			f = dt->frames;
			m = Mat(h, w, CV_8UC3);
			cvtColor(Mat(h, w, CV_8UC2, pixel), m, CV_YUV2RGB_YVYU);
		}
		return m;
	case DF_BGR:
		if (f != dt->frames) {
			f = dt->frames;
			m = Mat(h, w, CV_8UC3, pixel);
		}
		return m;
	default:
		assert(!"unknown data format!");
		return Mat();
	}
}
