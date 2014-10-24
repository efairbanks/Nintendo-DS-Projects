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
#define NUMVOICES	8
#define BUFSIZE		300
#define SQUARESIZE  30
#define TONROWS		5
#define TONCOLS		8

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	int i, j, curvoice = 0;

	bool tonmatrix[TONROWS][TONCOLS];

	int  midmatrix[TONROWS][TONCOLS];

	int  lstmatrix[TONROWS][TONCOLS];


	SoundEngine* soundengine = new SoundEngine( SAMPLERATE, NUMVOICES );

	SoundSetup( soundengine, SAMPLERATE, BUFSIZE );


	for( i = 0; i < TONROWS; i++ )

		for( j = 0; j < TONCOLS; j++ ){

			midmatrix[i][j*2]	= i * 7 + 3 + j;

			midmatrix[i][j*2+1]	= i * 7 + j;

		}


	for( i = 0; i < TONROWS; i++ )

		for( j = 0; j < TONCOLS; j++ )
			
			tonmatrix[i][j] = FALSE;


	while(1) {


		touchPosition touch;

		touchRead( &touch );


		for( i = 0; i < TONROWS; i++ )

			for( j = 0; j < TONCOLS; j++ )

				lstmatrix[i][j] = tonmatrix[i][j];


		for( i = 0; i < TONROWS; i++ ){ // Row

			for( j = 0; j < TONCOLS/2; j++ ){ // Col

				if( recttest( touch.px, touch.py, j * ( SQUARESIZE * 2 ) + 8, i * SQUARESIZE + 13, SQUARESIZE - 2, SQUARESIZE - 2 ) )
					
					tonmatrix[i][j*2] = TRUE;

				if( recttest( touch.px, touch.py, j * ( SQUARESIZE * 2 ) + 8 + SQUARESIZE, i * SQUARESIZE + 13 + ( SQUARESIZE / 2 ), SQUARESIZE - 2, SQUARESIZE - 2 ) )

					tonmatrix[i][j*2+1] = TRUE;

			}

		}


		if( touch.px + touch.py == 0 ){

			for( i = 0; i < TONROWS; i++ )

				for( j = 0; j < TONCOLS; j++ )

					tonmatrix[i][j] = FALSE;

			for( i = 0; i < NUMVOICES; i++ )

				soundengine->stop( i );

		}


		for( i = 0; i < TONROWS; i++ )

			for( j = 0; j < TONCOLS; j++ )

				if( tonmatrix[i][j] == 1 && lstmatrix[i][j] == 0 ){

					soundengine->playnote( midmatrix[(TONROWS - 1) - i][j] + 24, curvoice );

					curvoice = ( curvoice + 1 ) % NUMVOICES;

				}

			

		for( i = 0; i < TONROWS; i++ ){ // Row

			for( j = 0; j < TONCOLS/2; j++ ){ // Col

				GraphicsEng::Instance()->DrawRect( j * ( SQUARESIZE * 2 ) + 8, i * SQUARESIZE + 13, SQUARESIZE - 2, SQUARESIZE - 2, tonmatrix[i][j*2] ? SQUARESIZE/2 : 1, RGB15( 31, 31, 31 ), ALPHA_ON, SUB_LAYER );

				GraphicsEng::Instance()->DrawRect( j * ( SQUARESIZE * 2 ) + 8 + SQUARESIZE, i * SQUARESIZE + 13 + ( SQUARESIZE / 2 ), SQUARESIZE - 2, SQUARESIZE - 2, tonmatrix[i][j*2+1] ? SQUARESIZE/2 : 1, RGB15( 31, 31, 31 ), ALPHA_ON, SUB_LAYER );

			}

		}


		GraphicsEng::Instance()->Update();


	}

}
