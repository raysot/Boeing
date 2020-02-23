/*
	FLIGHTPLAN STORAGE
*/


typedef struct _wp
	{
	char wptid[20];		// Waypoint ID
	char type;			// VIR/NDB/FIX/MANUAL
	double lat;			// Latitude
	double lon;			// Longitude
	char freq[10];		// Frequency
	long alt;			// Altitude constraint
	int course;			// Course to/from
	char InOutBound;	// In/Outbound direction
	} WPT;


#ifdef CDU_MAIN
WPT WaypointList[50]; // Enough space for possible duplicates?
int Waypoints;
char fpActive = 0;
char fpModified = 0;
#else
extern int Waypoints;
extern WPT WaypointList[];
extern char fpActive;
extern char fpModified;
#endif
