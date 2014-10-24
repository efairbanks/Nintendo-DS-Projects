
#include "blackbox.h"


class SoundEngine : public BlackBox {

public:

	SoundEngine( int sr );
	
	int Process();
	
	void NoteOn( int note );
	
	void SetModFreqMul( int freq );
	
	void SetModAmt( int amt );
	
private:

	SinOsc* osc;
	
	SinOsc* modulator;
	
	Line* line;
	
	int freq;
	
	int modamt;
	
	int modfreq;

	FeedBackDelay* delay;

};