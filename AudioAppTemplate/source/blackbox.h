
#ifndef BLACKBOX
#define BLACKBOX


#define MAXAMP	 0x7FFF
#define MAXPHASE 0x7FFFFFFF


class BlackBox{

	
	public:
	
	virtual int Process() = 0;
	

};


class Oscillator : public BlackBox{

	
	public:
	
	virtual int Process() = 0;
	
	int SetSampleRate( int sr );
	
	int SetAmplitude( int amp );
	
	int SetFreqHZ( int hz );
	
	int SetFreqMS( int ms );
	
	int SetPhase( int p );
	
	
	protected:
	
	unsigned int samplerate;
	
	unsigned int delta;
	
	unsigned int phase;
	
	int amplitude;
	

};


class FeedBackDelay : public BlackBox{

	
public:

	FeedBackDelay( int sr, int maxdelay );

	int SetDelay( int ms );

	int SetFeedBack( int fb );

	int SetAmplitude( int amp );

	int Signal( int sig );

	int Process();


private:

	int signal;

	int* buffer;

	int  buffersize;

	int  feedback;

	int  amplitude;

	int  samplerate;

	int  delta;

	int  playcursor;


};


class Phasor : public Oscillator{

	
	public:
	
	Phasor( int sr );
	
	int Process();
	

};


class SinOsc : public Oscillator{


	public:

	SinOsc( int sr );

	int Process();


};


#endif