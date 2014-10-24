/*---------------------------------------------------------------------------------

	Basic template code for starting a DS app

---------------------------------------------------------------------------------*/

#include <nds.h>
#include <stdio.h>
#include "general.h"
#include "blackbox.h"
#include "graphicseng.h"
#include "soundengine.h"

#define SAMPLERATE	2<<15
#define BUFSIZE		2<<12

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	int stream[256];

	SoundEngine* soundengine = new SoundEngine( SAMPLERATE );

	SoundSetup( soundengine, SAMPLERATE, BUFSIZE );
	
	Line* line = new Line( 60 );
	
	line->SetLengthMS(1000);
	
	line->SetDelayMS(1000);
	
	line->SetStartVal(MAXAMP);
	
	line->SetEndVal(0);
	
	line->Reset();


	while(1) {

		touchPosition touch;
		
		touchRead( &touch );
		
		scanKeys();
		
		if( keysHeld() & KEY_TOUCH ){
		
			soundengine->NoteOn( 0 + touch.px>>2 );
		
			soundengine->SetModFreqMul( touch.py>>3 );
			
			GraphicsEng::Instance()->DrawRect(

				touch.px-3,touch.py-3,6,6,1,RGB15(60,60,60),ALPHA_ON,SUB_LAYER

			);
		
		}
		
		stream[0] = (line->Process()*191)/MAXAMP;
		
		for( int i = 0; i < 256; i++ ){
			
			GraphicsEng::Instance()->DrawPoint( (255-i), 191-stream[i], RGB15(255,255,255), ALPHA_ON, MAIN_LAYER );
			
		}
		
		for( int i = 0; i < 255; i++ ){
		
			if( i > 0 ) stream[(255-i)] = stream[(255-i)-1];
			
		}
		
		GraphicsEng::Instance()->Update();


	}

}
