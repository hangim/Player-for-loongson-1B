/****************************************
*		Player for loongson 1B			*
*										*
*		Version: 0.1.0.140123_Alpha		*
*										*
*		Copyright © 2014 hang.im 		*
*		All rights reserved. 			*
****************************************/

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <iso646.h>

#include "control.h"

#ifndef PIPE_BUF
#define PIPE_BUF 4096
#endif

#ifndef File_Buffer_Size
#define File_Buffer_Size 256
#endif

int pipe_fd = 0;
const char * fifo_name = "/tmp/player_fifo";
const char * player_list = "/tmp/player_list.txt";

char fifo_buffer[PIPE_BUF];

/******************************************************************************/
int fifo_init(){
	// check whether exist fifo
	if ( access( fifo_name, F_OK ) == -1 ){
		// don't exist fifo
		// create fifo
		if ( mkfifo( fifo_name, 0777 ) == -1 ){
			// create fifo failed
			fprintf( stderr, "create fifo failed\n" );
			return -1;
		}
	}

	return 0;
}

int fifo_get_fd(){
	// open fifo
	if ( pipe_fd <= 0 )
		if ( ( pipe_fd = open( fifo_name, O_WRONLY ) ) == -1 ){
			// open fifo failed
			fprintf( stderr, "open fifo failed\n" );
		}
	return pipe_fd;
}

int fifo_write( const char * cmd ){
	if ( fifo_get_fd() == -1 )
		return -1;

	if ( write( pipe_fd, cmd, strlen( cmd ) ) == -1 ){
		fprintf( stderr, "write fifo failed\n" );
		return -1;
	}

	// fifo_close();
	return 0;
}

/******************************************************************************/
int fifo_play( const struct Music * music ){
	if ( snprintf( fifo_buffer, PIPE_BUF, "loadfile \"%s/%s\"\n", music->path, music->name ) < PIPE_BUF ){
		// check str length
		return fifo_write( fifo_buffer );
	}
	return -1;
}

int fifo_playList( const struct MusicList * musicList ){
	if ( musicList->head == NULL ){
		fprintf( stderr, "maybe you don't have music.\n" );
		return -1;
	}

	struct Music * music = musicList->head;
	char file_buffer[File_Buffer_Size];

	// Write playlist to file
	int playlist_fd = open( player_list, O_WRONLY | O_NONBLOCK );
	if ( playlist_fd == -1 ){
		fprintf( stderr, "open player_list failed\n" );
		return -1;
	}

	while ( music->next != musicList->head ){
		music = music->next;
		snprintf( file_buffer, File_Buffer_Size, "%s/%s\n", music->path, music->name );
		write( playlist_fd, file_buffer, strlen( file_buffer ) );
	}
	close( playlist_fd );

	// Loadlist
	snprintf( fifo_buffer, PIPE_BUF, "loadlist %s -shuffle\n", player_list );
	return fifo_write( fifo_buffer );
}

int fifo_stop(){
	return fifo_write( "stop\n" );
}

int fifo_pause(){
	return fifo_write( "pause\n" );
}

int fifo_quit(){
	return fifo_write( "quit\n" );
}

int fifo_pre_music(){
	return fifo_write( "pt_step -1\n" );
}

int fifo_next_music(){
	return fifo_write( "pt_step 1\n" );
}

int fifo_mute( const int state ){
	snprintf( fifo_buffer, PIPE_BUF, "mute %d\n", state );
	return fifo_write( fifo_buffer );
}

int fifo_set_volume( const int volume ){
	snprintf( fifo_buffer, PIPE_BUF, "volume %d\n", volume );
	return fifo_write( fifo_buffer );
}

int fifo_close(){
	if ( fifo_get_fd() > 0 )
		close( fifo_get_fd() );
	pipe_fd = 0;
	return 0;
}
