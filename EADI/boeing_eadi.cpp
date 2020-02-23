/*
** BOEING_EADI.CPP boeing specific code
** Copyright (c) 1999-2003 Don Lafontaine; ALL RIGHTS RESERVED
*/
#include "eadi.h"
#include <externs.h>
#include "fonts.h"
#include <defines.h>
void reshape(int w, int h);

#define SAMPLES 20
typedef struct st // Speed Trend sample data
{
	double speed; // speed difference between samples.
	long delta;   // time in milliseconds between samples
} ST;


extern double WindowSize;
int						flap[10];
int						flapsmax[10];
int						flapsmin[10];
int						flapsret[10];
int						maxspd;
int						minspd;
int						gearmax;
int						topmost = 0;
int						v1,v2,vr;
int						mouseptr = 1;
int						showframe=0;

// The function called whenever a key is pressed. 
void keyPressed(unsigned char key, int x, int y)
	{
	RECT				lprect;

	hwnd = GetActiveWindow();    // Make sure this is set properly.
    /* avoid thrashing this procedure */
	x = y = 0;
	switch(key)
		{
		case '.':									// Hide the mouse pointer
			mouseptr^=1;
			ShowCursor(mouseptr);
			break;
		case '8':									// Reset the dimmer 
			dimmer = 1.0;
			if (bal)
				{
				glDeleteLists(pll,1);
				pll = 0;
				glDeleteLists(bal,1);
				bal = 0;
				}
			break;		
		case '9':									// Dim the lights.
			dimmer -= 0.1;
			if (bal)
				{
				glDeleteLists(pll,1);
				pll = 0;
				glDeleteLists(bal,1);
				bal = 0;
				}
			break;
		case '0':									// Brighten
			dimmer += 0.1;
			if (bal)
				{
				glDeleteLists(pll,1);
				pll = 0;
				glDeleteLists(bal,1);
				bal = 0;
				}
			break;
		case 'm':
		case 'M':
			showframe^=1;
			if (showframe == 1)
				padding = 5.0;
			else
				padding = 0.0;
			reshape(height, width);
			break;
		case '1':
			cb -= 0.01;
			if (cb < 0.0)
				cb = 0.0;
		   	glClearColor (cr, cg, cb, 0.0);			
			break;
		case '2':
			cb += 0.01;
			if (cb > 1.0)
				cb = 1.0;
		   	glClearColor (cr, cg, cb, 0.0);			
			break;
		case '[':									// Increase decision height value
			decision_height-=10;
			if (decision_height < 0) decision_height = 0;
			break;
		case ']':									// Decrease decision height
			decision_height+=10;
			if (decision_height > 9000) decision_height = 9000;
			break;
		case 'q':									// DEBUG ONLY
		case 'Q':
			display_stats^=1;
			break;
		case 'i':
		case 'I':									// Imperial / US
			imperial^=1;
			break;
	    case 't':									// Rotate display 90 degrees
		case 'T':
			tilt += 90.0;
			if (tilt > 270.0)
				tilt = 0.0;
			break;
		case 'w':									// Toggle flight director mode
		case 'W':
			simplefd ^= 1;
			break;
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
		case 'f':	
		case 'F':		// Toggle Window Border
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
		default:	
			break;
		}

    /* If escape is pressed, kill everything. */
    	if (key == ESCAPE)
    		{
			exit(0);
    		}
	}

	// Draw 7 segment led digits
int draw_digit(float x, float y, char digit, float fw, float fh)
{
#define LLEFT 1
#define ULEFT 2
#define MID 4
#define TOP 8
#define BOT 16
#define LRIGHT 32
#define URIGHT 64
#define PER 128

int					segments = 0;
	switch(digit)
	{
	case '0':
		segments = LLEFT|LRIGHT|ULEFT|URIGHT|TOP|BOT;
		break;
	case '1':
		segments = LRIGHT|URIGHT;
		break;
	case '2':
		segments = TOP|MID|BOT|URIGHT|LLEFT;
		break;
	case '3':
		segments = TOP|BOT|MID|URIGHT|LRIGHT;
		break;
	case '4':
		segments = ULEFT|MID|URIGHT|LRIGHT;
		break;
	case '5':
		segments = TOP|BOT|MID|ULEFT|LRIGHT;
		break;
	case '6':
		segments = TOP|MID|BOT|ULEFT|LLEFT|LRIGHT;
		break;
	case '7':
		segments = TOP|URIGHT|LRIGHT;
		break;
	case '8':
		segments = TOP|BOT|MID|ULEFT|URIGHT|LLEFT|LRIGHT;
		break;
	case '9':
		segments = TOP|MID|BOT|ULEFT|URIGHT|LRIGHT;
		break;
	case '.':
		segments = PER;
		break;
	default:
		return(0);
		break;
	}
	glLineWidth(1.0);
	if (segments & PER)
	{
		glBegin(GL_POINTS);
		glVertex2f(x+(fw + 0.5), y);
	}
	glBegin(GL_LINES);
	if (segments & BOT)
	{
		glVertex2f(x+0.1, y); //bottom
		glVertex2f(x+(fw-0.1), y);
	}
	if (segments & TOP)
	{
		glVertex2f(x+0.1, y+fh); //top
		glVertex2f(x+(fw - 0.1), y+fh);
	}
	if (segments & MID)
	{
		glVertex2f(x+0.1, y+(fh/2.0)); //middle
		glVertex2f(x+(fw - 0.1), y+(fh/2.0));
	}
	if (segments & LLEFT)
	{
		glVertex2f(x, y+0.1); //lower left
		glVertex2f(x, y+((fh / 2.0) - 0.1));
	}
	if (segments & ULEFT)
	{
		glVertex2f(x, y+((fh / 2.0) + 0.1)); // upper left
		glVertex2f(x, y+(fh - 0.1));
	}
	if (segments & LRIGHT)
	{
		glVertex2f(x+fw, y+0.1); //lower right
		glVertex2f(x+fw, y+((fh / 2.0) - 0.1));
	}
	if (segments & URIGHT)
	{
		glVertex2f(x+fw, y+((fh / 2.0) + 0.1)); // upper right
		glVertex2f(x+fw, y+(fh - 0.1));
	}
	glEnd();
	return(1);
}

draw_digit_string(float x, float y, char * string)
{
	unsigned int		init;
	float				fw = 1.0;
	float				fh = 2.0;

	for (init = 0; init < strlen(string); init++)
	{
		if (string[init] == '.')
			x -= (fw + (fw/2.0));
		draw_digit(x, y, string[init], fw, fh);
		x+= (fw + (fw / 2.0));
	}
	return(0);
}

void DrawGLHorizon(void)
	{
	static GLuint		dl = 0;
	static int			loops = 0;
	static int			time, oldtime;
	GLUquadricObj		*quadObj1;
	static double		rot = 0;
	double				hd;
	double				ws;
	RECT				lprect;


	if (firsttime)
	{
		if (!windowframes)
		{
			hwnd = GetActiveWindow();    // Make sure this is set properly.
			GetWindowRect(hwnd, &lprect);

			wsl = GetWindowLong(hwnd, GWL_STYLE);
			wsl ^= WS_CAPTION;
			wsl ^= WS_THICKFRAME;
			wsl ^= WS_BORDER;
			if (!SetWindowLong(hwnd, GWL_STYLE, wsl))
			{
				MessageBox(0, "Error changing window settings", "ERROR", 0);
			}
			SetWindowPos(hwnd, HWND_TOP, (int)lprect.left, (int)lprect.top, (int)lprect.right - (int)lprect.left, 
				(int)lprect.bottom - (int)lprect.top, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
			glutShowWindow();
		}
		firsttime = 0;
	}

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

	if (tilt != 0)
	{
		glPushMatrix();
		glRotatef(tilt, 0.0, 0.0, 1.0);
	}
	/* Calculate frame rates */
    oldtime = timeGetTime() - time;
	loops++;
	if (oldtime >= 1000)
		{
		time = timeGetTime();
		frames = loops;
		loops = 0;
		}

	hd = hdg;
	ws = (double)all.wWindSpeed;


// MOVE THIS CONVERSION -------------------------------------------------------------
	if (!all.bPlaneOnGround)
		{
		wca = atan2( ws*sin(d2r(hd)-d2r(wd)), tas - ws * cos(d2r(hd)-d2r(wd)) );
		wca = r2d(wca);
		}
	else
		wca = 0.0;
// ----------------------------------------------------------------------------------


	hdg+=magvar;

    wbank = bank; 
		
	if (wbank < 0.0)
		wbank = fabs(wbank);
	if (wbank > 180.0)
		wbank = (360.0 - wbank)*-1.0;
    wpitch = pitch;
    if (wpitch > 180)
        wpitch = (360 - wpitch) *-1.0f;

	wpitch*=-1.0;
	bankok = 1;

	draw_horizon();
	quadObj1 = gluNewQuadric();
	gluQuadricDrawStyle(quadObj1, GLU_FILL);
	setColor(cr, cg, cb);
	if (!dl)
		{
		dl = 300;
		glNewList(dl, GL_COMPILE);
		glPushMatrix();
		glTranslatef(-26.0, 23.9, 0.0);
		    gluPartialDisk(quadObj1, 3.0f, 5.0f, 50.0f, 1.0f, 270.0f,90.0);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(17.8, 23.9, 0.0);
		    gluPartialDisk(quadObj1, 3.0f, 5.0f, 50.0f, 1.0f, 0.0f,90.0);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(17.8,-23.9, 0.0);
		    gluPartialDisk(quadObj1, 3.0f, 5.0f, 50.0f, 1.0f, 90.0f,90.0);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(-26.0, -23.9, 0.0);
		    gluPartialDisk(quadObj1, 3.0f, 5.0f, 50.0f, 1.0f, 180.0f,90.0);
		glPopMatrix();
		glEndList();
		}
	glCallList(dl);
	gluDeleteQuadric(quadObj1);
	
	if (simplefd)
		draw_plane();

	if (bankok)
		draw_flightdirector();

	draw_speedbar();
	draw_vsi();
	draw_altitudebar();

	if (bankok)
		draw_autopilot();

	draw_compass();

	if (bankok)
		{
		if (all.vor1_id[0] == 'I')
			{
			setColor(1,1,1);
			sprintf(display_buffer, "%4.4s/%03.0f%c", all.vor1_id, ilsobs+magvar,DEGREE);
//	        Puts(stright+2.5, 35, 3.0, display_buffer, 0);
//			Puts(stright+16.5, 36.5, 1.5, "O", 0);
	        Puts(-27.0, 35, 3.0, display_buffer, 0);
			if (atof(all.sDME1) == -1.0)
				strcpy(display_buffer, "---");
			else
				sprintf(display_buffer, "DME %-4.4s", all.sDME1);
			Puts(-27.0, 32, 3.0, display_buffer, 0);
			}
		else
			{
			ilsobs = -1.0;
			}
		}

	if (smask)
		{
		setColor(1.0,1.0,1.0);
		glRectf(tposx, tposy, tposx-20.0, tposy-20.0);
		setColor(1.0,0.0,0.0);
		glRectf(tposx, tposy, tposx-1.0, tposy-1.0);
		}

	if (display_stats)
		{
		setColor(1.0,0,0);
		glRectf(37, -45, 47, -49);
		setColor(1,1,1);
		drawbox(37, -45, 47, -49, 0, 0);
		sprintf(display_buffer, "FPS: %03.0f", (double)frames);
		CPuts(42, -48, 3, display_buffer);
		}

	if (showframe)
	{
		glColor3f(1,1,1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, frameid);

		glBegin (GL_QUADS);
		glTexCoord2f (0.0, 0.0); glVertex3f ((WindowSize+padding)*-1.0, 
			(WindowSize+padding)*-1.0, 0.0);
		glTexCoord2f (1.0, 0.0); glVertex3f ((WindowSize+padding), 
			(WindowSize+padding)*-1.0, 0.0);
		glTexCoord2f (1.0, 1.0); glVertex3f ((WindowSize+padding), 
			(WindowSize+padding), 0.0);
		glTexCoord2f (0.0, 1.0); glVertex3f ((WindowSize+padding)*-1.0, 
			(WindowSize+padding), 0.0);
		glEnd ();
		glDisable(GL_TEXTURE_2D);
	}

	if (tilt != 0)
	{
		glPopMatrix();
	}
	glutSwapBuffers();
    }

void pitchladder()
	{
	double				init;
	double stall = (fabs(pitch) / ((maxaoa/82.0)*100.0))*100.0;

	maxaoa = (double)((100.0*(double)all.wAOA/32767.0));
	stall = (-1.0*pitch)+(((maxaoa/82.0)*100.0)-21.5);
//	if (!pll)
//		{
//		pll = 311;
//		glNewList(pll,GL_COMPILE);
		glLineWidth(2.0);
		glVertexPointer(2, GL_DOUBLE, 0, pitchladderv);
		glDrawArrays(GL_LINES, 0, 96);
		glLineWidth(1.0);

//		sprintf(display_buffer, "MAX AOA: %.1f - RAW: %.1f", (-1.0*pitch)+(((maxaoa/82.0)*100.0)-21.5), (double)all.wAOA/32767.0);
//		CPuts(0,0,3,display_buffer);

		if (all.dwFlapsCommanded != 0)
		{
			setColor(1,1,0);
			glBegin(GL_LINES);
				glVertex2f(-8.0, stall);
				glVertex2f(-12.0, stall);
				glVertex2f(-8.0, stall);
				glVertex2f(-8.0, stall - 1.0);
	
				glVertex2f(-9.0, stall);
				glVertex2f(-10.0, stall + 2.0);
				glVertex2f(-10.0, stall);
				glVertex2f(-11.0, stall + 2.0);
				glVertex2f(-11.0, stall);
				glVertex2f(-12.0, stall + 2.0);
	
				glVertex2f(8.0, stall);
				glVertex2f(12.0, stall);
				glVertex2f(8.0, stall);
				glVertex2f(8.0, stall - 1.0);

				glVertex2f(9.0, stall);
				glVertex2f(10.0, stall + 2.0);
				glVertex2f(10.0, stall);
				glVertex2f(11.0, stall + 2.0);
				glVertex2f(11.0, stall);
				glVertex2f(12.0, stall + 2.0);
	
			glEnd();
		}

		setColor(1,1,1);
		for (init = 10.0; init < 60.0; init+=10.0)
			{
			Puts(-12.0, init-1, 3.0, itoa(init, display_buffer, 10), 0);
			Puts(9.5, init-1, 3.0, itoa(init, display_buffer, 10), 0);
			Puts(-12.0, (init+1)*-1.0, 3.0, itoa(init, display_buffer, 10), 0);
			Puts(9.5, (init+1)*-1.0, 3.0, itoa(init, display_buffer, 10), 0);
			}
//		glEndList();
//		}
//	glCallList(pll);
	}

void draw_horizon(void)
	{
	double					slip;
	double					x1,y1,x2,y2;
	static double			planeheight = 0.0;
    GLUquadricObj			*quadObj1;


    quadObj1 = gluNewQuadric();
    gluQuadricDrawStyle(quadObj1, GLU_FILL);

	setColor(sr,sg,sb);
	glRectf(30.0 ,35.0, -30.0, -35.0);

	/* Draw the artificial Horizon */
glPushMatrix();
	glTranslatef(-4.0,0.0, 0.0);
glPushMatrix();
	glRotatef(bank, 0.0, 0.0, 1.0);

	pitch*=10.0;
	if (pitch > 40)
		pitch = 40;
	else if (pitch < -40)
		pitch = -40;

glPushMatrix();
	glTranslatef(0,pitch, 0.0);

#ifdef FILL
	setColor(gr,gg,gb);	
	glRectf(40.0 ,0.0, -40.0, -60.0);
#endif
	setColorx(1.0, 1.0, 1.0, 0.7);
	glLineWidth(1.0);
	pitchladder();
	setColor(1,1,1);
// Horizon LINE
	glLineWidth(2.0);
	glBegin(GL_LINES);
		glVertex2i(-50, 0);
		glVertex2i(50, 0);
	glEnd();

glPopMatrix();

// This is the side slip indicator
		if (all.bSideSlip > 127)
			slip = (double)all.bSideSlip-255.0;
		else
			slip = (double)all.bSideSlip;
		slip/=10.0;
		bbankv2[0]=slip+1.5;
		bbankv2[2]=slip-1.5;
		bbankv2[4]=slip-1.5;
		bbankv2[6]=slip+1.5;
		bbankv2[8]=slip+1.5;
		glVertexPointer(2, GL_DOUBLE, 0, bbankv1);
		glDrawArrays(GL_LINE_STRIP, 0, 4);
		glVertexPointer(2, GL_DOUBLE, 0, bbankv2);
		glDrawArrays(GL_LINE_STRIP, 0, 5);
glPopMatrix();
		setColor(sr, sg, sb);
		gluPartialDisk(quadObj1, 25.0, 40.0f, 9.0f, 1.0f, 315.0f, 90.0);
		setColor(1,1,1);
		glBegin(GL_TRIANGLES);
			glVertex2d(0.0, 25.0);
			glVertex2d(1.0, 27.0);
			glVertex2d(-1.0, 27.0);
		glEnd();
	setColor(1.0,1.0,1.0);
#define PI 3.1415926535897932384626433832795
	if (!bal)
		{
		bal = 312;
		glNewList(bal, GL_COMPILE);
			bankstart = 25.0;
			bankends = 26.0;
			bankendl = 28.0;
		glBegin(GL_LINES);

		x1 = cos(-1.0*(225.0 * PI) / 180.0) * bankstart;
		y1 = sin(-1.0*(225.0 * PI) / 180.0) * bankstart;
		x2 = cos(-1.0*(225.0 * PI) / 180.0) * bankends;
		y2 = sin(-1.0*(225.0 * PI) / 180.0) * bankends;
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		x1 = cos(-1.0*(240.0 * PI) / 180.0) * bankstart;
		y1 = sin(-1.0*(240.0 * PI) / 180.0) * bankstart;
		x2 = cos(-1.0*(240.0 * PI) / 180.0) * bankendl;
		y2 = sin(-1.0*(240.0 * PI) / 180.0) * bankendl;
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		x1 = cos(-1.0*(250.0 * PI) / 180.0) * bankstart;
		y1 = sin(-1.0*(250.0 * PI) / 180.0) * bankstart;
		x2 = cos(-1.0*(250.0 * PI) / 180.0) * bankends;
		y2 = sin(-1.0*(250.0 * PI) / 180.0) * bankends;
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		x1 = cos(-1.0*(260.0 * PI) / 180.0) * bankstart;
		y1 = sin(-1.0*(260.0 * PI) / 180.0) * bankstart;
		x2 = cos(-1.0*(260.0 * PI) / 180.0) * bankends;
		y2 = sin(-1.0*(260.0 * PI) / 180.0) * bankends;
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		x1 = cos(-1.0*(280.0 * PI) / 180.0) * bankstart;
		y1 = sin(-1.0*(280.0 * PI) / 180.0) * bankstart;
		x2 = cos(-1.0*(280.0 * PI) / 180.0) * bankends;
		y2 = sin(-1.0*(280.0 * PI) / 180.0) * bankends;
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		x1 = cos(-1.0*(290.0 * PI) / 180.0) * bankstart;
		y1 = sin(-1.0*(290.0 * PI) / 180.0) * bankstart;
		x2 = cos(-1.0*(290.0 * PI) / 180.0) * bankends;
		y2 = sin(-1.0*(290.0 * PI) / 180.0) * bankends;
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		x1 = cos(-1.0*(300.0 * PI) / 180.0) * bankstart;
		y1 = sin(-1.0*(300.0 * PI) / 180.0) * bankstart;
		x2 = cos(-1.0*(300.0 * PI) / 180.0) * bankendl;
		y2 = sin(-1.0*(300.0 * PI) / 180.0) * bankendl;
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		x1 = cos(-1.0*(315.0 * PI) / 180.0) * bankstart;
		y1 = sin(-1.0*(315.0 * PI) / 180.0) * bankstart;
		x2 = cos(-1.0*(315.0 * PI) / 180.0) * bankends;
		y2 = sin(-1.0*(315.0 * PI) / 180.0) * bankends;
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		glEnd();
		glEndList();
		}
	glCallList(bal);
	glLineWidth(1.0);
glPopMatrix();
	if (mask)
		{
		setColor(0.0,0.0,cb);
			glRectf(-29.0, 50.0+padding, -50.0-padding, -50.0-padding);
			glRectf(20.9, 50.0+padding, 50.0+padding, -50.0-padding);
			glRectf(-30.0, -27.0, 22.9, -50.0-padding);
			glRectf(-30.0, 27.0, 22.9, 50.0+padding);
		}
	if (all.wInnerMarker || all.wMiddleMarker || all.wOuterMarker)
		{
			glPushMatrix();
			glLineWidth(2.0);
			glTranslatef(16.0, 23.0, 0.0);
			setColor(1.0,1.0,1.0);
        	gluQuadricDrawStyle(quadObj1, GLU_LINE);
        	gluDisk(quadObj1, 3.0f, 3.0f, 90.0f, 1.0f);
			glPopMatrix();
			if (all.wInnerMarker)
				CPuts(16.0, 22.5, 2.5, "IM");
			else if (all.wMiddleMarker)
				CPuts(16.0, 22.5, 2.5, "MM");
			else if (all.wOuterMarker)
				CPuts(16.0, 22.5, 2.5, "OM");
		}
	gluDeleteQuadric(quadObj1);
	return;
	}

void draw_plane(void)
	{
	double					inside;
	double					bx,by;

	glLineWidth(2.0);

/* Plane symbol */
		bx = -4.0;
		by = 0.0;
		setColor(0, 0, 0);
	// Fuselage
		drawbox(bx+-0.7, -0.7, bx+0.7, 0.7, 0, 1);
	// Left Wing
			inside = 8.5;
		drawbox(bx+(inside*-1.0), 0.7, bx+((inside*-1.0)-10.0), -0.7, 0, 1);
		drawbox(bx+(inside*-1.0), 0.7, bx+((inside*-1.0)-1.0), -3.7, 0, 1);
	//Right Wing
		drawbox(bx+inside, 0.7, bx+(inside+10.0), -0.7, 0, 1);
		drawbox(bx+inside, 0.7, bx+(inside+1.0), -3.7, 0, 1);
// Draw the outline
			setColor(1.0, 1.0, 1.0);
		drawbox(bx+-0.7, -0.7, bx+0.7, 0.7, 0, 0);
		glBegin(GL_LINE_STRIP);
			glVertex2d(bx+(inside*-1.0), 0.7);
			glVertex2d(bx+((inside*-1.0)-10.0), 0.7);
			glVertex2d(bx+((inside*-1.0)-10.0), -0.7);
			glVertex2d(bx+((inside*-1.0)-1.0), -0.7);
			glVertex2d(bx+((inside*-1.0)-1.0), -3.7);
			glVertex2d(bx+(inside*-1.0), -3.7);
			glVertex2d(bx+(inside*-1.0), 0.7);
		glEnd();
		glBegin(GL_LINE_STRIP);
			glVertex2d(bx+inside, 0.7);
			glVertex2d(bx+(inside+10.0), 0.7);
			glVertex2d(bx+(inside+10.0), -0.7);
			glVertex2d(bx+(inside+1.0), -0.7);
			glVertex2d(bx+(inside+1.0), -3.7);
			glVertex2d(bx+inside, -3.7);
			glVertex2d(bx+inside, 0.7);
		glEnd();
		
	glLineWidth(1.0);
	}

void draw_speedbar(void)
	{
	double					ts; // tape speed (Also Y position
	long					lastsc = -777;
	long					apspd;
	double					smoothias;
	long					speedcheck = 0;
	double					sos;
	double					spd;
	double					target;
	double					tap = stright-2.0;
	int						strend = 0;
	register double			init;
	double					top, bottom;
	double					barber;

	static double			ospd;				// Store previous speed
	static double			lastaccell = 0.0;	// this will hold the actual calculated speed trend
	static long				time, IntervalTime; // used to calculate deltas
	static ST				speedtrends[SAMPLES];    // this hold the speeds for the moving average speed trend
	static char				full = 0;			// don't display the speed trend until we have a full set.
	static int				sti = 0;			// Speed trend index
	long					TotalTime = 0;		// total delta time for all samples.
	double					TotalSpeed = 0;		// total speeds
	int						loop;
	static double			la[SAMPLES];

	int						avg;
	double					mach;
	

// Draw the Speed bar 
	setColor(br, bg, bb);
#ifdef FILL
	glRectf(stleft, sttop, stright, stbottom);
#endif
	apspd = (long)all.wAPAirSpeed;
	spd = ias;

	if (ospd == 0.0)
	{
		ospd = ias;
		time = timeGetTime();
	}
// Calculate speed trend.  (Predicted speed in 10 seconds) *********
	if (ias - ospd != 0.0)
	{
		speedtrends[sti].speed = ias - ospd;
		ospd = ias;

		speedtrends[sti].delta = timeGetTime() - time;
		time = timeGetTime();
	
		sti++;
	
		if (full)
		{
			TotalSpeed = 0.0;
			TotalTime = 0;
			for (loop = 0; loop < SAMPLES; loop++)
			{
				TotalSpeed+=speedtrends[loop].speed;
				TotalTime+=speedtrends[loop].delta;
				lastaccell += la[loop];
			}
			la[sti] = (TotalSpeed/(double)SAMPLES) * (10000.0/(double)(TotalTime/SAMPLES));
			lastaccell /= SAMPLES;
		}
		if (sti > SAMPLES-1) 
		{
			full = 1; // we can now display trends
			sti = 0;
		}
//		setColor(1,1,1);
//		sprintf(display_buffer, "TIME: %ld SPEED: %f", TotalTime, TotalSpeed);
//		Puts(-30.0, 20.0, 2, display_buffer, 0);
//		sprintf(display_buffer, "LAST: %f", lastaccell);
//		Puts(-30.0, 15.0, 2, display_buffer, 0);
	}
	target = ias + lastaccell;
// ******************************************************************
//	tap = stright-1.0;

	setColor(1,1,1);
	glLineWidth(1.0);

	if (target < 0.0)
		target = 0.0;

// Flap limits not visible on the ground.
	if (all.bPlaneOnGround)
		{
		barber = (double)all.dwBarberPole/128.0;
		}
	else
		{
		if (get_flappos() == 0)
			barber = (double)all.dwBarberPole/128.0;
		else
			barber = flapsmax[get_flappos()];
		}

// Hack to allow the airbus speed tape to start at 40knots and only to start
//		moving at 30knots
		barber -= 30.0;
		spd -= 30.0;
		if (spd < 0.0)
			spd = 0.0;
		apspd -= 30.0;
		target -= 30.0;
// Reduce Float precision.
	barber = ((int)((barber)*10.0+0.5))/10.0;
	target = ((int)((target)*10.0+0.5))/10.0;

	lines = 0;
	setColor(1,1,1);
	top = (sttop+5.0)*2.0;
	bottom = stbottom*2.0;
	for (init = top; init > bottom; init-=0.1)
		{
			ts = init / 2.0;
// Reduce float precision.
		smoothias = ((int)((spd+init)*10.0+0.5))/10.0;
// Nothing gets drawn below zero knots.
		if (smoothias < 0.0)
			break;

// This sets the Barber Pole speed limit on the speed scale.
		if (bankok)
			{
			if (smoothias == (double)v1-30.0)
			{
				Puts(stright+1, ts, 2.0, "V1", 0);
			}
			if (smoothias == (double)v2-30.0)
			{
				Puts(stright+1, ts, 2.0, "V2", 0);
			}
			if (smoothias == (double)vr-30.0)
			{
				Puts(stright+1, ts, 2.0, "VR", 0);
			}

			if (smoothias == barber)
				{
				setColor(0.7,0.0,0.0);
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1, 0x0F0F);
				glLineWidth(8.0);
				glBegin(GL_LINES);
					glVertex2d(stright+0.8, ts);
					glVertex2d(stright+0.8, sttop);
				glEnd();
				glDisable(GL_LINE_STIPPLE);
				glLineWidth(1.0);
				glBegin(GL_LINES);
					glVertex2d(stright+1.5, ts);
					glVertex2d(stright+1.5, sttop);
				glEnd();
				}
			
// This sets the Speed Trend arrow indication on the speed scale.
			if (smoothias == target)
				{
				strend = 1;
				glLineWidth(2.0);
					setColor(0, 1, 0);
				glBegin(GL_LINES);
				if (lastaccell != 0.0)
					{
						if (ts > 0.0)
							{
							glVertex2d(tap, ts);
							glVertex2d(tap-0.7, ts-0.7);
							glVertex2d(tap, ts);
							glVertex2d(tap+0.7, ts-0.7);
							}
						else
							{	
							glVertex2d(tap, ts);
							glVertex2d(tap-0.7, ts+0.7);
							glVertex2d(tap, ts);
							glVertex2d(tap+0.7, ts+0.7);
							}
					}
				glVertex2d(tap, ts);
				glVertex2d(tap, 0.0);
				glEnd();
				glLineWidth(1.0);
				}
// This sets the Autopilot Speed hold reading on the speed scale.
			if (smoothias == floor((double)apspd))
				{
				glLineWidth(2.0);
					setColor(1.0, 0.6, 1.0); // MAGENTA
					glBegin(GL_LINE_STRIP);
						glVertex2d(stright-2.0, ts);
						glVertex2d(stright, ts+1.5);
						glVertex2d(stright+3.0, ts+1.5);
						glVertex2d(stright+3.0, ts-1.5);
						glVertex2d(stright, ts-1.5);
						glVertex2d(stright-2.0, ts);
					glEnd();
				glLineWidth(1.0);
				}
			}
		setColor(1.0,1.0,1.0);
		speedcheck = (long)(spd*10.0)+(long)(init*10.0);
		if (!(speedcheck%100) && (speedcheck != lastsc))
			{
			lines++;
				ltoa((long)speedcheck/10+30, display_buffer, 10);
			if (strlen(display_buffer) > 1)
				{
				switch(display_buffer[strlen(display_buffer)-2])
					{
					case '0':
					case '2':
					case '4':
					case '6':
					case '8':
							RPuts(stright-3.0, ts-1.5, 4, display_buffer);
						break;
					}
				}
			glBegin(GL_LINES);
				glVertex2d(stright-2.0, ts);
				glVertex2d(stright, ts);
			glEnd();
			lastsc = speedcheck;
			}
		}

// This sets the infinite Speed Trend when the trend is off the speed scale.
	if (bankok)
		{
		if (!strend)
			{
			strend = 1;
			setColor(1, 1, 0);
			if (lastaccell != 0.0)
				{
				glLineWidth(2.0);
				glBegin(GL_LINES);
					if (lastaccell < 0.0)
						glVertex2d(tap, -35.0);
					else 
						glVertex2d(tap, 35.0);
					glVertex2d(tap, 0.0);
				glEnd();
				glLineWidth(1.0);
				}
			}
		}

// This is the graphics mask
	if (mask)
		{
		setColor(0.0,0.0,cb);
		glRectf(stright+10.0, sttop, stleft, sttop+(50.0-sttop));
		glRectf(stright+10.0, stbottom, stleft, stbottom-(50.0-fabs(stbottom)));
		}



	// set NO V SPD flag
	if (v1 == 0 && v2 == 0 && vr == 0)
		{
		setColor(1,1,0);
		CPuts(stright+2, 33.0, 2.5, "NO");
		CPuts(stright+2, 30.0, 2.5, "V");
		CPuts(stright+2, 26.0, 2.5, "S");
		CPuts(stright+2, 23.0, 2.5, "P");
		CPuts(stright+2, 20.0, 2.5, "D");
		}

// This is the box for the current speed value.  Not for the Airbus
		setColor(cr, cg, cb);

		glRectf(stleft, -4.0, stright-3.5, 4.0);
//		glBegin(GL_POLYGON);
//			glVertex2d(stleft, -4.0);
//			glVertex2d(stright-3.0, -4.0);
//			glVertex2d(stright-3.0, 4.0);
//			glVertex2d(stleft, 4.0);
//			glVertex2d(stleft, -4.0);
//		glEnd();
		glBegin(GL_POLYGON);
			glVertex2d(stright-3.5, -1.5);
			glVertex2d(stright-2.0, 0.0);
			glVertex2d(stright-3.5, 1.5);
		glEnd();
		setColor(1.0,1.0,1.0);
		glBegin(GL_LINE_STRIP);
			glVertex2d(stleft, -4.0);
			glVertex2d(stright-3.5, -4.0);
			glVertex2d(stright-3.5, -1.5);
			glVertex2d(stright-2.0, 0.0);
			glVertex2d(stright-3.5, 1.5);
			glVertex2d(stright-3.5, 4.0);
			glVertex2d(stleft, 4.0);
			glVertex2d(stleft, -4.0);
		glEnd();
		sprintf(display_buffer, "%03.0f", ias);
		Puts(stleft+1.0, -2.0, 5.0, display_buffer, 0);
		setColor(1.0, 0.6, 1.0);
		if (all.dwAPMachNumberHold)
			sprintf(display_buffer, "%.0f", ((double)all.dwAPMachNumber/65535.0)*1000.0);
		else
			itoa(all.wAPAirSpeed, display_buffer, 10);
		Puts(stleft, sttop+1.0, 4.0, display_buffer, 0);
		setColor(1.0, 1.0, 1.0);
// MOVE THIS CONVERSION
		sos = 38.967854 * sqrt(((double)all.wOutsideAirTemp / 256.0) + 273.15);
		mach = tas / sos;
		if (mach >= 0.4)
			if (mach > 1.0)
				sprintf(display_buffer, "%.03f", mach);
			else
				sprintf(display_buffer, ".%03d", (int)(mach*1000.0));
		else
			sprintf(display_buffer, "GS %ld", gs);
		Puts(stleft, stbottom-8, 5.0, display_buffer, 0);
	}

void draw_vsi(void)
	{
	double					s,e;
	SINT16					apvs;
	double					np;
	double					tvsi = vs;
	double					ttvsi = tvsi;
	double					tapvsi;
	double					ttapvsi;
	double					MaxDeflection = 26.0;
	int init;
	double ticks[6];

	if (tvsi > 6000.0)
		tvsi = 6000.0;
	if (tvsi < -6000.0)
		tvsi = -6000.0;

	ticks[5] = 23.0;
	ticks[4] = 19.7;
	ticks[3] = 16.5;
	ticks[2] = 13.25;
	ticks[1] = 10.0;
	ticks[0] = 5.0;

	if (tvsi < 0.0)
		tvsi = fabs(tvsi);

	if (tvsi < 500.0)
		np = tvsi/500.0 * ticks[0];
	else if (tvsi < 1000.0)
		np = ((tvsi-500.0)/500.0 * (ticks[1] - ticks[0])) + ticks[0];
	else if (tvsi < 1500.0)
		np = ((tvsi-1000.0)/500.0 * (ticks[2] - ticks[1])) + ticks[1];
	else if (tvsi < 2000.0)
		np = ((tvsi-1500.0)/500.0 * (ticks[3] - ticks[2])) + ticks[2];
	else if (tvsi < 4000.0)
		np = ((tvsi-2000.0)/2000.0 * (ticks[4] - ticks[3])) + ticks[3];
	else if (tvsi <= 6000.0)
		np = ((tvsi-4000.0)/2000.0 * (ticks[5] - ticks[4])) + ticks[4];

	if (ttvsi < 0.0)
	{
		tvsi = ttvsi;
		np *= -1.0;
	}

/* Draw the VSI */

	setColor(br, bg, bb);
		glBegin(GL_POLYGON);
			glVertex2d(atright+2.0, 25);
			glVertex2d(atright+2.0,6);
			glVertex2d(atright+3.5,4.5);
			glVertex2d(atright+10.0, 4.5);
			glVertex2d(atright+10.0, 12);
			glVertex2d(atright+5.0, 25);
			glVertex2d(atright+2.0, 25);
		glEnd();
		glRectf(atright+3.5, 4.5, atright+10.0, -4.5);
		glBegin(GL_POLYGON);
			glVertex2d(atright+2.0, -25);
			glVertex2d(atright+2.0,-6);
			glVertex2d(atright+3.5,-4.5);
			glVertex2d(atright+10.0, -4.5);
			glVertex2d(atright+10.0, -12);
			glVertex2d(atright+5.0, -25);
			glVertex2d(atright+2.0, -25);
		glEnd();
		setColor(1.0,1.0,1.0);
/*
		Puts(atright+1.0, (6000/300)-1, 3, "6",0);
		Puts(atright+1.0, (4000/300)-1, 3, "4",0);
		Puts(atright+1.0, (1000/300)-1, 3, "1",0);
		Puts(atright+1.0, (-6000/300)-1, 3, "6",0);
		Puts(atright+1.0, (-4000/300)-1, 3, "4",0);
		Puts(atright+1.0, (-1000/300)-1, 3, "1",0);
*/

		Puts(atright+2.0, (ticks[1])-1, 2.5, "1",0);
		Puts(atright+2.0, (ticks[3])-1, 2.5, "2",0);
		Puts(atright+2.0, (ticks[5])-1, 2.5, "6",0);
		Puts(atright+2.0, (ticks[1]*-1)-1, 2.5, "1",0);
		Puts(atright+2.0, (ticks[3]*-1)-1, 2.5, "2",0);
		Puts(atright+2.0, (ticks[5]*-1)-1, 2.5, "6",0);

		s = atright+4.0; 
		e = atright+4.5;


	apvs = all.wAPVerticalSpeed;
	ttapvsi = tapvsi = apvs;
	
	sprintf(display_buffer, "%.0f", vs);
	if ((int)vs - (((int)vs / 100)*100) < 50)
		{
		if (abs((int)vs)/100 == 0)
			strcpy(display_buffer, "");
		else
			sprintf(display_buffer, "%.0f00", fabs(vs)/100);
		}
	else
		{
		sprintf(display_buffer, "%.0f50", fabs(vs)/100);
		}
	if ((vs < -400.0) || (vs > 400.0))
		{
		if (vs < 5.0)
			Puts(atright+1.0, -28, 3, display_buffer, 0);
		else
			Puts(atright+1.0, 26, 3, display_buffer, 0);
		}
	glLineWidth(2.0);
	setColor(1.0,1.0,1.0);
	glBegin(GL_LINES);
	glVertex2d(atright+15.0, 0);
	if (apvs > 32768)
		apvs = 65536 - apvs * -1;
	if (apvs > 6000)
		apvs = 6000;
	else if (apvs < -6000)
		apvs = -6000;
	if (vs > 6000.0)
		vs = 6000.0;
	else if (vs < -6000.0)
		vs = -6000.0;
	glVertex2f(e, np); //vs/300.0);
	glEnd();
	setColor(1,1,1);
	glBegin(GL_LINES);
		for (init = 0; init < 6; init++)
		{
		glVertex2d(s, ticks[init]);
		glVertex2d(e, ticks[init]);
		glVertex2d(s, ticks[init]*-1);
		glVertex2d(e, ticks[init]*-1);
		}
		/*
		glVertex2d(s, 6000/300);
		glVertex2d(e, 6000/300);
		glVertex2d(s, 4000/300);
		glVertex2d(e, 4000/300);
		glVertex2d(s, 2000/300);
		glVertex2d(e, 2000/300);
		glVertex2d(s, 1000/300);
		glVertex2d(e, 1000/300);
		glVertex2d(s, -1000/300);
		glVertex2d(e, -1000/300);
		glVertex2d(s, -2000/300);
		glVertex2d(e, -2000/300);
		glVertex2d(s, -4000/300);
		glVertex2d(e, -4000/300);
		glVertex2d(s, -6000/300);
		glVertex2d(e, -6000/300);
		*/
	glEnd();
	glLineWidth(1.0);
	if (tapvsi < 0.0)
		tapvsi = fabs(tapvsi);

	if (tapvsi < 500.0)
		np = tapvsi/500.0 * ticks[0];
	else if (tapvsi < 1000.0)
		np = ((tapvsi-500.0)/500.0 * (ticks[1] - ticks[0])) + ticks[0];
	else if (tapvsi < 1500.0)
		np = ((tapvsi-1000.0)/500.0 * (ticks[2] - ticks[1])) + ticks[1];
	else if (tapvsi < 2000.0)
		np = ((tapvsi-1500.0)/500.0 * (ticks[3] - ticks[2])) + ticks[2];
	else if (tapvsi < 4000.0)
		np = ((tapvsi-2000.0)/2000.0 * (ticks[4] - ticks[3])) + ticks[3];
	else if (tapvsi <= 6000.0)
		np = ((tapvsi-4000.0)/2000.0 * (ticks[5] - ticks[4])) + ticks[4];

	if (ttapvsi < 0.0)
	{
		tapvsi = ttapvsi;
		np *= -1.0;
	}

	setColor(1.0,0.6,1.0);  //MAGENTA

	glBegin(GL_LINES);
		glVertex2d(e, np-0.5);
		glVertex2d(s, np-0.5);
		glVertex2d(e, np+0.5);
		glVertex2d(s, np+0.5);
	glEnd();

	setColor(0.0,0.0,0.0);
	glRectf(atright+10.0, 20.0, atright+20.0, -20.0);


	}

void draw_altitudebar(void)
	{
		double TempVertices[1000];
		double TapeAlt;
		double smoothalt;
		double line = stbottom-1.9; //-3.5;
		double center = 0.0; //line + 19.7;

		smoothalt = ((int)((alt)*10.0+0.5))/10.0;

		// ------------------------------------
		// Autopilot Altitude setting box
		// ------------------------------------
		setColor(br, bg, bb);
		glRectf(atleft, attop, atright, atbottom);

		glEnable(GL_STENCIL_TEST);
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_NEVER, 0x0, 0x0);
		glStencilOp(GL_INCR, GL_INCR, GL_INCR);

		setColor(1,1,1);
		glRectf(atleft-2.0, attop, atright, atbottom);

		glStencilFunc(GL_EQUAL, 0x1, 0x1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);


//		setColor(0,0,0);
//		glRectf(25.0, 45.0, 38.0, 49.0);
//		setColor(1,1,1);
//		glLineWidth(1.0);

//		TempVertices[0] = 25.0;	TempVertices[1] = 45.0;
//		TempVertices[2] = 38.0;	TempVertices[3] = 45.0;
//		TempVertices[4] = 38.0;	TempVertices[5] = 49.0;
//		TempVertices[6] = 25.0;	TempVertices[7] = 49.0;
//		TempVertices[8] = 25.0;	TempVertices[9] = 45.0;
//		glVertexPointer(2, GL_FLOAT, 0, TempVertices);
//		glDrawArrays(GL_LINE_STRIP, 0, 5);
//		setColor(1,0,1);
//		sprintf(display_buffer, "%.0fFT", apalt);
//		Puts(25.5, 46.5, 3, display_buffer, 0);

		// ----------------------------
		// Vertical line
		// ----------------------------

//		glLineWidth(2.0);

//		TempVertices[0] = 26.0; TempVertices[1] = line;
//		TempVertices[2] = 26.0; TempVertices[3] = line + 40.0;
//		glVertexPointer(2, GL_FLOAT, 0, TempVertices);
//		glDrawArrays(GL_LINES, 0, 2);

		// --------------------------------------
		// Draw the altitude tape
		// --------------------------------------
		setColor(1,1,1);
		glLineWidth(2.0);
		for (TapeAlt = floor(alt - 420.0); TapeAlt <= floor(alt+420.0); TapeAlt+=1.0 /*0.1*/,line+=(0.0087*10.0))
		{
			if (TapeAlt < 0.0)
				continue;

			if ((int)TapeAlt == ((int)((apalt)*10.0+0.5))/10.0)
			{
				setColor(1.0, 0.6, 1.0); // MAGENTA
				glLineWidth(2.0);

				glBegin(GL_LINE_STRIP);
				glVertex2d(atleft, line);
				glVertex2d(atleft-1.0, line+1);
				glVertex2d(atleft-1.0, line+4);
				glVertex2d(atleft+4.0, line+4);
				glVertex2d(atleft+4.0, line-4);
				glVertex2d(atleft-1.0, line-4);
				glVertex2d(atleft-1.0, line-1);
				glVertex2d(atleft, line);
				glEnd();
				setColor(1,1,1);
			}
			sprintf(display_buffer, "%.1f", TapeAlt);
			if (
				(strcmp(&display_buffer[strlen(display_buffer)-5],"000.0") == 0) ||
				(strcmp(&display_buffer[strlen(display_buffer)-5],"200.0") == 0) ||
				(strcmp(&display_buffer[strlen(display_buffer)-5],"400.0") == 0) ||
				(strcmp(&display_buffer[strlen(display_buffer)-5],"600.0") == 0) ||
				(strcmp(&display_buffer[strlen(display_buffer)-5],"800.0") == 0)
				)
			{
				sprintf(display_buffer, "%.0f", floor(TapeAlt/1000.0));
				Puts(atleft+3.0, line-1.5, 4, display_buffer,0);
				sprintf(display_buffer, "%.0f", TapeAlt);
				Puts(atleft+6.7, line-1.2, 3, &display_buffer[strlen(display_buffer)-3],0);
				glBegin(GL_LINES);
				glVertex2f(atleft, line);
				glVertex2f(27.0, line);
				glEnd();
			}
			else if (strcmp(&display_buffer[strlen(display_buffer)-4],"00.0") == 0)
			{
//				sprintf(display_buffer, "%.0f", TapeAlt);
//				Puts(29, line-0.5, 3.5, display_buffer,0);
				glBegin(GL_LINES);
				glVertex2f(atleft, line);
				glVertex2f(27.0, line);
				glEnd();
			}
		}

	glDisable(GL_STENCIL_TEST);
		// ---------------------------------------------
		// Current Altitude indicator box
		// ---------------------------------------------

		setColor(0,0,0);
		glRectf(40.0, center+4.0, 34.5, center-4.0);
		glRectf(28.5, center+4.0, 34.5, center-4.0);
		glBegin(GL_TRIANGLES);
				glVertex2f(27.5, center);
				glVertex2f(28.5, center+1.0);
				glVertex2f(28.5, center-1.0);
		glEnd();
		setColor(1,1,1);
		glBegin(GL_LINE_STRIP);
				glVertex2f(27.5, center);
				glVertex2f(28.5, center+1.0);
				glVertex2f(28.5, center+4.0);
				glVertex2f(34.5, center+4.0);
				glVertex2f(34.5, center+4.0);
				glVertex2f(40.0, center+4.0);
				glVertex2f(40.0, center-4.0);
				glVertex2f(34.5, center-4.0);
				glVertex2f(34.5, center-4.0);
				glVertex2f(28.5, center-4.0);
				glVertex2f(28.5, center-1.0);
				glVertex2f(27.5, center);
		glEnd();

		// ---------------------------------
		// Display the HUNDREDS
		// ---------------------------------

		sprintf(display_buffer, "%.0f", floor(alt/100));
		RPuts(35.5, center-1.5, 4, &display_buffer[strlen(display_buffer)-1]);
		sprintf(display_buffer, "%.0f", floor(alt/1000));
		RPuts(33.5, center-2, 5, display_buffer);

		// ----------------------------------------
		// Odometer like display for the TENS
		// ----------------------------------------
		glEnable(GL_STENCIL_TEST);
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_NEVER, 0x0, 0x0);
		glStencilOp(GL_INCR, GL_INCR, GL_INCR);

		setColor(1,1,1);
		glRectf(40.0, center+4.0, 34.5, center-4.0);

		glStencilFunc(GL_EQUAL, 0x1, 0x1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		// --------------------------------
		// Draw using this stencil mask
		// --------------------------------

		line = center+4.5;
		for (double init = alt + 40.0; init > alt - 40.0; init-=1.0, line-= 0.15)
		{
			sprintf(display_buffer, "%.0f", init);
			if ((strcmp(&display_buffer[strlen(display_buffer)-2], "00") == 0)
			|| (strcmp(&display_buffer[strlen(display_buffer)-2], "20") == 0)
			|| (strcmp(&display_buffer[strlen(display_buffer)-2], "40") == 0)
			|| (strcmp(&display_buffer[strlen(display_buffer)-2], "60") == 0)
			|| (strcmp(&display_buffer[strlen(display_buffer)-2], "80") == 0))
			{
				sprintf(display_buffer, "%.0f", init);
				CPuts(37.5, line, 4, &display_buffer[strlen(display_buffer)-2]);
			}
		}
		glDisable(GL_STENCIL_TEST);
	}


/*
void draw_altitudebar(void)
	{
	double					alt_from_baro;
	char					db2[100];
	double					balt;
	double					scaling;
	long					test;
	int						amtr;
	double					alt_copy = alt;
	int						gotap = 0;
	double					factor = 1.0;
	double					init;
	int						loop;
	double					clamp;
	static char				altimeter[7];

	setColor(br, bg, bb);
#ifdef FILL
		glRectf(atleft, attop, atright, atbottom);
#endif
// Correct altitude based on kohlsman and altimeter settings.
	// Kohlsman = all.wAltimeter     
	// Altimeter = all.wBarometricPressure
	alt_from_baro = ((double)all.wAltimeter/16.0 - (double)all.wBarometricPressure/16.0) * 9.000804114572453;

	if ((long)ra <= 2500)
		{
			setColor(1.0,1.0,1.0);
		if ((long)ra <= decision_height)
			{
			if (!all.bPlaneOnGround)
					setColor(1.0,0.0,0.0);
			}
		else if ((long)ra <= 400)
			{
			}
			ltoa((long)ra, display_buffer, 10);
			if ((long)ra > 100)
				{
				switch(display_buffer[strlen(display_buffer)-2])
					{
					case '0':
					case '2':
					case '4':
					case '6':
					case '8':
						display_buffer[strlen(display_buffer)-1] = '0';
						strcpy(radalt, display_buffer);
						break;
					}
				}
			else
				strcpy(radalt, display_buffer);
		if (all.bPlaneOnGround)
			strcpy(radalt, "0");
//		setColor(0.0,0.0,0.0);
//		drawbox(-10.0, -22.5, 2.0, -26.0, 0.0, 1);
		setColor(1.0,1.0,1.0);
//		if (variation == B747)
			RPuts(20, 31.0, 4, radalt);
//		else
//			{
//			CPuts(-4.0, -25.6, 4, radalt);
//			}
		}

	glLineWidth(1.0);
	setColor(1.0, 1.0, 1.0);
		scaling = 350.0;
		factor = 1.0;
	alt_copy = ((int)((alt)*10.0+0.5))/10.0;
	if (FDSActive)
		balt = alt_copy;// + (alt_from_baro*3.2808399);
	else
	{
		balt = alt + (alt_from_baro*3.2808399);
		balt = ((int)((balt)*10.0+0.5))/10.0;
	}	
	for (init = scaling; init > -1.0*scaling; init-=1.0)
		{
		if ((long)(init+alt_copy) == (long)(all.wGroundAlt * 3.2808399)+decision_height)
			{
			setColor(0, 1, 0);
//			if (variation == B747)
//				{
				glLineWidth(2.0);
				glBegin(GL_LINE_STRIP);
					glVertex2d(atright, init/10.0);
					glVertex2d(atleft, init/10.0);
					glVertex2d(atleft-1.5, (init/10.0)-1.0);
					glVertex2d(atleft-1.5, (init/10.0)+1.0);
					glVertex2d(atleft, init/10.0);
				glEnd();
				glLineWidth(1.0);
//				}
			}
		if ((long)(init+alt_copy) == (long)(all.wGroundAlt * 3.2808399))
			{
			setColor(1,1,1);
					{
					setColor(190.0/255.0, 115.0/255.0, 0.0/255.0);
					glLineWidth(2.0);
					glBegin(GL_LINES);
						glVertex2d(atleft, init/10.0);
						glVertex2d(atright, init/10.0);

						glVertex2d(atleft+1.0, init/10.0);
						glVertex2d(atleft+2.5, (init/10.0)-2);
						glVertex2d(atleft+4.0, init/10.0);
						glVertex2d(atleft+5.5, (init/10.0)-2);
						glVertex2d(atleft+7.0, init/10.0);
						glVertex2d(atleft+8.5, (init/10.0)-2);
						glVertex2d(atleft+10.0, init/10.0);
						glVertex2d(atleft+11.5, (init/10.0)-2);
					glEnd();
					}
				glLineWidth(1.0);
			}

		if (bankok)
			{
			if ((long)(init+balt)== (long)apalt)
				{
				gotap = 1;
				setColor(1.0, 0.6, 1.0); // MAGENTA
				glLineWidth(2.0);

				glBegin(GL_LINE_STRIP);
				glVertex2d(atleft, (init/10.0)/factor);
				glVertex2d(atleft-1.0, ((init/10.0)+1)/factor);
				glVertex2d(atleft-1.0, (init/10.0)+4.0);
				glVertex2d(atleft+2.0, (init/10.0)+4.0);
				glVertex2d(atleft+2.0, (init/10.0)-4.0);
				glVertex2d(atleft-1.0, (init/10.0)-4.0);
				glVertex2d(atleft-1.0, ((init/10.0)-1.0)/factor);
				glVertex2d(atleft, (init/10.0)/factor);
				glEnd();
				}
			}
		if (FDSActive)
			test = (long)(((long)(alt_copy))+init);
		else
			test = (long)(((long)(alt_copy+(alt_from_baro*3.2808399)))+init);
			if (test >= 0)
				{
				setColor(1.0, 1.0, 1.0);
				if (!(test % 100))
					{	
					sprintf(db2, "%02ld", (long)(balt+init)/1000);
					sprintf(display_buffer, "%03ld", (long)test % 1000);
					switch(display_buffer[0])
						{
						case '0':
						case '2':
						case '4':
						case '6':
						case '8':
							//Puts(atleft+1.5, (init/10.0)-1, 3.5, db2, 0);
							RPuts(atright-4.3, (init/10.0)-1, 3.5, db2);
							display_buffer[2] = '0';
							RPuts(atright-0.4, (init/10.0)-0.9, 2.5, display_buffer);
							break;
						default:
							break;
						}
					glLineWidth(1.0);
					glBegin(GL_LINES);
						glVertex2d(atleft, init/10.0);
						glVertex2d(atleft+1.5, init/10.0);
					glEnd();
					if (!strcmp(display_buffer, "000"))
						{
						glBegin(GL_LINES);
							glVertex2d(atleft+0.5, (init/10.0)-2);
							glVertex2d(atright-1.0, (init/10.0)-2);
							glVertex2d(atleft+0.5, (init/10.0)+2);
							glVertex2d(atright-1.0, (init/10.0)+2);
						glEnd();
						}
					}
				}
		}
		
//==============================================================
// This is the AP altitude bug position when not in tape range.
//==============================================================
	if (!gotap)
		{
		if (apalt < alt_copy)
			init = -35.0;
		else
			init = 35.0;
		setColor(1.0, 0.6, 1.0); // MAGENTA
		glLineWidth(2.0);
		glBegin(GL_LINE_STRIP);
			glVertex2d(atleft, init/factor);
			glVertex2d(atleft-1.0, (init+1)/factor);
			glVertex2d(atleft-1.0, init+4);
			glVertex2d(atleft+2.0, init+4);
			glVertex2d(atleft+2.0, init-4);
			glVertex2d(atleft-1.0, init-4);
			glVertex2d(atleft-1.0, (init-1)/factor);
			glVertex2d(atleft, init/factor);
		glEnd();
		}
	if (mask)
		{
		setColor(0.0,0.0,cb);
		glRectf(atleft-2.0, 35, atright+3.0, 40);
		glRectf(atleft-2.0, -35, atright+3.0, -40);
		}


	if (bankok)
		{
		setColor(0,1,0);
		if (imperial)
			{
			sprintf(display_buffer, "%.0f", ((double)all.wAltimeter/16.0));
			}
		else
			sprintf(display_buffer, "%.2f", ((double)all.wAltimeter/16.0) / 33.8639);
		if ((strcmp(display_buffer, "1013") == 0) ||(strcmp(display_buffer, "29.92") == 0))
			{
				Puts(atleft+3, -41, 5, "STD", 0);
			}
		else
			{
				Puts(atleft+3, -41, 5, display_buffer, 0);
			}
		}
	setColor(cr,cg,cb);
	if (FDSActive)
		alt_copy;// += (alt_from_baro*3.2808399);
	else
		alt_copy += (alt_from_baro*3.2808399);

		glBegin(GL_POLYGON);
			glVertex2d(atleft+2, -4);
			glVertex2d(atright+2, -4);
			glVertex2d(atright+2, 4);
			glVertex2d(atleft+2, 4);
			glVertex2d(atleft+2, -4);
		glEnd();

		glBegin(GL_POLYGON);
			glVertex2d(atleft+2, 1);
			glVertex2d(atleft+1, 0);
			glVertex2d(atleft+2, -1);
		glEnd();
		setColor(1.0,1.0,1.0);
		glBegin(GL_LINE_STRIP);
			glVertex2d(atleft+2, -4);
			glVertex2d(atright+2, -4);
			glVertex2d(atright+2, 4);
			glVertex2d(atleft+2, 4);
			glVertex2d(atleft+2, 1);
			glVertex2d(atleft+1, 0);
			glVertex2d(atleft+2, -1);
			glVertex2d(atleft+2, -4);
		glEnd();
		glLineWidth(1.0);
		sprintf(display_buffer, "%02ld", (long)((long)alt_copy/1000));
		if ((long)alt_copy/1000 == 0)
			{
			setColor(0.0, 1.0, 0.0);
			glRectf(atleft+2.5, -1.2, atleft+7.0, 1.2);
			}
		else
			Puts(atleft+3.8, -1.2, 4.0, display_buffer, 0);
		setColor(1.0, 1.0, 1.0);

#if 0
	setColor(1.0, 1.0, 1.0);
	glRectf(-1000.0, -1000.0, 1000.0, 1000.0);
#endif

// This sets up an odometer like readout ========================================================
	sprintf(display_buffer, "%05.6f", alt);
	for (loop = strlen(display_buffer); loop >= 0; loop--)
	{
		if (display_buffer[loop] == '.')
		{
			loop-=2;
			break;
		}
	}
	amtr = atof(&display_buffer[loop]);

	sprintf(altimeter, "%03ld", (long)((long)alt_copy % 1000));
	strcpy(display_buffer, altimeter);             // Print the 100'th foot digit
	display_buffer[1] = '\0';
	Puts(atleft+8.7, -0.7, 3.0, display_buffer, 0);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, speedid);

	clamp = amtr / 100.0;
	clamp /= 1.8;
	if (clamp > 1.0 - 0.22)
	{
		clamp = 0.0;
	}
	glBegin (GL_QUADS);
	glTexCoord2f (0.0, clamp+0.22);				glVertex2f (atleft+10.2, 3.5);
	glTexCoord2f (0.12, clamp+0.22);				glVertex2f (atleft+14.0, 3.5);
	glTexCoord2f (0.12, clamp);		glVertex2f (atleft+14.0, -3.5);
	glTexCoord2f (0.0, clamp);			glVertex2f (atleft+10.2, -3.5);
	glEnd ();

	glDisable(GL_TEXTURE_2D);

// ==============================================================================================
	}
*/

// Set FMA positions
#define LEFTFMA		-24.0
#define MIDDLEFMA	-4.0
#define RIGHTFMA	16.0
#define TOPFMA		43.0
#define BOTTOMFMA	40.0

void draw_autopilot(void)
	{
	double					bx, by;
	static int				vstrigger = 0;
	static int				delay = 0;
	double					aph,h;
	static int				landing = 0;

	bx = by = 0;
	/* Draw the autopilot status box */
	setColor(1.0, 1.0, 1.0);
	glLineWidth(2.0);
	// BOEING CENTERPOINTS    -19, -3, and 20
	glBegin(GL_LINES);
		glVertex2i(-15, 45.0);
		glVertex2i(-15, 40.0);
		glVertex2i(7, 45.0);
		glVertex2i(7, 40.0);
		setColor(0.0, 1.0, 0.0);
	glEnd();
	glLineWidth(1.0);
	setColor(0.0, 1.0, 0.0);
	if (landing)
	{
		if ((all.NAVAID_FLAGS & AP_ILS_LOC_ACQ) && (all.NAVAID_FLAGS & AP_ILS_GS_ACQ))
		{
			CPuts(-4.0, 28, 4, "LAND 3");
		}
		else
		{
			if (all.dwAPMaster)
				CPuts(-4.0, 28, 5, "CMD");
			else if (all.FLIGHT_DIRECTOR_ACTIVE)
				CPuts(-4.0, 28, 5, "FD");
		}
	}
	else
	{
		if (all.dwAPMaster)
			CPuts(-4.0, 28, 5, "CMD");
		else if (all.FLIGHT_DIRECTOR_ACTIVE)
			CPuts(-4.0, 28, 5, "FD");
	}

	// Top left (GREEN)
	if (all.dwAPAirSpeedHold)
		{
		CPuts(LEFTFMA, TOPFMA, 3, "SPD");
		}
	else if (all.dwAPMachNumberHold)
		{
		CPuts(LEFTFMA, TOPFMA, 3.0, "MACH");
		}

	if (all.dwAPNav1Hold)
	{
		if ((all.NAVAID_FLAGS & AP_NAV1_RAD_ACQ) == 0)
		{
			setColor(1,1,1);
			CPuts(MIDDLEFMA, BOTTOMFMA, 3, "LNAV");
		}
		else
		{
			setColor(0,1,0);
			CPuts(MIDDLEFMA, TOPFMA, 3, "LNAV");
		}
	}

	if (all.dwAPLocalizerHold)
	{
		landing = 1;
		if ((all.NAVAID_FLAGS & AP_ILS_LOC_ACQ) == 0)
		{
			setColor(1,1,1);
			CPuts(MIDDLEFMA, BOTTOMFMA, 3, "LOC");
		}
		else
		{
			if (ra > 50.0)
			{
				setColor(0,1,0);
				CPuts(MIDDLEFMA, TOPFMA, 3, "LOC");
			}
		}
	}

	if (all.dwAPGlideSlopeHold)
	{
		if (!all.dwAPAltitudeHold)   // MSFS BUG, Glide Slope hold not removed if alt hold selected
		{
			landing = 1;
			if ((all.NAVAID_FLAGS & AP_ILS_GS_ACQ) == 0)
			{
				setColor(1,1,1);
				CPuts(RIGHTFMA, BOTTOMFMA, 3, "G/S");
			}
			else
			{
				setColor(0,1,0);
				CPuts(RIGHTFMA, TOPFMA, 3, "G/S");
			}
		}
	}

	if (all.dwAPHeadingHold)
	{
		h = ceil(hdg);
		aph = ceil((360.0*(double)all.dwAPHeading)/((double)65536));

		setColor(0,1,0);
		if (aph == h)
			CPuts(MIDDLEFMA, TOPFMA, 3, "HDG HOLD");
		else
			CPuts(MIDDLEFMA, TOPFMA, 3, "HDG SEL");
	}

	if (all.wAPVerticalSpeed)
	{
		setColor(0,1,0);
		CPuts(RIGHTFMA, TOPFMA, 3.0, "V/S");
	}		
	else if (all.dwAPAltitudeHold)
	{
		setColor(0,1,0);
		CPuts(RIGHTFMA, TOPFMA, 3.0, "ALT");
	}

	if (landing)
	{
		if ((ra <= 50.0) && (all.bPlaneOnGround != 1))
		{
			setColor(0,1,0);
			CPuts(RIGHTFMA, TOPFMA, 3, "FLARE");
		}
		if (ra <= 1500.0)
		{
			if (!all.bPlaneOnGround)
			{
				setColor(1,1,1);
				CPuts(MIDDLEFMA, BOTTOMFMA, 2, "ROLLOUT");
			}
		}
		if (ra <= 5.0)
		{
			setColor(0,1,0);
			CPuts(MIDDLEFMA, TOPFMA, 3, "ROLLOUT");
		}
	}

	setColor(1.0, 0.6, 1.0); // MAGENTA
	sprintf(display_buffer, "%.0f", apalt);
// *********************************************************
// Fixed the negative AP altitude settings display 
// (Thanks Timm Linder Sept 1, 2003)
// *********************************************************
		Puts(atright-5.5, 36.4, 4, &display_buffer[strlen(display_buffer)-3], 0);
		display_buffer[strlen(display_buffer)-3] = 0;
		RPuts(atright-5.5, 36.0, 5, display_buffer);
/*
	if ((apalt >= 1000.0) || (apalt <= -1000.0))
		{
/*
		if (atoi(display_buffer) >= 10)
		else if (atoi(display_buffer) >= 0)
			Puts(atright-8.1, 36.0, 5, display_buffer, 0);
		if (atoi(display_buffer) <= -10)
			Puts(atright-11.3, 36.0, 5, display_buffer, 0);
		else if (atoi(display_buffer) < 0)
			Puts(atright-8.8, 36.0, 5, display_buffer, 0);

		}
	else
		Puts(atright-7.5, 36.4, 4, display_buffer, 0);
*/
	if (all.NAVAID_FLAGS & NAV1_IS_ILS)
	{		
		glPushMatrix();
		glTranslatef(-4.0, 0.0, 0.0);
		glLineWidth(2.0);
		setColor(1.0, 0.4, 1.0);
//		bx = 27.8;
		bx = 22.8;
		by = -1.0*((double)all.bGlideSlopeNeedle/8.4);
		glBegin(GL_POLYGON);
		glVertex2d(bx, by-2.0);
		glVertex2d(bx+1.5, by);
		glVertex2d(bx, by+2.0);
		glVertex2d(bx-1.5, by);
		glEnd();
		setColor(1.0,1.0,1.0);
		sprintf(display_buffer, "%c", APTSYM);
		CPuts(bx-0.2, 13.4, 3, display_buffer);
		CPuts(bx-0.2, 6.7, 3, display_buffer);
		CPuts(bx-0.2, -0.5, 3, "-");
		CPuts(bx-0.2, -7.3, 3, display_buffer);
		CPuts(bx-0.2, -14.6, 3, display_buffer);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-4.0, 0.0, 0.0);
		glLineWidth(2.0);
		setColor(1.0, 0.4, 1.0);
		if (all.bLocalizerNeedle > 128)
			bx = -1.0*((255-all.bLocalizerNeedle)/6.4);
		else
			bx = (all.bLocalizerNeedle/6.4);
		glBegin(GL_POLYGON);
//		by = -31.0;
		by = -25.0;
		glVertex2d(bx, by-1.5);
		glVertex2d(bx+1.5, by);
		glVertex2d(bx, by+1.5);
		glVertex2d(bx-1.5, by);
		glEnd();
		setColor(1.0,1.0,1.0);
		CPuts(-20, by-.5, 3, display_buffer);
		CPuts(-10, by-.5, 3, display_buffer);
		CPuts(0, by-.5, 3, "I");
		CPuts(10, by-.5, 3, display_buffer);
		CPuts(20, by-.5, 3, display_buffer);
		glPopMatrix();
	}	
	glLineWidth(1.0);
	if (ra <= decision_height)
		setColor(1,0.5,0);
	else
		setColor(0.0,1.0,0.0);
	sprintf(display_buffer, "DH%d", decision_height);
//	Puts(atleft-20.0, 35.2, 3, display_buffer, 0);
	RPuts(20, 35.0, 3, display_buffer);

		setColor(1.0,1.0,1.0);
		RPuts(20, -30.0, 3, "MDA");
		sprintf(display_buffer, "%d", (long)(all.wGroundAlt * 3.2808399)+decision_height);
		RPuts(20, -34.0, 3.5, display_buffer);
}

// Flight director shows how much bank is required for turns and climbs/descents.  
// Once the required inputs are made,  the FD centers.
void draw_flightdirector(void)
	{
    double					fdpitch, fdbank;
	double					b, p;
	static double			vertex[100];   // A vertex array that should be big enough....

	if (all.FLIGHT_DIRECTOR_ACTIVE)
		{
		glPushMatrix();
		glTranslatef(-4.0,0.0, 0.0);
		fdpitch = pitch;
		fdbank = bank;
		if (fdbank > 180.0)
		    fdbank = ((360.0 - fdbank));
		else
			fdbank*=-1.0;
	
	    if (fdpitch > 180.0)
			fdpitch = ((360.0 - fdpitch));
		else
			fdpitch*=-1.0;

		b = fdbank-all.FLIGHT_DIRECTOR_BANK;
		p = fdpitch+all.FLIGHT_DIRECTOR_PITCH;
		p *= -1.0;
//		b *= -1.0;
		if (b < -20.0)
			b = -20.0;
		else if (b > 20.0)
			b = 20.0;
		if (p < -20.0)
			p = -20.0;
		else if (p > 20.0)
			p = 20.0;

		if (simplefd)
			{
			glLineWidth(3.0);
			setColor(1.0, 0.6, 1.0); // MAGENTA
			vertex[0] = b; vertex[1] = 13.0;
			vertex[2] = b; vertex[3] = -13.0;
			vertex[4] = 13; vertex[5] = p;
			vertex[6] = -13; vertex[7] = p;
			glVertexPointer(2, GL_DOUBLE, 0, vertex);
			glDrawArrays(GL_LINES, 0, 4);
			glLineWidth(1.0);
			}
		else
			{
			setColor(0.0,0.0,0.0);
			glVertexPointer(2, GL_DOUBLE, 0, planebasev);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			setColor(1.0,1.0,1.0);
			glVertexPointer(2, GL_DOUBLE, 0, planev);
			glDrawArrays(GL_LINE_STRIP, 0, 8);
			glPushMatrix();
			glTranslatef(0.0, p, 0.0);
			glRotatef(b*-1.0, 0.0, 0.0, 1.0);
			setColor(0.8,0.0,0.8);
			glVertexPointer(2, GL_DOUBLE, 0, wingsv);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glPopMatrix();
			}
		
		glPopMatrix();
		}
	}

#define LEFT 1
#define RIGHT 2

void draw_compass(void)
	{
	int					angle = 360;
	GLUquadricObj		*quadObj1;
	double				aphdg;

	aphdg = ceil((360.0*(double)all.dwAPHeading)/((double)65536));

	setColor(br, bg, bb);
	
	glPushMatrix();
    glTranslatef(-4.0, -63.0, 0.0);   //WAS -3.0, -77.0
    quadObj1 = gluNewQuadric();
    gluQuadricDrawStyle(quadObj1, GLU_FILL);

    gluPartialDisk(quadObj1, 0.0f, compassradius, 30.0f, 1.0f, 270.0f, 180.0);
    gluDeleteQuadric(quadObj1);
/* Draw the compass */
    setColor(1.0, 1.0, 1.0);

	angle = hdg;

	draw_compassface(arcmode^1);
	glPushMatrix();
	glRotatef(wca*-1.0, 0.0, 0.0, 1.0);
	
	glBegin(GL_LINES);
// This is the OBS line
		glVertex2f(0.0, 0.0);
		glVertex2f(0.0, compassradius);
		glVertex2f(-1.0, (compassradius / 2.0) + (compassradius / 4.0));
		glVertex2f( 1.0, (compassradius / 2.0) + (compassradius / 4.0));
	glEnd();
	
	glPopMatrix();

	glPopMatrix();

	// This is the current heading pointer (White triangle);
	glLineWidth(2.0);
	setColor(1.0, 1.0, 1.0);
	glBegin(GL_LINE_STRIP);
	glVertex2d(-5.5, -32.0);
	glVertex2d(-4.0, -35.0);
	glVertex2d(-2.5, -32.0);
	glVertex2d(-5.5, -32.0);
	glEnd();
		
	// Autopilot heading and MAG/TRU display
	setColor(1.0,0.6,1.0); 
	sprintf(display_buffer, "%.0fM", aphdg?aphdg:360.0);
	Puts(-14.0, -45.0, 3, display_buffer, 0);
	setColor(0.0,1.0,0.0);
	Puts(0, -44.5, 2.5, "MAG", 0);
	setColor(0,0,0);
	glRectf(-40,-46,40,-50);

	return;
	}
