//#define WIN32_LEAN_AND_MEAN
#define FILL 1             // Produce rectangles?

#include <windows.h>
#include <IPCuser.h>
#define MSFS 1
extern HDC hdc;
extern HGLRC hrc;
extern WNDCLASS a;

#define PLANECOUNT 5
#define A320	0
#define B757	1
#define B747	2
#define B777	3
#define B767	4

#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
/* UNIX
#include <unistd.h>
#include <linux/lp.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/io.h>
#include <netinet/in.h>
#include <arpa/inet.h>
*/
#include <sys/types.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "vertices.h"
#include <tgaload.h>
// #include <fnt.h>
#define MAX_FONTS 4
#include <Fs98State.h>
#include <structs.h>
#include <prototypes.h>
#define SBOEING	0
#define SAIRBUS	1
#define WIDTH 650
#define HEIGHT 570
#define ORTHO 50.0 
//   DISPLAY LISTS
#define PITCHL 		0
#define PLANEL 		1
#define SPEEDL 		2
#define VSIL   		3
#define ALTITUDEL	4
#define COMPASSL	5
#define GLIDESLOPEL	6
#define OBSL		7
#define ESCAPE 27
#define MACH 625.0

#ifndef EXTERN_EADI
char variation = 0;
int mask = 1;
int smask = 0;
unsigned int size;
int sock;
char *subversion = " (Pre-release ALPHA)";
char radalt[10];
//extern char *font_names[];
//fntRenderer *texout;
//fntTexFont *font_list[MAX_FONTS];
HWND hwnd;
long wsl;
int				fdils = 0;   // FD switches to ILS tracking mode.
int				fdmodechange = 0;     // For the flashing FD
int				fdframes = 0;   // How many frames must it flash.
int				fdflash = 0;    // fps/2 frames on fps/2 frames off
int				detail = 0;
double			tilt = 0.0;
int				windowframes = 1;
double			padding = 0.0;
double			cr = 0.0,				cg = 0.0,			cb = 0.0;
double			br = 50.0/255.0,		bg = 50.0/255.0,	bb = 50.0/255.0;
double			sr = 35.0/255.0,		sg = 114.0/255.0,	sb = 183.0/255.0;
double			gr = 162.0/255.0,		gg = 96.0/255.0,	gb = 0.0/255.0;
double			bankstart, bankends, bankendl;
double			clipx = WIDTH; 
double			clipy = HEIGHT;
double			ortho = ORTHO*2;            
int				lines;
double			compassradius = 37.0;
double			Nav1Rad;
double			Nav2Rad;
extern char		*wintitle;
int update_interval = 200;
double			adf;
extern double	dimmer;
extern int		frames;
extern DWORD	basetick;
double			tposx = 0.0;
double			tposy = 0.0;
int				flapcount = 0;
int				simplefd = 1;
int				width, height;
int firsttime = 1;
int				monochrome = 0;
int				wx, wy;
extern int		pretty;
int				imperial = 0;
int				display_stats = 1;
#ifdef AIRBUS
int				style = SAIRBUS;
#else
int				style = SBOEING;
#endif
int				colorstyle = B777;
int				window;
int				decision_height = 200;
double			stleft = -49.0;    // Speed tape dimensions.
double			stright = -38.0;    // Speed tape dimensions.
double			stbottom = -35.0;    // Speed tape dimensions.
double			sttop = 35.0;    // Speed tape dimensions.
double			atleft = 30.0;    // Alt tape dimensions
double			atright = 42.0;    // Alt tape dimensions    SUBTRACT 2 FOR THE AIRBUS
double			wbank = 0.0;          // When bank surpasses 40 degrees either side, Airbus gets fancy.
double			wpitch = 0.0;          // When pitch surpasses +25 degrees or -13 degrees, Airbus gets fancy.
int				bankok = 1;              // Bank/Pitch limit exceeded = 0
double			atbottom = -35.0;    // Alt tape dimensions
double			attop = 35.0;    // Alt tape dimensions
extern int		direct;
extern int		noserver;
extern int		fixblue;
int				notconnected = 0;
int				arcmode = 0;
FS98StateData	all;
extern char			*revision;
double			wca;
int				landing = 0;      // Airbus rollout flag
extern char			*airfile;
double			ffcorrection = 0.0;
FLAPS			flaps[20];
FILE			*logfile;
// *****************************
// Display lists
// *****************************
GLuint			displaylist[10];     // General
GLuint			pll = 0;   // Pitch ladder
GLuint			bal = 0;   // Bank angle lines
GLuint			pl = 0;    // Plane wings 
// ********************************************************************************
// THESE ARE ALL THE CONVERTED FS VARS (Conversions made after FSUIPC_Process();
// ********************************************************************************
double alt;
double vs;
double hdg;
double magvar;
double ias;
int  gs;
double tas;
double pitch;
double bank;
double ilsobs;
double apalt;
double wd;
double ra;   // Radio altimeter
//****************************************
// Aircraft Weights
//****************************************
double aircraft_weight = 0.0;
double green_dot = 0.0;

// ***************
// Prototypes
// ***************
extern void		dofps(int);
extern double	r2d(double deg);
extern double	d2r(double deg);
int				get_flappos(void);
void			getoutofdodge(void);
char *			split_eadi(void);
char			display_buffer[200];
extern char			*workingdir;
void			DrawGLHorizon(void);
void			keyPressed(unsigned char key, int x, int y);
void			draw_horizon(void);
void			draw_plane(void);
void			draw_flightdirector(void);
void			draw_airbus_compass(int);
void			draw_speedbar(void);
void			draw_vsi(void);
void			draw_altitudebar(void);
void			draw_autopilot(void);
void			draw_compassface(int);
void			draw_compass(void);
void			pitchladder(void);
int				draw_needles(void);
extern void		setColor(double, double, double);
void			countdown(void);
void			showalerts(void);

#else

extern char variation;
extern int mask;
extern int smask;
extern unsigned int size;
extern int sock;
extern char *subversion;
extern char *radalt[10];
//char *font_names[MAX_FONTS] = 
//	{
//	"helv.txf",
//	NULL
//	};
//extern fntRenderer *texout;
//extern fntTexFont *font_list[MAX_FONTS];
extern HWND hwnd;
extern long wsl;
extern int fdils;
extern int fdmodechange;
extern int				windowframes;
extern int				fdframes;
extern int				fdflash;
extern int				detail;
extern double			tilt;
extern double			padding;
extern double			cr,	cg,	cb;
extern double			br,	bg,	bb;
extern double			sr,	sg,	sb;
extern double			gr,	gg,	gb;
extern double			bankstart, bankends, bankendl;
extern double			clipx; 
extern double			clipy;
extern double			ortho;            
extern int				lines;
extern double			compassradius;
extern int update_interval;
extern double			Nav1Rad;
extern double			Nav2Rad;
char			wintitle[200];
extern double			adf;
extern double	dimmer;
extern int		frames;
extern DWORD	basetick;
extern double			tposx;
extern double			tposy;
extern int				flapcount;
extern int				simplefd;
extern int				width, height;
extern int firsttime;
extern int				monochrome;
extern int				wx, wy;
extern int		pretty;
extern int				imperial;
extern int				display_stats;
extern int				style;
extern int				colorstyle;
extern int				window;
extern int				decision_height;
extern double			stleft;
extern double			stright;
extern double			stbottom;
extern double			sttop;
extern double			atleft;
extern double			atright;
extern double			wbank;
extern double			wpitch;
extern int				bankok;
extern double			atbottom;
extern double			attop;
extern int		direct;
extern int		noserver;
extern int		fixblue;
extern int				notconnected;
extern int				arcmode;
extern FS98StateData	all;
char			revision[30];
extern double			wca;
extern int				landing;
char			airfile[200];
extern double			ffcorrection;
extern FLAPS			*flaps;
extern FILE			*logfile;
extern GLuint			*displaylist;
extern GLuint			pll;
extern GLuint			bal;
extern GLuint			pl;
extern double alt;
extern double vs;
extern double hdg;
extern double magvar;
extern double ias;
extern int  gs;
extern double tas;
extern double pitch;
extern double bank;
extern double ilsobs;
extern double apalt;
extern double wd;
extern double ra;
extern double aircraft_weight;
extern double green_dot;
extern char			*display_buffer;
char			workingdir[200];
extern void		dofps(int);
extern double	r2d(double deg);
extern double	d2r(double deg);
extern char *			split_eadi(void);
extern void			DrawGLHorizon(void);
extern void			keyPressed(unsigned char key, int x, int y);
extern void			draw_horizon(void);
extern void			draw_plane(void);
extern void			draw_flightdirector(void);
extern void			draw_airbus_compass(int);
extern void			draw_speedbar(void);
extern void			draw_vsi(void);
extern void			draw_altitudebar(void);
extern void			draw_autopilot(void);
extern void			draw_compassface(int);
extern void			draw_compass(void);
extern void			pitchladder(void);
extern int				draw_needles(void);
extern void		setColor(double, double, double);
extern void			countdown(void);
extern void			showalerts(void);
#endif
