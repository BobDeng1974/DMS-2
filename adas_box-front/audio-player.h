#ifndef OBJECT_H_
#define OBJECT_H_

#include "thpool.h"
#include "util.h"
extern threadpool g_thpool;

class AudioPlayer {
private:
	AudioPlayer() {}
	static void ldw_left(void* param)
	{
		system("play audio/ldw-left.mp3");
	}
	static void ldw_right(void* param)
	{
		system("play audio/ldw-right.mp3");
	}
	static void fcdw(void* param)
        {
                system("play audio/fcdw.mp3");
        }

public:
	static void init() {
		if(g_thpool==NULL)
			g_thpool = thpool_init(4);
	}
	static void fini(){
		thpool_destroy(g_thpool);
	}
	static void play_ldw_left() {
		thpool_add_work(g_thpool, ldw_left, 0);
	}
	static void play_ldw_right() {
		thpool_add_work(g_thpool, ldw_right, 0);
	}
	static void play_fcdw() {
                thpool_add_work(g_thpool, fcdw, 0);
        }

	static void play_dsm(enum DsmStatus status) {
		switch (status) {
			case LOOKLEFTRIGHT:
					system("play audio/dsm-front.mp3");
					break;
			case PHONE:
					system("play audio/dsm-phone.mp3");
					break;
			case SLEEP:
					system("play audio/dsm-sleep.mp3");
					break;
			case YAWN:
					system("play audio/dsm-yawn.mp3");
					break;
			default:
					break;
		}
	}
	
};

#endif
