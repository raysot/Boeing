// $Id$
/*----------------------------------------------------------------------------
$Log$
----------------------------------------------------------------------------*/
#define APT 0
#define ILS 1
#define RWY 2
#define OMI 3
#define NDB 4
#define VOR 5
#define FIX 6

#ifndef EXTERN

// These are roughly 10, 20, 40, 60, 80, 160, and 320 nm

double ranges[7];
double rangemarkers[7] = {5.0, 10.0, 20.0, 40.0, 80.0, 160.0, 320.0};
OBJECTS objects[200000];
FLIGHTPLAN flightplan[2000];

#else

// These are roughly 10, 20, 40, 60, 80, 160, and 320 nm

extern double ranges[7];
extern double rangemarkers[7];
extern OBJECTS objects[200000];
extern FLIGHTPLAN flightplan[2000];

#endif
