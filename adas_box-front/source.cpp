#include "source.h"

extern void init_adas_capture_source(struct Data *Data);
extern void fini_adas_capture_source(struct Data *Data);
extern void dequeue_adas_capture(struct Data *Data);
extern void enqueue_adas_capture(struct Data *Data);


extern int init_adas_video_source(struct Data *Data);
extern int dequeue_adas_video(struct Data *Data);
extern void fini_adas_video_source(struct Data *Data);

extern void init_adas_opencv_capture_source(struct Data *data);
extern void fini_adas_opencv_capture_source(struct Data *data);
extern void dequeue_adas_opencv_capture(struct Data *data);
extern void enqueue_adas_opencv_capture(struct Data *data);

extern void init_adas_opencv_video_source(struct Data *data);
extern void fini_adas_opencv_video_source(struct Data *data);
extern void dequeue_adas_opencv_video(struct Data *data);
extern void enqueue_adas_opencv_video(struct Data *data);

const int adas_camera_num = 2;

void init_adas_source(struct Data *Data, int type)
{
        switch(type)
        {
                //capture source
                case 0:
                        init_adas_capture_source(Data);
                        break;
                case 1:
                        init_adas_video_source(Data);
                        break;
		case 10:
			init_adas_opencv_capture_source(Data);
			break;
		case 11:
                        init_adas_opencv_video_source(Data);
                        break;
                default:
                        break;
        }
}

void fini_adas_source(struct Data *Data, int type)
{
        switch(type)
        {   
                //capture source
                case 0:
                        fini_adas_capture_source(Data);
                        break;
                case 1:
                        fini_adas_video_source(Data);
                        break;
		case 10:
			fini_adas_opencv_capture_source(Data);
			break;
		case 11:
                        fini_adas_opencv_video_source(Data);
                        break;
                default:
                        break;
        }
}

void dequeue_adas(struct Data *Data, int type)
{
        switch(type)
        {   
                //capture source
                case 0:
                        dequeue_adas_capture(Data);
                        break;
                case 1:
                        dequeue_adas_video(Data);
			break;
		case 10:
			dequeue_adas_opencv_capture(Data);
			break;
		case 11:
                        dequeue_adas_opencv_video(Data);
                        break;
                default:
                        break;
        }
}

void enqueue_adas(struct Data *Data, int type)
{
        switch(type)
        {   
                //capture source
                case 0:
                        enqueue_adas_capture(Data);
                        break;
                case 1:
                        break;
                default:
                        break;
        }
}
