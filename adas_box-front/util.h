#ifndef UTIL_H_
#define UTIL_H_

#include <assert.h>
#include <opencv2/opencv.hpp>
using namespace cv;
#include "data.h"

/**
 * generate opencv mat by dt
 * only one mat generated for the same frame
 * @param dt the Data struct
 * @param index camera index
 * @return generated mat
 */
Mat genMat(const Data* dt, int index);

/**
 * all the dsm statuses
 */

enum DsmStatus{LOOKLEFTRIGHT, PHONE, SLEEP, YAWN};

#endif /* UTIL_H_ */
