/****************************************
*		Player for loongson 1B			*
*										*
*		Version: 0.1.0.140123_Alpha		*
*										*
*		Copyright © 2014 hang.im 		*
*		All rights reserved. 			*
****************************************/

#ifndef MUSICLIST_H
#define MUSICLIST_H

struct Music{
	int id;
	struct Music * pre;
	struct Music * next;
	char path[256];
	char name[256];
};

struct MusicList{
	int number;
	struct Music * head;
};

/*** Music Functions ***/
struct Music * create_music( const char * path, const char * name );
struct Music * get_pre_music( const struct MusicList * musicList, const struct Music * music );
struct Music * get_next_music( const struct MusicList * musicList, const struct Music * music );

/*** MusicList Functions ***/
struct MusicList * create_musicList();
int add_music_to_musicList( struct MusicList * musicList, struct Music * music );
struct Music * get_music_by_id( const struct MusicList * musicList, const int id );
struct MusicList * load_musicList( const char * path );
struct MusicList * load_fmList( const char * path );

int travel_musicList( const struct MusicList * musicList );
#endif
