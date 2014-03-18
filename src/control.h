/****************************************
*		Player for loongson 1B			*
*										*
*		Version: 0.1.0.140123_Alpha		*
*										*
*		Copyright © 2014 hang.im 		*
*		All rights reserved. 			*
****************************************/

#include "musicList.h"

#ifndef CONTROL_H
#define CONTROL_H

/*** Fifo functions ***/
int fifo_init();
int fifo_get_fd();
int fifo_write( const char * cmd );
int fifo_close();

int fifo_play( const struct Music * music );
int fifo_playList( const struct MusicList * musicList );
int fifo_stop();
int fifo_pause();
int fifo_quit();
int fifo_pre_music();
int fifo_next_music();
int fifo_mute( const int state );
int fifo_set_volume( const int volume );
#endif
