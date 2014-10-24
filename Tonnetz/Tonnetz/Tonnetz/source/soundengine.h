
#include "blackbox.h"


class SoundEngine : public BlackBox {

public:

	SoundEngine( int sr, int numvoices );
	
	void playnote( int note, int voice );

	void stop( int voice );

	int Process();

private:

	int numvoices;

	SinOsc** oscs;

	FeedBackDelay* delay;

};