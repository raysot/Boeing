// $Id$
//#define WIN32_LEAN_AND_MEAN
#define EXTERN_EADI 1
#include "eadi.h"
#include <fonts.h>

extern int flap[10];
extern int flapsmax[10];
extern int flapsmin[10];
extern int flapsret[10];
extern int maxspd;
extern int minspd;
extern int gearmax;
extern int v1,v2,vr;
extern double adipos;
extern double spdpos;
extern int topmost;
extern double altpos;
extern int mouseptr;

double WindowSize = 30.0;

// ***************************************
// This code sucks,  thanks Microsoft
// ***************************************
int get_flappos()
	{
	double increment = 16383.0 / (double)flapcount;
	double init;
	int count = 0;

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
	FILE *geometry;
	RECT lprect;
	/* get rid of font display lists */
//	KillFont();
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
// ***********************************
// Write the defaults to the INI file
// ***********************************
	geometry = fopen("standby.ini", "w");
	if (geometry != NULL)
		{
		fprintf(geometry, "WINDOW:%d:%d:%d:%d:%d:\n", wx, wy, width, height, windowframes);
		fprintf(geometry, "COLOR:%d:\n", colorstyle);
		fprintf(geometry, "POSITIONS:%.2f:%.2f:%.2f:\n", adipos,spdpos,altpos);
		fprintf(geometry, "TYPE:%d:\n", style);
		fprintf(geometry, "OPTIONS:%d:%d:%.2f:%.1f:%d:%d:\n", 
			decision_height,imperial,cb, tilt, mouseptr, topmost);
		fprintf(geometry, "TAPES:%.1f:%.1f:%.1f:%.1f:\n",
			stleft, stright, atleft, atright);
		fclose(geometry);
		}
//	fclose(logfile);
	Data_Close();
	wglDeleteContext(hrc);
	glutDestroyWindow(window);

        /* exit the program...normal termination. */
    exit(0);
	}

void get_airfile(char *fn)
{
	FILE *af;
	int flapcount = 0;
	int fp;
	int line = 0;
	double fw = 0.0;
	char buffer[200];
	char *token;
	char *delimiter = ":";

	static double lbs = 0.0;

	if (!lbs)
	{
		af = fopen(fn, "r");
		if (af == NULL)
			return;
		while (fgets(buffer, 199, af))
		{
			line++;
/*
gearmax:200:
v1:140:
v2:150:
vr:145:
flap:0:340:0:0:
flap:1:226:180:170:
flap:5:206:170:160:
flap:15:186:160:150:
flap:20:186:150:140:
flap:25:174:140:130:

extern int flaps[10];
extern int flapsmax[10];
extern int flapsmin[10];
extern int flapsret[10];
extern int maxspd;
extern int minspd;
extern int gearmax;

*/
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
//	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	}

void reshape(int w, int h)
	{
	width = clipx = w; 
	height = clipy = h;
   	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
   	glMatrixMode(GL_PROJECTION);
   	glLoadIdentity();
	if (tilt == 90.0 || tilt == 270.0)
	   	gluOrtho2D(-110.0, 110.0, WindowSize * -1.0, WindowSize);
	else
	   	gluOrtho2D(WindowSize * -1.0, WindowSize, -110.0, 110.0);
	ortho = (50.0+padding)*2.0;
   	glMatrixMode(GL_MODELVIEW);
   	glLoadIdentity();
	}


INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, INT nCmdShow )
//int main(int argc, char** argv)
	{
	char ipaddr[20];
	FILE *geometry;
	char *token;
	int init2;
	int ff = 0;
	char fontfile[200];
	wx = wy = 100;
	width = height = 650;
	char buffer[200];
	
	display_stats = 0;

	open_log("eadi.log");
 	strcpy(revision, "0.1 (beta)");
//   	glutInit(&argc, argv);
//	strcpy(workingdir, argv[0]);
///	for (init2 = (int)strlen(workingdir); init2>=0; init2--)
//		if (workingdir[init2] == '\\')
//			{
//			workingdir[init2+1] = '\0';
//			break;
//			}
   	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA);//A);

	geometry = fopen("standby.ini", "r");
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
			else if (!strncmp((char *)buffer, "TAPES", 5))
				{
				token = strtok((char *)buffer, ":");
				token = strtok(NULL, ":");
				stleft = atof(token);
				token = strtok(NULL, ":");
				if (token != NULL)
					stright = atof(token);
				token = strtok(NULL, ":");
				if (token != NULL)
					atleft = atof(token);
				token = strtok(NULL, ":");
				if (token != NULL)
					atright = atof(token);
				}
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
			else if (!strncmp((char *)buffer, "POSITIONS", 9))
				{
				token = strtok((char *)buffer, ":");
				token = strtok(NULL, ":");
				adipos = atof(token);
				token = strtok(NULL, ":");
				spdpos = atof(token);
				token = strtok(NULL, ":");
				altpos = atof(token);
				}
			}
		fclose(geometry);
		}
//	get_airfile("speeds.dat");
//	sprintf(display_buffer, "V1:%d V2:%d VR:%d", v1, v2, vr);
//	MessageBox(0, display_buffer, "debug", 0);
   	glutInitWindowSize (width, height); 
   	glutInitWindowPosition (wx, wy);
   	sprintf(wintitle, "Ellie Systems Standby Set %s%s", revision, subversion);
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
//	if (argc > 1)
//		{
//		for (init2 = 1; init2 < argc; init2++)
//			{
//			if (argv[init2][0] == '-')
//				{
//				switch(argv[init2][1])
//					{
//					case 'p':
//					case 'P':
//						if (strlen(argv[init2]) == 2)
//							padding = atof(argv[init2+1]);
//						else
//							padding = atof(&argv[init2][2]);
//						break;
//					case 'a':
//					case 'A':
//						if (strlen(argv[init2]) == 2)
//							strcpy(airfile, argv[init2+1]);
//						else
//							strcpy(airfile, &argv[init2][2]);
//						break;
//					case 'f':
//					case 'F':
//						ff = 1;
//						strcpy(fontfile, argv[init2+1]);
//						break;
//					case 'i':
//					case 'I':
//						strcpy(ipaddr, argv[init2+1]);
//						break;
//					default:
//						exit(0);
//						break;
//					}
//				}
//			}
//		}
   	sock = make_socket(PORT, ipaddr);
	if (sock == -1)
		notconnected = 1;
	reshape(WIDTH, HEIGHT);

	load_font(NULL);
/*
	texout = new fntRenderer();

	if (!ff)
		{
		for (init2 = 0; font_names[init2] != NULL; init2++)
			{
			strcpy(display_buffer, font_names[init2]);
			font_list[init2] = new fntTexFont(display_buffer,GL_NEAREST,GL_LINEAR_MIPMAP_LINEAR);
			}
		}
	else
		{
		font_list[0] = new fntTexFont(fontfile,GL_NEAREST,GL_LINEAR_MIPMAP_LINEAR);
		}
	texout->setFont(font_list[0]);
*/
	glEnableClientState(GL_VERTEX_ARRAY);
	firsttime = 1;
  	glutMainLoop();
   	return 0;   /* ANSI C requires main to return int. */
	}

char *split_eadi()
	{
	static int time = timeGetTime();
	static int oldtime;
	DWORD result;
    oldtime = timeGetTime() - time;

	all.converter_active = 0;
		// No sense getting this stuff in real-time.

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
			Data_Read(0x0AF4, sizeof(WORD),		&all.wFuelWeight,			&result);
			Data_Read(0x0894, 2,					&all.wEng1Combustion,			&result);
			Data_Read(0x092C, 2,					&all.wEng2Combustion,			&result);
			Data_Read(0x09C4, 2,					&all.wEng3Combustion,			&result);
			Data_Read(0x0A5C, 2,					&all.wEng4Combustion,			&result);
			Data_Read(0x0e8c, 2,					&all.wOutsideAirTemp,			&result);
			Data_Read(0x02C4, sizeof(DWORD),		&all.dwBarberPole,				&result);
			Data_Read(0x0f48, 2,				&all.wBarometricPressure,		&result);
			Data_Read(0x0E90, sizeof(WORD),   &all.wWindSpeed,				&result);
			Data_Read(0x0E92, sizeof(WORD),   &all.wWindDirection,			&result);
			Data_Read(0x3000, 6,				&all.vor1_id,					&result);
// request to read everything
		Data_Read(0x0330, 2,				&all.wAltimeter,				&result);
		Data_Read(0x0350, sizeof(WORD),	&all.wBcdNav1Freq,				&result);
		Data_Read(0x0366, 1,				&all.bPlaneOnGround,			&result);
		Data_Read(0x2ee0, sizeof(BOOL),	&all.FLIGHT_DIRECTOR_ACTIVE,	&result);
		Data_Read(0x07bc, 4,				&all.dwAPMaster,				&result);
		Data_Read(0x07c4, 4,				&all.dwAPNav1Hold,				&result);
		Data_Read(0x07c8, 4,				&all.dwAPHeadingHold,			&result);
		Data_Read(0x07d0, 4,				&all.dwAPAltitudeHold,			&result);
		Data_Read(0x07dc, 4,				&all.dwAPAirSpeedHold,			&result);
		Data_Read(0x07e4, 4,				&all.dwAPMachNumberHold,		&result);
		Data_Read(0x07fc, 4,				&all.dwAPGlideSlopeHold,		&result);
		Data_Read(0x0800, 4,				&all.dwAPLocalizerHold,			&result);
		Data_Read(0x0810, 4,				&all.dwAPAutoThrottle,			&result);
		Data_Read(0x0bac, 2,				&all.wInnerMarker,				&result);	
		Data_Read(0x0bae, 2,				&all.wMiddleMarker,				&result);	
		Data_Read(0x0bb0, 2,				&all.wOuterMarker,				&result);	
		Data_Read(0x0BDC, 4,				&all.dwFlapsCommanded,			&result);
		Data_Read(0x0AEC, 2,				&all.wEngines,					&result);		
		Data_Read(0x0c29, 5,				&all.sDME1, 					&result);
		Data_Read(0x0c48, 1,				&all.bLocalizerNeedle,			&result);
		Data_Read(0x0c49, 1,				&all.bGlideSlopeNeedle,			&result);
		Data_Read(0x036E, sizeof(BYTE),	&all.bSideSlip,					&result);
		Data_Read(0x07cc, 4,				&all.dwAPHeading,				&result);
		Data_Read(0x07e2, 2,				&all.wAPAirSpeed,				&result);
		Data_Read(0x07e8, 4,				&all.dwAPMachNumber,			&result);	
		Data_Read(0x07f2, 2,				&all.wAPVerticalSpeed,			&result);
		Data_Read(0x02c8, 4,				&all.dwVS,						&result);
		Data_Read(0x0842, 2,				&all.wVerticalSpeed,			&result);	
		Data_Read(0x2ee8, sizeof(FLOAT64),&all.FLIGHT_DIRECTOR_PITCH,		&result);
		Data_Read(0x2ef0, sizeof(FLOAT64),&all.FLIGHT_DIRECTOR_BANK,		&result);
		Data_Read(0x3300, sizeof(BOOL16),	&all.NAVAID_FLAGS,				&result);
		Data_Read(0x0BB2, sizeof(WORD),	&all.wElevator,					&result);
		Data_Read(0x0BB6, sizeof(WORD),	&all.wAileron,					&result);

		Data_Read(0x5600, 1,				&all.converter_active,			&result);
		Data_Read(0x5658, sizeof(FLOAT64),&all.fDimmer,					&result);

			Data_Read(0x02b8, 4,				&all.dwTAS,						&result);
			Data_Read(0x02bc, 4,				&all.dwIAS,						&result);
			Data_Read(0x02B4, 4,				&all.dwGS,						&result);
			Data_Read(0x0570, 4,				&all.dwAltLo,					&result);
			Data_Read(0x0574, 4,				&all.dwAltHi,					&result);
			Data_Read(0x0E90, sizeof(WORD),   &all.wWindSpeed,				&result);
			Data_Read(0x0E92, sizeof(WORD),   &all.wWindDirection,			&result);
			Data_Read(0x0580, 4,				&all.dwHeading,					&result);
			Data_Read(0x02a0, sizeof(WORD),	&all.wMagVar,					&result);
			Data_Read(0x0578, 4,				&all.dwPitch,					&result);
			Data_Read(0x057c, 4,				&all.dwBank,					&result);
			Data_Read(0x0870, sizeof(WORD),	&all.wILSOBS,					&result);
			Data_Read(0x0b4c, sizeof(WORD),	&all.wGroundAlt,				&result);
			Data_Read(0x07d4, 4,				&all.dwAPAltitude,				&result);
			Data_Read(0x6d0a, 1,					&all.byDimmer,				&result);

			dimmer = (double)all.byDimmer / 255.0;
			dimmer = 1.0;
// send the data request
		if (Data_Process(&result) == false)
		{
//			sprintf(display_buffer, "ERROR: %d", result);
//			MessageBox(0, display_buffer, "DEBUG", 0);
			Data_Close();
			notconnected = 1;
		}
// Conversions

	if (!all.converter_active)
		{
		vs = ( ( (double)(all.dwVS * 60) * 3.28084) / 256.0 );
// Altitude
		alt = ((double)all.dwAltHi +(double)all.dwAltLo / (double)0x100000000) * 3.2808399;
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
// ILS OBS
		ilsobs = ((double)all.wILSOBS*360.0)/(double)0x10000;
		ilsobs -= 180.0;
		if (ilsobs < 0.0)
			ilsobs*=-1.0;
// Radio Altimeter
		ra = (double)((long)(alt) - (long)(all.wGroundAlt * 3.2808399));
// Autopilot Altitude setting
		apalt = ((double)all.dwAPAltitude / (double)0x10000) * 3.2808399;
		}
	gs = (int)(((double)all.dwGS/65536.0)*1.943846);
	wd = 360.0*(double)all.wWindDirection/65536.0;

	return((char *)&all);
	}

/*
$Log$
*/
