
#ifndef GENERAL_H
#define GENERAL_H

#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <maxmod9.h>
#include "blackbox.h"


typedef struct {


	char RIFF[4]; // "RIFF"

	u32 ChunkSize;

	char WAVE[4]; // "WAVE"

	char fmt[4];  // "fmt "

	u32 SubChunkSize; // 16

	u16 Format; // 1

	u16 Channels;

	u32 SampleRate;

	u32 BytesPerSecond;

	u16 BlockAlign;

	u16 BitsPerSample;

	char data[4]; // "data"

	u32 DataSize;
  
  
  union {
  
	u16* data16;
	u8*  data8;
  
  } Data;


} WAVheader;


WAVheader* loadWAV( char* filename );

void SoundSetup( BlackBox* sndeng, int srate, int bufsize );

int ipower( int var, int power );

bool recttest( int x, int y, int rx, int ry, int rwidth, int rheight );


#endif