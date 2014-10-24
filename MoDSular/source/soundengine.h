
#include "blackbox.h"
#define GLOBALBUFFERSIZE 3000

class SoundEngine : public BlackBox {

public:

	SoundEngine( int sr, int numvoices );
	
	void playnote( int note, int voice );

	void stop( int voice );

	int Process();

	int globalBuffer[GLOBALBUFFERSIZE];
	int globalBufferCursor;
	
private:

	int numvoices;

	SinOsc** oscs;

	FeedBackDelay* delay;

};
