
#include <nds.h>
#include <math.h>
#include "soundengine.h"


SoundEngine::SoundEngine(int sr ){


	this->osc = new SinOsc( sr );
	
	this->osc->SetAmplitude( 32767>>2 );
	
	this->modulator = new SinOsc( sr );

	this->delay = new FeedBackDelay( sr, 233 );

	this->delay->SetAmplitude( MAXAMP );

	this->delay->SetFeedBack( (MAXAMP * 1 / 3) );

	this->delay->SetDelay( 333 );
	
	this->line = new Line( sr );
	
	this->line->SetStartVal( MAXAMP );
	
	this->line->SetEndVal( 0 );
	
	this->line->SetLengthMS( 666 );
	
	this->line->SetDelayMS( 0 );
	
	this->modamt = 0;
	
	this->modfreq = 0;
	
	

}

void SoundEngine::NoteOn( int note ){

	this->freq = (int)(27.5*pow( 2.0, note/12.0 ));

	this->line->Reset();

}

void SoundEngine::SetModFreqMul( int multiplier ){

	this->modfreq = this->freq*multiplier;
	
	this->modulator->SetFreqHZ( this->modfreq );
	
}
	
void SoundEngine::SetModAmt( int amt ){

	this->modamt = amt;
	
	this->modulator->SetAmplitude( this->modamt );
	
}

int SoundEngine::Process(){


	int sample = 0;
	
	int line = this->line->Process();
	
	this->osc->SetFreqHZ( this->modulator->Process() + this->freq );
	
	this->SetModAmt(line>>5);

	sample = this->osc->Process();
	
	sample = (sample * line) >> 15;


	//this->delay->Signal( sample>>7 );

	//sample = this->delay->Process();

	return sample;


}