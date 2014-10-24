
#include "blackbox.h"
#include <nds.h>
#include <maxmod9.h>
#include <stdio.h>

// --- //

Line::Line( int samplerate ){
	
	this->sr = samplerate;
	
	this->start = 0;
	
	this->end = 0;
	
	this->delay = 0;
	
	this->samps = 0;
	
	this->length = 0;
	
	this->phase = 0;
	
	this->delta = 0;

}
	
int Line::Reset(){
	
	this->samps = 0;
	
	this->phase = 0;
	
	return 0;
	
}

int Line::Stop(){
	
	this->phase = 0x7fffffff;
	
	return 0;
	
}

int Line::SetStartVal( int start ){
	
	this->start = start;
	
	return 0;
	
}

int Line::SetEndVal( int end ){
	
	this->end = end;
	
	return 0;
	
}

int Line::SetDelayMS( int delay ){
	
	this->delay = ( this->sr * delay ) / 1000;;
	
	return 0;
	
}

int Line::SetLengthMS( int length ){
	
	this->delta = ((MAXPHASE/this->sr)*1000)/length;
	
	return 0;
	
}

int Line::Process(){
	
	/*
	int i = this->samps - this->delay;
	
	int out = ((this->start*(this->length-i))/this->length);
	
	out += (this->end*i)/this->length;
	
	if( this->samps < this->delay ) out = this->start;
	
	if( this->samps < this->length + this->delay ) this->samps++;
	*/
	
	int out = this->start;
	
	if( this->samps >= this->delay ){
		
		int i = this->phase >> 16;
		
		out = (((1<<15)-i)*this->start)>>15;
		
		out += (i*this->end)>>15;
		
		//out = (i*192)>>16;
		
	} else {
		
		this->samps++;
		
	}
	
	if( this->phase < MAXPHASE ) this->phase += this->delta;
	
	return out;
	
}

// --- //

ADSR::ADSR( int samplerate ){};

int ADSR::Reset(){return 0;};

int ADSR::Stop(){return 0;};

int ADSR::SetAttackMS( int atk ){return 0;};

int ADSR::SetDecayMS( int dcy ){return 0;};

int ADSR::SetSustainMS( int sus ){return 0;};

int ADSR::SetReleaseMS( int rls ){return 0;};

int ADSR::Process(){return 0;};

// --- //

int Oscillator::SetFreqHZ( int hz ){


	this->delta = (MAXPHASE/this->samplerate)*hz;
	
	return this->delta;
	

}

	
int Oscillator::SetFreqMS( int ms ){


	this->delta = (MAXPHASE/((this->samplerate*ms)/1000));
	
	return this->delta;


}


int Oscillator::SetAmplitude( int amp ){

	
	this->amplitude = amp;
	
	return this->amplitude;
	

}


int Oscillator::SetPhase( int p ){

	
	this->phase = p;
	
	return this->phase;
	

}


Phasor::Phasor( int sr ){


	this->samplerate = sr;

	this->phase = 0;

	this->amplitude = 0;

	this->delta = 0;


}

int Phasor::Process(){


	int sample = 0;


	this->phase += this->delta;
	
	while( this->phase >= MAXPHASE ) this->phase -= MAXPHASE;

	sample = ((this->phase>>16)*amplitude)>>15;

	
	return sample;


}


FeedBackDelay::FeedBackDelay(int sr, int maxdelay){

	this->samplerate = sr;

	this->buffersize = (sr*maxdelay)/1000;

	this->amplitude  = MAXAMP;

	this->buffer     = (int*)malloc( this->buffersize*sizeof(int) );

	memset( this->buffer, 0, this->buffersize*sizeof(int) );

	this->delta = 0;

	this->feedback = 0;

	this->playcursor = 0;

}

int FeedBackDelay::Process(){

	int out = 0;

	out = this->signal + this->buffer[this->playcursor];
	
	out = out > MAXAMP ? MAXAMP : out;
	
	out = out < -MAXAMP ? -MAXAMP : out;

	this->buffer[this->playcursor] = ( out * this->feedback ) >> 15;

	this->playcursor++;

	while( this->playcursor > delta ) this->playcursor -= delta;

	return ( out * this->amplitude ) >> 15;

}

int FeedBackDelay::Signal( int sig ){

	this->signal = sig;

	return this->signal;

}

int FeedBackDelay::SetAmplitude( int amp ){

	this->amplitude = amp;

	return this->amplitude;

}

int FeedBackDelay::SetDelay( int ms ){

	this->delta = (this->samplerate*ms)/1000;

	return this->delta;

}

int FeedBackDelay::SetDelayHZ( int hz ){

	this->delta = (this->samplerate/hz);
	
	return this->delta;

}

int FeedBackDelay::SetFeedBack( int fb){

	this->feedback = fb;

	return this->feedback;

}


SinOsc::SinOsc( int sr ){


	this->samplerate = sr;

	this->phase = 0;

	this->amplitude = 0;

	this->delta = 0;

}

int SinOsc::Process(){


	int sample = 0;


	this->phase += this->delta;

	while( this->phase >= MAXPHASE ) this->phase -= MAXPHASE;

	sample = ( sinLerp( (this->phase >> 15) - 32768 ) * this->amplitude ) >> 12;


	return sample;


}
