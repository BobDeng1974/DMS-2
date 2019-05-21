#include "source.h"
#include <opencv2/opencv.hpp>
/*
#ifdef __cplusplus
extern "C" 
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif

std::string mjpeg_filepath = "camera_0.mjpeg";
FILE* mjpeg_file;

AVCodec* pCodec;
AVCodecContext* pCodecCtx;
AVCodecParserContext* pCodecParserCtx;
AVPacket frame_packet;
AVFrame* ptr_frame;

struct SwsContext* img_convert_ctx;
AVFrame* pFrameYUV;

int frame_amount = 0; 
bool video_end = false;
unsigned char* cur_ptr;
int cur_size = 0;
bool is_need_offset = false;
const int in_buffer_size = 4096;
unsigned char in_buffer[in_buffer_size + AV_INPUT_BUFFER_PADDING_SIZE];

int init_ffmpeg_environment(void);
void read_one_frame(void* param);

int init_adas_video_source(struct Data *data)
{
    printf("enter function init_adas_video_source!\n");
    // initialize ffmpeg environment
    int ret = init_ffmpeg_environment();
    if(-1 == ret) {
        printf("init_ffmpeg_environment() failed\n");
        return ret;
    }

    // open mjpeg file
    mjpeg_file = fopen(mjpeg_filepath.c_str(), "rb");
    if (!mjpeg_file) {
    	printf("Could not open input stream %s\n", mjpeg_filepath.c_str());
        return -1;
    }

    // initialize  buffer
    data->buffers[0].start = (unsigned char *)malloc(data->cam_width * data->cam_height * 2);
    if (NULL != data->buffers[0].start) {
            memset(data->buffers[0].start, 0, data->cam_width * data->cam_height * 2);
            data->buffers[0].length = data->cam_width * data->cam_height * 2;
    } else {
            printf("Fail to allocate memomy for buffer\n");
            return -1;
    }

    return 0;
}

int init_ffmpeg_environment(void)
{
    // prepare ffmpeg context
    avcodec_register_all();
        pCodec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
        if (!pCodec) {
            printf("Codec not found\n");
            return -1;
        }

        pCodecCtx = avcodec_alloc_context3(pCodec);
        if (!pCodecCtx){
            printf("Could not allocate video codec context\n");
            return -1;
        }

        pCodecParserCtx = av_parser_init(AV_CODEC_ID_MJPEG);
        if (!pCodecParserCtx) {
            printf("Could not allocate video parser context\n");
            return -1;
        }

        if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
            printf("Could not open codec\n");
            return -1;
        }

        ptr_frame = av_frame_alloc();
        av_init_packet(&frame_packet);

    //disable ffmpeg log
    av_log_set_level(AV_LOG_QUIET);

    return 0;
}

int read_one_frame(struct Data* data)
{
    int PIC_IMAGE_SIZE_YUYV = data->cam_width * data->cam_height * 2;
    while(1)
    {
        if(is_need_offset)
        {
            fseek(mjpeg_file, 0 - cur_size, SEEK_CUR);
            is_need_offset = false;
        }
        cur_size = fread(in_buffer, 1, in_buffer_size, mjpeg_file);
        if (0 == cur_size)
        {
            video_end = true;
            break;
        }

        cur_ptr = in_buffer;

        while (cur_size > 0)
        {
            int len = av_parser_parse2(
                pCodecParserCtx, pCodecCtx,
                &frame_packet.data, &frame_packet.size,
                cur_ptr, cur_size,
                AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
            cur_ptr += len;
            cur_size -= len;

            if(0 == frame_packet.size)
                continue;
            avcodec_send_packet(pCodecCtx, &frame_packet);
            if (0 == avcodec_receive_frame(pCodecCtx, ptr_frame))
            {
                ++frame_amount;

                                uint8_t* ptr = (uint8_t*) data->buffers[0].start;

                                memcpy(ptr, ptr_frame->data[0], PIC_IMAGE_SIZE_YUYV / 2);
                                ptr += PIC_IMAGE_SIZE_YUYV / 2;

                                memcpy(ptr, ptr_frame->data[1], PIC_IMAGE_SIZE_YUYV / 4);
                                ptr += PIC_IMAGE_SIZE_YUYV / 4;

                                memcpy(ptr, ptr_frame->data[2], PIC_IMAGE_SIZE_YUYV / 4);
                                ptr += PIC_IMAGE_SIZE_YUYV / 4;

                                assert(ptr == (uint8_t*) data->buffers[0].start + data->buffers[0].length);

                is_need_offset = true;
                return 0;
            }
            else
            {
                printf("Decode Error.\n");
                return -1;
            }
        }
    }

    return -1;
}


int dequeue_adas_video(struct Data *data)
{
    read_one_frame(data);
    if(video_end)
    {
            printf("frame_count: %d\n",  frame_amount);
    }
}

void fini_adas_video_source(struct Data *Data)
{
        fclose(mjpeg_file);
        av_frame_free(&ptr_frame);
        av_parser_close(pCodecParserCtx);
        avcodec_close(pCodecCtx);
        av_free(pCodecCtx);
        sws_freeContext(img_convert_ctx);
        av_frame_free(&pFrameYUV);
}
*/
