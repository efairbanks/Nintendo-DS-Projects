
#ifndef BLACKBOX
#define BLACKBOX


#define MAXAMP	 0x7FFF
#define MAXPHASE 0x7FFFFFFF


class BlackBox{
	
	public:
	
		virtual int Process() = 0;
	
};

class Line : public BlackBox{

	public:
		
		Line( int samplerate );
		
		int Reset();
	
		int Stop();
	
		int SetStartVal( int start );
	
		int SetEndVal( int start );
	
		int SetDelayMS( int delay );
	
		int SetLengthMS( int length );
		
		int Process();
	
	private:
		
		int sr;
		
		int start;
		int end;
		
		unsigned int samps;
		unsigned int delay;
		unsigned int length;
		unsigned int phase;
		unsigned int delta;
	
};

class ADSR : public BlackBox{

	public:
	
		ADSR( int samplerate );
	
		int Reset();
	
		int Stop();
	
		int SetAttackMS( int atk );
	
		int SetDecayMS( int dcy );
	
		int SetSustainMS( int sus );
	
		int SetReleaseMS( int rls );
	
		int Process();
	
	private:
	
		int sr;
	
		unsigned int phase;
	
		int attack;
		int decay;
		int sustain;
		int release;

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
	
	int SetDelayHZ( int hz );

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
