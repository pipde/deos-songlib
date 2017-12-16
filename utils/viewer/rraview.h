//Author: Michael Raines
//
typedef struct _highlightData{
	int top;
	int bottom;
	int left;
	int right;
	int isActive;
	int isFinished;
	float colors[3];
} HighlightData;

typedef struct _backgroundData{
	float colors[3];
} BackgroundData;

typedef struct _waveData{
	float colors[3];
} WaveData;

typedef struct _zoomData{
	int leftIndex;
	int rightIndex;
	int channel;
} ZoomData;

typedef struct __screen_properties{

	float xl;
	float xr;
	float yt;
	float yb;
	float zt;
	float za;

} SCREEN_PROPERTIES;

void drawDivider();
void drawChannelMid();
void drawHighlight();
void drawTracks();
void drawZoom();
void display( void );
int determineChannelTop( int x, int y );
int determineChannelBottom( int x, int y );
void setZoomRange( int l, int r, int c );
void motionManager( int x, int y );
void mouseManager( int btn, int state, int x, int y );
int checkInput( char *input );
int processInput( FILE *fp );
void setGlOrtho( SCREEN_PROPERTIES sp);
void resizeScrollbar();
void updateScale();
void setWindowDim( int x, int y );
void reshapeManager( int x, int y );
void init();
void deallocate();
