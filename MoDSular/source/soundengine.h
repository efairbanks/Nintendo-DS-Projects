#include "blackbox.h"
#include "granularsampler.h"
#define GLOBALBUFFERSIZE 3000

class SoundEngine : public BlackBox {
public:
	SoundEngine( int sr );
	void playnote( int note, int voice );
	void stop( int voice );
	int Process();
	int globalBuffer[GLOBALBUFFERSIZE];
	int globalBufferCursor;
	s16* granulizerBuffer;
	int granulizerBufferSize;	
	GranularSampler* granulizer;
private:
	int numvoices;
	SinOsc** oscs;
	FeedBackDelay* delay;
};
