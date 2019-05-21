#include "source.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>

const char* capture_device[2] = { "/dev/video0", "/dev/video1"};

struct v4l2_buffer g_capture_cur_buffer[2];

int open_video_device(const char *device_name)
{
	struct stat st; 
        int fd; 

        if (-1 == stat (device_name, &st)) {
                printf("stat failed\n");
                return -1; 
        }   

        if (!S_ISCHR(st.st_mode)) {
                printf ( "device is no char device\n");
                return -1; 
        }   

        fd = open(device_name, O_RDWR , 0); 
        if (-1 == fd) {
                printf ( "Cannot open device\n");
                return -1; 
        }   
        return fd;
}


int init_mmap(struct Data *data, int fd, int index)
{
        struct v4l2_requestbuffers req;
        struct v4l2_buffer cur_buf;
        int i,n_buffers;
        //enum v4l2_buf_type type;

        memset(&req, 0, sizeof(req));

        /* initiate memory mapping usign IOCTL */
        /* setting the buffers count to 3 */
        req.count               = 3;
        req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        /* specifying the memory type to MMAP */
        req.memory              = V4L2_MEMORY_MMAP;

        if (-1 == ioctl (fd, VIDIOC_REQBUFS, &req)) {
                printf("failed to initiate memory mapping (%d)\n", errno);
                return -1;
        }

        /* if device doesn't supprt multiple buffers */
        if (req.count < 2) {
                printf("device doesn't support multiple buffers(%d)\n",
                       req.count);
                return -1;
        }


        /* allocating buffers struct*/
	/*
        device->buffers = calloc (req.count, sizeof (struct buffer));
        if (!(device->buffers)) {
                printf("failed to allocate buffers\n");
                return -1;
        }
	*/

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                /* using IOCTL to query the buffer status */
                memset(&cur_buf, 0, sizeof(cur_buf));
                cur_buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                cur_buf.memory      = V4L2_MEMORY_MMAP;
                cur_buf.index       = n_buffers;

                if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &cur_buf)) {
                        printf("failed to query buffer status %d\n", errno);
                        return -1;
                }

                /* mmapping the buffers */
                data->buffers[index][n_buffers].length = cur_buf.length;
                data->buffers[index][n_buffers].start =
                        mmap(NULL, cur_buf.length, PROT_READ | PROT_WRITE,
                             MAP_SHARED, fd, cur_buf.m.offset);
                if (MAP_FAILED == data->buffers[index][n_buffers].start) {
                        printf("failed to map buffer\n");
                        return -1;
                }
        }

        /* enqueueing buffers to device using IOCTL */
        for (i = 0; i < n_buffers; ++i) {
                memset(&cur_buf, 0, sizeof(cur_buf));
                cur_buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                cur_buf.memory      = V4L2_MEMORY_MMAP;
                cur_buf.index       = i;

                if (-1 == ioctl (fd, VIDIOC_QBUF, &cur_buf))
                        return -1;
        }
	data->fd[index]=fd;
        //device->n_buffers = n_bufifers;

	enum v4l2_buf_type type;	
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        // starting the streaming 
        if (-1 == ioctl (fd, VIDIOC_STREAMON, &type)) {
                printf("failed to start the streaming (%d)\n", errno);
                return -1;
        }
	
        return 1;
}

int init_video_device(struct Data *data, int fd, int index)
{
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;


        /* using IOCTL to quesry the device capabilities */
        if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &cap))
        {
                if (EINVAL == errno) {
                        printf ("device is no V4L2 device\n");
                        return -1;
                } else {
                        printf("Error getting device capabilities (%d)\n",
                               errno);
                        return -1;
                }
        }

        /* checking if the device supports video capture */
        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        {
                printf("device is no video capture device\n");
                return -1;
        }

        /* checking if the device supports video streaming */
        if (!(cap.capabilities & V4L2_CAP_STREAMING))
        {
                printf("device does not support streaming i/o\n");
                return -1;
        }


        /* Using IOCTL to query the capture capabilities */
        /*memset(&cropcap, 0, sizeof(cropcap));
        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (0 == ioctl (fd, VIDIOC_CROPCAP, &cropcap))
        {
                crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                crop.c = cropcap.defrect; 

                if (-1 == ioctl (fd, VIDIOC_S_CROP, &crop))
			printf("failed to set cropping rectengle\n");
        }*/


        /* setting the video data format */
        memset(&fmt, 0, sizeof(fmt));
        fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = data->cam_width;
        fmt.fmt.pix.height      = data->cam_height;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

        if (-1 == ioctl (fd, VIDIOC_S_FMT, &fmt)) {
                printf ("Failed to set video data format\n");
                return -1;
        }
	/*
	struct v4l2_control vc;  
	vc.id = V4L2_CID_VFLIP;  
	vc.value = 1;

	// Set up for mirror
	if (ioctl(fd, VIDIOC_S_CTRL, &vc) < 0) {  
    		printf("Failed to set up for miror!\n");  
    		return -1;  
	}  
	*/

        /* initalizing device memory */
        if (init_mmap (data, fd, index) < 0) {
                printf("device dosen't suppprt MMAP\n");
                return -1;
        }
        return 1;

}


void init_adas_capture_source(struct Data *data)
{
	printf("enter function init_avm_capture_source!\n");
	for(int index=0; index<adas_camera_num; index++) {
		int fd = open_video_device(capture_device[index]);
        	if (fd < 0) {
                	printf("failed to open video device\n");
                	return;
        	}
        	int ret = init_video_device(data, fd, index);
        	if (ret < 0) {
                	printf("failed to initalize video device\n");
        	}
	}
}

void close_video_device(struct Data *data, int index)
{
        int i;
        for(i = 0; i < 3; i++) {
        	munmap(data->buffers[index][i].start,
                               data->buffers[index][i].length);
        }
        close(data->fd[index]);
}

void fini_adas_capture_source(struct Data *data)
{
	for(int index=0; index<adas_camera_num; index++)
		close_video_device(data,index);
}

void dequeue_adas_capture(struct Data *data)
{
	for(int index=0; index<adas_camera_num; index++) {
		memset(&g_capture_cur_buffer[index], 0, sizeof(g_capture_cur_buffer[index]));
        	/* using IOCTL to dequeue a full buffer from the queue */
        	g_capture_cur_buffer[index].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        	g_capture_cur_buffer[index].memory = V4L2_MEMORY_MMAP;

        	if (-1 == ioctl(data->fd[index], VIDIOC_DQBUF, &g_capture_cur_buffer[index]))
                	printf("failed to dequeue buffer (%d)\n", errno);
		data->buffer_index[index] = g_capture_cur_buffer[index].index;
	}
}

void enqueue_adas_capture(struct Data *data)
{
	for(int index=0; index<adas_camera_num; index++) {
		/* using IOCTL to enqueue back the buffer after we used it */
        	if (-1 == ioctl(data->fd[index], VIDIOC_QBUF, &g_capture_cur_buffer[index]))
                	printf("failed to enqueue buffer (%d)\n", errno);
	}
}
