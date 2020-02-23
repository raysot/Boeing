/*
** BOEING_EADI.CPP boeing specific code
** Copyright (c) 1999-2002 Don Lafontaine; ALL RIGHTS RESERVED
*/
#include "eadi.h"
void reshape(int w, int h);
double Puts(double, double, double, char *, int);
int flap[10];
int flapsmax[10];
int flapsmin[10];
int flapsret[10];
int maxspd;
int minspd;
int gearmax;
int v1,v2,vr;
double adipos = 90.0;
double spdpos = 00.0;
double altpos = -90.0;
int		topmost = 0;
double tmpx = 0.0,tmpy = 0.0;
extern double tilt;
int mouseptr = 1;

/* The function called whenever a key is pressed. */
void keyPressed(unsigned char key, int x, int y)
	{
	RECT lprect;
	hwnd = GetActiveWindow();    // Make sure this is set properly.
    /* avoid thrashing this procedure */
	x = y = 0;
	switch(key)
		{
		case '.':
			mouseptr^=1;
			ShowCursor(mouseptr);
			break;
		case '1':
			cb -= 0.01;
			if (cb < 0.0)
				cb = 0.0;
			glClearColor(0.0, 0.0, cb, 0.0);
			break;
		case '2':
			cb += 0.01;
			if (cb > 1.0)
				cb = 1.0;
			glClearColor(0.0, 0.0, cb, 0.0);
			break;
		case 'h':
			tmpx -= 0.1;
			break;
		case 'j':
			tmpx += 0.1;
			break;
		case 't':
			tilt += 90.0;
			reshape(width, height);
			if (tilt >= 360.0)
				tilt = 0.0;
			break;
		case 'r':
		case 'R':
			adipos += 1.0;
			break;
		case 'c':
		case 'C':
			adipos -= 1.0;
			break;
		case 'y':
		case 'Y':
			spdpos += 1.0;
			break;
		case 'b':
		case 'B':
			spdpos -= 1.0;
			break;
		case 'u':
		case 'U':
			altpos += 1.0;
			break;
		case 'n':
		case 'N':
			altpos -= 1.0;
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
		case 'q':									// DEBUG ONLY
		case 'Q':
			display_stats^=1;
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

void draw_airspeed(void)
{
	int flip = 0;
	double tias = ias - 30.0;
	double angle = 0.0;
	double deg = 0.0;
	char tmp[5];

	setColor(0.0, 0.8, 1.0);
	Puts(-5.2, -5.7, 3.0, "KNOTS", 0);
	setColor(1.0, 1.0, 1.0);
	GLUquadricObj *quadObj1;
    quadObj1 = gluNewQuadric();
    gluQuadricDrawStyle(quadObj1, GLU_LINE);
	glLineWidth(2.0);
    gluDisk(quadObj1, 1.0f, 1.0f, 360.0f, 1.0f);
	gluDeleteQuadric(quadObj1);
	glLineWidth(1.0);
	glPushMatrix();
	for (deg = 0.0; deg <= 20.0; deg += 4.0)
		{
		glBegin(GL_LINES);
		if (deg == 0.0 || deg == 20.0)
			glVertex2d(0.0, 25.0);
		else
			glVertex2d(0.0, 24.0);
		glVertex2d(0.0, 22.0);
		glEnd();
		glRotatef(-4.0, 0.0, 0.0, 1.0);
		}
	glRotatef(-2.48, 0.0, 0.0, 1.0);
	for (deg = 20.0; deg <= 240.0; deg += 6.48)
		{
		glBegin(GL_LINES);
		if (flip)
		{
			flip = 0;
			glVertex2d(0.0, 25.0);
		}
		else
		{
			flip = 1;
			glVertex2d(0.0, 24.0);
		}
		glVertex2d(0.0, 22.0);
		glEnd();
		glRotatef(-6.48, 0.0, 0.0, 1.0);
		}

	glRotatef(0.78, 0.0, 0.0, 1.0);
	for (deg = 240.0; deg <= 265.0; deg += 5.7)
		{
		glBegin(GL_LINES);
		if (deg+5.7 >= 265.0)
			glVertex2d(0.0, 25.0);
		else
			glVertex2d(0.0, 24.0);
		glVertex2d(0.0, 22.0);
		glEnd();
		glRotatef(-5.7, 0.0, 0.0, 1.0);
		}
	glRotatef(1.7, 0.0, 0.0, 1.0);
	flip = 0;
	for (deg = 270.0; deg < 330.0; deg += 4.0)
		{
		flip++;
		
		glBegin(GL_LINES);
		if (flip == 5)
		{
			flip = 0;
			glVertex2d(0.0, 25.0);
		}
		else
			glVertex2d(0.0, 24.0);
		glVertex2d(0.0, 22.0);
		glEnd();
		glRotatef(-4.0, 0.0, 0.0, 1.0);
		}
	sprintf(display_buffer, "%.12f", deg);
//	MessageBox(0, display_buffer, "DEBUG", 0);
	glPopMatrix();
	if (tias > 0.0)
	{
		if (tias <= 50.0)
		{
			angle = (tias / 50.0) * 20.0;
		}
		else if (tias <= 220.0)
		{
			angle = ((tias-50) / 170.0) * 220.0;
			angle += 20.0;
		}
		else if (tias <= 270.0)
		{
			angle = ((tias-220) / 50.0) * 29.0;
			angle += 240.0;
		}
		else if (tias <= 420.0)
		{
			angle = ((tias-270) / 150.0) * 60.0;
			angle += 269.0;
		}
		else
			angle = 329.0;
	}
	glPushMatrix();
	glRotatef(angle*-1.0, 0.0, 0.0, 1.0);
	glLineWidth(2.0);
	glBegin(GL_LINES);
	glVertex2d(0.0, 1.0);
	glVertex2d(0.0, 22.0);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex2d(-1.0, 19.0);
	glVertex2d(1.0, 19.0);
	glVertex2d(0.0, 22.0);
	glEnd();
	glPopMatrix();

	Puts(-2.0, 17.8, 3.5, "30",0);
	Puts(5.0, 16.1, 3.5, "80",0);
	Puts(9.9, 12.3, 3.5, "100",0);
	Puts(13.6, 5.1, 3.5, "120",0);
	Puts(14.4, -3.5, 3.5, "140",0);
	Puts(11.1, -12.51, 3.5, "160",0);
	Puts(5, -17.4, 3.5, "180",0);
	Puts(-1.4, -21, 3.5, "200",0);
	Puts(-8, -18.9, 3.5, "220",0);
	Puts(-14.3, -15, 3.5, "240",0);
	Puts(-18, -11.3, 3.5, "250",0);
	Puts(-20.4, -1.8, 3.5, "300",0);
	Puts(-19.5, 5.2, 3.5, "350",0);
	Puts(-16, 11.4, 3.5, "400",0);

	sprintf(tmp, "%.0f", ias);
	sprintf(display_buffer, "%3.3s", tmp);
	Puts(-4.5, 5.6, 5.0, display_buffer, 0);
	drawbox(-6.0, 4.0, 6.0, 11.0, 0.0, 0);
//	sprintf(display_buffer, "%.2f %.2f", tmpx, tmpy);
//	Putsx(-10.0, 30.0, 4.0, display_buffer);
}

void draw_altitude(void)
{
	double deg = 0.0;
	int bigticks = 0;
	char altitude[20];
	int tickcount = 1;
	double talt = alt;
	int init;
	setColor(0.0, 1.0, 0.0);
	sprintf(display_buffer, "%.0f", ((double)all.wAltimeter/16.0));
	Puts(-11.5, 8.0, 3.0, "HPA", 0);
	Puts(-12.6, 4.0, 4.0, display_buffer, 0);
	sprintf(display_buffer, "%.2f", ((double)all.wAltimeter/16.0) / 33.8639);
	Puts(6.3, 8.0, 3.0, "IN", 0);
	Puts(2.5, 4.0, 4.0, display_buffer, 0);

	setColor(1.0, 1.0, 1.0);
	GLUquadricObj *quadObj1;
    quadObj1 = gluNewQuadric();
    gluQuadricDrawStyle(quadObj1, GLU_LINE);
	glLineWidth(2.0);
    gluDisk(quadObj1, 1.0f, 1.0f, 360.0f, 1.0f);
	gluDeleteQuadric(quadObj1);
	glLineWidth(1.0);
	glPushMatrix();
	for (deg = 0.0; deg <= 360.0; deg += 7.2)
		{
		glBegin(GL_LINES);
		if (bigticks == 5)
		{
			glVertex2d(0.0, 21.0);
			bigticks = 0;
		}
		else
			glVertex2d(0.0, 23.0);
		glVertex2d(0.0, 25.0);
		glEnd();
		bigticks++;
		glRotatef(-7.2, 0.0, 0.0, 1.0);
		}
	glPopMatrix();
	Puts(-1.0, 16.5, 4.0, "0",0);
	Puts(9.6, 13.3, 4.0, "1",0);
	Puts(16.5, 4.0, 4.0, "2",0);
	Puts(16.5, -7.1, 4.0, "3",0);
	Puts(9.4, -16.5, 4.0, "4",0);
	Puts(-1.0, -19.4, 4.0, "5",0);
	Puts(-11.4, -16.4, 4.0, "6",0);
	Puts(-18.7, -7.1, 4.0, "7",0);
	Puts(-18.7, 4.3, 4.0, "8",0);
	Puts(-12.1, 13.4, 4.0, "9", 0);

	sprintf(display_buffer, "%06.6f", talt);
	for (init = (int)strlen(display_buffer); init > 0; init--)
	{
		if (display_buffer[init] == '.')
		{
			strcpy(altitude, &display_buffer[init-3]);
			break;
		}
	}
	talt = atof(altitude);
	glPushMatrix();
	glRotatef(-1.0*(talt / 1000.0 * 360.0), 0.0, 0.0, 1.0);
	glLineWidth(2.0);
	glBegin(GL_LINES);
	glVertex2d(0.0, 1.0);
	glVertex2d(0.0, 21.0);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex2d(-1.0, 18.0);
	glVertex2d(1.0, 18.0);
	glVertex2d(0.0, 21.0);
	glEnd();
	glPopMatrix();
	sprintf(altitude, "%05.0f", alt);
	sprintf(display_buffer, "%2.2s", altitude);
	Puts(-6.5, -10.2, 5.0, display_buffer,0);
	sprintf(display_buffer, "%5s", altitude);
	Puts(-0.3, -9.7, 4.0, &display_buffer[2],0);
	drawbox(-8.0, -5.0, 8.0, -12.0, 0.0, 0);
	setColor(0.0, 0.8, 0.0);
	if (atoi(display_buffer) < 10000)
		drawbox(-6.7, -10.9, -6.7+3.0, -10.9+5.0, 0.0, 1);

//	sprintf(display_buffer, "%.2f %.2f", tmpx, tmpy);
//	Putsx(-10.0, 30.0, 4.0, display_buffer);

}

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

	int segments = 0;
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
	glLineWidth(2.0);
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
	unsigned int init;
	float fw = 1.0;
	float fh = 2.0;

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
	static GLuint dl = 0;
	static int loops = 0;
	static int time, oldtime;
	GLUquadricObj *quadObj1;
	static double rot = 0;
	double hd;
	double ws;
	RECT lprect;

	if (tilt)
	{
		glPushMatrix();
		glRotatef(tilt, 0, 0, 1.0);
	}

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
				MessageBox(0, "Error setting window settings", "ERROR", 0);
			}
			SetWindowPos(hwnd, HWND_TOP, (int)lprect.left, (int)lprect.top, (int)lprect.right - (int)lprect.left, 
				(int)lprect.bottom - (int)lprect.top, SWP_SHOWWINDOW|SWP_FRAMECHANGED);
			glutShowWindow();
		}
		firsttime = 0;
	}

	glClear(GL_COLOR_BUFFER_BIT);
	if (notconnected)
	{
		setColor(1.0, 0.0, 0.0);
		Puts(-9.0, 0.0, 5.0, "INOP",0);
		drawbox(-16.0, 5.0, 12.0, -1.0, 0.0, 0);
		glutSwapBuffers();
		if (make_socket(1,NULL) == -1)
			notconnected = 1;
		else
			notconnected = 0;
		return;
	}

//	glPushMatrix();
//	glRotatef(tilt, 0.0, 0.0, 1.0);
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

	// Check out bank limits ----------------------------------
	// if bankok == 0,  lot's of things turn off in the AIRBUS PFD
	//      Look for bankok to find them all
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
	// --------------------------------------------------------

	glPushMatrix();
	glTranslatef(0.0, adipos, 0.0);

	draw_horizon();
	
	if (simplefd)
		draw_plane();

//	if (bankok)
//		draw_flightdirector();

	quadObj1 = gluNewQuadric();
	gluQuadricDrawStyle(quadObj1, GLU_FILL);
	setColor(cr, cg, cb);
	if (!dl)
		{
		dl = 300;
		glNewList(dl, GL_COMPILE);
		glPushMatrix();
		glTranslatef(-22.0, 23.9, 0.0);
		    gluPartialDisk(quadObj1, 3.0f, 5.0f, 50.0f, 1.0f, 270.0f,90.0);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(21.8, 23.9, 0.0);
		    gluPartialDisk(quadObj1, 3.0f, 5.0f, 50.0f, 1.0f, 0.0f,90.0);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(21.8,-23.9, 0.0);
		    gluPartialDisk(quadObj1, 3.0f, 5.0f, 50.0f, 1.0f, 90.0f,90.0);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(-22.0, -23.9, 0.0);
		    gluPartialDisk(quadObj1, 3.0f, 5.0f, 50.0f, 1.0f, 180.0f,90.0);
		glPopMatrix();
		glEndList();
		}
	glCallList(dl);
	gluDeleteQuadric(quadObj1);

	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, spdpos, 0.0);

	draw_airspeed();

	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, altpos, 0.0);

	draw_altitude();

	glPopMatrix();

	if (display_stats)
		{
		setColor(1.0, 1.0, 0.0);
		sprintf(display_buffer, "FPS: %03.0f", (double)frames);
		Puts(-15, 35, 3, display_buffer,0);
		}
	if (tilt)
		glPopMatrix();
//	glPopMatrix();
	glutSwapBuffers();
    }


void pitchladder()
	{
	double init;

	if (!pll)
		{
		pll = 311;
		glNewList(pll,GL_COMPILE);
		glVertexPointer(2, GL_DOUBLE, 0, pitchladderv);
		glDrawArrays(GL_LINES, 0, 96);

		for (init = 10.0; init < 60.0; init+=10.0)
			{
			Puts(-12.0, init-1, 3.0, itoa(init, display_buffer, 10),0);
			Puts(8.5, init-1, 3.0, itoa(init, display_buffer, 10),0);
			Puts(-12.0, (init+1)*-1.0, 3.0, itoa(init, display_buffer, 10),0);
			Puts(8.5, (init+1)*-1.0, 3.0, itoa(init, display_buffer, 10),0);
			}
		setColor(1,1,1);
		glEndList();
		}
	glCallList(pll);
	}


void draw_horizon(void)
	{
	double slip;
	double x1,y1,x2,y2;
	static double planeheight = 0.0;
    GLUquadricObj *quadObj1;
    quadObj1 = gluNewQuadric();
    gluQuadricDrawStyle(quadObj1, GLU_FILL);

	setColor(sr,sg,sb);
	glRectf(30.0 ,35.0, -30.0, -35.0);

	/* Draw the artificial Horizon */
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
	setColor(1.0, 1.0, 1.0);
	glLineWidth(2.0);
	pitchladder();
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
//		glVertexPointer(2, GL_DOUBLE, 0, bbankv2);
//		glDrawArrays(GL_LINE_STRIP, 0, 5);
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
	glLineWidth(2.0);
	if (mask)
		{
		setColor(0.0,0.0,cb);
			glRectf(-25.0, 90.0+padding, -90.0-padding, -90.0-padding);
			glRectf(24.9, 90.0+padding, 90.0+padding, -90.0-padding);
			glRectf(-26.0, -27.0, 26.9, -90.0-padding);
			glRectf(-26.0, 27.0, 26.9, 90.0+padding);
		}
/*
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
				Putsx(14.7, 22.0, 2.5, "IM");
			else if (all.wMiddleMarker)
				Putsx(13.7, 22.0, 2.5, "MM");
			else if (all.wOuterMarker)
				Putsx(13.7, 22.0, 2.5, "OM");
		}
*/
	gluDeleteQuadric(quadObj1);
	return;
	}

void draw_plane(void)
	{
	double inside;
	double bx,by;
	glLineWidth(2.0);

/* Plane symbol */
		bx = 0.0;
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
		
	glLineWidth(2.0);
	}


// Flight director shows how much bank is required for turns and climbs/descents.  
// Once the required inputs are made,  the FD centers.
void draw_flightdirector(void)
	{
    double fdpitch, fdbank;
	double b, p;
	static double vertex[100];   // A vertex array that should be big enough....

	if (all.FLIGHT_DIRECTOR_ACTIVE)
		{
//		glPushMatrix();
//		glTranslatef(-4.0,0.0, 0.0);
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

		b = fdbank+all.FLIGHT_DIRECTOR_BANK;
		p = fdpitch+all.FLIGHT_DIRECTOR_PITCH;
		p *= -1.0;
		b *= -1.0;
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
			glLineWidth(2.0);
			if (colorstyle == SAIRBUS)
				setColor(0.41, 0.93, 0.39);
			else
				setColor(0.8, 0.0, 0.8);
			vertex[0] = b; vertex[1] = 10.0;
			vertex[2] = b; vertex[3] = -10.0;
			vertex[4] = 10; vertex[5] = p;
			vertex[6] = -10; vertex[7] = p;
			glVertexPointer(2, GL_DOUBLE, 0, vertex);
			glDrawArrays(GL_LINES, 0, 4);
			glLineWidth(2.0);
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
		
//		glPopMatrix();
		}
	}
