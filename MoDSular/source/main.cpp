/*---------------------------------------------------------------------------------

	Basic template code for starting a DS app

---------------------------------------------------------------------------------*/

#include <nds.h>
#include <stdio.h>
#include "general.h"
#include "blackbox.h"
#include "graphicseng.h"
#include "soundengine.h"
#include "granularsampler.h"

#define SAMPLERATE	32768
#define NUMVOICES	1
#define BUFSIZE		300
#define SQUARESIZE  30
#define TONROWS		5
#define TONCOLS		8

#define CIRCLEDIVS 360

void downSampleArray(int* in, int inSize, int* out, int outSize) {
	int lastOutIndex=0;
	int numbersProcessed=0;
	int runningSum=0;
	for(int i=0; i<inSize; i++) {
		if(lastOutIndex!=(i*outSize)/inSize) {
			out[lastOutIndex]=runningSum/numbersProcessed;
			numbersProcessed=0;
			runningSum=0;
			lastOutIndex=(i*outSize)/inSize;
		} else {
			runningSum+=in[i];
			numbersProcessed++;
		}
		out[outSize-1]=runningSum/numbersProcessed;
	}
}

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	SoundEngine* soundengine = new SoundEngine( SAMPLERATE, NUMVOICES );
	SoundSetup( soundengine, SAMPLERATE, BUFSIZE );

	int tempBuf[GLOBALBUFFERSIZE];
	int displayBuf[CIRCLEDIVS];

	int time = 0;
	u8 squares = 10;

	while(1) {
    // -------------------- //
		// -- INPUT HANDLING -- //
    // -------------------- //
		touchPosition touch;
		touchRead( &touch );
		scanKeys();
		u32 down=0, up=0, held=0;
		down=keysDown();
		held=keysHeld();
		up=keysUp();
		if(down&KEY_UP) squares++;
		if(down&KEY_DOWN) squares--;
		if(held&KEY_TOUCH) {
			soundengine->playnote(touch.px/2,0);
		}
		if(up&KEY_TOUCH) {
			soundengine->stop(0);
		}
		// -------------- //
		// -- GRAPHICS -- //
		// -------------- //
		for(int i=0; i<GLOBALBUFFERSIZE; i++) {
			tempBuf[i]=abs(soundengine->globalBuffer[i]);
		}
		downSampleArray(tempBuf,GLOBALBUFFERSIZE,displayBuf,CIRCLEDIVS);
		for(int i=0; i<CIRCLEDIVS; i++) {
			int radius=1000*displayBuf[i]/MAXAMP;
			int nextRadius=1000*displayBuf[(i+1)%CIRCLEDIVS]/MAXAMP;
			int angle=DEGREES_IN_CIRCLE*i/CIRCLEDIVS;
			int nextAngle=DEGREES_IN_CIRCLE*((i+1)%CIRCLEDIVS)/CIRCLEDIVS;
			int x=sinLerp(angle)*radius/DEGREES_IN_CIRCLE+(SCREEN_WIDTH/2);
			int y=cosLerp(angle)*radius/DEGREES_IN_CIRCLE+(SCREEN_HEIGHT/2);
			int nextX=sinLerp(nextAngle)*nextRadius/DEGREES_IN_CIRCLE+(SCREEN_WIDTH/2);
			int nextY=cosLerp(nextAngle)*nextRadius/DEGREES_IN_CIRCLE+(SCREEN_HEIGHT/2);
			GraphicsEng::Instance()->DrawLine(x,y,nextX,nextY,RGB15(31,31,31),ALPHA_ON,SUB_LAYER);
		} 
		for(int i=0; i<GLOBALBUFFERSIZE; i++) {
			int val = soundengine->globalBuffer[(soundengine->globalBufferCursor+i)%GLOBALBUFFERSIZE];
			val = val*(SCREEN_HEIGHT/2)/MAXAMP;
			GraphicsEng::Instance()->DrawPoint(	SCREEN_WIDTH*i/GLOBALBUFFERSIZE,
																					(SCREEN_HEIGHT/2)+val,
																					RGB15(31,31,31),
																					ALPHA_ON,
																					MAIN_LAYER);
		}
		GraphicsEng::Instance()->Update();
		time++;
	}
}
