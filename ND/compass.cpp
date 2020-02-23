#include <windows.h>
#define MSFS 1
extern HDC 		hdc;
extern HGLRC 		hrc;
extern WNDCLASS 	a;	
extern POINT point;

/* Standard Includes */

#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <math.h>

/* OpenGL */

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#ifndef _MSC_VER
#include <GL/glx.h>
#endif

#define EXTERN 1
#include <defaults.h>
#include <Fs98State.h>
#include <defines.h>
#include <structs.h>
#include <prototypes.h>
#include <fonts.h>
extern char display_buffer[1000];
extern void setColor(double, double, double);
extern double hdg;
extern double compassradius;
extern FS98StateData   all;
extern int arcmode;
extern int trackmode;
extern double wca;

extern double adf;
extern double Nav1Rad;
extern double cb;
extern double Nav2Rad;
// MAP MODES
#define PLANMODE 1
#define MAPMODE  2 
#define VORMODE  3 
#define ILSMODE  4

extern int mapmode;
// AIRBUS COLORS

#define SBOEING 0
#define SAIRBUS 1
int draw_tag(char *st, double z);
void draw_compassface(int rose);
int clr[2] = {0,0};
GLuint compass[2];
extern int draw_needles(void);
GLuint cfl = 0;   // Compass face list
GLuint cnl = 0;   // Compass needle list
extern double br, bg, bb;

int draw_tag(char *st, double z)
{
	CPuts(0.0, compassradius-15.0, 9.0, st);
	return(1);
}

void draw_compassface(int rose)
	{
	double			bearing;
	double 			aphdg;
	double			divisor = 1.0;
	int				odd = 1;
    GLUquadricObj	*quadObj1;

	sprintf(display_buffer, "%.0f", (360.0*(double)all.dwAPHeading)/((double)65536));

    aphdg = atof(display_buffer);

	adf = -1.0*(360.0*((double)all.wAdfNeedle/65536.0));
		
	if (all.adf_id[0])
	{	
		glLineWidth(1.5);
		glPushMatrix();
		glRotatef(adf, 0.0, 0.0, 1.0);
		setColor(0.0, 1.0, 1.0);
		glBegin(GL_LINES);

// POINT
		glVertex2f(0.0, compassradius);
		glVertex2f(2.0, compassradius - 2.0);
		glVertex2f(0.0, compassradius);
		glVertex2f(-2.0, compassradius - 2.0);
		glVertex2f(0.0, compassradius);
		glVertex2f(0.0, compassradius - 17.0);
		glVertex2f(-2.0, compassradius - 15.0);
		glVertex2f(2.0, compassradius - 15.0);
	
// TAIL
		glVertex2f(2.0, -1.0*compassradius + 17.0);
		glVertex2f(-2.0, -1.0*compassradius + 17.0);
		glVertex2f(0.0, -1.0*compassradius + 17.0);
		glVertex2f(0.0, -1.0*compassradius + 2.0);
			
		glVertex2f(0.0, -1.0*compassradius + 2.0);
		glVertex2f(2.0, -1.0*compassradius);
		glVertex2f(0.0, -1.0*compassradius + 2.0);
		glVertex2f(-2.0, -1.0*compassradius);
		glEnd();
		glPopMatrix();
	}
	
	setColor(1.0, 1.0, 1.0);

	glPushMatrix();   // Rotate to current heading and translate if in arc mode
	glRotatef(hdg, 0.0, 0.0, 1.0);
	draw_needles();
//-------------------------------------------------------====
	glPushMatrix();
	glRotatef(hdg*-1.0, 0, 0, 1);

  
	quadObj1 = gluNewQuadric();
	gluQuadricDrawStyle(quadObj1, GLU_FILL);
	setColor(0,0,cb);
	if (!arcmode)
		gluDisk(quadObj1, compassradius+1.0, compassradius*2, 30, 1);
	else
	{
		gluPartialDisk(quadObj1, compassradius+1.0, compassradius*1.5, 30, 1, 320, 90);
	}
         
	gluDeleteQuadric(quadObj1);

	if (arcmode)
	{
		glBegin(GL_TRIANGLES);
			glVertex2f(0,-90);
			glVertex2f(-90, -30);
			glVertex2f(-90, -90);
			glVertex2f(0,-90);
			glVertex2f(90, -30);
			glVertex2f(90, -0);
		glEnd();
	}
	glPopMatrix();
//--------------------------------------------------------====
// AUTOPILOT HEADING BUG ================================
	glPushMatrix();
	glRotatef(aphdg*-1.0, 0.0, 0.0, 1.0);
	divisor = 1.0;
	setColor(1.0, 0.0, 1.0);
	glBegin(GL_LINE_STRIP);
	if (rose)
	{
		compassradius = 70.0;             // FIX
	}
    else
    {
		compassradius = 140.0;
    }
	glVertex2f(-7.0/divisor, (compassradius - 1.0));
	glVertex2f(7.0/divisor, (compassradius - 1.0));
	glVertex2f(7.0/divisor, (compassradius + 2.0));
	glVertex2f(2.0/divisor, (compassradius + 2.0));
	glVertex2f(0.0, compassradius);
	glVertex2f(-2.0/divisor, (compassradius + 2.0));
	glVertex2f(-7.0/divisor, (compassradius + 2.0));
	glVertex2f(-7.0/divisor, (compassradius - 1.0));
	glEnd();
	if (mapmode != PLANMODE)
	{
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0xff00);
		glBegin(GL_LINES);
			glVertex2f(0.0, compassradius);
			glVertex2d(0.0,0.0);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}
	glPopMatrix();
	glPushMatrix();  
	if (rose)
		compass[rose] = 400;
	else
		compass[rose] = 401;
/* Draw the compass */
    setColor(1.0, 1.0, 1.0);
	
	quadObj1 = gluNewQuadric();
	gluQuadricDrawStyle(quadObj1, GLU_LINE);
	if (rose)
	{
		compassradius = 70.0;
	}
    else
    {
		compassradius = 140.0;
    }
	if (arcmode)
	{
		glLineWidth(2.0);
		gluDisk(quadObj1, compassradius, compassradius, 100.0f, 1.0f);
	}
	gluDeleteQuadric(quadObj1);
//REMOVE
	glPushMatrix();
	glRotatef(5.0, 0.0, 0.0, 1.0);
	for (bearing = 0.0; bearing < 360.0; bearing+=5.0)
	{
		glRotatef(-5.0, 0.0, 0.0, 1.0);
		glBegin(GL_LINES);
		if (odd)
		{
			glVertex2f(0.0, compassradius-7.0);
			glVertex2f(0.0, compassradius);
		}
		else
		{
			glVertex2f(0.0, compassradius-5.0);
			glVertex2f(0.0, compassradius);
		}
		glEnd();
		odd^=1;
		switch((int)bearing)
		{
			case 0:
			case 90:
			case 180:
			case 270:
			case 30:
			case 60:
			case 120:
			case 150:
			case 210:
			case 240:
			case 300:
			case 330:
				sprintf(display_buffer, "%d", (int)bearing / 10);
				draw_tag(display_buffer, 6.0);
				break;
		}
	}
	glPopMatrix();
	clr[rose] = 1;
	glPopMatrix();
	glPopMatrix();   // Rotate to current heading and translate if in arc mode

	return;
}

int draw_needles(void)
{
	double 		aphdg;
	double		magvar;

	magvar = (double)(all.wMagVar*360.0/0x10000);
    if (magvar > 180)
        magvar = magvar - 360;
    magvar *= -1.0;


    aphdg = floor((360.0*(double)all.dwAPHeading)/((double)65536));
	if (all.wNav1Obs == 0.0)
		all.wNav1Obs = 360.0;
	glPushMatrix();
	glRotatef((double)all.wNav1Obs*-1.0, 0.0, 0.0, 1.0);
	glLineWidth(1.5);
	setColor(1.0,1.0,1.0);
//	sprintf(display_buffer, "MM: %d", mapmode);
//	Puts(0,0,5,display_buffer, 0);

	if ((mapmode == VORMODE) || (mapmode == ILSMODE))
	{
		if (!arcmode)
		{
			setColor(1,1,1);
			glBegin(GL_LINE_STRIP);
			glVertex2f(0.0, compassradius - 6.0);
			glVertex2f(1.0, (compassradius - 6.0) - 1);

			glVertex2f(1.0, (compassradius / 2.0) + 4.0);
			glVertex2f(3.0, (compassradius / 2.0) + 4.0);
			glVertex2f(3.0, (compassradius / 2.0) + 2.0);
			glVertex2f(1.0, (compassradius / 2.0) + 2.0);

			glVertex2f(1.0, compassradius / 2.0);
			glVertex2f(-1.0, compassradius / 2.0);

			glVertex2f(-1.0, (compassradius / 2.0) + 2.0);
			glVertex2f(-3.0, (compassradius / 2.0) + 2.0);
			glVertex2f(-3.0, (compassradius / 2.0) + 4.0);
			glVertex2f(-1.0, (compassradius / 2.0) + 4.0);
	
			glVertex2f(-1.0, (compassradius - 6.0) - 1);
			glVertex2f(0.0, compassradius - 6.0);
			glEnd();

			glBegin(GL_LINE_STRIP);
			glVertex2f(0.0, -1.0*(compassradius - 6.0));
			glVertex2f(1.0, -1.0*((compassradius - 6.0) - 1));
			glVertex2f(1.0, -1.0*(compassradius / 2.0));
			glVertex2f(-1.0, -1.0*(compassradius / 2.0));
			glVertex2f(-1.0, -1.0*((compassradius - 6.0) - 1));
			glVertex2f(0.0, -1.0*(compassradius - 6.0));
			glEnd();
		}
// ARC MODE  Line extension
		else
		{
			glLineWidth(1.5);
			setColor(1.0, 0.0, 1.0);
			glBegin(GL_LINES);
				glVertex2f(0.0, (compassradius) * -1.0);
				glVertex2f(0.0, ((compassradius / 4.0) * -1.0)-8.0);
	
				glVertex2f(0.0, (compassradius));
				glVertex2f(0.0, ((compassradius / 4.0))+8.0);
			glEnd();
			setColor(1.0, 1.0, 1.0);
			glBegin(GL_LINE_STRIP);
				glVertex2f(0.0, ((compassradius / 4.0))+8.0);
				glVertex2f(1.0, ((compassradius / 4.0))+7.0);
				glVertex2f(1.0, ((compassradius / 4.0)));
				glVertex2f(-1.0, ((compassradius / 4.0)));
				glVertex2f(-1.0, ((compassradius / 4.0))+7.0);
				glVertex2f(0.0, ((compassradius / 4.0))+8.0);
			glEnd();
			glBegin(GL_LINE_STRIP);
				glVertex2f(1.0, ((compassradius / 4.0)*-1.0)-8.0);
				glVertex2f(-1.0, ((compassradius / 4.0)*-1.0)-8.0);
				glVertex2f(-1.0, ((compassradius / 4.0)*-1.0));
				glVertex2f(1.0, ((compassradius / 4.0)*-1.0));
				glVertex2f(1.0, ((compassradius / 4.0)*-1.0)-8.0);
			glEnd();
		}
		glLineWidth(2.0);
//		sprintf(display_buffer, "FLG: %x %s", all.NAVAID_FLAGS, (all.NAVAID_FLAGS & NAV1_IS_ILS)?"YES":"NO");
//		Puts(0, -10, 5, display_buffer, 0);
		if (all.NAVAID_FLAGS & NAV1_IS_ILS)
		{
			sprintf(display_buffer, "%c", APTSYM);
			setColor(1.0, 1.0, 1.0);
			CPuts(-12.5, 0, 6, display_buffer);
			CPuts(-25.0, 0, 6, display_buffer);
			CPuts(12.5, 0, 6, display_buffer);
			CPuts(25, 0, 6, display_buffer);
			setColor(1.0, 0.0, 1.0);
//			sprintf(display_buffer, "NDL: %d", all.bLocalizerNeedle);
//			Puts(0.0,0.0,5.0,display_buffer, 0);
			if (all.bLocalizerNeedle > 128)
			{
				glBegin(GL_LINE_STRIP);
				if (arcmode)
				{
					glVertex2d((-1.0*(double)((255-all.bLocalizerNeedle)/5))-1.0, compassradius / 4.0);
					glVertex2d((-1.0*(double)((255-all.bLocalizerNeedle)/5))+1.0, compassradius / 4.0);
					glVertex2d((-1.0*(double)((255-all.bLocalizerNeedle)/5))+1.0, (compassradius / 4.0) * -1.0);
					glVertex2d((-1.0*(double)((255-all.bLocalizerNeedle)/5))-1.0, (compassradius / 4.0) * -1.0);
					glVertex2d((-1.0*(double)((255-all.bLocalizerNeedle)/5))-1.0, compassradius / 4.0);
				}
				else             
				{
					glVertex2d((-1.0*(double)((255-all.bLocalizerNeedle)/5))-1.0, compassradius / 2.0);
					glVertex2d((-1.0*(double)((255-all.bLocalizerNeedle)/5))+1.0, compassradius / 2.0);
					glVertex2d((-1.0*(double)((255-all.bLocalizerNeedle)/5))+1.0, (compassradius / 2.0) * -1.0);
					glVertex2d((-1.0*(double)((255-all.bLocalizerNeedle)/5))-1.0, (compassradius / 2.0) * -1.0);
					glVertex2d((-1.0*(double)((255-all.bLocalizerNeedle)/5))-1.0, compassradius / 2.0);
				}
				glEnd();
			}
			else                              
			{
				glBegin(GL_LINE_STRIP);
				if (arcmode)
				{
					glVertex2d((double)(((double)all.bLocalizerNeedle)/5)-1.0, compassradius / 4.0);
					glVertex2d((double)(((double)all.bLocalizerNeedle)/5)+1.0, compassradius / 4.0);
					glVertex2d((double)(((double)all.bLocalizerNeedle)/5)+1.0, (compassradius / 4.0) * -1.0);
					glVertex2d((double)(((double)all.bLocalizerNeedle)/5)-1.0, (compassradius / 4.0) * -1.0);
					glVertex2d((double)(((double)all.bLocalizerNeedle)/5)-1.0, compassradius / 4.0);
				}
				else             
				{
					glVertex2d((double)(((double)all.bLocalizerNeedle)/5)-1.0, compassradius / 2.0);
					glVertex2d((double)(((double)all.bLocalizerNeedle)/5)+1.0, compassradius / 2.0);
					glVertex2d((double)(((double)all.bLocalizerNeedle)/5)+1.0, (compassradius / 2.0) * -1.0);
					glVertex2d((double)(((double)all.bLocalizerNeedle)/5)-1.0, (compassradius / 2.0) * -1.0);
					glVertex2d((double)(((double)all.bLocalizerNeedle)/5)-1.0, compassradius / 2.0);
				}
				glEnd();
			}
		}
		setColor(1.0,1.0,1.0);
	}
	glPopMatrix();
	glLineWidth(3.5);
	if (all.vor1_id[0] && ((all.NAVAID_FLAGS & NAV1_IS_ILS) == 0))
	{
		glLineWidth(1.5);
		glPushMatrix();
		glRotatef((Nav1Rad)*-1.0, 0.0, 0.0, 1.0);
		setColor(0.0, 1.0, 0.0);
		glBegin(GL_LINES);

// POINT
		glVertex2f(0.0, compassradius);
		glVertex2f(2.0, compassradius - 2.0);
		glVertex2f(0.0, compassradius);
		glVertex2f(-2.0, compassradius - 2.0);
		glVertex2f(0.0, compassradius);
		glVertex2f(0.0, compassradius - 17.0);
		glVertex2f(-2.0, compassradius - 15.0);
		glVertex2f(2.0, compassradius - 15.0);
	
// TAIL
		glVertex2f(2.0, -1.0*compassradius + 17.0);
		glVertex2f(-2.0, -1.0*compassradius + 17.0);
		glVertex2f(0.0, -1.0*compassradius + 17.0);
		glVertex2f(0.0, -1.0*compassradius + 2.0);
				
		glVertex2f(0.0, -1.0*compassradius + 2.0);
		glVertex2f(2.0, -1.0*compassradius);
		glVertex2f(0.0, -1.0*compassradius + 2.0);
		glVertex2f(-2.0, -1.0*compassradius);
		glEnd();
		glPopMatrix();
	}
	if (all.vor2_id[0])
	{
		glLineWidth(1.5);
		glPushMatrix();
		glRotatef((Nav2Rad)*-1.0, 0.0, 0.0, 1.0);
		setColor(0.0, 1.0, 0.0);
		glBegin(GL_LINE_STRIP);
		glVertex2f(0.0, compassradius);
		glVertex2f(1.0, compassradius - 2.0);

		glVertex2f(1.0, compassradius - 14.0);

		glVertex2f(3.0, compassradius - 14.0);
		glVertex2f(3.0, compassradius - 16.0);
		glVertex2f(-3.0, compassradius - 16.0);
		glVertex2f(-3.0, compassradius - 14.0);
		glVertex2f(-1.0, compassradius - 14.0);

		glVertex2f(-1.0, compassradius - 2.0);
		glVertex2f(0.0, compassradius);

		glEnd();
		glBegin(GL_LINE_STRIP);

		glVertex2f(0.0, -1.0*compassradius + 15.0);
		glVertex2f(1.0, -1.0*compassradius + 13.0);

		glVertex2f(1.0, -1.0*compassradius + 4.0);
		glVertex2f(3.0, -1.0*compassradius + 2.0);
		glVertex2f(3.0, -1.0*compassradius);
		glVertex2f(2.0, -1.0*compassradius);

		glVertex2f(0.0, -1.0*compassradius + 2.0);

		glVertex2f(-2.0, -1.0*compassradius);
		glVertex2f(-3.0, -1.0*compassradius);
		glVertex2f(-3.0, -1.0*compassradius + 2.0);
		glVertex2f(-1.0, -1.0*compassradius + 4.0);

		glVertex2f(-1.0, -1.0*compassradius + 13.0);
		glVertex2f(0.0, -1.0*compassradius + 15.0);
		glEnd();
		glPopMatrix();
	}

	glLineWidth(1.0);

	return(1);
}