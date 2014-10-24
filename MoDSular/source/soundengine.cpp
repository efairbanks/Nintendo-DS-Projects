
#include <nds.h>
#include <math.h>
#include "soundengine.h"


SoundEngine::SoundEngine(int sr, int voices){


	this->numvoices = voices;

	this->oscs = (SinOsc**)malloc( sizeof( SinOsc* ) * this->numvoices );

	for( int i = 0; i < this->numvoices; i++ ){

		this->oscs[i] = new SinOsc( sr );

	}


	this->delay = new FeedBackDelay( sr, 1000 );

	this->delay->SetAmplitude( MAXAMP );

	this->delay->SetFeedBack( MAXAMP / 3 );

	this->delay->SetDelay( 300 );

	this->globalBufferCursor=0;
}

void SoundEngine::playnote( int note, int voice ){


	this->oscs[voice]->SetFreqHZ( (int)(pow( 2.0, note / 12.0 ) * 55.0) );

	this->oscs[voice]->SetAmplitude( MAXAMP / this->numvoices );


}


void SoundEngine::stop( int voice ){

	this->oscs[voice]->SetFreqHZ( 0 );

	this->oscs[voice]->SetAmplitude( 0 );	

}

int SoundEngine::Process(){


	int sample = 0;


	for( int i = 0; i < numvoices; i++ )

		sample += this->oscs[i]->Process();


	this->delay->Signal( sample );

	sample = ( this->delay->Process() * 3 ) >> 2;

	sample = sample*4/5;

	this->globalBuffer[this->globalBufferCursor] = sample;

	this->globalBufferCursor = (this->globalBufferCursor+1)%GLOBALBUFFERSIZE;

	return sample;
}
