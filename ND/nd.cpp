#include <windows.h>
#define MSFS 1
extern HDC 			hdc;
extern HGLRC 		hrc;
HGLRC				hglrc;
extern WNDCLASS 	a;	
POINT				point;

// Standard Includes 
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <io.h>
// OpenGL 
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
// FSUIPC
#include <FSUIPC_User.h>
#include <Fs98State.h>
#include <defines.h>
// Our own stuff
#include <structs.h>
#undef EXTERN
#include <defaults.h>
#include <prototypes.h>
#include "nd.h"
#include <tgaload.h>
// Font routine prototypes
#include <fonts.h>
#include <registry.h>

#define ESCAPE 27

// MAP MODES
#define PLANMODE 1
#define MAPMODE  2 
#define VORMODE  3 
#define ILSMODE  4

// MODES
#define HDG 1
#define TRK 2
#define ILSV 3

int					terrain = 0;

GLuint				tlist1 = 101;
GLuint				tlist2 = 102;
int listno = 0;
int					mouseptr = 1;
HWND				hwnd;
long				wsl;					
HINSTANCE			hInstance;

int					mode = HDG;				// ND Mode switch HDG/TRK/ILSV

double				MAXRANGE=1.4;           // Max lat/lon distance to make the objects viewable.
double				tilt = 0.0;				// Orientation of the screen
long 				indx = 0;				// General purpose loop counter
int					width, height;			// window dimensions
int					wx, wy;					// placeholders
int					monochrome = 0;			// Is this used by anyone?
char				revision[30];
int					topmost = 0;
char				*subversion = "";
int					notconnected = 0;		// flag for FSUIPC connection
int					rangerings = 0;			
int					currentwpt = 0;			// For flightplan
int 				waypoints = 0;			// For flightplan
#ifdef TCAS
extern int			KillTCAS(void);			// Multiplayer TCAS
#endif
int					TCAS_Active = 0;		// AI Aircraft TCAS
double				padding = 0.0;			// Not really usefull
double 				scale = 200.0;			// Used for range calculation	
double 				lat, lon;				// General Purpose variables for great circle math
double				tdist, tbear;			// 
double				rlat, rlon;				//
double				coslat, coslon;			//
double				sinlat, sinlon;			//
double				acossinlat;				//
double				distance;				// Used to calculate Map Position
double				bearing;				// Ditto

int					firsttime = 1;          // Splash Screen 

double				headwind = 0.0;			// For Ground Speed calculation
double				alt;					// Converted altitude
double 				hdg;					// Heading
double				adf;					// ADF Heading
double				Nav2Rad;				// Radials
double				Nav1Rad;				// Radials
double 				magvar;					// Magnetic variation.
double				crosswind = 0.0;		// For Wind Correction Angle calculation
int					navaids = 0;			// Flag to toggle navaid display
double 				compassradius;			// Compass Radius for ARC and ROSE mode
int 				arcmode =  1;			// Arc/Rose mode toggle
int 				showvor = 0;			// VOR Display toggle
int 				showndb = 0;			// NDB Display toggle
int 				showapt = 0;			// ARPT Display toggle
int 				showfrq = 0;			// Freq display toggle
int 				showfix = 0;			// FIX Display toggle
int					hdgmode = 1;			// HDG/TRK/ILSV
int					windowframes = 1;		// WindowFrame toggle
int					display_stats = 0;		// Framerate display
int					lnavactive = 0;			// LNAV autopilot toggle
int					trackmode = 0;			// TRK mode toggle
int					mapmode = 0;			// Map mode settings (MAP/PLAN/ILS/VOR)
double				cb;						// This is the blue color to be used for the background.
int 				range = 2;				// Scope range
GLuint 				waylist;				// Vertex list for flight plan
char 				ipaddr[20];				// Might need this again some day
FS98StateData 		all;					// Structure for all the FSUIPC data
char 				display_buffer[200];	// General purpose 
long				total_objs;				// Number of navaids
int 				window;					//
double				track;					//

FILE				*logfile;				// For logging
extern double 		dimmer;					// For display dimmer
extern int 			frames;					// For framerate calculation
double				groundspeed = 0.0;		// 
double				wca;					// Wind correction Angle
double				WindowSize = 90.0;		// glOrtho2d size
int 				runways = 0;			// Display runway toggle (Not used)
TCAS_DATA			tcas[192];				// for AI TCAS

extern void			draw_compassface(int);
extern void 		setColor(double r, double g, double b);
char				*split_nd(void);
extern void 		dofps(int);
void				drawobjs(void);
int 				draw_runways(void);
void				draw_compass(void);
int 				comp(const void *el1, const void *el2);
void 				initstruct(void);
int 				read_sbp_flightplan(void);
void				getoutofdodge(void);
extern double		r2d(double deg);
extern double		d2r(double deg);
extern int check_lnav(void);
extern int fpwpt(char *st);

#define MAXINRANGE 20000
long objidx[MAXINRANGE];		// Index into the array (Navaids in range)
int idxcnt;						// Index count.
long ObjectTimer = 0;

// *****************************[ GEO Stuff ]***************************************
// Figure out how many rows we'll need,  and the number of columns.

// This terrain data is split 10800 columns by 5400 rows. (The whole earth)

short int data[5400][3600];

double LatSpread = 0.033333;
double LonSpread = 0.033333*3.0;

#define LATITUDE 0
#define LONGITUDE 1

void ErrorExit(LPTSTR lpszFunction) 
{ 
    TCHAR szBuf[80]; 
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    wsprintf(szBuf, 
        "%s failed with error %d: %s", 
        lpszFunction, dw, lpMsgBuf); 
 
    MessageBox(NULL, szBuf, "Error", MB_OK); 

    LocalFree(lpMsgBuf);
    ExitProcess(dw); 
}

DWORD WINAPI  ThreadProc( LPVOID lpParam )
{
	static DWORD ThreadID;
	int cs = 0;
	static int beenhere = 0;
	static int rowcount = 0;
	FILE * FileHandle;
	register int row = 0;
	register int col;
	int retcode;
	char LatDMS[30], LonDMS[30];
	int LatForce = 1;
	int LonForce = 1;
	char junk[20];
	double tlat,tlon;
	double elevation;
	static int alternate = 0;
	int ok = 0;
	double		x, x1, y, y1;
	double r,g,b,r1,g1,b1;
	double		navrange;
	double difference;
	double ps;
	int tlatend = 0;
	int tlonend = 0;
	static int latstart = 0;
	static int latend = 0;
	static int lonstart = 0;
	static int lonend = 0;

//	MessageBox(0,"IN","TEST",MB_OK);
//	if (!wglMakeCurrent(hdc, hglrc))
//	{
//		ErrorExit("THREAD");
//	}
	glPushMatrix();
	if (arcmode)
		glTranslatef(0.0, -70.0, 0.0);


	switch(range)
	{
	case 0:
//		return(0);
		break;
	case 1:
//		return(0);
		break;
	case 2:
		ps = 32;
		break;
	case 3:
		ps = 16;
		break;
	case 4:
		ps = 8;
		break;
	case 5:
		ps = 4;
		break;
	case 6:
		ps = 2;
		break;
	}

	if (!listno)
		glNewList(tlist2, GL_COMPILE);
	else
		glNewList(tlist1, GL_COMPILE);

	if (arcmode)
		ps *= 2;
	glPointSize(ps+1);
	glLineWidth(ps);
	glBegin(GL_POINTS);

	for (row = latstart; latend != 0?row <= latend:row < 5400; row++)
	{
		tlat = 90.0 - (LatSpread * (double)row);
		if (lat - tlat > (MAXRANGE*2))
			continue;
		else if (tlat - lat > (MAXRANGE*2))
			continue;
		if (latstart == 0)
			latstart = row;
		
		if (latend == 0)
			tlatend = row;

//		lon = -180.0;
		x1=y1=0.0;
		for (col = 0; col < 3600; col++)
		{
			tlon = -180.0 + (LonSpread * (double)col);
// Simple range restricting, we'll fine tune the rest later on. Gives us a quick guess as to what will be in view.
			if ((lon) - tlon > (MAXRANGE*2))
				continue;
			else if (tlon - (lon) > (MAXRANGE*2))
				continue;

			if (lonstart == 0)
				lonstart = col;
			if (lonend == 0)
				tlonend = col;

			elevation = ((double)data[row][col] * 3.2808399);
			r = 0.0; g = 0.2; b = 0.0;
			difference = alt - elevation;

			if (elevation < 0.0)
				setColor(0,0,1.0 - (((elevation*-1.0) / 39000.0) * 1.0));
			if (difference > 5000.0)
			{
				x1=y1=0.0;
				continue;
			}
			else if (difference <= 0.0)
			{
					cs = 5;
					r = 1.0; g = 0.0; b = 0.0;
					setColor(r,g,b);
			}
			else if (difference <= 2500.0)
			{
					cs = 4;
					r = 1.0; g = 0.5 + ((difference / 2500.0) * 0.5); b = 0.0;
					setColor(r,g,b);
			}
			else if (difference <= 5000.0)
			{
					cs = 3;
					r = 0.0; g = 1.0 - (0.5+(((difference-2500.0)/2500.0)*0.5)); b = 0.0;
					setColor(r,g,b);
			}


// Great circle math
			distance = acos(sinlat * sin(d2r(tlat)) + coslat * 
				cos(d2r(tlat)) * cos(d2r(lon)-d2r(tlon)));

			if (sin(d2r(tlon) - d2r(lon)) < 0)
				bearing = acos((sin(d2r(tlat)) - sinlat * cos(distance)) / 
					(sin(distance) * coslat));
			else
				bearing = 2.0 * MYPI - acos((sin(d2r(tlat)) - sinlat * cos(distance)) / 
					(sin(distance) * coslat)); 
	
			distance *= ((180.0*60.0)/MYPI);

// Is this navaid out of range?
			if (distance > (rangemarkers[range]*2.0))
				continue;

			bearing = (360.0-r2d(bearing))+magvar;
			bearing = fabs(hdg) - bearing;
			bearing -= 270.0;
			if (bearing < 0.0)
				bearing += 360.0;

			navrange = distance / (rangemarkers[range]*2.0);

			navrange *= compassradius;

			x = cos((MYPI / 180.0) * bearing) * navrange;
			y = sin((MYPI / 180.0) * bearing) * navrange;

//			if (ok == 0)
//			{
//			if (((double)data[row][col] * 3.2808399) > 27000.0)
//			{
				
			if ((x1 != 0.0) && (y1 != 0.0))
				{
				glVertex2f(x, y);
//				setColor(r1,g1,b1);
//				glVertex2f(x1,y1);
//				glVertex2f(x-(2*(7-range+1)), y);
//				glVertex2f(x-(2*(7-range+1)), y-(2*(7-range+1)));
//				glVertex2f(x,y-(2*(7-range+1)));

				}
			r1 = r;g1 = g;b1=b;
			x1 =x; y1 = y;
//			}
//			ok ++;
//			if (ok == 23)
//				ok = 0;
//				printf("DLAT: %.2f DLON: %.2f\n", lat*-1.0, lon);
//				printf("LAT: %s LON: %s\n", DecToDMS(lat*-1.0, LatDMS, LATITUDE), DecToDMS(lon, LonDMS, LONGITUDE));
//				printf("%06.0fFT %dM\n", (double)data[row][col] * 3.2808399,data[row][col] );
//			}
//			lon += Spread;
//			LonForce ++;
//			if (LonForce == 30)
//			{
//				tlon = ceil(lon);
//				lon = tlon;
//				LonForce = 1;
//			}
		}
//		row++;
//		lat += Spread;
//		LatForce ++;
//		if (LatForce == 30)
//		{
//			tlat = ceil(lat);
//			lat = tlat;
//			LatForce = 1;
//		}
	}
	glEnd();
	glEndList();
//	glEndList();
	glPopMatrix();
	latend = tlatend;
	lonend = tlonend;
//	fgetc(stdin);
	listno ^= 1;
//	ExitThread(0);
	return(0);
}


int DrawGEO()
{
	static DWORD ThreadID;
	int cs = 0;
	static int beenhere = 0;
	static int rowcount = 0;
	FILE * FileHandle;
	register int row = 0;
	register int col;
	int retcode;
	char LatDMS[30], LonDMS[30];
	int LatForce = 1;
	int LonForce = 1;
	char junk[20];
	double tlat,tlon;
	double elevation;
	static int alternate = 0;
	int ok = 0;
	double		x, x1, y, y1;
	double r,g,b,r1,g1,b1;
	double		navrange;
	double difference;
	double ps;
	HANDLE hThread;

//	lat = -90.0;
//	lon = -180.0;

	if (!beenhere)
	{
//		MessageBox(0, "OK","TEST",MB_OK);
		FileHandle = fopen("ETOPO2.dos.out", "rb");

		if (FileHandle == NULL)
			printf("Error ");
		while ((retcode = fread(data[rowcount++], sizeof(short int) * 3600, 1, FileHandle)) > 0)
		{
			if (rowcount >=5400) break;
		}
		
		fclose(FileHandle);

		beenhere = 1;
	}

	switch(range)
	{
	case 0:
		return(0);
		break;
	case 1:
		return(0);
		break;
	case 2:
		ps = 32;
		break;
	case 3:
		ps = 16;
		break;
	case 4:
		ps = 8;
		break;
	case 5:
		ps = 4;
		break;
	case 6:
		ps = 2;
		break;
	}


	alternate++;
	if (alternate > 1) 
	{
		if (alternate == 50)
			alternate = 0;
		glPushMatrix();
		if (arcmode)
			glTranslatef(0.0, -70.0, 0.0);
		if (!listno)
			glCallList(tlist1);
		else
			glCallList(tlist2);
//		glCallList(tlist);
		glPopMatrix();
		return(0);
	}

//	MessageBox(0, "Gonna run the proc","TEST",MB_OK);
	ThreadProc(NULL);
	glPushMatrix();
	if (arcmode)
		glTranslatef(0.0, -70.0, 0.0);
	if (!listno)
		glCallList(tlist1);
	else
		glCallList(tlist2);
//		glCallList(tlist);
		glPopMatrix();

	/*
	hThread = CreateThread( 
		NULL,				// default security attributes
		0,					// use default stack size  
		ThreadProc,			// thread function 
		NULL,				// argument to thread function 
		0,					// use default creation flags 
		&ThreadID);			// returns the thread identifier 
	if (hThread == NULL)
	{
		MessageBox(0, "Oops", "DEBUG", MB_OK);
	}
	*/
	
	return(0);
}



// *****************************[ GEO Stuff ]***************************************



void NDGetRegistry(void)
{
	if ((wx = (int)RegGetDWord("BOEING\\ND", "wx")) == -1)
		return;
	wy = (int)RegGetDWord("BOEING\\ND", "wy");
	width = (int)RegGetDWord("BOEING\\ND", "width");
	height = (int)RegGetDWord("BOEING\\ND", "height");
	windowframes = (int)RegGetDWord("BOEING\\ND", "windowframes");
	tilt = (double)RegGetDWord("BOEING\\ND", "tilt");
	mouseptr = (int)RegGetDWord("BOEING\\ND", "mouseptr");
	topmost = (int)RegGetDWord("BOEING\\ND", "topmost");
	arcmode = (int)RegGetDWord("BOEING\\ND", "arcmode");
	mapmode = (int)RegGetDWord("BOEING\\ND", "mapmode");
	range = (int)RegGetDWord("BOEING\\ND", "range");
	rangerings = (int)RegGetDWord("BOEING\\ND", "rangerings");
	showvor = (int)RegGetDWord("BOEING\\ND", "showvor");
	showndb = (int)RegGetDWord("BOEING\\ND", "showndb");
	showapt = (int)RegGetDWord("BOEING\\ND", "showapt");
	showfix = (int)RegGetDWord("BOEING\\ND", "showfix");
	showfrq = (int)RegGetDWord("BOEING\\ND", "showfrq");
}

void NDSetRegistry(void)
{
	RegSetDWord("BOEING\\ND", "wx", wx);
	RegSetDWord("BOEING\\ND", "wy", wy);
	RegSetDWord("BOEING\\ND", "width", width);
	RegSetDWord("BOEING\\ND", "height", height);
	RegSetDWord("BOEING\\ND", "windowframes", windowframes);
	RegSetDWord("BOEING\\ND", "tilt", tilt);
	RegSetDWord("BOEING\\ND", "mouseptr", mouseptr);
	RegSetDWord("BOEING\\ND", "topmost", topmost);
	RegSetDWord("BOEING\\ND", "arcmode", arcmode);
	RegSetDWord("BOEING\\ND", "mapmode", mapmode);
	RegSetDWord("BOEING\\ND", "range", range);
	RegSetDWord("BOEING\\ND", "rangerings", rangerings);
	RegSetDWord("BOEING\\ND", "showvor", showvor);
	RegSetDWord("BOEING\\ND", "showndb", showndb);
	RegSetDWord("BOEING\\ND", "showapt", showapt);
	RegSetDWord("BOEING\\ND", "showfix", showfix);
	RegSetDWord("BOEING\\ND", "showfrq", showfrq);
}

// ******************
// Sort routine
// ******************
int comp(const void *el1, const void *el2)
{
	OBJECTS *e1, *e2;

	e1 = (OBJECTS *)el1;
	e2 = (OBJECTS *)el2;
	return(strcmp(e1->id, e2->id));
}

// *******************************
// Read in the navaid database
// *******************************
void initstruct()
{
	int				in;
	int				init;
//	unsigned int 	a = 0;
//	unsigned int 	n = 0;
//	unsigned int 	f = 0;
//	unsigned int 	v = 0;

	in = open("navaids.dat", O_RDONLY|O_BINARY);
	if (in == -1)
	{
		navaids = 0;
		return;
	}

	indx = 0;
	
	fflush(stdout);
	while (read(in, (char *)&objects[indx++], sizeof(OBJECTS)))
	{
		if (objects[indx-1].type == FIX)
		{
			for (init = 0; init < (int)strlen(objects[indx-1].id); init++)
			{
				if (!isalpha(objects[indx-1].id[init]))
				{
					indx--;
					break;
				}
			}
		}
	};
	total_objs = indx;
	navaids = 1;
	close(in);
}

// *************************************************
// The function called whenever a key is pressed.
// *************************************************
void keyPressed(unsigned char key, int x, int y)
{	
	RECT lprect;
	hwnd = GetActiveWindow();
    /* avoid thrashing this procedure */
	switch(key)
	{
		case '.':
			mouseptr^=1;
			ShowCursor(mouseptr);
			break;
// Switch to arc mode
		case 'A':
			arcmode = 1;
			break;
// Switch to rose mode
		case 'R':
			arcmode = 0;
			break;
		case 'r':
			rangerings^=1;
			break;
// Reduce the amount of blue in the background
		case 'b':
			cb -= 0.01;
			if (cb < 0.0)
				cb = 0;
			glClearColor(0.0, 0.0, cb, 0.0);
			break;
// Increase the amount of blue in the background
		case 'B':
			cb += 0.01;
			if (cb > 1.0)
				cb = 1.0;
			glClearColor(0.0, 0.0, cb, 0.0);
			break;
		case '1':
			range = 0;
			break;
		case '2':
			range = 1;
			break;
		case '3':
			range = 2;
			break;
		case '4':
			range = 3;
			break;
		case '5':
			range = 4;
			break;
		case '6':
			range = 5;
			break;
		case '7':
			range = 6;
			break;
// Toggle autopilot
		case 'l':
		case 'L':
			lnavactive ^= 1;
			break;
// Dim the display
		case '9':
			dimmer -= 0.1;
			break;
// Brighten the display
		case '0':
			dimmer += 0.1;
			break;
		case 'T':
			terrain^=1;
			break;
// Rotate the display 90 degrees
		case 't':
			tilt+=90.0;
			if (tilt > 270.0)
				tilt = 0.0;
			break;
// Show/Hide the frames per second
		case 'q':
		case 'Q':
			display_stats^=1;
			break;
// Make the window stick to the top.
		case '/':
			GetWindowRect(hwnd, &lprect);
			topmost ^= 1;
			if (topmost)
				SetWindowPos(hwnd, HWND_TOPMOST, (int)lprect.left, (int)lprect.top, (int)lprect.right - (int)lprect.left, 
					(int)lprect.bottom - (int)lprect.top, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
			else
				SetWindowPos(hwnd, HWND_NOTOPMOST, (int)lprect.left, (int)lprect.top, (int)lprect.right - (int)lprect.left, 
					(int)lprect.bottom - (int)lprect.top, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
			glutShowWindow();
			break;
// Show/Hide the window borders
		case 'f':	
		case 'F':
			GetWindowRect(hwnd, &lprect);

			wsl = GetWindowLong(hwnd, GWL_STYLE);
			wsl ^= WS_CAPTION;
			wsl ^= WS_THICKFRAME;
			wsl ^= WS_BORDER;
			if (!SetWindowLong(hwnd, GWL_STYLE, wsl))
			{
				MessageBox(0, "Error setting window settings", "ERROR", 0);
			}
			SetWindowPos(hwnd, HWND_TOP, (int)lprect.left, (int)lprect.top, (int)lprect.right - (int)lprect.left, 
				(int)lprect.bottom - (int)lprect.top, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
			glutShowWindow();
			windowframes ^= 1;
			break;
// Show/hide the navaid frequencies
		case 's':
		case 'S':
	        showfrq^=1;
			break;
// Show/hide fixes
		case 'i':
	        showfix^=1;
			break;
// Show/hide VOR's
		case 'v':
	        showvor^=1;
			break;
// Show/hide NDB's
		case 'n':
	        showndb^=1;
			break;
// Show/hide the traffic
		case 'c':
			TCAS_Active^=1;			
			break;
// Show/hide airports
		case 'a':
	        showapt^=1;
			break;
// Turn on VOR mode
		case 'V':
			mapmode = VORMODE;
			break;
// Turn on ILS mode
		case 'I':
			mapmode = ILSMODE;
			break;
// Turn on MAP mode
		case 'M':
			mapmode = MAPMODE;
			break;
// Toggle plan mode
		case 'P':
			mapmode = PLANMODE;
			break;
// Go to previous waypoint
		case '[':
			currentwpt--;
			if (currentwpt < 0)
				currentwpt = 0;
			break;
// Go to next waypoint
		case ']':
			currentwpt++;
			if (currentwpt > waypoints)
				currentwpt--;
			break;
// Toggle Heading/Track mode
		case 'H':
		case 'h':
			trackmode^=1;
			break;
// Decrease range
		case '-':
			range++;
			if (range > 6)
				range = 6;
			scale = ranges[range];
			ObjectTimer = 0;
			break;
// Increase range
		case '=':
		case '+':
			range--;
			if (range < 0)
				range = 0;
			scale = ranges[range];
			ObjectTimer = 0;
			break;
		default:	
			break;
	}
// Multipliers used to set range
	switch (range)
	{
		case 0:
			MAXRANGE=0.2;
			break;
		case 1:
			MAXRANGE=0.4;
			break;
		case 2:
			MAXRANGE=0.7;
			break;
		case 3:
			MAXRANGE=1.5;
			break;
		case 4:
			MAXRANGE=2.8;
			break;
		case 5:
			MAXRANGE=5.8;
			break;
		case 6:
			MAXRANGE=14.1;
			break;
	}
    if (key == ESCAPE)
    {
        exit(0);
    }
}

extern int DoReadPosition( HINSTANCE );

// *******************
// Main drawing loop
// *******************
void DrawGLScene(void)
{
	int 			intPart;
	double			radians;
	double			ws, wd, hd, gs, tas;
	static int		time, oldtime;
	static int		loops = 0;
	static double	pers = 0.0;
	static double	test = 0.0;
	RECT			lprect;
	double			hs = 0.0;
    GLUquadricObj	*quadObj1;

// Wait a bit
	Sleep(20);
// Get more FSUIPC data
	split_nd();

// Tilt the display if the user wants to
	glPushMatrix();
	glRotatef(tilt, 0.0, 0.0, 1.0);

// Calculate frames / second 
    oldtime = timeGetTime() - time;
	loops++;
	if (oldtime >= 1000)
	{
		time = timeGetTime();
		frames = loops;
		loops = 0;
	}

// First time through the loop
	if (firsttime)
	{
// Set the window frames per the users defaults
		if (!windowframes)
		{
			hwnd = GetActiveWindow();
			GetWindowRect(hwnd, &lprect);

			wsl = GetWindowLong(hwnd, GWL_STYLE);
			wsl ^= WS_CAPTION;
			wsl ^= WS_THICKFRAME;
			wsl ^= WS_BORDER;
			if (!SetWindowLong(hwnd, GWL_STYLE, wsl))
			{
				MessageBox(0, "Error setting window settings", "ERROR", 0);
			}
			SetWindowPos(hwnd, HWND_TOP, (int)lprect.left, (int)lprect.top, (int)lprect.right - (int)lprect.left, 
				(int)lprect.bottom - (int)lprect.top, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
			glutShowWindow();
		}
		firsttime = 0;
		if ((hglrc = wglGetCurrentContext()) == NULL)
			MessageBox(0,"Can't get the current Context","DEBUG",MB_OK);
		if ((hdc = wglGetCurrentDC()) == NULL)
			MessageBox(0,"Can't get the current DC","DEBUG",MB_OK);
		}

// Show the instrument INOP sign if Flight Simulator is not found
// and keep trying to connect to it 
	if (notconnected)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		setColor(1.0, 0.0, 0.0);
		CPuts(0.0, 0.0, 5.0, "INOP");
		drawbox(-16.0, 5.0, 12.0, -1.0, 0.0, 0);
		glutSwapBuffers();
		if (make_socket(1,NULL) == -1)
			notconnected = 1;
		else
			notconnected = 0;
		return;
	}

	if (!all.MASTER_BATTERY)
		{
		glClear(GL_COLOR_BUFFER_BIT);
		glutSwapBuffers();
		return;
		}


// Calculate our present latitude and longitude
    intPart = all.dwLatHi;
    if (intPart >= 0)
        lat = intPart + (double)all.dwLatLo / (double)0x100000000;
    else
        lat = intPart - (double)all.dwLatLo / (double)0x100000000;
    lat *= 45.0 / 5000875.0;
    intPart = all.dwLonHi;
    if (intPart >= 0)
        lon = intPart + (double)all.dwLonLo / (double)0x100000000;
    else
        lon = intPart - (double)all.dwLonLo / (double)0x100000000;
    lon *= 90.0 / (256.0 * 4194304.0);

// This is for the great circle calculations.
	radians = d2r(lat);
	sinlat = sin(radians);
	acossinlat = acos(sinlat);
	coslat = cos(radians);
	radians = d2r(lon);
	coslon = cos(radians);
	sinlon = sin(radians);

// Get the present heading and correct with Magnetic Variation
    hdg = 360.0*(double)all.dwHeading/(double)0x100000000;
    magvar = 360.0*(double)all.wMagVar / 65536.0;
    if (magvar > 180.0)
		magvar = magvar - 360.0;
    magvar *= -1.0;
		
	hdg+=magvar;

	if (hdg < 0.0)
		hdg += 360.0;
	if (hdg > 360.0)
		hdg -= 360.0;

// Calculate the Wind Correction Angle
	gs = groundspeed;
	hd = hdg;
	wd = 360.0*(double)all.wWindDirection/65536.0;
	ws = (double)all.wWindSpeed;
	tas = ((double)all.dwTAS/128.0);

	headwind = ws * cos (d2r(wd) - d2r(hd));
	crosswind = ws * sin (d2r(wd) - d2r(hd));
// Obviously no wind correction angle if we're on the ground.
	if (!all.bPlaneOnGround)
	{
		wca = atan2( ws*sin(d2r(hd)-d2r(wd)), tas - ws * cos(d2r(hd)-d2r(wd)) );
		wca = r2d(wca);
	}
	else
		wca = 0.0;

	track = hdg + wca;

// If we're in track mode,  add the WCA to the heading
	if (trackmode)
		hdg += wca;

	if (hdg < 0.0)
		hdg += 360.0;
	if (hdg > 360.0)
		hdg -= 360.0;

// Plan mode makes the navaid display static with north up.
	if (mapmode == PLANMODE)
	{
		hs = hdg;
		hdg = 0.0;
	}

//	if (terrain)
//		DrawGEO();

// Draw the navigation aids
	if (navaids)
		drawobjs();

// Reset the heading if we're in plan mode, the compass still depicts current heading
	if (mapmode == PLANMODE)      // Make sure the compass reflects heading at all times.
		hdg = hs;

// Set up the graphics window
	if (arcmode)
	{
		compassradius = 140.0;
		glPushMatrix();
		glTranslatef(0.0, -70.0, 0.0);
	}
	else
	{
		compassradius = 70;
	}

// Display the track/OBS line
	if (mapmode != PLANMODE)
	{
		glLineWidth(2.0);
		glPushMatrix();
		if (!trackmode)
			glRotatef(wca*-1.0, 0.0, 0.0, 1.0);
		glBegin(GL_LINES);
		glVertex2f(0.0, 0.0);
		glVertex2f(0.0, compassradius);
		glVertex2f(-2.0, compassradius / 2.0);
		glVertex2f( 2.0, compassradius / 2.0);
		glVertex2f(-2.0, compassradius / 4.0);
		glVertex2f( 2.0, compassradius / 4.0);
		glVertex2f(-2.0, (compassradius / 2.0) + (compassradius / 4.0));
		glVertex2f( 2.0, (compassradius / 2.0) + (compassradius / 4.0));
		glEnd();
		sprintf(display_buffer, "%.0f", rangemarkers[range]);
		RPuts(-4, (compassradius / 2.0) - 3.0, 8.0, display_buffer);
		glPopMatrix();
	}
// Draw the compass
	draw_compass();

// Draw the plane symbol
	setColor(1.0,1.0,1.0);
	if (arcmode)
// Simple triangle in arc mode
	{
		glBegin(GL_LINE_STRIP);
			glVertex2f(0.0, -70.0);
			glVertex2f(4.0,-81.0);
			glVertex2f(-4.0, -81.0);
			glVertex2f(0.0, -70.0);
		glEnd();
	}
	else
// Goofy looking plane in rose mode
	{
		glBegin(GL_LINES);
		glVertex2d(-1.5, -5.0);
		glVertex2d(-1.5, 5.0);
		glVertex2d(1.5, -5.0);
		glVertex2d(1.5, 5.0);

		glVertex2d(1.5, 0);
		glVertex2d(6.5, 0);
		glVertex2d(-1.5, 0);
		glVertex2d(-6.5, 0);
		glVertex2d(-1.5, -5.0);
		glVertex2d(-4.0, -5.0);
		glVertex2d(1.5, -5.0);
		glVertex2d(4.0, -5.0);
		glEnd();
	}

// Show the next flight plan waypoint if there is one.
// Optionally set the autopilot to the correct course.
	check_lnav();              

// Display the frame rate if the user wants it
	if (display_stats)
		{
		setColor(1.0,0,0);
		glRectf(60, 62, 85, 69);
		setColor(1,1,1);
		drawbox(60, 62, 85, 69, 0, 0);
		sprintf(display_buffer, "FPS: %03.0f", (double)frames);
		CPuts(72, 63, 6, display_buffer);
		}

// Display mode flags
	setColor(0.0, 0.8, 1.0);
	if (TCAS_Active)
		Puts(-95, -60.0, 7.0, "TFC", 0);
	if (showvor)
		Puts(-95, -55.0, 7.0, "VOR", 0);
	if (showndb)
		Puts(-95, -50.0, 7.0, "NDB", 0);
	if (showfix)
		Puts(-95, -45.0, 7.0, "FIX", 0);
	if (showapt)
		Puts(-95, -40.0, 7.0, "ARPT", 0);

// Display Range Rings

	if (arcmode && rangerings && (mapmode != PLANMODE))
	{
		quadObj1 = gluNewQuadric();
		gluQuadricDrawStyle(quadObj1, GLU_FILL);
		setColor(1,1,1);
		glPushMatrix();
		glTranslatef(0.0, -70.0, 0.0);
		compassradius = 140.0;
		gluPartialDisk(quadObj1, compassradius/2.0, compassradius/2.0+0.5, 180, 1, 270, 180);
		gluPartialDisk(quadObj1, compassradius/4.0, compassradius/4.0+0.5, 180, 1, 270, 180);
		gluPartialDisk(quadObj1, (compassradius / 2.0) + (compassradius / 4.0), (compassradius / 2.0) + (compassradius / 4.0)+0.5, 30, 1, 290, 140);
		glPopMatrix();
	}

// Reset the display from the TILT
	glPopMatrix();
// Reset the display from the WCA (Wind Correction Angle)
	glPopMatrix();
// Swap the back buffer to the screen
	glutSwapBuffers();
// Clear the back buffer
    glClear(GL_COLOR_BUFFER_BIT);
}

// ******************
// GLUT callback
// ******************
void UpdateDisplay(void)
{
   	glutPostRedisplay();
}

// ******************************
// Initialize OpenGL settings
// ******************************
void init(void) 
{
    size = sizeof(Sa);
	ranges[0] = 1700.0;
	ranges[1] = 850.0;
	ranges[2] = 425.0;
	ranges[3] = 213.0;
	ranges[4] = 106.0;
	ranges[5] = 53.0;
	ranges[6] = 26.0;
	ranges[7] = 13.0;
	scale = ranges[1];
	range = 1;
   	glClearColor (0.0, 0.2, 0.0, 0.0);
   	glShadeModel (GL_SMOOTH);

// Enable full anti-aliasing
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
//	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
}

// ***************************************
// GLUT callback for window resizing
// ***************************************
void reshape(int w, int h)
{
    width = w; height = h; 
   	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
   	glMatrixMode(GL_PROJECTION);
   	glLoadIdentity();
   	gluOrtho2D(	((WindowSize+10.0) * -1.0) - padding, (WindowSize+10.0) + padding, 
				(WindowSize * -1.0) - padding, WindowSize + padding);
   	glMatrixMode(GL_MODELVIEW);
   	glLoadIdentity();
}

// *************************************
// Main entry point to the program
// *************************************
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, INT nCmdShow )
{
	FILE		*geometry;
	GLuint		textureid;
	char		*token;
//	int			ff = 0;
	char		buffer[200];

	wx = 100;
	wy = 100;
	width = 650;
	height = 650;

	hInstance = hInst;
	strcpy(revision, "0.1 (beta)");

// Load up the navaids
	initstruct();

	NDGetRegistry();

   	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   	glutInitWindowSize (width, height); 
   	glutInitWindowPosition (wx, wy);
   	sprintf(display_buffer, "Ellie Systems ND %s %s", revision, subversion);
   	window = glutCreateWindow (display_buffer);
   	init ();
   	glutDisplayFunc(DrawGLScene); 
   	glutReshapeFunc(reshape); 
   	glutIdleFunc(UpdateDisplay);
   	glutKeyboardFunc(keyPressed);
	glutOverlayDisplayFunc(NULL);
	glutMouseFunc(NULL);
	glutMotionFunc(NULL);
	glutPassiveMotionFunc(NULL);
	glutVisibilityFunc(NULL);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStatusFunc(NULL);
// Connect to Flight Simulator
   	if (make_socket(PORT, ipaddr) == -1)
		notconnected = 1;
	hdc = wglGetCurrentDC();
//	BuildFont();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_2D);
	atexit(getoutofdodge);

// Load the OpenGL font texture
	load_font(NULL);

	glEnable      ( GL_TEXTURE_2D );
	glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );
	glGenTextures ( 1, &textureid );
// Make sure out first loop is special
	firsttime = 1;
// Start the main program loop,  never to return
	open_log("nd.log");
   	glutMainLoop();
   	return 0;   /* ANSI C requires main to return int. */
}

int StoreObjsInRange(double range)
{
	double lat,lon;
	int intPart;
	double radians;
	long navaid;
	double cosdis,sindis;
	char problem = 0;

	idxcnt = 0;
// -----------------------------------------------
// Calculate our present latitude and longitude
// -----------------------------------------------

    intPart = all.dwLatHi;
    if (intPart >= 0.0)
        lat = (double)intPart + (double)all.dwLatLo / 4294967296.0;
    else
        lat = intPart - (double)all.dwLatLo / 4294967296.0;
    lat *= 45.0 / 5000875.0;
    intPart = all.dwLonHi;
    if (intPart >= 0.0)
        lon = (double)intPart + (double)all.dwLonLo / 4294967296.0;
    else
        lon = (double)intPart - (double)all.dwLonLo / 4294967296.0;
    lon *= 90.0 / (256.0 * 4194304.0);

// -------------------------------------------
// This is for the great circle calculations.
// -------------------------------------------

	radians = d2r(lat);
	sinlat = sin(radians);
	acossinlat = acos(sinlat);
	coslat = cos(radians);

	radians = d2r(lon);
	coslon = cos(radians);
	sinlon = sin(radians);

	for (navaid = 0; navaid < indx; navaid++)	
	{
// --------------------------------------------------------------
// Simple range restricting, we'll fine tune the rest later on. 
// Gives us a quick guess as to what will be in view.
// --------------------------------------------------------------

		if (lat - objects[navaid].lat > MAXRANGE)
			continue;
		else if (objects[navaid].lat - lat > MAXRANGE)
			continue;
		if ((lon) - objects[navaid].lon > MAXRANGE)
			continue;
		else if (objects[navaid].lon - (lon) > MAXRANGE)
			continue;
// --------------------
// Great circle math
// --------------------

		distance = acos(sinlat * objects[navaid].sinlat + coslat * 
			objects[navaid].coslat * cos(d2r(lon)-objects[navaid].lonrad));

		cosdis = cos(distance);
		sindis = sin(distance);

// -----------------------------
// Is this navaid out of range?
// -----------------------------

		if (r2distance(distance) > range)
			continue;

		if (idxcnt < MAXINRANGE)
			objidx[idxcnt++] = navaid;
		else
			problem = 1;
	}

	if (problem)
	{
		log_this("NAVAIDS LIST OVERRUN! (%ld MAXIMUM)\n", MAXINRANGE);
		idxcnt = MAXINRANGE; // Clamp it
	}

	return(1);
}


// ******************************
// Draw the requested navaids
// ******************************
void drawobjs(void)
{
	int			rc;
	double		x, y;
	double		navrange;
	long 		o;
	char 		appendage[10];
	int			ad = 0;

	setColor(1.0,1.0,1.0);

	glPushMatrix();
	if (arcmode)
		glTranslatef(0.0, -70.0, 0.0);

	if ((timeGetTime() - ObjectTimer) > 20000)
		{
		ObjectTimer = timeGetTime();
		StoreObjsInRange(rangemarkers[range]*4.0);
		}

//	for (o = 0; o < indx; o++)
	for (long loop = 0; loop < idxcnt; loop++)
	{
		o = objidx[loop];
// Simple range restricting, we'll fine tune the rest later on. Gives us a quick guess as to what will be in view.
		if (lat - objects[o].lat > MAXRANGE)
			continue;
		else if (objects[o].lat - lat > MAXRANGE)
			continue;
		if ((lon) - objects[o].lon > MAXRANGE)
			continue;
		else if (objects[o].lon - (lon) > MAXRANGE)
			continue;

// Great circle math
		distance = acos(sinlat * objects[o].sinlat + coslat * 
			objects[o].coslat * cos(d2r(lon)-objects[o].lonrad));

		if (sin(objects[o].lonrad - d2r(lon)) < 0)
			bearing = acos((objects[o].sinlat - sinlat * cos(distance)) / 
				(sin(distance) * coslat));
		else
			bearing = 2.0 * MYPI - acos((objects[o].sinlat - sinlat * cos(distance)) / 
				(sin(distance) * coslat)); 

		distance *= ((180.0*60.0)/MYPI);

// Is this navaid out of range?
		if (distance > rangemarkers[range]*4.0)
			continue;


// Set the correct navaid colors if this one is not part of our flight plan
		if (!(rc = fpwpt(objects[o].id)))
		{
			if (!stricmp(objects[o].id, all.vor1_id))
				setColor(1.0, 1.0, 0.0);
			else if (!stricmp(objects[o].id, all.vor2_id))
				setColor(1.0, 1.0, 0.0);
			else if (objects[o].type == APT) 
			{
				if (!showapt)
					continue;
				setColor(0.0, 0.8, 1.0);
			}
			else if (objects[o].type == VOR)
			{
				if (!showvor)
					continue;
				setColor(0.0, 1.0, 0.0);
			}
			else if (objects[o].type == NDB)
			{
				if (!showndb)
					continue;
				setColor(0.0, 0.0, 1.0);
			}
			else if (objects[o].type == FIX)
			{
			if (!showfix)
				continue;
			if (range > 3)
				continue;
			else
				setColor(0.5, 0.5, 0.0);
			}
			else 
				continue;
		}
		else
		{
// This navaid is in our flight plan,  draw it white,  or magenta (If its the next waypoint)
			if ((objects[o].type != APT) &&
				(objects[o].type != VOR) &&
				(objects[o].type != NDB) &&
				(objects[o].type != FIX))
				continue;
			if (rc == 1)
				setColor(1,1,1);
			else
				setColor(1,0,1);
		}

		bearing = (360.0-r2d(bearing))+magvar;
		bearing = fabs(hdg) - bearing;
		bearing -= 270.0;
		if (bearing < 0.0)
			bearing += 360.0;

		navrange = distance / (rangemarkers[range]*2.0);

		navrange *= compassradius;

		x = cos((MYPI / 180.0) * bearing) * navrange;
		y = sin((MYPI / 180.0) * bearing) * navrange;

// ACTUAL DRAWING

// Do we want to show frequencies?
		strcpy(appendage, "");
		if (showfrq)
		{
			if (((showvor) && (objects[o].type == VOR)) || ((showndb) && (objects[o].type == NDB)))
			{
				sprintf(appendage, " (%.2f)", objects[o].freq);	
			}
		}
		else
			strcpy(appendage, "");
// Display the navaid ID
		sprintf(display_buffer, "%s%s", objects[o].id, appendage);
		Puts(x+3.0, y-5.5, 7.0, display_buffer, 0);

// Display the navaid symbol
		if (objects[o].type == VOR)
		{
			sprintf(display_buffer, "%c",VORSYM);
			CPuts(x, y-4.5, 11.0, display_buffer);
		}
		else if (objects[o].type == APT)
		{
			sprintf(display_buffer, "%c",APTSYM);
			CPuts(x, y-4.5,11.0, display_buffer);
		}
		else if (objects[o].type == FIX)
		{
			sprintf(display_buffer, "%c",FIXSYM);
			CPuts(x, y-4.5, 11.0, display_buffer);
		}
		else if (objects[o].type == NDB)
		{
			sprintf(display_buffer, "%c",NDBSYM);
			CPuts(x, y-4.5, 11.0, display_buffer);
		}
	}

// Do we want to display traffic? (TCAS)
	if (TCAS_Active)
	{
		setColor(0.0, 0.8, 1.0);
		Puts(-90, (compassradius*-1.0) - 10.0, 4.0, "TFC", 0);
		setColor(1,1,1);
		for (o = 0 ; o < 196; o ++)
		{
			if (tcas[o].id == 0)
				continue;

// Great circle math
			distance = acos(sinlat * sin(d2r(tcas[o].lat)) + coslat * 
				cos(d2r(tcas[o].lat)) * cos(d2r(lon)-d2r(tcas[o].lon)));
	
			if (sin(d2r(tcas[o].lon) - d2r(lon)) < 0)
				bearing = acos((sin(d2r(tcas[o].lat)) - sinlat * cos(distance)) / 
					(sin(distance) * coslat));
			else
				bearing = 2.0 * MYPI - acos((sin(d2r(tcas[o].lat)) - sinlat * cos(distance)) / 
					(sin(distance) * coslat)); 

			distance *= ((180.0*60.0)/MYPI);

// Is this plane out of range?
			if (distance > rangemarkers[range]*2.0)
				continue;

			bearing = (360.0-r2d(bearing))+magvar;
			bearing = fabs(hdg) - bearing;
			bearing -= 270.0;
			if (bearing < 0.0)
				bearing += 360.0;

			navrange = distance / (rangemarkers[range]*2.0);

			navrange *= compassradius;

			x = cos((MYPI / 180.0) * bearing) * navrange;
			y = sin((MYPI / 180.0) * bearing) * navrange;

// This is used to display the traffic altitude relative to ours.
			if (tcas[o].alt > alt)
			{
				ad = (tcas[o].alt - alt) / 100;
				if (ad > 100)
					continue;
			}
			else if (alt > tcas[o].alt)
			{
				ad = ((alt - tcas[o].alt) / 100) * -1;
				if (ad < -100)
					continue;
			}

// Past 6nm it's a hollow diamond
			if (distance > 6.0)
			{
				sprintf(display_buffer, "%s%02d", ad > 0?"+":"", ad);
				Puts(x-5, y-7, 6, display_buffer,0);
// traffic climbing in excess of 500 ft/min
				if (tcas[o].vs > 500.0)
				{
					sprintf(display_buffer, "%c%c", TCASFAR, TCASUP);
					Puts(x-5, y-2, 6, display_buffer,0);
				}
// traffic descending in excess of 500 ft/min
				else if (tcas[o].vs < -500.0)
				{
					sprintf(display_buffer, "%c%c", TCASFAR, TCASDN);
					Puts(x-5, y-2, 6, display_buffer,0);
				}
				else
				{
					sprintf(display_buffer, "%c", TCASFAR);
					Puts(x-5, y-2, 6, display_buffer,0);
				}
			}
			else
// close in traffic is a full diamond
			{
				if ((ad < 12) && (ad > -12))
				{
					sprintf(display_buffer, "%s%02d", ad > 0?"+":"", ad);
					Puts(x-5, y-7, 6, display_buffer,0);
// traffic climbing in excess of 500 ft/min
					if (tcas[o].vs > 500.0)
					{
						sprintf(display_buffer, "%c%c", TCASNEAR, TCASUP);
						Puts(x-5, y-2, 6, display_buffer,0);
					}
// traffic descending in excess of 500 ft/min
					else if (tcas[o].vs < -500.0)
					{
						sprintf(display_buffer, "%c%c", TCASNEAR, TCASDN);
						Puts(x-5, y-2, 6, display_buffer,0);
					}
					else
					{
						sprintf(display_buffer, "%c", TCASNEAR);
						Puts(x-5, y-2, 6, display_buffer,0);
					}
				}
				else
				{
					sprintf(display_buffer, "%s%02d", ad > 0?"+":"", ad);
					Puts(x-5, y-7, 6, display_buffer,0);
// traffic climbing in excess of 500 ft/min
					if (tcas[o].vs > 500.0)
					{
						sprintf(display_buffer, "%c%c", TCASFAR, TCASUP);
						Puts(x-5, y-2, 6, display_buffer,0);
					}
// traffic descending in excess of 500 ft/min
					else if (tcas[o].vs < -500.0)
					{
						sprintf(display_buffer, "%c%c", TCASFAR, TCASDN);
						Puts(x-5, y-2, 6, display_buffer,0);
					}
					else
					{
						sprintf(display_buffer, "%c", TCASFAR);
						Puts(x-5, y-2, 6, display_buffer,0);
					}
				}
			}
		}
	}
	glPopMatrix();
	setColor(1,1,1);
// Re-read the flight plan in case it changes (MAP and PLAN mode are the only ones to show the flightplan)
	if ((mapmode == PLANMODE) || (mapmode == MAPMODE))
		read_sbp_flightplan();
	return;
}

// ***********************
// Draw the compass
// ***********************
void draw_compass(void)
{
	double bx = 77.0;
	double by;
	double wd, rwd;
	double pd;
	double bug = 70.0;

// Draw the compass face
	glPushMatrix();
	draw_compassface(arcmode^1);
	glPopMatrix();

// Display the compass heading.
	setColor(1,1,1);
	if (trackmode)
		sprintf(display_buffer, "%03.0f", floor(fabs(hdg-wca)));
	else
		sprintf(display_buffer, "%03.0f", floor(fabs(hdg)));
	CPuts(0, compassradius+5.0, 12, display_buffer);

	if (arcmode)
		glPopMatrix();	

// DISPLAY THE ADF
	setColor(0,1,1);
    adf = ((double)all.wAdfNeedle * 360.0) / 65536.0;
    Nav1Rad = ((double)(all.wNav1Rad * 360) / 65536.0) - 180.0;
	if (Nav1Rad < 0.0)
		Nav1Rad += 360.0;
    Nav2Rad = ((double)(all.wNav2Rad * 360) / 65536.0) - 180.0;
	if (Nav2Rad < 0.0)
		Nav2Rad += 360.0;
	if (all.adf_id[0])
	{
		Puts(-95, -66, 7, all.adf_id, 0);
	}
	sprintf(display_buffer, "%.0f", adf);
	Puts(-95, -71, 7, display_buffer, 0);

// Display Groundspeed and True Airspeed
	groundspeed = sqrt(
		( (double)all.wWindSpeed * (double)all.wWindSpeed ) + 
		( (((double)all.dwTAS/128) * ((double)all.dwTAS/128))) - 
		(2 * (double)all.wWindSpeed * ((double)all.dwTAS/128.0) * 
		cos((hdg*(MYPI/180.0)) - ((360.0*(double)all.wWindDirection/(double)0x100000000)) * (MYPI/180.0))) 
		);

	setColor(1,1,1);
	Puts(-95, 81, 6.0, "GS", 0);
	Puts(-76, 81, 6.0, "TAS", 0);

	sprintf(display_buffer, "%ld", (int)(((double)all.dwGS/65536.0)*1.943846));
	Puts(-88, 80.5, 8.0, display_buffer, 0);
	if (!(all.dwTAS/128))
		strcpy(display_buffer, "---");
	else
		sprintf(display_buffer, "%ld", all.dwTAS/128);
	Puts(-67, 80.5, 8.0, display_buffer, 0);

// Display the relative wind direction and pointer
	if (all.wWindSpeed != 0)
	{
		glPushMatrix();
		glTranslatef(-87, 67, 0);
		wd = (360.0*(double)all.wWindDirection/65536.0);
		if (wd < 0.0)
			wd+=360.0;
		pd = hdg;
		rwd = pd - wd;
		if (rwd < 0.0)
			rwd = 360.0+rwd;
		rwd -= 180.0;
		if (rwd < 0.0)
			rwd = 360.0+rwd;
		glRotatef(rwd, 0.0, 0.0, 1.0);
		setColor(1,1,1);
		glBegin(GL_LINES);
		glVertex2f(0.0, -6.0);
		glVertex2f(0.0, 6.0);
		glVertex2f(-1.5, 2.5);
		glVertex2f(0.0, 6.0);
		glVertex2f(1.5, 2.5);
		glVertex2f(0.0, 6.0);
		glEnd();
		glPopMatrix();
		setColor(1,1,1);
		sprintf(display_buffer, "%03.0f%c / %d", wd, DEGREE, all.wWindSpeed);
		Puts(-95, 74, 8.0, display_buffer, 0);
	}

// DISPLAY THE ILS/VOR DATA for the left nav radio
	setColor(0.0, 1.0, 0.0);
	if (all.bNav1BCFlags == 1)             // ILS??
		Puts(-95.0,-76.0, 7.0, "ILS L", 0);
	else if (all.bNav1ToFrom == 1)             // VOR??
		Puts(-95.0,-76.0, 7.0, "VOR L TO", 0);
	else if (all.bNav1ToFrom == 2)             // VOR??
		Puts(-95.0,-76.0, 7.0, "VOR L FR", 0);

	if (all.vor1_id[0])
	{
		Puts(-95.0, -81.0, 7, all.vor1_id, 0);
	}
	
	all.sDME1[4] = '\0';
	if (!strcmp(all.sDME1, "-1.0"))
		strcpy(display_buffer, "DME -----");
	else
	{
		sprintf(display_buffer, "DME %s", all.sDME1);
	}
	Puts(-95, -86, 7, display_buffer, 0);

// DISPLAY THE ILS/VOR DATA for the right nav radio
	if (all.bNav2BCFlags == 1)             // ILS??
		Puts(65, -76, 7, "ILS R", 0);
	else if (all.bNav2ToFrom == 1)             // VOR??
		Puts(65, -76, 7, "VOR R TO", 0);
	else if (all.bNav2ToFrom == 2)             // VOR??
		Puts(65, -76, 7, "VOR R FR", 0);

	if (all.vor2_id[0])
	{
		Puts(65, -81, 7, all.vor2_id, 0);
	}

	setColor(0,1,0);
	all.sDME2[4] = '\0';
	if (!strcmp(all.sDME2, "-1.0"))
		strcpy(display_buffer, "DME -----");
	else
	{
		sprintf(display_buffer, "DME %s", all.sDME2);
	}
	Puts(65, -86, 7, display_buffer, 0);

// Display the heading/track mode setting
	setColor(0.0, 0.8, 0.0);
	if (trackmode)
		RPuts(-11.0, 77.0, 8.0, "TRK");
	else
		RPuts(-11.0, 77.0, 8.0, "HDG");
	Puts(11.0, 77.0, 8.0, "MAG", 0);
	
	if (arcmode)
	{
		glPushMatrix();
		glTranslatef(0.0, -40.0, 0.0);
	}

	if (!arcmode)
	{
// If nav 1 is set to an ILS and it's active
		if (all.NAVAID_FLAGS & NAV1_IS_ILS)
		{		
			glLineWidth(2.0);
			by = -1.0*((double)all.bGlideSlopeNeedle/3.175);
			bx = 80.0;
			setColor(1,1,0);
			glBegin(GL_LINES);
			glVertex3f(bx-2.0,0,0);
			glVertex3f(bx+2.0,0,0);
			glEnd();

			sprintf(display_buffer, "%c", APTSYM);
// This displays the localizer offsets
			setColor(1.0,1.0,1.0);
			CPuts(bx, 36, 6, display_buffer);      // These are actually little circles
			CPuts(bx, 16, 6, display_buffer);
			CPuts(bx, -20, 6, display_buffer);
			CPuts(bx, -40, 6, display_buffer);

			setColor(0.8, 0, 0.8);
			glBegin(GL_LINE_STRIP);
			glVertex2f(bx, by-2.0);
			glVertex2f(bx+2.0, by);
			glVertex2f(bx, by+2.0);
			glVertex2f(bx-2.0, by);
			glVertex2f(bx, by-2.0);
			glEnd();
		}
	}
	if (arcmode)
	{
		glPopMatrix();
	}

// These are the lines around the heading display
	glLineWidth(2.0);
	setColor(1.0, 1.0, 1.0);
	glBegin(GL_LINE_STRIP);
		glVertex2f(9.0, 85.0);
		glVertex2f(9.0, 75.0);
		glVertex2f(-9.0, 75.0);
		glVertex2f(-9.0, 85.0);
	glEnd();

// If we're in track mode, rotate the compass for the wind correction angle
	if (trackmode)               
	{
		if (arcmode)
		{
			glPushMatrix();
			glTranslatef(0.0, -70.0, 0.0);
			bug = 140.0;
		}

		glPushMatrix();
		glRotatef(wca, 0.0, 0.0, 1.0);
	}

// Draw the heading bug
	glBegin(GL_LINE_STRIP);
	glVertex2f(0.0, bug);
	glVertex2f(-2.5, bug+5.0);
	glVertex2f(2.5, bug+5.0);
	glVertex2f(0.0, bug);
	glEnd();

	if (trackmode)               
	{
		glPopMatrix();
		if (arcmode)
			glPopMatrix();
	}
	return;
}

#if 0
// *******************
// Draw the runways
// *******************
int draw_runways()
{
	FILE *		rw;
	char 		rwbuf[100];

	char *		airport;

	char *		lata, *latb;
	char *		lona, *lonb;
	char *		ilsa, *ilsb;
	char *		taga, *tagb;
	char *		obsa, *obsb;
	int 		init = 0;

	return(0);
	if (runways == -1)
		return(0);

/*-----------------*/
	if (!runways)
	{
		rw = fopen("runways.dat", "r");
		if (rw == NULL)
		{
			runways = -1;
			return(0);
		}
		runways = 0;
		while (fgets(rwbuf, 99, rw))
		{
			if (rwbuf[0] == ';')
				continue;
			airport = strtok(rwbuf, ":");
			if (airport == NULL)
			{
				runways = 0;
				MessageBox(0, "Error in runway data", "FreeFD ND", 0);
				return(0);
			}
			lata = strtok(NULL, ":");
			if (lata == NULL)
			{
				runways = 0;
				MessageBox(0, "Error in runway data", "FreeFD ND", 0);
				return(0);
			}
			runway[runways].ay = atof(lata);
			lona = strtok(NULL, ":");
			if (lona == NULL)
			{
				runways = 0;
				MessageBox(0, "Error in runway data", "FreeFD ND", 0);
				return(0);
			}
			runway[runways].ax = atof(lona);
			latb = strtok(NULL, ":");
			if (latb == NULL)
			{
				runways = 0;
				MessageBox(0, "Error in runway data", "FreeFD ND", 0);
				return(0);
			}
			runway[runways].by = atof(latb);
			lonb = strtok(NULL, ":");
			if (lonb == NULL)
			{
				runways = 0;
				MessageBox(0, "Error in runway data", "FreeFD ND", 0);
				return(0);
			}
			runway[runways].bx = atof(lonb);
			ilsa = strtok(NULL, ":");
			if (ilsa == NULL)
			{
				runways = 0;
				MessageBox(0, "Error in runway data", "FreeFD ND", 0);
				return(0);
			}
			strcpy(runway[runways].ilsa, ilsa);
			ilsb = strtok(NULL, ":");
			if (ilsb == NULL)
			{
				runways = 0;
				MessageBox(0, "Error in runway data", "FreeFD ND", 0);
				return(0);
			}
			strcpy(runway[runways].ilsb, ilsb);
			taga = strtok(NULL, ":");
			if (taga == NULL)
			{
				runways = 0;
				MessageBox(0, "Error in runway data", "FreeFD ND", 0);
				return(0);
			}
			strcpy(runway[runways].sa, taga);
			tagb = strtok(NULL, ":");
			if (tagb == NULL)
			{
				runways = 0;
				MessageBox(0, "Error in runway data", "FreeFD ND", 0);
				return(0);
			}
			strcpy(runway[runways].sb, tagb);
			obsa = strtok(NULL, ":");
			if (obsa == NULL)
			{
				runways = 0;
				MessageBox(0, "Error in runway data", "FreeFD ND", 0);
				return(0);
			}
			strcpy(runway[runways].obsa, obsa);
			obsb = strtok(NULL, ":");
			if (obsb == NULL)
			{
				runways = 0;
				MessageBox(0, "Error in runway data", "FreeFD ND", 0);
				return(0);
			}
			strcpy(runway[runways].obsb, obsb);
			runways++;
		}
		fclose(rw);
	}
	else
	{
		setColor(0.5, 0.5, 0.5);
		init = 0;
		for (init = 0; init < runways ; init++)
		{
			if (lat - runway[init].ay > 1.0)
				continue;
			else if (runway[init].ay - lat > 1.0)
				continue;
	
			if (lon - runway[init].ax > 1.0)
				continue;
			else if (runway[init].ax - lon > 1.0)
				continue;
	
			if (lat - runway[init].by > 1.0)
				continue;
			else if (runway[init].by - lat > 1.0)
				continue;
	
			if (lon - runway[init].bx > 1.0)
				continue;
			else if (runway[init].bx - lon > 1.0)
				continue;
			glBegin(GL_LINES);
				glVertex2f(runway[init].ax, runway[init].ay);
				glVertex2f(runway[init].bx, runway[init].by);
			glEnd();
		}
	}
	return(1);
}

#endif // #if 0

// ******************************
// Get fresh data from FS
// ******************************
char *split_nd(void)
{
	DWORD result;
	int init;
	char GSlots[96];
	char ASlots[96];
	char OLD_ASlots[96];
	char achanged = 0;
//	char gchanged = 0;
// Set up all the reads we need
	Data_Read(0x023b, 1,					&all.byZuluHour,			&result);
	Data_Read(0x023c, 1,					&all.byZuluMin,				&result);
	Data_Read(0x3300, sizeof(BOOL16),		&all.NAVAID_FLAGS,			&result);
	Data_Read(0x0c29, 5,					&all.sDME1,					&result);
	Data_Read(0x0c33, 5,					&all.sDME2,					&result);
	Data_Read(0x0350, sizeof(WORD),			&all.wBcdNav1Freq,			&result);
	Data_Read(0x0c6a, sizeof(WORD),			&all.wAdfNeedle,			&result);
	Data_Read(0x3000, 6,					&all.vor1_id,				&result);
	Data_Read(0x301f, 6,					&all.vor2_id,				&result);
	Data_Read(0x303e, 6,					&all.adf_id,				&result);
	Data_Read(0x0564, sizeof(DWORD),		&all.dwLatHi,				&result);
	Data_Read(0x0560, sizeof(DWORD),		&all.dwLatLo,				&result);
	Data_Read(0x056c, sizeof(DWORD),		&all.dwLonHi,				&result);
	Data_Read(0x0568, sizeof(DWORD),		&all.dwLonLo,				&result);
	Data_Read(0x0580, sizeof(DWORD),		&all.dwHeading,				&result);
	Data_Read(0x02a0, sizeof(WORD),			&all.wMagVar,				&result);
	Data_Read(0x07cc, sizeof(DWORD),		&all.dwAPHeading,			&result);
	Data_Read(0x0c4e, sizeof(WORD),			&all.wNav1Obs,				&result);
	Data_Read(0x0c4a, sizeof(BYTE),			&all.bNav1BCFlags,			&result);
	Data_Read(0x0c4b, sizeof(BYTE),			&all.bNav1ToFrom,			&result);
	Data_Read(0x0E90, sizeof(WORD),			&all.wWindSpeed,			&result);
	Data_Read(0x0E92, sizeof(WORD),			&all.wWindDirection,		&result);
	Data_Read(0x02b8, sizeof(DWORD),		&all.dwTAS,					&result);
	Data_Read(0x02bc, sizeof(DWORD),		&all.dwIAS,					&result);
	Data_Read(0x02b4, sizeof(DWORD),		&all.dwGS,					&result);
	Data_Read(0x0c5a, sizeof(BYTE),			&all.bNav2BCFlags,			&result);
	Data_Read(0x0c5b, sizeof(BYTE),			&all.bNav2ToFrom,			&result);
	Data_Read(0x0c60, sizeof(WORD),			&all.wNav2Rad,				&result);
	Data_Read(0x0c50, sizeof(WORD),			&all.wNav1Rad,				&result);
	Data_Read(0x0c48, sizeof(BYTE),			&all.bLocalizerNeedle,		&result);
	Data_Read(0x0c49, sizeof(BYTE),			&all.bGlideSlopeNeedle,		&result);
	Data_Read(0x0366, 1,					&all.bPlaneOnGround,		&result);
	Data_Read(0x0570, 4,					&all.dwAltLo,				&result);
	Data_Read(0x0574, 4,					&all.dwAltHi,				&result);
	Data_Read(0xe008, 96,					GSlots,						&result);
	Data_Read(0xf008, 96,					ASlots,						&result);
	Data_Read(0x6d00, 1,					&all.byNDEFIS,				&result);
	Data_Read(0x6d01, 1,					&all.byNDLeft,				&result);
	Data_Read(0x6d02, 1,					&all.byNDRight,				&result);
	Data_Read(0x6d03, 1,					&all.byNDRange,				&result);
	Data_Read(0x281c, 4,					&all.MASTER_BATTERY,		&result);
	Data_Read(0x6d0a, 1,					&all.byDimmer,				&result);

	dimmer = (double)all.byDimmer / 255.0;

	dimmer = 1.0;
// send the data request
	if (Data_Process(&result) == false)
	{
// Something broke, assume we're no longer connected
		notconnected = 1;
		Data_Close();
	}

	if ( (all.byNDEFIS != 0) ||
		 (all.byNDLeft != 0) ||
		 (all.byNDRight != 0) ||
		 (all.byNDRange != 0) )
	{
// Arc/Rose mode
		if (all.byNDRight & ND_CTR)
			arcmode = 0;
		else
			arcmode = 1;

// Set the map mode

		if (all.byNDRight & ND_APP)
			mapmode = ILSMODE;
		else if (all.byNDRight & ND_PLAN)
			mapmode = PLANMODE;
		else if (all.byNDRight & ND_MAP)
			mapmode = MAPMODE;
		else if (all.byNDRight & ND_VOR)
			mapmode = VORMODE;

//  Set the range

		if (all.byNDRange & ND_R10)
			range = 0;
		else if (all.byNDRange & ND_R20)
			range = 1;
		else if (all.byNDRange & ND_R40)
			range = 2;
		else if (all.byNDRange & ND_R80)
			range = 3;
		else if (all.byNDRange & ND_R160)
			range = 4;
		else if (all.byNDRange & ND_R320)
			range = 5;
		else if (all.byNDRange & ND_R640)
			range = 6;
// Multipliers used to set range
		switch (range)
		{
			case 0:
				MAXRANGE=0.2;
				break;
			case 1:
				MAXRANGE=0.4;
				break;
			case 2:
				MAXRANGE=0.7;
				break;
			case 3:
				MAXRANGE=1.5;
				break;
			case 4:
				MAXRANGE=2.8;
				break;
			case 5:
				MAXRANGE=5.8;
				break;
			case 6:
				MAXRANGE=14.1;
				break;
		}

// Set the navaid display
		if (all.byNDEFIS & ND_STA)
			showfix = showvor = showndb = 1;
		else
			showfix = showvor = showndb = 0;
		if (all.byNDEFIS & ND_ARPT)
			showapt = 1;
		else
			showapt = 0;
		if (all.byNDEFIS & ND_TFC)
			TCAS_Active = 1;
		else
			TCAS_Active = 0;
	}
// The VOR id's are not null terminated,  fix that right now
	for (init = strlen(all.vor1_id)-1; init > 0; init--)
	{
		if (!isalnum(all.vor1_id[init]))
			all.vor1_id[init] = '\0';
		else
			break;
	}
	for (init = strlen(all.vor2_id)-1; init > 0; init--)
	{
		if (!isalnum(all.vor2_id[init]))
			all.vor2_id[init] = '\0';
		else
			break;
	}

	if (TCAS_Active)
	{
		for (init = 0; init < 96; init++)
		{
			if (ASlots[init] != OLD_ASlots[init])
			{
				Data_Read(0xf080+(init * sizeof(TCAS_DATA)), 40,	&tcas[init],					&result);
				achanged++;
				OLD_ASlots[init] = ASlots[init];
			}
		}
// send the data request
		if (Data_Process(&result) == false)
		{
		}
	}

// DEBUG junk
#if 0
	sprintf(display_buffer, "SLOTS CHANGED:  G: %d A: %d %s", gchanged, achanged, tcas[0].idATC);
	setColor(1,1,1);
	Putsx(10, 0, 4, display_buffer);
#endif

// Altitude
	alt = ((double)all.dwAltHi +(double)all.dwAltLo / (double)0x100000000) * 3.2808399;
	return((char *)&all);
}

void getoutofdodge(void)
{	
	FILE *geometry;
	RECT lprect;
// get rid of font display lists 
//	KillFont();

// Restore the window borders before we save the geometry
	if (!windowframes)
	{
		hwnd = GetActiveWindow();
		GetWindowRect(hwnd, &lprect);

		wsl = GetWindowLong(hwnd, GWL_STYLE);
		wsl ^= WS_CAPTION;
		wsl ^= WS_THICKFRAME;
		wsl ^= WS_BORDER;
		if (!SetWindowLong(hwnd, GWL_STYLE, wsl))
		{
			MessageBox(0, "Error setting window settings", "ERROR", 0);
		}
		SetWindowPos(hwnd, HWND_TOP, (int)lprect.left, (int)lprect.top, (int)lprect.right - (int)lprect.left, 
			(int)lprect.bottom - (int)lprect.top, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
		glutShowWindow();
	}

// SAVE THE CURRENT WINDOW GEOMETRY FOR NEXT TIME
	wx = glutGet(GLUT_WINDOW_X);
	wy = glutGet(GLUT_WINDOW_Y);

	NDSetRegistry();

// Shut down FS Link 
	Data_Close();
	wglDeleteContext(hrc);
	glutDestroyWindow(window);
}
