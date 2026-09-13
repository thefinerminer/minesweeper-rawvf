/*****************************************************************
 Original script by Maksim Bashov 2012-10-23.
 
 Modified during 2019-02 by Damien Moore. Split Program string to extract Version 
 and added Time, 3BV, 3BVS, Timestamp, Style, Questionmarks and Status. On 2019-02-24 
 fixed method for printing mouse event times. On 2019-03-04 made program backwards 
 compatible with Release 2.2 and earlier versions that have a shorter header.
 
 Modified 2020-01-25 by Damien Moore as Release 2 beta and earlier versions do not have 
 3bv, 3bvs or Timestamp values. Tidied code and wrote detailed comments. On 2020-02-03 
 fixed a major bug that caused Solved 3BV (in RAWVF2RAWVF) to be incorrect in certain
 videos. This bug occurred when using event[cur++] in the function used to create the
 missing first left click (since Viennasweeper does not record mouse event prior to the
 time starting and it is the release of the button that starts the timer). This version
 is being released as Viennasweeper (RMV) RAW version 6.
 
 Additional update 2021-05-24 per Tommy to add 3 new fields (nick, country, token) for 
 Viennasweeper 3.1. 
 
 Additional update 2023-04-14 to add loop to find last timed event. In most videos this
 is 3rd or 4th last event but sometimes more events need to be checked. Version 6.1.
 
 Additional update 2026-09-13 to parse Viennasweeper 5. VS5 uses the RMV2 specification.
 Information previously stored in the header is now also in variables (such as file type,
 version, timestamp). Skins can implement any cell size (this parser checks and scales
 all games to the default 16 pixel cell size and scales mouse movement). The new extended
 properties section stores keybind information and skin details and allows unlimited other
 custom variables. RMV2 also stores mouse moves relative to the top left corner instead of
 absolute distance from top left game window and it stores moves in a "reduced" format
 (relative to previous position thus fewer bytes but ints need to be signed). Released as
 Version 6.2.
 
 Works on all known RMV versions but not earlier UMF files.
 
*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXREP 100000
#define MAXNAME 1000

struct event
{
	int time;
	int x,y;
	unsigned char event;
};
typedef struct event event;

FILE* RMV;

//Initialise global variables
int mode,level,w,h,m;				//Mode, Level, Width, Height, Mines
int size;							//Number of game events
int* board;							//Stores board and mine locations
int qm,nf;							//Questionmarks, No Flags
int square_size=16;					//Default cell size
char name[MAXNAME];					//Player name
char nick[MAXNAME];					//Player nickname
char country[MAXNAME];				//Player country
char token[MAXNAME];				//Secret key (used in Scoreganizer tournaments)
char program[MAXNAME];				//Program name
char version[MAXNAME];				//Version name
char verstring[MAXNAME];			//Version string with all available information
char verlength[MAXNAME];			//Version string length
event video[MAXREP];				//Game events
int score;							//Score
int score_check;					//Check score matches time of last game event
char bbbv[MAXNAME];					//3bv as a string
int bbbvint;						//3bv as an integer
int bbbvs; 							//3bvs during calculations
float bbbvs_final;					//3bvs with decimals

//RMV1 specific
char timestamp[MAXNAME];			//Timestamp is stored in a string

//RMV2 specific
int timestamp_boardgen;				//Timestamp is stored in four bytes (little-endian)
int file_type;						//Variable to distinguish RMV1 and RMV2
char ext_prop_name[2][MAXNAME];		//Array with up to 2 property names (increase if needed)
char ext_prop_value[2][MAXNAME];	//Array with up to 2 property values (increase if needed)
int keybind=0;						//Boolean for using keybinds
char skin_name[MAXNAME];			//Skin file name

//==============================================================================================
//Function asks user to exit after program has run successfully in a terminal
//==============================================================================================
void pause()
{
	//fprintf(stderr,"Press enter to exit\n");
	//while(getchar()!='\n');
}


//==============================================================================================
//Function to print error messages
//==============================================================================================
void error(const char* msg)
{
	fprintf(stderr,"%s\n",msg);
	pause();
	exit(1);
}


//==============================================================================================
//Function is run if there is a parsing error
//==============================================================================================
int _fgetc(FILE* f)
{
	if(!feof(f)) return fgetc(f); else
	{
	error("Error 4: Unexpected end of file");
	}
}


//==============================================================================================
//Functions to read either 2, 3 or 4 bytes at a time as an integer
//==============================================================================================
int get2bytes(FILE* f)
{
	//Read 2 bytes into a string variable then return as int
	unsigned char c[2];
	c[0]=_fgetc(f);c[1]=_fgetc(f);
	//Return ends the function and returns output to variable get2bytes
	//We require a little-endian int with length two bytes so flip the order of the bytes
	//Multiplying by 256 is the same as shifting left by 1 byte (8 bits)
	return (int)c[1]+c[0]*256;
}

int get3bytes(FILE* f)
{
	//Read 3 bytes into a string variable then return as int
	//Function is used for mouse event times (a design decision to save one byte per event)
	unsigned char c[3];
	c[0]=_fgetc(f);c[1]=_fgetc(f);c[2]=_fgetc(f);
	return (int)c[2]+c[1]*256+c[0]*65536;
}

int get4bytes(FILE* f)
{
	//Read 4 bytes into a string variable then return as int	
	//Function used for fs (file size), vidsize (video size) and timestamp_boardgen (board creation timestamp)
	unsigned char c[4];
	int i;
	for(i=0;i<4;++i) c[i]=_fgetc(f);
	return (int)c[3]+c[2]*256+c[1]*65536+c[0]*16777216;
}


//==============================================================================================
//Function is used to print game events
//==============================================================================================
void print_event(event* e)
{
	const char* event_names[]={"","mv","lc","lr","rc","rr","mc","mr","","pressed","pressedqm","closed",
		"questionmark","flag","blast","lost","won","nonstandard","number0","number1","number2","number3",
		"number4","number5","number6","number7","number8","blast","reduced"};
	unsigned char c=e->event;
	
	//Mouse event	
	if(c<=7)
	{
		printf("%d.%03d %s %d %d (%d %d)\n",
			//Split time into before and after decimal place
			e->time/1000,e->time%1000,
			//Obtain event name from event array
			event_names[c],
			//Calculate column and row
			e->x/square_size+1,e->y/square_size+1,			
			//Optional re-basing of mouse movements to standard square size
			//Factor of 10000 removes issues with losing decimal places when using integer calculations
			e->x*10000/square_size*16/10000,e->y*10000/square_size*16/10000);						
	}
	//Board event	
	else if(c<=14 || (c>=18 && c<=27)) 
	{
		printf("%s %d %d\n",
			event_names[c],
			e->x,e->y);
	}
	//RMV2 uses a reduced mouse move that captures movement relative to prior move
	else if(c==28 && file_type==2)
	{
		printf("%d.%03d %s %d %d\n",
			e->time/1000,e->time%1000,
			event_names[c],
			e->x*10000/square_size*16/10000,e->y*10000/square_size*16/10000);					
	}	
	//Game ending event
	else if(c<=17)
	{
		printf("%s\n",
			event_names[c]);
	}
}


//==============================================================================================
//Function is used to fetch Time and Status
//==============================================================================================
void print_event2(event* e)
{	
	const char* event_names[]={"","mv","lc","lr","rc","rr","mc","mr","","pressed","pressedqm","closed",
		"questionmark","flag","blast","lost","won","nonstandard","number0","number1","number2","number3",
		"number4","number5","number6","number7","number8","blast","reduced"};
	unsigned char c=e->event;
	
	//Mouse event
	if(c<=7 || c==28)
	{
	//Put time of click into score variable
	score=e->time;
	}
	else {score=0;}

	//Win or lose status
	if(c==16||c==15)
	{
	printf("%s\n",event_names[c]);
	}
}


//==============================================================================================
//Function is used to read video data
//==============================================================================================
int readrmv()
{	
	//Initialise local variables	
	int i,j,cur=0;
	unsigned char c,d;
	const char* header_1="*rmv";
	int fs;							//File size
	int version_info_size; 			//Length of header starting at "Viennasweeper"
	int player_info_size; 			//Length of header starting at "NAME"
	int board_size;					//Number of bytes containing the board layout and board creation timestamp
	int preflags_size;				//Number of bytes containing the location of flags placed before game started
	int properties_size;			//Number of bytes containing game properties
	int vid_size;					//Number of bytes containing video events
	int cs_size;					//Number of bytes containing checksum
	int num_player_info;			//Number of bytes containing player information
	int name_length;				//Number of bytes containing player name
	int nick_length;				//Number of bytes containing player nick
	int country_length;				//Number of bytes containing player country
	int token_length;				//Number of bytes containing secret key
	int num_preflags;				//Number of bytes containing flags placed before starting game
	int is_first_event=1;			//Default value when checking for events before timer starts
	
	//RMV1 specific
	int result_string_size=0; 		//Default length header starting at "LEVEL" (not applicable to RMV2)
	
	//RMV2 specific
	int clone_id; 					//Viennasweeper=1, Arbiter=2, MSX=3, Clone=4	
	int clone_major_version; 		//Viennasweeper 5.0.0a2 would be major version 5
	int ext_properties_size; 		//Length of extended properties array	
	int num_extension_properties;	//Number of extended properties
	int ext_prop_name_size;			//Length of current extended property name (used while iterating array)
	int ext_prop_value_size;		//Length of current extended property value (used while iterating array)
	
	//Check file signature is *rmv
	for(i=0;i<4;++i) if(c=_fgetc(RMV)!=header_1[i]) error("No RMV header");

	//Check file type is RMV1 or RMV2
	int file_type = get2bytes(RMV);	
	if(file_type == 0 || file_type > 2) error("Invalid video type");	

	//Clone details
	if(file_type == 2)
	{
		clone_id=_fgetc(RMV);
		clone_major_version=_fgetc(RMV);
	}
	
	//File size
	fs=get4bytes(RMV);
	
	//Length of header string
	if(file_type == 1) result_string_size=get2bytes(RMV);
	
	//Game properties
	version_info_size=get2bytes(RMV);
	player_info_size=get2bytes(RMV);
	board_size=get2bytes(RMV);
	preflags_size=get2bytes(RMV);
	properties_size=get2bytes(RMV);
	
	//Extended properties size
	if (file_type == 2) ext_properties_size=get2bytes(RMV);
	
	//Video and checksum size
	vid_size=get4bytes(RMV);
	cs_size=get2bytes(RMV);
	
	//RMV1 specific	checks
	if(file_type == 1) 
	{	
		//Skip unused byte
		_fgetc(RMV);													
		
		//Length of result_string_size starts 3 bytes before 'LEVEL' and ends on the '#' before 'VERSION'
		//Early versions had maximum header length of 35 bytes if Intermediate and 9999.99.
		//Version 2.2 introduced a longer header string
		if (result_string_size>35)
		{
		//Read last part of string after 3BV
		for(i=0;i<result_string_size-32;++i) _fgetc(RMV);

		//Obtain last 3 bytes with the 3bv value (either :xx or xxx)
		//Note that lost games save 0 as the 3BV value
		for(i=0;i<3;++i) bbbv[i]=_fgetc(RMV);
		if (!isdigit(bbbv[0])) bbbv[0]=' ';
		if (!isdigit(bbbv[1])) bbbv[1]=' ';
		if (!isdigit(bbbv[2])) bbbv[2]=' ';

		//Throw away some bytes to get to Timestamp
		for(i=0;i<16;++i) _fgetc(RMV);

		//Obtain Timestamp
		for(i=0;i<10;++i) timestamp[i]=_fgetc(RMV);
		}
		
		//Release 2 beta and earlier versions do not have 3bv or Timestamp	
		else
		{
		bbbv[0]='0';
		timestamp[0]='0';
		for(i=0;i<result_string_size-3;++i) _fgetc(RMV);
		}

		//Throw away the 2 bytes with '# ' before 'Vienna...'
		_fgetc(RMV);
		_fgetc(RMV);
	}

	//Program name is 18 bytes 'Vienna Minesweeper'
	for(i=0;i<18;++i) program[i]=_fgetc(RMV);
	program[i]=0;

	//Throw away the ' - '
	_fgetc(RMV);
	_fgetc(RMV);
	_fgetc(RMV);

	//Put remainder of version string into a new string
	for(i=0;i<version_info_size-22;++i) version[i]=_fgetc(RMV);
	version[i]=0;

	//Home Edition 3.0H and Scoreganizer 3.0C and later have 1 extra byte (a period) before player name
	_fgetc(RMV);

	//Check next 2 bytes to see how many player information fields are populated
	num_player_info=get2bytes(RMV);

	//Fetch Player fields (name, nick, country, token) if they exist
	//These last 3 fields were defined in Viennasweeper 3.1 RC1
	if(num_player_info>0)
	{
		name_length=_fgetc(RMV);
		for(i=0;i<name_length;++i) name[i]=_fgetc(RMV);
		name[i]=0;
	}
	if(num_player_info>1)
	{
		nick_length=_fgetc(RMV);
		for(i=0;i<nick_length;++i) nick[i]=_fgetc(RMV);
		nick[i]=0;
	}
	if(num_player_info>2)
	{
		country_length=_fgetc(RMV);
		for(i=0;i<country_length;++i) country[i]=_fgetc(RMV);
		country[i]=0;
	}
	if(num_player_info>3)
	{
		token_length=_fgetc(RMV);
		for(i=0;i<token_length;++i) token[i]=_fgetc(RMV);
		token[i]=0;
	}

	//RMV1 does not use these bytes
	if(file_type == 1) {get4bytes(RMV);}
	
	//RMV2 stores the timestamp	here
	if(file_type == 2) {timestamp_boardgen = get4bytes(RMV);}	
	
	//Get board size and number of mines
	w=_fgetc(RMV);
	h=_fgetc(RMV);
	m=get2bytes(RMV);
	
	//Fetch board layout and put in memory
	board=(int*)malloc(sizeof(int)*w*h);
	for(i=0;i<w*h;++i) board[i]=0;

   //Every 2 bytes is x,y with 0,0 being the top left corner
	for(i=0;i<m;++i)
	{
		c=_fgetc(RMV);d=_fgetc(RMV);
		if(c>w || d>h) error("Invalid mine position");
		board[d*w+c]=1;
	}
   
	//Check number of flags placed before game started
	if(preflags_size)
	{
		num_preflags=get2bytes(RMV);
		for(i=0;i<num_preflags;++i)
		{
			c=_fgetc(RMV);d=_fgetc(RMV);

			video[cur].event=4;
			video[cur].x=square_size/2+c*square_size;
			video[cur].y=square_size/2+d*square_size;
			video[cur].time=0;
			cur++;

			video[cur].event=5;
			video[cur].x=square_size/2+c*square_size;
			video[cur].y=square_size/2+d*square_size;
			video[cur].time=0;
			cur++;
		}
	}	
	
	//Fetch game properties
	qm=_fgetc(RMV); 		//Value 1 if Questionmarks used otherwise 0
	nf=_fgetc(RMV);    		//Value 1 if no Flags were used otherwise 0
	mode=_fgetc(RMV);		//Value 0 for Classic, 1 UPK, 2 Cheat, 3 Density
	level=_fgetc(RMV);		//Value 0 for Beg, 1 Int, 2 Exp, 3 Custom      

	//RMV1 does not use other property placeholders (if they exist)
	if(file_type == 1)  
	{
		for(i=4;i<properties_size-2;++i) _fgetc(RMV);
	}

	//RMV2 stores 3bv then does not use other property placeholders (if they exist)
	if(file_type == 2)  
	{
		bbbvint = _fgetc(RMV);
		for(i=5;i<properties_size-2;++i) _fgetc(RMV);
	}
	
	//RMV2 allows custom square sizes
	if(file_type == 2)  
	{
		square_size=get2bytes(RMV);
	}	
	
	//RMV2 allows clones to store optional additional properties
	//Viennasweeper 5 stores keybind and skin if defaults are not used
	if(file_type == 2 && ext_properties_size>0) 
	{
		//Read all property bytes
		num_extension_properties = get2bytes(RMV);
		
		//Populate array with extended property names and values
		//Adjust maximum array size in global variables if needed
		for(i=0;i<num_extension_properties;++i)
		{
			//Property name length is one byte
			ext_prop_name_size = _fgetc(RMV);
			
			for(j=0;j<ext_prop_name_size;++j)
			{
				ext_prop_name[i][j]=fgetc(RMV);
			}				
			
			//Property value length is one byte
			ext_prop_value_size = _fgetc(RMV);
			
			for(j=0;j<ext_prop_value_size;++j)
			{
				ext_prop_value[i][j]=fgetc(RMV);
			}	

			//Check optional VS5 properties
			if (strcmp("vsweep_keybinds",ext_prop_name[i])==0) keybind=1;
			
			if (strcmp("vsweep_skin_fname",ext_prop_name[i])==0)			
			{
				for(j=0;j<ext_prop_value_size;++j)
				{
					skin_name[j]=ext_prop_value[i][j];
				}	
			}				
		}
	} 
		
	//Each iteration reads one event
	while(1)
	{
		video[cur].event=c=_fgetc(RMV);++i;
		
		if(c<=7)
		{
			i+=8;
			video[cur].time=get3bytes(RMV);
			_fgetc(RMV);
			//RMV1 co-ordinates are from top-left of UI
			if(file_type == 1) video[cur].x=get2bytes(RMV)-12;
			if(file_type == 1) video[cur].y=get2bytes(RMV)-56;
			//RMV2 co-ordinates are from top-left of grid
			if(file_type == 2) video[cur].x=get2bytes(RMV);
			if(file_type == 2) video[cur].y=get2bytes(RMV);			
			cur++;			
			
			//Viennasweeper does not record clicks before timer starts
			//LR starts timer so the first LC is missed in the video file
			//This code generates the missing LC in that case
			//In other cases it generates a ghost event thus event[0] is empty			
			if(is_first_event)
			{
				//Global variable set to 1 so on first iteration it becomes 0				
				is_first_event=0;
				//Copy first recorded event but set missing event to LC
				//This tactical fix means such a first click will show 0s duration
				video[cur].event=video[cur-1].event;
				video[cur-1].event=2;
				video[cur].time=video[cur-1].time;
				video[cur].x=video[cur-1].x;
				video[cur].y=video[cur-1].y;
				cur++;
			}
		}
		else if(c==8) error("Invalid event");
		//Get board event (ie, 'pressed' or 'number 3')
		else if(c<=14 || (c>=18 && c<=27))
		{
			i+=2;
			video[cur].x=_fgetc(RMV)+1;
			video[cur].y=_fgetc(RMV)+1;
			cur++;				
		}
		//RMV2 reduced mouse move captures movement relative to prior move so is 3 bytes instead of 9 bytes
		else if(c==28 && file_type ==2)
		{
			i+=2;
			video[cur].time=video[cur-1].time + _fgetc(RMV);
			signed int pos_change = _fgetc(RMV);		
			video[cur].x = (pos_change & 0xf0) >> 4; 	// upper 4 bits
			video[cur].y = pos_change & 0x0f;			// lower 4 bits	
			cur++;		
			
		}		
		//Get game status (ie, 'won')
		else if(c<=17)
		{
			break;
		}			
		else 
		{
			cur++;
		}
	}
	
	//Number of game events	
	size=cur+1;

	return 1;
}


//==============================================================================================
//Function is used to print video data
//==============================================================================================
void writetxt()
{
	//Initialise local variables	
	int i,j;
	const char* level_names[]={"Beginner","Intermediate","Expert","Custom"};
	const char* mode_names[]={"Classic","UPK","Cheat","Density"};

	//Code version and Program
	printf("RawVF_Version: RMV 6.2\n");
	printf("Program: %s\n",program);

	//Print Version
    printf("Version: ");

	//There are several different version string formats
	//For example, 'Vienna Minesweeper - Release 5.0.0a2 Copyright (C) 2008-2024"
	//For example, 'Vienna Minesweeper - Scoreganizer Client Edition - Release 3.0C Copyright (C) 2008-2012'
    //For example, 'Vienna Minesweeper - Home Edition - Release 3.0H Copyright (C) 2008-2012'
	//For example, 'Vienna Minesweeper - Home Edition - Release 2.2 (c)2008'
    //For example, 'Vienna Minesweeper - Home Edition - Release 2 beta.2008'

    //This fetches the Version string but stops at '(' if it exists
	for(i=0;i<sizeof(version);++i)
    {
   	if(version[i]!='(')
        verstring[i]=version[i];
     	else break;
    }

	//This deletes the word 'Copyright' from the above string
    if(verstring[strlen(verstring)-10]=='C')
    {
   	for (i=0;i<strlen(verstring)-11;++i)
		{
		verlength[i]=verstring[i];
		}
		printf("%s\n",verlength);
    }
	
	//This terminates the above string at the period for Release 2 beta and earlier
	else if(verstring[strlen(verstring)-23]=='.')
	{
   	for (i=0;i<strlen(verstring)-28;++i)
		{
		verlength[i]=verstring[i];
		}
		printf("%s\n",verlength);
	}
	
	//This is for any versions without a copyright notice or period in Version string
	else
	{
		printf("%s\n",verstring);
	}

	//Print Player 
	printf("Player: %s\n",name);
	
	//Print grid details	
	printf("Level: %s\n",level_names[level]);
	printf("Width: %d\n",w);
	printf("Height: %d\n",h);
	printf("Mines: %d\n",m);

	//Print Marks
	if(qm) printf("Marks: On\n");
    else printf("Marks: Off\n");

	//Print Time
    printf("Time: ");

	//Number events and status events do not have a time
	//In most videos the 3rd or 4th last event is the final timed event
	//Opening many cells on the last click needs a loop to find the last timed event
   	for (i=size-3;i>size-20;--i)
	{
		print_event2(video+i);
		if(score!=0)
		{
			printf("%d.%03d\n",score/1000,score%1000);
			break;
		}
	}

	//Print 3bv and 3bvs
	//Modern versions store in a variable while earlier versions store in the header string
	//Release 2 beta and earlier versions do not store this information so print blank rows
	if(bbbvint)
	{
		printf("BBBV: %d\n",bbbvint);
		//You need to float 3bvs int otherwise integer/integer = integer.
		bbbvs=(bbbvint*1000000)/(score);
		bbbvs_final=(float)bbbvs/1000;
		printf("BBBVS: %.03f\n",bbbvs_final);	
	} 	
	else if(bbbv[0]!='0')
	{
		//Convert string to an integer
		bbbvint=atoi(bbbv);
		printf("BBBV: %d\n",bbbvint);
		bbbvs=(bbbvint*1000000)/(score);
		bbbvs_final=(float)bbbvs/1000;
		printf("BBBVS: %.03f\n",bbbvs_final);	
	}   
	else
	{
		printf("BBBV: \n");
		printf("BBBVS: \n");		
	}	

	//Print Status
	if(i=size-1)
	{
		printf("Status: ");
		print_event2(video+i);
	}	
	
	//Print Timestamp
	//RMV2 uses a variable while earlier versions store this in the header string
	if(timestamp_boardgen)
	{
		printf("Timestamp: %d\n",timestamp_boardgen);
	}	
	else if(timestamp[0]!='0')
	{
		printf("Timestamp: %s\n",timestamp);
	}
	else
	{
		printf("Timestamp: \n");
	}	

	//Print Mode
	printf("Mode: %s\n",mode_names[mode]);

	//Print Style
	if(nf) printf("Style: NF\n");
	else printf("Style: FL\n");         

	//Print Keybind
	printf("Keybind: %d\n",keybind);
		
	//Print Skin
	printf("Skin: %s\n",skin_name);	

	//Print Square Size	
	printf("SquareSize: %d\n",square_size);		
	
	//Print Board
	printf("Board:\n");
	for(i=0;i<h;++i)
	{
		for(j=0;j<w;++j)
			if(board[i*w+j])
				printf("*");
			else
				printf("0");
		printf("\n");
	}

	//Print Mouse events
	printf("Events:\n");
	printf("0.000 start\n");
	for(i=0;i<size;++i)
	{
		print_event(video+i);
	}
}


//==============================================================================================
//Run program and display any error messages
//==============================================================================================
int main(int argc,char** argv)
{
	//Program can be run in command line as "program_name video.rmv>output.txt"
	//The output file is optional if you prefer printing to screen		
	if(argc<2)
	{
		printf("Error 1: Name of input file missing\n");
		printf("Usage: %s <input rmv> [nopause]\n",argv[0]);
		pause();
		return 0;
	}
	
	//Open video file	
	RMV=fopen(argv[1],"rb");

	//Error if video is not an RMV file	
	if(!RMV)
	{
		printf("Error 2: Could not open RMV\n");
		return 1;
	}

	//Error if video parsing fails
	if(!readrmv())
	{
		printf("Error 3: Invalid RMV\n");
		return 1;
	}
	
	//Print results, close file and free memory
	writetxt();
	fclose(RMV);free(board);

	//Program ends with message to exit		
	if(argc==2) pause();
	return 0;
}
