/*======================================================================
** validations.cpp
** Copyright (c) 2005 Don Lafontaine
** ALL RIGHTS RESERVED
** By Don Lafontaine
**----------------------------------------------------------------------
**      Various field/page validation routines. 
**
** DATE					COMMENTS						WHO
**----------------------------------------------------------------------
** 2005/01				Added COmment header			Don
**====================================================================*/
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "Fs98State.h"
#include <main.h>
#include <prototypes.h>
#include "cdu_prototypes.h"
#include "cdu_defines.h"
#include "cdu_structs.h"
#include "cdu_externs.h"
#include "cdu_pages.h"
#include <defines.h>
#include "Flightplan.h"

// This function just checks to see if the scratchpad contains data, 
// if not it downselects the data to the scratchpad, otherwise it puts the scratchpad data into the variable (if not null)

int check_scratchpad(char *value, char *variable)
	{

	if (spidx == 0)
		{
		strcpy(scratchpad, value);
		spidx = strlen(value);
		return(0);
		}
	if (variable != NULL)
		{
		if (!strncmp(scratchpad, "DELETE", 6))
			strcpy(variable, "");
		else
			strcpy(variable, scratchpad);
		spidx = 0;
		memset(scratchpad, 0, 500);
		}

	return(1);
	}

int PreSelect_l(void)
	{
	if (!check_scratchpad(CDUPage[CurrentCP].LSKEntry[function-1].lskValue, DataHolder.presel_l))
		return(0);

	return(1);
	}

int PreSelect_r(void)
	{
	if (!check_scratchpad(CDUPage[CurrentCP].LSKEntry[function-1].lskValue, DataHolder.presel_r))
		return(0);

	return(1);
	}

int SetModel(void)
	{
	if (!check_scratchpad(CDUPage[CurrentCP].LSKEntry[function-1].lskValue, DataHolder.model))
		return(0);

	return(1);
	}

int SetSpeeds(void)
	{
	char *holder;

	if (!strcmp(CDUPage[CurrentCP].LSKEntry[function-1].lskTitle, "V1"))
		{
		holder = DataHolder.V1;
		}
	else if (!strcmp(CDUPage[CurrentCP].LSKEntry[function-1].lskTitle, "VR"))
		{
		holder = DataHolder.VR;
		}
	else if (!strcmp(CDUPage[CurrentCP].LSKEntry[function-1].lskTitle, "V2"))
		{
		holder = DataHolder.V2;
		}
	if (!check_scratchpad(CDUPage[CurrentCP].LSKEntry[function-1].lskValue, holder))
		return(0);

	return(1);
	}

int PrepEFIS(void)
	{
	strcpy(DataHolder.EFISState, DataHolder.EFISToggle?"{f2}{c1.0,1.0,1.0}OFF{sc}{sd}{f1}{c0.0,1.0,0.0}ON":"{f1}{c0.0,1.0,0.0}OFF{c1.0,1.0,1.0}{sc}{sd}{f2}ON"); 
	return(1);
	}

int PrepDSP(void)
	{
	strcpy(DataHolder.DSPState, DataHolder.DSPToggle?"{f2}{c1.0,1.0,1.0}OFF{sc}{sd}{f1}{c0.0,1.0,0.0}ON":"{f1}{c0.0,1.0,0.0}OFF{c1.0,1.0,1.0}{sc}{sd}{f2}ON"); 
	return(1);
	}

int PrepADFVOR(void)
	{
	switch(DataHolder.ADFVORSetting)
		{
		case 1:
			strcpy(DataHolder.ADFVORState, "{f2}OFF{sc}{sd}{f1}{c0.0,1.0,0.0}ADF{c1.0,1.0,1.0}{f2}{sc}{sd}VOR");
			break;
		case 2:
			strcpy(DataHolder.ADFVORState, "{f2}OFF{sc}{sd}ADF{sc}{sd}{f1}{c0.0,1.0,0.0}VOR{c1.0,1.0,1.0}{f2}");
			break;
		default:
			strcpy(DataHolder.ADFVORState, "{f1}{c0.0,1.0,0.0}OFF{c1.0,1.0,1.0}{f2}{sc}{sd}ADF{sc}{sd}VOR");
			break;
		}
	return(1);
	}

int SetDSPMode(void)
	{
	DataHolder.DSPMode = function-1;
	return(0);
	}

int ChkDSPMode(void)
	{
	char *prompt = "                       <SEL>";
	switch(DataHolder.DSPMode)
		{
		case 0:
				place_string(LEFT, 3, LARGE, prompt);
			break;
		case 1:
				place_string(LEFT, 5, LARGE, prompt);
			break;
		case 2:
				place_string(LEFT, 7, LARGE, prompt);
			break;
		}
	return(0);
	}

int SetSYNMode(void)
	{
	DataHolder.SYNMode = function-1;
	return(0);
	}

int ChkSYNMode(void)
	{
	char *prompt = "                       <SEL>";

	switch(DataHolder.SYNMode)
		{
		case 0:
				place_string(LEFT, 3, LARGE, prompt);
			break;
		case 1:
				place_string(LEFT, 5, LARGE, prompt);
			break;
		case 2:
				place_string(LEFT, 7, LARGE, prompt);
			break;
		}
	return(0);
	}

int SetADFVOR(void)
	{
	DataHolder.ADFVORSetting++;
	if (DataHolder.ADFVORSetting > 2)
		DataHolder.ADFVORSetting = 0;
	return(1);
	}

int CheckEFIS(void)
	{
	if (DataHolder.EFISToggle)
		page = EFISCTL;
	return(0);
	}

int CheckDSP(void)
	{
	if (DataHolder.DSPToggle)
		page = DISPMODE;
	return(0);
	}

int ToggleEFIS(void)
	{
	DataHolder.EFISToggle ^= 1;
	return(1);
	}

int ToggleDSP(void)
	{
	DataHolder.DSPToggle ^= 1;
	return(1);
	}

int navrad_preval(void)
	{
	sprintf(display_buffer, "%3.3s        %3.3s", DataHolder.rad_l, DataHolder.rad_r);
	place_string(CENTER, 4, SMALL, "RADIAL");
	place_string(CENTER, 5, LARGE, display_buffer);
	place_string(CENTER, 12, SMALL, "PRESELECT");
	if (DataHolder.presel_l[0] == '\0')
		place_string(LEFT, 13, LARGE, "------");
	if (DataHolder.presel_r[0] == '\0')
		place_string(RIGHT, 13, LARGE, "------");
	return(0);
	}

int GetFreq(void)
	{
	char t[10];
	int res;
	int oi;
	double freq;
	int idx = 0;
	
	if (scratchpad[0] == '{')
		idx = 4;

	if (isdigit(scratchpad[idx]))
		freq = atof(&scratchpad[idx]);
	else
		{
		DBGetNavaids(&scratchpad[idx]);

		oi = find_object(&scratchpad[idx]);
		if (oi == -1)
			{
			strcpy(scratchpad, "{c1.0,0.0,0.0}OBJECT NOT IN DATABASE");
			return(0);
			}
		freq = objects[oi].freq;
		}

	spidx = 0;
	memset(scratchpad, 0, 500);

	sprintf(t, "%04d", (int)ceil((freq-100.0)*100.0));
	res = (int)strtol(t, NULL, 16);
	return(res);
	}

int vor_lVal(void)
	{
	DWORD result;
	int res;

	if (!check_scratchpad(CDUPage[CurrentCP].LSKEntry[function-1].lskValue, NULL))
		return(0);

	res = GetFreq();
	if (res == 0)
		return(0);

	Data_Write(0x0350, 2, &res, &result);
	Data_Process(&result);
	return(0);
	}

int vor_rVal(void)
	{
	DWORD result;
	int res;

	if (!check_scratchpad(CDUPage[CurrentCP].LSKEntry[function-1].lskValue, NULL))
		return(0);

	res = GetFreq();

	if (res == 0)
		return(0);

	Data_Write(0x0352, 2, &res, &result);
	Data_Process(&result);
	return(0);
	}

int crs_lVal(void)
	{
	DWORD result;
	int crs;

	if (!check_scratchpad(CDUPage[CurrentCP].LSKEntry[function-1].lskValue, NULL))
		return(0);

	crs  = atoi(scratchpad);
	spidx = 0;
	memset(scratchpad, 0, 500);

	if (crs > 360) return(0);
	if (crs < 0) return(0);

	Data_Write(0x0c4e, 2, &crs, &result);
	Data_Process(&result);
	return(0);
	}

int crs_rVal(void)
	{
	DWORD result;
	int crs;

	if (!check_scratchpad(CDUPage[CurrentCP].LSKEntry[function-1].lskValue, NULL))
		return(0);

	crs = atoi(scratchpad);

	spidx = 0;
	memset(scratchpad, 0, 500);

	if (crs > 360) return(0);
	if (crs < 0) return(0);

	Data_Write(0x0c5e, 2, &crs, &result);
	Data_Process(&result);
	return(0);
	}

int SetEFISMode(void)
	{
	DWORD result;

	all.byNDRight &= ND_VORR | ND_ADFR | ND_STD;

	DataHolder.EFISMode = function-7;
	switch(DataHolder.EFISMode)
		{
		case 0:
			all.byNDRight |= ND_APP;
			break;
		case 1:
			all.byNDRight |= ND_VOR;
			break;
		case 2:
			all.byNDRight |= ND_MAP;
			break;
		case 3:
			all.byNDRight |= ND_PLAN;
			break;
		case 4:
			all.byNDRight |= ND_CTR;
			break;
		}
	Data_Write(0x6d02, 1,				&all.byNDRight,	&result);
	Data_Process(&result);
	return(0);
	}

int ChkEFISMode(void)
	{
	char *prompt = "<SEL>                       ";
	switch(DataHolder.EFISMode)
		{
		case 0:
				place_string(RIGHT, 3, LARGE, prompt);
			break;
		case 1:
				place_string(RIGHT, 5, LARGE, prompt);
			break;
		case 2:
				place_string(RIGHT, 7, LARGE, prompt);
			break;
		case 3:
				place_string(RIGHT, 9, LARGE, prompt);
			break;
		case 4:
				place_string(RIGHT, 11, LARGE, prompt);
			break;
		}
	return(0);
	}

void SetRange()
	{
	DWORD result;

	all.byNDRange &= ND_HPA;

	switch(DataHolder.NDRangeSet)
		{
		case 0:
			strcpy(DataHolder.NDRange, "10 NM");
			all.byNDRange ^= ND_R10;
			break;
		case 1:
			strcpy(DataHolder.NDRange, "20 NM");
			all.byNDRange ^= ND_R20;
			break;
		case 2:
			strcpy(DataHolder.NDRange, "40 NM");
			all.byNDRange ^= ND_R40;
			break;
		case 3:
			strcpy(DataHolder.NDRange, "80 NM");
			all.byNDRange ^= ND_R80;
			break;
		case 4:
			strcpy(DataHolder.NDRange, "160 NM");
			all.byNDRange ^= ND_R160;
			break;
		case 5:
			strcpy(DataHolder.NDRange, "320 NM");
			all.byNDRange ^= ND_R320;
			break;
		case 6:
			strcpy(DataHolder.NDRange, "640 NM");
			all.byNDRange ^= ND_R640;
			break;
		}
	Data_Write(0x6d03, 1,				&all.byNDRange,	&result);
	Data_Process(&result);
	}

int SetEFISOpt(void)
	{
	DWORD result;

//	all.byNDEFIS = 0;
	all.byNDEFIS ^= CDUPage[CurrentCP].LSKEntry[function-1].NewPage;
	Data_Write(0x6d00, 1,				&all.byNDEFIS,	&result);
	Data_Process(&result);
	return(0);
	}

int IncrRange(void)
	{
	DataHolder.NDRangeSet++;
	if (DataHolder.NDRangeSet > 6)
		DataHolder.NDRangeSet = 6;

	SetRange();
	return(0);
	}

int DecrRange(void)
	{
	DataHolder.NDRangeSet--;
	if (DataHolder.NDRangeSet < 0)
		DataHolder.NDRangeSet = 0;

	SetRange();
	return(0);
	}

int ChkPosPage(void)
	{
	if (SubPage > 2)
		SubPage = 2;
	sprintf(display_buffer, "(%d/3)", SubPage+1);
	place_string(RIGHT, 1, SMALL, display_buffer);
	switch(SubPage)
		{
		case 0:
			page = POS;
			break;
		case 1:
			page = POS2;
			break;
		case 2:
			page = POS3;
			break;
		}
	return(0);
	}

int GetAirport(void)
	{
	char lat[20];
	char lon[20];

	if (!check_scratchpad(CDUPage[CurrentCP].LSKEntry[function-1].lskValue, NULL))
		return(0);

	DBGetAirport(scratchpad);

	if (Waypoints == 0)
		{
		strcpy(scratchpad, "AIRPORT NOT IN DATABASE");
		spidx = VERROR;
		}
	else
		{
		strcpy(CDUPage[CurrentCP].LSKEntry[function-1].lskValue, scratchpad);
		spidx = 0;
		memset(scratchpad, 0, 500);
		DecToDMS(WaypointList[0].lat, lat, LATITUDE);
		DecToDMS(WaypointList[0].lon, lon, LONGITUDE);
		sprintf(DataHolder.AirportPos, "%s%s", lat,lon); 
		}

	return(0);
	}

int GetOrigin(void)
	{
	if (!check_scratchpad(CDUPage[CurrentCP].LSKEntry[function-1].lskValue, NULL))
		return(0);

	if (!DBGetOriginAirport(scratchpad))
		{
		strcpy(scratchpad, "AIRPORT NOT IN DATABASE");
		spidx = VERROR;
		}
	else
		{
		spidx = 0;
		memset(scratchpad, 0, 500);

		strcpy(fpEP.Origin, OrigAirport.ICAO);
		fpEP.OrigLat = OrigAirport.lat;
		fpEP.OrigLon = OrigAirport.lon;
		}

	return(0);
	}

int ValidateRunway(void)
	{
	if (!check_scratchpad(CDUPage[CurrentCP].LSKEntry[function-1].lskValue, NULL))
		return(0);

	if (DBValidateRunway(scratchpad))
		{
		strcpy(DataHolder.OrigRWY, scratchpad);
		spidx = 0;
		memset(scratchpad, 0, 500);
		}
	else
		{
		strcpy(scratchpad, "INVALID RUNWAY FOR AIRPORT");
		spidx = VERROR;
		}
	}

int GetDestination(void)
	{
	if (!check_scratchpad(CDUPage[CurrentCP].LSKEntry[function-1].lskValue, NULL))
		return(0);

	if (!DBGetDestinationAirport(scratchpad))
		{
		strcpy(scratchpad, "AIRPORT NOT IN DATABASE");
		spidx = VERROR;
		}
	else
		{
		spidx = 0;
		memset(scratchpad, 0, 500);

		strcpy(fpEP.Destination, DestAirport.ICAO);
		fpEP.DestLat = DestAirport.lat;
		fpEP.DestLon = DestAirport.lon;
		}

	return(0);
	}

int PrepDepArr(void)
	{
	int init=0;
	Sids = 0;
	Stars = 0;
	
	while (strcmp(Sid[init++].trm_ident, ""));
	if (strcmp(fpEP.Origin, ""))
		{
		sprintf(tmpPageTitle, "%s DEPARTURES", fpEP.Origin);
		if (!strcmp(DAProc[0],""))
			DBGetDepartures(fpEP.Origin);
		if ((SubPage+1) > (int)(ceil((double)init/5.0)))
			SubPage--;
		sprintf(display_buffer, "%d/%d", SubPage+1, (int)(ceil((double)init/5.0)));
		place_string(RIGHT, 1, SMALL, display_buffer);

		strcpy(DAProc[0], 
			strcmp(Sid[(SubPage*5)].trm_ident, "") == 0?"":strtok(Sid[(SubPage*5)].trm_ident, " \0"));
		strcpy(DAProc[1], 
			strcmp(Sid[(SubPage*5)+1].trm_ident, "") == 0?"":strtok(Sid[(SubPage*5)+1].trm_ident, " \0"));
		strcpy(DAProc[2], 
			strcmp(Sid[(SubPage*5)+2].trm_ident, "") == 0?"":strtok(Sid[(SubPage*5)+2].trm_ident, " \0"));
		strcpy(DAProc[3], 
			strcmp(Sid[(SubPage*5)+3].trm_ident, "") == 0?"":strtok(Sid[(SubPage*5)+3].trm_ident, " \0"));
		strcpy(DAProc[4], 
			strcmp(Sid[(SubPage*5)+4].trm_ident, "") == 0?"":strtok(Sid[(SubPage*5)+4].trm_ident, " \0"));

		if (DBValidateRunway(NULL))
			{
			strcpy(DARunway[0], Runways[(SubPage*5)].ident);
			strcpy(DARunway[1], Runways[(SubPage*5)+1].ident);
			strcpy(DARunway[2], Runways[(SubPage*5)+2].ident);
			strcpy(DARunway[3], Runways[(SubPage*5)+3].ident);
			strcpy(DARunway[4], Runways[(SubPage*5)+4].ident);
			}
		}
	else if (strcmp(fpEP.Destination, ""))
		{
		sprintf(tmpPageTitle, "%s ARRIVALS", fpEP.Destination);
		if (!strcmp(DAProc[0],""))
			DBGetArrivals(fpEP.Destination);
		}
	else
		strcpy(tmpPageTitle, "DEP / ARR INDEX");

	return(0);
	}

int GetTransitions(void)
	{
//	DBGetTransitions(CDUPage[CurrentCP].LSKEntry[function-1].lskValue);

	return(0);
	}

int SelectProc(void)
	{
	char Procedure[100];
	strcpy(Procedure, CDUPage[CurrentCP].LSKEntry[function-1].lskValue);

	return(0);
	}

int SelectRunway(void)
	{
	char Runway[100];
	strcpy(Runway, CDUPage[CurrentCP].LSKEntry[function-1].lskValue);

	if (!strcmp(Runway, ""))
		return(0);

	strcpy(DataHolder.OrigRWY, Runway);
	return(1);
	}
