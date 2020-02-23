/*======================================================================
** Page definitions
** Copyright (c) 2005 Don Lafontaine
** ALL RIGHTS RESERVED
** By Don Lafontaine
**----------------------------------------------------------------------
**      This file contains everything needed to display CDU Pages.  
**
** DATE					COMMENTS						WHO
**----------------------------------------------------------------------
** 2005/01				Added COmment header			Don
**====================================================================*/
#include <defines.h>

// Page names
#define IDENT		0
#define INIT		1
#define INDEX		2
#define MAINT		3
//#define POS			4
#define PERF		5
#define TAKEOFF		6
#define APPROACH	7
#define NAVDATA		8
#define POLICY		9
#define MONITOR		10
#define RTELEGS		11
#define MENU		12
#define EFISCTL		13
#define EFISOPT		14
#define DISPMODE	15
#define DISPSYN		16
#define FMCCOMM		17
#define DEPARR		18
#define ROUTE		19
#define DATALINK	20
#define THRUSTLIM	21
#define NAVRAD		22
#define CROSSLOAD	23
#define PERFFACTR	24
#define SENSORS		25
#define DISCRETES	26
#define IRCMONITOR	27
#define POS			28
#define POS2		29
#define POS3		30

#define PROMPT		1
#define VALUE		2

// Include function prototypes included in these structures


#ifdef CDU_MAIN
// -----------------------------------------------------------------------------------
// Select pages used for cases where multiple waypoints are found for a certain name 
// -----------------------------------------------------------------------------------
int SubPage;				// Used for multi-page displays (2/3 etc..)
char SelectLookingFor[20];  // Waypoint name to list
int SelectReturnTo;			// Page to return to with the result
long SelectPointer;			// Index into objects for the selected waypoint
// ------------------------
// Current index markers
// ------------------------
int CurrentCP = 0;
int CurrentLSK = 0;
// ------------------
// Main data holder
// ------------------
DH DataHolder;
EP fpEP;

FP *ActiveFlightPlan;		// Active Flight Plan list.
FP *ActiveFPFirst;
FP *ActiveFPLast;

FP *FlightPlan;				// MOD Flight Plan list;
FP *FPFirst;
FP *FPLast;

int Sids = 0;
int Stars = 0;
SS Sid[100];
SS Star[100];
char DAProc[50][255];
char DARunway[50][255];
char tmpPageTitle[80];

CP CDUPage[] =
{
// IDENT -------------------------------------------------------------------------------------------
        {1.0, 1.0, 1.0, IDENT, 15, NULL, "IDENT", NULL,
		{
		{"MODEL",			DataHolder.model,		NULL,			&SetModel,		0},
		{"NAV DATA",		DataHolder.navdata,		NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{"OP PROGRAM",		DataHolder.program,		NULL,			NULL,			0},
		{"DRAG FACTOR",		DataHolder.drag,		NULL,			NULL,			0},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{"ENG RATING",		DataHolder.eng_rating,	NULL,			NULL,			0},
		{"ACTIVE",			DataHolder.active,		NULL,			NULL,			0},
		{NULL,				DataHolder.inactive,	NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{"F-F FACTOR",		DataHolder.fuelflow,	NULL,			NULL,			0},
		{NULL,				"POS INIT>",			NULL,			&SetPage,		POS}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// INIT --------------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, INIT, 0, NULL, "INIT/REF INDEX", NULL,
		{
		{NULL,				"<IDENT",				NULL,			&SetPage,		IDENT},
		{NULL,				"<POS",					NULL,			&SetPage,		POS},
		{NULL,				"<PERF",				NULL,			&SetPage,		PERF},
		{NULL,				"<TAKEOFF",				NULL,			&SetPage,		TAKEOFF},
		{NULL,				"<APPROACH",			NULL,			&SetPage,		APPROACH},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"NAV DATA>",			NULL,			&SetPage,		NAVDATA},
		{NULL,				"DATA LINK>",			NULL,			&SetPage,		DATALINK},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"MAINT>",				NULL,			&SetPage,		MAINT}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// DATALINK ----------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, DATALINK, 15, NULL, "DATA LINK",NULL,
		{
		{"ROUTE",			"<REQUEST",				NULL,			NULL,			0},
		{"WIND DATA",		"<REQUEST",				NULL,			NULL,			0},
		{"PERF INIT",		"<REQUEST",				NULL,			NULL,			0},
		{"TAKEOFF DATA",	"<REQUEST",				NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{"CO ROUTE",		DataHolder.coroute,		NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{"POS/RWY",			DataHolder.posrwy,		NULL,			NULL,			0},
		{"DATA LINK",		"READY",				NULL,			NULL,			0},
		{NULL,				"PERF INIT>",			NULL,			&SetPage,		PERF}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// PERF INIT ----------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, PERF, 15, NULL, "PERF INIT",NULL,
		{
		{"GROSS WT",		NULL,					NULL,			NULL,			0},
		{"FUEL",			NULL,					NULL,			NULL,			0},
		{"ZFW",				NULL,					NULL,			NULL,			0},
		{"RESERVES",		NULL,					NULL,			NULL,			0},
		{"COST INDEX",		NULL,					NULL,			NULL,			0},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{"CRZ ALT",			NULL,					NULL,			NULL,			0},
		{"CRZ WIND",		NULL,					NULL,			NULL,			0},
		{"ISA DEV",			NULL,					NULL,			NULL,			0},
		{"T/C OAT",			NULL,					NULL,			NULL,			0},
		{"TRANS ALT",		NULL,					NULL,			NULL,			0},
		{NULL,				"TAKEOFF>",				NULL,			&SetPage,		TAKEOFF}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// POS/INIT (Page 1) ----------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, POS, 15, &ChkPosPage, "POS INIT", NULL,
		{
		{NULL,				NULL,					NULL,			NULL,			0},
		{"REF AIRPORT",		DataHolder.airport,		NULL,			&GetAirport,	0},
		{"GATE",			DataHolder.gate,		NULL,			NULL,			0},
		{"GMT",				DataHolder.time,		NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{"LAST POS",		DataHolder.lastpos,		NULL,			NULL,			0},
		{NULL,				DataHolder.AirportPos,	NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{"GPS POS",			DataHolder.gpspos,		NULL,			NULL,			0},
		{"SET IRS POS",		DataHolder.irspos,		NULL,			NULL,			0},
		{NULL,				"ROUTE>",				NULL,			&SetPage,		ROUTE}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// POS/INIT (Page 2) ----------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, POS2, 13, &ChkPosPage, "POS REF", NULL,
		{
		{"FMS POS (GPS)",	DataHolder.gpspos,		NULL,			NULL,			0},
		{"IRS (3)",			DataHolder.gpspos,		NULL,			NULL,			0},
		{"RNP / ACTUAL",	"TODO",					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"<PURGE",				NULL,			NULL,			0},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{"GS",				DataHolder.gs,			NULL,			NULL,			0},
		{NULL,				DataHolder.gs,			NULL,			NULL,			0},
		{"DME-DME",			"TODO",					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"INHIBIT>",				NULL,			NULL,			0},
		{NULL,				"BRG/DIST>",			NULL,			NULL,			0}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// POS/INIT (Page 3) ----------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, POS3, 13, &ChkPosPage, "POS REF", NULL,
		{
		{"IRS L",			DataHolder.gpspos,		NULL,			NULL,			0},
		{"IRS C",			DataHolder.gpspos,		NULL,			NULL,			0},
		{"IRS R",			DataHolder.gpspos,		NULL,			NULL,			0},
		{"GPS/IRS",			DataHolder.gpspos,		NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{"GS",				DataHolder.gs,			NULL,			NULL,			0},
		{NULL,				DataHolder.gs,			NULL,			NULL,			0},
		{NULL,				DataHolder.gs,			NULL,			NULL,			0},
		{NULL,				DataHolder.gs,			NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"BRG/DIST>",			NULL,			NULL,			0}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// TAKEOFF REF -------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, TAKEOFF, 15, NULL, "TAKEOFF REF", NULL,
		{
		{"TEMP SEL",		DataHolder.to_temp,		NULL,			NULL,			0},
		{"TO N1",			DataHolder.to_n1,		NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"<POS INIT",			NULL,			&SetPage,		POS},
		{NULL,				"<PERF INIT",			NULL,			&SetPage,		PERF},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{"V1",				DataHolder.V1,			NULL,			&SetSpeeds,		0},
		{"VR",				DataHolder.VR,			NULL,			&SetSpeeds,		0},
		{"V2",				DataHolder.V2,			NULL,			&SetSpeeds,		0},
		{NULL,				"ROUTE>",				NULL,			&SetPage,		ROUTE},
		{NULL,				"DEPARTURE>",			NULL,			&SetPage,		DEPARR},
		{NULL,				NULL,					NULL,			NULL,			0}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// THRUST LIM --------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, THRUSTLIM, 15, NULL, "THRUST LIM", NULL,
		{
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"<GA",					NULL,			NULL,			0},
		{NULL,				"<CON",					NULL,			NULL,			0},
		{NULL,				"<CRZ",					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{"CLB N1",			DataHolder.clb_n1,		NULL,			NULL,			0},
		{NULL,				"CLB>",					NULL,			NULL,			0},
		{NULL,				"CLB 1>",				NULL,			NULL,			0},
		{NULL,				"CMB 2>",				NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"APPROACH>",			NULL,			NULL,			0}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// NAV RADIO ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, NAVRAD, 0, navrad_preval, "NAV RADIO", NULL,
		{
		{"VOR L",			DataHolder.vor_l,		NULL,			&vor_lVal,		0},
		{"CRS",				DataHolder.crs_l,		NULL,			&crs_lVal,		0},
		{"ADF L",			DataHolder.adf_l,		NULL,			NULL,			0},
		{"ILS - MLS",		DataHolder.ils_mls,		NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				DataHolder.presel_l,	NULL,			&PreSelect_l,	0},
		{"VOR R",			DataHolder.vor_r,		NULL,			&vor_rVal,		0},
		{"CRS",				DataHolder.crs_r,		NULL,			&crs_rVal,		0},
		{"ADF R",			DataHolder.adf_r,		NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				DataHolder.presel_r,	NULL,			&PreSelect_r,	0}
		},
		{
		{1.0, 1.0, 1.0, 0.0, 1.0, 0.0}, {1.0, 1.0, 1.0, 0.0, 1.0, 0.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 0.0, 1.0, 0.0}, {1.0, 1.0, 1.0, 0.0, 1.0, 0.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// MAINT ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, MAINT, 0, NULL, "MAINTENANCE INDEX", NULL,
		{
		{NULL,				"<CROSS LOAD",			NULL,			NULL,			0},
		{NULL,				"<SENSORS",				NULL,			NULL,			0},
		{NULL,	 			"<DISCRETES",			NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{NULL,				"PERF FACTR>",			NULL,			&SetPage,		PERFFACTR},
		{NULL,				"IRS MONITOR>",			NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"POLICY>",				NULL,			&SetPage,		POLICY}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// POLICY ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, POLICY, 15, NULL, "AIRLINE POLICY", NULL,
		{
		{"ACCEL HT",		"1500",					NULL,			NULL,			0},
		{"MNVR MARGIN",		"1.30",					NULL,			NULL,			0},
		{"MIN CRZ TIME",	"1",					NULL,			NULL,			0},
		{"CO SPD",			"300/.800",				NULL,			NULL,			0},
		{"SPD TRANS",		"250/10000",			NULL,			NULL,			0},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{"OPTION CODE",		"0000000028",			NULL,			NULL,			0},
		{"R/C CLB",			"100",					NULL,			NULL,			0},
		{"R/C CRZ",			"0",					NULL,			NULL,			0},
		{"CO CRZ THR",		"CLB",					NULL,			NULL,			0},
		{"SPD RESTR",		"000/0",				NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// PERFFACTR ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, PERFFACTR, 15, NULL, "PERF FACTORS", NULL,
		{
		{"PERF CODE",		"1110",					NULL,			NULL,			0},
		{"DRAG/F-F",		"+0.0/+0.0",			NULL,			NULL,			0},
		{"TO1/TO2",			"-5/-15",				NULL,			NULL,			0},
		{"MNVR MARGIN",		"1.20",					NULL,			NULL,			0},
		{"MIN CRZ TIME",	"1",					NULL,			NULL,			0},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{"OPTION CODE",		"000001004806",			NULL,			NULL,			0},
		{"R/C CLB",			"100",					NULL,			NULL,			0},
		{"THR/CRZ",			"CRZ/100",				NULL,			NULL,			0},
		{"THR RED",			"1500",					NULL,			NULL,			0},
		{"ACCEL HT",		"1500/1500",			NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// ROUTE ---------------------------------------------------------------------------------------
//		{1.0, 1.0, 1.0, ROUTE, NULL, "RTE 1", NULL,
//		{
//		{"ORIGIN",			"1110",					NULL,			NULL,			0},
//		{"RUNWAY",			"22R",					NULL,			NULL,			0},
//		{"ROUTE",			"-------",				NULL,			NULL,			0},
//		{NULL,				NULL,					NULL,			NULL,			0},
//		{NULL,				NULL,					NULL,			NULL,			0},
//		{NULL,				"<RTE 2",				NULL,			NULL,			0},
//		{"DEST",			"0000",					NULL,			NULL,			0},
//		{"FLT NO",			"100",					NULL,			NULL,			0},
//		{"CO ROUTE",		"-------",				NULL,			NULL,			0},
//		{NULL,				NULL,					NULL,			NULL,			0},
//		{NULL,				"ALTN>",				NULL,			NULL,			0},
//		{NULL,				"ACTIVATE>",			NULL,			NULL,			0}
//		},
//		{
//		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
//		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
//		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
//		}
//		},
// MENU ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, MENU, 0, NULL, "MENU", NULL,
		{
		{NULL,				"<FMC",					NULL,			NULL,			0},
		{NULL,				"<SAT",					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"<CAB INT",				NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{"EFIS CTL",		DataHolder.EFISState,	&PrepEFIS,		&ToggleEFIS,	0},
		{NULL,				"EFIS>",				NULL,			&CheckEFIS,		0},
		{"DSP CTL",			DataHolder.DSPState,	&PrepDSP,		&ToggleDSP,		0},
		{NULL,				"DSP>",					NULL,			&CheckDSP,			0},
		{"MAINT INFO",		"DISPLAY>",				NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// FMCCOMM ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, FMCCOMM, 0, NULL, "FMC COMM", NULL,
		{
		{"UPLINK",			"<RTE 1",				NULL,			NULL,			0},
		{"UPLINK (2)",		"<ALTN",				NULL,			NULL,			0},
		{NULL,				"<PERF",				NULL,			NULL,			0},
		{NULL,				"<TAKEOFF",				NULL,			NULL,			0},
		{NULL,				"<WIND",				NULL,			NULL,			0},
		{NULL,				"<DES FORECAST",		NULL,			NULL,			0},
		{NULL,				"POS REPORT>",			NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{"DATA LINK",		"READY",				NULL,			NULL,			0}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// EFISCTL ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, EFISCTL, 0, NULL, "EFIS CONTROL", NULL,
		{
		{"BARO SET",		DataHolder.baro,		NULL,			NULL,			0},
		{"RAD / BARO SET",	DataHolder.minrefsel,	NULL,			NULL,			0},
		{"MINS SET",		DataHolder.mins,		NULL,			NULL,			0},
		{NULL,				"<MINS RESET",			NULL,			NULL,			0},
		{NULL,				"<RANGE INCR",			NULL,			&IncrRange,		0},
		{DataHolder.NDRange,"<RANGE DECR",			NULL,			&DecrRange,		0},
		{"MODE",			"APP>",					&ChkEFISMode,	&SetEFISMode,	0},
		{NULL,				"VOR>",					&ChkEFISMode,	&SetEFISMode,	0},
		{NULL,				"MAP>",					&ChkEFISMode,	&SetEFISMode,	0},
		{NULL,				"PLN>",					&ChkEFISMode,	&SetEFISMode,	0},
		{NULL,				"CTR>",					&ChkEFISMode,	&SetEFISMode,	0},
		{"{f2}_ _ _ _ _ _ _ _ _ _",	"OPTIONS>",		NULL,			&SetPage,		EFISOPT}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// EFISOPT ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, EFISOPT, 15, NULL, "EFIS OPTIONS", NULL,
		{
		{NULL,				"<WXR",					NULL,			&SetEFISOpt,	ND_WXR},
		{NULL,				"<STA",					NULL,			&SetEFISOpt,	ND_STA},
		{NULL,				"<WPT",					NULL,			&SetEFISOpt,	ND_WPT},
		{NULL,				"<ARPT",				NULL,			&SetEFISOpt,	ND_ARPT},
		{NULL,				"<DATA",				NULL,			&SetEFISOpt,	ND_DATA},
		{NULL,				"<POS",					NULL,			&SetEFISOpt,	ND_POS},
		{NULL,				"FPV>",					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"MTRS>",				NULL,			NULL,			0},
		{NULL,				"TFC>",					NULL,			&SetEFISOpt,	ND_TFC},
		{"SEL ADF / VOR",	DataHolder.ADFVORState,	PrepADFVOR,		SetADFVOR,		0},
		{NULL,				"CONTROL>",				NULL,			&SetPage,		EFISCTL}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// DISPMODE ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, DISPMODE, 15, NULL, "DISPLAY MODES", NULL,
		{
		{"SEL DISPLAY",		"<L INBRD",				&ChkDSPMode,	&SetDSPMode,	0},
		{NULL,				"<LWR CTR",				&ChkDSPMode,	&SetDSPMode,	0},
		{NULL,				"<R INBRD",				&ChkDSPMode,	&SetDSPMode,	0},
		{"EICAS",			"<ENG",					NULL,			NULL,			0},
		{NULL,				"<STAT",				NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{"MODE",			"CHKL>",				NULL,			NULL,			0},
		{NULL,				"COMM>",				NULL,			NULL,			0},
		{NULL,				"NAV>",					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"CANCEL/RCL>",			NULL,			NULL,			0},
		{NULL,				"SYNOPTICS>",			NULL,			&SetPage,		DISPSYN}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// DISPSYN ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, DISPSYN, 15, NULL, "DISPLAY SYNOPTICS", NULL,
		{
		{"SEL DISPLAY",		"<L INBRD",				&ChkSYNMode,	&SetSYNMode,	0},
		{NULL,				"<LWR CTR",				&ChkSYNMode,	&SetSYNMode,	0},
		{NULL,				"<R INBRD",				&ChkSYNMode,	&SetSYNMode,	0},
		{NULL,				"<DOOR",				NULL,			NULL,			0},
		{NULL,				"<GEAR",				NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"ELEC>",				NULL,			NULL,			0},
		{NULL,				"HYD>",					NULL,			NULL,			0},
		{NULL,				"FUEL>",				NULL,			NULL,			0},
		{NULL,				"AIR>",					NULL,			NULL,			0},
		{NULL,				"FCTL>",				NULL,			NULL,			0},
		{NULL,				"MODES>",				NULL,			&SetPage,		DISPMODE}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// APPROACH ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, APPROACH, 15, NULL, "APPROACH REF", NULL,
		{
		{"GROSS WT",		"<L INBRD",				NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{"QNH LANDING",		"<SELECT QFE",			NULL,			NULL,			0},
		{"TO DO",			"TO DO 2",				NULL,			NULL,			0},
		{"TO DO 3",			"TO DO 4",				NULL,			NULL,			0},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{"VREF",			"To Do",				NULL,			NULL,			0},
		{NULL,				"To Do",				NULL,			NULL,			0},
		{NULL,				"To Do",				NULL,			NULL,			0},
		{"FLAP / SPEED",	"_ _ / _ _ _",			NULL,			NULL,			0},
		{"FRONT COURSE",	"To Do",				NULL,			NULL,			0},
		{NULL,				"THRUST LIM>",			NULL,			&SetPage,		THRUSTLIM}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// ROUTE ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, ROUTE, 15, NULL, "RTE 1 (Variable)", NULL,
		{
		{"ORIGIN",			fpEP.Origin,			NULL,			&GetOrigin,		0},
		{"CO ROUTE",		DataHolder.coroute,		NULL,			NULL,			0},
		{"RUNWAY",			DataHolder.OrigRWY,		NULL,			&ValidateRunway,0},
		{"VIA",				"via 1",				NULL,			NULL,			0},
		{NULL,				"via 2",				NULL,			NULL,			0},
		{NULL,				"<RTE 2",				NULL,			NULL,			0},
		{"DEST",			fpEP.Destination,		NULL,			&GetDestination,0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{"TO",				"to 1",					NULL,			NULL,			0},
		{NULL,				"to 2",					NULL,			NULL,			0},
		{NULL,				"PERF INIT>",			NULL,			&SetPage,		PERF}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// DEPARR ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, DEPARR, 0, PrepDepArr, tmpPageTitle, NULL,
		{
		{"SIDS",			DAProc[0],				NULL,			&SelectProc,	0},
		{NULL,				DAProc[1],				NULL,			&SelectProc,	0},
		{NULL,				DAProc[2],				NULL,			&SelectProc,	0},
		{NULL,				DAProc[3],				NULL,			&SelectProc,	0},
		{NULL,				DAProc[4],				NULL,			&SelectProc,	0},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{"RUNWAYS",			DARunway[0],			NULL,			&SelectRunway,	0},
		{NULL,				DARunway[1],			NULL,			&SelectRunway,	0},
		{NULL,				DARunway[2],			NULL,			&SelectRunway,	0},
		{NULL,				DARunway[3],			NULL,			&SelectRunway,	0},
		{NULL,				DARunway[4],			NULL,			&SelectRunway,	0},
		{NULL,				"ROUTE>",				NULL,			&SetPage,		ROUTE}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// NAV DATA ---------------------------------------------------------------------------------------
		{1.0, 1.0, 1.0, NAVDATA, 0, NULL, "REF NAV DATA", NULL,
		{
		{"IDENT",			NULL,					NULL,			NULL,			0},
		{"LATITUDE",		NULL,					NULL,			NULL,			0},
		{"MAG VAR",			NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"<INDEX",				NULL,			&SetPage,		INIT},
		{"FREQ",			NULL,					NULL,			NULL,			0},
		{"LONGITUDE",		NULL,					NULL,			NULL,			0},
		{"ELEVATION",		NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				NULL,					NULL,			NULL,			0},
		{NULL,				"INHIBIT>",				NULL,			NULL,			0}
		},
		{
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}
		},
// LAST ENTRY, DO NOT REMOVE =======================================================================
		{1.0, 1.0, 1.0, 9999, 0, NULL, NULL, NULL,	{
		{NULL,	NULL,	NULL,	NULL,	0},	{NULL,	NULL,	NULL,	NULL,	0},	{NULL,	NULL,	NULL,	NULL,	0},	{NULL,	NULL,	NULL,	NULL,	0},
		{NULL,	NULL,	NULL,	NULL,	0},	{NULL,	NULL,	NULL,	NULL,	0},	{NULL,	NULL,	NULL,	NULL,	0},	{NULL,	NULL,	NULL,	NULL,	0},
		{NULL,	NULL,	NULL,	NULL,	0},	{NULL,	NULL,	NULL,	NULL,	0},	{NULL,	NULL,	NULL,	NULL,	0},	{NULL,	NULL,	NULL,	NULL,	0}
		},
		{{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		}}
};

#else
extern int SubPage;
extern char SelectLookingFor[];		// Waypoint name to list
extern int SelectReturnTo;			// Page to return to with the result
extern long SelectPointer;			// Index into objects for the selected waypoint
extern int CurrentCP;
extern int CurrentLSK;
extern DH DataHolder;
extern EP fpEP;
extern CP CDUPage[];
extern int Sids;
extern int Stars;
extern SS Sid[];
extern SS Star[];

extern FP *FlightPlan;
extern FP *FPFirst;
extern FP *FPLast;
extern FP *ActiveFlightPlan;		// Active Flight Plan list.
extern FP *ActiveFPFirst;
extern FP *ActiveFPLast;

extern char tmpPageTitle[80];
extern char DAProc[50][255];
extern char DARunway[50][255];
#endif