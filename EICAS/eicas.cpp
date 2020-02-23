#include <Windows.h>
#define FSUIPC 1
#include <Fs98State.h>
#include <structs.h>
#include <main.h>
#undef EXTERN
#include <defaults.h>
#include <prototypes.h>
#include "FSUIPC_User.h"
#include <tgaload.h>
#include <fonts.h>

// Display variations.
#ifdef _MSC_VER
int		mouseptr = 1;
int		windowframes = 1;
extern HDC			hdc;
extern WSADATA		ws;
extern HGLRC		hrc;
extern WNDCLASS		a;
POINT				point;
HWND hwnd;
long wsl;
#endif
int					topmost = 0;
char				avionics = 0;   //  True = set radio and AP info into EICAS message list.
char				APMset = 0;
int					monochrome = 0;
char				airfile[200];
char				APATset = 0;
char				variation = 0;
char *				subversion = "";
int					calibrate = 0;
double              tpercent = 0.0;
#define CLB         0
#define MCT			1
#define TOGA		2
extern int			direct;
double				padding = 0.0;        // This is for users to leave a gap at window edge
extern int			noserver;
int					flaps_reset = 0;
int					flaps_calibrated = 0;
double				ffcorrection = 0.0;
int					notconnected = 0;
int firsttime = 1;
double				tilt = 0;
int					window;
int					window2;
int					width, height;
int					wx, wy;
char				display_buffer[100];
int					eicas = 0;
double				MinOilPSI = 55.0;

int					refresh = 0;	// Make sure that when we switch displays that 									// we have the required data.
FS98StateData		all;
REQUEST				req;
char				config[128];
char				revision[30];
extern double		dimmer;
extern int			frames;
int					emc = 0;
int					fillgauge = 1;
int					display_stats = 0;
double cb = 0.0;
typedef struct eml
	{
	char type;
	char text[90];
	char silenced;
	} EICAS_MSG;

EICAS_MSG			eicas_msg[25];
// EICAS MESSAGES

#define CENTER		0
#define LEFTMAIN	1
#define RIGHTMAIN	2
#define LEFTAUX		3
#define RIGHTAUX	4

#define WARNING		0
#define CAUTION		1
#define ADVISORY	2
#define MIDLEVEL	3
#define LOWLEVEL	4
#define MEMO		5

#define MAXMSG		255
#define VARIATIONS 6
#define B777 0
#define B747 1
#define B738 2
#define A320 3
#define B767 4
#define CTRL 5

#define RADIANS  0.01745329   // Multiply degrees by RADIANS to get radians.
#ifndef _MSC_VER
#define Sleep(x) usleep(x * 1000)
#else
#define MSFS 1
#endif

char *				split_eicas(void);
int 				main(int, char **);
int 				drawmeter(int, int, int, char *, char, char);
int 				drawflaps(int, int, int, int);
void 				drawenginegauge(double, double, double, double, double, int);
void 				drawnggauge(double, double, double, double, double, int);
void 				drawfuelflowgauge(double, double, double, double, double, int);
void 				drawAirbusN1(double, double, double, double, double, int, int);
void 				drawAirbusEGT(int, int, int, double, double, int);
void 				draweprgauge(int, int, int, double, double, int);
int					drawbarenginegauge(double x, double y, double maxvalue, double value, double gsize, char precision);
void 				drawbank(int x, int y, int radius, double hdg);
int 				drawoilgauge(int x, int y, double max, double level, char side, char *name);
void 				check_gear(int, int);
extern void			setColor(double, double, double);
void				getoutofdodge(void);
extern void			dofps(int);

double WindowSize = 90.0;
int draw_arc(double x, double y, double radius, double start_deg, double end_deg)
{
	double x1, x2, y1, y2;
	double rad, pi, init;
	pi = 4 * atan(1.0);

    rad = start_deg * (pi / 180.0);
	x1 = x + sin(rad) * radius;
	y1 = y + cos(rad) * radius;



	setColor(1.0, 1.0, 1.0);
	glLineWidth(2.0);
	glBegin(GL_LINES);

    for (init=start_deg; init < end_deg; init+=2.0) 
	{
          /* Convert degrees to radians */
        rad = init * (pi / 180.0);
		x2 = x + sin(rad) * radius;
		y2 = y + cos(rad) * radius;
		glVertex2f(x1, y1);
		glVertex2f(x2, y2);
		x1 = x2;
		y1 = y2;
    }

	rad = end_deg * (pi / 180.0);
	x2 = x + sin(rad) * radius;
	y2 = y + cos(rad) * radius;
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);

	glEnd();
	return(0);
}

void control_synoptic()
{
	static GLuint textureid;
	static int loaded = 0;

	glEnable      ( GL_TEXTURE_2D );
	if (!loaded)
	{
		textureid = tgaLoadAndBind ( "synoptic.tga",  TGA_LOW_QUALITY );
		loaded = 1;
	}
	else
		glBindTexture(GL_TEXTURE_2D, textureid);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0,0.0); glVertex3f(-90.0, -90.0, 0.0);
	glTexCoord2f(1.0,0.0); glVertex3f(90.0, -90.0, 0.0);
	glTexCoord2f(1.0,1.0); glVertex3f(90.0, 90.0, 0.0);
	glTexCoord2f(0.0,1.0); glVertex3f(-90.0, 90.0, 0.0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glLineWidth(1.0);
	setColor(1.0,1.0,1.0);
	glBegin(GL_LINES);
	glVertex2d(-32.0, 12.0);    // left inbrd
	glVertex2d(-32.0, 37.0);
	glVertex2d(-30.0, 12.0);
	glVertex2d(-34.0, 12.0);
	glVertex2d(-30.0, 37.0);
	glVertex2d(-34.0, 37.0);

	glVertex2d(32.0, 12.0);     // right inbrd
	glVertex2d(32.0, 37.0);
	glVertex2d(30.0, 12.0);
	glVertex2d(34.0, 12.0);
	glVertex2d(30.0, 37.0);
	glVertex2d(34.0, 37.0);

	glVertex2d(-72.0, 12.0);    // left outbrd
	glVertex2d(-72.0, 37.0);
	glVertex2d(-70.0, 12.0);
	glVertex2d(-74.0, 12.0);
	glVertex2d(-70.0, 37.0);
	glVertex2d(-74.0, 37.0);

	glVertex2d(72.0, 12.0);     // right outbrd
	glVertex2d(72.0, 37.0);
	glVertex2d(70.0, 12.0);
	glVertex2d(74.0, 12.0);
	glVertex2d(70.0, 37.0);
	glVertex2d(74.0, 37.0);

	glVertex2d(-28.0, -35.0);   // left elevator
	glVertex2d(-28.0, -60.0);
	glVertex2d(-30.0, -35.0);
	glVertex2d(-26.0, -35.0);
	glVertex2d(-30.0, -60.0);
	glVertex2d(-26.0, -60.0);

	glVertex2d(28.0, -35.0);    // right elevator
	glVertex2d(28.0, -60.0);
	glVertex2d(30.0, -35.0);
	glVertex2d(26.0, -35.0);
	glVertex2d(30.0, -60.0);
	glVertex2d(26.0, -60.0);

	glVertex2d(-23.0, -55.0);   // Rudder
	glVertex2d(23.0, -55.0);
	glVertex2d(-23.0, -53.0);
	glVertex2d(-23.0, -57.0);
	glVertex2d(23.0, -53.0);
	glVertex2d(23.0, -57.0);
	glEnd();
	glBegin(GL_TRIANGLES);
		glVertex2d(-32.0, 24.5+(all.aileron_axis*-1 / 1200));
		glVertex2d(-34.0, 26.5+(all.aileron_axis*-1 / 1200));
		glVertex2d(-34.0, 22.5+(all.aileron_axis*-1 / 1200));
		glVertex2d(-72.0, 24.5+(all.aileron_axis*-1 / 1200));
		glVertex2d(-74.0, 26.5+(all.aileron_axis*-1 / 1200));
		glVertex2d(-74.0, 22.5+(all.aileron_axis*-1 / 1200));
		glVertex2d(32.0, 24.5+(all.aileron_axis / 1200));
		glVertex2d(34.0, 26.5+(all.aileron_axis / 1200));
		glVertex2d(34.0, 22.5+(all.aileron_axis / 1200));
		glVertex2d(72.0, 24.5+(all.aileron_axis / 1200));
		glVertex2d(74.0, 26.5+(all.aileron_axis / 1200));
		glVertex2d(74.0, 22.5+(all.aileron_axis / 1200));
		glVertex2d(28.0, -47.5+(all.elevator_axis / 1200));
		glVertex2d(30.0, -49.5+(all.elevator_axis / 1200));
		glVertex2d(30.0, -45.5+(all.elevator_axis / 1200));
		glVertex2d(-28.0, -47.5+(all.elevator_axis / 1200));
		glVertex2d(-30.0, -49.5+(all.elevator_axis / 1200));
		glVertex2d(-30.0, -45.5+(all.elevator_axis / 1200));
		glVertex2d(0.0+(all.rudder_axis / 700), -55);
		glVertex2d(-2.0+(all.rudder_axis / 700), -57);
		glVertex2d(2.0+(all.rudder_axis / 700), -57);
	glEnd();
	setColor(0.0, 0.8, 1.0);
	Puts(-9.0, -63, 4.0, "RUDDER",0);
	Puts(-41, 5, 4.0, "L INBD",0);
	Puts(-35, 0, 4.0, "AIL",0);
	Puts(23, 5, 4.0, "R INBD",0);
	Puts(28, 0, 4.0, "AIL",0);
	Puts(-83, 5, 4.0, "L OUTBD",0);
	Puts(-77, 0, 4.0, "AIL",0);
	Puts(62, 5, 4.0, "R OUTBD",0);
	Puts(67, 0, 4.0, "AIL",0);
	Puts(-50,-60,4.0,"L ELEV",0);
	Puts(34,-60,4.0,"R ELEV",0);
}


void draw_B777_eicas1(void)
{
	double totfuel;
	double rad;
	double g1x1,g1y1,g1x2,g1y2;
	double g2x1,g2y1,g2x2,g2y2;
	int init;


 	drawenginegauge(-70, 60, 16, 100.0, ((double)all.wEng1EPR * 100.0)/16384.0, 0);
  	drawenginegauge(-22, 60, 16, 100.0, ((double)all.wEng2EPR * 100.0)/16384.0, 0);
	drawenginegauge(-70, 30, 16, 100.0, ((double)all.wEng1N1 * 100.0)/16384.0, 0);
  	drawenginegauge(-22, 30, 16, 100.0, ((double)all.wEng2N1 * 100.0)/16384.0, 0);
	setColor(1.0, 1.0, 0.0);
	if ((int)((double)all.TURB_ENGINE_1_PCT_REVERSER * 100))
	{
		Puts(-60.0,40.0,4.0,"REV",0);
	}
	if ((int)((double)all.TURB_ENGINE_2_PCT_REVERSER * 100))
	{
		Puts(-12.0,40.0,4.0,"REV",0);
	}
  	drawenginegauge(-70, 0, 16, 750.0, ((double)all.wEng1EGT * 860.0)/16384.0, 500);
  	drawenginegauge(-22, 0, 16, 750.0, ((double)all.wEng2EGT * 860.0)/16384.0, 500);

	draw_arc(-5, -75, 10, 180.0, 360.0);
	draw_arc(15, -75, 10, 180.0, 360.0);

    rad = 360.0 * (3.14 / 180.0);
	g1x1 = -5.0 + sin(rad) * 10.0;
	g1y1 = -75.0 + cos(rad) * 10.0;
    rad = 180.0 * (3.14 / 180.0);
	g1x2 = -5.0 + sin(rad) * 10.0;
	g1y2 = -75.0 + cos(rad) * 10.0;
	glBegin(GL_LINES);
	glVertex2d(g1x1, g1y1);
	glVertex2d(g1x1+2.0, g1y1);

	glVertex2d(g1x2, g1y2);
	glVertex2d(g1x2+2.0, g1y2);

	glVertex2d(g1x1+2.0, g1y1);
	glVertex2d(g1x2+2.0, g1y2);
	glEnd();
    rad = 360.0 * (3.14 / 180.0);
	g2x1 = 15.0 + sin(rad) * 10.0;
	g2y1 = -75.0 + cos(rad) * 10.0;
    rad = 180.0 * (3.14 / 180.0);
	g2x2 = 15.0 + sin(rad) * 10.0;
	g2y2 = -75.0 + cos(rad) * 10.0;
	glBegin(GL_LINES);
	glVertex2d(g2x1, g2y1);
	glVertex2d(g2x1+2.0, g2y1);

	glVertex2d(g2x2, g2y2);
	glVertex2d(g2x2+2.0, g2y2);

	glVertex2d(g2x1+2.0, g2y1);
	glVertex2d(g2x2+2.0, g2y2);
	glEnd();

	glBegin(GL_LINES);
	glVertex2d(-13.0, -75.0);
	glVertex2d(-15.0, -75.0);
	glVertex2d(7.0, -75.0);
	glVertex2d(5.0, -75.0);
	glVertex2d(-5.0, -75.0);
	glVertex2d(-5.0, g1y1);
	glVertex2d(-5.0, g1y2);
	glVertex2d(-5.0, g1y2+2.0);
	glVertex2d(15.0, -75.0);
	glVertex2d(15.0, g2y1);
	glVertex2d(15.0, g2y2);
	glVertex2d(15.0, g2y2+2.0);
	glEnd();

	setColor(0.0, 0.8, 1.0);
	Puts(-13.0, -62.0, 5.0, "FWD",0);
	Puts(7.0, -62.0, 5.0, "AFT",0);
	Puts(-1.0, -70.0, 5.0, "OP",0);
	Puts(-1.0, -82.0, 5.0, "CL",0);
	CPuts(-46, 45, 5.0, "EPR");
	CPuts(-46, 15, 5.0, "N1");
	CPuts(-46, -15, 5.0, "EGT");
  	Puts(-85, 80, 6.0, "TAT",0);
	RPuts(53, -78, 5, "TOTAL FUEL");
	RPuts(53, -85, 5, "TEMP");
	Puts(73, -78, 5, "LBS X",0);
	Puts(73, -85, 5, "1000",0);
	Puts(-70.0, -62.0, 5.0, "DUCT PRESS",0);
	Puts(-87.0, -78.0, 5.0, "CAB ALT",0);
	Puts(-50.0, -78.0, 5.0, "RATE",0);
	Puts(-87.0, -85.0, 5.0, "LOG ALT",0);
	Puts(-40.0, -85.0, 5.0, "AP",0);
	setColor(1.0f, 1.0f, 1.0f);

	// PLACEBO, must figure out how to do these... **********************************

	// Duct Pressure
	Puts(-85.0, -62.0, 6.0, "22",0);
	Puts(-35.0, -62.0, 6.0, "22",0);
	// Cabin ALT
	Puts(-62.0, -78.0, 6.0, "100",0);
	// RATE
	Puts(-30.0, -78.0, 6.0, "0",0);
	// AP
	Puts(-30.0, -85.0, 6.0, "0,0",0);

	// ******************************************************************************

	sprintf(display_buffer, "%s%.0f%cC", (double)all.wTAT/256.0 > 0.0?"+":"", (double)all.wTAT/256.0, DEGREE);
  	Puts(-70, 80, 7.0, display_buffer,0);

	totfuel = ((((double) all.dwFuelCenterPct / 83886.0 ) / 100.0 ) *   (double)all.dwFuelCenterCap );
	totfuel += ((((double)all.dwFuelLeftPct / 83886.0 ) / 100.0 ) *     (double)all.dwFuelLeftCap );
	totfuel += ((((double)all.dwFuelLeftAuxPct / 83886.0 ) / 100.0 ) *  (double)all.dwFuelLeftAuxCap );
	totfuel += ((((double)all.dwFuelRightPct / 83886.0 ) / 100.0 ) *    (double)all.dwFuelRightCap );
	totfuel += ((((double)all.dwFuelRightAuxPct / 83886.0 ) / 100.0 ) * (double)all.dwFuelRightAuxCap );
	sprintf(display_buffer, "%.1f", (totfuel*6.78)/1000.0);
	Puts(53, -78, 7.0, display_buffer,0);
	sprintf(display_buffer, "%.0f%cC", (double)all.wOutsideAirTemp/256.0, DEGREE);
	Puts(53, -85, 6.0, display_buffer,0);

	setColor(1.0, 1.0, 0.0);
	for (init = 0; init < emc; init++)
	{
		switch(eicas_msg[init].type)
		{
			case MIDLEVEL:
			case LOWLEVEL:
			case MEMO:
				setColor(1,1,1);
				break;
			case CAUTION:
				setColor(255.0/255.0, 160.0/255.0, 0.0);
				break;
			case WARNING:
				setColor(1,0,0);
				break;
			case ADVISORY:
				setColor(255.0/255.0, 160.0/255.0, 0.0);
				break;
		}
		Puts(10,  75.0-(double)(init*5), 4.0, eicas_msg[init].text,0);
	}
	drawflaps(40, -30, (int)(((double)all.dwFlapsLeft/16383.0)*100.0), (int)(((double)all.dwFlapsCommanded/16383.0)*100.0));
	check_gear(42, -17);
}

void draw_B777_eicas2()
{
	int init;
	int percent1,percent2;
 	drawenginegauge(-70, 60, 16, 100.0, ((double)all.wEng1N2 * 100.0)/16384.0, 0);
  	drawenginegauge(-22, 60, 16, 100.0, ((double)all.wEng2N2 * 100.0)/16384.0, 0);
	setColor(0.0f, 0.8f, 1.0f);
	CPuts(-46, 45, 5.0, "N2");
	CPuts(-46, 30, 5.0, "FF");
	CPuts(-46, 10, 5.0, "OIL");
	CPuts(-46, 4, 5.0, "PRESS");
	CPuts(-46, -18, 5.0, "OIL");
	CPuts(-46, -24, 5.0, "TEMP");
	CPuts(-46, -41, 5.0, "OIL QTY");
	CPuts(-46, -68, 5.0, "VIB");
  	setColor(1.0,1.0,1.0);

	sprintf(display_buffer, "%.1f", all.fEng1FF_PPH/1000.0);
  	drawbox(-78 ,28,-62,35, 0, 0);
  	CPuts(-70, 29.5, 6.0, display_buffer);
	
	sprintf(display_buffer, "%.1f", all.fEng1FF_PPH/1000.0);
  	drawbox(-30, 28,-14,35, 0, 0);
  	CPuts(-22, 29.5, 6, display_buffer);
	
	sprintf(display_buffer, "%03.0f", ((double)all.wEng1OilPressure* 220.0)/65535.0);
	drawbox(-78 ,5, -62, 12, 0, 0); 
  	CPuts(-70, 6.5, 6, display_buffer);
	
	sprintf(display_buffer, "%03.0f", ((double)all.wEng2OilPressure* 220.0)/65535.0);
  	drawbox(-30, 5, -14, 12, 0, 0);  	
  	CPuts(-22, 6.5, 6, display_buffer);

	glBegin(GL_LINES);
	glVertex2d(-61, -2.5);  //Left
	glVertex2d(-61, 18.5);
	glVertex2d(-31, -2.5);  //Right
	glVertex2d(-31, 18.5);
	setColor(1.0, 0.0, 0.0);
	glVertex2d(-59, -2.5);
	glVertex2d(-63, -2.5);
	glVertex2d(-33, -2.5);
	glVertex2d(-29, -2.5);
	glEnd();
	setColor(1.0, 1.0, 1.0);
	percent1 = ((((double)all.wEng1OilPressure* 220.0)/65535.0) / 220) * 100;
	percent1/=5;
	percent2 = ((((double)all.wEng2OilPressure* 220.0)/65535.0) / 220) * 100;
	percent2/=5;
	glBegin(GL_TRIANGLES);
	glVertex2d(-61, -2.5+percent1);
	glVertex2d(-58, (-2.5+percent1)+2.0);
	glVertex2d(-58, (-2.5+percent1)-2.0);
	glVertex2d(-31, -2.5+percent2);
	glVertex2d(-34, (-2.5+percent2)+2.0);
	glVertex2d(-34, (-2.5+percent2)-2.0);
	glEnd();

	sprintf(display_buffer, "%03.0f", ((double)all.wEng1OilTemp * 140.0) / 16384.0);
	drawbox(-78 ,-23, -62, -16, 0, 0); 
  	CPuts(-70, -21.5, 6, display_buffer);
	
	sprintf(display_buffer, "%03.0f", ((double)all.wEng2OilTemp * 140.0) / 16384.0);
  	drawbox(-30, -23, -14, -16, 0, 0);  	
  	CPuts(-22, -21.5, 6, display_buffer);

	glBegin(GL_LINES);
	glVertex2d(-61, -29.5);  //Left
	glVertex2d(-61, -9.5);
	glVertex2d(-31, -29.5);  //Right
	glVertex2d(-31, -9.5);
	setColor(1.0, 0.0, 0.0);
	glVertex2d(-59, -9.5);
	glVertex2d(-63, -9.5);
	glVertex2d(-33, -9.5);
	glVertex2d(-29, -9.5);
	glEnd();
	setColor(1.0, 1.0, 1.0);
	percent1 = ((((double)all.wEng1OilTemp * 140.0) / 16384.0) / 140) * 100;
	percent1/=5;
	percent2 = ((((double)all.wEng2OilTemp * 140.0) / 16384.0) / 140) * 100;
	percent2/=5;
	glBegin(GL_TRIANGLES);
	glVertex2d(-61, -29.5+percent1);
	glVertex2d(-58, (-29.5+percent1)+2.0);
	glVertex2d(-58, (-29.5+percent1)-2.0);
	glVertex2d(-31, -29.5+percent2);
	glVertex2d(-34, (-29.5+percent2)+2.0);
	glVertex2d(-34, (-29.5+percent2)-2.0);
	glEnd();

	sprintf(display_buffer, "%03.0f", ((double)all.dwEng1OilQty * 100.0) / 16384.0);
	drawbox(-78 ,-43,-62,-36, 0, 0);
  	CPuts(-70, -41.5, 6, display_buffer);
	
	sprintf(display_buffer, "%03.0f", ((double)all.dwEng2OilQty * 100.0) / 16384.0);
  	drawbox(-30,-43,-14,-36, 0, 0);  	
  	CPuts(-22, -41.5, 6, display_buffer);
	
	sprintf(display_buffer, "%01.1f", ((double)all.dwEng1Vibration * 5.0) / 16384.0);
  	drawbox(-78 ,-63,-62,-70, 0, 0);
  	CPuts(-70, -68.5, 6, display_buffer);
	
	sprintf(display_buffer, "%01.1f", ((double)all.dwEng1Vibration * 5.0) / 16384.0);
  	drawbox(-30,-63,-14,-70, 0, 0);  	
  	CPuts(-22, -68.5, 6, display_buffer);

	glBegin(GL_LINES);
	glVertex2d(-61, -56.5);  //Left
	glVertex2d(-61, -76.5);
	glVertex2d(-31, -56.5);  //Right
	glVertex2d(-31, -76.5);
	glEnd();
	percent1 = ((((double)all.dwEng1Vibration * 5.0) / 16384.0) / 20) * 100;
	percent2 = ((((double)all.dwEng2Vibration * 5.0) / 16384.0) / 20) * 100;
	glBegin(GL_TRIANGLES);
	glVertex2d(-61, -76.5+percent1);
	glVertex2d(-58, (-76.5+percent1)+2.0);
	glVertex2d(-58, (-76.5+percent1)-2.0);
	glVertex2d(-31, -76.5+percent2);
	glVertex2d(-34, (-76.5+percent2)+2.0);
	glVertex2d(-34, (-76.5+percent2)-2.0);
	glEnd();
	setColor(1.0, 1.0, 1.0);

	if (emc > 15)
	{
		setColor(1.0, 1.0, 0.0);
		for (init = 15; init < emc; init++)
		{
			switch(eicas_msg[init].type)
			{
				case MIDLEVEL:
				case LOWLEVEL:
				case MEMO:
					setColor(1,1,1);
					break;
				case CAUTION:
					setColor(255.0/255.0, 160.0/255.0, 0.0);
					break;
				case WARNING:
					setColor(1,0,0);
					break;
				case ADVISORY:
					setColor(255.0/255.0, 160.0/255.0, 0.0);
					break;
			}
		Puts(10,  75.0-(double)(init*5), 4.0, eicas_msg[init].text,0);
		}
	}
}

int drawOilPSIgauge(double x, double y, double radius, double maxvalue, double value, int yellow)
	{
	double x2, y2;
	double vangle;
	static double x3, y3;
	GLUquadricObj *quadObj2;
	
	glPushMatrix();

	if (value < 0.0)
		value = 0.0;
	if (value > maxvalue)
		value = maxvalue;
	vangle = ((value/maxvalue) * 315.0) + 22.0;
	setColor(1.0f, 1.0f, 1.0f);

	glTranslatef(x, y, 0.0);
	quadObj2 = gluNewQuadric();
	if (value > maxvalue)
		setColor(1.0f,0.0f,0.0f);
	else
		setColor(0.5f,0.5f,0.5f);
//	gluQuadricDrawStyle(quadObj2, GLU_FILL);	
//	gluPartialDisk(quadObj2, 0.0f, (double)radius, 100.0f, 1.0f, 112.0f, vangle-135.0);	
	gluQuadricDrawStyle(quadObj2, GLU_LINE);	
	glLineWidth(2.0);	
	setColor(1,0,0);
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 112.0f, 40.0);
	setColor(1,1,0);
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 152.0f, 40.0);
	setColor(1,1,1);
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 192.0f, 215.0);

	gluDeleteQuadric(quadObj2);	

	glPopMatrix();
	setColor(1,0,0);
	x2 = x + cos((-22.0) * 3.14 / 180.0) * radius;
	y2 = y + sin((-22.0) * 3.14 / 180.0) * radius;
	x3 = x + cos((-22.0) * 3.14 / 180.0) * (radius-2.0);
	y3 = y + sin((-22.0) * 3.14 / 180.0) * (radius-2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	x2 = x + cos((-62.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-62.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-62.0) * 3.14 / 180.0) * (radius+2.0);
	y3 = y + sin((-62.0) * 3.14 / 180.0) * (radius+2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	setColor(1,1,0);
	x2 = x + cos((-102.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-102.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-102.0) * 3.14 / 180.0) * (radius+2.0);
	y3 = y + sin((-102.0) * 3.14 / 180.0) * (radius+2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	setColor(1,1,1);
	x2 = x + cos((-315.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-315.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-315.0) * 3.14 / 180.0) * (radius-2.0);
	y3 = y + sin((-315.0) * 3.14 / 180.0) * (radius-2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	x2 = x + cos((-180.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-180.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-180.0) * 3.14 / 180.0) * (radius-2.0);
	y3 = y + sin((-180.0) * 3.14 / 180.0) * (radius-2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	glLineWidth(1.0);	

	setColor(1.0,1.0,1.0);
	x3 = x + cos((-22.0) * 3.14 / 180.0) * (radius-3.0);
	y3 = y + sin((-22.0) * 3.14 / 180.0) * (radius-3.0);
	Puts(x3, y3, 4.0, "0",0);
	x3 = x + cos((-180.0) * 3.14 / 180.0) * (radius-3.0);
	y3 = y + sin((-180.0) * 3.14 / 180.0) * (radius-3.0);
	Puts(x3, y3, 4.0, "50",0);
	x3 = x + cos((-300.0) * 3.14 / 180.0) * (radius-10.0);
	y3 = y + sin((-300.0) * 3.14 / 180.0) * (radius-10.0);
	Puts(x3, y3, 4.0, "100",0);
	x3 = x + cos((-1.0*vangle) * 3.14 / 180.0) * radius;
	y3 = y + sin((-1.0*vangle) * 3.14 / 180.0) * radius;
	glLineWidth(2.0);

	glBegin(GL_LINES);
	glVertex2f(x, y);
	glVertex2f(x3, y3);
	glEnd();

	glLineWidth(1.0);
	if ( value < 10.0)
		return(1);
	return(0);
	}

void drawOilTempgauge(double x, double y, double radius, double maxvalue, double value, int yellow)
	{
	double x2, y2;
	double vangle;
	static double x3, y3;
	GLUquadricObj *quadObj2;
	
	glPushMatrix();

	if (value < 0.0)
		value = 0.0;
	if (value > maxvalue)
		value = maxvalue;
	vangle = ((value/maxvalue) * 180.0) + 90.0;
	setColor(1.0f, 1.0f, 1.0f);

	glTranslatef(x, y, 0.0);
	quadObj2 = gluNewQuadric();
	if (value > maxvalue)
		setColor(1.0f,0.0f,0.0f);
	else
		setColor(0.5f,0.5f,0.5f);
//	gluQuadricDrawStyle(quadObj2, GLU_FILL);	
//	gluPartialDisk(quadObj2, 0.0f, (double)radius, 100.0f, 1.0f, 135.0f, vangle-135.0);	
	gluQuadricDrawStyle(quadObj2, GLU_LINE);	
	glLineWidth(2.0);	
	setColor(1,1,1);
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 135.0f, 180.0);
	setColor(1,1,0);
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 315.0f, 5.0);
	setColor(1,0,0);
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 320.0f, 40.0);

	gluDeleteQuadric(quadObj2);	

	glPopMatrix();
	setColor(1,1,1);
	x2 = x + cos((-45.0) * 3.14 / 180.0) * radius;
	y2 = y + sin((-45.0) * 3.14 / 180.0) * radius;
	x3 = x + cos((-45.0) * 3.14 / 180.0) * (radius-2.0);
	y3 = y + sin((-45.0) * 3.14 / 180.0) * (radius-2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	x2 = x + cos((-90.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-90.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-90.0) * 3.14 / 180.0) * (radius-2.0);
	y3 = y + sin((-90.0) * 3.14 / 180.0) * (radius-2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	x2 = x + cos((-135.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-135.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-135.0) * 3.14 / 180.0) * (radius-2.0);
	y3 = y + sin((-135.0) * 3.14 / 180.0) * (radius-2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	x2 = x + cos((-180.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-180.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-180.0) * 3.14 / 180.0) * (radius-2.0);
	y3 = y + sin((-180.0) * 3.14 / 180.0) * (radius-2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	setColor(1,1,0);
	x2 = x + cos((-225.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-225.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-225.0) * 3.14 / 180.0) * (radius+2.0);
	y3 = y + sin((-225.0) * 3.14 / 180.0) * (radius+2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	x2 = x + cos((-230.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-230.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-230.0) * 3.14 / 180.0) * (radius-2.0);
	y3 = y + sin((-230.0) * 3.14 / 180.0) * (radius-2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	setColor(1,0,0);
	x2 = x + cos((-231.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-231.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-231.0) * 3.14 / 180.0) * (radius+2.0);
	y3 = y + sin((-231.0) * 3.14 / 180.0) * (radius+2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	x2 = x + cos((-270.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-270.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-270.0) * 3.14 / 180.0) * (radius-2.0);
	y3 = y + sin((-270.0) * 3.14 / 180.0) * (radius-2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();

	setColor(1.0,1.0,1.0);
	x3 = x + cos((-90.0) * 3.14 / 180.0) * (radius-3.0);
	y3 = y + sin((-90.0) * 3.14 / 180.0) * (radius-3.0);
	Puts(x3, y3, 4.0, "0",0);
	x3 = x + cos((-180.0) * 3.14 / 180.0) * (radius-3.0);
	y3 = y + sin((-180.0) * 3.14 / 180.0) * (radius-3.0);
	Puts(x3, y3, 4.0, "100",0);
	x3 = x + cos((-260.0) * 3.14 / 180.0) * (radius-5.0);
	y3 = y + sin((-260.0) * 3.14 / 180.0) * (radius-5.0);
	Puts(x3, y3, 4.0, "200",0);
	x3 = x + cos((-1.0*vangle) * 3.14 / 180.0) * radius;
	y3 = y + sin((-1.0*vangle) * 3.14 / 180.0) * radius;
	glLineWidth(2.0);


	glBegin(GL_LINES);
	glVertex2f(x, y);
	glVertex2f(x3, y3);
	glEnd();

	glLineWidth(1.0);
	}
	

#define VEM 235.0
void draw737VIBgauge(double x, double y, double radius, double maxvalue, double value, int yellow)
	{
	double x2, y2;
	double vangle;
	static double x3, y3;
	GLUquadricObj *quadObj2;
	double init;
	int tick;
	
	glPushMatrix();

	if (value < 0)
			value = 0;
	if (value > maxvalue)
		vangle = 135.0+VEM;
	else
		vangle = 135.0+((value/maxvalue) * VEM);
	setColor(1.0f, 1.0f, 1.0f);

	
	glTranslatef(x, y, 0.0);
	quadObj2 = gluNewQuadric();
	if (value > maxvalue)
		setColor(1.0f,0.0f,0.0f);
	else
		setColor(0.5f,0.5f,0.5f);

	gluQuadricDrawStyle(quadObj2, GLU_FILL);	
	gluPartialDisk(quadObj2, 0.0f, (double)radius, 100.0f, 1.0f, 225.0f, vangle-135.0);	
	gluQuadricDrawStyle(quadObj2, GLU_LINE);	
	glLineWidth(1.0);
	setColor(1.0f, 1.0f, 1.0f);
	glLineWidth(2.0);	
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 225.0f, VEM);
	glLineWidth(1.0);	

	gluDeleteQuadric(quadObj2);	

	glPopMatrix();
	
	setColor(1.0,1.0,1.0);
	for (init = 135.0, tick = 0; init <= VEM+135.0; tick++, init += (VEM/5.0))
		{
		x2 = x + cos((-1.0*init) * 3.14 / 180.0) * radius;
		y2 = y + sin((-1.0*init) * 3.14 / 180.0) * radius;
		x3 = x + cos((-1.0*init) * 3.14 / 180.0) * (radius-3.0);
		y3 = y + sin((-1.0*init) * 3.14 / 180.0) * (radius-3.0);
		glBegin(GL_LINES);
			glVertex2f(x2, y2);
			glVertex2f(x3, y3);
		glEnd();
		x3 = x + cos((-1.0*init) * 3.14 / 180.0) * (radius-(4.0+(tick-1)));
		y3 = y + sin((-1.0*init) * 3.14 / 180.0) * (radius-(4.0+(tick-1)));
		sprintf(display_buffer, "%d", tick);
		Puts(x3, y3, 4, display_buffer,0);
		}
	x3 = x + cos((-1.0*vangle) * 3.14 / 180.0) * radius;
	y3 = y + sin((-1.0*vangle) * 3.14 / 180.0) * radius;
	glLineWidth(2.0);

	glBegin(GL_LINES);
	glVertex2f(x, y);
	glVertex2f(x3, y3);
	glEnd();

	glLineWidth(1.0);
	}
		
void drawHydPSIgauge(double x, double y, double radius, double maxvalue, double value, int yellow)
	{
	double x2, y2;
	double vangle = 0.0;
	static double x3, y3;
	GLUquadricObj *quadObj2;
	
	glPushMatrix();

	if (value < 0)
			value = 0;
	if (value > maxvalue)
		value = maxvalue;
	if (value <= 2.0)
		vangle = ((value/2.0) * 90.0) + 45.0;
	else if (value <= 4.0)
		vangle = (((value-2.0)/2.0) * 180.0) + 135.0;
	setColor(1.0f, 1.0f, 1.0f);

	glTranslatef(x, y, 0.0);
	quadObj2 = gluNewQuadric();
	if (value > maxvalue)
		setColor(1.0f,0.0f,0.0f);
	else
		setColor(0.5f,0.5f,0.5f);
//	gluQuadricDrawStyle(quadObj2, GLU_FILL);	
//	gluPartialDisk(quadObj2, 0.0f, (double)radius, 100.0f, 1.0f, 135.0f, vangle-135.0);	
	gluQuadricDrawStyle(quadObj2, GLU_LINE);	
	glLineWidth(2.0);	
	setColor(1,0,0);
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 135.0f, 90.0);
	setColor(1,1,0);
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 225.0f, 45.0);
	setColor(1,1,1);
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 270.0f, 55.0);
	setColor(1,1,0);
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 325.0f, 30.0);
	setColor(1,0,0);
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 355.0f, 50.0);

	gluDeleteQuadric(quadObj2);	

	glPopMatrix();
	setColor(1,0,0);
	x2 = x + cos((-90.0) * 3.14 / 180.0) * radius;
	y2 = y + sin((-90.0) * 3.14 / 180.0) * radius;
	x3 = x + cos((-90.0) * 3.14 / 180.0) * (radius-2.0);
	y3 = y + sin((-90.0) * 3.14 / 180.0) * (radius-2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	x2 = x + cos((-135.0) * 3.14 / 180.0) * (radius-2.0);
	y2 = y + sin((-135.0) * 3.14 / 180.0) * (radius-2.0);
	x3 = x + cos((-135.0) * 3.14 / 180.0) * (radius+2.0);
	y3 = y + sin((-135.0) * 3.14 / 180.0) * (radius+2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	x2 = x + cos((-270.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-270.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-270.0) * 3.14 / 180.0) * (radius+2.0);
	y3 = y + sin((-270.0) * 3.14 / 180.0) * (radius+2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	x2 = x + cos((-45.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-45.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-45.0) * 3.14 / 180.0) * (radius-2.0);
	y3 = y + sin((-45.0) * 3.14 / 180.0) * (radius-2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	x2 = x + cos((-315.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-315.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-315.0) * 3.14 / 180.0) * (radius-2.0);
	y3 = y + sin((-315.0) * 3.14 / 180.0) * (radius-2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	setColor(1,1,0);
	x2 = x + cos((-180.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-180.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-180.0) * 3.14 / 180.0) * (radius+2.0);
	y3 = y + sin((-180.0) * 3.14 / 180.0) * (radius+2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	x2 = x + cos((-235.0) * 3.14 / 180.0) * (radius);
	y2 = y + sin((-235.0) * 3.14 / 180.0) * (radius);
	x3 = x + cos((-235.0) * 3.14 / 180.0) * (radius+2.0);
	y3 = y + sin((-235.0) * 3.14 / 180.0) * (radius+2.0);
	glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x3, y3);
	glEnd();
	glLineWidth(1.0);	

	setColor(1.0,1.0,1.0);
	x3 = x + cos((-45.0) * 3.14 / 180.0) * (radius-3.0);
	y3 = y + sin((-45.0) * 3.14 / 180.0) * (radius-3.0);
	Puts(x3, y3, 4.0, "0",0);
	x3 = x + cos((-90.0) * 3.14 / 180.0) * (radius-3.0);
	y3 = y + sin((-90.0) * 3.14 / 180.0) * (radius-3.0);
	Puts(x3, y3, 4.0, "1",0);
	x3 = x + cos((-135.0) * 3.14 / 180.0) * (radius-3.0);
	y3 = y + sin((-135.0) * 3.14 / 180.0) * (radius-3.0);
	Puts(x3, y3, 4.0, "2",0);
	x3 = x + cos((-225.0) * 3.14 / 180.0) * (radius-4.0);
	y3 = y + sin((-225.0) * 3.14 / 180.0) * (radius-4.0);
	Puts(x3, y3, 4.0, "3",0);
	x3 = x + cos((-315.0) * 3.14 / 180.0) * (radius-6.0);
	y3 = y + sin((-315.0) * 3.14 / 180.0) * (radius-6.0);
	Puts(x3, y3, 4.0, "4",0);
#if 0
	for (init = 0.0, tick = 0; init <= EM; tick++, init += (EM/10.0))
		{
		x3 = x + cos((-1.0*init) * 3.14 / 180.0) * (radius-4.0);
		y3 = y + sin((-1.0*init) * 3.14 / 180.0) * (radius-4.0);
		switch(tick)
			{
			case 2:
				Puts(x3, y3, 4, "2",0);
				break;
			case 4:
				Puts(x3, y3, 4, "4",0);
				break;
			case 6:
				Puts(x3, y3, 4, "6",0);
				break;
			case 8:
				Puts(x3, y3, 4, "8",0);
				break;
			case 10:
				Puts(x3, y3, 4, "10",0);
				break;
			}
		}
#endif
	x3 = x + cos((-1.0*vangle) * 3.14 / 180.0) * radius;
	y3 = y + sin((-1.0*vangle) * 3.14 / 180.0) * radius;
	glLineWidth(2.0);

	glBegin(GL_LINES);
	glVertex2f(x, y);
	glVertex2f(x3, y3);
	glEnd();
	glLineWidth(2.0);

	glLineWidth(1.0);
	}
	
void B737_fuel_gauge(double x, double y, double radius, int tank)
	{
	double pounds;
	double percentage;
//-----------------------
	char st[10];
	static double x3, y3;
	GLUquadricObj *quadObj2;
	switch(tank)
		{
		case CENTER:
			percentage = ((double)all.dwFuelCenterPct / 8388608.0 ); 
			pounds = percentage * (double)all.dwFuelCenterCap;
			break;
		case LEFTMAIN:
			percentage = ((double)all.dwFuelLeftPct / 8388608.0 ); 
			pounds = percentage * (double)all.dwFuelLeftCap;
			break;
		case RIGHTMAIN:
			percentage = ((double)all.dwFuelRightPct / 8388608.0 ); 
			pounds = percentage * (double)all.dwFuelRightCap;
			break;
		case LEFTAUX:
			percentage = ((double)all.dwFuelLeftAuxPct / 8388608.0 ); 
			pounds = percentage * (double)all.dwFuelLeftAuxCap;
			break;
		case RIGHTAUX:
			percentage = ((double)all.dwFuelRightAuxPct / 8388608.0 ); 
			pounds = percentage * (double)all.dwFuelRightAuxCap;
			break;
		default:
			return;
			break;
		}
	setColor(1,1,1);

	sprintf(st, "%.0f", pounds * 0.454);
	Puts(x-6, y-2.5, 5, st,0);
	glPushMatrix();

	glTranslatef(x, y, 0.0);
	quadObj2 = gluNewQuadric();
	gluQuadricDrawStyle(quadObj2, GLU_LINE);	
	gluPartialDisk(quadObj2, (double)radius-4.0, (double)radius, 10.0f, 1.0f, 225.0f, 270.0);
	glLineWidth(3.0);
	setColor(0,0,0);
	gluPartialDisk(quadObj2, (double)radius-4.0, (double)radius-4.0, 10.0f, 1.0f, 225.0f, 270.0);
	gluPartialDisk(quadObj2, (double)radius, (double)radius, 10.0f, 1.0f, 225.0f, 270.0);

	gluQuadricDrawStyle(quadObj2, GLU_FILL);	
	setColor(1.0f, 1.0f, 1.0f);	
	glLineWidth(1.0);	
	gluPartialDisk(quadObj2, (double)radius-2.0, (double)radius, 100.0f, 1.0f, 225.0f, 270.0*percentage);
	gluQuadricDrawStyle(quadObj2, GLU_LINE);	
	gluPartialDisk(quadObj2, (double)radius, (double)radius, 100.0f, 1.0f, 225.0f, 270.0*percentage);
	gluPartialDisk(quadObj2, (double)radius-2.0, (double)radius-2.0, 100.0f, 1.0f, 225.0f, 270.0*percentage);

	gluDeleteQuadric(quadObj2);	

	glPopMatrix();

	return;
	}


void draw_B738_eicas1()
	{	
	static char eng1lit = 0;
	static char eng2lit = 0;
	static char f11set = 0;
	static char f12set = 0;
	static char f13set = 0;
	static char f21set = 0;
	static char f22set = 0;
	static char f23set = 0;
	static int f11count = 0;
	static int f12count = 0;
	static int f13count = 0;
	static int f21count = 0;
	static int f22count = 0;
	static int f23count = 0;
	static int f1interval = 0;
	static int f2interval = 0;

	fillgauge = 1;
 	drawnggauge(-70, 60, 16, 100.0, ((double)all.wEng1N1 * 100.0)/16384.0, 0);
  	drawnggauge(-22, 60, 16, 100.0, ((double)all.wEng2N1 * 100.0)/16384.0, 0);
  	drawenginegauge(-70, 30, 16, 750.0, ((double)all.wEng1EGT * 860.0)/16384.0, 500);
  	drawenginegauge(-22, 30, 16, 750.0, ((double)all.wEng2EGT * 860.0)/16384.0, 500);
 	drawenginegauge(-70, 0, 16, 100.0, ((double)all.wEng1N2 * 100.0)/16384.0, 0);
  	drawenginegauge(-22, 0, 16, 100.0, ((double)all.wEng2N2 * 100.0)/16384.0, 0);
  	drawfuelflowgauge(-70, -30, 15, 13000.0, ((double)all.fEng1FF_PPH), 0);
  	drawfuelflowgauge(-22, -30, 15, 13000.0, ((double)all.fEng2FF_PPH), 0);
	fillgauge = 0;
/* This is EICAS 1 on the 747 */
  	B737_fuel_gauge(-75, -70, 12, LEFTMAIN);
  	B737_fuel_gauge(-45, -65, 12, CENTER);
  	B737_fuel_gauge(-15, -70, 12, RIGHTMAIN);
 	drawHydPSIgauge(22, -57, 14, 4.0, ((double)all.dwEng1HydraulicPressure/4.0)/1000.0, 0);
  	drawHydPSIgauge(70, -57, 14, 4.0, ((double)all.dwEng2HydraulicPressure/4.0)/1000.0, 0);
	sprintf(display_buffer, "%d", (int)((all.dwEng1HydraulicQty / 16383.0) * 100.0));
	Puts(22-((strlen(display_buffer) - 2)*5), -80, 5.0, display_buffer,0);
	sprintf(display_buffer, "%d", (int)((all.dwEng2HydraulicQty / 16383.0) * 100.0));
	Puts(70-((strlen(display_buffer) - 2)*5), -80, 5.0, display_buffer,0);
	drawbox(17,-81,29,-75, 0, 0);
	drawbox(65,-81,77,-75, 0, 0);
 	fillgauge = 1;
 	draw737VIBgauge(22, -22, 14, 5.0, (((double)all.dwEng1Vibration/16384) * 5.0), 0);
  	draw737VIBgauge(70, -22, 14, 5.0, (((double)all.dwEng2Vibration/16384) * 5.0), 0);
	fillgauge = 0;
	sprintf(display_buffer, "%d", (int)((all.dwEng1OilQty / 16384.0) * 100.0));
	Puts(22-((strlen(display_buffer) - 2)*5), -5, 5.0, display_buffer,0);
	sprintf(display_buffer, "%d", (int)((all.dwEng2OilQty / 16384.0) * 100.0));
	Puts(70-((strlen(display_buffer) - 2)*5), -5, 5.0, display_buffer,0);
	drawbox(17,-6,29,0, 0, 0);
	drawbox(65,-6,77,0, 0, 0);
 	drawOilTempgauge(22, 16, 14, 200.0, ((double)all.wEng1OilTemp* 140.0)/16384.0, 0);
  	drawOilTempgauge(70, 16, 14, 200.0, ((double)all.wEng2OilTemp* 140.0)/16384.0, 0);
 	f13set = drawOilPSIgauge(22, 48, 14, 100.0, ((double)all.wEng1OilPressure* 220.0)/65535.0, 0);
  	f23set = drawOilPSIgauge(70, 48, 14, 100.0, ((double)all.wEng2OilPressure* 220.0)/65535.0, 0);

	if (all.wEng1StarterSwitch == 4)
		f11set = 1;
	else
		f11set = 0;
	if (all.wEng2StarterSwitch == 4)
		f21set = 1;
	else 
		f21set = 0;

	if (f11set)
		{
		setColor(230.0/255.0, 168.0/255.0, 68.0/255.0);
		glRectf(8.0, 80, 38, 86);
		setColor(0,0,0);
		CPuts(23.0, 83.5, 3.0, "START VALVE");
		CPuts(23.0, 80.5, 3.0, "OPEN");
		}
	if (f12set)
		{
		setColor(230.0/255.0, 168.0/255.0, 68.0/255.0);
		glRectf(8.0, 72, 38, 78);
		setColor(0,0,0);
		CPuts(23.0, 75.5, 3.0, "OIL FILTER");
		CPuts(23.0, 72.5, 3.0, "BYPASS");
		}
	if (f13set)
		{
		setColor(230.0/255.0, 168.0/255.0, 68.0/255.0);
		glRectf(8.0, 64, 38, 70);
		setColor(0,0,0);
		CPuts(23.0, 67.5, 3.0, "LOW OIL");
		CPuts(23.0, 64.5, 3.0, "PRESSURE");
		}

	if (f21set)
		{
		setColor(230.0/255.0, 168.0/255.0, 68.0/255.0);
		glRectf(56.0, 80, 86, 86);
		setColor(0,0,0);
		CPuts(71.0, 83.5, 3.0, "START VALVE");
		CPuts(71.0, 80.5, 3.0, "OPEN");
		}
	if (f22set)
		{
		setColor(230.0/255.0, 168.0/255.0, 68.0/255.0);
		glRectf(56.0, 72, 86, 78);
		setColor(0,0,0);
		CPuts(71.0, 75.5, 3.0, "OIL FILTER");
		CPuts(71.0, 72.5, 3.0, "BYPASS");
		}
	if (f23set)
		{
		setColor(230.0/255.0, 168.0/255.0, 68.0/255.0);
		glRectf(56.0, 64, 86, 70);
		setColor(0,0,0);
		CPuts(71.0, 67.5, 3.0, "LOW OIL");
		CPuts(71.0, 64.5, 3.0, "PRESSURE");
		}

	setColor(0.0,1,1);
	glLineWidth(2.0);
	glBegin(GL_LINES);
		glVertex2f(-90.0, -47.5);
		glVertex2f(0, -47.5);
		glVertex2f(0, -35.5);
		glVertex2f(90.0, -35.5);
		glVertex2f(0, 60);
		glVertex2f(0, -90);
	glEnd();

	// LEFT
	Puts(-48, 45, 4.0, "N1",0);
	Puts(-49, 15, 4.0, "EGT",0);
	Puts(-48, -15, 4.0, "N2",0);
	Puts(-52, -40, 4.0, "FF/FU",0);
	Puts(-56.5, -45, 4.0, "KG X 1000",0);
	Puts(-55, -79, 4.0, "FUEL KG",0);
	Puts(-49.5, -62, 4.0, "CTR",0);
	Puts(-76, -65, 4.0, "1",0);
	Puts(-16, -65, 4.0, "2",0);
	// RIGHT
	CPuts(32, -44, 4.0, "A");
	CPuts(59, -44, 4.0, "B");
	CPuts(47, -69, 4.0, "HYD P");
	CPuts(47, -80, 4.0, "HYD Q %");
	CPuts(47, -32, 4.0, "VIB");
	CPuts(47, -5, 4.0, "OIL Q %");
	CPuts(47, 3, 4.0, "OIL T");
	CPuts(47, 33, 4.0, "OIL P");
  	Puts(-30, 80, 5.0, "TAT",0);
	sprintf(display_buffer, "%.0f%cc", (double)all.wTAT/256.0, DEGREE);
	setColor(1.0f, 1.0f, 1.0f);
  	Puts(-18, 80, 6.0, display_buffer,0);

	return;
	}

void set_eicas_msgs(void)
	{
		int eltrim = (all.ELEVATOR_TRIM / 0.262) * 32;
		int aitrim = (all.AILERON_TRIM / 0.174) * 32;
		int rutrim = (all.RUDDER_TRIM / 0.174) * 32;
	emc=0;

	sprintf(eicas_msg[emc++].text, "  ELEVATOR TRIM %d", eltrim);
	eicas_msg[emc-1].type = ADVISORY;
	sprintf(eicas_msg[emc++].text, "  AILERON TRIM %d", aitrim);
	eicas_msg[emc-1].type = ADVISORY;
	sprintf(eicas_msg[emc++].text, "  RUDDER TRIM %d", rutrim);
	eicas_msg[emc-1].type = ADVISORY;

	if ((((double)all.wEng1OilTemp * 140.0) / 16384.0) > 160.0)
		{
		strcpy(eicas_msg[emc++].text, "ENG 1 OIL TEMP");
		eicas_msg[emc-1].type = WARNING;
		}
	if ((((double)all.wEng1OilTemp * 140.0) / 16384.0) < 65.0)
		{
		strcpy(eicas_msg[emc++].text, "ENG 1 OIL TEMP");
		eicas_msg[emc-1].type = WARNING;
		}
	if (((all.wEng1OilPressure * 220.0)/65535.0) < MinOilPSI)
		{
		strcpy(eicas_msg[emc++].text, "ENG 1 OIL PRESS");
		eicas_msg[emc-1].type = WARNING;
		}
	if (!all.wEng1Combustion)
		{
		strcpy(eicas_msg[emc++].text, "ENG 1 SHUTDOWN");
		eicas_msg[emc-1].type = WARNING;
		}
	if (all.wParkingBrake)
		{
		strcpy(eicas_msg[emc++].text, "PARKING BRAKE SET");
		eicas_msg[emc-1].type = WARNING;
		}
	if (all.wEngines >= 2)
		{
		if ((((double)all.wEng2OilTemp * 140.0) / 16384.0) > 160.0)
			{
			strcpy(eicas_msg[emc++].text, "ENG 2 OIL TEMP");
			eicas_msg[emc-1].type = WARNING;
			}
		if ((((double)all.wEng2OilTemp * 140.0) / 16384.0) < 65.0)
			{
			strcpy(eicas_msg[emc++].text, "ENG 2 OIL TEMP");
			eicas_msg[emc-1].type = WARNING;
			}
		if (((all.wEng2OilPressure * 220.0)/65535.0) < MinOilPSI)
			{
			strcpy(eicas_msg[emc++].text, "ENG 2 OIL PRESS");
			eicas_msg[emc-1].type = WARNING;
			}
		if (!all.wEng2Combustion)
			{
			strcpy(eicas_msg[emc++].text, "ENG 2 SHUTDOWN");
			eicas_msg[emc-1].type = WARNING;
			}
		}
	if (all.wEngines >= 3)
		{
		if ((((double)all.wEng3OilTemp * 140.0) / 16384.0) > 160.0)
			{
			strcpy(eicas_msg[emc++].text, "ENG 3 OIL TEMP");
			eicas_msg[emc-1].type = WARNING;
			}
		if ((((double)all.wEng3OilTemp * 140.0) / 16384.0) < 65.0)
			{
			strcpy(eicas_msg[emc++].text, "ENG 3 OIL TEMP");
			eicas_msg[emc-1].type = WARNING;
			}
		if (((all.wEng3OilPressure * 220.0)/65535.0) < MinOilPSI)
			{	
			strcpy(eicas_msg[emc++].text, "ENG 3 OIL PRESS");
			eicas_msg[emc-1].type = WARNING;			
			}
		if (!all.wEng3Combustion)
			{
			strcpy(eicas_msg[emc++].text, "ENG 3 SHUTDOWN");
			eicas_msg[emc-1].type = WARNING;
			}
		}
	if (all.wEngines >= 4)
		{
		if ((((double)all.wEng4OilTemp * 140.0) / 16384.0) > 160.0)
			{
			strcpy(eicas_msg[emc++].text, "ENG 4 OIL TEMP");
			eicas_msg[emc-1].type = WARNING;
			}
		if ((((double)all.wEng4OilTemp * 140.0) / 16384.0) < 65.0)
			{
			strcpy(eicas_msg[emc++].text, "ENG 4 OIL TEMP");
			eicas_msg[emc-1].type = WARNING;
			}
		if (((all.wEng4OilPressure * 220.0)/65535.0) < MinOilPSI)
			{
			strcpy(eicas_msg[emc++].text, "ENG 4 OIL PRESS");
			eicas_msg[emc-1].type = WARNING;
			}
		if (!all.wEng4Combustion)
			{
			strcpy(eicas_msg[emc++].text, "ENG 4 SHUTDOWN");
			eicas_msg[emc-1].type = WARNING;
			}
		}
	if (!all.dwAPMaster)
		{
		if (APMset)
			{
			strcpy(eicas_msg[emc++].text, "AUTOPILOT DISC");
			eicas_msg[emc-1].type = CAUTION;
			}
		}
	else
		{
		APMset = 1;
		}	
	if (!all.dwAPAutoThrottle)
		{
		if (APATset)
			{
			strcpy(eicas_msg[emc++].text, "AUTOTHROTTLE DISC");
			eicas_msg[emc-1].type = WARNING;
			}
		}
	else
		APATset = 1;
	if (!all.byStrobeOn)
		{
		if (!all.bPlaneOnGround)
			{
			strcpy(eicas_msg[emc++].text, "  STROBE LIGHT OFF");
			eicas_msg[emc-1].type = ADVISORY;
			}
		}	
	if (all.bOverSpeed)
		{
		strcpy(eicas_msg[emc++].text, "OVERSPEED");
		eicas_msg[emc-1].type = CAUTION;
		}

	if (avionics)
		{
		if (all.dwAPWingLeveler)
			{
			strcpy(eicas_msg[emc++].text, "  Wing Leveller");
			eicas_msg[emc-1].type = ADVISORY;
			}	
		if (all.dwAPNav1Hold)
			{
			strcpy(eicas_msg[emc++].text, "  NAV 1 Hold");
			eicas_msg[emc-1].type = ADVISORY;
			}
		if (all.dwAPHeadingHold)
			{
			sprintf(eicas_msg[emc++].text, "  Heading Hold %.0f", (360.0 * (double)all.dwAPHeading)/0x10000);
			eicas_msg[emc-1].type = ADVISORY;
			}
		if (all.dwAPAltitudeHold)
			{
			sprintf(eicas_msg[emc++].text, "  Altitude Hold %.0f", (all.dwAPAltitude * 3.2808399) / 0x10000);
			eicas_msg[emc-1].type = ADVISORY;
			}
		if (all.dwAPAttitudeHold)
			{
			strcpy(eicas_msg[emc++].text, "  Attitude Hold");
			eicas_msg[emc-1].type = ADVISORY;
			}
		if (all.dwAPAirSpeedHold)
			{
			sprintf(eicas_msg[emc++].text, "  Air Speed Hold %.0f", (double)all.wAPAirSpeed);
			eicas_msg[emc-1].type = ADVISORY;
			}
		if (all.dwAPMachNumberHold)
			{
			sprintf(eicas_msg[emc++].text, "  MACH hold %.3f", (double)all.dwAPMachNumber/65536.0);
			eicas_msg[emc-1].type = ADVISORY;
			}
		if (all.dwAPVerticalSpeedHold)
			{
			sprintf(eicas_msg[emc++].text, "  Vertical Speed Hold %.0f", (double)all.wAPVerticalSpeed);
			eicas_msg[emc-1].type = ADVISORY;
			}
		if (all.dwAPRPMHold)
			{
			sprintf(eicas_msg[emc++].text, "  RPM Hold %.0f", (double)all.wAPRPM);
			eicas_msg[emc-1].type = ADVISORY;
			}
		if (all.dwAPGlideSlopeHold)
			{
			strcpy(eicas_msg[emc++].text, "  Glide Slope Hold");
			eicas_msg[emc-1].type = ADVISORY;
			}
		if (all.dwAPLocalizerHold)
			{
			strcpy(eicas_msg[emc++].text, "  Localizer Hold");
			eicas_msg[emc-1].type = ADVISORY;
			}
		if (all.dwAPBackCourseHold)
			{
			strcpy(eicas_msg[emc++].text, "  Back Course Hold");
			eicas_msg[emc-1].type = ADVISORY;
			}
		if (all.dwAPYawDamper)
			{
			strcpy(eicas_msg[emc++].text, "  Yaw Damper");
			eicas_msg[emc-1].type = ADVISORY;
			}
		if (all.dwAPToGa)
			{
			strcpy(eicas_msg[emc++].text, "  TOGA");
			eicas_msg[emc-1].type = ADVISORY;
			}
		if (all.dwAPAutoThrottle)
			{
			strcpy(eicas_msg[emc++].text, "  Auto Throttle");
			eicas_msg[emc-1].type = ADVISORY;
			}
		}
	}
/*
** This is pretty much a debug tool for now,  
** but I might use it to calibrate flaps etc settings.
*/

int get_flappos()
	{
	double increment = 16383.0 / (double)flaps_calibrated;
	double init;
	int count = 0;

	for (init = 0.0; init <= 0x7fff; init += increment)
		{
		if ((all.dwFlapsCommanded > (init-100.0)) && (all.dwFlapsCommanded < (init + 100.0)))
			return(count);
		count++;
		}
	return(0);
	}

void calibrate_instrument()
	{
	if (!flaps_reset)
		{
		if (all.dwFlapsCommanded == 0)
			{
			flaps_reset = 1;
			}
		else
			{
			setColor(1.0,0,0);
			Puts(-10, 0, 5, "PLEASE SET FLAPS UP",0);
			}
		}
	else if (!flaps_calibrated)
		{
		if (!all.dwFlapsCommanded)
			{
			setColor(1.0, 0.0, 0.0);
			Puts(-10, 0, 5, "PLEASE SET ONE NOTCH OF FLAPS",0);	
			}
		else
			flaps_calibrated = 16383 / all.dwFlapsCommanded;
		}
	else
		{
		setColor(0,0,1);
		Puts(-85.0, 80.0, 4, "FLAPS :",0);
		Puts(-85.0, 76.0, 4, "GEAR :",0);
		Puts(-85.0, 72.0, 4, "EPR :",0);
		Puts(-85.0, 68.0, 4, "N1 :",0);
		Puts(-85.0, 64.0, 4, "N2 :",0);
		Puts(-85.0, 60.0, 4, "FF :",0);
		Puts(-85.0, 56.0, 4, "EGT :",0);
		Puts(-85.0, 52.0, 4, "Oil Pressure :",0);
		Puts(-85.0, 48.0, 4, "Oil Temperature :",0);
		Puts(-85.0, 44.0, 4, "Oil Quantity :",0);
		Puts(-85.0, 40.0, 4, "Vibration :",0);
		setColor(1,1,1);
		sprintf(display_buffer, "(%d)", flaps_calibrated); 
		Puts(-65.0, 80.0, 4, display_buffer,0);
		sprintf(display_buffer, "Left %ld", (long)all.dwFlapsLeft); 
		Puts(-55.0, 80.0, 4, display_buffer,0);
		sprintf(display_buffer, "Right %ld", (long)all.dwFlapsRight); 
		Puts(-25.0, 80.0, 4, display_buffer,0);
		sprintf(display_buffer, "Commanded %ld (%d)", (long)all.dwFlapsCommanded, get_flappos()); 
		Puts(5.0, 80.0, 4, display_buffer,0);

		sprintf(display_buffer, "Left %ld", (long)all.dwLeftGearPosition);
		Puts(-55.0, 76.0, 4, display_buffer,0);
		sprintf(display_buffer, "Right %ld", (long)all.dwRightGearPosition);
		Puts(-25.0, 76.0, 4, display_buffer,0);
		sprintf(display_buffer, "Nose %ld", (long)all.dwNoseGearPosition);
		Puts(5.0, 76.0, 4, display_buffer,0);

		sprintf(display_buffer, "1 %.2f", (double)all.wEng1EPR);
		Puts(-55.0, 72.0, 4, display_buffer,0);
		sprintf(display_buffer, "2 %.2f", (double)all.wEng2EPR);
		Puts(-25.0, 72.0, 4, display_buffer,0);
		sprintf(display_buffer, "3 %.2f", (double)all.wEng3EPR); 
		Puts(5.0, 72.0, 4, display_buffer,0);
		sprintf(display_buffer, "4 %.2f", (double)all.wEng4EPR); 
		Puts(35.0, 72.0, 4, display_buffer,0);

		sprintf(display_buffer, "1 %.2f", (double)all.wEng1N1); 
		Puts(-55.0, 68.0, 4, display_buffer,0);
		sprintf(display_buffer, "2 %.2f", (double)all.wEng2N1); 
		Puts(-25.0, 68.0, 4, display_buffer,0);
		sprintf(display_buffer, "3 %.2f", (double)all.wEng3N1); 
		Puts(5.0, 68.0, 4, display_buffer,0);
		sprintf(display_buffer, "4 %.2f", (double)all.wEng4N1); 
		Puts(35.0, 68.0, 4, display_buffer,0);

		sprintf(display_buffer, "1 %.2f", (double)all.wEng1N2); 
		Puts(-55.0, 64.0, 4, display_buffer,0);
		sprintf(display_buffer, "2 %.2f", (double)all.wEng2N2); 
		Puts(-25.0, 64.0, 4, display_buffer,0);
		sprintf(display_buffer, "3 %.2f", (double)all.wEng3N2); 
		Puts(5.0, 64.0, 4, display_buffer,0);
		sprintf(display_buffer, "4 %.2f", (double)all.wEng4N2); 
		Puts(35.0, 64.0, 4, display_buffer,0);

		sprintf(display_buffer, "1 %.2f", (double)all.fEng1FF_PPH); 
		Puts(-55.0, 60.0, 4, display_buffer,0);
		sprintf(display_buffer, "2 %.2f", (double)all.fEng2FF_PPH); 
		Puts(-25.0, 60.0, 4, display_buffer,0);
		sprintf(display_buffer, "3 %.2f", (double)all.fEng3FF_PPH); 
		Puts(5.0, 60.0, 4, display_buffer,0);
		sprintf(display_buffer, "4 %.2f", (double)all.fEng4FF_PPH); 
		Puts(35.0, 60.0, 4, display_buffer,0);
		}
	}

void put_A320_N2(double x, double y, double value)
	{
	if (value > 113.0)
		setColor(1,0,0);
	if (value >= 100.0)
  		sprintf(display_buffer, "%-3.0f", value);
	else
  		sprintf(display_buffer, "%-2.0f", floor(value));

  	Puts(x-6, y, 7.0, display_buffer,0);
	if (value < 100.0)
		{
  		sprintf(display_buffer, "%-.1f", value - floor(value));
		Puts(x+3, y, 5, &display_buffer[1],0);
		}
	}

void draw_A320_ecam1()
	{	
	double totfuel;
	int init;
	int power;

	MinOilPSI = 25.0;

 	drawAirbusN1(-30.0, 55.0, 16.0, 115.5, ((double)all.wEng1N1 * 100.0)/16384.0, 0, 1);
  	drawAirbusEGT(-30, 18, 16, 860.0, ((double)all.wEng1EGT * 860.0)/16384.0, 500);
 	drawAirbusN1(30.0, 55.0, 16.0, 115.5, ((double)all.wEng2N1 * 100.0)/16384.0, 0, 2);
  	drawAirbusEGT(30, 18, 16, 860.0, ((double)all.wEng2EGT * 860.0)/16384.0, 500);

	setColor(1,1,1);
  	CPuts(0, 55, 5, "N1");
  	CPuts(0, 20, 5, "EGT");
  	CPuts(0, 5, 5, "N2");
   	CPuts(0, -5, 5, "FF");
	glLineWidth(1.0);
	glBegin(GL_LINES);
		glVertex2f(-6, 5);
		glVertex2f(-15, 1);
		glVertex2f(-6, 7);
		glVertex2f(-15, 3);
		glVertex2f(6, 5);
		glVertex2f(15, 1);
		glVertex2f(6, 7);
		glVertex2f(15, 3);

		glVertex2f(-6, -3);
		glVertex2f(-15, -7);
		glVertex2f(-6, -5);
		glVertex2f(-15, -9);
		glVertex2f(6, -3);
		glVertex2f(15, -7);
		glVertex2f(6, -5);
		glVertex2f(15, -9);
	glEnd();

  	setColor(0.0f, 0.8f, 1.0f);
	sprintf(display_buffer, "%cC", DEGREE);
 	CPuts(0, 50, 5, "%");
  	CPuts(0, 15, 5, display_buffer);
//  	Puts(-3, 17, 2, "o",0);
  	CPuts(0, 0, 5, "%");
  	CPuts(0, -10, 4, "KG/H");
	setColor(0,1,0);
 	put_A320_N2(-30, 1, ((double)all.wEng1N2 * 100.0)/16384.0);
 	put_A320_N2(30, 1, ((double)all.wEng2N2 * 100.0)/16384.0);
	setColor(0,1,0);
  	sprintf(display_buffer, "%ld", (long)(all.fEng1FF_PPH*0.454));
	Puts(-37, -9, 5, display_buffer,0);
	sprintf(display_buffer, "%ld", (long)(all.fEng1FF_PPH*0.454));
	Puts( 23, -9, 5, display_buffer,0);
	totfuel = ((((double)all.dwFuelCenterPct / 83886.0 ) / 100.0 ) * (double)all.dwFuelCenterCap );
	totfuel += ((((double)all.dwFuelLeftPct / 83886.0 ) / 100.0 ) * (double)all.dwFuelLeftCap );
	totfuel += ((((double)all.dwFuelLeftAuxPct / 83886.0 ) / 100.0 ) * (double)all.dwFuelLeftAuxCap );
	totfuel += ((((double)all.dwFuelRightPct / 83886.0 ) / 100.0 ) * (double)all.dwFuelRightCap );
    totfuel += ((((double)all.dwFuelRightAuxPct / 83886.0 ) / 100.0 ) * (double)all.dwFuelRightAuxCap );
	setColor(1.0,1.0,1.0);
	Puts(-70, -25, 6.0, "FOB:",0);
	sprintf(display_buffer, "%-.0f0", ((totfuel*6.78)*0.454)/10.0);

	setColor(0,1,0);
	Puts(-50, -25, 6.0, display_buffer,0);
	
	power = TOGA;
	if (tpercent < 98.3)
		power = MCT;
	if (tpercent < 90.8)
		power = CLB;
	setColor(0.0f, 0.8f, 1.0f);
	switch(power)
		{
		case TOGA:
			Puts(60, 70, 6, "TOGA",0);
			setColor(0,1,0);
			Puts(60, 65, 6.0, "103",0);
			Puts(70, 65, 5.0, ".5",0);
			break;
		case MCT:
			Puts(60, 70, 6, "MCT",0);
			setColor(0,1,0);
			Puts(60, 65, 6.0, "98",0);
			Puts(67, 65, 5.0, ".3",0);
			break;
		case CLB:
			Puts(60, 70, 6, "CLB",0);
			setColor(0,1,0);
			Puts(60, 65, 6.0, "90",0);
			Puts(67, 65, 5.0, ".8",0);
			break;
		}
	Puts(-25, -25, 5.0, "KG",0);
	setColor(1.0,1.0,1.0);
	glLineWidth(3.0);
	glBegin(GL_LINES);
		glVertex2f(-72, -30);
		glVertex2f( 72, -30);
		glVertex2f(22, -30);
		glVertex2f(22, -80);
	glEnd();
			setColor(1.0, 1.0, 0.0);
			for (init = 0; init < emc; init++)
			{
				switch(eicas_msg[init].type)
				{
					case MIDLEVEL:
					case LOWLEVEL:
					case MEMO:
						setColor(1,1,1);
						break;
					case CAUTION:
						setColor(255.0/255.0, 160.0/255.0, 0.0);
						break;
					case WARNING:
						setColor(1,0,0);
						break;
					case ADVISORY:
						setColor(255.0/255.0, 160.0/255.0, 0.0);
						break;
				}
				Puts(-72,  -35.0-(double)(init*5), 4.0, eicas_msg[init].text,0);
			}
	}

void draw_B747_eicas1()
	{	
	double totfuel;
	int init;
  	drawbarenginegauge(-75, 60, 1.4, ((double)all.wEng1EPR*1.60)/16384.0, 35, 2);
  	drawbarenginegauge(-50, 60, 1.4, ((double)all.wEng2EPR*1.60)/16384.0, 35, 2);
  	drawbarenginegauge(-25, 60, 1.4, ((double)all.wEng3EPR*1.60)/16384.0, 35, 2);
  	drawbarenginegauge(  0, 60, 1.4, ((double)all.wEng4EPR*1.60)/16384.0, 35, 2);
  	drawbarenginegauge(-75, 12, 100.0, ((double)all.wEng1N1 * 100.0)/16384.0, 30, 1);
  	drawbarenginegauge(-50, 12, 100.0, ((double)all.wEng2N1 * 100.0)/16384.0, 30, 1);
  	drawbarenginegauge(-25, 12, 100.0, ((double)all.wEng3N1 * 100.0)/16384.0, 30, 1);
  	drawbarenginegauge(  0, 12, 100.0, ((double)all.wEng4N1 * 100.0)/16384.0, 30, 1);
  	drawbarenginegauge(-75, -30, 750.0, ((double)all.wEng1EGT * 860.0)/16384.0, 30, 0);
  	drawbarenginegauge(-50, -30, 750.0, ((double)all.wEng2EGT * 860.0)/16384.0, 30, 0);
  	drawbarenginegauge(-25, -30, 750.0, ((double)all.wEng3EGT * 860.0)/16384.0, 30, 0);
  	drawbarenginegauge(  0, -30, 750.0, ((double)all.wEng4EGT * 860.0)/16384.0, 30, 0);

	setColor(0.0f, 1.0f, 1.0f);
  	Puts(-70, 75, 5.0, "TAT",0);
	setColor(1.0f, 1.0f, 1.0f);
	sprintf(display_buffer, "%.0f%cC", (double)all.wTAT/256.0, DEGREE);
	Puts(-54, 75, 4.0, display_buffer,0);

  	setColor(0.0f, 0.8f, 1.0f);
  	Puts(-41, 40, 5, "EPR",0);
	Puts(-39, -3, 5, "N1",0);
	Puts(-41, -45, 5, "EGT",0);
	Puts(10, -70, 4, "TOTAL FUEL",0);
	Puts(25, -75, 4, "TEMP",0);
	setColor(1.0,1.0,1.0);
	Puts(60, -70, 4, "LBS X",0);
	Puts(60, -73, 4, "1000",0);
	totfuel = ((((double) all.dwFuelCenterPct / 83886.0 ) / 100.0 ) *   (double)all.dwFuelCenterCap );
	totfuel += ((((double)all.dwFuelLeftPct / 83886.0 ) / 100.0 ) *     (double)all.dwFuelLeftCap );
	totfuel += ((((double)all.dwFuelLeftAuxPct / 83886.0 ) / 100.0 ) *  (double)all.dwFuelLeftAuxCap );
	totfuel += ((((double)all.dwFuelRightPct / 83886.0 ) / 100.0 ) *    (double)all.dwFuelRightCap );
	totfuel += ((((double)all.dwFuelRightAuxPct / 83886.0 ) / 100.0 ) * (double)all.dwFuelRightAuxCap );
	sprintf(display_buffer, "%.1f", (totfuel*6.78)/1000);
	Puts(42, -70, 5.0, display_buffer,0);
	sprintf(display_buffer, "%.0f%cC", (double)all.wOutsideAirTemp/256.0, DEGREE);
	Puts(42, -75, 4.0, display_buffer,0);
	drawflaps(40, -30, (int)(((double)all.dwFlapsLeft/16383.0)*100.0), (int)(((double)all.dwFlapsCommanded/16383.0)*100.0));
	check_gear(42, -17);
			setColor(1.0, 1.0, 0.0);
			for (init = 0; init < emc; init++)
			{
				switch(eicas_msg[init].type)
				{
					case MIDLEVEL:
					case LOWLEVEL:
					case MEMO:
						setColor(1,1,1);
						break;
					case CAUTION:
						setColor(255.0/255.0, 160.0/255.0, 0.0);
						break;
					case WARNING:
						setColor(1,0,0);
						break;
					case ADVISORY:
						setColor(255.0/255.0, 160.0/255.0, 0.0);
						break;
				}
				Puts(20,  70.0-(double)(init*5), 4.0, eicas_msg[init].text,0);
			}
	}

void display_value(double x, double y, double value, int boxit, int decimal)
	{
	if (!decimal)
		{
		if (!boxit)
			sprintf(display_buffer, "%.0f", value);
		else
			sprintf(display_buffer, "%05.0f", value);
		}
	else
		sprintf(display_buffer, "%.1f", value);
	
	Puts((x+1)+(5 - strlen(display_buffer)), y+2, 5, display_buffer,0);
	if (boxit)
 		drawbox( x + 2, y, x+14, y+7, 0, 0);
	}	

void draw_oil_gauge(double x, double y, double max, double value1, double value2)
	{
	double bv1, bv2;       // Bars can't surpass maximum.  
	if (value1 > max)
		bv1 = max;
	else if (value1 < 0.0)
		bv1 = 0.0;
	else
		bv1 = value1;
	if (value2 > max)
		bv2 = max;
	else if (value2 < 0.0)
		bv2 = 0.0;
	else
		bv2 = value2;
	glLineWidth(2.0);
	glBegin(GL_LINES);
		glVertex2f(x, y-10.0);
		glVertex2f(x, y+10.0);
	glEnd();
	glBegin(GL_LINE_STRIP);
		glVertex2f(x, (y-10.0)+(((bv1*100.0)/max)/5.0));
		glVertex2f(x-3, (y-9.0)+(((bv1*100.0)/max)/5.0));
		glVertex2f(x-3, (y-11.0)+(((bv1*100.0)/max)/5.0));
		glVertex2f(x, (y-10.0)+(((bv1*100.0)/max)/5.0));
	glEnd();
	glBegin(GL_LINE_STRIP);
		glVertex2f(x, (y-10.0)+(((bv2*100.0)/max)/5.0));
		glVertex2f(x+3, (y-9.0)+(((bv2*100.0)/max)/5.0));
		glVertex2f(x+3, (y-11.0)+(((bv2*100.0)/max)/5.0));
		glVertex2f(x, (y-10.0)+(((bv2*100.0)/max)/5.0));
	glEnd();
	sprintf(display_buffer, "%.0f", value1);
	Puts(x-12, y, 4, display_buffer,0);
	sprintf(display_buffer, "%.0f", value2);
	Puts(x+5, y, 4, display_buffer,0);
	glLineWidth(1.0);
	}

void draw_vibration(double x, double y, double max, double value1, double value2)
	{
	double bv1, bv2;       // Bars can't surpass maximum.  
	if (value1 > max)
		bv1 = max;
	else if (value1 < 0.0)
		bv1 = 0.0;
	else
		bv1 = value1;
	if (value2 > max)
		bv2 = max;
	else if (value2 < 0.0)
		bv2 = 0.0;
	else
		bv2 = value2;
	glLineWidth(2.0);
	glBegin(GL_LINES);
		glVertex2f(x+4, y-10.0);
		glVertex2f(x+4, y+10.0);
		glVertex2f(x-4, y-10.0);
		glVertex2f(x-4, y+10.0);
	glEnd();
	glBegin(GL_LINE_STRIP);
		glVertex2f(x-4, (y-10.0)+(((bv1*100.0)/max)/5.0));
		glVertex2f(x-1, (y-9.0)+(((bv1*100.0)/max)/5.0));
		glVertex2f(x-1, (y-11.0)+(((bv1*100.0)/max)/5.0));
		glVertex2f(x-4, (y-10.0)+(((bv1*100.0)/max)/5.0));
	glEnd();
	glBegin(GL_LINE_STRIP);
		glVertex2f(x+4, (y-10.0)+(((bv2*100.0)/max)/5.0));
		glVertex2f(x+1, (y-9.0)+(((bv2*100.0)/max)/5.0));
		glVertex2f(x+1, (y-11.0)+(((bv2*100.0)/max)/5.0));
		glVertex2f(x+4, (y-10.0)+(((bv2*100.0)/max)/5.0));
	glEnd();
	sprintf(display_buffer, "%.1f", value1);
	Puts(x-14, y, 4, display_buffer,0);
	sprintf(display_buffer, "%.1f", value2);
	Puts(x+5, y, 4, display_buffer,0);
	glLineWidth(1.0);
	}

void draw_B747_eicas2()
	{	
  	drawbarenginegauge(-75, 70, 100, ((double)all.wEng1N2 * 100.0)/16384.0, 35, 0);
  	drawbarenginegauge(-50, 70, 100, ((double)all.wEng2N2 * 100.0)/16384.0, 35, 0);
  	drawbarenginegauge(-25, 70, 100, ((double)all.wEng3N2 * 100.0)/16384.0, 35, 0);
  	drawbarenginegauge(  0, 70, 100, ((double)all.wEng4N2 * 100.0)/16384.0, 35, 0);
	setColor(0.0f, 1.0f, 1.0f);
  	Puts(-39, 50, 4, "N2",0);
  	Puts(-39, 22, 4, "FF",0);
  	Puts(-43, -0, 4, "OIL P",0);
  	Puts(-43, -25, 4, "OIL T",0);
  	Puts(-43, -45, 4, "OIL Q",0);
  	Puts(-39, -70, 4, "VIB",0);
  	setColor(1.0,1.0,1.0);
	
	display_value(-81, 20, (all.fEng1FF_PPH/1000.0), 1, 1);
	display_value(-56, 20, (all.fEng2FF_PPH/1000.0), 1, 1);
	display_value(-31, 20, (all.fEng3FF_PPH/1000.0), 1, 1);
	display_value( -6, 20, (all.fEng4FF_PPH/1000.0), 1, 1);
	
	draw_oil_gauge(-62.5, -0, 220.0, ((double)all.wEng1OilPressure*220.0)/65535.0, ((double)all.wEng2OilPressure*220.0)/65535.0);
	draw_oil_gauge(-12.5, -0, 220.0, ((double)all.wEng3OilPressure*220.0)/65535.0, ((double)all.wEng4OilPressure*220.0)/65535.0);

	draw_oil_gauge(-62.5, -25, 140.0, ((double)all.wEng1OilTemp*140.0)/16384.0, ((double)all.wEng2OilTemp*140.0)/16384.0);
	draw_oil_gauge(-12.5, -25, 140.0, ((double)all.wEng3OilTemp*140.0)/16384.0, ((double)all.wEng4OilTemp*140.0)/16384.0);
	
	display_value(-81, -47, ((double)all.dwEng1OilQty * 100.0) / 16384.0, 0, 0);
	display_value(-56, -47, ((double)all.dwEng2OilQty * 100.0) / 16384.0, 0, 0);
	display_value(-31, -47, ((double)all.dwEng3OilQty * 100.0) / 16384.0, 0, 0);
	display_value( -6, -47, ((double)all.dwEng4OilQty * 100.0) / 16384.0, 0, 0);
	
	draw_vibration(-62.5, -60, 5.0, ((double)all.dwEng1Vibration*5.0)/16384.0, ((double)all.dwEng2Vibration*5.0)/16384.0);
	draw_vibration(-12.5, -60, 5.0, ((double)all.dwEng3Vibration*5.0)/16384.0, ((double)all.dwEng4Vibration*5.0)/16384.0);
	}

int drawflaps(int x, int y, int value, int commanded)
        {
        char buffer[100];
		switch(commanded)
			{
			case 0:
				strcpy(buffer, "UP");
				break;
			case 16:
				strcpy(buffer, "1");
				break;
			case 33:
				strcpy(buffer, "5");
				break;
			case 49:
				strcpy(buffer, "15");
				break;
			case 66:
				strcpy(buffer, "20");
				break;
			case 83:
				strcpy(buffer, "25");
				break;
			case 100:
				strcpy(buffer, "30");
				break;
			default:
				strcpy(buffer, " ");
			}
		
        if (value < 0)
        	value = 0;
        if (value > 100)
        	value = 100;

	if (value == 0)
		return(1);
	glLineWidth(1.0);
	setColor(1.0,1.0,1.0);
        drawbox(x-.1, y-.1, x+4.1, y-30, 0, 0);
	setColor(0.7,0.7,0.7);
        glRecti(x, y, x+4, y-value*.3);
	if ((value < (commanded-1)) || (value > (commanded+1)))
        	 setColor(1.0f, 0.0f, 1.0f);
	else
        	 setColor(0.0f, 1.0f, 0.0f);
	Puts(x+5.5, (y-commanded*.3)+1, 5, buffer,0);
	glLineWidth(2.5);

	glBegin(GL_LINES);
		glVertex3f(x-1, y-commanded*.3, 0);
		glVertex3f(x+5.3, y-commanded*.3, 0);
	glEnd();
  	setColor(0.0f, 0.8f, 1.0f);
        CPuts(x-4, y-5, 5, "F");
        CPuts(x-3.7, y-10, 5, "L");
        CPuts(x-4, y-15, 5, "A");
        CPuts(x-4, y-20, 5, "P");
        CPuts(x-4, y-25, 5, "S");
        return(1);
        }

int drawbarenginegauge(double x, double y, double maxvalue, double value, double gsize, char precision)
    {
    char buffer[100];
	double gv = value;			
    if (value < 0)
        gv = 0;
    if (value > maxvalue)
        gv = maxvalue;

	setColor(1.0,1.0,1.0);
    drawbox(x-.1, y-.1, x+4.1, y-gsize, 0, 0);
	setColor(0.7,0.7,0.7);
    glRectf(x, y-gsize, x+4, y-(gsize-((gv/maxvalue)*gsize)));
	setColor(1.0,1.0,1.0);
	drawbox(x-6, y+1, x+10, y+7, 0, 0);

	if (value > maxvalue)
		setColor(1.0f, 0.0f, 0.0f);

	if (precision == 0)
  		sprintf(buffer, "%.0f", value);
	else if ((value >= 100.0) || (precision == 1))
  		sprintf(buffer, "%-3.1f", value);
	else
  		sprintf(buffer, "%-2.2f", value);

  	Puts(x-5, y+2.5, 5.0, buffer,0);

        return(1);
        }

#define EM 210.0
void drawenginegauge(double x, double y, double radius, double maxvalue, double value, int yellow)
	{
	double x2, y2;
	double vangle;
	char st[10];
	static double x3, y3;
	GLUquadricObj *quadObj2;
	
	glPushMatrix();

	if (value < 0)
			value = 0;
	if (value > maxvalue)
		vangle = EM;
	else
		vangle = (value/maxvalue) * EM;
	setColor(1.0f, 1.0f, 1.0f);

	
	glTranslatef(x, y, 0.0);
	quadObj2 = gluNewQuadric();
	if (value > maxvalue)
		setColor(1.0f,0.0f,0.0f);
	else
		setColor(0.5f,0.5f,0.5f);

		gluQuadricDrawStyle(quadObj2, GLU_FILL);	
		gluPartialDisk(quadObj2, 0.0f, (double)radius, 100.0f, 1.0f, 90.0f, vangle);	
	gluQuadricDrawStyle(quadObj2, GLU_LINE);	
	setColor(1.0f, 1.0f, 1.0f);
	glLineWidth(2.0);	
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 90.0f, EM);
	glLineWidth(1.0);	

	gluDeleteQuadric(quadObj2);	

	glPopMatrix();
	
	setColor(1.0,1.0,1.0);
	x3 = x + cos((-1.0*vangle) * 3.14 / 180.0) * radius;
	y3 = y + sin((-1.0*vangle) * 3.14 / 180.0) * radius;
	drawbox(x, y+1, x+radius+2.0, y+7.8, 0, 0);

	if (value > maxvalue)
		setColor(1.0f, 0.0f, 0.0f);
	if ((yellow) && (value >= yellow))
		setColor(1.0f, 1.0f, 0.0f);

	if (value >= 100.0)
  		sprintf(st, "%-3.1f", value);
	else
  		sprintf(st, "%-2.2f", value);

  	Puts(x+2, y+2, 6.0, st,0);
glLineWidth(2.0);
	glBegin(GL_LINES);
	glVertex2f(x, y);
	glVertex2f(x3, y3);
	glEnd();
	glLineWidth(2.0);

	setColor(1.0f, 0.0f, 0.0f);
	x2 = x + cos((EM*-1.0) * 3.14 / 180.0) * radius;
	y2 = y + sin((EM*-1.0) * 3.14 / 180.0) * radius;
	
	x3 = x + cos((EM*-1.0) * 3.14 / 180.0) * (radius+3.0);
	y3 = y + sin((EM*-1.0) * 3.14 / 180.0) * (radius+3.0);
	glBegin(GL_LINES);
	glVertex2f(x2, y2);
	glVertex2f(x3, y3);
	glEnd();
	glLineWidth(1.0);
	}

void drawnggauge(double x, double y, double radius, double maxvalue, double value, int yellow)
	{
	double x2, y2;
	double vangle;
	char st[10];
	static double x3, y3;
	GLUquadricObj *quadObj2;
	double init;
	int tick;
	
	glPushMatrix();

	if (value < 0)
			value = 0;
	if (value > maxvalue)
		vangle = EM;
	else
		vangle = (value/maxvalue) * EM;
	setColor(1.0f, 1.0f, 1.0f);

	
	glTranslatef(x, y, 0.0);
	quadObj2 = gluNewQuadric();
	if (value > maxvalue)
		setColor(1.0f,0.0f,0.0f);
	else
		setColor(0.5f,0.5f,0.5f);

	gluQuadricDrawStyle(quadObj2, GLU_FILL);	
	gluPartialDisk(quadObj2, 0.0f, (double)radius, 100.0f, 1.0f, 90.0f, vangle);	
	gluQuadricDrawStyle(quadObj2, GLU_LINE);	
	setColor(1.0f, 1.0f, 1.0f);
	glLineWidth(2.0);	
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 90.0f, EM);
	glLineWidth(1.0);	

	gluDeleteQuadric(quadObj2);	

	glPopMatrix();
	
	setColor(1.0,1.0,1.0);
	for (init = 0.0, tick = 0; init <= EM; tick++, init += (EM/10.0))
		{
		x2 = x + cos((-1.0*init) * 3.14 / 180.0) * radius;
		y2 = y + sin((-1.0*init) * 3.14 / 180.0) * radius;
		x3 = x + cos((-1.0*init) * 3.14 / 180.0) * (radius-3.0);
		y3 = y + sin((-1.0*init) * 3.14 / 180.0) * (radius-3.0);
		glBegin(GL_LINES);
			glVertex2f(x2, y2);
			glVertex2f(x3, y3);
		glEnd();
		x3 = x + cos((-1.0*init) * 3.14 / 180.0) * (radius-4.0);
		y3 = y + sin((-1.0*init) * 3.14 / 180.0) * (radius-4.0);
		switch(tick)
			{
			case 2:
				Puts(x3, y3, 4, "2",0);
				break;
			case 4:
				Puts(x3, y3, 4, "4",0);
				break;
			case 6:
				Puts(x3, y3, 4, "6",0);
				break;
			case 8:
				Puts(x3, y3, 4, "8",0);
				break;
			case 10:
				Puts(x3, y3, 4, "10",0);
				break;
			}
		}
	x3 = x + cos((-1.0*vangle) * 3.14 / 180.0) * radius;
	y3 = y + sin((-1.0*vangle) * 3.14 / 180.0) * radius;
	drawbox(x, y+1, x+radius+2.0, y+7.0, 0, 0);

	if (value > maxvalue)
		setColor(1.0f, 0.0f, 0.0f);
	if ((yellow) && (value >= yellow))
		setColor(1.0f, 1.0f, 0.0f);

	if (value >= 100.0)
  		sprintf(st, "%-3.1f", value);
	else
  		sprintf(st, "%-2.2f", value);

  	Puts(x+2, y+2, 5.0, st,0);
	glBegin(GL_LINES);
	glVertex2f(x, y);
	glVertex2f(x3, y3);
	glEnd();

	setColor(1.0f, 0.0f, 0.0f);
	x2 = x + cos((EM*-1.0) * 3.14 / 180.0) * radius;
	y2 = y + sin((EM*-1.0) * 3.14 / 180.0) * radius;
	
	x3 = x + cos((EM*-1.0) * 3.14 / 180.0) * (radius+3.0);
	y3 = y + sin((EM*-1.0) * 3.14 / 180.0) * (radius+3.0);
	glBegin(GL_LINES);
	glVertex2f(x2, y2);
	glVertex2f(x3, y3);
	glEnd();

	glLineWidth(1.0);
	}
	
void drawfuelflowgauge(double x, double y, double radius, double maxvalue, double value, int yellow)
	{
	double x2, y2;
	double vangle;
	char st[10];
	static double x3, y3;
	GLUquadricObj *quadObj2;
	double init;
	int tick;
	
	glPushMatrix();

	if (value < 0)
			value = 0;
	if (value > maxvalue)
		vangle = EM;
	else
		vangle = (value/maxvalue) * EM;
	setColor(1.0f, 1.0f, 1.0f);
	
	glTranslatef(x, y, 0.0);
	quadObj2 = gluNewQuadric();
	if (value > maxvalue)
		setColor(1.0f,0.0f,0.0f);
	else
		setColor(0.5f,0.5f,0.5f);

		gluQuadricDrawStyle(quadObj2, GLU_FILL);	
		gluPartialDisk(quadObj2, 0.0f, (double)radius, 100.0f, 1.0f, 90.0f, vangle);	
	gluQuadricDrawStyle(quadObj2, GLU_LINE);	
	glLineWidth(1.0);
	setColor(1.0f, 1.0f, 1.0f);
	glLineWidth(2.0);	
	gluPartialDisk(quadObj2, radius, radius, 100.0f, 1.0f, 90.0f, EM);
	glLineWidth(1.0);	

	gluDeleteQuadric(quadObj2);	

	glPopMatrix();
	
	setColor(1.0,1.0,1.0);
	for (init = 0.0, tick = 0; init <= EM; tick++, init += (EM/6.0))
		{
		x2 = x + cos((-1.0*init) * 3.14 / 180.0) * radius;
		y2 = y + sin((-1.0*init) * 3.14 / 180.0) * radius;
		x3 = x + cos((-1.0*init) * 3.14 / 180.0) * (radius-3.0);
		y3 = y + sin((-1.0*init) * 3.14 / 180.0) * (radius-3.0);
		glBegin(GL_LINES);
			glVertex2f(x2, y2);
			glVertex2f(x3, y3);
		glEnd();
		x3 = x + cos((-1.0*init) * 3.14 / 180.0) * (radius-4.0);
		y3 = y + sin((-1.0*init) * 3.14 / 180.0) * (radius-4.0);
		switch(tick)
			{
			case 2:
				Puts(x3, y3, 4, "2",0);
				break;
			case 4:
				Puts(x3, y3, 4, "4",0);
				break;
			case 6:
				Puts(x3, y3, 4, "6",0);
				break;
			}
		}
	x3 = x + cos((-1.0*vangle) * 3.14 / 180.0) * radius;
	y3 = y + sin((-1.0*vangle) * 3.14 / 180.0) * radius;
	drawbox(x, y+1, x+radius+2.0, y+7.0, 0, 0);

	if (value > maxvalue)
		setColor(1.0f, 0.0f, 0.0f);
	if ((yellow) && (value >= yellow))
		setColor(1.0f, 1.0f, 0.0f);

	value *= 0.454;
	value/=1000.0;
	if (value >= 100.0)
  		sprintf(st, "%-3.1f", value);
	else
  		sprintf(st, "%-2.2f", value);

  	Puts(x+2, y+2, 5.0, st,0);
glLineWidth(2.0);
	glBegin(GL_LINES);
	glVertex2f(x, y);
	glVertex2f(x3, y3);
	glEnd();

	glLineWidth(1.0);
	}
		
void drawAirbusN1(double x, double y, double radius, double maxvalue, double value, int yellow, int engine)
	{
	double x2, y2;
	double vangle;
	char st[10];
	static double x3, y3;
	GLUquadricObj *quadObj2;
	SWORD throttle = 0;
	double tangle;;
	glPushMatrix();

	switch(engine)
		{
		case 1:
			throttle = all.wEng1Throttle;
			break;
		case 2:
			throttle = all.wEng2Throttle;
			break;
		case 3:
			throttle = all.wEng3Throttle;
			break;
		case 4:
			throttle = all.wEng4Throttle;
			break;
		}


	if (throttle < 0)
		{
		tangle = -1.0;
		}
	else
		{
		tangle = ((double)throttle / 16383.0) * 225.0;
		tpercent = ((double)throttle / 16383.0) * 115.5;
		}

	if (value < 0)
			value = 0;
	if (value > maxvalue)
		vangle = -135.0;
	else
		vangle = (value/maxvalue) * 225.0;
	setColor(1.0f, 1.0f, 1.0f);

	vangle += 135.0;
	tangle += 135.0;
	
	glTranslatef(x, y, 0.0);
	quadObj2 = gluNewQuadric();
	gluQuadricDrawStyle(quadObj2, GLU_LINE);	
	if (value > maxvalue)
		setColor(1.0f,0.0f,0.0f);
	else
		setColor(0.5f,0.5f,0.5f);

	setColor(1.0f, 1.0f, 1.0f);	
	glLineWidth(1.0);	
	gluPartialDisk(quadObj2, (double)radius-3.0, (double)radius, 6.0f, 1.0f, 225.0f, 225.0);
	glLineWidth(3.0);
	setColor(0,0,0);
	gluPartialDisk(quadObj2, (double)radius-3.0, (double)radius-3.0, 6.0f, 1.0f, 225.0f, 225.0);
	gluPartialDisk(quadObj2, (double)radius, (double)radius, 6.0f, 1.0f, 225.0f, 225.0);
	glLineWidth(2.0);
	setColor(1.0f, 0.0f, 0.0f);
	gluPartialDisk(quadObj2, (double)radius-2.0, (double)radius, 100.0f, 1.0f, 70.0f, 20.0);
	setColor(1.0f, 1.0f, 1.0f);	
	gluPartialDisk(quadObj2, (double)radius, (double)radius, 100.0f, 1.0f, 225.0f, 225.0);

	gluDeleteQuadric(quadObj2);	

	glPopMatrix();

/*  THIS IS THE YELLOW LINE */	
	setColor(1.0f, 1.0f, 0.0f);
	glLineWidth(3.0);
	x2 = x + cos(-305.0 * RADIANS) * (double)(radius+2.0);
	y2 = y + sin(-305.0 * RADIANS) * (double)(radius+2.0);
	
	x3 = x + cos(-305.0 * RADIANS) * (double)(radius-2.0);
	y3 = y + sin(-305.0 * RADIANS) * (double)(radius-2.0);
	glBegin(GL_LINES);
	glVertex2i(x2, y2);
	glVertex2i(x3, y3);
	glEnd();
	glLineWidth(1.0);

	setColor(1.0,1.0,1.0);
	x3 = x + cos((-1.0*vangle) * RADIANS) * (double)(radius+3.0);
	y3 = y + sin((-1.0*vangle) * RADIANS) * (double)(radius+3.0);
	drawbox(x-2, y-13, x+radius+2, y-4, 0, 0);

	setColor(0,1,0);
	if (value >= 100.0)
  		sprintf(st, "%-3.0f", value);
	else
  		sprintf(st, "%-2.0f", floor(value));

  	Puts(x, y-11, 7.0, st,0);
	if (value < 100.0)
		{
  		sprintf(st, "%-.1f", value - floor(value));
		Puts(x+9, y-11, 5, &st[1],0);
		}

	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x3, y3);
	glEnd();
	glLineWidth(1.0);
	x3 = x + cos((-1.0*tangle) * RADIANS) * (double)(radius+3.0);
	y3 = y + sin((-1.0*tangle) * RADIANS) * (double)(radius+3.0);
	setColor(0,0.8,0.8);
	Puts(x3-1.75, y3, 5.0, "o",0);
	setColor(1,1,1);
	if (throttle < 0.0)
		{
		glLineWidth(0.5);
		setColor(0,0,0);
		drawbox(x-9, y-1, x+9, y+7, 0, 1);
		setColor(244.0/255.0, 159.0/255.0, 12.0/255.0);
		if (throttle < -2000.0)
			setColor(0.0,1.0,0.0);
		Puts(x-8, y, 7.0, "REV",0);
		setColor(1,1,1);
		drawbox(x-9, y-1, x+9, y+7, 0, 0);
		}
	
	}
		
void drawAirbusEGT(int x, int y, int radius, double maxvalue, double value, int yellow)
	{
	int x2, y2;
	double vangle;
	char st[10];
	static int x3, y3;
	GLUquadricObj *quadObj2;
	
	glPushMatrix();

	if (value < 0)
			value = 0;

	if (value > maxvalue)
		vangle = 90.0;
	else
		vangle = ((value/maxvalue) * 180.0);
	setColor(1.0f, 1.0f, 1.0f);
	
	vangle -= 180.0;
	if (vangle < 0.0)
		vangle = 360.0 + vangle;

	glTranslatef(x, y, 0.0);
	quadObj2 = gluNewQuadric();
	gluQuadricDrawStyle(quadObj2, GLU_LINE);	
	if (value > maxvalue)
		setColor(1.0f,0.0f,0.0f);
	else
		setColor(0.5f,0.5f,0.5f);

	setColor(1.0f, 0.0f, 0.0f);
	gluPartialDisk(quadObj2, (double)radius-2.0, (double)radius, 100.0f, 1.0f, 45.0f, 45.0);
	setColor(1.0f, 1.0f, 1.0f);
	glLineWidth(2.0);

	gluPartialDisk(quadObj2, (double)radius, (double)radius, 100.0f, 1.0f, 270.0f, 180.0);
	glLineWidth(1.0);	

	gluDeleteQuadric(quadObj2);	

	glPopMatrix();

	glBegin(GL_LINES);
		glVertex2f(x-radius, y);
		glVertex2f(x-(radius-3), y);
		glVertex2f(x+radius, y);
		glVertex2f(x+(radius-3), y);
		glVertex2f(x, y+radius);
		glVertex2f(x, y+(radius-3));
	glEnd();
/*  THIS IS THE YELLOW LINE	*/
	glLineWidth(3.0);
	setColor(1.0f, 1.0f, 0.0f);
	x2 = x + cos(-300.0 * RADIANS) * (radius+2.0);
	y2 = y + sin(-300.0 * RADIANS) * (radius+2.0);
	
	x3 = x + cos(-300.0 * RADIANS) * (radius-2.0);
	y3 = y + sin(-300.0 * RADIANS) * (radius-2.0);
	glBegin(GL_LINES);
	glVertex2i(x2, y2);
	glVertex2i(x3, y3);
	glEnd();


	setColor(0.0,1.0,0.0);
	x3 = x + cos((-1.0*vangle) * RADIANS) * (radius+5.0);
	y3 = y + sin((-1.0*vangle) * RADIANS) * (radius+5.0);

	glLineWidth(4.0);
	glBegin(GL_LINES);
		glVertex2i(x, y);
		glVertex2i(x3, y3);
	glEnd();
	setColor(0,0,0);
	glPushMatrix();
	glTranslatef(x, y, 0.0);
	quadObj2 = gluNewQuadric();
	gluQuadricDrawStyle(quadObj2, GLU_FILL);	
	gluPartialDisk(quadObj2, (double)0, (double)radius-5, 100.0f, 1.0f, 270.0f, 360.0);
	gluPartialDisk(quadObj2, (double)radius+3.0, (double)radius+6.0, 100.0f, 1.0f, 270.0f, 180.0);
	gluDeleteQuadric(quadObj2);	
	glPopMatrix();
setColor(0,1,0);
  	sprintf(st, "%-3.0f", value);
  	Puts(x-5, y-4, 6.0, st,0);
	setColor(1,1,1);
	glLineWidth(1.0);
	drawbox(x-10, y-5, x+10, y+2, 0, 0);

	}

#define EM 210.0
void draweprgauge(int x, int y, int radius, double maxvalue, double value, int yellow)
	{
	int x2, y2;
	double vangle;
	char st[10];
	static int x3, y3;
	GLUquadricObj *quadObj2;


	value -= 1.0;
	maxvalue -= 1.0;

	if (value < 0)
			value = 0;
	if (value > maxvalue)
		vangle = EM-45.0;
	else
		vangle = (value/maxvalue) * (EM-45.0);
	setColor(1.0f, 1.0f, 1.0f);

	
	glPushMatrix();
	glTranslatef(x, y, 0.0);
	if (value > maxvalue)
		setColor(1.0f,0.0f,0.0f);
	else
		setColor(0.5f,0.5f,0.5f);

	setColor(1.0f, 1.0f, 1.0f);
	glLineWidth(2.0);	
	quadObj2 = gluNewQuadric();
	gluQuadricDrawStyle(quadObj2, GLU_LINE);	
	gluPartialDisk(quadObj2, (double)radius, (double)radius, 100.0f, 1.0f, 135.0f, EM-45.0);
	glLineWidth(1.0);	
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x, y, 0.0);
	gluPartialDisk(quadObj2, (double)radius-3.0, (double)radius, 8.0f, 1.0f, 135.0f, EM-45.0);
	setColor(0,0,0);
	gluPartialDisk(quadObj2, (double)radius-3.0, (double)radius-2.9, 8.0f, 1.0f, 135.0f, EM-45.0);
	glPopMatrix();
	gluDeleteQuadric(quadObj2);	
	
	setColor(1,1,1);
	Puts(x+12, y-11, 3, "1.0",0);
	Puts(x-10, y-13, 3, "1.4",0);
	Puts(x-14, y+10, 3, "1.8",0);
	setColor(1.0f, 0.0f, 0.0f);
	x2 = x + cos((EM*-1.0) * MYPI / 180) * radius;
	y2 = y + sin((EM*-1.0) * MYPI / 180) * radius;
	
	x3 = x + cos((EM*-1.0) * MYPI / 180) * (radius-1);
	y3 = y + sin((EM*-1.0) * MYPI / 180) * (radius-1);
	glBegin(GL_LINES);
	glVertex2i(x2, y2);
	glVertex2i(x3, y3);
	glEnd();
	setColor(1.0,1.0,1.0);
	vangle += 45.0;
	x3 = x + cos((-1.0*vangle) * MYPI / 180) * radius;
	y3 = y + sin((-1.0*vangle) * MYPI / 180) * radius;
	drawbox(x, y+1, x+radius+2, y+7, 0, 0);

	if (value > maxvalue)
		setColor(1.0f, 0.0f, 0.0f);
	if ((yellow) && (value >= yellow))
		setColor(1.0f, 1.0f, 0.0f);

	if (value >= 100.0)
  		sprintf(st, "%-3.1f", value+1.0);
	else
  		sprintf(st, "%-2.2f", value+1.0);

  	Puts(x+2, y+2, 5.0, st,0);

	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x3, y3);
	glEnd();
	}
		
void check_gear(int x, int y)
	{
	char state = 0;

	if (all.dwNoseGearPosition == 0x3fff)
		{
		state++;
		}
	if (all.dwLeftGearPosition == 0x3fff)
		{
		state++;
		}
	if (all.dwRightGearPosition == 0x3fff)
		{
		state++;
		}
	if (state == 3)
		{
		setColor(0.0,1.0,0.0);
		CPuts(x, y+1.5, 5, "DOWN");
		}
	else if ( (all.dwNoseGearPosition == 0) && 
		      (all.dwLeftGearPosition == 0) && 
			  (all.dwRightGearPosition == 0) )
		{
		return;
		}
	else
		setColor(1.0,0.0,0.0);
	drawbox(x-8, y, x+8, y+7, 0, 0);
	setColor(0.0,1.0,1.0);
	CPuts(x, y-5, 5, "GEAR");
	}

/* The function called whenever a key is pressed. */
void keyPressed(unsigned char key, int x, int y)
	{
	FILE *geometry;
	RECT lprect;

	hwnd = GetActiveWindow();
	if (x || y)
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
		case 'q':
		case 'Q':
			display_stats ^= 1;
			break;
		case '9':
			dimmer -= 0.1;
			break;
		case '0':
			dimmer += 0.1;
			break;
		case 'a':
		case 'A':
			avionics ^= 1;
			break;
		case 't':
		case 'T':
			tilt+=90.0;
			if (tilt > 270.0)
				tilt = 0.0;
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
		case 'v':
		case 'V':
			variation++;
			if (variation > VARIATIONS)
				variation = 0;
			break;
		case 'd':
		case 'D':
			dimmer -= .01;
			break;
		case 'c':
		case 'C':
			calibrate ^= 1;
			break;
		case 'b':
		case 'B':
			dimmer += .01;
			break;
		case 'r':
		case 'R':
			emc = 0;
			APATset = 0;
			APMset = 0;
			break;
		case 's':
		case 'S':
			eicas^=1;
			break;
		}
	if (dimmer < 0.0) dimmer = 0.0;
	if (dimmer > 1.0) dimmer = 1.0;
    	if (key == 27)
    	{
			exit(0);
		}

	}

void getoutofdodge()
{
	FILE *geometry;
	RECT lprect;

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
		fprintf(geometry, "TYPE:%d:%d:\n", eicas,variation);
		fprintf(geometry, "WINDOW:%d:%d:%d:%d:%d:\n", wx, wy, width, height, windowframes);
		fprintf(geometry, "OPTIONS:%f:%d:%d:\n", tilt,mouseptr,topmost);
		fclose(geometry);
	}

	Data_Close();
	wglDeleteContext(hrc);

        /* shut down our window */
	glutDestroyWindow(window);

        /* exit the program...normal termination. */
	exit(0);
}

void InitGL(int Width, int Height)	       
	{
	if (Width || Height)
  	glClearColor(0.0f, 0.0f, cb, 0.0f);	
  	glClearDepth(1.0);	
  	glShadeModel(GL_FLAT);		
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
//	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	}

void reshape(int w, int h)
	{
	width = w; height = h;
	glViewport(0,0,(GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(	(WindowSize * -1.0) - padding, WindowSize + padding, 
				(WindowSize * -1.0) - padding, WindowSize + padding);
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();
	}

void DrawGLEicas()
	{
	static int time, oldtime;
	static int loops = 0;
	static double pers = 0.0;
	RECT lprect;

	hwnd = GetActiveWindow();

	/* Calculate frame rates */
    oldtime = timeGetTime() - time;
	loops++;
	if (oldtime >= 1000)
		{
		time = timeGetTime();
		frames = loops;
		loops = 0;
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

	if (notconnected)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		setColor(1.0, 0.0, 0.0);
		CPuts(0.0, 0.0, 5.0, "INOP");
		drawbox(-26.0, 5.0, 12.0, -1.0, 0.0, 0);
		glutSwapBuffers();
		if (make_socket(1,NULL) == -1)
			notconnected = 1;
		else
			notconnected = 0;
		return;
	}

	split_eicas();

	if (!all.MASTER_BATTERY)
		{
		glClear(GL_COLOR_BUFFER_BIT);
		glutSwapBuffers();
		return;
		}


	glPushMatrix();
	glRotatef(tilt, 0.0, 0.0, 1.0);
  	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	Sleep(10);

	if (!calibrate)
		{
		setColor(1.0,1.0,1.0);	
		if (!refresh)
			{
			if (!eicas)
				{
				switch(variation)
					{
					case B777:
						draw_B777_eicas1();
						break;
					case B747:
						draw_B747_eicas1();
						break;
					case B738:
						draw_B738_eicas1();
						break;
					case A320:
						draw_A320_ecam1();
						break;
					case CTRL:
						control_synoptic();
						break;
					default:
						break;
					}
				}
			else
				{
				switch(variation)
					{
					case B777:
						draw_B777_eicas2();
						break;
					case B747:
						draw_B747_eicas2();
						break;
					default:
						break;
					}
				}
			}
		}
	else
		calibrate_instrument();
	if (display_stats)
		{
		glColor3f(1.0, 1.0, 0.0);
		sprintf(display_buffer, "FPS:%d", frames);
        Puts(20, 82, 4.0, display_buffer,0);
		}
	glPopMatrix();
  	glutSwapBuffers();
	}

void idlefunc(void)
	{
	glutPostRedisplay();
	}

int main(int argc, char **argv) 
	{  
	char ipaddr[20];
	FILE *geometry;
	int init2;
	char *token;
	wx = wy = 100;
	width = height = 400;
	char buffer[1000];
	strcpy(revision, "0.1 (beta)");
	strcpy(config, "eicas.ini");
	for (init2 = 1; init2 < argc; init2++)
		{
		if (argv[init2][0] == '-')
			{
			switch(argv[init2][1])
				{
				case 'a':
				case 'A':
					if (strlen(argv[init2]) == 2)
						strcpy(airfile, argv[init2+1]);
					else							
						strcpy(airfile, &argv[init2][2]);
					break;
				case 'p':
				case 'P': 
					if (strlen(argv[init2]) < 3)
						{
						if (init2+1 <= argc)
							{
							padding = atof(argv[init2+1]);
							}
						}
					else
						padding = atof(&argv[init2][2]);
					break;
				case 'i':
				case 'I':
					if (strlen(argv[init2]) < 3)
						{
						if (init2+1 <= argc)
							{
							strcpy(config, argv[init2+1]);
							}
						}
					else
						strcpy(config, &argv[init2][2]);
					break;
				}
			}
		}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	geometry = fopen(config, "r");
	if (geometry != NULL)
		{
		while (fgets((char *)buffer, 100, geometry))
			{
			if (!strncmp((char *)buffer, "TYPE", 4))
			{
				token = strtok((char *)buffer, ":");
				token = strtok(NULL, ":");
				if (token != NULL) eicas = atoi(token);
				token = strtok(NULL, ":");
				if (token != NULL) variation = atoi(token);
			}

			if (!strncmp((char *)buffer, "WINDOW", 6))
				{
				token = strtok((char *)buffer, ":");
				token = strtok(NULL, ":");
				if (token != NULL) wx = atoi(token);
				token = strtok(NULL, ":");
				if (token != NULL) wy = atoi(token);
				token = strtok(NULL, ":");
				if (token != NULL) width = atoi(token);
				token = strtok(NULL, ":");
				if (token != NULL) height = atoi(token);
				token = strtok(NULL, ":");
				if (token != NULL) windowframes = atoi(token);
				}
			if (!strncmp((char *)buffer, "OPTIONS", 7))
				{
				token = strtok((char *)buffer, ":");
				token = strtok(NULL, ":");
				if (token != NULL) tilt = atof(token);
				token = strtok(NULL, ":");
				token = strtok(NULL, ":");
				if (token != NULL) mouseptr = atoi(token);
				token = strtok(NULL, ":");
				if (token != NULL) topmost = atoi(token);
				}
			}
		fclose(geometry);
		}
	glutInitWindowSize(width,height);
	glutInitWindowPosition(wx, wy);
	sprintf(display_buffer, "Ellie Systems EICAS %s %s", revision, subversion);
	window = glutCreateWindow(display_buffer);
  	InitGL(650, 650);
	glutDisplayFunc(DrawGLEicas);
	glutReshapeFunc(reshape);
	glutIdleFunc(idlefunc);
	glutKeyboardFunc(keyPressed);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	atexit(getoutofdodge);
	ipaddr[0] = '\0';

	sock = make_socket(PORT, ipaddr);
	if (sock == -1)
		notconnected = 1;
	emc = 0;
	load_font(NULL);

	firsttime = 1;
	glutMainLoop();
	return(1);
	}

char *split_eicas(void)
	{
	DWORD result;
#ifdef _MSC_VER

// FUEL TANKS
		Data_Read(0x0B74, sizeof(DWORD),		&all.dwFuelCenterPct,		&result);
		Data_Read(0x0B78, sizeof(DWORD),		&all.dwFuelCenterCap,		&result); 
		Data_Read(0x0B7C, sizeof(DWORD),		&all.dwFuelLeftPct,			&result); 
		Data_Read(0x0B80, sizeof(DWORD),		&all.dwFuelLeftCap,			&result); 
		Data_Read(0x0B84, sizeof(DWORD),		&all.dwFuelLeftAuxPct,		&result); 
		Data_Read(0x0B88, sizeof(DWORD),		&all.dwFuelLeftAuxCap,		&result); 
    	Data_Read(0x0B94, sizeof(DWORD),		&all.dwFuelRightPct,		&result);
		Data_Read(0x0B98, sizeof(DWORD),		&all.dwFuelRightCap,		&result);
		Data_Read(0x0B9C, sizeof(DWORD),		&all.dwFuelRightAuxPct,		&result);
		Data_Read(0x0BA0, sizeof(DWORD),		&all.dwFuelRightAuxCap,		&result);
// TEMPERATURE
		Data_Read(0x11D0, sizeof(WORD),			&all.wTAT, &result);
		Data_Read(0x0E8C, sizeof(WORD),			&all.wOutsideAirTemp,		&result);
// FLAPS AND GEAR
		Data_Read(0x0BDC, sizeof(DWORD),		&all.dwFlapsCommanded,		&result);
		Data_Read(0x0BE0, sizeof(DWORD),		&all.dwFlapsLeft,			&result);
		Data_Read(0x0BE4, sizeof(DWORD),		&all.dwFlapsRight,			&result);
		Data_Read(0x0BEC, sizeof(DWORD),		&all.dwNoseGearPosition,	&result);
		Data_Read(0x0BF0, sizeof(DWORD),		&all.dwLeftGearPosition,	&result);
		Data_Read(0x0BF4, sizeof(DWORD),		&all.dwRightGearPosition,	&result);
// ENGINE 1
		Data_Read(0x0892, sizeof(WORD),			&all.wEng1StarterSwitch,	&result);
		Data_Read(0x0894, sizeof(WORD),			&all.wEng1Combustion,		&result);
		Data_Read(0x088C, sizeof(WORD),			&all.wEng1Throttle,			&result);
		Data_Read(0x0898, sizeof(WORD),			&all.wEng1N1,				&result);
		Data_Read(0x0896, sizeof(WORD),			&all.wEng1N2,				&result);
		Data_Read(0x08BC, sizeof(WORD),			&all.wEng1EPR,				&result);
		Data_Read(0x0918, sizeof(FLOAT64),		&all.fEng1FF_PPH,			&result);
		Data_Read(0x08D4, sizeof(DWORD),		&all.dwEng1Vibration,		&result);
		Data_Read(0x08B8, sizeof(WORD),			&all.wEng1OilTemp,			&result);
		Data_Read(0x08BA, sizeof(WORD),			&all.wEng1OilPressure,		&result);
		Data_Read(0x08D0, sizeof(DWORD),		&all.dwEng1OilQty,			&result);
		Data_Read(0x08BE, sizeof(WORD),			&all.wEng1EGT,				&result);
		Data_Read(0x08D8, sizeof(DWORD),		&all.dwEng1HydraulicPressure,	&result);
		Data_Read(0x08DC, sizeof(DWORD),		&all.dwEng1HydraulicQty,		&result);
// ENGINE 2
		Data_Read(0x092A, sizeof(WORD),			&all.wEng2StarterSwitch,	&result);
		Data_Read(0x092C, sizeof(WORD),			&all.wEng2Combustion,		&result);
		Data_Read(0x0924, sizeof(WORD),			&all.wEng2Throttle,			&result);
		Data_Read(0x0930, sizeof(WORD),			&all.wEng2N1,				&result);
		Data_Read(0x092E, sizeof(WORD),			&all.wEng2N2,				&result);
		Data_Read(0x0954, sizeof(WORD),			&all.wEng2EPR,				&result);
		Data_Read(0x09B0, sizeof(FLOAT64),		&all.fEng2FF_PPH,			&result);
		Data_Read(0x096C, sizeof(DWORD),		&all.dwEng2Vibration,		&result);
		Data_Read(0x0950, sizeof(WORD),			&all.wEng2OilTemp,			&result);
		Data_Read(0x0952, sizeof(WORD),			&all.wEng2OilPressure,		&result);
		Data_Read(0x0968, sizeof(DWORD),		&all.dwEng2OilQty,			&result);
		Data_Read(0x0956, sizeof(WORD),			&all.wEng2EGT,				&result);
		Data_Read(0x0970, sizeof(DWORD),		&all.dwEng2HydraulicPressure,	&result);
		Data_Read(0x0974, sizeof(DWORD),		&all.dwEng2HydraulicQty,		&result);
// ENGINE 3
		Data_Read(0x09C2, sizeof(WORD),			&all.wEng3StarterSwitch,	&result);
		Data_Read(0x09C4, sizeof(WORD),			&all.wEng3Combustion,		&result);
		Data_Read(0x09BC, sizeof(WORD),			&all.wEng3Throttle,			&result);
		Data_Read(0x09C8, sizeof(WORD),			&all.wEng3N1,				&result);
		Data_Read(0x09C6, sizeof(WORD),			&all.wEng3N2,				&result);
		Data_Read(0x09EC, sizeof(WORD),			&all.wEng3EPR,				&result);
		Data_Read(0x0A48, sizeof(FLOAT64),		&all.fEng3FF_PPH,			&result);
		Data_Read(0x0A04, sizeof(DWORD),		&all.dwEng3Vibration,		&result);
		Data_Read(0x09E8, sizeof(WORD),			&all.wEng3OilTemp,			&result);
		Data_Read(0x09EA, sizeof(WORD),			&all.wEng3OilPressure,		&result);
		Data_Read(0x0A00, sizeof(DWORD),		&all.dwEng3OilQty,			&result);
		Data_Read(0x09EE, sizeof(WORD),			&all.wEng3EGT,				&result);
		Data_Read(0x0A08, sizeof(DWORD),		&all.dwEng3HydraulicPressure,	&result);
		Data_Read(0x0A0C, sizeof(DWORD),		&all.dwEng3HydraulicQty,		&result);
// ENGINE 4
		Data_Read(0x0A5A, sizeof(WORD),			&all.wEng4StarterSwitch,	&result);
		Data_Read(0x0A5C, sizeof(WORD),			&all.wEng4Combustion,		&result);
		Data_Read(0x0A54, sizeof(WORD),			&all.wEng4Throttle,			&result);
		Data_Read(0x0A60, sizeof(WORD),			&all.wEng4N1,				&result);
		Data_Read(0x0A5E, sizeof(WORD),			&all.wEng4N2,				&result);
		Data_Read(0x0A84, sizeof(WORD),			&all.wEng4EPR,				&result);
		Data_Read(0x0AE0, sizeof(FLOAT64),		&all.fEng4FF_PPH,			&result);
		Data_Read(0x0A9C, sizeof(DWORD),		&all.dwEng4Vibration,		&result);
		Data_Read(0x0A80, sizeof(WORD),			&all.wEng4OilTemp,			&result);
		Data_Read(0x0A82, sizeof(WORD),			&all.wEng4OilPressure,		&result);
		Data_Read(0x0A98, sizeof(DWORD),		&all.dwEng4OilQty,			&result);
		Data_Read(0x0A86, sizeof(WORD),			&all.wEng4EGT,				&result);
		Data_Read(0x0AA0, sizeof(DWORD),		&all.dwEng4HydraulicPressure,	&result);
		Data_Read(0x0AA4, sizeof(DWORD),		&all.dwEng4HydraulicQty,		&result);
// AUTOPILOT
		Data_Read(0x07BC, sizeof(DWORD),		&all.dwAPMaster,			&result);
		Data_Read(0x0810, sizeof(DWORD),		&all.dwAPAutoThrottle,		&result);
		Data_Read(0x3328, sizeof(int),			&all.elevator_axis,		&result);
		Data_Read(0x332a, sizeof(int),			&all.aileron_axis,		&result);
		Data_Read(0x332c, sizeof(int),			&all.rudder_axis,		&result);
		Data_Read(0x207c, sizeof(FLOAT64),		&all.TURB_ENGINE_1_PCT_REVERSER,		&result);
		Data_Read(0x217c, sizeof(FLOAT64),		&all.TURB_ENGINE_2_PCT_REVERSER,		&result);

		if (avionics)
			{
			Data_Read(0x07C0, sizeof(DWORD),	&all.dwAPWingLeveler,		&result);
			Data_Read(0x07C4, sizeof(DWORD),	&all.dwAPNav1Hold,			&result);
			Data_Read(0x07C8, sizeof(DWORD),	&all.dwAPHeadingHold,		&result);
			Data_Read(0x07CC, sizeof(DWORD),	&all.dwAPHeading,			&result);
			Data_Read(0x07D0, sizeof(DWORD),	&all.dwAPAltitudeHold,		&result);
			Data_Read(0x07D4, sizeof(DWORD),	&all.dwAPAltitude,  		&result);
			Data_Read(0x07D8, sizeof(DWORD),	&all.dwAPAttitudeHold,		&result);
			Data_Read(0x07DC, sizeof(DWORD),	&all.dwAPAirSpeedHold,		&result);
			Data_Read(0x07E2, sizeof(WORD),		&all.wAPAirSpeed,			&result);
			Data_Read(0x07E4, sizeof(DWORD),	&all.dwAPMachNumberHold,	&result);
			Data_Read(0x07E8, sizeof(WORD),		&all.dwAPMachNumber,		&result);
			Data_Read(0x07EC, sizeof(DWORD),	&all.dwAPVerticalSpeedHold,	&result);
			Data_Read(0x07F2, sizeof(SWORD),	&all.wAPVerticalSpeed,		&result);
			Data_Read(0x07F4, sizeof(DWORD),	&all.dwAPRPMHold,			&result);
			Data_Read(0x07FA, sizeof(WORD),		&all.wAPRPM,				&result);
			Data_Read(0x07FC, sizeof(DWORD),	&all.dwAPGlideSlopeHold,	&result);
			Data_Read(0x0800, sizeof(DWORD),	&all.dwAPLocalizerHold,		&result);
			Data_Read(0x0804, sizeof(DWORD),	&all.dwAPBackCourseHold,	&result);
			Data_Read(0x0808, sizeof(DWORD),	&all.dwAPYawDamper,			&result);
			Data_Read(0x080C, sizeof(DWORD),	&all.dwAPToGa,				&result);
			Data_Read(0x0810, sizeof(DWORD),	&all.dwAPAutoThrottle,		&result);
			}
// CAUTIONS
		Data_Read(0x036D, sizeof(BYTE),			&all.bOverSpeed,			&result);
// BRAKES
		Data_Read(0x0BC8, sizeof(WORD),			&all.wParkingBrake,			&result);
// SPOILERS / AILERONS / ELEVATORS / RUDDER
		Data_Read(0x0BD0, sizeof(WORD),			&all.wSpoilerCommanded,		&result);
		Data_Read(0x2ea0, sizeof(double),		&all.ELEVATOR_TRIM,			&result);
		Data_Read(0x2eb0, sizeof(double),		&all.AILERON_TRIM,			&result);
		Data_Read(0x2ec0, sizeof(double),		&all.RUDDER_TRIM,			&result);
// MISC
		Data_Read(0x0366, sizeof(BYTE),			&all.bPlaneOnGround,		&result);
		Data_Read(0x0281, sizeof(BYTE),			&all.byStrobeOn,			&result);
		Data_Read(0x0AEC, sizeof(WORD),			&all.wEngines,				&result);
		Data_Read(0x281c, 4,					&all.MASTER_BATTERY,		&result);
		Data_Read(0x6d0a, 1,					&all.byDimmer,				&result);

		dimmer = (double)all.byDimmer / 255.0;

		dimmer = 1.0;
		if (Data_Process(&result) == false)
		{
			Data_Close();
			notconnected = 1;
		}
#else
	fd1.dwAPMaster				=all.dwAPMaster                 ; 
	fd1.bOverSpeed				=all.bOverSpeed                 ; 
	fd1.wParkingBrake           =all.wParkingBrake                   ; 
	fd1.wSpoilerCommanded       =all.wSpoilerCommanded                   ; 
	fd1.dwAPAutoThrottle        =all.dwAPAutoThrottle                ; 
	fd1.bPlaneOnGround          =all.bPlaneOnGround                ; 
	fd1.byStrobeOn				=all.byStrobeOn                ; 

	fd1.dwFuelCenterPct           =all.dwFuelCenterPct                 ; 
	fd1.dwFuelCenterCap           =all.dwFuelCenterCap                 ; 
	fd1.dwFuelLeftPct             =all.dwFuelLeftPct                   ; 
	fd1.dwFuelLeftCap             =all.dwFuelLeftCap                   ; 
	fd1.dwFuelLeftAuxPct          =all.dwFuelLeftAuxPct                ; 
	fd1.dwFuelLeftAuxCap          =all.dwFuelLeftAuxCap                ; 
        fd1.dwFuelRightPct            =all.dwFuelRightPct                  ;
	fd1.dwFuelRightCap            =all.dwFuelRightCap                  ;
	fd1.dwFuelRightAuxPct         =all.dwFuelRightAuxPct               ;
	fd1.dwFuelRightAuxCap         =all.dwFuelRightAuxCap               ;
	fd1.dwFlapsCommanded          =all.dwFlapsCommanded                ;
	fd1.dwFlapsLeft               =all.dwFlapsLeft                     ;
	fd1.dwFlapsRight              =all.dwFlapsRight                    ;
	fd1.dwNoseGearPosition        =all.dwNoseGearPosition                ;
	fd1.dwLeftGearPosition        =all.dwLeftGearPosition                ;
	fd1.dwRightGearPosition       =all.dwRightGearPosition                ;

	fd1.wEng1N1          =all.wEng1N1                ;
	fd2.wEng1N2          =all.wEng1N2                ;
	fd1.wEng1EPR         =all.wEng1EPR               ;
	fd2.fEng1FF_PPH=all.fEng1FF_PPH      ;
	fd2.wEng1Combustion   =all.wEng1Combustion         ;
	fd2.wEng1Vibration   =all.wEng1Vibration         ;
	fd2.wEng1OilTemp     =all.wEng1OilTemp        ;
	fd2.wEng1OilPressure =all.wEng1OilPressure        ;
	fd2.dwEng1OilQty     =all.dwEng1OilQty                    ;
	fd1.wEng1EGT         =all.wEng1EGT             ;

	fd1.wEng2N1          =all.wEng2N1                ;
	fd2.wEng2N2          =all.wEng2N2                ;
	fd1.wEng2EPR         =all.wEng2EPR               ;
	fd2.fEng2FF_PPH=all.fEng2FF_PPH      ;
	fd2.wEng2Vibration   =all.wEng2Vibration         ;
	fd2.wEng2OilTemp     =all.wEng2OilTemp        ;
	fd2.wEng2OilPressure =all.wEng2OilPressure        ;
	fd2.dwEng2OilQty     =all.dwEng2OilQty                    ;
	fd1.wEng2EGT         =all.wEng2EGT             ;

	fd1.wEng3N1          =all.wEng3N1                ;
	fd2.wEng3N2          =all.wEng3N2                ;
	fd1.wEng3EPR         =all.wEng3EPR               ;
	fd2.fEng3FF_PPH=all.fEng3FF_PPH      ;
	fd2.wEng3Vibration   =all.wEng3Vibration         ;
	fd2.wEng3OilTemp     =all.wEng3OilTemp        ;
	fd2.wEng3OilPressure =all.wEng3OilPressure        ;
	fd2.dwEng3OilQty     =all.dwEng3OilQty                    ;
	fd1.wEng3EGT         =all.wEng3EGT             ;

	fd1.wEng4N1          =all.wEng4N1                ;
	fd2.wEng4N2          =all.wEng4N2                ;
	fd1.wEng4EPR         =all.wEng4EPR               ;
	fd2.fEng4FF_PPH=all.fEng4FF_PPH      ;
	fd2.wEng4Vibration   =all.wEng4Vibration         ;
	fd2.wEng4OilTemp     =all.wEng4OilTemp        ;
	fd2.wEng4OilPressure =all.wEng4OilPressure        ;
	fd2.dwEng4OilQty     =all.dwEng4OilQty                    ;
	fd1.wEng4EGT         =all.wEng4EGT             ;
#endif
	refresh = 0;

	set_eicas_msgs();
	return((char *)&all);
	}
