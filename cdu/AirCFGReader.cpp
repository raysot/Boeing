/*======================================================================
** AirCFGReader
** Copyright (c) 2005 Don Lafontaine
** ALL RIGHTS RESERVED
** By Don Lafontaine
**----------------------------------------------------------------------
**      Reading of the Aircraft CFG file. May no longer be needed
**
** DATE					COMMENTS						WHO
**----------------------------------------------------------------------
** 2005/01				Added comment header			Don
**====================================================================*/
#include <windows.h>
#include <Fs98State.h>
#include "cdu_structs.h"
#define CDU_MAIN 0
#include "cdu_externs.h"
#include "cdu_prototypes.h"
#include <string.h>
#include <stdio.h>

int ReadAircraftCFG()
{
	FILE *fh;
	char buffer[1000];
	char *token;
	char fullpath[1000];
	strcpy(fullpath, all.szFSPath);
	strcat(fullpath, all.szAirfileName);

	fh = fopen(fullpath, "r");
	if (fh == NULL)
	{
//		MessageBox(0, fullpath, "DEBUG: Can't open aircraft.cfg file", 0);
		return(0);
	}
	while (fgets(buffer, 999, fh))
	{
		token = strtok(buffer, " =\r\n\0");
		if (token == NULL)
			continue;
		if (!stricmp(token, "max_gross_weight"))
		{
			token = strtok(NULL, " =\r\n\0");
			if (token == NULL)
				continue;
//			sprintf(perfinit.max_grwt, "%.1f", atof(token)/1000.0);
		}
		if (!stricmp(token, "empty_weight"))
		{
			token = strtok(NULL, " =\r\n\0");
			if (token == NULL)
				continue;
//			sprintf(perfinit.zfw, "%.1f", atof(token)/1000.0);
		}
		else if (!stricmp(token, "static_thrust"))
		{
			token = strtok(NULL, " =\r\n\0");
			if (token == NULL)
				continue;
//			strcpy(air_data.static_thrust, token);
		}
		else if (!stricmp(token, "drag_scalar"))
		{
			token = strtok(NULL, " =\r\n\0");
			if (token == NULL)
				continue;
//			strcpy(ident.drag_ff, token);
		}
		else if (!stricmp(token, "ui_type"))   // This is the aircraft model number
		{
			token = strtok(NULL, " =\r\n\0");
			if (token == NULL)
				continue;
//			strcpy(ident.model, token);
		}
		
	}
	fclose(fh);
	return(1);
}
