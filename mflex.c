 /*********************************
 *								 **
*                                 *
*   Media Flex    (C) 1993        *
*                                 *
*   (c) 1993  CCS                 *
*                                 *
*                                 *
* This source is copyright © 1993 *
*         in all respects.        *
**                               *
*********************************/
///////////////////////////////////////////////////////
//
// 18/04/94	:	RS, uses realtime.library now
// 19/04/94 :	RS, tooltype "SERIAL=ON" added to decide wheather to use the serial port or not
//					high-end always uses serialport at the mo
// 20/04/94 :	RS, added TOOLTYPE 'SCI=ON' for serial interface
//
//
//
//
//
//
//


#include <stdio.h>					// and the thing we all use!
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/stdio.h>				// doses stdio

#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <graphics/view.h>
#include <graphics/videocontrol.h>
#include <graphics/displayinfo.h>
#include <intuition/intuitionbase.h>
#include <intuition/classes.h>
#include <utility/tagitem.h>

#include <dos.h>

#include "mflex.h"
#include "globals.h"

#include <proto/exec.h>				// use amiga library stuff
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/asl.h>
#include <proto/diskfont.h>
#include <proto/utility.h>
#include <proto/icon.h>

#ifdef LOWEND_ON
#include "le_editor.h"
#else
#include "editor.h"
#endif

#include "files.h"
#include "errors.h"
#include "funcs.h"					// functions protos
#include "database.h"				// dbase definitions
#include "eventhandler.h"
#include "edithook.h"
#include "EditDisplay.h"
#include "VTR_Ctrl.h"
#include "DTAGSrequestor.h"
#include "SearchReq.h"
#include "InputProj.h"
#include "KeyEditor.h"

#include "editorGUI.h"				// gadgets include
#include "EditDisplayGUI.h"
#include "VTR_CtrlGUI.h"
#include "DTAGSGUI.h"
#include "DCGUI.h"
#include "TimeLineGUI.h"
#include "SearchReqGUI.h"
#include "InputProjGUI.h"
#include "KeyEditorGUI.h"
#include "TransitionGUI.h"
#include "VideoSetupGUI.h"







extern	BPTR			debugwin;

extern	BOOL			StartNew;

		BOOL			LowEnd_Status = FALSE;




// --- global definitions local to this module

struct TextAttr le_mflex8 = {
	( STRPTR )"le_mflex.font", 8, 0x00, 0x00 };

struct TextAttr mflex8 = {
	( STRPTR )"mflex.font", 8, 0x00, 0x00 };

struct TextAttr mflex10 = {
	( STRPTR )"mflex.font", 10, 0x00, 0x00 };

struct TextAttr mflex12 = {
	( STRPTR )"mflex.font", 12, 0x00, 0x00 };

struct TextAttr mflex14 = {
	( STRPTR )"mflex.font", 14, 0x00, 0x00 };

struct TextAttr mflex16 = {
	( STRPTR )"mflex.font", 16, 0x00, 0x00 };

struct TextAttr mflex18 = {
	( STRPTR )"mflex.font", 18, 0x00, 0x00 };

struct TextAttr mflexLED15 = {
	( STRPTR )"mflexLED.font", 15, 0x00, 0x00 };

struct TextAttr mflexLED27 = {
	( STRPTR )"mflexLED.font", 27, 0x00, 0x00 };

struct TextAttr helvetica11 = {
	( STRPTR )"helvetica.font", 11, 0x00, 0x00 };

struct TextAttr helvetica13 = {
	( STRPTR )"helvetica.font", 13, 0x00, 0x00 };

struct TextAttr helvetica18 = {
	( STRPTR )"helvetica.font", 18, 0x00, 0x00 };

struct	FileRequester	*filereq;

struct	TextFont
						*le_mainfont8,
						*mainfont8,
						*mainfont10,
						*mainfont12,
						*mainfont14,
						*mainfont16,
						*mainfont18,
						*LEDfont15,
						*LEDfont27,
						*helveticafont11,
						*helveticafont13,
						*helveticafont18;
struct	Hook			MyHook;

struct	Process 	 	*mflex_proc;

struct	Window			*oldwin;



#ifdef	LOWEND_ON
#define	LE_V1_0_WIDTH	640
#define	MIN_HEIGHT		400
#define	SERIAL_ON		FALSE
#else
#define	LE_V1_0_WIDTH	1280
#define	MIN_HEIGHT		768
#define	SERIAL_ON		TRUE
#endif
		ULONG			fps				=	MOD_PAL;	// frame per second
		LONG			argcount;
		LONG			main_depth		= 4;
		LONG			main_mon		= DEFAULT_MONITOR_ID;
		LONG			main_height		= MIN_HEIGHT;
		LONG			main_width		= LE_V1_0_WIDTH;
		LONG			main_transwidth = 0;

		ULONG			OSver			=	37;			// the version of the OS
		ULONG			framerate		= 25;

		UBYTE			*screen_name;

		BOOL			god_mode		= FALSE;
		BOOL			debug_menus		= FALSE;
		BOOL			scrolling		= TRUE;
		BOOL			use_serialport	= SERIAL_ON;
		BOOL			use_sci			= FALSE;

		FILE			*logfp;

		UBYTE			TC_Seperator=':';

		char myfilename[256];





	BOOL	ask4screen=FALSE;
	ULONG	screen_type=NULL;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 


void Save_ScreenMode_ToolType(
	LONG sm,
	LONG mh,
	LONG mw
){
	struct	DiskObject *diskobj;
	UBYTE	**oldtxtptrs;
	UBYTE	*txt;
	BOOL	save_tt=FALSE;

	if ( diskobj=GetDiskObject( myfilename ) ){
		oldtxtptrs = diskobj->do_ToolTypes;

		if( txt = FindToolType( oldtxtptrs, "MONITORID" ) ) {
			sprintf( txt, "%08lx", sm );
			save_tt = TRUE;
		}
		if( txt = FindToolType( oldtxtptrs, "HEIGHT" ) ) {
			sprintf( txt, "%d", mh );
			save_tt = TRUE;
		}
		if( txt = FindToolType( oldtxtptrs, "WIDTH" ) ) {
			sprintf( txt, "%d", mw );
			save_tt = TRUE;
		}

		if( save_tt )
			PutDiskObject( myfilename, diskobj );

		FreeDiskObject( diskobj );
	}
}




	LONG	new_width;
	LONG	new_height;

ULONG Ask4NewScreen( void ){
//	struct	TagItem	AslTags[] = { TAG_DONE };
struct	ScreenModeRequester	*screenreq;     
	ULONG	sid;

	screenreq = (struct ScreenModeRequester *) AllocAslRequest( ASL_ScreenModeRequest, NULL );

	if ( !screenreq )
		return NULL;

	if ( AslRequestTags( screenreq,
			ASLSM_TitleText, 				"Select new screen format...",
			ASLSM_InitialHeight,			200,
			ASLSM_InitialDisplayHeight,		main_height,
			ASLSM_InitialDisplayDepth,		main_depth,
			ASLSM_InitialDisplayID,			screen_type,
			ASLSM_DoHeight,					TRUE,
			ASLSM_DoWidth,					TRUE,
//			ASLSM_DoDepth,					TRUE,
			ASLSM_MinHeight,				400,
			ASLSM_MinWidth,					640,
//			ASLSM_MinDepth,					4,
				TAG_DONE ) != FALSE) {
		sid = screenreq->sm_DisplayID;
		main_height = screenreq->sm_DisplayHeight;
		main_width = screenreq->sm_DisplayWidth;
//		main_depth = screenreq->sm_DisplayDepth;
		FreeAslRequest( screenreq );
//		screen_type = sid;

		Save_ScreenMode_ToolType( sid, main_height, main_width );

		return( sid );
	}
	else{
		FreeAslRequest( screenreq );
		return NULL;
	}
}


/********************* All Functions are listed below
 ***********/

ULONG	ColorTable[ 1+ (256*3) +1];
UWORD	MyDriPens[] = {
			2,	// detail
			1,	// block
			1,	// text			(normal text)
			2,	// shine		(topleft gadget col)
			1,	// shadow		(botright gadget col)
			5,	// fill			(window hi color)
			2,	// filltext		(window hi text color)
			4,	// backround
			13,	// hilighttext
			2,	// bardetail
			11,	// barblock
			1,	// bartrim
			0xffff };



/**********************************************************************
 *
 *						MySetupScreen
 *
 *	Description : opens up the work screen
 *	Returns		: NULL if succeeded
 *	Globals		:
 *
 */
int MySetupScreen( void ){
struct	TagItem VTags[] = { VTAG_BORDERBLANK_SET,0l,TAG_DONE };
	struct	DimensionInfo disp_data;
	WORD	height,maxheight,
			width,maxwidth;
	ULONG	st;

	struct	Rectangle cust_region = { 0,0, 1024+120, 768 };

	if( ask4screen ){
		if( st = Ask4NewScreen() )
			screen_type = st;
	//	else
	//		screen_type = main_mon|HIRESLACE_KEY;
	}
	else {
		if( (main_mon>>16) < 0xf )
			screen_type = main_mon|HIRESLACE_KEY;
		else
			screen_type = main_mon;
	}

	if( god_mode ) main_transwidth=256;

//	if( debug_menus )
//	printf("\nUsing displayID <%lx>, Height %d, Depth %d\n",screen_type,main_height,main_depth);


#ifdef	LOWEND_ON
	LoadPalette( &ColorTable[0], MFILE_LOWPAL, main_depth );
#else
	LoadPalette( &ColorTable[0], MFILE_HIPAL, main_depth );
#endif

	if( ModeNotAvailable( screen_type ) ) {
		if( debug_menus )
		printf("display mode <%lx> not available, switching to default.\n", screen_type );
		screen_type = DEFAULT_MONITOR_ID|HIRESLACE_KEY;
	}

	if( GetDisplayInfoData( NULL, (UBYTE *)(&disp_data), sizeof( disp_data ), DTAG_DIMS, screen_type ) ){
		height = (disp_data.TxtOScan.MaxY - disp_data.TxtOScan.MinY)+1;
		maxheight = (disp_data.MaxOScan.MaxY - disp_data.MaxOScan.MinY)+1;
		maxwidth = (disp_data.MaxOScan.MaxX - disp_data.MaxOScan.MinX)+1;
		width = (disp_data.TxtOScan.MaxX - disp_data.TxtOScan.MinX)+1;
		if( main_depth > disp_data.MaxDepth ) main_depth = disp_data.MaxDepth;
	}
	else
		return 1L;


/*	if( width < 362 )
		cust_region.MaxX = width;
	else
	if( width > 640 )
		cust_region.MaxX = width;
	else
		cust_region.MaxX = 640;
*/
	cust_region.MaxX = maxwidth;
	if(main_height > maxheight) 
		cust_region.MaxY = maxheight-34;
	else
		cust_region.MaxY = maxheight;

	if ( ! ( Scr = OpenScreenTags( NULL,
					SA_Left,		0,
					SA_Top,			0,
#ifdef LOWEND_ON
					SA_Width,		main_width+main_transwidth,
					SA_Height,		main_height,
					SA_Font,		&helvetica13,
//					SA_DClip,		&cust_region,
#else
					SA_Width,		1280,
					SA_Height,		768,
					SA_Font,		&helvetica18,
				//	SA_DClip,		&cust_region,
#endif
					SA_AutoScroll,	scrolling,
					SA_Depth,		main_depth,
				//	SA_Colors,		&ScreenColors[0],
					SA_Colors32,	&ColorTable[0],
					SA_Type,		PUBLICSCREEN,
					SA_DisplayID,	screen_type,
					SA_Overscan,	OSCAN_TEXT,
					SA_Pens,		&MyDriPens[0],
					SA_PubName,		MFLEX_PUBNAME,
					SA_Title,		MFLEX_NAME,
					SA_Interleaved,	TRUE,
					SA_VideoControl,&VTags[0],
					SA_ShowTitle,	TRUE,
					SA_Behind,		TRUE,
					TAG_DONE )))
		return( 1L );

/*
	// this supports Picasso and others
	screen_type = GetVPModeID( &Scr->ViewPort );
	if( GetDisplayInfoData( NULL, (UBYTE *)(&disp_data), sizeof( disp_data ), DTAG_DIMS, screen_type ) ){
		height = (disp_data.TxtOScan.MaxY - disp_data.TxtOScan.MinY)+1;
		width = (disp_data.TxtOScan.MaxX - disp_data.TxtOScan.MinX)+1;
		if( height > 400 )
			main_height = height;
		if( main_depth > disp_data.MaxDepth ) main_depth = disp_data.MaxDepth;
	}
 */

	if ( ! ( VisualInfo = GetVisualInfo( Scr, TAG_DONE )))
		return( 2L );

	PubScreenStatus( Scr, NULL );

	return( 0L );
}




#include "Welcome.c"




/**********************************************************************
 *
 *						OpenLibraries
 *
 *	Description : Open lots of libraries that I need.
 *	Returns		: FALSE if failed any library
 *	Globals		:
 *
 */
long OpenLibraries( void ){
	long	rc=TRUE;

	if ( !(GfxBase = (struct GfxBase *) OpenLibrary((UBYTE *) "graphics.library" , 36l ))) {
		WriteStr("\graphics.library\n");
		rc= FALSE;
		}

	if ( !(DosBase = (struct DosBase *) OpenLibrary((UBYTE *) "dos.library", 36l ))) {
		WriteStr("\tdos.library\n");
		rc= FALSE;
		}

	if ( !(IntuitionBase = (struct IntuitionBase *) OpenLibrary((UBYTE *) "intuition.library", 36l ))) {
		WriteStr("\tintuition.library\n");
		rc= FALSE;
		}

	if ( !(AslBase = (struct Library *) OpenLibrary((UBYTE *) "asl.library", 38l ))) {
		WriteStr("\tasl.library\n");
		rc= FALSE;
		}

	if ( !(GadToolsBase = (struct Library *) OpenLibrary((UBYTE *) "gadtools.library", 38l ))) {
		WriteStr("\tgadtools.library\n");
		rc= FALSE;
		}

	if ( !(DiskFontBase = (struct Library *) OpenLibrary((UBYTE *) "diskfont.library", 36l ))) {
		WriteStr("\tdiskfont.library\n");
		rc= FALSE;
		}

	if ( !(UtilityBase = (struct Library *) OpenLibrary((UBYTE *) "utility.library", 36l ))) {
		WriteStr("\tutility.library\n");
		rc= FALSE;
		}

	if ( !(IconBase = (struct Library *) OpenLibrary((UBYTE *) "icon.library" , 36l ))) {
		WriteStr("\icon.library\n");
		rc = FALSE;
		}

	if ( !(ExpansionBase = (struct Library *) OpenLibrary((UBYTE *) "expansion.library", 0l ))) {
		WriteStr("\texpansion.library\n");
		rc= FALSE;
		}

	if ( !(IFFParseBase = (struct Library *) OpenLibrary((UBYTE *) "iffparse.library", 0l ))) {
		WriteStr("\tiffparse.library\n");
		rc= FALSE;
		}

	if ( !(RealTimeBase = (struct Library *) OpenLibrary((UBYTE *) "realtime.library", 0l ))) {
		WriteStr("\trealtime.library\n");
		rc= TRUE;
		}

	return rc;
}










/**********************************************************************
 *
 *						CloseLibraries
 *
 *	Description : Close the libraries which are opened by me.
 *	Returns		: void
 *	Globals		:
 *
 */
void CloseLibraries( void ){
	if (RealTimeBase)	CloseLibrary( (struct Library *) RealTimeBase );
	if (IFFParseBase)	CloseLibrary( (struct Library *) IFFParseBase );
	if (ExpansionBase)	CloseLibrary( (struct Library *) ExpansionBase );
	if (IconBase)		CloseLibrary( (struct Library *) IconBase );
	if (UtilityBase)	CloseLibrary( (struct Library *) UtilityBase );
	if (DiskFontBase)	CloseLibrary( (struct Library *) DiskFontBase );
	if (GadToolsBase)	CloseLibrary( (struct Library *) GadToolsBase );
	if (AslBase)		CloseLibrary( (struct Library *) AslBase );
	if (IntuitionBase)	CloseLibrary( (struct Library *) IntuitionBase );
	if (DosBase)		CloseLibrary( (struct Library *) DosBase );
	if (GfxBase)		CloseLibrary( (struct Library *) GfxBase );
}







	BOOL	init_dbase = FALSE;
	BOOL	init_dtags = FALSE;
	BOOL	init_edit = FALSE;


extern void FreeVideoIcons(void);


/**********************************************************************
 *
 *						ExitProgram
 *
 *	Description : Deallocate and close all resources I greedily grabbed before.
 *	Returns		: void
 *	Globals		:
 *
 */
long ExitProgram(
	long code			// exit error code
){
	int cnt=0;

	if( mflex_proc )
		mflex_proc->pr_WindowPtr = oldwin;

	CloseWelcomeWindow();

	if( TransWnd )
		CloseTransWindow();

	FreeVideoIcons();

	if( VideoSetupWnd )
		CloseVideoSetupWindow();

	if( ListEditorWnd )
		SaveWindowPos();

	if ( InputProjWnd )
		CloseInputProjWin();

	if ( init_edit )
		CloseDownEditor();

	if ( KeyEditorWnd )
		CloseKeyEditorDisplay();

	if( init_dtags )
		FreeDTAGSrequestor();

	if ( init_dbase )
		CloseDownDataBase();

	if ( SearchReqWnd )
		CloseSearchReqWin();

	if( DTAGS0Wnd )
		CloseDTAGSrequestor();

	if( TimeLinePrefsWnd )
		CloseTimeLinePrefsWindow();

	if( TimeLineWnd )
		CloseTimeLineWin();

	if ( VTR_CtrlWnd )
		CloseVTR_CtrlWin();

	if ( EditDisplayWnd )
		CloseEditDisplayWin();

	if ( DCWnd )
		CloseDCWindow();

	if( debugwin )
		CloseDebug();

	if ( TempWnd )
		CloseWindow( TempWnd );

	if ( ObjWnd )
		CloseWindow( ObjWnd );

	if ( CursorWnd )
		CloseWindow( CursorWnd );

	if ( ListEditorWnd )
		CloseListEditorWindow();

	if ( Scr ) {
		SavePalette();
		while( Scr->FirstWindow ) {
			if( cnt > 10 )
				AskRequestLow("Urgent!","You have 10 seconds to comply, close THEM!","OK");
			else
			if( cnt > 6 )
				AskRequestLow("Urgent!","Now come one, please close them!","OK");
			else
			if( cnt > 4 )
				AskRequestLow("Urgent!","You aren't trying hard are you, close them all!","OK");
			else
			if( cnt > 2 )
				AskRequestLow("Urgent!","NOW! please close all windows on MediaFlex!","OK");
			else
				AskRequestLow("Urgent!","Please close all windows on the MediaFlex screen!","OK");
			cnt++;
		}

		CloseDownScreen();
		if( Scr ) AskRequestLow("Urgent!","Cant close main screen???","RETRY|OK");

	}

	if ( filereq )
		FreeAslRequest( filereq );

	if ( le_mainfont8 )
		CloseFont( le_mainfont8 );
	if ( mainfont8 )
		CloseFont( mainfont8 );
	if ( mainfont10 )
		CloseFont( mainfont10 );
	if ( mainfont12 )
		CloseFont( mainfont12 );
	if ( mainfont14 )
		CloseFont( mainfont14 );
	if ( mainfont16 )
		CloseFont( mainfont16 );
	if ( mainfont18 )
		CloseFont( mainfont18 );
	if ( LEDfont15 )
		CloseFont( LEDfont15 );
	if ( LEDfont27 )
		CloseFont( LEDfont27 );
	if ( helveticafont18 )
		CloseFont( helveticafont18 );
	if ( helveticafont13 )
		CloseFont( helveticafont13 );
	if ( helveticafont11 )
		CloseFont( helveticafont11 );

	CloseTimer();

	CloseLibraries();

	fclose( logfp );

	exit( code );

	return( code );
}



	BOOL	round_bevel=FALSE;

extern	UWORD TranscriptMode;

//
// this reads in the tooltypres from the media flex icon
//
void Get_IconToolTypes( void ){
	struct	DiskObject *diskobj;
	UBYTE	**oldtxtptrs;
	UBYTE	*txt;

	if ( diskobj=GetDiskObject( myfilename ) ){
		oldtxtptrs = diskobj->do_ToolTypes;
		while( *oldtxtptrs ){
			txt = *oldtxtptrs;
			if( strnicmp( txt, "ASKSCREEN",9 )==NULL )
				ask4screen = TRUE;
			else
			if( strnicmp( txt, "MONITORID=",10 )==NULL ){
				main_mon = DEFAULT_MONITOR_ID;
				sscanf( txt+10,"%lx",&main_mon );
			}
			else
			if( strnicmp( txt, "HEIGHT=",7 )==NULL ){
				sscanf( txt+7,"%d",&main_height );
				if( main_height < 400 ) main_height = 400;
			}
			else
			if( strnicmp( txt, "WIDTH=",6 )==NULL ){
				sscanf( txt+6,"%d",&main_width );
				if( main_width < 640 ) main_width = 640;
			}
			else
			if( strnicmp( txt, "DEPTH=",6 )==NULL )
				main_depth = atoi( txt+6 );
			else
			if( strnicmp( txt, "BEVEL=ROUND",11 )==NULL )
				round_bevel = TRUE;
			else
			if( strnicmp( txt, "BEVEL=SQUARE",12 )==NULL )
				round_bevel = FALSE;
			else
			if( strnicmp( txt, "DEBUGON",7 )==NULL )
				debug_menus = TRUE;
			else
			if( strnicmp( txt, "NOSCROLL",8 )==NULL )
				scrolling = FALSE;
			else
			if( strnicmp( txt, "SEPERATOR=",10 )==NULL )
				TC_Seperator = *(txt+10);
			else
			if( strnicmp( txt, "SERIAL=ON",9 )==NULL )
				use_serialport = TRUE;
			else
			if( strnicmp( txt, "SCI=ON",6 )==NULL ) {
				use_serialport = TRUE;
				use_sci = TRUE;
			}
			else
			if( strnicmp( txt, "CHAOS",5 )==NULL )
				god_mode = TRUE;
			oldtxtptrs++;
		}
		FreeDiskObject( diskobj );
	}

	if( !debug_menus )
		TranscriptMode = NULL;
}





/**********************************************************************
 *
 *						InitProgram
 *
 *	Description : Allocate and open all resources I need     eatem up yum yum  :-)
 *	Returns		: void
 *	Globals		:
 *
 */
void InitProgram( int argc, char *argv[]){
	struct	TagItem	AslTags[] = { 0,0 };

	int err;
	BPTR	mflock;


	if( mflock = Lock( "MediaFlex:", ACCESS_READ ) )
		UnLock( mflock );
	else
		AssignPath( "MediaFlex", "PROGDIR:" );


	logfp = fopen( MFILE_LOGF,"w+" );

#ifdef LOWEND_ON
	LowEnd_Status = TRUE;
#endif

	if ( ! SetupErrorHandler() ){
		fprintf (stderr,"Failed to Init ErrorHandler\n");
		ExitProgram( FATAL_EXIT );
	}

	mflex_proc = (struct Process *) FindTask( 0l );

	FreeSignal( SIGBREAKB_CTRL_C );
	FreeSignal( SIGBREAKB_CTRL_D );
	FreeSignal( SIGBREAKB_CTRL_E );
	FreeSignal( SIGBREAKB_CTRL_F );

	if ( ! OpenLibraries() ) {
		WriteStr("Could not open requested Libraries.\n");
		ExitProgram( FATAL_EXIT );
		}

	Get_IconToolTypes( );

	InitTimer();

	OSver = GfxBase->LibNode.lib_Version;		// get KickStart version Number

	if( OSver < 30 ){
		WriteStr("Need AmigaOS 3.0 (V39) or higher.\012");
		ExitProgram( FATAL_EXIT );
	}


	if ( DiskFontBase ) {
		if ( ! (le_mainfont8  = OpenDiskFont( &le_mflex8 )) ) {
			WriteStr(" Could no open le_mflex/08 font\n.");
			}
		if ( ! (mainfont8  = OpenDiskFont( &mflex8 )) ) {
			WriteStr(" Could no open mflex/08 font\n.");
			}
		if ( ! (mainfont10 = OpenDiskFont( &mflex10 )) ) {
			WriteStr(" Could no open mflex/10 font\n.");
			}
		if ( ! (mainfont12 = OpenDiskFont( &mflex12 )) ) {
			WriteStr(" Could no open mflex/12 font\n.");
			}
		if ( ! (mainfont14 = OpenDiskFont( &mflex14 )) ) {
			WriteStr(" Could no open mflex/14 font\n.");
			}
		if ( ! (mainfont16 = OpenDiskFont( &mflex16 )) ) {
			WriteStr(" Could no open mflex/16 font\n.");
			}
		if ( ! (mainfont18 = OpenDiskFont( &mflex18 )) ) {
			WriteStr(" Could no open mflex/18 font\n.");
			}
		if ( ! (LEDfont15 = OpenDiskFont( &mflexLED15 )) ) {
			WriteStr(" Could no open mflexLED/15 font\n.");
			}
		if ( ! (LEDfont27 = OpenDiskFont( &mflexLED27 )) ) {
			WriteStr(" Could no open mflexLED/27 font\n.");
			}
		if ( ! (helveticafont11 = OpenDiskFont( &helvetica11 )) ) {
			WriteStr(" Could no open helvetica/11 font\n.");
			}
		if ( ! (helveticafont13 = OpenDiskFont( &helvetica13 )) ) {
			WriteStr(" Could no open helvetica/13 font\n.");
			}
		if ( ! (helveticafont18 = OpenDiskFont( &helvetica18 )) ) {
			WriteStr(" Could no open helvetica/18 font\n.");
			}
		}

	OpenWelcomeWindow();

	InitEditHook( &MyHook, cHook );
	InitLVHook(&renderHook,RenderHook);

ShowWelcomeStatus( "Opening MainScreen." );
	if ( !Scr ){
		if ( argc > 1 )
			screen_name = argv[1];
		if ( MySetupScreen() ) {
			WriteStr("Could not open main screen requested.\n");
			ExitProgram( FATAL_EXIT );
		}
	}
	CurrentScr = Scr;
ShowWelcomeStatus( "Opening Main Window." );
	if ( err = OpenListEditorWindow() ) {
		printf("%d\n",err);
		WriteStr("Could not open main window requested.\n");
		ExitProgram( FATAL_EXIT );
	}
	CurrentWnd = ListEditorWnd;

	if( mflex_proc ) {
		oldwin = mflex_proc->pr_WindowPtr;
		if( ListEditorWnd )
			mflex_proc->pr_WindowPtr = ListEditorWnd;
	}

	filereq = (struct FileRequester *) AllocAslRequest( ASL_FileRequest,AslTags );
	if ( !filereq )
		ExitProgram( FATAL_EXIT );

	OpenDCWindow();

ShowWelcomeStatus( "Loading window positions." );
	LoadWindowPos();

ShowWelcomeStatus( "Initializing Database." );
	InitialiseDataBase(); init_dbase = TRUE;

//	FPS = framerate;

ShowWelcomeStatus( "Initializing DTAGS Requestor." );
	InitDTAGSrequestor(); init_dtags = TRUE;

	if( !LowEnd_Status || god_mode ){
		ShowWelcomeStatus( "Initializing transitions." );
		InitTransWindow();
		if( OpenTransWindow() ) printf("cant open  transwindow\n");
	}

	if( debug_menus )
		OpenDebug();

ShowWelcomeStatus( "Initializing Main Editor." );
	if( InitialiseEditor() ) {
		if( !debug_menus ) {
			init_edit = TRUE;
			AskRequestLow(  "HARDWARE ERROR", "Digital BroadCaster 32 can't be detected!", "OK" );
			if( !debug_menus )
				ExitProgram( FATAL_EXIT );
		}
	}
	init_edit = TRUE;

ShowWelcomeStatus( "Done." );
	CloseWelcomeWindow();

//	CloseWorkBench();

	Set_RegionXYDetect();

	// if either fas/chip ram is l ow then dont bother opening project screen first
	if( (AvailMem( MEMF_FAST|MEMF_LARGEST ) <  256000) ||
		(AvailMem( MEMF_CHIP|MEMF_LARGEST ) < 1000000) ||
		LowEnd_Status ){
		SwitchToEditor();
	}
	else
	if( SwitchToProject() ) {
		WriteStr("cant open project screen?\n");
		ExitProgram( FATAL_EXIT );
	}
}





void CloseDisplayDown( void ) {
	if( TransWnd )
		CloseTransWindow();

	if( ListEditorWnd )
		SaveWindowPos();

//	if ( init_edit )
//		CloseDownEditor();

	if( VideoSetupWnd )
		CloseVideoSetupWindow();

	if( init_dtags )
		FreeDTAGSrequestor();

	if ( SearchReqWnd )
		CloseSearchReqWin();

	if( DTAGS0Wnd )
		CloseDTAGSrequestor();

	if( TimeLinePrefsWnd )
		CloseTimeLinePrefsWindow();

	if( TimeLineWnd )
		CloseTimeLineWin();

	if ( VTR_CtrlWnd )
		CloseVTR_CtrlWin();

	if ( EditDisplayWnd )
		CloseEditDisplayWin();

	if ( DCWnd )
		CloseDCWindow();

	if( debugwin )
		CloseDebug();

	if ( TempWnd )
		CloseWindow( TempWnd );

	if ( ObjWnd )
		CloseWindow( ObjWnd );

	if ( CursorWnd )
		CloseWindow( CursorWnd );

	if ( ListEditorWnd )
		CloseListEditorWindow();

	if ( Scr ) {
		WORD cnt=0;

		while( Scr->FirstWindow ) {
			if( cnt > 10 )
				AskRequestLow("Urgent!","You have 10 seconds to comply, close THEM!","OK");
			else
			if( cnt > 6 )
				AskRequestLow("Urgent!","Now come one, please close them!","OK");
			else
			if( cnt > 4 )
				AskRequestLow("Urgent!","You aren't trying hard are you, close them all!","OK");
			else
			if( cnt > 2 )
				AskRequestLow("Urgent!","NOW! please close all windows on MediaFlex!","OK");
			else
				AskRequestLow("Urgent!","Please close all windows on the MediaFlex screen!","OK");
			cnt++;
		}

		CloseDownScreen();
		if( Scr ) AskRequestLow("Urgent!","Cant close main screen???","RETRY|OK");
	}
}



void OpenDisplayUp( void ){
	WORD err;

	if ( !Scr ){
		if ( MySetupScreen() ) {
			WriteStr("Could not open main screen requested.\n");
			ExitProgram( FATAL_EXIT );
		}
	}
	CurrentScr = Scr;

	if ( err = OpenListEditorWindow() ) {
		printf("Could not open main window requested (%d) \n",err);
		ExitProgram( FATAL_EXIT );
	}
	CurrentWnd = ListEditorWnd;

	if( mflex_proc ) {
		oldwin = mflex_proc->pr_WindowPtr;
		if( ListEditorWnd )
			mflex_proc->pr_WindowPtr = ListEditorWnd;
	}

	OpenDCWindow();

	InitDTAGSrequestor(); init_dtags = TRUE;

	if( !LowEnd_Status || god_mode ){
		InitTransWindow();
		if( OpenTransWindow() ) printf("cant open  transwindow\n");
	}

	if( debug_menus )
		OpenDebug();

	if( InitialiseEditor() ) {
		if( !debug_menus ) {
			init_edit = TRUE;
			AskRequestLow(  "HARDWARE ERROR", "Digital-BroadCaster32 can't be detected!", "OK" );
			if( !debug_menus )
				ExitProgram( FATAL_EXIT );
		}
	}
	init_edit = TRUE;

	ShowStatus("New Screen.");

//	CloseWorkBench();

	StartNew = FALSE;

	Set_RegionXYDetect();

	SwitchToEditor();
}




// This will change screen modes of the screen
void ChangeScreenMode( void ){
	CloseDisplayDown();
	ask4screen = TRUE;
	OpenDisplayUp();
}


static void FullName(BPTR parent, char *name, char *buf, int len)
{
   int i = 0;

   {
      if(NameFromLock(parent, buf, len-1))
      {
         i = strlen(buf);
         if(buf[i-1] != ':' && buf[i-1] != '/')
         {
            buf[i++] = '/';
            buf[i] = 0;
         }
      }
   }

   if(i < len-1) strncpy(buf+i, name, len-i-1);

   return;
}



/***************************************************************************
 **************************************************************************
 ****
 ***					Main
 **
 **	Description : calls all the above functions and works
 **		Here is well it all starts and ends.....
 **		In between is where the mystery lies....
 **
 **
 **
 **
 ** Returns		: return code
 **
 **	Globals		: up there at the very top
 **
 */

//void main(int argc,char *argv[]) {
int main (int argc, union { char **args; struct WBStartup *wbmsg; } argv ) {

		// actual program starts here
		// WOW!, 5GL or what....
	struct WBArg *wba;

	argcount = argc;

	if (argc==0){
		wba=argv.wbmsg->sm_ArgList;
		FullName(wba->wa_Lock,wba->wa_Name,myfilename,256);
	} else {
		strcpy(myfilename,argv.args[0]);
	}

	InitProgram(argc, &argv.args[0]);
	while ( EventHandler() );
	ExitProgram(0);

}




/* END */
/* END */
/* END */
