/*---------------------------------------------------------------------------------
	Basic template code for starting a DS app
---------------------------------------------------------------------------------*/
#include <nds.h>
#include <stdio.h>
#include "general.h"
#include "blackbox.h"
#include "graphicseng.h"
#include "soundengine.h"
#define SAMPLERATE	32768
#define CIRCLEDIVS 360
#define BUFSIZE 300

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
	SoundEngine* soundengine = new SoundEngine( SAMPLERATE );
	SoundSetup( soundengine, SAMPLERATE, BUFSIZE );
	int* tempBuf = (int*)malloc(sizeof(int)*soundengine->granulizerBufferSize);
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
		if(down&KEY_TOUCH) soundengine->granulizer->SetPlaybackRate(0,1);
		if(held&KEY_TOUCH) {
			soundengine->granulizer->SetPlaybackPos(touch.px,SCREEN_WIDTH);
			soundengine->granulizer->SetGrainSize(touch.py);
		}
		if(up&KEY_TOUCH) {
			soundengine->granulizer->SetPlaybackRate(1,1);
			soundengine->stop(0);
		}
		// -------------- //
		// -- GRAPHICS -- //
		// -------------- //
		for(int i=0; i<soundengine->granulizerBufferSize; i++) {
			tempBuf[i]=abs(soundengine->granulizerBuffer[i]);
		}
		downSampleArray(tempBuf,soundengine->granulizerBufferSize,displayBuf,CIRCLEDIVS);
		for(int i=0; i<CIRCLEDIVS; i++) {
			int maxRadius=700;
			int radius=maxRadius*displayBuf[i]/MAXAMP;
			int nextRadius=maxRadius*displayBuf[(i+1)%CIRCLEDIVS]/MAXAMP;
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
