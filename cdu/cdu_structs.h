// Structures for the CDU

//#include "cdu_pages.h"

//General aircraft modelling data
typedef struct ad
{
	char static_thrust[20];
} AIRCRAFTDATA;
	
typedef struct objtag
{
	char type;
	char id[7];
//	char name[20];
	char rwy[5];

// lat = Latitude, sinlat = sin(latitude), 
// coslat = cos(latitude), latrad = d2r(latitude)
	double  lat;
	double  sinlat;
	double  coslat;
	double  latrad;
	double  acossinlat;

// lon = Longitude, sinlon = sin(longitude),
// coslon = cos(longitude), lonrad = d2r(longitude)
	double  lon;
	double  sinlon;
	double  coslon;
	double  lonrad;

//	long alt;
	double freq;
//	long range;
//	int magvar;
	int obs;
	int length;
} OBJECTS;

typedef struct ep
	{
	char Origin[10];
	double OrigLat;
	double OrigLon;
	char Runway[10];
	double RwyLat;
	double RwyLon;
	char Destination[10];
	double DestLat;
	double DestLon;
	char Direction;							// E (East) or W (West)
	} EP;									// Endpoints (Airports)

typedef struct fp
	{
	char wptid[20];
	double latitude;
	double longitude;
	int dme;								// Distance from this waypoint
	int bearing;							// Bearing from this waypoint
	int alt;								// Crossing alt 
	int spd;								// Crossing speed
	BOOL active;							// true if executed.

	struct fp *Prev;
	struct fp *Next;	
	} FP, *FPP;

typedef struct ss							// Temporary holding spot for Sids and Stars
	{
	char trm_ident[20];						// Procedure name
	char transition[20];					// Transition
	double lat;								// lat/lon of waypoint
	double lon;
	double distance;						// DME
	double bearing;							// Heading to/from wpt
	char waypoint[20];						// Waypoint id.
	} SS;									// SID/STARs

typedef struct dh
	{
	char time[10];
	char model[20];
	char navdata[20];
	char program[20];
	char drag[10];
	char eng_rating[20];
	char active[20];
	char inactive[20];
	char fuelflow[10];
	char coroute[20];
	char posrwy[20];
	char airport[10];
	char gate[10];
	char lastpos[20];
	char AirportPos[30];
	char gpspos[20];
	char irspos[20];
	char to_temp[10];
	char to_n1[10];
	char V1[5];
	char VR[5];
	char V2[5];
	char clb_n1[10];
	char presel_l[20];
	char presel_r[20];
	char gs[20];
	char vor_l[50];
	char crs_l[50];
	char rad_l[50];
	char rad_r[50];
	char vor_r[50];
	char crs_r[50];
	char adf_l[50];
	char adf_r[50];
	char ils_mls[50];
	int DSPToggle;
	int EFISToggle;
	char EFISState[100];
	char DSPState[100];
	char baro[20];							// Barometric reference selector
	char minrefsel[100];					// Minimums Reference Select mode
	char NDRange[100];						// Range setting
	int NDRangeSet;
	char mins[100];
	int	ADFVORSetting;
	char ADFVORState[100];
	int DSPMode;
	int SYNMode;
	int EFISMode;
	char OrigRWY[10];
	} DH;

typedef struct lc
	{
	double tr,tg,tb;						// Title Colors
	double vr,vg,vb;						// Value Colors
	} LC;

typedef struct le
	{
	char *lskTitle;                         // Title to display over the LSK button line
	char *lskValue;                         // Value to display in the LSK button line (Could be a prompt EG: ACTIVATE >)
	int (*lskPreValidate)(void);            // Pre-validation routine to call
	int (*lskSelectFunc)(void);				// Routine to call when LSK pressed
	int NewPage;							// If this LSK takes you to a new page, this is it.
	} LE;

typedef struct cp
	{
	double tr,tg,tb;						// Title Colors
	int Page;                               // Page number
	int Dashes;								// Where is the dashed line.
	int (*PagePreValidate)(void);           // Page Pre-validation routine
	char *PageTitle;                        // Page title
	int (*PagePostValidate)(void);          // Page Post-validation
	LE LSKEntry[12];                        // Array of 12 LSK structures as described above
	LC LSKColor[12];						// Array of color settings
	} CP;

typedef struct apt
	{
	char ICAO[10];
	char ID[10];
	double lat;
	double lon;
	} _APT;

typedef struct rwy
	{
	char ident[10];
	double lat;
	double lon;
	} _RWY;

#ifdef DB_MODULE
_RWY Runways[25];
_APT DestAirport,OrigAirport;
#else
extern _RWY Runways[25];
extern _APT DestAirport,OrigAirport;		// Airports
#endif
