#ifndef SOURCE_H
#define SOURCE_H

#include "data.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const int adas_camera_num;

void init_adas_source(struct Data* data, int type);
void fini_adas_source(struct Data* data, int type);
void dequeue_adas(struct Data* data, int type);
void enqueue_adas(struct Data* data, int type);

#ifdef __cplusplus
}
#endif

#endif
