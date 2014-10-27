#include <nds.h>
#include <math.h>
#include "soundengine.h"

SoundEngine::SoundEngine(int sr){
	this->numvoices = 1;
	this->oscs = (SinOsc**)malloc( sizeof( SinOsc* ) * this->numvoices );
	for( int i = 0; i < this->numvoices; i++ ){
		this->oscs[i] = new SinOsc( sr );
	}
	this->granulizerBufferSize=sr;
	this->granulizerBuffer=(s16*)malloc(sizeof(s16)*this->granulizerBufferSize);
	this->granulizer = new GranularSampler(sr,this->granulizerBufferSize,sr,this->granulizerBuffer);
	for(int i=0; i<this->granulizerBufferSize; i++) {
		this->granulizerBuffer[i]=(((i*(i/5000)*110)%MAXAMP)*((i*4)%MAXAMP))/(MAXAMP*2);
		this->granulizerBuffer[i]+=((rand()%MAXAMP)*(MAXAMP-((i*12)%MAXAMP)))/(MAXAMP*3);
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
	sample = (sample)/2;
	sample = sample + (this->granulizer->Process());
	this->globalBuffer[this->globalBufferCursor] = sample;
	this->globalBufferCursor = (this->globalBufferCursor+1)%GLOBALBUFFERSIZE;
	
	sample = sample*4/50;
	return sample;
}
