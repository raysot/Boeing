// $Id$
//#define WIN32_LEAN_AND_MEAN
#define EXTERN_EADI 1
#include "eadi.h"
#include <externs.h>
#include <fonts.h>
#include <Registry.h>

extern int debugmode;
extern int				flap[10];
extern int				flapsmax[10];
extern int				flapsmin[10];
extern int				flapsret[10];
extern int				maxspd;
extern int				minspd;
extern int				topmost;
extern int				gearmax;
extern int				v1,v2,vr;
extern int				mouseptr;

double					WindowSize = 50.0;   // glOrtho2d size

void PFDGetRegistry(void)
{
	if ((wx = (int)RegGetDWord("BOEING\\PFD", "wx")) == -1)
		return;
	wy = (int)RegGetDWord("BOEING\\PFD", "wy");
	width = (int)RegGetDWord("BOEING\\PFD", "width");
	height = (int)RegGetDWord("BOEING\\PFD", "height");
	windowframes = (int)RegGetDWord("BOEING\\PFD", "windowframes");
	tilt = (double)RegGetDWord("BOEING\\PFD", "tilt");
	mouseptr = (int)RegGetDWord("BOEING\\PFD", "mouseptr");
	topmost = (int)RegGetDWord("BOEING\\PFD", "topmost");

	colorstyle = (int)RegGetDWord("BOEING\\PFD", "ColorStyle");
	style = (int)RegGetDWord("BOEING\\PFD", "Style");
	decision_height = (int)RegGetDWord("BOEING\\PFD", "Decision Height");
	imperial = (int)RegGetDWord("BOEING\\PFD", "Imperial");
	cb = (int)RegGetDWord("BOEING\\PFD", "cb");
	stleft = (int)RegGetDWord("BOEING\\PFD", "sl");
	stright = (int)RegGetDWord("BOEING\\PFD", "sr");
	atleft = (int)RegGetDWord("BOEING\\PFD", "al");
	atright = (int)RegGetDWord("BOEING\\PFD", "ar");
	debugmode = (int)RegGetDWord("BOEING\\PFD", "DebugMode");
}

void PFDSetRegistry(void)
{
	RegSetDWord("BOEING\\PFD", "wx", wx);
	RegSetDWord("BOEING\\PFD", "wy", wy);
	RegSetDWord("BOEING\\PFD", "width", width);
	RegSetDWord("BOEING\\PFD", "height", height);
	RegSetDWord("BOEING\\PFD", "windowframes", windowframes);
	RegSetDWord("BOEING\\PFD", "tilt", tilt);
	RegSetDWord("BOEING\\PFD", "mouseptr", mouseptr);
	RegSetDWord("BOEING\\PFD", "topmost", topmost);

	RegSetDWord("BOEING\\PFD", "ColorStyle", colorstyle);
	RegSetDWord("BOEING\\PFD", "Style", style);
	RegSetDWord("BOEING\\PFD", "Decision Height", decision_height);
	RegSetDWord("BOEING\\PFD", "Imperial", imperial);
	RegSetDWord("BOEING\\PFD", "cb", cb);
	RegSetDWord("BOEING\\PFD", "sl", stleft);
	RegSetDWord("BOEING\\PFD", "sr", stright);
	RegSetDWord("BOEING\\PFD", "al", atleft);
	RegSetDWord("BOEING\\PFD", "ar", atright);
}

// ***************************************
// This code sucks,  thanks Microsoft
// ***************************************
int get_flappos()
	{
	double				increment = 16383.0 / (double)flapcount;
	double				init;
	int					count = 0;

	for (init = 0.0; init <= 16383.0; init += increment)
		{
		if ((all.dwFlapsCommanded > (init-100.0)) && (all.dwFlapsCommanded < (init + 100.0)))
			return(count);
		count++;
		}
	return(count);
	}

// ******************************************************
// Routine called before exiting     
// ******************************************************
void getoutofdodge()
	{
	FILE			*geometry;
	RECT			lprect;

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

        /* shut down our window */
	wx = glutGet(GLUT_WINDOW_X);
	wy = glutGet(GLUT_WINDOW_Y);

	PFDSetRegistry();
/*
// ***********************************
// Write the defaults to the INI file
// ***********************************
	geometry = fopen("eadi.ini", "w");
	if (geometry != NULL)
		{
		fprintf(geometry, "WINDOW:%d:%d:%d:%d:%d:\n", wx, wy, width, height, windowframes);
		fprintf(geometry, "COLOR:%d:\n", colorstyle);
		fprintf(geometry, "TYPE:%d:\n", style);
		fprintf(geometry, "OPTIONS:%d:%d:%.2f:%.1f:%d:%d:\n", 
			decision_height, imperial, cb, tilt, mouseptr, topmost);
		fprintf(geometry, "TAPES:%.1f:%.1f:%.1f:%.1f:\n",
			stleft, stright, atleft, atright);
		fprintf(geometry, "TILT:%f:\n", tilt);
		fclose(geometry);
		}
*/
	Data_Close();
	wglDeleteContext(hrc);
	glutDestroyWindow(window);

        /* exit the program...normal termination. */
    exit(0);
	}

void get_airfile(char *fn)
{
	FILE				*af;
	int					flapcount = 0;
	int					fp;
	int					line = 0;
	double				fw = 0.0;
	char				buffer[200];
	char				*token;
	char				*delimiter = ":";

	static double		lbs = 0.0;

	if (!lbs)
	{
		af = fopen(fn, "r");
		if (af == NULL)
			return;
		while (fgets(buffer, 199, af))
		{
			line++;
			if (buffer[0] == '#')
				continue;
			if (strlen(buffer) < 3)
				continue;
			token = strtok(buffer, delimiter);
			if (token == NULL)
			{
				break;			
			}
			if (!strcmpi(token, "gearmax"))
			{
				token = strtok(NULL, delimiter);
				if (token == NULL)
					break;
				gearmax = atoi(token);
			}
			else if (!strcmpi(token, "v1"))
			{
				token = strtok(NULL, delimiter);
				if (token == NULL)
					break;
				v1 = atoi(token);
			}
			else if (!strcmpi(token, "v2"))
			{
				token = strtok(NULL, delimiter);
				if (token == NULL)
					break;
				v2 = atoi(token);
			}
			else if (!strcmpi(token, "vr"))
			{
				token = strtok(NULL, delimiter);
				if (token == NULL)
					break;
				vr = atoi(token);
			}
			else if (!strcmpi(token, "flap"))
			{
				token = strtok(NULL, delimiter);
				if (token == NULL)
					break;		
				fp = atoi(token);
				flap[flapcount] = fp;
				token = strtok(NULL, delimiter);
				if (token == NULL)
					break;		
				flapsmax[flapcount] = atoi(token);
				token = strtok(NULL, delimiter);
				if (token == NULL)
					break;		
				flapsmin[flapcount] = atoi(token);
				token = strtok(NULL, delimiter);
				if (token == NULL)
					break;		
				flapsret[flapcount] = atoi(token);
				if (flap[flapcount] == 0)
				{
					maxspd = flapsmax[flapcount];
					minspd = flapsmin[flapcount];
				}
				flapcount++;
			}
			else
			{
				break;
			}
		}
		fclose(af);
	}

	aircraft_weight = lbs * 0.454;

	fw = ((((double)all.dwFuelCenterPct / 83886.0 ) / 100.0 ) * (double)all.dwFuelCenterCap );
	fw += ((((double)all.dwFuelLeftPct / 83886.0 ) / 100.0 ) * (double)all.dwFuelLeftCap );
	fw += ((((double)all.dwFuelLeftAuxPct / 83886.0 ) / 100.0 ) * (double)all.dwFuelLeftAuxCap );
	fw += ((((double)all.dwFuelRightPct / 83886.0 ) / 100.0 ) * (double)all.dwFuelRightCap );
    fw += ((((double)all.dwFuelRightAuxPct / 83886.0 ) / 100.0 ) * (double)all.dwFuelRightAuxCap );

	fw *= ((double)all.wFuelWeight / 256.0);

	fw *= 0.454;


	aircraft_weight += fw;

}


/*
** Timer callback to fine tune FSUIPC reads
*/ 
void RefreshData(int v)
	{
	if (v){};
	split_eadi();
	glutTimerFunc(update_interval, RefreshData, 0);
	return;
	}

void idlefunc(void)
	{
	split_eadi();
	glutPostRedisplay();
	Sleep(10);
	}

void init(void) 
	{
    size = sizeof(Sa);

   	glClearColor (cr, cg, cb, 0.0);
   	glShadeModel (GL_FLAT);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_TEXTURE_2D);
//	frameid = tgaLoadAndBind ( "faceplate.tga",  0);
	load_font(NULL);
	load_speedo();

	}

void reshape(int w, int h)
	{
	width = clipx = w; 
	height = clipy = h;
   	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
   	glMatrixMode(GL_PROJECTION);
   	glLoadIdentity();
   	gluOrtho2D(	(WindowSize * -1.0)-padding, WindowSize+padding, 
				(WindowSize * -1.0)-padding, WindowSize+padding);
	ortho = (WindowSize+padding)*2.0;
   	glMatrixMode(GL_MODELVIEW);
   	glLoadIdentity();
	}

int main(int argc, char** argv)
	{
	char				ipaddr[20];
	FILE				*geometry;
	char				*token;
	int					init2;
	int					ff = 0;
	char				fontfile[200];
	char				buffer[200];

	wx = wy = 100;
	width = height = 650;

	strcpy(revision, "0.1 (beta)");
   	glutInit(&argc, argv);
	strcpy(workingdir, argv[0]);
	for (init2 = strlen(workingdir); init2>=0; init2--)
	{
		if (workingdir[init2] == '\\')
		{
			workingdir[init2+1] = '\0';
			break;
		}
	}

   	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA| GLUT_STENCIL);//A);

	PFDGetRegistry();
/*
	geometry = fopen("eadi.ini", "r");
	if (geometry != NULL)
		{
		while (fgets((char *)buffer, 100, geometry))
			{
			if (!strncmp((char *)buffer, "TYPE", 4))
				{
				token = strtok((char *)buffer, ":");
				token = strtok(NULL, ":");
				}
			else if (!strncmp((char *)buffer, "COLOR", 5))
				{
				token = strtok((char *)buffer, ":");
				token = strtok(NULL, ":");
				colorstyle = atoi(token);
				}
			else if (!strncmp((char *)buffer, "OPTIONS", 7))
				{
				token = strtok((char *)buffer, ":");
				token = strtok(NULL, ":");
				if (token != NULL)
					decision_height = atoi(token);
				token = strtok(NULL, ":");
				if (token != NULL)
					imperial = atoi(token);
				token = strtok(NULL, ":");
				if (token != NULL)
					cb = atof(token);
				token = strtok(NULL, ":");
				if (token != NULL)
					tilt = atof(token);
				token = strtok(NULL, ":");
				if (token != NULL)
					mouseptr = atoi(token);
				token = strtok(NULL, ":");
				if (token != NULL)
					topmost = atoi(token);
				}
//			else if (!strncmp((char *)buffer, "TAPES", 5))
//				{
//				token = strtok((char *)buffer, ":");
//				token = strtok(NULL, ":");
//				stleft = atof(token);
//				token = strtok(NULL, ":");
//				if (token != NULL)
//					stright = atof(token);
//				token = strtok(NULL, ":");
//				if (token != NULL)
//					atleft = atof(token);
//				token = strtok(NULL, ":");
//				if (token != NULL)
//					atright = atof(token);
//				}
			else if (!strncmp((char *)buffer, "WINDOW", 6))
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
			else if (!strncmp((char *)buffer, "TILT", 4))
				{
				token = strtok((char *)buffer, ":");
				token = strtok(NULL, ":");
				tilt = atof(token);
				}
			}
		fclose(geometry);
		}
  */
	glutInitWindowSize (width, height); 
   	glutInitWindowPosition (wx, wy);
   	sprintf(wintitle, "Ellie Systems EADI %s%s", revision, subversion);
   	window = glutCreateWindow (wintitle);
   	init ();
   	glutDisplayFunc(DrawGLHorizon); 
   	glutReshapeFunc(reshape); 
   	glutIdleFunc(idlefunc);
   	glutKeyboardFunc(keyPressed);
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
	atexit(getoutofdodge);
	ipaddr[0] = '\0';
	strcpy(airfile, "default.cfg");
	if (argc > 1)
		{
		for (init2 = 1; init2 < argc; init2++)
			{
			if (argv[init2][0] == '-')
				{
				switch(argv[init2][1])
					{
					case 'p':
					case 'P':
						if (strlen(argv[init2]) == 2)
							padding = atof(argv[init2+1]);
						else
							padding = atof(&argv[init2][2]);
						break;
					case 'a':
					case 'A':
						if (strlen(argv[init2]) == 2)
							strcpy(airfile, argv[init2+1]);
						else
							strcpy(airfile, &argv[init2][2]);
						break;
					case 'f':
					case 'F':
						ff = 1;
						strcpy(fontfile, argv[init2+1]);
						break;
					case 'i':
					case 'I':
						strcpy(ipaddr, argv[init2+1]);
						break;
					default:
						exit(0);
						break;
					}
				}
			}
		}
   	sock = make_socket(PORT, ipaddr);
	if (sock == -1)
		notconnected = 1;
	reshape(WIDTH, HEIGHT);
	glEnableClientState(GL_VERTEX_ARRAY);
	firsttime = 1;
  	glutMainLoop();
   	return 0;   /* ANSI C requires main to return int. */
	}

char *split_eadi()
	{
	static int				time = timeGetTime();
	static int				oldtime;
	DWORD					result;
	double FDSAlt = 0.0;

    oldtime = timeGetTime() - time;

	all.converter_active = 0;
		// No sense getting this stuff in real-time.

		Data_Read(0x3080, sizeof(double),		&all.Z_Acceleration,		&result);
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
		Data_Read(0x0AF4, sizeof(WORD),			&all.wFuelWeight,			&result);
		Data_Read(0x0894, 2,					&all.wEng1Combustion,			&result);
		Data_Read(0x092C, 2,					&all.wEng2Combustion,			&result);
		Data_Read(0x09C4, 2,					&all.wEng3Combustion,			&result);
		Data_Read(0x0A5C, 2,					&all.wEng4Combustion,			&result);
		Data_Read(0x0e8c, 2,					&all.wOutsideAirTemp,			&result);
		Data_Read(0x02C4, sizeof(DWORD),		&all.dwBarberPole,				&result);
		Data_Read(0x0f48, 2,					&all.wBarometricPressure,		&result);
		Data_Read(0x0E90, sizeof(WORD),			&all.wWindSpeed,				&result);
		Data_Read(0x0E92, sizeof(WORD),			&all.wWindDirection,			&result);
		Data_Read(0x3000, 6,					&all.vor1_id,					&result);
// request to read everything
		Data_Read(0x0330, 2,					&all.wAltimeter,				&result);
		Data_Read(0x0350, sizeof(WORD),			&all.wBcdNav1Freq,				&result);
		Data_Read(0x0366, 1,					&all.bPlaneOnGround,			&result);
		Data_Read(0x2ee0, sizeof(BOOL),			&all.FLIGHT_DIRECTOR_ACTIVE,	&result);
		Data_Read(0x07bc, 4,					&all.dwAPMaster,				&result);
		Data_Read(0x07c4, 4,					&all.dwAPNav1Hold,				&result);
		Data_Read(0x07c8, 4,					&all.dwAPHeadingHold,			&result);
		Data_Read(0x07d0, 4,					&all.dwAPAltitudeHold,			&result);
		Data_Read(0x07dc, 4,					&all.dwAPAirSpeedHold,			&result);
		Data_Read(0x07e4, 4,					&all.dwAPMachNumberHold,		&result);
		Data_Read(0x07fc, 4,					&all.dwAPGlideSlopeHold,		&result);
		Data_Read(0x0800, 4,					&all.dwAPLocalizerHold,			&result);
		Data_Read(0x0810, 4,					&all.dwAPAutoThrottle,			&result);
		Data_Read(0x0bac, 2,					&all.wInnerMarker,				&result);	
		Data_Read(0x0bae, 2,					&all.wMiddleMarker,				&result);	
		Data_Read(0x0bb0, 2,					&all.wOuterMarker,				&result);	
		Data_Read(0x0BDC, 4,					&all.dwFlapsCommanded,			&result);
		Data_Read(0x0AEC, 2,					&all.wEngines,					&result);		
		Data_Read(0x0c29, 5,					&all.sDME1, 					&result);
		Data_Read(0x0c48, 1,					&all.bLocalizerNeedle,			&result);
		Data_Read(0x0c49, 1,					&all.bGlideSlopeNeedle,			&result);
		Data_Read(0x036E, sizeof(BYTE),			&all.bSideSlip,					&result);
		Data_Read(0x07cc, 4,					&all.dwAPHeading,				&result);
		Data_Read(0x07e2, 2,					&all.wAPAirSpeed,				&result);
		Data_Read(0x07e8, 4,					&all.dwAPMachNumber,			&result);	
		Data_Read(0x07f2, 2,					&all.wAPVerticalSpeed,			&result);
		Data_Read(0x02c8, 4,					&all.dwVS,						&result);
		Data_Read(0x0842, 2,					&all.wVerticalSpeed,			&result);	
		Data_Read(0x2ee8, sizeof(FLOAT64),		&all.FLIGHT_DIRECTOR_PITCH,		&result);
		Data_Read(0x2ef0, sizeof(FLOAT64),		&all.FLIGHT_DIRECTOR_BANK,		&result);
		Data_Read(0x3300, sizeof(BOOL16),		&all.NAVAID_FLAGS,				&result);
		Data_Read(0x0BB2, sizeof(WORD),			&all.wElevator,					&result);
		Data_Read(0x0BB6, sizeof(WORD),			&all.wAileron,					&result);

//		Data_Read(0x5600, 1,				&all.converter_active,			&result);
//		Data_Read(0x5658, sizeof(FLOAT64),&all.fDimmer,					&result);

		Data_Read(0x02b8, 4,					&all.dwTAS,					&result);
		Data_Read(0x02bc, 4,					&all.dwIAS,					&result);
		Data_Read(0x02B4, 4,					&all.dwGS,					&result);
		
		if (FDSActive)
			Data_Read(0x0570, 8,				&FDSAlt,				&result);
		Data_Read(0x0570, 4,					&all.dwAltLo,				&result);
		Data_Read(0x0574, 4,					&all.dwAltHi,				&result);

		Data_Read(0x3324, 4,					&all.altimeter,				&result);
		Data_Read(0x0E90, sizeof(WORD),			&all.wWindSpeed,				&result);
		Data_Read(0x0E92, sizeof(WORD),			&all.wWindDirection,			&result);
		Data_Read(0x0580, 4,					&all.dwHeading,				&result);
		Data_Read(0x02a0, sizeof(WORD),			&all.wMagVar,					&result);
		Data_Read(0x0578, 4,					&all.dwPitch,				&result);
		Data_Read(0x057c, 4,					&all.dwBank,				&result);
		Data_Read(0x0870, sizeof(WORD),			&all.wILSOBS,					&result);
		Data_Read(0x0b4c, sizeof(WORD),			&all.wGroundAlt,				&result);
		Data_Read(0x07d4, 4,					&all.dwAPAltitude,			&result);
		Data_Read(0x11be, 2,					&all.wAOA,				&result);
		Data_Read(0x281c, 4,					&all.MASTER_BATTERY,	&result);
		Data_Read(0x6d04, 2,					&v1,						&result);
		Data_Read(0x6d06, 2,					&vr,						&result);
		Data_Read(0x6d08, 2,					&v2,						&result);
//		Data_Read(0x6d0a, 1,					&all.byDimmer,				&result);

//		dimmer = (double)all.byDimmer / 255.0;

// send the data request
		if (Data_Process(&result) == false)
		{
			
			Data_Close();
			notconnected = 1;
		}
// Conversions

//	if (!all.converter_active)
//		{
		vs = ( ( (double)(all.dwVS * 60) * 3.28084) / 256.0 );
// Altitude
//		if (FDSActive)
//			alt = ((double)FDSAlt / (double)0x100000000) * 3.2808399;
//		else
			alt = ((double)all.dwAltHi +(double)all.dwAltLo / (double)0x100000000) * 3.2808399;
//			sprintf(display_buffer, "Altitude: %.2f", alt);
//	MessageBox(0,display_buffer, "debug", MB_OK);
// Heading
		hdg = 360.0*(double)all.dwHeading/(double)0x100000000;
// Magnetic Variation
		magvar = (double)(all.wMagVar*360.0/0x10000);
		if (magvar > 180)
		    magvar = magvar - 360;
		magvar *= -1.0;
// True Airspeed
		tas = (double)all.dwTAS / 128.0;
// Indicated Airspeed
		ias = (double)all.dwIAS / 128.0;
// Banking in degrees
		bank = 360.0*(double)all.dwBank/(double)0x100000000;
		if (bank > 180.0)
			bank = ((360.0 - bank));
		else
			bank *= -1.0f;
//		bank *= -1.0;
// Pitch in degrees
		pitch = 360.0*(double)all.dwPitch/(double)0x100000000;
		if (pitch > 180.0)
		    pitch = ((360.0 - pitch) / 10.0) *-1.0f;
		else
		    pitch /= 10.0f;
// ILS OBS       Strange beast,  it's the inverse heading to follow the ILS.  
		ilsobs = (double)(all.wILSOBS * 360.0) / 65536.0;
		ilsobs -= 180.0;    // Correct the silly thing.
		if (ilsobs < 0.0)
			ilsobs += 360.0;
// Radio Altimeter
		ra = (double)((long)(alt) - (long)(all.wGroundAlt * 3.2808399));
// Autopilot Altitude setting
		apalt = ((double)all.dwAPAltitude / (double)0x10000) * 3.2808399;

		maxaoa = (double)(100.0-(100.0*(double)all.wAOA/32767.0));

//		}
	gs = (int)(((double)all.dwGS/65536.0)*1.943846);
	wd = 360.0*(double)all.wWindDirection/65536.0;

	return((char *)&all);
	}

