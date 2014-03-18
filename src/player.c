/****************************************
*		Player for loongson 1B			*
*										*
*		Version: 0.1.0.140123_Alpha		*
*										*
*		Copyright © 2014 hang.im 		*
*		All rights reserved. 			*
****************************************/

#include <iso646.h> // c alternative tokens
#include <unistd.h>

#include "player.h"

// #define NO_MPLAYER
// #define PC

#define BUTTON_NUMBER 13

// the attributes of the screen
const int Screen_Width = 480;
const int Screen_Height = 272;
const int Screen_Bpp = 16;

#ifndef PC
const char * Music_Path = "/mnt/music";
#else
const char * Music_Path = "/home/zh/music";
#endif

const char * FM_Path = "conf/fm.ini";

// int is_playing = 0;
struct MusicList * musicList = NULL;
struct MusicList * fmList = NULL;
struct MusicList * nowList = NULL;
struct Music * music = NULL;
SDL_Surface * main_screen = NULL;
TTF_Font * mainFont = NULL;
TTF_Font * subFont = NULL;

// 0 --> music
// 1 --> fm
int mode = 1;

struct Button buttons[BUTTON_NUMBER];
struct Button * const background_button  = &buttons[0];
struct Button * const pre_music_button   = &buttons[1];
struct Button * const play_button        = &buttons[2];
struct Button * const pause_button       = &buttons[3];
struct Button * const next_music_button  = &buttons[4];
struct Button * const quit_button        = &buttons[5];
struct Button * const volume_inc_button  = &buttons[6];
struct Button * const volume_dec_button  = &buttons[7];
struct Button * const mute_button		 = &buttons[8];
struct Button * const mode_button		 = &buttons[9];

struct Button * const pre_music_message  = &buttons[10];
struct Button * const now_music_message  = &buttons[11];
struct Button * const next_music_message = &buttons[12];

int main(){
	atexit( quit );

	// init SDL
	if ( SDL_Init( SDL_INIT_VIDEO ) == -1 ){
		fprintf( stderr, "SDL_Init: %s\n", SDL_GetError() );
		exit( 1 );
	}
	
	// init TTF
	if ( TTF_Init() == -1 ){
		fprintf( stderr, "TTF_Init: %s\n", TTF_GetError() );
		exit( 1 );
	}

	// init font
	mainFont = TTF_OpenFont( "font/Inconsolata.ttf", 22 );
	TTF_SetFontStyle( mainFont, TTF_STYLE_ITALIC );
	subFont = TTF_OpenFont( "font/Inconsolata.ttf", 20 );
	// TTF_SetFontStyle( subFont, TTF_STYLE_NORMAL );
	if ( mainFont == NULL or subFont == NULL ){
		fprintf( stderr, "TTF_OpenFont: %s\n", TTF_GetError() );
		exit( 1 );
	}

#ifndef PC
	// hide cursor
	if ( SDL_ShowCursor(SDL_DISABLE) == -1 ){
		fprintf( stderr, "SDL_ShowCursor: %s\n", SDL_GetError() );
		exit( 1 );
	}
#endif

	// set up screen
	main_screen = SDL_SetVideoMode( Screen_Width, Screen_Height, Screen_Bpp, SDL_SWSURFACE );
	if ( main_screen == NULL ){
		fprintf( stderr, "SDL_SetVideoMode: %s\n", SDL_GetError() );
		exit(1);
	}

	// init buttons
	init_buttons( buttons ); 
	// refresh_screen( main_screen, buttons );

	// load musicList
	musicList = load_musicList( Music_Path );
	fmList = load_fmList( FM_Path );
	// travel_musicList( musicList );
	// music = musicList->head->next; // first music

	// show_music_message();
	
	// init fifo
	if ( fifo_init() == -1 ){
		fprintf(stderr, "fifo_init\n" );
		exit( 1 );
	}

	// start mplayer
	if ( has_mplayer() == -1 and start_mplayer() == -1 )
		;

	welcome();
	mode_button_up();
	refresh_screen( main_screen, buttons );
	// show_music_message();

	// loop to get event
	loop_event( buttons );

	return 0;
}

int loop_event( struct Button buttons[] ){
	const unsigned int interval = 400;

	SDL_Event event;
	struct Point point;
	int i;
	unsigned int button_down_time = 0;
	unsigned int button_up_time = 0;
	unsigned int now_time;
	
	while ( 1 ) {
		// SDL_PollEvent( &event );
		// sleep( 1 );
		SDL_WaitEvent( &event );

		now_time = SDL_GetTicks();

		if ( event.type == SDL_QUIT )
			return 0;

		if ( event.type == SDL_MOUSEBUTTONDOWN ){
			point.x = event.motion.x;
			point.y = event.motion.y;

			if ( now_time - button_down_time > interval )
				button_down_time = now_time;
			else
				continue;

			for ( i = 0; i < BUTTON_NUMBER; i++ ){
				if ( buttons[i].visible  and buttons[i].down_fun != NULL 
					and is_in_button( point, buttons[i] ) ){
					buttons[i].down_fun();
				}
			}
			refresh_screen( main_screen, buttons );
			continue;
		}

		if ( event.type == SDL_MOUSEBUTTONUP ){
			point.x = event.motion.x;
			point.y = event.motion.y;

			if ( now_time - button_up_time > interval )
				button_up_time = now_time;
			else
				continue;

			for ( i = 0; i < BUTTON_NUMBER; i++ ){
				if ( buttons[i].visible and buttons[i].up_fun != NULL 
					and is_in_button( point, buttons[i] ) ){
					buttons[i].up_fun();
				}
			}
			refresh_screen( main_screen, buttons );
			continue;
		}
	}
}

int	refresh_screen( SDL_Surface * screen, struct Button buttons[] ){
	int i;
	for ( i = 0; i < BUTTON_NUMBER; i++ ){
		if ( buttons[i].visible == 1 )
			// apply image to screen
			SDL_BlitSurface( buttons[i].img[buttons[i].current_img], NULL, screen, &buttons[i].det );
	}

	// update scren
	SDL_Flip( screen );
	return 0;
}

int is_in_button( const struct Point point, const struct Button button ){
	if ( point.x < button.det.x or point.x > button.det.x + button.det.w )
		return 0;
	if ( point.y < button.det.y or point.y > button.det.y + button.det.h )
		return 0;
	return 1;
}

int init_buttons( struct Button buttons[] ){
	struct Button * b;
	
	// background
	b = background_button;
	b->visible     = 1;
	b->img_number  = 1;
	b->current_img = 0;
	b->det.x       = 0;
	b->det.y       = 0;
	b->det.w       = 480;
	b->det.h       = 272;
	b->img         = ( SDL_Surface ** ) calloc( b->img_number, sizeof( SDL_Surface * ) );
	b->img[0]      = load_image( "img/background.bmp" );
	b->down_fun    = NULL;
	b->up_fun      = NULL;

	// pre_music_button
	b = pre_music_button;
	b->visible     = 1;
	b->img_number  = 2;
	b->current_img = 0;
	b->det.x       = 43;
	b->det.y       = 197;
	b->det.w       = 74;
	b->det.h       = 70;
	b->img         = ( SDL_Surface ** ) calloc( b->img_number, sizeof( SDL_Surface * ) );
	b->img[0]      = load_image( "img/button_playing_pre_normal.bmp" );
	b->img[1]      = load_image( "img/button_playing_pre_press.bmp" );
	b->down_fun    = pre_music_button_down;
	b->up_fun      = pre_music_button_up;

	// play_button
	b = play_button;
	b->visible     = 1;
	b->img_number  = 2;
	b->current_img = 0;
	b->det.x       = 123;
	b->det.y       = 197;
	b->det.w       = 74;
	b->det.h       = 70;
	b->img         = ( SDL_Surface ** ) calloc( b->img_number, sizeof( SDL_Surface * ) );
	b->img[0]      = load_image( "img/button_playing_play_normal.bmp" );
	b->img[1]      = load_image( "img/button_playing_play_press.bmp" );
	b->down_fun    = play_button_down;
	b->up_fun      = play_button_up;

	// pause_button
	b = pause_button;
	b->visible     = 1;
	b->img_number  = 2;
	b->current_img = 0;
	b->det.x       = 203;
	b->det.y       = 197;
	b->det.w       = 74;
	b->det.h       = 70;
	b->img         = ( SDL_Surface ** ) calloc( b->img_number, sizeof( SDL_Surface * ) );
	b->img[0]      = load_image( "img/button_playing_suspension_normal.bmp" );
	b->img[1]      = load_image( "img/button_playing_suspension_press.bmp" );
	b->down_fun    = pause_button_down;
	b->up_fun      = pause_button_up;

	// next_music_button
	b = next_music_button;
	b->visible     = 1;
	b->img_number  = 2;
	b->current_img = 0;
	b->det.x       = 283;
	b->det.y       = 197;
	b->det.w       = 74;
	b->det.h       = 70;
	b->img         = ( SDL_Surface ** ) calloc( b->img_number, sizeof( SDL_Surface * ) );
	b->img[0]      = load_image( "img/button_playing_next_normal.bmp" );
	b->img[1]      = load_image( "img/button_playing_next_press.bmp" );
	b->down_fun    = next_music_button_down;
	b->up_fun      = next_music_button_up;

	// quit_button
	b = quit_button;
	b->visible     = 1;
	b->img_number  = 2;
	b->current_img = 0;
	b->det.x       = 5;
	b->det.y       = 5;
	b->det.w       = 60;
	b->det.h       = 60;
	b->img         = ( SDL_Surface ** ) calloc( b->img_number, sizeof( SDL_Surface * ) );
	b->img[0]      = load_image( "img/quit_button1.bmp" );
	b->img[1]      = load_image( "img/quit_button1.bmp" );
	b->down_fun    = quit_button_down;
	b->up_fun      = quit_button_up;

	// volume_inc_button
	b = volume_inc_button;
	b->visible     = 1;
	b->img_number  = 2;
	b->current_img = 0;
	b->det.x       = 410;
	b->det.y       = 15;
	b->det.w       = 60;
	b->det.h       = 60;
	b->img         = ( SDL_Surface ** ) calloc( b->img_number, sizeof( SDL_Surface * ) );
	b->img[0]      = load_image( "img/volume_inc_button1.bmp" );
	b->img[1]      = load_image( "img/volume_inc_button2.bmp" );
	b->down_fun    = volume_inc_button_down;
	b->up_fun      = volume_inc_button_up;

	// volume_dec_button
	b = volume_dec_button;
	b->visible     = 1;
	b->img_number  = 2;
	b->current_img = 0;
	b->det.x       = 410;
	b->det.y       = 197;
	b->det.w       = 60;
	b->det.h       = 60;
	b->img         = ( SDL_Surface ** ) calloc( b->img_number, sizeof( SDL_Surface * ) );
	b->img[0]      = load_image( "img/volume_dec_button1.bmp" );
	b->img[1]      = load_image( "img/volume_dec_button2.bmp" );
	b->down_fun    = volume_dec_button_down;
	b->up_fun      = volume_dec_button_up;

	// mute_button
	b = mute_button;
	b->visible     = 1;
	b->img_number  = 4;
	b->current_img = 0;
	b->det.x       = 340;
	b->det.y       = 5;
	b->det.w       = 60;
	b->det.h       = 60;
	b->img         = ( SDL_Surface ** ) calloc( b->img_number, sizeof( SDL_Surface * ) );
	b->img[0]      = load_image( "img/mute_button1.bmp" );
	b->img[1]      = load_image( "img/mute_button2.bmp" );
	b->img[2]      = load_image( "img/mute_button3.bmp" );
	b->img[3]      = load_image( "img/mute_button4.bmp" );
	b->down_fun    = mute_button_down;
	b->up_fun      = mute_button_up;

	// mode_button
	b = mode_button;
	b->visible     = 1;
	b->img_number  = 2;
	b->current_img = 0;
	b->det.x       = 80;
	b->det.y       = 5;
	b->det.w       = 60;
	b->det.h       = 60;
	b->img         = ( SDL_Surface ** ) calloc( b->img_number, sizeof( SDL_Surface * ) );
	b->img[0]      = load_image( "img/music.bmp" );
	b->img[1]      = load_image( "img/fm.bmp" );
	b->down_fun    = NULL;
	b->up_fun      = mode_button_up;

	// pre_music_message
	b = pre_music_message;
	b->visible     = 0;
	b->img_number  = 1;
	b->current_img = 0;
	b->det.x       = 20;
	b->det.y       = 80;
	b->det.w       = 380;
	b->det.h       = 25;
	b->img         = ( SDL_Surface ** ) calloc( b->img_number, sizeof( SDL_Surface * ) );
	b->img[0]      = NULL;
	b->down_fun    = NULL;
	b->up_fun      = pre_music_button_up;

	// now_music_message
	b = now_music_message;
	b->visible     = 0;
	b->img_number  = 1;
	b->current_img = 0;
	b->det.x       = 20;
	b->det.y       = 115;
	b->det.w       = 380;
	b->det.h       = 25;
	b->img         = ( SDL_Surface ** ) calloc( b->img_number, sizeof( SDL_Surface * ) );
	b->img[0]      = NULL;
	b->down_fun    = NULL;
	b->up_fun      = NULL;

	// next_music_message
	b = next_music_message;
	b->visible     = 0;
	b->img_number  = 1;
	b->current_img = 0;
	b->det.x       = 20;
	b->det.y       = 150;
	b->det.w       = 380;
	b->det.h       = 25;
	b->img         = ( SDL_Surface ** ) calloc( b->img_number, sizeof( SDL_Surface * ) );
	b->img[0]      = NULL;
	b->down_fun    = NULL;
	b->up_fun      = next_music_button_up;
}

SDL_Surface * load_image( const char * filename ){
	// temporary storage for the image that's loaded
	SDL_Surface* loadedImage = NULL;

	// the optimized image that will be used
	SDL_Surface* optimizedImage = NULL;

	// load the image
	loadedImage = SDL_LoadBMP( filename );

	// if nothing went wrong in loading the image
	if( loadedImage != NULL ){
		// create an optimized image
		optimizedImage = SDL_DisplayFormat( loadedImage );

		// map the color key
		Uint32 colorkey = SDL_MapRGB( optimizedImage->format, 249, 199, 99 );

		// set all pixels of colorkey to be transparent
		SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, colorkey );

		// free the old image
		SDL_FreeSurface( loadedImage );
	}

	// return the optimized image
	return optimizedImage;
}

int uninit_buttons( struct Button buttons[] ){
	int i, j;
	for ( i = 0; i < BUTTON_NUMBER; i++ ){
		for ( j = 0; j < buttons[i].img_number; j++ ){
			if ( buttons[i].img[j] != NULL ){
				// free the loaded images
				SDL_FreeSurface( buttons[i].img[j] );
				buttons[i].img[j] = NULL;
			}
		}
	}
}

/******************************************************************************/
int play_button_down(){
	printf( "play_button_down\n" );

	play_button->current_img = 1;

	return 0;
}

int play_button_up(){
	printf("play_button_up\n");
	play_button->current_img = 0;

	// travel_musicList( nowList );
	
	// is_playing = 1;
	// return fifo_playList( nowList );

	return play_music( music );
}

/******************************************************************************/
int pause_button_down(){
	printf( "pause_button_down\n" );
	
	pause_button->current_img = 1;

	return 0;
}

int pause_button_up(){
	printf("pause_button_up\n");
	pause_button->current_img = 0;

	if ( has_mplayer() == -1 ){
		fprintf( stderr, "You should play first. Maybe something wrong\n" );
		return -1;
	}

	return fifo_pause();
}

/******************************************************************************/
int pre_music_button_down(){
	printf( "pre_music_button_down\n" );
	pre_music_button->current_img = 1;

	return 0;
}

int pre_music_button_up(){
	printf("pre_music_button_up\n");
	pre_music_button->current_img = 0;

	music = get_pre_music( nowList, music );

	return play_music( music );
}

/******************************************************************************/
int next_music_button_down(){
	printf( "next_music_button_down\n" );
	next_music_button->current_img = 1;

	return 0;
}

int next_music_button_up(){
	printf( "next_music_button_up\n" );
	next_music_button->current_img = 0;

	music = get_next_music( nowList, music );

	return play_music( music );
}

/******************************************************************************/
int quit_button_down(){
	printf( "quit_button_down\n" );
	quit_button->current_img = 1;

	return 0;
}

int quit_button_up(){
	printf( "quit_button_up\n" );
	// quit_button->current_img = 0;
	exit( 0 );
}

/******************************************************************************/
int volume_inc_button_down(){
	printf( "volume_inc_button_down\n" );
	volume_inc_button->current_img = 1;

	return 0;
}

int volume_inc_button_up(){
	printf( "volume_inc_button_up\n" );
	volume_inc_button->current_img = 0;

	if ( has_mplayer() == -1 ){
		printf( "You should play first. Maybe something wrong\n" );
		return -1;
	}

	return fifo_set_volume( 1 );
}

/******************************************************************************/
int volume_dec_button_down(){
	printf( "volume_dec_button_down\n" );
	volume_dec_button->current_img = 1;

	return 0;
}

int volume_dec_button_up(){
	printf( "volume_dec_button_up\n" );
	volume_dec_button->current_img = 0;

	if ( has_mplayer() == -1 ){
		printf( "You should play first. Maybe something wrong\n" );
		return -1;
	}

	return fifo_set_volume( -1 );
}

/******************************************************************************/
int mute_button_down(){
	mute_button->current_img = ( mute_button->current_img + 1 ) % mute_button->img_number;
	return 0;
}

int mute_button_up(){
	mute_button->current_img = ( mute_button->current_img + 1 ) % mute_button->img_number;

	if ( mute_button->current_img == 0 )
		return fifo_mute( 0 ); // stop mute
	else
		return fifo_mute( 1 ); // mute
}

/******************************************************************************/
int mode_button_up(){
	if ( has_mplayer() != -1 ){
		fifo_stop();
	}

	if ( mode == 0){ // 0 --> music
		mode = 1;
		nowList = fmList;
		mode_button->current_img = mode;
	} else { // 1 --> fm
		mode = 0;
		nowList = musicList;
		mode_button->current_img = mode;
	}
	music = nowList->head->next; // first music
	show_music_message();
}
/******************************************************************************/
int help_button_down(){
	return 0;
}

int help_button_up(){
	return 0;
}

/******************************************************************************/
int list_button_down(){
	return 0;
}

int list_button_up(){
	return 0;
}

/******************************************************************************/
int has_mplayer(){
	FILE *p;
	int state = -1;

	if ( ( p = popen( "ps -ef | grep -c player_fifo", "r" ) ) != NULL ){
		int i;
		if ( fscanf( p, "%d", &i ) != -1 and i > 2 )
			state = 1;
		pclose( p );
	}
#ifndef NO_MPLAYER
	return state;
#else
	return 1;
#endif
}

int start_mplayer(){
	pid_t pid = fork();
	if ( pid > 0 ){ // parent
		sleep( 1 ); // wait child
		return 0;
	} else if ( pid == 0 ){ // child
		if ( has_mplayer() == -1 )
			execl( "/usr/bin/mplayer", "mplayer", "-slave", "-nocache", "-novideo", "-input",
				"file=/tmp/player_fifo", "-idle", "-quiet", "-srate", "48000", NULL );
//			execl( "/usr/bin/mplayer", "mplayer", "-slave", "-cache", "1024", "-novideo",
//			"-input", "file=/tmp/player_fifo", "-idle", "-quiet", "-srate", "48000", NULL );
		exit( 0 );
	} else // fork failed
		return -1;
}

int play_music(){
	show_music_message();

	if ( music == NULL )
		return -1;
	if ( has_mplayer() == -1 and start_mplayer() == -1 )
		return -1;

	return fifo_play( music );
}

int show_music_message(){
	static char message[512];
	SDL_Surface * img;
	struct Music * tmpMusic;

	static const SDL_Color mainColor = { 255, 255, 0 };
	static const SDL_Color subColor = { 255, 255, 255 };

	if ( music == NULL ){
		pre_music_message->visible = 0;
		now_music_message->visible = 0;
		next_music_message->visible = 0;
		return -1;
	}

	// show now_music_message
	snprintf( message, sizeof( message ) , "%3d. %s", music->id, music->name );
	img = now_music_message->img[0];
	if ( img != NULL ){
		SDL_FreeSurface( img );
		img = NULL;
	}

	img = TTF_RenderText_Solid( mainFont, message, mainColor );
	now_music_message->img[0] = img;
	now_music_message->visible = ( int ) ( img != NULL );

	// show pre_music_message
	img = pre_music_message->img[0];
	if ( img != NULL ){
		SDL_FreeSurface( img );
		img = NULL;
	}
	if ( tmpMusic = get_pre_music( nowList, music ) ){
		snprintf( message, sizeof( message ) , "%3d. %s", tmpMusic->id, tmpMusic->name );
		img = TTF_RenderText_Solid( subFont, message, subColor );
		pre_music_message->img[0] = img;
	}
	pre_music_message->visible = ( int ) ( img != NULL );

	// show next_music_message
	img = next_music_message->img[0];
	if ( img != NULL ){
		SDL_FreeSurface( img );
		img = NULL;
	}
	if ( tmpMusic = get_next_music( nowList, music ) ){
		snprintf( message, sizeof( message ) , "%3d. %s", tmpMusic->id, tmpMusic->name );
		img = TTF_RenderText_Solid( subFont, message, subColor );
		next_music_message->img[0] = img;
	}
	next_music_message->visible = ( int ) ( img != NULL );

	refresh_screen( main_screen, buttons );
	return 0;
}

void quit(){
	if ( has_mplayer() )
		fifo_quit();

	fifo_close();

	if ( mainFont != NULL )
		TTF_CloseFont( mainFont );

	if ( subFont != NULL )
		TTF_CloseFont( subFont );

	TTF_Quit();

	// uninit buttons
	uninit_buttons( buttons );
	
	SDL_Quit();
}

int welcome(){

	SDL_Surface * welcome_img = load_image( "img/welcome.bmp" );
	SDL_BlitSurface( welcome_img, NULL, main_screen, NULL );
	SDL_Flip( main_screen );

	SDL_Event event;
	while ( 1 ){
		SDL_WaitEvent( &event );
		if ( event.type == SDL_MOUSEBUTTONDOWN ){
			SDL_FreeSurface( welcome_img );
			return 0;
		}
	}
}
