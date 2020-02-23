#define BOEING 1
#ifdef BOEING
#ifdef _MSC_VER
#include <windows.h>
#define MSFS 1
extern HDC 				hdc;
// extern WSADATA 		ws;
extern HGLRC 			hrc;
extern WNDCLASS 		a;	
extern POINT			point;
#else
#include <unistd.h>
#include <linux/lp.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/io.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define Sleep(x) usleep(x*1000)
#endif

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
#include <structs.h>
#include <prototypes.h>
extern char					display_buffer[1000];
extern void setColor(double, double, double);
extern double				hdg;
extern double				compassradius;
extern FS98StateData		all;
extern int					arcmode;
extern int					trackmode;
extern double				wca;

extern double				adf;
extern double				Nav1Rad;

extern double				Nav2Rad;

// AIRBUS COLORS

#define SBOEING 0
#define SAIRBUS 1
extern int					style;
int draw_tag(char *st, double z);
void draw_compassface(int rose);
int							clr[2] = {0,0};
GLuint						compass[2];
extern int draw_needles(void);
GLuint						cfl = 0;   // Compass face list
GLuint						cnl = 0;   // Compass needle list
extern double				br, bg, bb;

int draw_tag(char *st, double z)
	{
	z = 3.0;
	CPuts(0.0, compassradius-4, z, st);
	return(1);
	}

/* OLD COMPASS
void draw_compassface(int rose)
	{
	double					tb;
	double					bearing;
	double 					aphdg;
	double					divisor = 1.0;
	int						odd = 1;
    GLUquadricObj			*quadObj1;

	sprintf(display_buffer, "%.0f", (360.0*(double)all.dwAPHeading)/((double)65536));
        aphdg = atof(display_buffer);

		adf = -1.0*(360.0*((double)all.wAdfNeedle/65536.0));
		
		if (all.adf_id[0])
			{	
			glLineWidth(3.0);
			glPushMatrix();
			glRotatef(adf, 0.0, 0.0, 1.0);
			setColor(0.0, 1.0, 1.0);
			glBegin(GL_LINES);
				glVertex2f(0.0, compassradius);
				glVertex2f(2.0, compassradius - 4.0);
				glVertex2f(0.0, compassradius);
				glVertex2f(-2.0, compassradius - 4.0);
				glVertex2f(0.0, compassradius);
				glVertex2f(0.0, compassradius - 20.0);
				glVertex2f(-2.0, compassradius - 20.0);
				glVertex2f(2.0, compassradius - 20.0);
				glVertex2f(0.0, -1.0*compassradius);
				glVertex2f(0.0, -1.0*compassradius + 20.0);
				glVertex2f(-2.0, -1.0*compassradius + 20.0);
				glVertex2f(2.0, -1.0*compassradius + 20.0);
				
				glVertex2f(0.0, -1.0*compassradius+4.0);
				glVertex2f(2.0, -1.0*compassradius);
				glVertex2f(0.0, -1.0*compassradius+4.0);
				glVertex2f(-2.0, -1.0*compassradius);
			glEnd();
			glPopMatrix();
			}
	
		setColor(1.0, 1.0, 1.0);

		glPushMatrix();   // Rotate to current heading and translate if in arc mode
		glRotatef(hdg, 0.0, 0.0, 1.0);
		draw_needles();
//-------------------------------------------------------====
//--------------------------------------------------------====
// AUTOPILOT HEADING BUG ================================
	glPushMatrix();
	glRotatef(aphdg*-1.0, 0.0, 0.0, 1.0);
	divisor = 4.0;
	setColor(1.0, 0.6, 1.0); // MAGENTA
	glBegin(GL_LINE_STRIP);
		glVertex2f(-7.0/divisor, (compassradius - 2.0));
		glVertex2f(7.0/divisor, (compassradius - 2.0));
		glVertex2f(7.0/divisor, compassradius);
		glVertex2f(2.0/divisor, compassradius);
		glVertex2f(0.0, compassradius-1.0);
		glVertex2f(-2.0/divisor, compassradius);
		glVertex2f(-7.0/divisor, compassradius);
		glVertex2f(-7.0/divisor, (compassradius - 2.0));
	glEnd();
	glPopMatrix();

	glPopMatrix();	//Heading
	
	glPushMatrix();  
	if (rose)
		compass[rose] = 400;
	else
		compass[rose] = 401;

    setColor(1.0, 1.0, 1.0);
	
//	quadObj1 = gluNewQuadric();
//	gluQuadricDrawStyle(quadObj1, GLU_LINE);
//	gluDeleteQuadric(quadObj1);
//REMOVE
//		glRotatef(5.0, 0.0, 0.0, 1.0);
		tb = hdg - 60.0;
		if (tb < 0.0) tb+=360.0;
		tb = ((int)((tb)*10.0))/10.0;
//		tb = Round(tb, 1);

		for (double rb = -60.0, bearing = tb; rb < 120.0; bearing+=0.1, rb+=0.1)
			{
//			bearing = Round(bearing, 1);
			bearing = ((int)((bearing)*10.0))/10.0;
			if (fmod(bearing, 5.0) == 0.0)
				{
				glPushMatrix();
				glRotatef(rb, 0.0, 0.0, 1.0);
				glBegin(GL_LINES);
				if (odd)
					{
					glVertex2f(0.0, compassradius-1.0);
					glVertex2f(0.0, compassradius);
					}
				else
					{
					glVertex2f(0.0, compassradius-0.5);
					glVertex2f(0.0, compassradius);
					}
				glEnd();
				odd^=1;
				switch(bearing >= 360.0?(int)(bearing-360.0):(int)bearing)
					{
					case 0:
					case 10:
					case 20:
					case 30:
					case 40:
					case 50:
					case 60:
					case 70:
					case 80:
					case 90:
					case 100:
					case 110:
					case 120:
					case 130:
					case 140:
					case 150:
					case 160:
					case 170:
					case 180:
					case 190:
					case 200:
					case 210:
					case 220:
					case 230:
					case 240:
					case 250:
					case 260:
					case 270:
					case 280:
					case 290:
					case 300:
					case 310:
					case 320:
					case 330:
					case 340:
					case 350:
						sprintf(display_buffer, "%d", (int)bearing / 10);
						draw_tag(display_buffer, 6.0);
						break;
					}
				glPopMatrix();
				}
			}
//		glPopMatrix();
		clr[rose] = 1;
		glPopMatrix();
//	glPopMatrix();   // Rotate to current heading and translate if in arc mode
	return;
	}
*/
void draw_compassface(int rose)
	{
	double					bearing;
	double 					aphdg;
	double					divisor = 1.0;
	int						odd = 1;
    GLUquadricObj			*quadObj1;

	sprintf(display_buffer, "%.0f", (360.0*(double)all.dwAPHeading)/((double)65536));
        aphdg = atof(display_buffer);

		adf = -1.0*(360.0*((double)all.wAdfNeedle/65536.0));
		
		if (all.adf_id[0])
			{	
			glLineWidth(3.0);
			glPushMatrix();
			glRotatef(adf, 0.0, 0.0, 1.0);
			setColor(0.0, 1.0, 1.0);
			glBegin(GL_LINES);
				glVertex2f(0.0, compassradius);
				glVertex2f(2.0, compassradius - 4.0);
				glVertex2f(0.0, compassradius);
				glVertex2f(-2.0, compassradius - 4.0);
				glVertex2f(0.0, compassradius);
				glVertex2f(0.0, compassradius - 20.0);
				glVertex2f(-2.0, compassradius - 20.0);
				glVertex2f(2.0, compassradius - 20.0);
				glVertex2f(0.0, -1.0*compassradius);
				glVertex2f(0.0, -1.0*compassradius + 20.0);
				glVertex2f(-2.0, -1.0*compassradius + 20.0);
				glVertex2f(2.0, -1.0*compassradius + 20.0);
				
				glVertex2f(0.0, -1.0*compassradius+4.0);
				glVertex2f(2.0, -1.0*compassradius);
				glVertex2f(0.0, -1.0*compassradius+4.0);
				glVertex2f(-2.0, -1.0*compassradius);
			glEnd();
			glPopMatrix();
			}
	
		setColor(1.0, 1.0, 1.0);

		glPushMatrix();   // Rotate to current heading and translate if in arc mode
		glRotatef(hdg, 0.0, 0.0, 1.0);
		draw_needles();
//-------------------------------------------------------====
//--------------------------------------------------------====
// AUTOPILOT HEADING BUG ================================
	glPushMatrix();
	glRotatef(aphdg*-1.0, 0.0, 0.0, 1.0);
	divisor = 4.0;
	setColor(1.0, 0.6, 1.0); // MAGENTA
	glLineWidth(2.0);
	glBegin(GL_LINE_STRIP);
		glVertex2f(-7.0/divisor, (compassradius));
		glVertex2f(7.0/divisor, (compassradius));
		glVertex2f(7.0/divisor, compassradius+2.0);
		glVertex2f(5.0/divisor, compassradius+2.0);
		glVertex2f(0.0, compassradius);
		glVertex2f(-5.0/divisor, compassradius+2.0);
		glVertex2f(-7.0/divisor, compassradius+2.0);
		glVertex2f(-7.0/divisor, (compassradius));
	glEnd();
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
				glVertex2f(0.0, compassradius-1.0);
				glVertex2f(0.0, compassradius);
				}
			else
				{
				glVertex2f(0.0, compassradius-0.5);
				glVertex2f(0.0, compassradius);
				}
			glEnd();
			odd^=1;
			switch((int)bearing)
				{
				case 0:
				case 10:
				case 20:
				case 30:
				case 40:
				case 50:
				case 60:
				case 70:
				case 80:
				case 90:
				case 100:
				case 110:
				case 120:
				case 130:
				case 140:
				case 150:
				case 160:
				case 170:
				case 180:
				case 190:
				case 200:
				case 210:
				case 220:
				case 230:
				case 240:
				case 250:
				case 260:
				case 270:
				case 280:
				case 290:
				case 300:
				case 310:
				case 320:
				case 330:
				case 340:
				case 350:
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
	double 						aphdg;
	double						magvar;

	magvar = (double)(all.wMagVar*360.0/0x10000);
    if (magvar > 180)
        magvar = magvar - 360;
    magvar *= -1.0;


        aphdg = floor((360.0*(double)all.dwAPHeading)/((double)65536));
	if (all.wNav1Obs == 0.0)
		all.wNav1Obs = 360.0;

	glLineWidth(3.5);
	if (all.vor2_id[0])
		{
		glLineWidth(4.0);
		glPushMatrix();
		glRotatef((Nav2Rad*-1.0), 0.0, 0.0, 1.0);
		if (style == SBOEING)
			setColor(0.0, 1.0, 0.0);
		else 
			setColor(1,1,1);
		glBegin(GL_LINES);
				glVertex2f(0.0, compassradius);
				glVertex2f(2.0, compassradius - 4.0);
				glVertex2f(0.0, compassradius);
				glVertex2f(-2.0, compassradius - 4.0);

				glVertex2f(0.0, compassradius);
				glVertex2f(0.0, compassradius - 20.0);

				glVertex2f(-2.0, compassradius - 20.0);
				glVertex2f(2.0, compassradius - 20.0);

				glVertex2f(0.0, -1.0*compassradius);
				glVertex2f(0.0, -1.0*compassradius + 20.0);

				glVertex2f(-2.0, -1.0*compassradius + 20.0);
				glVertex2f(2.0, -1.0*compassradius + 20.0);

				glVertex2f(0.0, -1.0*compassradius+4.0);
				glVertex2f(2.0, -1.0*compassradius);
				glVertex2f(0.0, -1.0*compassradius+4.0);
				glVertex2f(-2.0, -1.0*compassradius);

		glEnd();
		glPopMatrix();
		}
	glLineWidth(1.0);

	return(1);
	}

#endif