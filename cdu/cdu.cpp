/*======================================================================
** Main CDU code
** Copyright (c) 2005 Don Lafontaine
** ALL RIGHTS RESERVED
** By Don Lafontaine
**----------------------------------------------------------------------     
**
** DATE					COMMENTS						WHO
**----------------------------------------------------------------------
** 2005/01				Added comment header			Don
**====================================================================*/
#include <windows.h>
#define MSFS 1
extern HDC		hdc;
extern WSADATA	ws;
extern HGLRC	hrc;
extern WNDCLASS a;
HWND hwnd;
long wsl;

#include <Fs98State.h>
#include <main.h>
#include <prototypes.h>
#include <fonts.h>
#include "FSUIPC_User.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <tgaload.h>

int page_ident(void);
int DrawCurrentPage(void);


#define CDU_MAIN 1
#include "cdu_prototypes.h"
#include "cdu_defines.h"
#include "cdu_structs.h"
#include "cdu_externs.h"
#include "cdu_pages.h"
#include "Flightplan.h"

int				mouseptr = 1;		// Mouse pointer visible?
int				wx, wy;				// Window sizes
char			*token;				// Scratch token variable (for strtok)
int				firsttime = 1;		// First time through the glut loop we set up the window
double			tilt = 0;			// Usefull in this?
char			*config = "cdu.ini";
int				monochrome;			// Unused.
int				windowframes = 1;	// window border toggle
int				fullscreen = 0;		// Toggle to view CDU screen only.
double			sunlight = 1.0;		// Not useful
int				window;				// Glut Window handle
int				width = 350;		
int				height = 350;
char			display_buffer[100];// General array for display strings
int				refresh = 0;		// Make sure that when we switch displays that we have the required data.
FS98StateData	all;				// This should go the way of the dodo.
double			version = 0.1;		//
double			WindowSize = 50.0;	// This is the size of the screen,  in other instruments it would be a square.
//int				sock;				
int				function = 0;		// Window to display
int				spidx = 0;			// Current index into scratchpad buffer
char			scratchpad[500];	// Holds the scratchpad data
int				valid_keys = 0;		// Holds valid LSK keys (OR'd together based on the following defines)
int				page = IDENT;		// Initial CDU page
double			totfuel = 0.0;		// Fuel totalizer
double			lat,lon;			//
GLuint			cduid;				// GL texture ID's
GLuint			litid;						
double			screen_top = 100.0 - 3.0;				// Actual CDU display coordinates
double			screen_bottom = 100.0 - 44.5;
double			screen_left = 10.0;
double			screen_right = 90.0;
double linesplit = (screen_top - screen_bottom) / 17.0;	// Clever and lazy way to calculate line positions.

int main(int, char **);

// Just what the name suggests, draws dashed lines in the CDU
void draw_dashed_line(double left, double right, int line)
{
	double y = 100.0 - ((double)line * linesplit);	// Here's that clever line calculation stuff
	glEnable(GL_LINE_STIPPLE);						// Tell OpenGL we're drawing dashed lines
	glLineStipple(1,0xff00);						// Here's the bitmask. '1111111100000000' '----    '
	glBegin(GL_LINES);								// Draw a line
	glVertex2d(left, y);
	glVertex2d(right, y);
	glEnd();
	glDisable(GL_LINE_STIPPLE);						// Back to regular lines before we leave
}

// Scratchpad processor if there's nothing in the scratchpad, take the *st parameter and put it there.
// Otherwise,  put the scratchpad data and put it in *st
int process_scratchpad(char *st)
{
	if (spidx != 0)
	{
		if (!strcmp("DELETE", scratchpad))	// If the scratchpad has "DELETE" we wipe out the data selected
		{
			strcpy(st, "");
			spidx = 0;
			scratchpad[0] = '\0';
			return(VDELETED);
		}
		else								// Put the scratchpad data in *st
		{
			strcpy(st, scratchpad);
			spidx = 0;						// Clear out the scratchpad
			scratchpad[0] = '\0';
			return(VADDED);
		}
	}
	else
	{
		if (strlen(st) != 0)				// Move *st into the scratchpad, and tell us how long it is
		{
			strcpy(scratchpad, st);
			spidx = strlen(st);
			return(VCOPIED);
		}
	}
	return(0);
}

// Display a string at (line) on (side) of the screen, in (font)
// (side) is LEFT, LEFTTITLE, RIGHT, RIGHTTITLE, or CENTER
void place_string(int side, int line, double font, char *st)
	{
	double y;
	double x;
	double size = 10.0;
	char string[100];

	if (font == 1)
		sprintf(string, "{f1}%s", st); // {f1/f2} is special font drawing code to switch fonts.
	else
		{
		sprintf(string, "{f2}%s", st);
		}

	if (line == 1) 
		line += 2;
	else 
		line += 3;
	if (side == LEFT)
		x = screen_left;
	else if (side == RIGHT)
		x = screen_right;
	else if (side == LEFTTITLE)		// Titles are a little indented on either side
		x = screen_left+2.0;
	else if (side == RIGHTTITLE)
		x = screen_right-2.0;

	y = (100.0 - ((double)line * linesplit))-0.5;

	if ((side == LEFT) || (side == LEFTTITLE))
		Puts(x, y, size-7, string, 0);					// Put with the anchor on the left
	else if ((side == RIGHT) || (side == RIGHTTITLE))
		RPuts(x,y, size-7, string);						// Put with the anchor on the right
	else if (side == CENTER)	
		CPuts(50, y, size-7, string);					// Put with the anchor in the center
	
	return;
	}

/* The function called whenever a key is pressed. */
void keyPressed(unsigned char key, int x, int y)
	{
	static repeats = 0;
	RECT lprect;

	hwnd = GetActiveWindow();
	function = 0;
	sprintf(display_buffer, "KEY->%d", key);
	if (spidx == VERROR)
	{	
		spidx = 0;
		memset(scratchpad, 0, 500);
	}

	if (key == '`')
	{
		fullscreen ^= 1;	// ^= is an exclusive OR,  essentially toggles between 1 and 0
		if (fullscreen)
		{
			screen_top = 100.0;								// Set the new screen size
			screen_bottom = 0.0;
			screen_left = 5.0;
			screen_right = 95.0;
			linesplit = (screen_top - screen_bottom) / 17.0;
			glutMouseFunc(NULL);							// No more mouse clicking!
		}
		else
		{
			screen_top = 100.0 - 3.0;						// Set the new screen size
			screen_bottom = 100.0 - 45.5;
			screen_left = 9.1;
			screen_right = 88.0;
			linesplit = (screen_top - screen_bottom) / 17.0;
			glutMouseFunc(mouse);							// Reinstate the mouse clicking
		}
	}
	else if (key == '~')									// Toggle the window frames
	{
		GetWindowRect(hwnd, &lprect);

		wsl = GetWindowLong(hwnd, GWL_STYLE);
		wsl ^= WS_CAPTION;
		wsl ^= WS_THICKFRAME;
		wsl ^= WS_BORDER;
		if (!SetWindowLong(hwnd, GWL_STYLE, wsl))
		{
			MessageBox(0, "Error setting window parameters", "ERROR", 0);
		}
		SetWindowPos(hwnd, HWND_TOP, (int)lprect.left, (int)lprect.top, (int)lprect.right - (int)lprect.left, 
			(int)lprect.bottom - (int)lprect.top, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
		glutShowWindow();
		windowframes ^= 1;
	}
	else if (key == 127)
	{
		repeats = 0;
		strcpy(scratchpad, "DELETE");
		spidx = 7;
	}
	else if (key == 8)
	{
		repeats++;
		if (repeats >= 3)	// Back space 3 times and it means erase the whole shebang
		{
			spidx = 0;
			scratchpad[0] = '\0';
		}
		else if (spidx > 0)
		{
			scratchpad[--spidx] = '\0';
		}
	}
	else if (isprint(key))			// Displayable key?
		{
		if (key == '.')				// *** This is wierd, shouldn't be here
		{
			if (fullscreen)
			{
				mouseptr^=1;
				ShowCursor(mouseptr);
			}
			else
				ShowCursor(true);
		}
		repeats = 0;
		key = toupper(key);
		scratchpad[spidx++] = (char)key;
		scratchpad[spidx] = '\0';
		}
    else if (key == 27)				// exit the program...normal termination.
        exit(0);
	}

/* The function called whenever a function key is pressed. */
void CtrlkeyPressed(int key, int x, int y)
	{
	function = 0;
	sprintf(display_buffer, "CTRL KEY->%d", key);
	if (spidx == VERROR)
	{	
		spidx = 0;
		memset(scratchpad, 0, 500);
	}

	switch(key)
	{
		case GLUT_KEY_LEFT:
			sunlight-=0.1;
			break;
		case GLUT_KEY_RIGHT:
			sunlight+=0.1;
			break;
		case GLUT_KEY_INSERT:
			page = ROUTE;
			return;
			break;
		case GLUT_KEY_HOME:
			page = PERF;
			return;
			break;
		case GLUT_KEY_END:
			page = MENU;
			return;
			break;
		case GLUT_KEY_PAGE_DOWN:
			SubPage++;
			break;
		case GLUT_KEY_PAGE_UP:
			SubPage--;
			if (SubPage < 0) 
				SubPage = 0;
			break;
		default:
			break;
	}
	function = key;

	return;
	}

// Clean exit, save our settings! (SOS?)
void getoutofdodge()	
{
	FILE *geometry;
	RECT lprect;
	FILE *bkp;

	hwnd = GetActiveWindow();
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

	wx = glutGet(GLUT_WINDOW_X);
	wy = glutGet(GLUT_WINDOW_Y);
	geometry = fopen(config, "w");
	if (geometry != NULL)
	{
		fprintf(geometry, "TYPE:%d:\n", fullscreen);
		fprintf(geometry, "WINDOW:%d:%d:%d:%d:%d:\n", wx, wy, width, height, windowframes);
		fprintf(geometry, "TILT:%f:\n", tilt);
		fclose(geometry);
	}

	Data_Close();				// Shut down FSUIPC
	wglDeleteContext(hrc);
        
	glutDestroyWindow(window);	// shut down our window 

	CloseDatabase();
	bkp = fopen("DATA.BKP", "wb");		// Save our current programming state in case we crashed
	if (bkp != NULL)
		{
		fwrite(&DataHolder, sizeof(DH), 1, bkp); 
		fclose(bkp);
		}

        /* exit the program...normal termination. */
	exit(0);
}

// Initialize OpenGL
void InitGL(int Width, int Height)	       
	{
  	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	
  	glClearDepth(1.0);			
  	glShadeModel(GL_FLAT);		
  	glMatrixMode(GL_PROJECTION);

	gluOrtho2D(0.0, 100.0, 0.0, 100.0);
	
  	glMatrixMode(GL_MODELVIEW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_TEXTURE_2D);

	// Load our textures and fonts.
	load_font("cduFont");
	cduid = tgaLoadAndBind ( "cdu.tga",  0);
	litid = tgaLoadAndBind ( "lightson.tga",  0);
	}

// Did the window move? if so, let's get the new sizes
void reshape(int w, int h)
	{
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	width = w; height = h;

	gluOrtho2D(0.0, 100.0, 0.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	}

// Main drawing loop
void DrawGLCDU()
	{
	static double distance = 0.0;
// This is for setting the window parameters, nothing more
	RECT lprect;
// =======================================================

// Clear the window
  	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// First time through we set up the window parameters
	if (firsttime)
	{
		hwnd = GetActiveWindow();
		if (!windowframes)
		{
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
	}

// Time of day
	double tod = ((double)all.byZuluHour*60.0) + (double)all.byZuluMin;

// If we're not in full screen mode,  display the keypad texture.
	if (!fullscreen)
	{
		glColor3f(sunlight,sunlight,sunlight);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, cduid);

		glBegin (GL_QUADS);
		glTexCoord2f (0.0, 0.0); glVertex3f (0.0, 0.0, 0.0);
		glTexCoord2f (1.0, 0.0); glVertex3f (100.0, 0.0, 0.0);
		glTexCoord2f (1.0, 1.0); glVertex3f (100.0, 100.0, 0.0);
		glTexCoord2f (0.0, 1.0); glVertex3f (0.0, 100.0, 0.0);
		glEnd ();

		if (all.byLightsOn)
		{
			glColor3f(1.0,1.0,1.0);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, litid);

			glBegin (GL_QUADS);
			glTexCoord2f (0.0, 0.0); glVertex3f (0.0, 0.0, 0.0);
			glTexCoord2f (1.0, 0.0); glVertex3f (100.0, 0.0, 0.0);
			glTexCoord2f (1.0, 1.0); glVertex3f (100.0, 100.0, 0.0);
			glTexCoord2f (0.0, 1.0); glVertex3f (0.0, 100.0, 0.0);
			glEnd ();
		}

		glDisable(GL_TEXTURE_2D);
	}
// Get all the FSUIPC data
//	get_data();

	DrawCurrentPage();

// Page flip the video buffers
  	glutSwapBuffers();
	Sleep(10);
#ifndef _MSC_VER
	frames++;
#endif
	}

// Hmm What's this?
int page_ident(void)
	{
	page = IDENT;
	return(1);
	}

// This is the EXEC light
void light_exec()
{
	glColor3f(1,1,0);
	glLineWidth(4.0);
	glBegin(GL_LINES);
	glVertex2d(77.6, 100.0-57.3);
	glVertex2d(83.9, 100.0-57.3);
	glEnd();
}

// Used when we want to see video coordinates (DEBUGGING)
void mouse2(int x, int y)
{
	double sx, sy;
	sx = ((double)x/(double)width) * 100.0;
	sy = ((double)y/(double)height) * 100.0;
	sprintf(display_buffer, "X: %.1f Y: %.1f", 
		sx, sy);
	glColor3f(1,1,1);
	place_string(CENTER, 1, SMALL, display_buffer);
}

// Button clicks
void mouse(int button, int state, int x, int y)
{
	DWORD result;
	int left = 0;
	double sx, sy;
	sx = ((double)x/(double)width) * 100.0;
	sy = ((double)y/(double)height) * 100.0;
	if (state != GLUT_DOWN)
		return;

// If there was an error, clear the scratchpad
	if (spidx == VERROR)
	{	
		spidx = 0;
		memset(scratchpad, 0, 500);
	}

	if ((sy > 47.0) && (sy < 52.0))     // row 1
	{
		if ((sx > 36.7) && (sx < 47.3))
			page = DEPARR;
		else if ((sx > 12.2) && (sx < 22.6))
			page = INIT;
		else if ((sx > 24.5) && (sx < 35.0))
			page = ROUTE;
		else if ((sx > 82.1) && (sx < 85.8))
			{
			dimmer-=0.1;
			if (dimmer < 0.0)
				dimmer = 0.0;
			all.byDimmer = (int)(dimmer * 255.0);
			Data_Write(0x6d0a, 1,					&all.byDimmer,						&result);
			Data_Process(&result);
			}
		else if ((sx > 85.8) && (sx < 89.2))
			{
			dimmer+=0.1;
			if (dimmer > 1.0)
				dimmer = 1.0;
			all.byDimmer = (int)(dimmer * 255.0);
			Data_Write(0x6d0a, 1,					&all.byDimmer,						&result);
			Data_Process(&result);
			}
	}
	else if ((sy > 53.3) && (sy < 58.4))		// row 2
	{
		if ((sx > 48.7) && (sx < 59.5))
			page = FMCCOMM;
	}
	else if ((sy > 59.7) && (sy < 64.6))		// row 3
	{
		if ((sx > 12.2) && (sx < 22.6))
			page = MENU;
		else if ((sx > 24.5) && (sx < 35.0))
			page = NAVRAD;
		else if ((sx > 41.5) && (sx < 48.9))
			scratchpad[spidx++] = 'A';
		else if ((sx > 51.2) && (sx < 58.5))
			scratchpad[spidx++] = 'B';
		else if ((sx > 61.2) && (sx < 68.7))
			scratchpad[spidx++] = 'C';
		else if ((sx > 71.0) && (sx < 78.1))
			scratchpad[spidx++] = 'D';
		else if ((sx > 80.2) && (sx < 87.7))
			scratchpad[spidx++] = 'E';
	}
	else if ((sy > 66.2) && (sy < 71.2))		// row 4
	{
		if ((sx > 12.2) && (sx < 22.6))
		{
			SubPage--;
			if (SubPage < 0) SubPage = 0;
		}
		else if ((sx > 24.5) && (sx < 35.0))
		{
			SubPage++;
		}
		else if ((sx > 41.5) && (sx < 48.9))
			scratchpad[spidx++] = 'F';
		else if ((sx > 51.2) && (sx < 58.5))
			scratchpad[spidx++] = 'G';
		else if ((sx > 61.2) && (sx < 68.7))
			scratchpad[spidx++] = 'H';
		else if ((sx > 71.0) && (sx < 78.1))
			scratchpad[spidx++] = 'I';
		else if ((sx > 80.2) && (sx < 87.7))
			scratchpad[spidx++] = 'J';
	}
	else if ((sy > 73.4) && (sy < 78.4))		// row 5
	{
		if ((sx > 41.5) && (sx < 48.9))
			scratchpad[spidx++] = 'K';
		else if ((sx > 51.2) && (sx < 58.5))
			scratchpad[spidx++] = 'L';
		else if ((sx > 61.2) && (sx < 68.7))
			scratchpad[spidx++] = 'M';
		else if ((sx > 71.0) && (sx < 78.1))
			scratchpad[spidx++] = 'N';
		else if ((sx > 80.2) && (sx < 87.7))
			scratchpad[spidx++] = 'O';
		else if ((sx > 11.9) && (sx < 19.4))
			scratchpad[spidx++] = '1';
		else if ((sx > 21.5) && (sx < 29.0))
			scratchpad[spidx++] = '2';
		else if ((sx > 31.7) && (sx < 39.2))
			scratchpad[spidx++] = '3';
	}
	else if ((sy > 79.2) && (sy < 84.2))		// row 6
	{
		if ((sx > 41.5) && (sx < 48.9))
			scratchpad[spidx++] = 'P';
		else if ((sx > 51.2) && (sx < 58.5))
			scratchpad[spidx++] = 'Q';
		else if ((sx > 61.2) && (sx < 68.7))
			scratchpad[spidx++] = 'R';
		else if ((sx > 71.0) && (sx < 78.1))
			scratchpad[spidx++] = 'S';
		else if ((sx > 80.2) && (sx < 87.7))
			scratchpad[spidx++] = 'T';
		else if ((sx > 11.9) && (sx < 19.4))
			scratchpad[spidx++] = '4';
		else if ((sx > 21.5) && (sx < 29.0))
			scratchpad[spidx++] = '5';
		else if ((sx > 31.7) && (sx < 39.2))
			scratchpad[spidx++] = '6';
	}
	else if ((sy > 85.4) && (sy < 90.3))		// row 7
	{
		if ((sx > 41.5) && (sx < 48.9))
			scratchpad[spidx++] = 'U';
		else if ((sx > 51.2) && (sx < 58.5))
			scratchpad[spidx++] = 'V';
		else if ((sx > 61.2) && (sx < 68.7))
			scratchpad[spidx++] = 'W';
		else if ((sx > 71.0) && (sx < 78.1))
			scratchpad[spidx++] = 'X';
		else if ((sx > 80.2) && (sx < 87.7))
			scratchpad[spidx++] = 'Y';
		else if ((sx > 11.9) && (sx < 19.4))
			scratchpad[spidx++] = '7';
		else if ((sx > 21.5) && (sx < 29.0))
			scratchpad[spidx++] = '8';
		else if ((sx > 31.7) && (sx < 39.2))
			scratchpad[spidx++] = '9';
	}
	else if ((sy > 91.5) && (sy < 96.4))		// row 8
	{
		if ((sx > 41.5) && (sx < 48.9))
			scratchpad[spidx++] = 'Z';
		else if ((sx > 51.2) && (sx < 58.5))
			scratchpad[spidx++] = ' ';
		else if ((sx > 61.2) && (sx < 68.7))
		{
			strcpy(scratchpad, "DELETE");
			spidx = 7;
		}
		else if ((sx > 71.0) && (sx < 78.1))
			scratchpad[spidx++] = '/';
		else if ((sx > 80.2) && (sx < 87.7))
		{
			scratchpad[spidx] = '\0';
			spidx--;
		}
		else if ((sx > 11.9) && (sx < 19.4))
			scratchpad[spidx++] = '.';
		else if ((sx > 21.5) && (sx < 29.0))
			scratchpad[spidx++] = '0';
		else if ((sx > 31.7) && (sx < 39.2))
			scratchpad[spidx++] = '-';

	}



	if ((sx > 0.9) && (sx < 6.9))  // Left LSKs
	{
		left = 1;
	}
	if ((sx > 91.7) && (sx < 97.7)) // Right LSKs
	{
		left = 0;
	}

	if ((sy > 12.5) && (sy < 15.8))
	{
		if (left)
			function = LSK1L;
		else
			function = LSK1R;
		return;
	}
	if ((sy > 17.2) && (sy < 20.7))
	{
		if (left)
			function = LSK2L;
		else
			function = LSK2R;
		return;
	}
	if ((sy > 21.9) && (sy < 25.4))
	{
		if (left)
			function = LSK3L;
		else
			function = LSK3R;
		return;
	}
	if ((sy > 27.0) && (sy < 30.3))
	{
		if (left)
			function = LSK4L;
		else
			function = LSK4R;
		return;
	}
	if ((sy > 31.8) && (sy < 35.3))
	{
		if (left)
			function = LSK5L;
		else
			function = LSK5R;
		return;
	}
	if ((sy > 36.7) && (sy < 40.4))
	{
		if (left)
			function = LSK6L;
		else
			function = LSK6R;
		return;
	}

}

// Program entry point!
int main(int argc, char **argv) 
	{  
	FILE *geometry;			// File handle for the INI file
	FILE *bkp;				// File handle for the state backup file
	char buffer[100];		// Junk buffer
	int ff = 0;

	OpenDatabase();
//	DBGetNavaids();

	geometry = fopen(config, "r");  // Read the INI file
	if (geometry != NULL)
		{
		while (fgets((char *)buffer, 100, geometry))
			{
			if (!strncmp((char *)buffer, "TYPE", 4))
			{
				token = strtok((char *)buffer, ":");
				token = strtok(NULL, ":");
				fullscreen = atoi(token);
			}

			if (!strncmp((char *)buffer, "WINDOW", 6))
				{
				token = strtok((char *)buffer, ":");
				token = strtok(NULL, ":");
				wx = atoi(token);
				token = strtok(NULL, ":");
				wy = atoi(token);
				token = strtok(NULL, ":");
				width = atoi(token);
				token = strtok(NULL, ":");
				height = atoi(token);
				token = strtok(NULL, ":");
				windowframes = atoi(token);
				}
			if (!strncmp((char *)buffer, "TILT", 4))
				{
				token = strtok((char *)buffer, ":");
				token = strtok(NULL, ":");
				tilt = atof(token);
				}
			}
		fclose(geometry);
		}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(width,height);
	glutInitWindowPosition(wx, wy);
	sprintf(display_buffer, "Ellie Systems CDU %.1f (beta)", version);
	window = glutCreateWindow(display_buffer);
	glutDisplayFunc(DrawGLCDU);
	glutReshapeFunc(reshape);
	glutIdleFunc(DrawGLCDU);
	glutKeyboardFunc(keyPressed);
	glutSpecialFunc(CtrlkeyPressed);
	if (fullscreen)
	{
		screen_top = 100.0;
		screen_bottom = 0.0;
		screen_left = 5.0;
		screen_right = 95.0;
		linesplit = (screen_top - screen_bottom) / 17.0;
		glutMouseFunc(NULL);	
	}
	else
		glutMouseFunc(mouse);	
// ---------------------------------------------
// Use this for displaying mouse coordinates
//	glutPassiveMotionFunc(mouse2);
// ---------------------------------------------
	glEnable(GL_TEXTURE_2D);
  	InitGL(width, height);
// ******************************************************************************************************
// *** For testing, we turn off FSUIPC connection
// ***---------------------------------------------------------------------------------------------------
//	make_socket(PORT, "1.1.1.1");			// Crazy old holdover from Linux days
// ******************************************************************************************************
	LoadNavData();							// Load all the navaids from the database
	atexit(getoutofdodge);					// If we want to leave, wanted or now, save our settings

	bkp = fopen("DATA.BKP", "rb");			// Read the last settings
	if (bkp != NULL)
		{
		fread(&DataHolder, sizeof(DH), 1, bkp); 
		fclose(bkp);
		}

	glutMainLoop();							// Fire up the loop
	return(1);
	}

// Get all FSUIPC data and perform some calculations
char *get_data(void)
{
	int intPart;
	DWORD result;
	static int airfile_read = 0;
	int v1,v2,vr;

	v1 = atoi(DataHolder.V1);
	vr = atoi(DataHolder.VR);
	v2 = atoi(DataHolder.V2);
#ifdef _MSC_VER

// FUEL TANKS
		Data_Read(0x0B74, sizeof(DWORD),		&all.dwFuelCenterPct,			&result);
		Data_Read(0x0B78, sizeof(DWORD),		&all.dwFuelCenterCap,			&result); 
		Data_Read(0x0B7C, sizeof(DWORD),		&all.dwFuelLeftPct,				&result); 
		Data_Read(0x0B80, sizeof(DWORD),		&all.dwFuelLeftCap,				&result); 
		Data_Read(0x0B84, sizeof(DWORD),		&all.dwFuelLeftAuxPct,			&result); 
		Data_Read(0x0B88, sizeof(DWORD),		&all.dwFuelLeftAuxCap,			&result); 
    	Data_Read(0x0B94, sizeof(DWORD),		&all.dwFuelRightPct,			&result);
		Data_Read(0x0B98, sizeof(DWORD),		&all.dwFuelRightCap,			&result);
		Data_Read(0x0B9C, sizeof(DWORD),		&all.dwFuelRightAuxPct,			&result);
		Data_Read(0x0BA0, sizeof(DWORD),		&all.dwFuelRightAuxCap,			&result);
// TEMPERATURE
		Data_Read(0x11D0, sizeof(WORD),			&all.wTAT,						&result);
		Data_Read(0x0E8C, sizeof(WORD),			&all.wOutsideAirTemp,			&result);
// FLAPS AND GEAR
		Data_Read(0x0BDC, sizeof(DWORD),		&all.dwFlapsCommanded,			&result);
		Data_Read(0x0BE0, sizeof(DWORD),		&all.dwFlapsLeft,				&result);
		Data_Read(0x0BE4, sizeof(DWORD),		&all.dwFlapsRight,				&result);
		Data_Read(0x0BEC, sizeof(DWORD),		&all.dwNoseGearPosition,		&result);
		Data_Read(0x0BF0, sizeof(DWORD),		&all.dwLeftGearPosition,		&result);
		Data_Read(0x0BF4, sizeof(DWORD),		&all.dwRightGearPosition,		&result);
// ENGINE 1
		Data_Read(0x0892, sizeof(WORD),			&all.wEng1StarterSwitch,		&result);
		Data_Read(0x0894, sizeof(WORD),			&all.wEng1Combustion,			&result);
		Data_Read(0x088C, sizeof(WORD),			&all.wEng1Throttle,				&result);
		Data_Read(0x0898, sizeof(WORD),			&all.wEng1N1,					&result);
		Data_Read(0x0896, sizeof(WORD),			&all.wEng1N2,					&result);
		Data_Read(0x08BC, sizeof(WORD),			&all.wEng1EPR,					&result);
		Data_Read(0x0918, sizeof(FLOAT64),		&all.fEng1FF_PPH,				&result);
		Data_Read(0x08D4, sizeof(DWORD),		&all.dwEng1Vibration,			&result);
		Data_Read(0x08B8, sizeof(WORD),			&all.wEng1OilTemp,				&result);
		Data_Read(0x08BA, sizeof(WORD),			&all.wEng1OilPressure,			&result);
		Data_Read(0x08D0, sizeof(DWORD),		&all.dwEng1OilQty,				&result);
		Data_Read(0x08BE, sizeof(WORD),			&all.wEng1EGT,					&result);
		Data_Read(0x08D8, sizeof(DWORD),		&all.dwEng1HydraulicPressure,	&result);
		Data_Read(0x08DC, sizeof(DWORD),		&all.dwEng1HydraulicQty,		&result);
// ENGINE 2
		Data_Read(0x092A, sizeof(WORD),			&all.wEng2StarterSwitch,		&result);
		Data_Read(0x092C, sizeof(WORD),			&all.wEng2Combustion,			&result);
		Data_Read(0x0924, sizeof(WORD),			&all.wEng2Throttle,				&result);
		Data_Read(0x0930, sizeof(WORD),			&all.wEng2N1,					&result);
		Data_Read(0x092E, sizeof(WORD),			&all.wEng2N2,					&result);
		Data_Read(0x0954, sizeof(WORD),			&all.wEng2EPR,					&result);
		Data_Read(0x09B0, sizeof(FLOAT64),		&all.fEng2FF_PPH,				&result);
		Data_Read(0x096C, sizeof(DWORD),		&all.dwEng2Vibration,			&result);
		Data_Read(0x0950, sizeof(WORD),			&all.wEng2OilTemp,				&result);
		Data_Read(0x0952, sizeof(WORD),			&all.wEng2OilPressure,			&result);
		Data_Read(0x0968, sizeof(DWORD),		&all.dwEng2OilQty,				&result);
		Data_Read(0x0956, sizeof(WORD),			&all.wEng2EGT,					&result);
		Data_Read(0x0970, sizeof(DWORD),		&all.dwEng2HydraulicPressure,	&result);
		Data_Read(0x0974, sizeof(DWORD),		&all.dwEng2HydraulicQty,		&result);
// ENGINE 3
		Data_Read(0x09C2, sizeof(WORD),			&all.wEng3StarterSwitch,		&result);
		Data_Read(0x09C4, sizeof(WORD),			&all.wEng3Combustion,			&result);
		Data_Read(0x09BC, sizeof(WORD),			&all.wEng3Throttle,				&result);
		Data_Read(0x09C8, sizeof(WORD),			&all.wEng3N1,					&result);
		Data_Read(0x09C6, sizeof(WORD),			&all.wEng3N2,					&result);
		Data_Read(0x09EC, sizeof(WORD),			&all.wEng3EPR,					&result);
		Data_Read(0x0A48, sizeof(FLOAT64),		&all.fEng3FF_PPH,				&result);
		Data_Read(0x0A04, sizeof(DWORD),		&all.dwEng3Vibration,			&result);
		Data_Read(0x09E8, sizeof(WORD),			&all.wEng3OilTemp,				&result);
		Data_Read(0x09EA, sizeof(WORD),			&all.wEng3OilPressure,			&result);
		Data_Read(0x0A00, sizeof(DWORD),		&all.dwEng3OilQty,				&result);
		Data_Read(0x09EE, sizeof(WORD),			&all.wEng3EGT,					&result);
		Data_Read(0x0A08, sizeof(DWORD),		&all.dwEng3HydraulicPressure,	&result);
		Data_Read(0x0A0C, sizeof(DWORD),		&all.dwEng3HydraulicQty,		&result);
// ENGINE 4
		Data_Read(0x0A5A, sizeof(WORD),			&all.wEng4StarterSwitch,		&result);
		Data_Read(0x0A5C, sizeof(WORD),			&all.wEng4Combustion,			&result);
		Data_Read(0x0A54, sizeof(WORD),			&all.wEng4Throttle,				&result);
		Data_Read(0x0A60, sizeof(WORD),			&all.wEng4N1,					&result);
		Data_Read(0x0A5E, sizeof(WORD),			&all.wEng4N2,					&result);
		Data_Read(0x0A84, sizeof(WORD),			&all.wEng4EPR,					&result);
		Data_Read(0x0AE0, sizeof(FLOAT64),		&all.fEng4FF_PPH,				&result);
		Data_Read(0x0A9C, sizeof(DWORD),		&all.dwEng4Vibration,			&result);
		Data_Read(0x0A80, sizeof(WORD),			&all.wEng4OilTemp,				&result);
		Data_Read(0x0A82, sizeof(WORD),			&all.wEng4OilPressure,			&result);
		Data_Read(0x0A98, sizeof(DWORD),		&all.dwEng4OilQty,				&result);
		Data_Read(0x0A86, sizeof(WORD),			&all.wEng4EGT,					&result);
		Data_Read(0x0AA0, sizeof(DWORD),		&all.dwEng4HydraulicPressure,	&result);
		Data_Read(0x0AA4, sizeof(DWORD),		&all.dwEng4HydraulicQty,		&result);
		Data_Read(0x07BC, sizeof(DWORD),		&all.dwAPMaster,				&result);
		Data_Read(0x0810, sizeof(DWORD),		&all.dwAPAutoThrottle,			&result);
		Data_Read(0x3328, sizeof(int),			&all.elevator_axis,				&result);
		Data_Read(0x332a, sizeof(int),			&all.aileron_axis,				&result);
		Data_Read(0x332c, sizeof(int),			&all.rudder_axis,				&result);
		Data_Read(0x207c, sizeof(FLOAT64),		&all.TURB_ENGINE_1_PCT_REVERSER,&result);
		Data_Read(0x217c, sizeof(FLOAT64),		&all.TURB_ENGINE_2_PCT_REVERSER,&result);
		Data_Read(0x07C0, sizeof(DWORD),		&all.dwAPWingLeveler,			&result);
		Data_Read(0x07C4, sizeof(DWORD),		&all.dwAPNav1Hold,				&result);
		Data_Read(0x07C8, sizeof(DWORD),		&all.dwAPHeadingHold,			&result);
		Data_Read(0x07CC, sizeof(DWORD),		&all.dwAPHeading,				&result);
		Data_Read(0x07D0, sizeof(DWORD),		&all.dwAPAltitudeHold,			&result);
		Data_Read(0x07D4, sizeof(DWORD),		&all.dwAPAltitude,  			&result);
		Data_Read(0x07D8, sizeof(DWORD),		&all.dwAPAttitudeHold,			&result);
		Data_Read(0x07DC, sizeof(DWORD),		&all.dwAPAirSpeedHold,			&result);
		Data_Read(0x07E2, sizeof(WORD),			&all.wAPAirSpeed,				&result);
		Data_Read(0x07E4, sizeof(DWORD),		&all.dwAPMachNumberHold,		&result);
		Data_Read(0x07E8, sizeof(WORD),			&all.dwAPMachNumber,			&result);
		Data_Read(0x07EC, sizeof(DWORD),		&all.dwAPVerticalSpeedHold,		&result);
		Data_Read(0x07F2, sizeof(SWORD),		&all.wAPVerticalSpeed,			&result);
		Data_Read(0x07F4, sizeof(DWORD),		&all.dwAPRPMHold,				&result);
		Data_Read(0x07FA, sizeof(WORD),			&all.wAPRPM,					&result);
		Data_Read(0x07FC, sizeof(DWORD),		&all.dwAPGlideSlopeHold,		&result);
		Data_Read(0x0800, sizeof(DWORD),		&all.dwAPLocalizerHold,			&result);
		Data_Read(0x0804, sizeof(DWORD),		&all.dwAPBackCourseHold,		&result);
		Data_Read(0x0808, sizeof(DWORD),		&all.dwAPYawDamper,				&result);
		Data_Read(0x080C, sizeof(DWORD),		&all.dwAPToGa,					&result);
		Data_Read(0x0810, sizeof(DWORD),		&all.dwAPAutoThrottle,			&result);
// CAUTIONS
		Data_Read(0x036D, sizeof(BYTE),			&all.bOverSpeed,				&result);
// BRAKES
		Data_Read(0x0BC8, sizeof(WORD),			&all.wParkingBrake,				&result);
// SPOILERS / AILERONS / ELEVATORS / RUDDER
		Data_Read(0x0BD0, sizeof(WORD),			&all.wSpoilerCommanded,			&result);
		Data_Read(0x2ea0, sizeof(double),		&all.ELEVATOR_TRIM,				&result);
		Data_Read(0x2eb0, sizeof(double),		&all.AILERON_TRIM,				&result);
		Data_Read(0x2ec0, sizeof(double),		&all.RUDDER_TRIM,				&result);
// MISC
		Data_Read(0x0366, sizeof(BYTE),			&all.bPlaneOnGround,			&result);
		Data_Read(0x0281, sizeof(BYTE),			&all.byStrobeOn,				&result);
		Data_Read(0x0280, sizeof(BYTE),			&all.byLightsOn,				&result);
		Data_Read(0x0AEC, sizeof(WORD),			&all.wEngines,					&result);
		Data_Read(0x0AF4, sizeof(WORD),			&all.wFuelWeight,				&result);
		Data_Read(0x0564, sizeof(DWORD),		&all.dwLatHi,					&result);
		Data_Read(0x0560, sizeof(DWORD),		&all.dwLatLo,					&result);
		Data_Read(0x056c, sizeof(DWORD),		&all.dwLonHi,					&result);
		Data_Read(0x0568, sizeof(DWORD),		&all.dwLonLo,					&result);
		Data_Read(0x023b, 1,					&all.byZuluHour,				&result);
		Data_Read(0x023c, 1,					&all.byZuluMin,					&result);
		Data_Read(0x023e, 2,					&all.wDayOfYear,				&result);
		Data_Read(0x3d00, 256,					&all.szAircraftName,			&result);
		Data_Read(0x3c00, 256,					&all.szAirfileName,				&result);
		Data_Read(0x3e00, 256,					&all.szFSPath,					&result);

		Data_Read(0x0350, 2,					&all.wBcdNav1Freq,				&result);
		Data_Read(0x0352, 2,					&all.wBcdNav2Freq,				&result);
		Data_Read(0x3000, 6,					&all.vor1_id,					&result);
		Data_Read(0x301f, 6,					&all.vor2_id,					&result);
		Data_Read(0x303e, 6,					&all.adf_id,					&result);
		Data_Read(0x0c4e, 2,					&all.wNav1Obs,					&result);
		Data_Read(0x0c50, 2,					&all.wNav1Rad,					&result);
		Data_Read(0x0c5e, 2,					&all.wNav2Obs,					&result);
		Data_Read(0x0c60, 2,					&all.wNav2Rad,					&result);

		Data_Read(0x6d00, 1,					&all.byNDEFIS,					&result);
		Data_Read(0x6d01, 1,					&all.byNDLeft,					&result);
		Data_Read(0x6d02, 1,					&all.byNDRight,					&result);
		Data_Read(0x6d03, 1,					&all.byNDRange,					&result);
		Data_Read(0x6d0a, 1,					&all.byDimmer,					&result);

		Data_Write(0x6d04, 2,					&v1,							&result);
		Data_Write(0x6d06, 2,					&vr,							&result);
		Data_Write(0x6d08, 2,					&v2,							&result);

		if (Data_Process(&result) == false)
		{
			Data_Close();
		}

// Set up the dimmer
		dimmer = (double)all.byDimmer / 255.0;
		dimmer = 1.0;

// First wave of conversions the simple ones. Save that
	sprintf(DataHolder.time, "%02d%02dz", all.byZuluHour,all.byZuluMin);
	sprintf(DataHolder.vor_l, "{f1}%.2f {f2}A {f1}%s", bcdToFreq(all.wBcdNav1Freq), all.vor1_id);
	sprintf(DataHolder.vor_r, "{f1}%.2f {f2}A {f1}%s", bcdToFreq(all.wBcdNav2Freq), all.vor2_id);
	sprintf(DataHolder.crs_l, "%d", all.wNav1Obs);
	sprintf(DataHolder.crs_r, "%d", all.wNav2Obs);
	sprintf(DataHolder.rad_l, "%.0f", (double)all.wNav1Rad*360.0/65536.0);
	sprintf(DataHolder.rad_r, "%.0f", (double)all.wNav2Rad*360.0/65536.0);

// Fuel totalizer
	totfuel = ((((double) all.dwFuelCenterPct / 83886.0 ) / 100.0 )		* (double)all.dwFuelCenterCap );
	totfuel += ((((double)all.dwFuelLeftPct / 83886.0 ) / 100.0 )		* (double)all.dwFuelLeftCap );
	totfuel += ((((double)all.dwFuelLeftAuxPct / 83886.0 ) / 100.0 )	* (double)all.dwFuelLeftAuxCap );
	totfuel += ((((double)all.dwFuelRightPct / 83886.0 ) / 100.0 )		* (double)all.dwFuelRightCap );
	totfuel += ((((double)all.dwFuelRightAuxPct / 83886.0 ) / 100.0 )	* (double)all.dwFuelRightAuxCap );

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

// Convert to DMS (Degree Minute Seconds)
	DecToDMS(lat, DataHolder.lastpos, LATITUDE);
	DecToDMS(lon, display_buffer, LONGITUDE);
// Everything at the same position until we can fake differences
	strcat(DataHolder.lastpos, display_buffer);
	strcpy(DataHolder.gpspos, DataHolder.lastpos);
	strcpy(DataHolder.irspos, DataHolder.lastpos);
#endif
	return((char *)&all);
}

// Frequencies in FSUIPC are expressed in HEX. Convert to human numbers. Whats the matter with Peter anyhow!
double bcdToFreq(int bcd)
	{
	char bcds[5];
	sprintf(bcds, "%4.4x", bcd);	// Convert the HEX to a string
	sprintf(display_buffer, "1%2.2s.%s", bcds, &bcds[strlen(bcds)-2]); // Add the hundred and the decimal
	return(atof(display_buffer));   // Convert that to a float, and return
	}

// Convert decimal degrees to DMS (Degree Minute Seconds) for display
int DecToDMS(double decimal, char *DMS, int which)
{
	double whole;
	char temp[10];
	char orientation;
	sprintf(temp, "%.0f", decimal);		// Let's get the whole number from that
	whole = atof(temp);
	if (whole < 0.0)
	{
		if (which == LATITUDE)
			orientation = 'S';
		else
			orientation = 'W';
	}
	else
		if (which == LATITUDE)
			orientation = 'N';
		else
			orientation = 'E';

	if (which == LONGITUDE)
		{
		if (fabs((decimal-whole) * 60.0) < 10.0)
			// EG: W122º02.4
			sprintf(DMS, "%c%03.0f{s2}0%.1f", orientation, fabs(decimal), fabs((decimal-whole) * 60.0)); 
		else
			sprintf(DMS, "%c%03.0f{s2}%.1f", orientation, fabs(decimal), fabs((decimal-whole) * 60.0)); 
		}
	else
		{
		if (fabs((decimal-whole) * 60.0) < 10.0)
			sprintf(DMS, "%c%02.0f{s2}0%.1f", orientation, fabs(decimal), fabs((decimal-whole) * 60.0)); 
		else
			sprintf(DMS, "%c%02.0f{s2}%.1f", orientation, fabs(decimal), fabs((decimal-whole) * 60.0)); 
		}
	return(0);
}

// Holy moly there's gotta be a better way than this crap
void JulianToDayMonth(int julian, int *day, int *month)
{
	julian-=1;
	if		(julian <= 31)
	{
		*day = julian; *month = 1;
	}
	else if (julian <= 59)
	{
		*day = julian-31; *month = 2;
	}
	else if (julian <= 90)
	{
		*day = julian-59; *month = 3;
	}
	else if (julian <= 120)
	{
		*day = julian-90; *month = 4;
	}
	else if (julian <= 151)
	{
		*day = julian-120; *month = 5;
	}
	else if (julian <= 181)
	{
		*day = julian-151; *month = 6;
	}
	else if (julian <= 212)
	{
		*day = julian-181; *month = 7;
	}
	else if (julian <= 243)
	{
		*day = julian-212; *month = 8;
	}
	else if (julian <= 273)
	{
		*day = julian-243; *month = 9;
	}
	else if (julian <= 304)
	{
		*day = julian-273; *month = 10;
	}
	else if (julian <= 334)
	{
		*day = julian-304; *month = 11;
	}
	else 
	{
		*day = julian-334; *month = 12;
	}
	return;
}

// Change this to a linked list when time permits no reason to set aside that much memory
OBJECTS				objects[200000];
int					navaids = 0;
long				total_objs = 0;
// *******************************
// Read in the navaid database
// *******************************
void LoadNavData()
{
	int				in;
	int				init;
	unsigned int 	a = 0;
	unsigned int 	n = 0;
	unsigned int 	f = 0;
	unsigned int 	v = 0;
	long			indx;		

	in = open("navaids.dat", O_RDONLY|O_BINARY);
	if (in == -1)
	{
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

// Find an ID in the database
long find_object(char *name)
	{
	int init;

	for (init = 0; init < total_objs; init++)
		{
		if (!strcmp(objects[init].id, name)) 
			return(init);
		}
	return(-1);
	}

// Junk junk junk
int initialize_pages(void)
{
	return(1);
}

// Loop through the page array looking for the one we're on
// This draws the entire page, and handles all the callbacks, workhorse
int DrawCurrentPage()
	{
	int init = 0;
	// Look for the current page
	CurrentCP = 0;
	while (CDUPage[init].Page != 9999)
		{
		int line = 2;
		if (CDUPage[init].Page == page)
			{
			if (CDUPage[init].Dashes)
				{
				double y = (100.0 - ((double)CDUPage[init].Dashes * linesplit));

				glLineStipple(6, 0x6666);
				glEnable(GL_LINE_STIPPLE);
				glBegin(GL_LINES);
				glVertex2d(screen_left, y);
				glVertex2d(screen_right, y);
				glEnd();
				}

			CurrentCP = init;
			// Pre-validate the page if needed
			if (CDUPage[init].PagePreValidate != NULL)
				CDUPage[init].PagePreValidate();
			// Display the title
			setColor(CDUPage[init].tr,CDUPage[init].tg,CDUPage[init].tb);
			place_string(CENTER, 1, LARGE, CDUPage[init].PageTitle);
			// Process the LSK entries
			for (int i = 0; i < 6; i++)
				{
				// Do the left side
				if (CDUPage[init].LSKEntry[i].lskPreValidate != NULL)
					CDUPage[init].LSKEntry[i].lskPreValidate();
				setColor(CDUPage[init].LSKColor[i].tr,CDUPage[init].LSKColor[i].tg,CDUPage[init].LSKColor[i].tb);
				if (CDUPage[init].LSKEntry[i].lskTitle != NULL) 
					place_string(LEFTTITLE, (i*2)+2, SMALL, CDUPage[init].LSKEntry[i].lskTitle);
				setColor(CDUPage[init].LSKColor[i].vr,CDUPage[init].LSKColor[i].vg,CDUPage[init].LSKColor[i].vb);
				if (CDUPage[init].LSKEntry[i].lskValue != NULL) 
					place_string(LEFT, (i*2)+3, LARGE, CDUPage[init].LSKEntry[i].lskValue);
				// Do the right side
				if (CDUPage[init].LSKEntry[i+6].lskPreValidate != NULL)
					CDUPage[init].LSKEntry[i+6].lskPreValidate();
				setColor(CDUPage[init].LSKColor[i+6].tr,CDUPage[init].LSKColor[i+6].tg,CDUPage[init].LSKColor[i+6].tb);
				if (CDUPage[init].LSKEntry[i+6].lskTitle != NULL) 
					place_string(RIGHTTITLE, (i*2)+2, SMALL, CDUPage[init].LSKEntry[i+6].lskTitle);
				setColor(CDUPage[init].LSKColor[i+6].vr,CDUPage[init].LSKColor[i+6].vg,CDUPage[init].LSKColor[i+6].vb);
				if (CDUPage[init].LSKEntry[i+6].lskValue != NULL) 
					place_string(RIGHT, (i*2)+3, LARGE, CDUPage[init].LSKEntry[i+6].lskValue);
				}
			// Display the scratch pad contents.
			setColor(1,1,1);
			if (spidx > 0)
				place_string(LEFT, 14, LARGE, scratchpad);

			switch(function)
				{
				case LSK1L:
					if (CDUPage[init].LSKEntry[0].lskSelectFunc != NULL)
						CDUPage[init].LSKEntry[0].lskSelectFunc();
					break;
				case LSK2L:
					if (CDUPage[init].LSKEntry[1].lskSelectFunc != NULL)
						CDUPage[init].LSKEntry[1].lskSelectFunc();
					break;
				case LSK3L:
					if (CDUPage[init].LSKEntry[2].lskSelectFunc != NULL)
						CDUPage[init].LSKEntry[2].lskSelectFunc();
					break;
				case LSK4L:
					if (CDUPage[init].LSKEntry[3].lskSelectFunc != NULL)
						CDUPage[init].LSKEntry[3].lskSelectFunc();
					break;
				case LSK5L:
					if (CDUPage[init].LSKEntry[4].lskSelectFunc != NULL)
						CDUPage[init].LSKEntry[4].lskSelectFunc();
					break;
				case LSK6L:
					if (CDUPage[init].LSKEntry[5].lskSelectFunc != NULL)
						CDUPage[init].LSKEntry[5].lskSelectFunc();
					break;
				case LSK1R:
					if (CDUPage[init].LSKEntry[6].lskSelectFunc != NULL)
						CDUPage[init].LSKEntry[6].lskSelectFunc();
					break;
				case LSK2R:
					if (CDUPage[init].LSKEntry[7].lskSelectFunc != NULL)
						CDUPage[init].LSKEntry[7].lskSelectFunc();
					break;
				case LSK3R:
					if (CDUPage[init].LSKEntry[8].lskSelectFunc != NULL)
						CDUPage[init].LSKEntry[8].lskSelectFunc();
					break;
				case LSK4R:
					if (CDUPage[init].LSKEntry[9].lskSelectFunc != NULL)
						CDUPage[init].LSKEntry[9].lskSelectFunc();
					break;
				case LSK5R:
					if (CDUPage[init].LSKEntry[10].lskSelectFunc != NULL)
						CDUPage[init].LSKEntry[10].lskSelectFunc();
					break;
				case LSK6R:
					if (CDUPage[init].LSKEntry[11].lskSelectFunc != NULL)
						CDUPage[init].LSKEntry[11].lskSelectFunc();
					break;
				}
			function = 0;
			break;
			}
		init++;
		}
	return(1);
	}

// Sets the next page when PROMPT LSK has been selected
int SetPage(void)
	{
	page = CDUPage[CurrentCP].LSKEntry[function-1].NewPage;
	SubPage = 0; // Reset the subpage if we're switching pages. (Subpage is used for multipage displays)
	return(1);
	}
