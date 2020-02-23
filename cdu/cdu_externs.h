#if CDU_MAIN
extern int				direct;
extern int				noserver;
extern double			dimmer;
extern int				frames;
#else
extern double			screen_left, screen_right, screen_top, screen_bottom;
extern int				monochrome;
extern int				window;
extern int				width; 
extern int				height;
extern char				display_buffer[];
extern int				refresh;		// Make sure that when we switch displays that we have the required data.
extern FS98StateData	all;
extern double			version;
extern double			WindowSize;
extern int				sock;
extern int				function;
extern int				spidx;			// Current index into scratchpad buffer
extern char				scratchpad[];	// Holds the scratchpad data
extern int				valid_keys;		// Holds valid LSK keys (OR'd together based on the following defines)
extern int				page;
extern double			totfuel;
extern double			lat,lon;
extern OBJECTS			objects[];
extern int				navaids;
extern long				total_objs;
#endif
