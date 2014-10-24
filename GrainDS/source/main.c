
// GrainDS -- Phate

/*

DISCLAIMER: This code is absolute and utter crap. View, use, and modify at your own risk. There is
a very real chance that reading this source code will make you worse at programming.

Compiled with devkitPro Updater v1.5.0 on Windows XP SP3

*/

#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fat.h>
#include <maxmod9.h>


#define SAMPLERATE	22000 	// This is a little define for the sample rate so I can change it on the fly if need be.
#define BUFFERSIZE	1200 	// Same here.
#define MAXSDIVS	85		// Also here.

// ----- //

typedef struct{

	u32 ChunkID;
	u32 ChunkSize;
	u32 Format;
	u32 Subchunk1ID;
	u32 Subchunk1Size;
	
	u16 AudioFormat;
	u16 NumChannels;
	
	u32 SampleRate;		// This is a structure for holding
	u32 ByteRate;		// the header info for a WAV file.
						// It is set up so that you can simply
	u16 BlockAlign;		// load the first 44 bytes of header
	u16 BitsPerSample;	// directly into it, then read the data
						// out of it with no hassle.
	u32 Subchunk2ID;
	u32 Subchunk2Size;
	
	u32 index;
	s16* data;

} waveheader;

// ----- //

typedef struct{

	char* name;
	
	struct dirlist* next;

} dirlist;

// ----- //

u16 bpm 		= 175;			// Global variables for volume and BPM.
u16 vol			= 190;			// Fairly self explanatory. They have
u8  lp			= 0;			// to be global so they can be accessed
u8	pitch		= 99;			// by the callback function. ( I'll
bool interp		= 0;			// explain what that means in a bit )

bool stoggle	= ~0;

waveheader* audio;				// This is a global pointer for the wav
								// file that we're going to load. Again,
								// it has to be global so that the callback
								// function can access it.

u32 globms 		= 0;		// Number of milliseconds passed since the
u8  shift		= 0;		// beginning of the program.

u32	rev			= 0;

u8  gindex		= 0;
u8	cursindex	= 0;
u8	lstsindex	= 0;

u8	sampledivs	= 8;

u8  seq[MAXSDIVS];
u8	sav[4][MAXSDIVS + 1];


// ----- //

/*

These are function declarations. I could've defined the functions
here as well, but I decided to define them below the main function
so that my code would be a bit more readable.

custommminit is a little wrapper function I wrote to simplify setting
up MaxMod, which is the library I'm using for streaming sound. If you
really want to understand why I do what I do here, I suggest reading
the MaxMod documentation. Basically all this does is tell MaxMod that
I will only be using the sound streaming features, sets up a streaming
sound channel, and gives that channel my callback function.

stream_callback is the callback function that MaxMod needs for streaming
sound stuff. Basically, there's a buffer that needs to be constantly filled
with sound data, and so MaxMod has you define a function with specific
arguments. You then hand that function to MaxMod when you set up your sound
stream, and it calls that function each time it needs to fill the buffer.

*/

mm_word	stream_callback( mm_word length, mm_addr dest, mm_stream_formats format );

void	custommminit();
void	globmscallback();

int		bpmtos(		int bp		);
int		calcshift(	int input	);
int 	calcsindex( int shft, waveheader* aud, int bp	);

waveheader* loadwav();

void writepixel( u16 x, u16 y, u8 color, u8* video );
void drawrect( u16 xoff, u16 yoff, u16 width, u16 height, u16 color, u8* video );
void drawbar ( 	u16 numcols, u16 numrows, u16 xpadding, u16 ypadding, u16 colnum,	\
				u16 rownum, u16 screenwidth, u16 screenheight, u16 screenxoff,		\
				u16 screenyoff, u16 percentfilledx, u16 percentfilledy, u16 color, \
				u8* video );

// ----- //


//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	touchPosition touch;

	u16* vBuf;
	u16* vBufSub;
	
	u8* backbuffer = malloc( 192*256 );
	
	memset( backbuffer, 0, 192*256 );


	videoSetMode( MODE_5_2D );
	videoSetModeSub( MODE_5_2D );

	vramSetBankA( VRAM_A_MAIN_BG );
	vramSetBankC( VRAM_C_SUB_BG );

	PrintConsole topScreen = *consoleInit( 0, 0,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, 1 );
	consoleSelect(&topScreen);

	vBuf = bgGetGfxPtr( bgInit( 3, BgType_Bmp8, BgSize_B8_256x256, 4, 0 ) );
	vBufSub = bgGetGfxPtr( bgInitSub( 3, BgType_Bmp8, BgSize_B8_256x256, 0, 0 ) );
	
	bgSetPriority( 0, 0 );
	bgSetPriority( 3, 3 );
	

	lcdMainOnBottom();
	
	
	int i = 0, j = 0, k = 0, l = 0;
	
	for( l = 0; l < sampledivs; l++ ) seq[l] = l;
	
	
	BG_PALETTE[0] = 0;	
	for( i = 1; i < 32; i++ ) BG_PALETTE[i]		= RGB15( 9+i/2, i*2/3+9, i*2/3+9 ); // Set up main palette

	BG_PALETTE_SUB[0]	= 0;
	BG_PALETTE_SUB[1]	= RGB15( 31, 22, 26 ); // Set up sub palette
	BG_PALETTE_SUB[2]	= RGB15( 30, 27, 26 );
	BG_PALETTE_SUB[3]	= RGB15( 24, 30, 28 );
	
	memset( vBuf, 		0, 192*256 );
	memset( vBufSub,	0, 192*256 );


	// ----- //
	
	/*
	
	Right here I load the first 44 bytes (the header) of the WAV file named "wavfile.wav"
	into my header struct. (after I allocate data for it) Once I've done this, the header
	tells me how large the WAV file is, and I can allocate that much more data and
	continue reading the remainder of the file.
	
	The reason I have a global pointer to a wavheader struct instead of just having a
	global wavheader struct is that for whatever reason, there is only so much space
	allowed for globals, and so the DS has a fit if you try to do so. If you instead
	malloc it afterwards and set the global pointer to that memory, it doesn't complain.
	
	Anyways, just basic file loading. Google the WAV file specs for more detail.
	
	*/
	
	
	while( ( audio = loadwav() ) == NULL ){
	
		swiWaitForVBlank();
		
		consoleClear();
		
		iprintf("\x1b[11;0H        FILESYSTEM ERROR\n" );
	
	}
	
	consoleClear();
	
	iprintf("BULLSHIT");
	
	if( audio != NULL ) bpm = (240*(audio->SampleRate)*(audio->NumChannels))/(audio->Subchunk2Size>>(audio->BitsPerSample/16));
	
	
	// ----- //
	
	
	custommminit();		// This sets up MaxMod and gives it a pointer to our callback function. See below for more info.


	while(1) {
	
	// ----- //
	
	/*
	
	The main loop. Uses the console included in libnds to print out data about what we're doing.
	Also updates the global variables according to touch screen data so we can interact with out program.
	swiWaitForVBlank() keeps us from using crap-tons of unneccessary power.
	
	*/
		
		scanKeys();
		
		touchRead(&touch);
		int keys  = keysDown();
		int keysr = keysDownRepeat();
		int held  = keysHeld();
		int kup   = keysUp();
		
		if( held & KEY_Y ){
			
			iprintf("\x1b[1;0H\n");
			iprintf("  Touch x =         %04i, %04i\n", touch.rawx, touch.px);			// For keeping track
			iprintf("  Touch y =         %04i, %04i\n", touch.rawy, touch.py);			// of things. Some of
			iprintf("  Filesize:           %5d KB\n", audio->Subchunk2Size >> 10 );	// this stuff is obsolete.
			iprintf("  Srate:              %5d HZ\n", audio->SampleRate );
			iprintf("  BPM:                %8d\n"  , bpm );
			iprintf("  LoopTime:           %8d\n"  , bpmtos(bpm)/sampledivs );
			iprintf("  Overflow            %8d\n"  , shift );
			iprintf("  Grain Index:        %8d\n"  , gindex );
			iprintf("  NumGrains:          %8d\n"  , sampledivs );
			iprintf("  Interpolation:      %8d\n"  , interp );
			iprintf("  Interp Amplitude    %8d\n"  , (sinLerp( audio->index*16384/(bpmtos(bpm)/sampledivs) ) ) );
			iprintf("\n\n");
			iprintf("  Hold L button and use touch\n");
			iprintf("  pad to change volume and BPM.\n");
			iprintf("  Left D-Pad is reverse. Hold\n");
			iprintf("  right on D-Pad and use touch\n");
			iprintf("  screen to adjust pitch and\n");
			iprintf("  up and down control the\n");
			iprintf("  number of grains.\n");
		
		} if( kup & KEY_Y ) consoleClear();
		
		if( held & KEY_L ){
		
			
			if( touch.px != 0	) bpm = touch.px;
			if( touch.py != 0	) vol = 191 - touch.py;
			
			
			if( keys & KEY_UP		){								// This updates the sequencer grid
																	// and keeps everything up to date.
				memcpy( seq, sav[0], MAXSDIVS );					// 
																	// ...I think...
				gindex = gindex * sav[0][MAXSDIVS] / sampledivs;
				
				sampledivs = sav[0][MAXSDIVS];
				
			
			}
			
			if( keys & KEY_DOWN		){
			
				memcpy( seq, sav[1], MAXSDIVS );
				
				gindex = gindex * sav[1][MAXSDIVS] / sampledivs;
				
				sampledivs = sav[1][MAXSDIVS];
			
			}
			
			if( keys & KEY_LEFT		){
			
				memcpy( seq, sav[2], MAXSDIVS );
				
				gindex = gindex * sav[2][MAXSDIVS] / sampledivs;
				
				sampledivs = sav[2][MAXSDIVS];
			
			}
			
			if( keys & KEY_RIGHT	){
			
				memcpy( seq, sav[3], MAXSDIVS );
				
				gindex = gindex * sav[3][MAXSDIVS] / sampledivs;
				
				sampledivs = sav[3][MAXSDIVS];
			
			}
			
		
		}
		
		
		if( held & KEY_R ){
		
			if( keys & KEY_UP		){
			
				memcpy( sav[0], seq, MAXSDIVS );
				
				sav[0][MAXSDIVS] = sampledivs;
			
			}
			
			if( keys & KEY_DOWN		){
			
				memcpy( sav[1], seq, MAXSDIVS );
				
				sav[1][MAXSDIVS] = sampledivs;
			
			}
			
			if( keys & KEY_LEFT		){
			
				memcpy( sav[2], seq, MAXSDIVS );
				
				sav[2][MAXSDIVS] = sampledivs;
			
			}
			
			if( keys & KEY_RIGHT	){
			
				memcpy( sav[3], seq, MAXSDIVS );
				
				sav[3][MAXSDIVS] = sampledivs;
			
			}
		
		}
		
		
		if( !(held & KEY_L) && !(held & KEY_R) ){
		
			if( keys  & KEY_START	) stoggle = ~stoggle;
			if( keys  & KEY_A		) audio->index = 0;
			if( keys  & KEY_B		) interp = ~interp;
			if( held  & KEY_LEFT	) rev = 1;
			else				  	  rev = 0;
			if( keysr & KEY_UP		){
			
				if( ++sampledivs >= MAXSDIVS ) sampledivs = 1;
				
				seq[sampledivs - 1] = seq[sampledivs - 2];
			
			}
			
			if( keysr & KEY_DOWN	){
			
				if( --sampledivs >= MAXSDIVS || sampledivs == 0 ) sampledivs = MAXSDIVS - 1;
				
				for( i = 0; i < sampledivs; i++ ) if( seq[i] >= sampledivs ) seq[i] = (sampledivs - 1);
			
			}
		
		}
		
		
		if( keys & KEY_SELECT ) for( i = 0; i < sampledivs; i++ ) seq[i] = i;
		
		
		if( held & KEY_RIGHT ){
		
			if( touch.px != 0	) pitch = touch.px*100/256;
		
		}
		
		
		if( ( touch.px > 0 || touch.py > 0 ) && !( held & KEY_L ) && !( held & KEY_RIGHT ) ){
		
			int curval = ((191-touch.py - 32) * sampledivs / 160);
			
			if( curval < 0 ) curval = 0;
			
			seq[touch.px*sampledivs/256] = curval;

		}
		
		
		drawrect( 0, 0, 256, 192, 0, backbuffer );
		
		if( !(held & KEY_Y) ){
		
			for( k = 0; k < sampledivs; k++ ) drawbar( sampledivs, 1 , 1, 1, k, 0, 256 , 160, 0, 32, 100, (seq[k]+1)*100/sampledivs, 31*seq[k]/sampledivs+1, backbuffer );	// Draw seq
			
			drawbar( 1, 1, 1, 1, 0, 0, 256, 32, 0, 0, ((audio->index)+(gindex*(bpmtos(bpm)/sampledivs)))*100/(bpmtos(bpm)), 100, 31*seq[gindex]/sampledivs+1, backbuffer ); 												// Draw Gindex
			
		
		}
		
		swiWaitForVBlank();
		
		dmaCopy( backbuffer, vBuf, 256*192 );
		
		
		drawrect( 0, 0, 256, 192, 0, backbuffer );
		
		drawbar( 1, 1, 1, 1, 0, 0, 85, 192, 0	, 0, 100, pitch,		1, backbuffer );															// Draw pitch
		drawbar( 1, 1, 1, 1, 0, 0, 86, 192, 85	, 0, 100, bpm*100/256,	2, backbuffer );															// Draw BPM
		drawbar( 1, 1, 1, 1, 0, 0, 85, 192, 171, 0, 100, vol*100/191,	3, backbuffer );															// Draw Vol
		
		swiWaitForVBlank();
		
		dmaCopy( backbuffer, vBufSub,	256*192*2 );
		
		
		/*
		void drawbar( 	u8 numcols, u8 numrows, u8 xpadding, u8 ypadding, u8 colnum,
				u8 rownum, u8 screenwidth, u8 screenheight, u8 screenxoff,
				u8 screenyoff, u8 percentfilledx, u8 percentfilledy, u16 color,
				u16* video )
		*/
	
	}

	// ----- //

	return 0; // Hopefully our program never gets here, otherwise something went horribly, horribly wrong.
}


// ----- //


mm_word stream_callback( mm_word length, mm_addr dest, mm_stream_formats format ) {


	s16 *target = dest;

	int len = length;


	if( stoggle ){
	
	shift = calcshift(bpm*pitch/100) / sampledivs;	// Helps prevent integer overflow because it is not fun.
	
	
	for( ; len; len-- ){
		
		/*
		
		There's some fancy-pants integer math going on here, but basically I just run through an index value that's max
		value is determined by the BPM you want. I had to do some bitshifting to avoid integer overflows, and this resulted
		in a compromise in sound quality, but this shouldn't be an issue with a granular system. I sure wish that the DS
		supported hardware floating point calculations.
		
		I don't remember how accurate this description is anymore. I probably should've used fixed-point
		math here, and I don't think I did. The GrainDS audio engine is slightly retarded. I have different
		statements for whether or not sin/cos interpolation is on or not. Get ready for more horribly inefficient
		code.
		
		*/
		
		
		int sample;
		
		
		if( !(interp > 0) ){
		
		
			if( audio->index >= (bpmtos(bpm) / sampledivs) ){
			
				audio->index = 0;
				
				
				gindex++;
				
				if( gindex >= sampledivs ) gindex = 0;
			
			}
			
			
			if( !rev ) sample = audio->data[ calcsindex(shift, audio, bpm*pitch/100) + ((audio->Subchunk2Size/2)*seq[gindex]/sampledivs) ] * vol / 192;
			else sample = audio->data[ ((audio->Subchunk2Size/2)/sampledivs) - calcsindex(shift, audio, bpm*pitch/100) + ((audio->Subchunk2Size/2)*seq[gindex]/sampledivs) ] * vol / 192;
			
			
			*target++ = sample; // left sample
			*target++ = sample; // right sample
		
		
		}
		
		
		if( interp > 0 ){


			int samp1, samp2, locsindex;
			
			
			if( audio->index >= (bpmtos(bpm) / sampledivs) ){
			
				audio->index = 0;
				
				
				lstsindex = cursindex + 1;
				
				if( lstsindex >= sampledivs ) lstsindex = 0;
				
				
				cursindex = seq[gindex++];
				
				if( gindex >= sampledivs ) gindex = 0;
			
			}
			
			if( cursindex == lstsindex ) globms = 1;
			
			locsindex = calcsindex(shift, audio, bpm*pitch/100);
			
			samp1 = audio->data[ locsindex + ((audio->Subchunk2Size>>1)*lstsindex/sampledivs) ];
			
			samp2 = audio->data[ locsindex + ((audio->Subchunk2Size>>1)*cursindex/sampledivs) ];
			
			
			int interpvol1		= sinLerp( ((audio->index) * 8192)/(bpmtos(bpm)/sampledivs) );
			int interpvol2		= (1<<12) - interpvol1;
			
			samp1 = samp1 * interpvol1>>12;
			samp2 = samp2 * interpvol2>>12;
			
			
			sample = (samp1 + samp2) * vol / 192;
			
			
			*target++ = sample; // left sample
			*target++ = sample; // right sample
		
		
		}
	
	
	}
	
	} else for( ; len; len-- ){ *target++ = 0; *target++ = 0; }
	
	return length;
	
}


// ----- //


void custommminit(){

	mm_ds_system* sys = malloc( sizeof( mm_ds_system ) );		// This basically just tells MaxMod
	sys->mod_count 			= 0;								// that we don't want to use any special
	sys->samp_count			= 0;								// features, just streaming sound. For
	sys->mem_bank			= 0;								// whatever reason, MaxMod uses little
	sys->fifo_channel		= FIFO_MAXMOD;						// structures to set up all of it's stuff,
	mmInit( sys );												// then you hand the initialization functions
																// a pointer to the structure and it sets
																// everything up for you.


																// This is the same crap as above, but
																// for setting up a stream this time
																// around.
	mm_stream* mystream = malloc( sizeof( mm_stream ) );
	mystream->sampling_rate	= SAMPLERATE;						// sampling rate = 22khz
	mystream->buffer_length	= BUFFERSIZE;						// buffer length = 800 samples
	mystream->callback		= stream_callback;					// set callback function
	mystream->format		= MM_STREAM_16BIT_STEREO;			// format = stereo 16-bit
	mystream->timer			= MM_TIMER0;						// use hardware timer 0
	mystream->manual		= false;							// use automatic filling
	mmStreamOpen( mystream );									// Open the stream

}


// ----- //


/*

These are just a set of functions for doing random stuff related to my terribly inefficient
and confusing pseudo-audio-engine.

*/


int bpmtos( int bpm ){

	return SAMPLERATE*240/bpm;

}


// ----- //


int calcshift( int bp ){

	return ( (audio->Subchunk2Size >> 9) * (bpmtos(bp) >> 8) ) >> 16;

}


// ----- //


int calcsindex( int shft, waveheader* aud, int bp ){

	return ((aud->index++ >> shft) * (aud->Subchunk2Size>>1) / bpmtos(bp)) << shft;

}


// ----- //


waveheader* loadwav(){


	FILE* infile;
	
	waveheader* wavfile;
	
	DIR *pdir;
	struct dirent *pent;
	struct stat statbuf;

	char* curstring;
	dirlist* dl		= NULL;
	dirlist* dlinc	= NULL;


	if( fatInitDefault() ){	// Initialize libfat so that we can read from the filesystem

		int keys = 0;
		
		pdir=opendir("/GrainDS/samples/");
		
		chdir("/GrainDS/samples/");
		
		if (pdir){
		
			while ((pent=readdir(pdir))!=NULL) {
			
				stat(pent->d_name,&statbuf);
				
				if(strcmp(".", pent->d_name) == 0 || strcmp("..", pent->d_name) == 0) continue;
				
/*				if(S_ISDIR(statbuf.st_mode)){
				
					dirlist* newdl;
					
					newdl = malloc( sizeof( dirlist) );
					
					newdl->name = malloc( sizeof( pent->d_name ) + 1 );
					
					memset( newdl->name, 0, sizeof( pent->d_name ) );
					
					strcpy( newdl->name + 1, pent->d_name );
					
					newdl->name[0] = 49;
					
					newdl->next = dl;
					
					dl = newdl;
				
				}
*/				
				if(!(S_ISDIR(statbuf.st_mode))){
				
					dirlist* newdl;
					
					newdl = malloc( sizeof( dirlist) );
					
					memset( newdl, 0, sizeof( dirlist ) );
					
					newdl->name = malloc( sizeof( pent->d_name ) + 1 );
					
					memset( newdl->name, 0, sizeof( pent->d_name ) );
					
					strcpy( newdl->name + 1, pent->d_name );
					
					newdl->name[0] = 48;
					
					newdl->next = dl;
					
					dl = newdl;
				
				}
			
			}
			
			
			dlinc = dl;
			
			
			while( dlinc != NULL ){
			
				if( dlinc->name != NULL ) iprintf( "%s\n", dlinc->name );
				
				dlinc = dlinc->next;
			
			}
			
			
			dlinc = dl;
			
			
			while( !(keys & KEY_START) ){
			
				scanKeys();
				
				keys = keysDown();
				
				if( keys & KEY_SELECT ){
				
					dirlist* templist;
					
					consoleClear();
					
					dlinc = dlinc->next;
					
					if( dlinc == NULL ) dlinc = dl;
					
					templist = dlinc;
					
					while( templist != NULL ){
					
						if( templist->name != NULL ) iprintf( "%s\n", templist->name );
						
						templist = templist->next;
					
					}
				
				}
				
				swiWaitForVBlank();
			
			}
			
			
			consoleClear();
			
			
		} else {
		
			iprintf ("\x1b[11;0H        FILESYSTEM ERROR\n");
		
		}

	
	} else {
	
		iprintf("\x1b[11;0H        FILESYSTEM ERROR\n");
	
	}


	//curstring = malloc( dlinc->name + 17 );
	
	//strcpy( curstring, "/GrainDS/samples/" );
	
	//strcpy( curstring + 16, dlinc->name + 1 );
	
	
	if( (infile = fopen( dlinc->name + 1, "rb" )) != NULL ){
	
		
		if( ( wavfile = malloc( sizeof(waveheader) ) ) == NULL ) return NULL;
		
		memset( wavfile, 0, sizeof(waveheader) );
		
		fread( wavfile, 1, 44, infile );
		
		if( ( wavfile->data = malloc( wavfile->Subchunk2Size ) ) == NULL ) return NULL;
		
		fread( wavfile->data, 1, wavfile->Subchunk2Size, infile );
		
		return wavfile;
		
	}
	
	closedir(pdir);
	
	return NULL;

}


// ----- //


/*

Drawing functions. Fairly self-explanatory.

*/


void writepixel( u16 x, u16 y, u8 color, u8* video ){

	video[ ((192-y)*256) + x ] = color;

}


// ----- //


void drawrect( u16 xoff, u16 yoff, u16 width, u16 height, u16 color, u8* video ){

	int i, j;
	
	for( i = yoff; i < (yoff + height); i++ )
	
		for( j = xoff; j < (xoff + width); j++ )
		
			writepixel( j, i, color, video );

}


// ----- //


void drawbar( 	u16 numcols, u16 numrows, u16 xpadding, u16 ypadding, u16 colnum,	\
				u16 rownum, u16 screenwidth, u16 screenheight, u16 screenxoff,		\
				u16 screenyoff, u16 percentfilledx, u16 percentfilledy, u16 color, \
				u8* video ){

	drawrect( (screenwidth*colnum/numcols) + xpadding + screenxoff, (screenheight*rownum/numrows) + ypadding + screenyoff, \
			  ((screenwidth/numcols)  - (xpadding*2))*percentfilledx/100, ((screenheight/numrows) - (ypadding*2))*percentfilledy/100, color, video );

}

// ----- //


/*

Some debug info:

If SampPos ever goes back to zero before Index, it means that
there's an overflow going on in the sample-lookup, which is
going to cause significant artifacting. This is a problem.
With the current settings, BPMs down to about 25 should be
fairly accurate, but anything lower is fair game for audio.
nastiness.

I don't even remember what I was talking about here. I think
that it's just explaining a problem related to me being functionally
retarded when it comes to writing code on platforms without floating
point arithmetic. I've (luckilly) discovered better ways of dealing
with this, but GrainDS was a huge hack job, since I wasn't very
familiar with using fixed-point arithmetic at the time.

*/
