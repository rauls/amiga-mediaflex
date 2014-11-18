
/*********************************************************************
 *********************************************************************
 **
 *						funcs.c
 *	Description : lots of functions which have more or less no catagory
 *  Returns		: depending on functions
 *	Globals		: filereq
 *
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <utility/tagitem.h>
#include <graphics/rastport.h>

#include <proto/exec.h>				// use amiga library stuff
#include <proto/dos.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/asl.h>

#include "funcs.h"			// use functions from funcs.c
#include "globals.h"
#include "editor.h"
#include "database.h"
#include "db_funcs.h"

#include "editorGUI.h"
#include "EditDisplayGUI.h"
#include "TimeLineGUI.h"
#include "DTAGSGUI.h"


extern	struct	FileRequester	*filereq;


/**********************************************************************
 *
 *						PutsXY
 *
 *	Description : Put string at x y  coord! with A pen & B pen
 *	Returns		: void
 *	Globals		: ListEditorWnd
 *
 */
void PutsXY(
	long x,				// coordinates
	long y,
	UBYTE *string,		// string to show
	UBYTE pen,			// forground pen number
	UBYTE pen2			// backround pen number
){
	WinPutsXY( ListEditorWnd, x,y, string, pen, pen2);
}


/**********************************************************************
 *
 *						WinPutsXY
 *
 *	Description : Put string at x y  coord! with A pen & B pen @ Window
 *	Returns		: void
 *	Globals		: ListEditorWnd
 *
 */
void WinPutsXY(
	struct Window *win,
	long x,				// coordinates
	long y,
	UBYTE *string,		// string to show
	UBYTE pen,			// forground pen number
	UBYTE pen2			// backround pen number
){

	WORD	stlen = strlen(string);

	SetABPenDrMd( win->RPort, pen, pen2, JAM2 );

	Move( win->RPort, x,y );
	Text( win->RPort, (STRPTR)string, stlen );
}






/**********************************************************************
 *
 *						SetWindowFontSize
 *
 *	Description : sets the window list font size
 *	Returns		: void
 *	Globals		:
 *
 */
void SetWindowFontSize(
	ULONG	ysize
){
	Project.LeftWin.Font_Height = ysize;
	Project.RightWin.Font_Height = ysize;
}



/**********************************************************************
 *
 *						UseWindowFontSize
 *
 *	Description : sets the rastports font size according to list window size
 *	Returns		: void
 *	Globals		:
 *
 */
void UseWindowFontSize(
	void
){
	ULONG	ysize =
	Project.LeftWin.Font_Height;

	switch( ysize ){
		case 8 :
			SetFont( ListEditorWnd->RPort, mainfont8 );
			break;
		case 10 :
			SetFont( ListEditorWnd->RPort, mainfont10 );
			break;
		case 12 :
			SetFont( ListEditorWnd->RPort, mainfont12 );
			break;
		case 14 :
			SetFont( ListEditorWnd->RPort, mainfont14 );
			break;
		case 16 :
			SetFont( ListEditorWnd->RPort, mainfont16 );
			break;
		case 18 :
			SetFont( ListEditorWnd->RPort, mainfont18 );
			break;
		// - - - - - - - - - - - - - - - - - - - - - - -
		default :
			SetFont( ListEditorWnd->RPort, mainfont12 );
			break;
	}
}










/**********************************************************************
 *
 *						Frame2Text
 *
 *	Description : converts a 32bit frame number into a text string
 *					of hours:minutes:seconds:frames that is
 *					placed into the input UBYTE pointer ("text")
 *	Returns		: void
 *	Globals		:
 *
 */
void Frame2Text(
	ULONG frame,
	UBYTE *text
){
	WORD	tc[4];

	Frame2TCode( frame, &tc[0] );
	sprintf( text,"%02d:%02d:%02d:%02d" , tc[3],tc[2],tc[1],tc[0]);
}





/**********************************************************************
 *
 *						Text2Frame
 *
 *	Description : converts the txt "00:00:00:00"  into a frame
 *					timecode number
 *
 *	Returns		: frame nmber
 *	Globals		:
 *
 */
ULONG Text2Frame(
	UBYTE *text
){
	ULONG	timec;

	timec = ((*(text+0))&0xf)*(10*60*60*FPS);// hours conv
	timec+= ((*(text+1))&0xf)*(60*60*FPS);

	timec+= ((*(text+3))&0xf)*(10*60*FPS);	// minutes conv
	timec+= ((*(text+4))&0xf)*(60*FPS);

	timec+= ((*(text+6))&0xf)*(10*FPS);		// seconds conv
	timec+= ((*(text+7))&0xf)*(FPS);

	timec+= ((*(text+9))&0xf)*(10);			// frame count
	timec+= ((*(text+10))&0xf);

	return ( timec );
}


/**********************************************************************
 *
 *						Frame2TCode
 *
 *	Description : converts a 32bit frame number into individual
 *					components of hours:minutes:seconds:frames that is
 *					placed into the input WORD array[4]
 *	Returns		: void
 *	Globals		:
 *  TODO        : to get the frame/per/sec number instead of hard-code 25
 *
 */
void Frame2TCode(
	ULONG frame,
	WORD tc[]
){
	tc[0] = (WORD)(frame%25);frame/=FPS;			// frames
	tc[1] = (WORD)(frame%60);frame/=60;			// secs
	tc[2] = (WORD)(frame%60);frame/=60;			// mins
	tc[3] = (WORD)(frame%24);					// hours
}


/**********************************************************************
 *
 *						TCode2Frame
 *
 *	Description : converts components of hours:minutes:seconds:frames
 *					that is	placed into the input WORD array[4]
 *					into a 32bit frame number
 *	Returns		: ULONG frame number
 *	Globals		:
 *
 */
ULONG TCode2Frame(
	WORD tc[]
){
	return( (ULONG)(tc[0] + (tc[1]*FPS) + (tc[2]*60) + (tc[3]*60*60)) );
}



/**********************************************************************
 *
 *						AslFileRequest
 *
 *	Description : Put up the ASL FileRequester
 *	Returns		: UBYTE pointer to filename ascii (NULL terminated)
 *	Globals		:
 *
 */
UBYTE *AslFileRequest(
	UBYTE type,				// 'l'=load / 's'=save
	UBYTE *title,			// title of requester window
	UBYTE *drawer,			// the drawer to open into
	UBYTE *pattern			// the pattern to use
){
	UBYTE	FileName[180], *char1, *ret;
	UWORD	flag1, flag2, len, x, y;

    ret = 0;

	x = ListEditorWnd->MouseX-150;				// get current mouse coords
	y = ListEditorWnd->MouseY-122;

    switch( type ) {
    	case    's':
			char1 = "Save";
			flag1 = FILF_SAVE | FILF_PATGAD;
			flag2 = TRUE;
			break;
        case    'l':
			char1 = "Load";
			flag1 = FILF_PATGAD;
			flag2 = FALSE;
			break;
		}

	if ( AslRequestTags( filereq,
					    ASL_OKText, char1,
					    ASL_Hail, flag1,
						ASLFR_Screen, Scr,
						ASLFR_TitleText, title,
						ASLFR_InitialLeftEdge, x,
						ASLFR_InitialTopEdge, y,
						ASLFR_InitialWidth, 300,
						ASLFR_InitialHeight, 284,
						ASLFR_InitialDrawer, drawer,
						ASLFR_DoSaveMode, flag2,
						ASLFR_DoPatterns, TRUE,
						ASLFR_InitialPattern, pattern,
						TAG_DONE ) ) {
        strcpy( FileName, filereq->rf_Dir );
		len = strlen( FileName );
		if ( len-- )
			if ( FileName[ len ] != '/' && FileName[ len ] != ':' )
				strcat( FileName, "/" );
        strcat( FileName, filereq->rf_File );
        ret = FileName;
    }

	return( ret );
}







/**********************************************************************
 *
 *						AskRequest
 *
 *	Description : Put up a simple notification requester
 *	Returns		: 1=ACCEPT  0=CANCEL
 *	Globals		:
 *
 */
LONG AskRequest(
	UBYTE *body,		// text to be placed in window
	UBYTE type			// 1= only 1 ACCEPT gadget  2=ACCEPT/CANCEL gadgets
){
	ULONG	flags = IDCMP_GADGETUP;

	struct	EasyStruct easydata = {
		sizeof (struct EasyStruct),
		0l,
		" Notification",
		0l,
		0l};

	easydata.es_TextFormat = body;

	if ( type == 1 )
		easydata.es_GadgetFormat = "ACCEPT";
	else
		easydata.es_GadgetFormat = "ACCEPT|CANCEL";

	return EasyRequest( ListEditorWnd, &easydata, &flags , 0 );

}




/**********************************************************************
 *
 *						SaveCMXList
 *
 *	Description : Save any list to system using FILE *fp
 *	Returns		: 1=ACCEPT  0=CANCEL
 *	Globals		:
 *
 */
void SaveCMXList(
	FILE *fp,
	union NODE *dbase
){
static	UBYTE	timecode[] = { "00:00:00:00 " };
	UWORD	total,audio_mode,video_mode;
	union	NODE *curr;


	curr = GetFirstNode( dbase );
	total = dbase->HeadNode.HeadData->lst_tot;

	switch ( BIGMODE( dbase ) ){
		case MF_SHOTMODE :
			fprintf(fp,"TITLE: SHOTLIST CMX Listing (AG-800 FORMAT)\n");
			fprintf(fp,"FCM: NON-DROP FRAME\n");
			while( total-- && curr ){
				fprintf(fp,"%.04d %.03d  ",curr->ListNode.index,
										   curr->SHOTRELREEL_PTR->ListNode.index);

				audio_mode = curr->SHOTRELREELDATA_PTR->audio_mode;
				video_mode = curr->SHOTRELREELDATA_PTR->video_mode;

				if ( audio_mode )
					fprintf(fp,"A%.01d    " ,audio_mode);
				else
				if ( video_mode == 1)
					fprintf(fp,"V     ");
				else
				if ( video_mode == 2)
					fprintf(fp,"V2    ");
				else
					fprintf(fp,"ASSM  ");

				fprintf(fp,"C    000 ");

				// orig in/out
				Frame2Text( curr->SHOTRELREEL_PTR->ReelNode.in, timecode );
				fprintf(fp,"%s ",timecode);
				Frame2Text( curr->SHOTRELREEL_PTR->ReelNode.out, timecode );
				fprintf(fp,"%s ",timecode);

				// shotlist in/out
				Frame2Text( curr->ListNode.in, timecode );
				fprintf(fp,"%s ",timecode);
				Frame2Text( curr->ListNode.out, timecode );
				fprintf(fp,"%s ",timecode);

				if ( curr->EDLRELSHOTDATA_PTR->note[0] )
					fprintf(fp,"\n%s",curr->EDLRELSHOTDATA_PTR->note);	//comment

				fprintf(fp,"\n");

				curr = UpNode( dbase, curr );
			}
			break;

		case MF_EDLMODE :
			fprintf(fp,"TITLE: EDL LIST CMX Listing (AG-800 FORMAT)\n");
			fprintf(fp,"FCM: NON-DROP FRAME\n");
			while( total-- && curr ){
				fprintf(fp,"%.04d %.03d  ",curr->ListNode.index,
										   curr->SHOTRELREEL_PTR->ListNode.index);

				audio_mode = curr->EDLRELREELDATA_PTR->audio_mode;
				video_mode = curr->EDLRELREELDATA_PTR->video_mode;

				if ( audio_mode )
					fprintf(fp,"A%.01d    " ,audio_mode);
				else
				if ( video_mode == 1)
					fprintf(fp,"V     ");
				else
				if ( video_mode == 2)
					fprintf(fp,"V2    ");
				else
					fprintf(fp,"ASSM  ");

				fprintf(fp,"C    000 ");

				// original in/out
				Frame2Text( curr->EDLRELSHOT_PTR->ShotNode.in, timecode );
				fprintf(fp,"%s ",timecode);
				Frame2Text( curr->EDLRELSHOT_PTR->ShotNode.out, timecode );
				fprintf(fp,"%s ",timecode);

				// edl in/out
				Frame2Text( curr->ListNode.in, timecode );
				fprintf(fp,"%s ",timecode);
				Frame2Text( curr->ListNode.out, timecode );
				fprintf(fp,"%s ",timecode);

				if ( curr->EDLRELSHOTDATA_PTR->note[0] )
					fprintf(fp,"\n%s",curr->EDLRELSHOTDATA_PTR->note);	//comment

				fprintf(fp,"\n");

				curr = UpNode( dbase, curr );
			}
			break;
	}
}




/**********************************************************************
 *
 *						SaveList
 *
 *	Description : Save any list to system using FILE *fp
 *	Returns		: 1=ACCEPT  0=CANCEL
 *	Globals		:
 *
 *
void SaveList(
	FILE *fp,
	union NODE *dbase
){
	UWORD	total;
	struct	ListNode *curr;

	curr = GetFirstNode( dbase );
	total = dbase->listhead.HeadNode.HeadData->lst_tot;
	fwrite( dbase, 1, size of header, fp);
	while( total-- && curr ){
		fwrite( &curr->edit, 1, list->element_size, fp);
		curr = UpNode( curr );
	}
}
 */



/**********************************************************************
 *
 *						LoadList
 *
 *	Description : Save any list to system using FILE *fp
 *	Returns		: 1=ACCEPT  0=CANCEL
 *	Globals		:
 *
 *
void LoadList(
	FILE *fp,
	struct Header *list
){
	UWORD	total;
	struct	ListNode *curr;
	struct	Header th;

	fread( &th, 1, sizeof(struct Header), fp);
	total = th.lst_element_tot;
	curr = list->present;
	while( total-- && curr ){
		curr = AddNodeNext( list, curr );
		// change details here
		fread( &curr->edit, 1, list->element_size, fp);
		curr->edit.marker = 0;
		curr->edit.usage_count = 0;
		EditMarker(list,curr,MARKF_VISIBLE,'S');
		MakeNodeVisible(list,curr,TRUE);

	}
}
 */



/**********************************************************************
 *
 *						SavePalette
 *
 *	Description : Saves the color palette for future use
 *	Returns		: 1=ACCEPT  0=CANCEL
 *	Globals		:
 *
 */
void SavePalette( void ){
#define	MAXCOLORS 16
	FILE	*fp;
	ULONG	Colors32[16*3];
	UBYTE	col,lp;

	if ( (fp=fopen( "mflex.palette","rb+" )) ){
		GetRGB32( Scr->ViewPort.ColorMap, 0, 16, &Colors32[0] );
		fseek( fp,0x30,0 );
		for ( lp=0;lp<MAXCOLORS*3;lp++ ){
			col = Colors32[lp]>>24;
			fputc( col, fp );
		}
		fclose(fp);
	}
}



/**********************************************************************
 *
 *						LoadPalette
 *
 *	Description : loads the color palette into array table
 *	Returns		: 1=ACCEPT  0=CANCEL
 *	Globals		:
 *
 */
void LoadPalette(
	ULONG	Colors32[]
){
#define	MAXCOLORS 16
	FILE	*fp;
	UBYTE	lp,gun;

	if ( fp=fopen( "mflex.palette","rb" ) )
		;
	else
		fp=fopen( "mflex.defpalette","rb" );

	if ( fp ){
		fseek( fp,0x30,0 );
		for ( lp=0;lp<MAXCOLORS*3;lp++ ) {
			gun = fgetc( fp );
			Colors32[ lp+1 ] = ((gun<<24)|(gun<<16)|(gun<<8)|(gun));
		}
		Colors32[lp+2] = 0;
		fclose(fp);
	}

	Colors32[0]=(MAXCOLORS<<16);
	Colors32[ (MAXCOLORS*3)+1 ] = 0;

}






/**********************************************************************
 *
 *						SaveWindowPos
 *
 *	Description : Saves all windows positions
 *	Returns		:
 *	Globals		:
 *
 */
void SaveWindowPos( void ){
	FILE	*fp;

	if ( (fp=fopen( "mflex.winpos","wb+" )) ){
		fprintf(fp,"\nWindowPositions\n");
		fprintf(fp,"EditDisplay\t%d %d %d %d\n",
										EditDisplayLeft,
										EditDisplayTop,
										EditDisplayWidth,
										EditDisplayHeight);
		fprintf(fp,"DTAGSwin\t%d %d %d %d\n",
										DTAGS0Left,
										DTAGS0Top,
										DTAGS0Width,
										DTAGS0Height);
		fprintf(fp,"TimeLine\t%d %d %d %d\n",
										TimeLineLeft,
										TimeLineTop,
										TimeLineWidth,
										TimeLineHeight);

		fclose(fp);
	}
}



/**********************************************************************
 *
 *					LoadWindowPos
 *
 *	Description : reads all windows positions
 *	Returns		:
 *	Globals		:
 *
 */
void LoadWindowPos( void ){
	FILE	*fp;
	UBYTE	temp[80],
			l[8],t[8],w[8],h[8];

	if ( (fp=fopen( "mflex.winpos","rb+" )) )
		;
	else
		fp=fopen( "mflex.defwinpos","rb+" );

	if ( fp ){
		fscanf(fp," %s ",temp);
		fscanf(fp," %s\t%s %s %s %s ",temp,l,t,w,h);
			EditDisplayLeft = atoi(l);
			EditDisplayTop = atoi(t);
//			EditDisplayWidth = atoi(w);
//			EditDisplayHeight = atoi(h);
		fscanf(fp," %s\t%s %s %s %s ",temp,l,t,w,h);
			DTAGS0Left = atoi(l);
			DTAGS0Top = atoi(t);
			DTAGS0Width = atoi(w);
			DTAGS0Height = atoi(h);
		fscanf(fp," %s\t%s %s %s %s ",temp,l,t,w,h);
			TimeLineLeft = atoi(l);
			TimeLineTop = atoi(t);
			TimeLineWidth = atoi(w);
//			TimeLineHeight = atoi(h);
		fclose(fp);
	}
}




/**********************************************************************
 *
 *						CurrElementSet
 *
 *	Description : set element to what ever you want define by BELOW
 *	Returns		: void
 *	Globals		:
 *
 */
UWORD CurrElementSet(
	ULONG	option,				// define in funcs.h
	union	NODE *dbase,		// the DBASE list header to toggle
	UWORD	element				// element # in SHOTLIST
){
	union	NODE *curr;

	// get Node address
	while ( element && !(curr = GetNode(dbase, element)) ){
		element--;
//DEBUG
// printf("FUNCS: @CurrElementSet(), getnode( %08lx, %d )\n",dbase, element);
	}

	if ( option & CES_MARK )
		EditMarker(dbase,curr,MARKF_SELECT,'S');
	else
	if ( option & CES_UNMARK )
		EditMarker(dbase,curr,MARKF_SELECT,'D');
	else
	if ( option & CES_TOGGLEMARK )
		EditMarker(dbase,curr,MARKF_SELECT,'T');

	if ( (option & CES_MAKEPRESENT) && (element) ) {
		dbase->HeadNode.present = curr;					// present pointer  = current pointer
		dbase->HeadNode.HeadData->lst_pos = element;	// current elem #
	}

	return curr->ListNode.marker;
}





/**********************************************************************
 *
 *						IssueHelp
 *
 *	Description : give help to the user via amiga guides
 *	Returns		: void
 *	Globals		:
 *
 */

void IssueHelp(
	LONG	type
){
#define GUIDE_START "run amigaguide mediaflex:HELPGUIDES/"
#define	GUIDE_POST	"pubscreen"
static	UBYTE *cli = {"                                                                                    "};
static	UBYTE guides[][20] = { "main.guide","new.guide" };
static	UBYTE docs[][20] = { "xxx" };
		UBYTE *helpfile,accept;

	if ( type >=0 && type <= 2 ){
		helpfile = &guides[type][0];

		printf("CLI: %s%s %s %s\n",GUIDE_START, helpfile, GUIDE_POST,MFLEX_NAME);
		sprintf(cli,"%s%s %s %s",GUIDE_START, helpfile, GUIDE_POST,MFLEX_NAME);
		SystemTagList( cli,TAG_DONE );
	}
}






// - - - - - - - - - - - -  CUT HERE - - - - - - - - - - - - -

