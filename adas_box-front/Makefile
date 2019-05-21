TARGET = adas-box-front

CC = g++
CPPFLAGS = -g3 -std=c++11 -I./ -I/opt/intel/computer_vision_sdk/deployment_tools/inference_engine/include \
	-I/installation/OpenCV-3.4/include -I/opt/intel/computer_vision_sdk/deployment_tools/inference_engine/src/extension
CFLAGS += -L/installation/OpenCV-3.4/lib

LIBS = -lm -lstdc++ \
	-lopencv_core -lpthread\
	-lavcodec -lavformat -lavutil -lswscale \
	-L. -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_imgcodecs -lopencv_calib3d \
	-L/opt/intel/computer_vision_sdk/deployment_tools/inference_engine/lib/ubuntu_16.04/intel64 \
	-ldl -linference_engine


SRCS = $(wildcard ./*.cpp)
OBJS = $(SRCS:.cpp=.o)
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
clean:
	rm -rf $(TARGET) *.o

