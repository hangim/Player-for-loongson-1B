/****************************************
*		Player for loongson 1B			*
*										*
*		Version: 0.1.0.140123_Alpha		*
*										*
*		Copyright © 2014 hang.im 		*
*		All rights reserved. 			*
****************************************/

#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <iso646.h> // C alternative tokens

#include "musicList.h"

/******************************************************************************/
struct Music * create_music( const char * path, const char * name ){
	struct Music * music = ( struct Music * ) malloc( sizeof( struct Music ) );
	strncpy( music->path, path, 256 );
	strncpy( music->name, name, 256 );
	return music;
}

struct Music * get_pre_music( const struct MusicList * musicList, const struct Music * music ){
	if ( musicList == NULL or musicList->number <= 1 )
		return NULL;

	if ( music == musicList->head->next ) // if it's first music
		return musicList->head->pre; // last music
	else
		return music->pre;
}

struct Music * get_next_music( const struct MusicList * musicList, const struct Music * music ){
	if ( musicList == NULL or musicList->number <= 1 )
		return NULL;
	
	if ( music == musicList->head->pre ) // if it's last music
		return musicList->head->next; // first music
	else
		return music->next;
}

/******************************************************************************/
struct MusicList * create_musicList(){
	struct MusicList * musicList = ( struct MusicList * )malloc( sizeof( struct MusicList ) );
	musicList->number = 0;
	musicList->head = ( struct Music * )malloc( sizeof( struct Music ) );
	musicList->head->next = NULL;
	musicList->head->pre = NULL;
	return musicList;
}

int add_music_to_musicList( struct MusicList * musicList, struct Music * music ){
	if ( musicList->head->next == NULL ){
		music->id = 1;
		music->pre = musicList->head;
		music->next = musicList->head;

		musicList->head->next = music;
		musicList->head->pre = music;
	} else {
		music->id = musicList->head->pre->id + 1;
		music->next = musicList->head;
		music->pre = musicList->head->pre;

		musicList->head->pre->next = music;
		musicList->head->pre = music;
	}
	musicList->number += 1;
	return 0;
}

struct Music * get_music_by_id( const struct MusicList * musicList, const int id ){
	if ( id <= 0 or id > musicList->number )
		return NULL;

	struct Music * music = musicList->head;
	while ( music->id != id )
		music = music->next;

	return music;
}

/******************************************************************************/
struct MusicList * load_musicList( const char * path ){
	DIR * pDir;
	struct dirent * ent;
	char filename[256];

	struct MusicList * musicList = create_musicList();

	if ( ( pDir = opendir( path ) ) == NULL )
		return musicList;

	while ( ( ent = readdir( pDir ) ) != NULL ){
		if ( ent->d_type == DT_REG ){
			sprintf( filename, "%s", ent->d_name );
			if ( strlen( filename ) > 4 and strcmp( filename + strlen( filename ) - 4, ".mp3" ) == 0 ){
				struct Music * music = create_music( path, ent->d_name );
				add_music_to_musicList( musicList, music );
			}
		}
	}

	closedir( pDir );
	return musicList;
}

/******************************************************************************/
struct MusicList * load_fmList( const char * path ){
	FILE * fp;
	char fmPath[256];
	char fmName[256];

	struct MusicList * musicList = create_musicList();

	if ( ( fp = fopen( path , "r") ) == NULL )
		return musicList;

	while ( !feof( fp ) ){
		fscanf( fp, "%[^,],%s\n", fmName, fmPath );
		struct Music * music = create_music( fmPath, fmName );
		add_music_to_musicList( musicList, music );
	}

	fclose( fp );
	return musicList;
}

/******************************************************************************/
int travel_musicList( const struct MusicList * musicList ){
	if ( musicList->head == NULL )
		return -1;

	struct Music * music = musicList->head;
	printf( "\n" );
	while ( music->next != musicList->head ){
		music = music->next;
		printf( "%4d : %s \t %s\n", music->id, music->name, music->path );
	}
	return 0;
}
