/****************************************
*		Player for loongson 1B			*
*										*
*		Version: 0.1.0.140123_Alpha		*
*										*
*		Copyright © 2014 hang.im 		*
*		All rights reserved. 			*
****************************************/

#include "musicList.h"
#include "control.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

#ifndef PLAYER_H
#define PLAYER_H

struct Point{
	int x;
	int y;
};

struct Button{
	int visible;
	int img_number;
	int current_img;
	SDL_Rect det;
	int ( *down_fun )();
	int ( *up_fun )();
	SDL_Surface **img;
};

SDL_Surface * load_image( const char * filename );
int	refresh_screen( SDL_Surface * screen, struct Button buttons[] );

int is_in_button( const struct Point point, const struct Button button );
int init_buttons( struct Button buttons[] );
int uninit_buttons( struct Button buttons[] );

int loop_event( struct Button buttons[] );

int play_button_down();
int pause_button_down();
int pre_music_button_down();
int next_music_button_down();
int volume_inc_button_down();
int volume_dec_button_down();
int list_button_down();
int mute_button_down();
int help_button_down();
int quit_button_down();

int play_button_up();
int pause_button_up();
int pre_music_button_up();
int next_music_button_up();
int volume_inc_button_up();
int volume_dec_button_up();
int list_button_up();
int mute_button_up();
int mode_button_up();
int help_button_up();
int quit_button_up();

int has_mplayer();
int start_mplayer();
int play_music();
int show_music_message();

void quit();
int welcome();

#endif
