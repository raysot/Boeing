#include <windows.h>
#include <stdio.h>
#include <math.h>
// OpenGL 
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <Fs98State.h>
#include <defaults.h>
#define EXTERN
#include <structs.h>
#include "nd.h"
#include <fonts.h>
#include <prototypes.h>

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


extern int 				waypoints;			// For flightplan
extern int					currentwpt;			// For flightplan
int 				refresh = 1;			// For flightplan
extern int 			frames;					// For framerate calculation
extern FS98StateData 	all;					// Structure for all the FSUIPC data
extern int				mapmode;			// Map mode settings (MAP/PLAN/ILS/VOR)
extern char *		display_buffer;
extern void 		setColor(double r, double g, double b);
extern int				lnavactive;			// LNAV autopilot toggle
extern int				trackmode;			// TRK mode toggle
extern double			wca;					// Wind correction Angle
extern long				total_objs;				// Number of navaids
extern int 				arcmode;			// Arc/Rose mode toggle
extern double				coslat, coslon;			//
extern double				sinlat, sinlon;			//
extern double				acossinlat;				//
extern double				distance;				// Used to calculate Map Position
extern double 				lat, lon;				// General Purpose variables for great circle math
extern double				tdist, tbear;			// 
extern double				rlat, rlon;				//
extern double				bearing;				// Ditto
extern double 				magvar;					// Magnetic variation.
extern double				groundspeed;		// 
extern double 				hdg;					// Heading
extern int 					range;				// Scope range
extern double 				compassradius;			// Compass Radius for ARC and ROSE mode

int fpwpt(char *st)
{
//	int init;

	return(0);
/*
	if (!strcmp(flightplan[currentwpt].id, st))
		return(1);

	for (init = 0; init < waypoints; init++)
		if (!strcmp(flightplan[init].id, st))
			return(-1);
	return(0);
*/
}

// *************************************************
// This function updates the next waypoint display 
// and corrects the autopilot if vnav is active
// *************************************************
int check_lnav()
{
	unsigned long	result;
	static int		delay = 0;
	double			aphdg;

	if (waypoints == 0)
		return(0);
// Are we close enough to skip to the next waypoint?
	if (flightplan[currentwpt].distance < 3.0 && (!all.bPlaneOnGround))
		currentwpt ++;

// Whoops, went beyond the end of the flight plan
	if (currentwpt == waypoints)
		currentwpt--;

// Display the next waypoint,  ETA and range to the next waypoint in the top 
// right of the window.  Only valid in MAP and PLAN mode.
	if ((mapmode == MAPMODE) || (mapmode == PLANMODE))
	{
		setColor(1,0,1);
		Puts(65, 84, 4.0, flightplan[currentwpt].id, 0);
		setColor(1,1,1);
		Puts(65, 80, 4.0, flightplan[currentwpt].seta, 0);
		sprintf(display_buffer, "%0.1f", flightplan[currentwpt].distance);
		strcat(display_buffer, " NM");
		Puts(65, 76, 4.0, display_buffer, 0);
	}

	delay--;
	aphdg = floor((360.0*(double)all.dwAPHeading)/((double)65536));

	// If we're the active LNAV AP,  send the updates to the simulator
	if (lnavactive)
	{      
		if (trackmode)
			all.dwAPHeading = (DWORD)((flightplan[currentwpt].bearing-wca) * 0x10000 / 360.0);
		else
			all.dwAPHeading = (DWORD)(flightplan[currentwpt].bearing * 0x10000 / 360.0);
		if (delay<0)
		{
			Data_Write(0x07cc, sizeof(DWORD),	&all.dwAPHeading,		&result);
			Data_Process(&result);
			delay = frames;
		}
	}
	return(1);
}

// ***************************************************************
// This function looks up a flight plan navaid in the database
// ***************************************************************
int find_waypoint(char *id, double *lat, double *lon)
{
	long init;

	for (init = 0; init < total_objs; init++)
	{
		if (!strcmp(objects[init].id, id))
		{
			*lat = objects[init].lat;
			*lon = objects[init].lon;
			return(1);
		}
	}
	return(0);
}

// ******************************************
// Read a squawkbox flightplan and/or update
// the ETA's
// ******************************************
int read_sbp_flightplan(void)
{
	double			eta;
	double			etah;
	double			navrange;
	double			x,y;
	FILE			*fp;
	double			flightplanv[2000];
	double			latitude, longitude;
	char 			planbuf[100];
	char			origin[5];
	char			destination[5];
	double			dest_lat = 0.0;
	double			dest_lon = 0.0;
	char			*token;
	int				init;
	int				init2;
//	double			rp = (ranges[abs(range-5)] / ranges[0]) * 100.0;
//	int				r = rp/6;
	
	if (arcmode)
	{
		glPushMatrix();
		glTranslatef(0.0, -70.0, 0.0);
	}
	if ( refresh )
	{
		fp = fopen("flightplan.sbp", "r");

		if (fp == NULL)
		{
			refresh = 0;
			return(0);
		}

		fgets(planbuf, 99, fp);
		token = strtok(planbuf, ":");
		if (token == NULL)
		{
			waypoints = 0;
			MessageBox(0, "Error in flight plan origin, plan ignored", "FreeFD ND Flight Plan", 0);
			return(0);
		}
		strcpy(origin, token);

		waypoints = 0;
		if (find_waypoint(origin, &latitude, &longitude))
		{
			//     Set the first waypoint to origin
			strcpy(flightplan[waypoints].id, origin);
			flightplan[waypoints].lat = latitude;
			flightplan[waypoints].latrad = d2r(latitude);
			flightplan[waypoints].sinlat = sin(flightplan[waypoints].latrad);
			flightplan[waypoints].coslat = cos(flightplan[waypoints].latrad);
			flightplan[waypoints].lon = longitude;
			flightplan[waypoints].lonrad = d2r(longitude);
			flightplan[waypoints].sinlon = sin(flightplan[waypoints].lonrad);
			flightplan[waypoints].coslon = cos(flightplan[waypoints].lonrad);
			waypoints++;
		}

		token = strtok(NULL, ":");
		if (token == NULL)
		{
			waypoints = 0;
			MessageBox(0, "Error in flight plan destination! Flight plan ignored", "FreeFD ND Flight Plan", 0);
			return(0);
		}
		strcpy(destination, token);

		if (find_waypoint(destination, &latitude, &longitude))
		{
			dest_lat = latitude;
			dest_lon = longitude;
		}

		while (fgets(planbuf, 99, fp))
		{
			if (strlen(planbuf) < 10)
				continue;
			token = strtok(planbuf, ":");
			token = strtok(NULL, ":");
			if (token == NULL)
			{
				waypoints = 0;
				MessageBox(0, "Error in flight plan waypoint, Plan ignored", "FreeFD ND Flight Plan", 0);
				return(0);
			}
			strcpy(flightplan[waypoints].id, token);
			for (init = 0; init < (int)strlen(flightplan[waypoints].id); init++)
				if (flightplan[waypoints].id[init] == ' ')
				{
					flightplan[waypoints].id[init] = '\0';
					break;
				}
			token = strtok(NULL, ":");
			if (token == NULL)
			{
				waypoints = 0;
				MessageBox(0, "Error in flight plan waypoint, Plan ignored", "FreeFD ND Flight Plan", 0);
				return(0);
			}
			flightplan[waypoints].lat = atof(token);
			token = strtok(NULL, ":");
			if (token == NULL)
			{
				waypoints = 0;
				MessageBox(0, "Error in flight plan waypoint, Plan ignored", "FreeFD ND Flight Plan", 0);
				return(0);
			}
			flightplan[waypoints].lon = atof(token);
			flightplan[waypoints].latrad = d2r(flightplan[waypoints].lat);
			flightplan[waypoints].sinlat = sin(flightplan[waypoints].latrad);
			flightplan[waypoints].coslat = cos(flightplan[waypoints].latrad);
			flightplan[waypoints].lonrad = d2r(flightplan[waypoints].lon);
			flightplan[waypoints].sinlon = sin(flightplan[waypoints].lonrad);
			flightplan[waypoints].coslon = cos(flightplan[waypoints].lonrad);
			waypoints++;
		}
		strcpy(flightplan[waypoints].id, destination);
		flightplan[waypoints].lat = dest_lat;
		flightplan[waypoints].lon = dest_lon;
		flightplan[waypoints].latrad = d2r(flightplan[waypoints].lat);
		flightplan[waypoints].sinlat = sin(flightplan[waypoints].latrad);
		flightplan[waypoints].coslat = cos(flightplan[waypoints].latrad);
		flightplan[waypoints].lonrad = d2r(flightplan[waypoints].lon);
		flightplan[waypoints].sinlon = sin(flightplan[waypoints].lonrad);
		flightplan[waypoints].coslon = cos(flightplan[waypoints].lonrad);
		waypoints++;
		fclose(fp);
		refresh  =0;
	}

// Update the flight plan eta's and draw the flight plan
	init2 = 0;
	for (init = 0; init < waypoints; init++)
	{
		distance = acos(sinlat * flightplan[init].sinlat + coslat * 
			flightplan[init].coslat * cos(d2r(lon)-flightplan[init].lonrad));
		if (sin(flightplan[init].lonrad - d2r(lon)) < 0)
			bearing = acos((flightplan[init].sinlat - sinlat * cos(distance)) / 
						(sin(distance) * coslat));
		else
			bearing = 2.0 * MYPI - acos((flightplan[init].sinlat - sinlat * cos(distance)) / 
						(sin(distance) * coslat)); 
		distance *= ((180.0*60.0)/MYPI);
		bearing = (360.0-r2d(bearing))+magvar;
		flightplan[init].bearing = bearing;
		flightplan[init].distance = distance;

// Calculate ETA for each waypoint.------
		eta = distance / groundspeed;
		eta = (eta*60.0)+(double)all.byZuluMin;
		etah = (double)all.byZuluHour + floor(eta/60.0);
		if (floor(eta/60.0)) 
			eta = eta - (floor(eta/60.0)*60.0);
		
		if (etah >= 24)
			flightplan[init].etahour = etah-24;
		else
			flightplan[init].etahour = etah;

		flightplan[init].etamin = eta;
		if ((flightplan[init].etahour < 10.0) && (flightplan[init].etamin < 10.0))
			sprintf(flightplan[init].seta, "0%.0f0%.1fZ", flightplan[init].etahour, flightplan[init].etamin);
		else if (flightplan[init].etahour < 10.0)
			sprintf(flightplan[init].seta, "0%.0f%.1fZ", flightplan[init].etahour, flightplan[init].etamin);
		else if (flightplan[init].etamin < 10.0)
			sprintf(flightplan[init].seta, "%.0f0%.1fZ", flightplan[init].etahour, flightplan[init].etamin);
		else 
			sprintf(flightplan[init].seta, "%.0f%.1fZ", flightplan[init].etahour, flightplan[init].etamin);

// Draw the flight plan route
		bearing = fabs(hdg) - bearing;
		bearing -= 270.0;
		if (bearing < 0.0)
			bearing += 360.0;
		navrange = distance / (rangemarkers[range]*2.0);
		navrange *= compassradius;
		x = cos((MYPI / 180.0) * bearing) * navrange;
		y = sin((MYPI / 180.0) * bearing) * navrange;
// Add this waypoint to the vector array
		flightplanv[init2++] = x;
		flightplanv[init2++] = y;
		setColor(1.0, 1.0, 1.0);
		if (init == currentwpt)
			setColor(1,0,1);
		sprintf(display_buffer, "    %s", flightplan[init].id);
		Puts(x, y, 3.0, display_buffer,0);
		sprintf(display_buffer, "    %s", flightplan[init].seta);
		Puts(x, y-4, 3.0, display_buffer,0);
		CPuts(x, y-1.8, 6.0, "r");  // Display the star
	}
// All done,  now draw the vector array.
	setColor(1.0, 0.0, 1.0);		
	glLineWidth(1.0);
	glVertexPointer(2, GL_DOUBLE, 0, flightplanv);
	glDrawArrays(GL_LINE_STRIP, 0, init2/2);
	if (arcmode)
		glPopMatrix();
	setColor(1,1,1);
	return(1);
}
