/*---------------------------------------------------------------------------------

	Basic template code for starting a DS app

---------------------------------------------------------------------------------*/
#include <nds.h>
#include <stdio.h>
#include "blackbox.h"
#include "general.h"
#include "graphicseng.h"
#include "granularsampler.h"

#define SAMPLERATE 32768
#define BUFFERSIZE 4096
#define NUMSAMPLES 1

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	WAVheader* af;

	s16* audio;

	int audiosize = 0;

	GranularSampler* soundengine = NULL;

	af = (WAVheader*)malloc( sizeof( WAVheader ) );

	af = loadWAV( "/GrainDS/samples/wavfile.wav" );

	audiosize = ( ( af->DataSize / af->Channels ) >> 1 );

	audio = (s16*)malloc( sizeof( u16 ) * audiosize );

	for( int i = 0; i < audiosize; i++ ) audio[i] = af->Data.data16[i];

	soundengine = new GranularSampler( SAMPLERATE, audiosize, af->SampleRate, audio );

	SoundSetup( soundengine, SAMPLERATE, BUFFERSIZE );


	consoleDemoInit();

	iprintf("SampleRate: %dHz\nBitRes: %dBit\nChannels: %d\nSamples: %d\nMAXPHASE: %d",
			af->SampleRate,
			af->BitsPerSample,
			af->Channels,
			audiosize,
			MAXPHASE>>31);


	while(1) {


		touchPosition touch;

		touchRead( &touch );

		scanKeys();


		if( touch.px + touch.py > 0 ){

			if( keysHeld() & KEY_LEFT ){
				
				soundengine->SetPlaybackRate( touch.px, 128 );

			} else {

				soundengine->SetPlaybackPos( touch.px, 256 );

			}

			if( keysHeld() & KEY_DOWN ){

				soundengine->SetPitch( touch.py, 96 );

			} else {

				soundengine->SetGrainSize( touch.py );

			}

		}


		swiWaitForVBlank();


	}

}
