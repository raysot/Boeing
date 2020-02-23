/*======================================================================
** Database.cpp
** Copyright (c) 1997-2004 Don Lafontaine
** Copyright (c) 2005- Ellie Systems
** ALL RIGHTS RESERVED
** By Don Lafontaine
**----------------------------------------------------------------------
**      Database support routines. 
**
** DATE					COMMENTS						WHO
**----------------------------------------------------------------------
** 2005/01				Added COmment header			Don
** 2005/05				Converted database code to 
**						SQLAPI.							Don
**====================================================================*/
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Flightplan.h"
#include "cdu_defines.h"

#define DB_MODULE 1
#include "cdu_structs.h"

#include "cdu_pages.h"
#include <SQLAPI.h>

SAConnection con;
char error_buffer[100];

int OpenDatabase()
	{

	try
		{
		con.Connect("c:\\public\\DAFIF.GDB", "sysdba", "masterkey", SA_InterBase_Client);
		}
	catch(SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		}

	return(0);

	}

int CloseDatabase()
	{
	try
		{
		con.Disconnect();
		}
	catch(SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		}
	return(0);
	}


int DBGetNavaids (char *name)
{
	char sel_str[200];
	int num = 0;
	int rows = 0;

	sprintf(sel_str, "select NAV_IDENT,'TYPE',WGS_DLAT,WGS_DLONG,FREQ from NAV \
					 where NAV_IDENT='%s' and 'TYPE'!='R';", name);

	SACommand cmd(&con, sel_str);

	try
		{
		cmd.Execute();
		}
	catch(SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		}

	rows = cmd.FieldCount();

	Waypoints = 0;
	try
		{
		while (cmd.FetchNext())
			{
			strcpy(WaypointList[Waypoints].wptid, (const char*)cmd.Field("nav_ident").asString());
			WaypointList[Waypoints].type = cmd.Field("\"type\"").asString()[0];
			WaypointList[Waypoints].lat = cmd.Field("wgs_dlat").asDouble();
			WaypointList[Waypoints].lon = cmd.Field("wgs_dlong").asDouble();
			strcpy(WaypointList[Waypoints].freq, (const char*)cmd.Field("frequency").asString());
			Waypoints++;
			num++;
			}
		}
	catch (SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		}

    return(0);
}            

int DBGetOriginAirport(char *name)
{
	char sel_str[200];
	int rows = 0;

	sprintf(sel_str, "select ARPT_IDENT,WGS_DLAT,WGS_DLONG from ARPT \
					 where (ICAO='%s') or (FAA_HOST_ID='%s');", name, name);

	SACommand cmd(&con, sel_str);

	try
		{
		cmd.Execute();
		}
	catch(SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		return(0);
		}

	strcpy(OrigAirport.ICAO,name);

	Waypoints = 0;
	try
		{
		while (cmd.FetchNext())
			{
			rows++;
			strcpy(OrigAirport.ID,cmd.Field("arpt_ident").asString());
			OrigAirport.lat = cmd.Field("wgs_dlat").asDouble();
			OrigAirport.lon = cmd.Field("wgs_dlong").asDouble();
			}
		}
	catch (SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		return(0);
		}

	if (rows)
	    return(1);
	else
		return(0);
}

DBValidateRunway(char *name)
{
	char sel_str[200];
	int rows = 0;
	struct _rwy 
	{
		char low[10];
		char high[10];
		double lat,lon;
	} rwy;
	int retcode = 0;
	int rwys = 0;

	sprintf(sel_str, "select HIGH_IDENT,LOW_IDENT from RWY \
					 where ARPT_IDENT='%s';", OrigAirport.ID);

	SACommand cmd(&con, sel_str);

	try
		{
		cmd.Execute();
		}
	catch(SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		return(0);
		}

	try
		{
		while (cmd.FetchNext())
			{
			strcpy(rwy.low, cmd.Field("low_ident").asString());
			strcpy(rwy.high, cmd.Field("high_ident").asString());
			strcpy(Runways[rwys++].ident, rwy.low);
			strcpy(Runways[rwys++].ident, rwy.high);
			if (name != NULL)
				{
				if ((strcmp(rwy.low, name) == 0) || (strcmp(rwy.high, name) == 0))
					retcode = 1;
				}
			rows++;
			}
		}
	catch (SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		return(0);
		}

	if (name == NULL)
		retcode = 1;
	return(retcode);
}

int DBGetDestinationAirport(char *name)
{
	char sel_str[200];
	int rows = 0;

	sprintf(sel_str, "select ARPT_IDENT,WGS_DLAT,WGS_DLONG from ARPT \
					 where (ICAO='%s') or (FAA_HOST_ID='%s');", name, name);

	SACommand cmd(&con, sel_str);

	try
		{
		cmd.Execute();
		}
	catch(SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		return(0);
		}

	strcpy(DestAirport.ICAO,name);

	Waypoints = 0;
	try
		{
		while (cmd.FetchNext())
			{
			rows++;
			strcpy(DestAirport.ID,cmd.Field("arpt_ident").asString());
			DestAirport.lat = cmd.Field("wgs_dlat").asDouble();
			DestAirport.lon = cmd.Field("wgs_dlong").asDouble();
			}
		}
	catch (SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		return(0);
		}

	if (rows)
		return(1);
	else
		return(0);
}

int DBGetAirport (char *name)
{
	char sel_str[200];
	int num = 0;
	int rows = 0;

	sprintf(sel_str, "select ARPT_IDENT,WGS_DLAT,WGS_DLONG from ARPT \
					 where (ICAO='%s') or (FAA_HOST_ID='%s');", name, name);

	SACommand cmd(&con, sel_str);

	try
		{
		cmd.Execute();
		}
	catch(SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		}

	rows = cmd.FieldCount();
	if (rows == 0)
		return(0);

	Waypoints = 0;
	try
		{
		while (cmd.FetchNext())
			{
			strcpy(DestAirport.ID,cmd.Field("arpt_ident").asString());
			WaypointList[Waypoints].lat = cmd.Field("wgs_dlat").asDouble();
			WaypointList[Waypoints].lon = cmd.Field("wgs_dlong").asDouble();
			Waypoints++;
			num++;
			}
		}
	catch (SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		}

    return(1);
}            

int DBGetDepartures (char *name)
	{
	char sel_str[200];
	int num = 0;
	int rows = 0;

	sprintf(sel_str, "select DISTINCT TRM_IDENT,PROC from TRM_SEG where ICAO=\'%s\' and PROC=\'1\';", name);

	SACommand cmd(&con, sel_str);

	try
		{
		cmd.Execute();
		}
	catch(SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		}

	rows = cmd.FieldCount();
	if (rows == 0)
		return(0);

	Waypoints = 0;
	try
		{
		while (cmd.FetchNext())
			{
			strcpy(Sid[Waypoints].trm_ident, cmd.Field("trm_ident").asString());
			Waypoints++;
			num++;
			}
		}
	catch (SAException &x)
		{
        sprintf(error_buffer, "%s\n", (const char*)x.ErrText());
		MessageBox(0, error_buffer, "Database ERROR", MB_OK);
		}

    return(1);
	}

int DBGetArrivals (char *name)
	{
	return(0);
	}
