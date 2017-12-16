//Author: Michael Raines
//Purpose: A utility to allow users to visualize RRA files in wave form.

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef FREEGLUT
#include <GL/freeglut_ext.h>
#else
#include <GL/glut.h>
#endif
#include "glui.h"

#ifdef __cplusplus
extern "C" {
	#endif
    #include "../../lib/util.h"
    #include "../../lib/rra.h"
	#ifdef __cplusplus
}
#endif

#include "rraview.h"

GLUI *pane;
GLUI_Scrollbar *scroll;

int* channel0;
int* channel1;
int VIEW_SIZE = 500000;
int channelSize;
int channelCount;
float channel0_bottom;
float channel0_mid;
float channel0_top;
float channel1_top;
float channel1_mid;
float channel1_bottom;
int VIEW_DEBUG;
SCREEN_PROPERTIES sp;
float waveScale = 1;
int isDragging = 0;
int skip = 200;
int isNewZoom = 0;
float clipLimit = 15000;

int dividerWidth = 2;
int WINDOW_HEIGHT = 750;
int WINDOW_WIDTH = 1000;
int xScale = 0;
int yScale = 0;

int leftClicked = 0;
int justClicked = 0;
int selectedChannel = 0;

char* PROGRAM_NAME = "rraview";
char* PROGRAM_VERSION = "0.01";
int main_window;
int scroll_position;

HighlightData hl;
BackgroundData bg;
WaveData wd;
ZoomData zd;

const int STATE_IDLE = 0;
const int STATE_HIGHLIGHTING = 1;
int STATE;

/****** DRAW ROUTINES *******/
void drawDivider(){
	float midpoint = sp.yt/2;
	glColor3f( 0.0, 0.0, 0.0 );
	glLineWidth( 5 );
	glBegin( GL_LINES );
		glVertex2f( sp.xl - scroll_position, midpoint );
		glVertex2f( sp.xr - scroll_position, midpoint );
	glEnd();
	if( channelCount == 2 ){
		float trackSplitPoint = midpoint/2;
		channel0_mid = midpoint - (midpoint/4);
		channel1_mid = trackSplitPoint/2;
		glColor3f( 0.5, 0.5, 0.5 );
		glBegin( GL_LINES );
			glVertex2f( sp.xl - scroll_position, trackSplitPoint );
			glVertex2f( sp.xr - scroll_position, trackSplitPoint );
		glEnd();
	}else{
		channel1_mid = midpoint/2;
	}
	//Candy Coating TODO: FIX TO LOOK BETTER
}

void drawChannelMid(){
	glLineWidth(1);
	glColor3f( 0.5, 0.5, 0.5 );
	glBegin( GL_LINES );
		glVertex2f( sp.xl - scroll_position, channel0_mid );
		glVertex2f( sp.xr - scroll_position, channel0_mid );
	glEnd();
	if( channelCount == 2 ){
		glBegin( GL_LINES );
			glVertex2f( sp.xl - scroll_position, channel1_mid );
			glVertex2f( sp.xr - scroll_position, channel1_mid );
		glEnd();
	}
	//Draw Zoom Split
	glBegin( GL_LINES );
		glVertex2f( sp.xl - scroll_position, sp.yt - (sp.yt/4));
		glVertex2f( sp.xr - scroll_position, sp.yt - (sp.yt/4));
	glEnd();
}

void drawHighlight(){
	if( hl.isActive ){
		glColor3f( hl.colors[0], hl.colors[1], hl.colors[2] );
		glBegin(GL_POLYGON);
			glVertex2f( hl.left - scroll_position, hl.top );
			glVertex2f( hl.left - scroll_position, hl.bottom );
			glVertex2f( hl.right - scroll_position, hl.bottom );
			glVertex2f( hl.right - scroll_position, hl.top );
		glEnd();
	}
}

void drawTracks(){
	int i;
	glColor3f( wd.colors[0], wd.colors[1], wd.colors[2] );
	glLineWidth( 1 );
	channel0_top = sp.yt/2;
	if( channelCount == 2 ){
		channel0_bottom = sp.yt/4;
		channel1_top = channel0_bottom;
		channel1_bottom = sp.yb;
		channel1_mid = (channel1_top + channel1_bottom)/2;
	}else{
		channel0_bottom = sp.yb;
	}
	channel0_mid = (channel0_top + channel0_bottom)/2.0;
	float yScale = (channel0_top - channel0_mid)/clipLimit;
	if( VIEW_DEBUG ){
		printf("DEBUG: Channel 0 Top: %.2f Mid: %.2f Bottom: %.2f\n", channel0_top, channel0_mid, channel0_bottom);
		if( channelCount == 2 ){
			printf("DEBUG: Channel 1 Top: %.2f Mid: %.2f Bottom: %.2f\n", channel1_top, channel1_mid, channel1_bottom);
		}
	}
	glBegin( GL_LINE_STRIP );
	float y;
	for( i = sp.xl ; i < channelSize;){
		if( i > sp.xr ){
			if( VIEW_DEBUG )
				printf("DEBUG: Breaking track draw at index %d as it is out of view bounds.\n",i);
			break;
		}
		if( abs(channel0[i]) < clipLimit ){
			y = channel0_mid + ((float)channel0[i] * yScale);
		}else if( channel0[i] > 0 ){
			y = channel0_top;
		}else{
			y = channel0_bottom;
		}
		glVertex2f( ((float)i - scroll_position), y );
		if( i+skip < channelSize )
			i+=skip;
		else
			break;
	}
	glEnd();
	if( channelCount == 2 ){
		glBegin( GL_LINE_STRIP );
			for( i = sp.xl; i < channelSize;){
				if( i > sp.xr )
					break;
				if( abs(channel1[i]) < clipLimit ){
					y = channel1_mid + ((float)channel1[i] * yScale);
				}else if( channel1[i] > 0 ){
					y = channel1_top;
				}else{
					y = channel1_bottom;
				}
				glVertex2f( ((float)i - scroll_position), y);
				if( i+skip < channelSize )
					i+=skip;
				else
					break;
			}
		glEnd();
	}
}

void drawZoom(){
	int i;
	int size = zd.rightIndex-zd.leftIndex;
	float x = sp.xl - scroll_position;
	float y;
	int xPadding = (sp.xr-sp.xl)/size;
	float yScale = (sp.yt - (sp.yt - (sp.yt/4)))/clipLimit;
	glColor3f( wd.colors[0], wd.colors[1], wd.colors[2] );
	glLineWidth( 1 );
	if( VIEW_DEBUG ){
		printf("DEBUG: Drawing Zoom set of %d samples from leftIndex %d to right index %d.\n",size, zd.leftIndex, zd.rightIndex);
		printf("DEBUG: Starting draw with X: %f  and xPadding: %d.\n",x, xPadding);
	}
	glBegin(GL_LINE_STRIP);
	for( i = zd.leftIndex; i <= zd.rightIndex; i++){
		if( i < channelSize ){
			if( selectedChannel == 0 ){
				if( abs(channel0[i]) < clipLimit ){
					y = (sp.yt - (sp.yt/4)) + ((float)channel0[i] * yScale);
				}else if( channel0[i] < 0 ){
					y = sp.yt/2;
				}else{
					y = sp.yt;
				}
			}else{
				if( abs(channel1[i]) < clipLimit ){
					y = (sp.yt - (sp.yt/4)) + ((float)channel1[i] * yScale);
				}else if( channel1[i] < 0 ){
					y = sp.yt/2;
				}else{
					y = sp.yt;
				}
			}
			glVertex2f( x, y );
			x += xPadding;
		}else{
			if( VIEW_DEBUG )
				printf("DEBUG: Breaking Zoom draw. Selection outside of known set.\n");
			break;
		}
	}
	glEnd();
}

void display( void ){
	glClear( GL_COLOR_BUFFER_BIT );
	drawDivider();
	drawHighlight();
	drawChannelMid();
	drawTracks();
	if(hl.isFinished)
		drawZoom();
	glutSwapBuffers();
}

/******* POSITIONING HELPERS **********/

int determineChannelTop( int x, int y ){
	if( y <= channel0_top && y >= channel0_bottom ){
		selectedChannel = 0;
		return channel0_top - (dividerWidth * yScale);
	}else if( y <= channel1_top ){
		selectedChannel = 1;
		return channel1_top - (dividerWidth * yScale);
	}
	return 0;
}

int determineChannelBottom( int x, int y ){
	if( y >= channel0_bottom && y <= channel0_top ){
		return channel0_bottom + ((dividerWidth+1) * yScale);
	}else if( y <= channel1_top ){
		return channel1_bottom;
	}
	return 0;
}

void setZoomRange( int l, int r, int c ){
	zd.leftIndex = l;
	zd.rightIndex = r;
	zd.channel = c;
}

/******* EVENT MANAGERS OTHER THAN RESHAPE ********/
void idleManager( void )
{
	  if ( glutGetWindow() != main_window )
		      glutSetWindow(main_window);
	  //glutPostRedisplay();
	  GLUI_Master.auto_set_viewport();
}
void motionManager( int x, int y ){
	int dx = 1;
	static int lastx = 0;
	y = WINDOW_HEIGHT -y;
	if( leftClicked ){
		if( determineChannelTop(x*xScale,y*yScale) ){
			if( justClicked ){
				justClicked = 0;
				if( VIEW_DEBUG )
					printf("DEBUG: Starting Highlight at X: %d   Y: %d\n", x, y);
				hl.isActive=1;
				hl.top = determineChannelTop(x*xScale,y*yScale);
				hl.bottom = determineChannelBottom(x*xScale,y*yScale);
				hl.left = (x * xScale)+scroll_position;
				hl.right = (x * yScale+scroll_position);
				lastx = hl.right;
			}else if( abs((lastx/xScale) - x) > dx ){
				hl.right = (x * xScale)+scroll_position;
				lastx = hl.right;
				glutPostRedisplay();
			}
		}
	}
}

void mouseManager( int btn, int state, int x, int y ){
	y = WINDOW_HEIGHT - y;
	if(btn==GLUT_LEFT_BUTTON && state==GLUT_DOWN){
		if( VIEW_DEBUG )
			printf("DEBUG: Left mouse pressed at X: %d   Y: %d\n",x,y);
		leftClicked=1;
		justClicked=1;
	}else if(btn==GLUT_LEFT_BUTTON && state==GLUT_UP){
		if( VIEW_DEBUG )
			printf("DEBUG: Left mouse released at X: %d   Y: %d\n",x,y);
		leftClicked=0;
		if(hl.isActive){
			if(hl.left < hl.right){
				if(hl.right > sp.xr)
					hl.right=sp.xr;
				setZoomRange(hl.left,hl.right,selectedChannel);
			}else{
				if(hl.right < sp.xl)
					hl.right=sp.xl;
				setZoomRange(hl.right,hl.left,selectedChannel);
			}
			hl.isFinished=1;
			glutPostRedisplay();
		}
	}else if(btn==GLUT_RIGHT_BUTTON && state==GLUT_UP){
		if( VIEW_DEBUG )
			printf("DEBUG: Right mouse released - Disabeling highlight if active\n");
		if( hl.isActive ){
			hl.isActive = 0;
			glutPostRedisplay();
		}
	}
}

/******* RRA PROCESSING ROUTINES **********/

int checkInput( char *input ){
	if(VIEW_DEBUG){
		printf("DEBUG: Checking input %s\n", input );
	}
	FILE *fp = fopen( input, "r" );

	if( fp == NULL ){
		printf("Unable to open %s\n", input);
		return 0;
	}else{
		if(VIEW_DEBUG){
			printf("DEBUG: Input file is openable\n");
		}
		if( processInput( fp ) ){

		}else{
			printf("Unable to process contents of %s, make sure it is a well formed RRA file.\n", input);
			return 0;
		}
	}
	fclose(fp);
	return 1;
}

int processInput( FILE *fp ){
	int i;
	RRA *track = newRRAHeader();
	if( track == NULL ) return 0;
	if( VIEW_DEBUG )
		printf("DEBUG: Header read sucessfully\n");
	readRRAHeader( fp, track, 0 );
	channelCount = track->channels;
	if( track->channels == 2){
		channelSize = ((track->samples)/2) + 1;
		channel0 = (int*)malloc( channelSize * sizeof(int) );
		channel1 = (int*)malloc( channelSize * sizeof(int) );
	}else{
		channelSize = track->samples;
		channel0 = (int*)malloc( channelSize * sizeof(int) );
	}
	if( VIEW_DEBUG )
		printf("DEBUG: %d channels found with %d samples.\n",channelCount, channelSize);
	for( i = 0; feof( fp ) == 0 && i<track->samples; i++) {
		int amp = readRRAAmplitude( fp, track->bitsPerSample, 0);
		if( channelCount == 2){
			if( i % 2 == 0 )
				channel0[i/2]=amp;
			else
				channel1[i/2]=amp;
		}else{
			channel0[i]=amp;
		}
	}
	if( VIEW_DEBUG )
		printf("DEBUG: Completed read of file\n");
	
	return 1;
}

/****** WINDOWING AND SIZING FUNCTIONS ********/

void setGlOrtho( SCREEN_PROPERTIES sp){
	updateScale();
	//glOrtho( sp.xl , sp.xr, sp.yb, sp.yt, sp.zt, sp.za );
}

void updateScale(){
	xScale = (sp.xr-sp.xl)/WINDOW_WIDTH;
	yScale = (sp.yt-sp.yb)/WINDOW_HEIGHT;
	if( VIEW_DEBUG )
		printf("DEBUG: Scale updated to %dppx %dppy\n",xScale,yScale);
	glutPostRedisplay();
}

void setWindowDim( int x, int y ){
	WINDOW_HEIGHT = y;
	WINDOW_WIDTH = x;
	updateScale();
	resizeScrollbar();
	glutPostRedisplay();
}

void reshapeManager( int x, int y ){
	setWindowDim(x,y);
	glutPostRedisplay();
}

/******* INITIALIZING ALL VALUES ********/

void init(){
	STATE = STATE_IDLE;
	sp.xl = 0;
	sp.xr = VIEW_SIZE;
	sp.yb = 0;
	sp.yt = VIEW_SIZE;
	sp.zt = 0;
	sp.za = VIEW_SIZE;

	bg.colors[0] = .9;
	bg.colors[1] = .9;
	bg.colors[2] = .9;

	hl.top = 0;
	hl.left = 0;
	hl.bottom = 0;
	hl.right = 0;
	hl.colors[0] = .0;
	hl.colors[1] = 1.0;
	hl.colors[2] = .0;
	hl.isActive = 0;
	hl.isFinished = 0;

	wd.colors[0] = .0;
	wd.colors[1] = .0;
	wd.colors[2] = 1.0;

	glClearColor( 0.9, 0.9, 0.9, 0.0 );
	glColor3f(1.0, 1.0, 1.0);
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( sp.xl , sp.xr, sp.yb, sp.yt, sp.zt, sp.za );
	updateScale();
	idleManager();
}

/********* CLEANUP *********/

void deallocate(){
	if( channelCount > 1){
		free( channel0 );
		free( channel1 );
	}else{
		free( channel0 );
	}
	if( VIEW_DEBUG )
		printf("DEBUG: Freed %d channels, %d samples each.\n", channelCount, channelSize);
}

/********* GLUI **********/
void controlCallBack( int control ){
	if( control == 0 ){
		if( VIEW_DEBUG )
			printf( "DEBUG: Scrollbar callback received. New Scroll Position %d\n", scroll_position );
		sp.xl = scroll_position;
		sp.xr = scroll_position+VIEW_SIZE;
		/*
		sp.yt = scroll_position;
		sp.yb = scroll_position+VIEW_SIZE;
		sp.za = scroll_position;
		sp.zt = scroll_position+VIEW_SIZE;
		*/
		setGlOrtho(sp);
	}
}

void resizeScrollbar(){
	scroll->set_w( WINDOW_WIDTH - 15 );
}

void setupGLUI( int window ){
	GLUI_Master.set_glutMouseFunc( mouseManager );
	GLUI_Master.set_glutReshapeFunc( reshapeManager );
	GLUI_Master.set_glutIdleFunc( idleManager );
	pane = GLUI_Master.create_glui_subwindow(window, GLUI_SUBWINDOW_BOTTOM);
	//GLUI_Panel *bp = new GLUI_Panel( pane, "Bottom Pane" );
	scroll = new GLUI_Scrollbar( pane, "Scrollbar", 1, &scroll_position, 0, controlCallBack);
	resizeScrollbar();
	scroll->set_int_limits( 0, channelSize, GLUI_LIMIT_CLAMP );

	//GLUI_Main *glui_subwin = GLUI_Master.create_subwindow(window, GLUI_SUBWINDOW_BOTTOM);
	pane->set_main_gfx_window(window);
}

int main( int argc, char* argv[] ){
	VIEW_DEBUG = 0;
	int argc2 = 0;
	if( checkInput( argv[1] )){
		//GLUT stuff
		glutInit( &argc2, argv);
		glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB );
		glutInitWindowSize( WINDOW_WIDTH, WINDOW_HEIGHT );
		glutInitWindowPosition( 0, 0 );
		main_window = glutCreateWindow( argv[1] );
		glutDisplayFunc( display );
		glutMotionFunc( motionManager );
		setupGLUI( main_window );
		init();
		glutMainLoop();
	}
	return 0;
}
