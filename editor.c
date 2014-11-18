/**********************************************************************
 *********************************************************************
 **
 *
 *						editor.c
 *	Description : handles all aspects of the editor code & interface
 *  Returns		:
 *	Globals		:
 *
 *
 *
 *
 */

#include <exec/types.h>
#include <utility/tagitem.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfx.h>
#include <devices/inputevent.h>
#include <dos/dos.h>

#include <proto/exec.h>				// use amiga library stuff
#include <proto/dos.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/asl.h>

#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "database.h"
#include "funcs.h"			// use functions from funcs.c
#include "editor.h"
#include "DTAGSrequestor.h"
#include "TimeLineGUI.h"

#include "editorGUI.h"
#include "InputReqGUI.h"
#include "DTAGSGUI.h"
#include "DCGUI.h"

//#define	DEBUG	1



//---> globals in EDITOR.C


ULONG	VTR_TimeCode 		= 0;
ULONG	VTR_TCMode			= TC_IN;
//ULONG	FontList_Height 	= 12;

BPTR	debugwin;

struct	BitMap *mybmap,*tempbmap;

static UBYTE *empty_string = {"                                                                                                                            "};
static UBYTE *empty = {"                                                                                                                            "};

//----> editor.c definitions
#define	BUSYLIST	BusyPointer( ListEditorWnd );
#define	NORMALLIST	NormalPointer( ListEditorWnd );













//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


/**********************************************************************
 *
 *						VTR_Handler
 *
 *	Description : handles all VTR funtions!
 *	Returns		:
 *	Globals		: VTR_TimeCode
 *
 */
void VTR_Handler(
	union	NODE *dbase,
	UWORD gadget
){
	union	NODE *curr;
	WORD	active;

	dbase = PRESENTLIST_PTR;
	// get present node address
	curr = dbase->HeadNode.present;

	switch( BIGMODE(dbase) ){
		case MF_REELMODE:
		case MF_EDLMODE:
		case MF_SHOTMODE:
				active = TRUE;
				break;
		default	:
				active = FALSE;
				break;
	}

	if ( active ){
		switch( gadget ){
			case GD_VT_BEG	:	// begining of timecode
				switch ( VTR_TCMode ){
					case TC_IN 	:	VTR_TimeCode = ValidateInTC( dbase, curr, 0, FALSE );break;
					case TC_OUT	:	VTR_TimeCode = ValidateOutTC( dbase, curr, 0, FALSE );break;
					case TC_KEY	:	VTR_TimeCode = ValidateKeyFrameTC( dbase, curr, 0, FALSE );break;
				}
				RefreshVTR();
				break;

			case GD_VT_END	:	// end of timecode
				switch ( VTR_TCMode ){
					case TC_IN	:	VTR_TimeCode = ValidateInTC( dbase, curr, 1<<31, FALSE );break;
					case TC_OUT	:	VTR_TimeCode = ValidateOutTC( dbase, curr, 1<<31, FALSE );break;
					case TC_KEY	:	VTR_TimeCode = ValidateKeyFrameTC( dbase, curr, 1<<31, FALSE );break;
				}
				RefreshVTR();
				break;

			case GD_VT_TOGB	:	// toggle backward
				switch ( VTR_TCMode ){
					case TC_IN	:	VTR_TimeCode = ValidateInTC( dbase, curr, VTR_TimeCode-1, FALSE );break;
					case TC_OUT	:	VTR_TimeCode = ValidateOutTC( dbase, curr, VTR_TimeCode-1, FALSE );break;
					case TC_KEY	:	VTR_TimeCode = ValidateKeyFrameTC( dbase, curr, VTR_TimeCode-1, FALSE );break;
				}
				RefreshVTR();
				break;

			case GD_VT_TOGF	:	// toggle forward
				switch ( VTR_TCMode ){
					case TC_IN	:	VTR_TimeCode = ValidateInTC( dbase, curr, VTR_TimeCode+1, FALSE );break;
					case TC_OUT	:	VTR_TimeCode = ValidateOutTC( dbase, curr, VTR_TimeCode+1, FALSE );break;
					case TC_KEY	:	VTR_TimeCode = ValidateKeyFrameTC( dbase, curr, VTR_TimeCode+1, FALSE );break;
				}
				RefreshVTR();
				break;
		}
	}
}






/**********************************************************************
 *
 *						Node2VTR
 *
 *	Description : changes the VTR gadget
 *					according to the NODE timecodes
 *				  called by: when the MX gadget is pressed
 *	Returns		: void
 *	Globals		:
 *
 */
void Node2VTR(
	UWORD gad
){
	union	NODE *curr;
	LONG	gads[]={ GD_VT_IN, GD_VT_OUT, GD_VT_KEY };
	LONG	lc;

	// find out which my-MX gadget(s) was hit
	switch ( gad ){
		case GD_VT_IN  : VTR_TCMode = TC_IN; break;
		case GD_VT_OUT : VTR_TCMode = TC_OUT; break;
		case GD_VT_KEY : VTR_TCMode = TC_KEY; break;
	}

/*	// un check all gadgets except one selected
	for (lc=1;lc<4;lc++) {
		if ( VTR_TCMode != lc )
			GT_SetGadgetAttrs( ListEditorGadgets[ gads[lc-1] ],ListEditorWnd,0,GTCB_Checked, FALSE, TAG_DONE );
	}
	// set that gadget \/
	GT_SetGadgetAttrs( ListEditorGadgets[ gads[ VTR_TCMode-1] ],ListEditorWnd,0,GTCB_Checked, TRUE, TAG_DONE );
*/

	// get me the current nodes address
	curr = PRESENTNODE_PTR;

	// now do appropriate refresh on VTRTC according to which MX gadget
	switch ( VTR_TCMode ){
						// source in
		case TC_IN	:	VTR_TimeCode = curr->ListNode.in;
						break;
						// source out
		case TC_OUT	:	VTR_TimeCode = curr->ListNode.out;
						break;
						// key frame
		case TC_KEY:	VTR_TimeCode = curr->ListNode.key_frame;
						break;
	}

	RefreshVTR();
}







/**********************************************************************
 *
 *						RefreshVTR
 *	Description : show the time codes of element in time_code-gadget
 *  Returns		:
 *	Globals		:
 *
 */
void RefreshVTR( void ){
static	UBYTE	timecode[] = { "00:00:00:00 " };
	struct	TagItem	GadTags[]={ GTST_String,0,
								TAG_DONE,TAG_DONE };
	GadTags[0].ti_Data = (ULONG)timecode;

	Frame2Text( VTR_TimeCode, timecode );
	GT_SetGadgetAttrsA( ListEditorGadgets[ GD_CurrentTC ],ListEditorWnd,0,GadTags );
}








/**********************************************************************
 *
 *						VTR2Node
 *
 *	Description : changes the VTR gadget
 *					according to the NODE timecodes
 *				  called by: when the MX gadget is pressed
 *	Returns		: void
 *	Globals		:
 *
 */
void VTR2Node(
	UWORD gad
){
	union	NODE *dbase;
	union	NODE *curr;

	LONG	gads[]={ GD_VT_IN, GD_VT_OUT, GD_VT_KEY};
	LONG	lc;

	// get me the current nodes address
	dbase = Project.CurrentWindow->listhead;
	curr = dbase->HeadNode.present;

	// find out which my-MX gadget(s) was hit
	switch ( gad ){
		case GD_VT_IN  :
			VTR_TCMode = TC_IN;
			break;
		case GD_VT_OUT :
			VTR_TCMode = TC_OUT;
			break;
		case GD_VT_KEY :
			VTR_TCMode = TC_KEY;
			break;
	}
/*	// un check all gadgets except one selected
	for (lc=1;lc<4;lc++) {
		if ( VTR_TCMode != lc )
			GT_SetGadgetAttrs( ListEditorGadgets[ gads[lc-1] ],ListEditorWnd,0,GTCB_Checked, FALSE, TAG_DONE );
	}
	// set that gadget \/
	GT_SetGadgetAttrs( ListEditorGadgets[ gads[ VTR_TCMode-1] ],ListEditorWnd,0,GTCB_Checked, TRUE, TAG_DONE );
*/
	switch ( VTR_TCMode ){
		case TC_IN	:	curr->ListNode.in = VTR_TimeCode; break;
		case TC_OUT	:	curr->ListNode.out = VTR_TimeCode; break;
		case TC_KEY :	curr->ListNode.key_frame = VTR_TimeCode; break;
	}

	// refresh stuff!
	TCodes2Gads( dbase );
	RefreshElement( Project.CurrentWindow );
	if ( CURRENTTYPE == MF_EDLMODE )
		EDLInfoRender();
}



/**********************************************************************
 *
 *						ValidateTimeCode
 *
 *	Description : change the VTR according to the NODE timecodes
 *				  called by: when the MX gadget is pressed
 *	Returns		: void
 *	Globals		:
 *
 */
ULONG ValidateTimeCode(
	union	NODE *dbase,
	ULONG	timecode
){
	union	NODE *curr;
	curr = dbase->HeadNode.present;

	switch ( VTR_TCMode ){
			 // source in
		case TC_IN	:	timecode = ValidateInTC( dbase, curr, timecode, FALSE );break;
			 // source out
		case TC_OUT	:	timecode = ValidateOutTC( dbase, curr, timecode, FALSE );break;
			 // key frame
		case TC_KEY	:	timecode = ValidateKeyFrameTC( dbase, curr, timecode, FALSE );break;
			 // duration
		case TC_DUR	:	timecode = ValidateDurationTC( dbase, curr, timecode, FALSE );break;
	}
	return timecode;
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -




/**********************************************************************
 *
 *						CalcLine
 *
 *	Description : Calculate which line ON THE SCREEN we are on in the SHOTLIST/EDL/TRANS
 *	Returns		: line number (0...)
 *	Globals		:
 *
 */
UWORD CalcLine(
	WORD y			// y pixel coordinate
){

	return (UWORD) ( (y-WIN_POSY) / (Project.CurrentWindow->ELEMENT_YSIZE) );
}










/**********************************************************************
 *
 *						MouseRegion
 *
 *	Description : returns the region the mouse is located in!
 *	Returns		: the region code # it is in
 *	Globals		: ListEditorWnd
 *
 */
WORD MouseRegion( void ){
	WORD	x,				// coordinates that are used to test
			y,
			reg=-1,			// the returned region (-1 = NULL)
			n;

WORD	regions[][5] = {
	//  list			bottom		top				right		left
		REG_ANYLIST,	WIN_POSY2, (WIN_POSY-11), WIN3_POSX2,	WIN_POSX,
		REG_SHOTLIST,	WIN_POSY2, (WIN_POSY-11), WIN_POSX2,	WIN_POSX,
		REG_EDLLIST,	WIN_POSY2, (WIN_POSY-11), WIN2_POSX2,	WIN2_POSX,
		REG_TRANSLIST,	WIN_POSY2, (WIN_POSY-11), WIN3_POSX2,	WIN3_POSX
		};

#define NUMOFREGIONS 4

	x = ListEditorWnd->MouseX;
	y = ListEditorWnd->MouseY;

	for ( n=0; n<NUMOFREGIONS; n++) {
		if ( y < regions[n][1] &&
			 y > regions[n][2] &&
			 x < regions[n][3] &&
			 x > regions[n][4] )
			reg = regions[n][0];
	}
	return( reg );
}






// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//						NODE data to----> GADGET data
//
//



/**********************************************************************
 *
 *						TCodes2Gads
 *	Description : show the time codes of element in gadgets
 *  Returns		:
 *	Globals		:
 *
 */
void TCodes2Gads(
	union NODE *dbase
){
	UBYTE	timecode[] = { "00:00:00:00 " };
	struct	TagItem	GadTags[]={ GTST_String,0,
								TAG_DONE,TAG_DONE };
	LONG	tc,active;
	union NODE *curr;
	curr = dbase->HeadNode.present;

	switch( BIGMODE(dbase) ){
		case MF_REELMODE:
		case MF_EDLMODE:
		case MF_SHOTMODE:
				active = TRUE;
				break;
		default	:
				active = FALSE;
				break;
	}


	if ( active ){
		GadTags[0].ti_Data = (ULONG)timecode;

		Frame2Text( curr->ListNode.in, timecode );
		GT_SetGadgetAttrsA( ListEditorGadgets[ GD_SourceIn ],ListEditorWnd,0,GadTags );

		Frame2Text( curr->ListNode.out, timecode );
		GT_SetGadgetAttrsA( ListEditorGadgets[ GD_SourceOut ],ListEditorWnd,0,GadTags );

		Frame2Text( curr->ListNode.key_frame, timecode );
		GT_SetGadgetAttrsA( ListEditorGadgets[ GD_KeyFrame ],ListEditorWnd,0,GadTags );

		tc = curr->ListNode.out - curr->ListNode.in;
		if ( tc < 0 ) tc = -tc;
		Frame2Text( tc, timecode );
		GT_SetGadgetAttrsA( ListEditorGadgets[ GD_Duration ],ListEditorWnd,0,GadTags );
	}
}




/**********************************************************************
 *
 *						Audio2Gads
 *	Description : show the audio/video flags of elements in gadgets
 *  Returns		:
 *	Globals		:
 *
 */
void Audio2Gads(
	union NODE *dbase
){
	LONG	gads[]={ GD_Ch1, GD_Ch2, GD_Ch3, GD_Ch4 },
			lc,
			audio_mode=0, audio_vol1=0, audio_vol2=0;
	union NODE *curr;
	curr = dbase->HeadNode.present;

	switch( BIGMODE(dbase) ){
		case MF_SHOTMODE	:
		case MF_EDLMODE		:
		case MF_REELMODE	:
			audio_mode = Getaudio_mode( dbase, curr );

			if ( audio_mode > 4 ) audio_mode = 0;

			if ( audio_mode ){
				audio_vol1 = Getaudio_volume( dbase, curr, 0 );
				audio_vol2 = Getaudio_volume( dbase, curr, 1 );
			}
			break;
	}

	// un check all gadgets except one selected
	for (lc=1;lc<5;lc++) {
		if ( audio_mode != lc )
			GT_SetGadgetAttrs( ListEditorGadgets[ gads[lc-1] ],ListEditorWnd,0,GTCB_Checked, FALSE, TAG_DONE );
	}

	// do volume levels if audio exists
// DEBUG
// printf("EDITOR: (L407), audio_mode=%d\n",audio_mode);
	if ( audio_mode ){
		GT_SetGadgetAttrs( ListEditorGadgets[ gads[audio_mode-1] ],ListEditorWnd,0,GTCB_Checked, TRUE, TAG_DONE );
		GT_SetGadgetAttrs( ListEditorGadgets[ GD_Vol_L ],ListEditorWnd,0, GTSL_Level, audio_vol1, TAG_DONE );
		GT_SetGadgetAttrs( ListEditorGadgets[ GD_Vol_R ],ListEditorWnd,0, GTSL_Level, audio_vol2, TAG_DONE );
	}
	// else set volumes to ZERO
	else {
		GT_SetGadgetAttrs( ListEditorGadgets[ GD_Vol_L ],ListEditorWnd,0, GTSL_Level, 0, TAG_DONE );
		GT_SetGadgetAttrs( ListEditorGadgets[ GD_Vol_R ],ListEditorWnd,0, GTSL_Level, 0, TAG_DONE );
	}
}








/**********************************************************************
 *
 *						Video2Gads
 *	Description : show the audio/video flags of elements in gadgets
 *  Returns		:
 *	Globals		:
 *
 */
void Video2Gads(
	union NODE *dbase
){
	LONG	gads[]={ GD_Vid1, GD_Vid2, GD_GFX1 },
			lc,video_mode=0;

	union NODE *curr;
	curr = dbase->HeadNode.present;

	switch( BIGMODE(dbase) ){
		case MF_SHOTMODE	:
		case MF_EDLMODE		:
		case MF_REELMODE	:
			video_mode = Getvideo_mode( dbase, curr );
			break;
	}


	// un check all gadgets except one selected
	for (lc=1;lc<4;lc++) {
		if ( video_mode != lc )
			GT_SetGadgetAttrs( ListEditorGadgets[ gads[lc-1] ],ListEditorWnd,0,GTCB_Checked, FALSE, TAG_DONE );
	}

	if ( video_mode )
		GT_SetGadgetAttrs( ListEditorGadgets[ gads[video_mode-1] ],ListEditorWnd,0,GTCB_Checked, TRUE, TAG_DONE );
}






/**********************************************************************
 *
 *						Node2Gads
 *
 *	Description : all element details are placed in the gadgets to be edited
 *					by the the user
 *	Returns		: void
 *	Globals		:
 *
 */
void Node2Gads(
	union NODE *dbase
){
	union NODE *curr;
	curr = dbase->HeadNode.present;

	// Comment2Gadget & Descript2Gadget & Note2Gadget

	switch( BIGMODE( dbase ) ){
		case MF_BIGSHOTMODE :
			RefreshNotes( dbase );
			break;
		case MF_BIGEDLMODE :
			RefreshNotes( dbase );
			break;
		case MF_BIGTAPEMODE :
			RefreshNotes( dbase );
			break;
		case MF_SHOTMODE :
			RefreshDescriptor( dbase );
			RefreshComment( dbase );
			RefreshNotes( dbase );
			TCodes2Gads( dbase );
			Audio2Gads( dbase );		// kill these two later
			Video2Gads( dbase );
			break;
		case MF_EDLMODE	:
			RefreshDescriptor( dbase );
			RefreshComment( dbase );
			RefreshNotes( dbase );
			TCodes2Gads( dbase );
			Audio2Gads( dbase );		// kill these two later
			Video2Gads( dbase );
			break;
		case MF_TAPEMODE :
			RefreshNotes( dbase );
			break;
		case MF_REELMODE :
			RefreshNotes( dbase );
			TCodes2Gads( dbase );
			Audio2Gads( dbase );		// kill these two later
			Video2Gads( dbase );
			break;
	}
}





/**********************************************************************
 *
 *						RefreshDescriptor
 *
 *	Description : Current descriptor is redrawn on to the fake gadgets
 *	Returns		: void
 *	Globals		:
 *
 */
void RefreshDescriptor(
	union NODE *dbase
){
	union NODE *curr;
	UBYTE	*string;

	string = empty;

	curr = dbase->HeadNode.present;

	if ( !(string = Getdescript( dbase, curr )) )
		string = empty;

	WinPutsXY( DCWnd, 5, 11, string, PEN_DESCRIPTION,4);
}




/**********************************************************************
 *
 *						RefreshComment
 *
 *	Description : Current descriptor is redrawn on to the fake gadgets
 *	Returns		: void
 *	Globals		:
 *
 */
void RefreshComment(
	union NODE *dbase
){
	union NODE *curr;
	UBYTE	*string;

	string = empty;

	curr = dbase->HeadNode.present;

	if ( !(string = Getcomment( dbase, curr )) )
		string = empty;

	WinPutsXY( DCWnd, 5, 11+16, string, PEN_DESCRIPTION, 4);
}




/**********************************************************************
 *
 *						RefreshNotes
 *
 *	Description : Current note is redrawn on to the gadget
 *	Returns		: void
 *	Globals		:
 *
 */
void RefreshNotes(
	union NODE *dbase
){
	union NODE *curr;
	UBYTE	*string;

	string = empty;

	curr = dbase->HeadNode.present;

	if ( !(string = Getnote( dbase, curr )) )
		string = empty;

	GT_SetGadgetAttrs( ListEditorGadgets[ GD_Note ],ListEditorWnd,0,
		GTST_String, string, TAG_DONE );
}









// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//						GADGET data to----> NODE data
//
//

/**********************************************************************
 *
 *						AudioGads2Node
 *	Description : extract the Audio flags of the gadgets and place in elements
 *  Returns		:
 *	Globals		:
 *
 */
void AudioGads2Node(
	union NODE *dbase,
	UWORD gad
){
	LONG	chk,video_mode, audio_mode;
	union	NODE *curr;
	curr = dbase->HeadNode.present;

	switch( BIGMODE(dbase) ){
		case MF_REELMODE	:
			video_mode = Getvideo_mode( dbase, curr );
			break;
	}

	if ( !video_mode ){
		GT_GetGadgetAttrs( ListEditorGadgets[ gad ],ListEditorWnd,0,GTCB_Checked, &chk, TAG_DONE );
		if ( chk )
			switch ( gad ){
				case GD_Ch1 :	audio_mode = 1; break;
				case GD_Ch2 :	audio_mode = 2; break;
				case GD_Ch3 :	audio_mode = 3; break;
				case GD_Ch4 :	audio_mode = 4; break;
			}
		else
			audio_mode = 0;
	}

	Setaudio_mode( dbase, curr, audio_mode );

	Audio2Gads( dbase );
}



/**********************************************************************
 *
 *						VolumeGads2Node
 *	Description : extract the Audio flags of the gadgets and place in elements
 *  Returns		:
 *	Globals		:
 *
 */
void VolumeGads2Node(
	union NODE *dbase,
	UWORD gad
){
	LONG	vol,audio_mode;
	union	NODE *curr;
	curr = dbase->HeadNode.present;

	switch( BIGMODE(dbase) ){
		case MF_REELMODE	:
			audio_mode = Getaudio_mode( dbase, curr );
			break;
	}

	if ( audio_mode ){
		GT_GetGadgetAttrs( ListEditorGadgets[ gad ],ListEditorWnd,0,GTSL_Level, &vol, TAG_DONE );
		if ( gad == GD_Vol_L )
			Setaudio_volume( dbase, curr, vol, 0 );
		else
			Setaudio_volume( dbase, curr, vol, 1 );
	}
	Audio2Gads( dbase );
}





/**********************************************************************
 *
 *						VideoGads2Node
 *	Description : extract the Audio flags of the gadgets and place in elements
 *  Returns		:
 *	Globals		:
 *
 */
void VideoGads2Node(
	union NODE *dbase,
	UWORD gad
){
	LONG	chk,audio_mode,video_mode;
	union	NODE *curr;
	curr = dbase->HeadNode.present;

	switch( BIGMODE(dbase) ){
		case MF_REELMODE	:
			audio_mode = Getaudio_mode( dbase, curr );
			break;
	}

	if ( !audio_mode ){
		GT_GetGadgetAttrs( ListEditorGadgets[ gad ],ListEditorWnd,0,GTCB_Checked, &chk, TAG_DONE );
		if ( chk )
			switch ( gad ){
				case GD_Vid1 :	video_mode = 1; break;
				case GD_Vid2 :	video_mode = 2; break;
				case GD_GFX1 :	video_mode = 3; break;
			}
		else
			video_mode = 0;
	}

	Setvideo_mode( dbase, curr, video_mode );

	Video2Gads( dbase );
}








/**********************************************************************
 *
 *						DescriptGad2Node
 *	Description : extract the descriptor of the gadgets and place in elements
 *  Returns		:
 *	Globals		:
 *
 */
void DescriptGad2Node( void ){
	switch( BIGMODE(Project.CurrentWindow->listhead) ){
		case MF_SHOTMODE:
		case MF_EDLMODE:
			OpenDTAGSrequestor();
			break;
	}
}






/**********************************************************************
 *
 *						CommentGad2Node
 *	Description : extract the comments of the gadgets and place in elements
 *  Returns		:
 *	Globals		:
 *
 */
void CommentGad2Node( void ){
	switch( BIGMODE(Project.CurrentWindow->listhead) ){
		case MF_SHOTMODE:
		case MF_EDLMODE:
			OpenDTAGSrequestor();
			break;
	}
}





/**********************************************************************
 *
 *						NoteGad2Node
 *	Description : extract the comments of the note gadget and place in elements
 *  Returns		:
 *	Globals		:
 *
 */
void NoteGad2Node(
	union NODE *dbase
){
	union NODE *curr;
	UBYTE	*string;
	curr = dbase->HeadNode.present;

	GT_GetGadgetAttrs( ListEditorGadgets[ GD_Note ], ListEditorWnd, 0,
			GTST_String, &string, TAG_DONE );

	strcpy( curr->ListNode.NodeData->note, string );

	RefreshElement( Project.CurrentWindow );
}




//- - - - - - - - - - - - TIMECODEs gadgets -->> NODES


/**********************************************************************
 *
 *						SourceInGad2Node
 *	Description : extract the source in gadget and place in node
 *				  modifying the in/out times
 *  Returns		:
 *	Globals		:
 *
 */
void SourceInGad2Node(
	union NODE *dbase
){
	UBYTE	*txt;
	union NODE *curr;

	switch( BIGMODE(dbase) ){
		case MF_SHOTMODE:
		case MF_EDLMODE :
			curr = dbase->HeadNode.present;

			VTR_TCMode = TC_IN;

			GT_GetGadgetAttrs( ListEditorGadgets[ GD_SourceIn ],ListEditorWnd,0,
					GTST_String, &txt, TAG_DONE );
			ValidateInTC( dbase, curr, Text2Frame( txt ), TRUE );
			break;
	}
}





/**********************************************************************
 *
 *						SourceOutGad2Node
 *	Description : extract the sourceout gadget and place in node
 *  Returns		:
 *	Globals		:
 *
 */
void SourceOutGad2Node(
	union NODE *dbase
){
	UBYTE	*txt;
	union	NODE *curr;

	switch( BIGMODE(dbase) ){
		case MF_SHOTMODE:
		case MF_EDLMODE:
			curr = dbase->HeadNode.present;

			VTR_TCMode = TC_OUT;
			GT_GetGadgetAttrs( ListEditorGadgets[ GD_SourceOut ],ListEditorWnd,0,
					GTST_String, &txt, TAG_DONE );
			ValidateOutTC( dbase, curr, Text2Frame( txt ), TRUE );
			break;
	}
}





/**********************************************************************
 *
 *						KeyFrameGad2Node
 *	Description : extract the keyframe gadget and place in node
 *  Returns		:
 *	Globals		:
 *
 */
void KeyFrameGad2Node(
	union NODE *dbase
){
	UBYTE	*txt;
	union NODE *curr;

	switch( BIGMODE(dbase) ){
		case MF_SHOTMODE:
		case MF_EDLMODE:
			curr = dbase->HeadNode.present;

			VTR_TCMode = TC_KEY;
			GT_GetGadgetAttrs( ListEditorGadgets[ GD_KeyFrame ],ListEditorWnd,0,
					GTST_String, &txt, TAG_DONE );
			ValidateKeyFrameTC( dbase, curr, Text2Frame( txt ), TRUE );
			break;
	}
}



/**********************************************************************
 *
 *						DurationGad2Node
 *	Description : extract the duration gadget and place in node
 *  Returns		:
 *	Globals		:
 *
 */
void DurationGad2Node(
	union NODE *dbase
){
	UBYTE	*txt;
	union	NODE *curr;

	switch( BIGMODE(dbase) ){
		case MF_SHOTMODE:
		case MF_EDLMODE:
			curr = dbase->HeadNode.present;

			GT_GetGadgetAttrs( ListEditorGadgets[ GD_Duration ],ListEditorWnd,0,
					GTST_String, &txt, TAG_DONE );
			ValidateDurationTC( dbase, curr, Text2Frame( txt ), TRUE );
			break;
	}
}




/**********************************************************************
 *
 *						CurrentTCGad2Node
 *	Description : extract the current
 *  Returns		:
 *	Globals		:
 *
 */
void CurrentTCGad2Node(
	union NODE *dbase
){
	UBYTE	*txt;
	union	NODE *curr;

	switch( BIGMODE(dbase) ){
		case MF_SHOTMODE:
		case MF_EDLMODE	:
			curr = dbase->HeadNode.present;

			GT_GetGadgetAttrs( ListEditorGadgets[ GD_CurrentTC ],ListEditorWnd,0,
					GTST_String, &txt, TAG_DONE );
			VTR_TimeCode = ValidateTimeCode( dbase, Text2Frame( txt ) );
	}
}










// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


/**********************************************************************
 *
 *						InitialiseEditor
 *
 *	Description : re-display things once initialy
 *	Returns		: void
 *	Globals		:
 *
 */
void InitialiseEditor( void ){
	// allocate a bit map for tempary buffer
	if ( !mybmap)
		mybmap = AllocBitMap( 640,20,4,BMF_INTERLEAVED,NULL );
	if ( !tempbmap)
		tempbmap = AllocBitMap( 640,20,4,BMF_INTERLEAVED,NULL );

	// set up window configurations
	Project.LeftWin.x = WIN_POSX;
	Project.LeftWin.y = WIN_POSY;
	Project.LeftWin.Win_Height = DEF_YPIXELS;
	Project.LeftWin.Win_Width = DEF_XPIXELS;
	Project.LeftWin.Font_Height = 12;
	Project.LeftWin.Font_Width = DEF_FONT_X;

	Project.RightWin.x = WIN2_POSX;
	Project.RightWin.y = WIN2_POSY;
	Project.RightWin.Win_Height = DEF_YPIXELS;
	Project.RightWin.Win_Width = DEF_XPIXELS;
	Project.RightWin.Font_Height = 12;
	Project.RightWin.Font_Width = DEF_FONT_X;

	Project.TransWin.x = WIN3_POSX;
	Project.TransWin.y = WIN3_POSY;
	Project.TransWin.Win_Height = DEF_YPIXELS;
	Project.TransWin.Win_Width = WIN3_POSX2-WIN3_POSX;
	Project.TransWin.Font_Height = 12;
	Project.TransWin.Font_Width = DEF_FONT_X;

	// set up current/previous windows
	Project.CurrentWindow = &Project.LeftWin;
	Project.PreviousWindow = &Project.RightWin;

	// global font for all intuitext is 10
	SetFont( ListEditorWnd->RPort, mainfont10 );

	// draw two little fake gadget boxex where the listno will be shown
	Draw3DBox( ListEditorWnd->RPort, WIN_POSX + 13, WIN_POSY2 +1, 71, 15, 2, 4, PEN_MBEV_SHADOW);
	Draw3DBox( ListEditorWnd->RPort, WIN2_POSX + 13, WIN2_POSY2 +1, 71, 15, 2, 4, PEN_MBEV_SHADOW);

	// set the windows dbases to INDEX | INDEX
	SetWindowList( &Project.LeftWin, LS_INDEX );
	SetWindowList( &Project.RightWin, LS_INDEX );

	// set the windows to SHOTLIST | EDLLIST
	SetWindowDBase( &Project.LeftWin, 0 );
	SetWindowDBase( &Project.RightWin, 1 );

	// lets see them NOW!
	RefreshAllWindows();

	// and what they are too
	RefreshAllTitles();
	RefreshAllListTitles();
	RefreshAllTotals();

	// oohhh yeah.....  show the screen to
	ScreenToFront( Scr );
}


void CloseDownEditor( void ){
	// de-allocate a bit map from tempary buffer
	if ( mybmap )
		FreeBitMap( mybmap );
	if ( tempbmap )
		FreeBitMap( tempbmap );
}














/**********************************************************************
 *
 *						RefreshTotals
 *
 *	Description : redisplay totals on top!
 *	Returns		: void
 *	Globals		:
 *
 */
void RefreshTotals(
	struct WindowStruct *window
){
static	UBYTE	string[DEF_ELEMENT_SIZE];
		UBYTE	*ptr;
		WORD	tc1[4];

//BEGIN FUDGE
	if ( window == &Project.TransWin )
	return;
//END FUDGE

	ptr = string;

	Frame2TCode( window->listhead->HeadNode.HeadData->sel_duration, &tc1[0] );
	ptr+=sprintf(ptr, "Total Time = %02d:%02d:%02d:%02d /",tc1[3], tc1[2], tc1[1],tc1[0]);

	Frame2TCode( window->listhead->HeadNode.HeadData->tot_duration, &tc1[0] );
	ptr+=sprintf(ptr, " %02d:%02d:%02d:%02d",tc1[3], tc1[2], tc1[1],tc1[0]);

	ptr+=sprintf(ptr, "    Elements = %04d /",window->listhead->HeadNode.HeadData->sel_elements);

	ptr+=sprintf(ptr, " %04d",window->listhead->HeadNode.HeadData->scr_tot);

	PutsXY( window->x+WIN_TOTALPOSX, window->y+WIN_TOTALPOSY+DEF_FONT_Y, string, PEN_TOTALS_VALS, PEN_TOTALS_BACK);
}






/**********************************************************************
 *
 *						RefreshAllTotals
 *
 *	Description : redisplay all totals @ top
 *	Returns		: void
 *	Globals		:
 *
 */
void RefreshAllTotals( void ){
	RefreshTotals( &Project.LeftWin );
	RefreshTotals( &Project.RightWin );
//	RefreshTotals( &Project.TransWin );
}

void RefreshCurrTotal( void ){
	RefreshTotals( Project.CurrentWindow );
}




/**********************************************************************
 *
 *						RefreshListTotals
 *
 *	Description : redisplay list totals on top!
 *	Returns		: void
 *	Globals		:
 *
 */
void RefreshListTitle(
	struct WindowStruct *window
){
	WORD	lp;
	UBYTE	lists[][13] =
		 {
			"          :",
			"SHOT INDEX:",
			"EDL  INDEX:",
			"TAPE INDEX:",
			"SHOT LIST :",
			"EDL  LIST :",
			"TAPE LIST :",
			"REEL LIST :",
			"TRANS LIST:",
			"          :"
		 };

	UBYTE	*ptr;

//BEGIN FUDGE
	if ( window == &Project.TransWin ){
		ptr = &lists[8][0];
	PutsXY( window->x+WIN_LISTTITLEPOSX, window->y+WIN_LISTTITLEPOSY+DEF_FONT_Y, ptr, PEN_LISTTITLES_F1, PEN_LISTTITLES_B1);
	return;
	}
//END FUDGE

	lp = TEXTLINE_SIZE-12;
	ptr = window->listhead->HeadNode.HeadData->note;

	while ( *ptr && lp ){
		ptr++;
		lp--;
	}
	//====> fills in line with spaces to pad it
	while( lp-- > 0 )
		*(ptr++) = ' ';
	*ptr=0;


	switch( BIGMODE(window->listhead) ){
		case MF_BIGSHOTMODE	:	ptr = &lists[1][0]; break;
		case MF_BIGEDLMODE	:	ptr = &lists[2][0]; break;
		case MF_BIGTAPEMODE	:	ptr = &lists[3][0]; break;
		case MF_SHOTMODE	:	ptr = &lists[4][0]; break;
		case MF_EDLMODE 	:	ptr = &lists[5][0]; break;
		case MF_TAPEMODE	:	ptr = &lists[6][0]; break;
		case MF_REELMODE	:	ptr = &lists[7][0]; break;
		case MF_TRANSMODE	:	ptr = &lists[8][0];
		 break;
		default				:	ptr = &lists[0][0]; break;
	}
	PutsXY( window->x+WIN_LISTTITLEPOSX, window->y+WIN_LISTTITLEPOSY+DEF_FONT_Y, ptr, PEN_LISTTITLES_F1, PEN_LISTTITLES_B1);

	ptr = window->listhead->HeadNode.HeadData->note;
	PutsXY( window->x+WIN_LISTTITLEPOSX2, window->y+WIN_LISTTITLEPOSY+DEF_FONT_Y, ptr, PEN_LISTTITLES_F2, PEN_LISTTITLES_B2);
}



/**********************************************************************
 *
 *						RefreshAllListTotals
 *
 *	Description : redisplay all list totals @ top
 *	Returns		: void
 *	Globals		:
 *
 */
void RefreshAllListTitles( void ){
	RefreshListTitle( &Project.LeftWin );
	RefreshListTitle( &Project.RightWin );
	RefreshListTitle( &Project.TransWin );
}

void RefreshCurrListTitle( void ){
	RefreshListTitle( Project.CurrentWindow );
}






/**********************************************************************
 *
 *						RefreshTitle
 *
 *	Description : redisplay column bits title according to display bit type
 *	Returns		: void
 *	Globals		:
 *
 */
void RefreshTitle(
	struct	WindowStruct *window,
	ULONG	option
){
	UBYTE	TitleText[][14]	= { "Shot ","Reel"," In          "," Out         ",
								" Duration    ","# ","Descriptor  ","Comments    ",
								"Notes       "};
	UBYTE	*ptr;
	WORD	lp;

//BEGIN FUDGE
	if ( window == &Project.TransWin ){
		PutsXY( window->x+WIN_TOTALPOSX, window->y+WIN_TITLEPOSY+DEF_FONT_Y, "  In           Out          G.P.I. ", PEN_TITLEFORG2, PEN_TITLEBACK2);
		return;
	}
//END FUDGE

	ptr = window->WINHEADDATA_PTR->column_name;

	switch( option ){
		case RT_RECALCSTRING :
			ptr+=sprintf(ptr,"     ");
			switch ( BIGMODE(window->listhead) ){
				case MF_TRANSMODE	:
					sprintf(ptr, "In           Out          G.P.I. ");
					break;
				case MF_SHOTMODE	:
				case MF_EDLMODE		:
					for ( lp=0; lp<DISP_MAXIMUM; lp++)
						if ( window->WINHEADDATA_PTR->column_mask&(1<<lp) )
							ptr+=sprintf(ptr,"%s",TitleText[lp]);
					break;
				case MF_REELMODE	:
					sprintf(ptr, "Reel In           Out          Description_of_Reel");
					break;
				case MF_TAPEMODE	:
					sprintf(ptr, "Tape Description_of_Tape");
					break;
				case MF_BIGSHOTMODE	:
				case MF_BIGEDLMODE	:
					sprintf(ptr, "Shot Description_Title");
					break;
				case MF_BIGTAPEMODE	:
					sprintf(ptr, "Tape Description_Title");
					break;
			}
			lp = TEXTLINE_SIZE;
			ptr = window->WINHEADDATA_PTR->column_name;
			while ( *ptr && lp ){
				ptr++;
				lp--;
			}
			//====> fills in line with spaces to pad it
			while( lp-- > 0 )
				*(ptr++) = ' ';
			*ptr=0;

		case RT_REFRESHSTRING :
			ptr = window->WINHEADDATA_PTR->column_name;

			if ( Project.CurrentWindow == window )
				PutsXY( window->x+WIN_TOTALPOSX, window->y+WIN_TITLEPOSY+DEF_FONT_Y, ptr, PEN_TITLEFORG,  PEN_TITLEBACK);
			else
				PutsXY( window->x+WIN_TOTALPOSX, window->y+WIN_TITLEPOSY+DEF_FONT_Y, ptr, PEN_TITLEFORG2, PEN_TITLEBACK2);
			break;

		}

}




/**********************************************************************
 *
 *						RefreshAllTitles
 *
 *	Description : redisplay all window titles according to display type
 *	Returns		: void
 *	Globals		:
 *
 */
void RefreshAllTitles( void ){
	RefreshTitle( &Project.LeftWin, RT_RECALCSTRING );
	RefreshTitle( &Project.RightWin, RT_RECALCSTRING );
	RefreshTitle( &Project.TransWin, RT_RECALCSTRING );
}

void RefreshCurrTitle( void ){
	RefreshTitle( Project.CurrentWindow, RT_RECALCSTRING );
}




//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


/**********************************************************************
 *
 *						RefreshElementXY
 *
 *	Description : display one List element(current) on given Y line number
 *	Returns		: void
 *	Globals		:
 *
 */
void RefreshElementXY(
	struct	WindowStruct *window,
	union NODE *curr,
	WORD	linenum				// line # to show at
){
static	UBYTE	*line={"                                                                                                   "};
		UBYTE	*stat={"    "},	// 1*space 2*symbol 1*space
				*temp;

		union 	NODE *dbase;
		ULONG	display = window->listhead->HeadNode.HeadData->column_mask,
				audio_mode,video_mode;
		WORD	tc1[4],
				selA,selB, loop, ycoord, cnt;

	UseWindowFontSize();

	dbase 	= window->listhead;
	loop 	= ELEMENT_SIZE;				// horizontal x char size
	ycoord  = window->y + (linenum * (window->ELEMENT_YSIZE));

//BEGIN FUDGE
	if ( window == &Project.TransWin ){
		selA = PEN_ELEMENT_SILENT;			// else set plain color
		selB = PEN_ELEMENT_VISIBLE;
		PutsXY( window->x+(0*FONT_XSIZE)+2, ycoord+window->Font_Height, "  00:00:00:00  00:00:00:00  Cut", selA, selB );
		SetFont( ListEditorWnd->RPort, mainfont10 );
		return;
	}
//END FUDGE


	if ( curr && window->listhead->HeadNode.HeadData->lst_pos ){
		temp = line;
		switch ( BIGMODE(window->listhead) ){
			case MF_REELMODE :
				temp+=sprintf(temp, "%04d", Getindex( dbase, curr ) );

				Frame2TCode( curr->ListNode.in, &tc1[0] );
				temp+=sprintf(temp, " %02d:%02d:%02d:%02d ",tc1[3], tc1[2], tc1[1],tc1[0]);

				Frame2TCode( curr->ListNode.out, &tc1[0] );
				temp+=sprintf(temp, " %02d:%02d:%02d:%02d ",tc1[3], tc1[2], tc1[1],tc1[0]);

				temp+=sprintf(temp, " %.52s ", Getnote( dbase, curr ) );
				break;

			case MF_TAPEMODE :
			case MF_BIGSHOTMODE :
			case MF_BIGEDLMODE :
			case MF_BIGTAPEMODE :
				temp+=sprintf(temp, "%04d ", Getindex( dbase, curr ) );
				temp+=sprintf(temp, "%.52s ", Getnote( dbase, curr ) );
				break;

			case MF_EDLMODE  :
			case MF_SHOTMODE :
				if ( display&DISP_EVENT )
					temp+=sprintf(temp, "%04d ", Getindex(dbase, curr) );

				if ( display&DISP_REEL )
					temp+=sprintf(temp, "%4d", GetReelindex( dbase, curr) );

				if ( display&DISP_IN ){
					Frame2TCode( curr->ListNode.in, &tc1[0] );
					temp+=sprintf(temp, " %02d:%02d:%02d:%02d ",tc1[3], tc1[2], tc1[1],tc1[0]);
				}
				if ( display&DISP_OUT ){
					Frame2TCode( curr->ListNode.out, &tc1[0] );
					temp+=sprintf(temp, " %02d:%02d:%02d:%02d ",tc1[3], tc1[2], tc1[1],tc1[0]);
				}
				if ( display&DISP_DURATION ){
					Frame2TCode( curr->ListNode.out - curr->ListNode.in, &tc1[0] );
					temp+=sprintf(temp, " %02d:%02d:%02d:%02d ",tc1[3], tc1[2], tc1[1],tc1[0]);
				}
				if ( display&DISP_USECNT ){
					cnt = Getusage_count( dbase, curr );

					if ( cnt > 9)
						temp+=sprintf(temp, "# ",cnt );
					else if ( cnt > 0 )
						temp+=sprintf(temp, "%.1d ",cnt );
					else if ( cnt < 1 )
						temp+=sprintf(temp, "- ",cnt );
				}

				if ( display&DISP_DESCRIPT )
						temp+=sprintf(temp, "%.52s", Getdescript( dbase, curr ) );
				else
				if ( display&DISP_COMMENT )
						temp+=sprintf(temp, "%.52s", Getcomment( dbase, curr ) );
				else
				if ( display&DISP_NOTE )
						temp+=sprintf(temp, "%.52s", Getnote( dbase, curr ) );
				break;

		}

		switch ( BIGMODE(window->listhead) ){
			case MF_TAPEMODE :
			case MF_BIGSHOTMODE :
			case MF_BIGEDLMODE :
			case MF_BIGTAPEMODE :
				selA = PEN_ELEMENT_SILENT;

				if (curr->ListNode.marker & MARKF_SELECT)
					selB = PEN_ELEMENT_SELECT;
				else
				if (curr->ListNode.marker & MARKF_FOUND)
					selB = PEN_ELEMENT_FOUND;
				else
				if (curr->ListNode.marker & MARKF_VISIBLE)
					selB = PEN_ELEMENT_VISIBLE;
				else
					selB = PEN_ELEMENT_HIDDEN;

				*(stat+1) = GFXCHAR_SILENT;
				*(stat+2) = GFXCHAR_SILENT;
				break;

			case MF_EDLMODE :
			case MF_SHOTMODE :
			case MF_REELMODE :
				audio_mode = Getaudio_mode( dbase, curr );
				video_mode = Getvideo_mode( dbase, curr );

				//====> select what color to use to display element
				//====> if marked set backround color
				if (curr->ListNode.marker & MARKF_SELECT)
					selB = PEN_ELEMENT_SELECT;
				else
				if (curr->ListNode.marker & MARKF_FOUND)
					selB = PEN_ELEMENT_FOUND;
				else
				if (curr->ListNode.marker & MARKF_VISIBLE)
					selB = PEN_ELEMENT_VISIBLE;
				else
					selB = PEN_ELEMENT_HIDDEN;

				if ( audio_mode ){		// if audio exists set color
					selA = PEN_ELEMENT_AUDIO;
					*(stat+1) = GFXCHAR_AUDIO+((audio_mode-1)*2);
					*(stat+2) = GFXCHAR_AUDIO+((audio_mode-1)*2)+1;
				}
				else
				if ( video_mode == 3){		// if video exists set color
					selA = PEN_ELEMENT_GRAPHIC;
					*(stat+1) = GFXCHAR_GRAPHIC;
					*(stat+2) = GFXCHAR_GRAPHIC+1;
				}
				else
				if ( video_mode ){		// if video exists set color
					selA = PEN_ELEMENT_VIDEO;
					*(stat+1) = GFXCHAR_VIDEO+((video_mode-1)*2);
					*(stat+2) = GFXCHAR_VIDEO+((video_mode-1)*2)+1;
				}
				else {
 		 			selA = PEN_ELEMENT_SILENT;			// else set plain color
					*(stat+1) = GFXCHAR_SILENT;
					*(stat+2) = GFXCHAR_SILENT;
				}
				break;
		}

		temp=line;							// goes to end of line
		while( *temp && loop ){
			temp++;
			loop--;
		}
		//====> fills in line with spaces to pad it
		while( loop-- > 0 )
			*(temp++) = ' ';
		*temp=0;

		//====> then _display it!
		// clear |> cursor
		PutsXY( window->x, ycoord+window->Font_Height, " ", selA, PEN_ELEMENT_BACK );

		// show datatype symbols
		PutsXY( window->x+(1*FONT_XSIZE)+2, ycoord+window->Font_Height, stat+1, selA, PEN_ELEMENT_BACK );

		// show text info including timecodes if there!
		PutsXY( window->x+(4*FONT_XSIZE)+2, ycoord+window->Font_Height, line, selA, selB );
	}

	//====> just show blank empty line
	else {
		SetAPen( ListEditorWnd->RPort, PEN_ELEMENT_BACK );
		RectFill( ListEditorWnd->RPort, window->x-1, ycoord, window->x+BLITBOB_XSIZE+3, ycoord+BLITBOB_YSIZE );
	}

	SetFont( ListEditorWnd->RPort, mainfont10 );
}








/**********************************************************************
 *
 *						RefreshXElement
 *	Description : o this displays the specified element if it exists
 *					in the list window
 *  Returns		: NULL
 *	Globals		: usual
 *
 */
void RefreshXElement(
	struct	WindowStruct	*window,		// the window to display at
	WORD	element_pos						// the list element number
){
	union	NODE *curr;
	WORD	line;

	if ( window->listhead->HeadNode.HeadData->scr_tot ){
		// calc new line pos
		line = (element_pos - window->listhead->HeadNode.HeadData->scr_top);
		// draw element line
		if ( line < MAX_ELEMENT_LINES ){
//DEBUG
// printf("EDITOR: @RefreshXElelement() GetNode( %d)\n",element_pos);
			curr = GetNode( window->listhead, element_pos );
			RefreshElementXY( window, curr, line );
		}
	}
}






/**********************************************************************
 *
 *						RefreshElement
 *	Description : o this redraws the current list element being edited
 *					in the list window
 *				  o it internally keeps track of Y-POS
 *  Returns		: NULL
 *	Globals		: usual
 *
 */
void RefreshElement(
	struct	WindowStruct *window		// the window to display at
){
//DEBUG
// printf("EDITOR: (L1395) window->listhead=%08lx\n",window->listhead);
	RefreshXElement( window , window->listhead->HeadNode.HeadData->lst_pos );
}








/**********************************************************************
 *
 *						RefreshElementAll
 *	Description : this displays the current list element being edited
 *					in the list window plus refreshes the gadgets
 *  Returns		: NULL
 *	Globals		: usual
 *
 */
void RefreshElementAll(
	struct	WindowStruct	*window
){

	if ( window->listhead->HeadNode.HeadData->lst_pos ){
		ReAlignPresent( window );
		HiLightPresent( window );
		Node2Gads( window->listhead );
	}
	RefreshTotals( window );
}








/**********************************************************************
 *
 *						InteliRefreshWindow
 *
 *	Description : re-display list in window at current position with highlights
 *					Shows nothing if list empty.
 *					PLUS: it does it inteligently
 *	Returns		: void
 *	Globals		:
 *
 */
void InteliRefreshWindow(
	struct	WindowStruct *window,	// window to refresh
	ULONG	option					// how to do it
#define IRW_INTELIREFRESH	(1)
#define IRW_REFRESHALL		(2)
 ){
static	last_scr_top,
		last_scr_tot;

	WORD	iter		// loop counter
			,y  =0;		// y coordinate
	union	NODE	*curr=0;

	InteliRefreshVertSlider( window );

	InteliRefreshHorizSlider( window );

	iter = MAX_ELEMENT_LINES;				//=> set loop number

//DEBUG
 printf("EDITOR: @InteliRefreshWindow(), iter = %d\n",iter);

	//====> if there are elements in shotlist get address of node
	if(window) {
	if ( window->listhead->HeadNode.HeadData->lst_tot )
		curr = GetNode( window->listhead, window->listhead->HeadNode.HeadData->scr_top );
	}
else printf("\n\n\nWINDOW = NULL \n\n\n");
	switch ( option ){

		case IRW_INTELIREFRESH :
				if (last_scr_top != window->listhead->HeadNode.HeadData->scr_top ||
					last_scr_tot != window->listhead->HeadNode.HeadData->scr_tot )
					;		// procede
				else
					break;	// cancel

		case IRW_REFRESHALL :
				//====> draw loop that redraws all elements
				while( iter-- ) {
					RefreshElementXY( window, curr, y );
					curr = UpNode( window->listhead, curr );
					y++;
				}
				break;
	}

	RefreshElementAll( window );

	last_scr_top = window->listhead->HeadNode.HeadData->scr_top;
	last_scr_tot = window->listhead->HeadNode.HeadData->scr_tot;
}




/**********************************************************************
 *
 *						ClearListWindow
 *
 *	Description : re-display list in window at current position with highlights
 *					Shows nothing if list empty.
 *					PLUS: it does it inteligently
 *	Returns		: void
 *	Globals		:
 *
 */
void ClearListWindow(
	struct	WindowStruct *window	// window to refresh
){
	SetAPen( ListEditorWnd->RPort, PEN_ELEMENT_BACK );
	RectFill( ListEditorWnd->RPort, window->x, window->y, window->x+XPIXELS_AVAIL+DEF_TYPE_XSIZE-3, window->y+YPIXELS_AVAIL );
}

void ClearAllWindows( void ){
	ClearListWindow( &Project.LeftWin );
	ClearListWindow( &Project.RightWin );
}







void RefreshAllWindows( void ){
	RefreshLeftWindow();
	RefreshRightWindow();
	RefreshTransWindow();
}

void RefreshCurrWindow( void ){
	if ( Project.PreviousWindow->listhead == Project.CurrentWindow->listhead )
		RefreshPrevWindow();
	InteliRefreshWindow( Project.CurrentWindow, IRW_REFRESHALL );
}

void RefreshPrevWindow( void ){
	InteliRefreshWindow( Project.PreviousWindow, IRW_REFRESHALL );
}

void RefreshLeftWindow( void ){
	InteliRefreshWindow( &Project.LeftWin, IRW_REFRESHALL );
}

void RefreshRightWindow( void ){
	InteliRefreshWindow( &Project.RightWin, IRW_REFRESHALL );
}

void RefreshTransWindow( void ){
//	InteliRefreshWindow( &Project.TransWin, IRW_REFRESHALL );
}

// - - - >


void InteliRefreshAllWindows( void ){
	InteliRefreshLeftWindow();
	InteliRefreshRightWindow();
//	InteliRefreshTransWindow();
}

void InteliRefreshCurrWindow( void ){
	if ( Project.PreviousWindow->listhead == Project.CurrentWindow->listhead )
		InteliRefreshPrevWindow();
	InteliRefreshWindow( Project.CurrentWindow, IRW_INTELIREFRESH );
}


void InteliRefreshPrevWindow( void ){
	InteliRefreshWindow( Project.PreviousWindow, IRW_INTELIREFRESH );
}

void InteliRefreshLeftWindow( void ){
	InteliRefreshWindow( &Project.LeftWin, IRW_INTELIREFRESH );
}

void InteliRefreshRightWindow( void ){
	InteliRefreshWindow( &Project.RightWin, IRW_INTELIREFRESH );
}

void InteliRefreshTransWindow( void ){
	InteliRefreshWindow( &Project.TransWin, IRW_INTELIREFRESH );
}





// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -





/**********************************************************************
 *
 *						UseVertSliderValue
 *
 *	Description : reposition list depending on VERT slider
 *	Returns		: void
 *	Globals		:
 *
 */
void UseVertSliderValue(
	struct	WindowStruct *window,
	ULONG	value
){
	window->listhead->HeadNode.HeadData->scr_top = value;
	InteliRefreshWindow( window, IRW_INTELIREFRESH );
}







/**********************************************************************
 *
 *						UseSliderValue
 *
 *	Description : select new list depending on HORIZ slider
 *	Returns		: void
 *	Globals		:
 *
 */
void UseHorizSliderValue(
	struct	WindowStruct *window,
	ULONG	value
){
	ShowCurrentMode();
	SetWindowList( window, value );
}










/**********************************************************************
 *
 *						InteliRefreshVertSlider
 *
 *	Description : vertical slider of the List is redrawn
 *	Returns		: void
 *	Globals		:
 *
 */
void InteliRefreshVertSlider(
	struct WindowStruct *window
){
	LONG	top,tot,GadID;


	if( window == &Project.LeftWin ) {
		GadID = GD_LEFT_POS;
	}
	else
	if( window == &Project.RightWin ) {
		GadID = GD_RIGHT_POS;
	}

	top = window->listhead->HeadNode.HeadData->scr_top;
	tot = window->listhead->HeadNode.HeadData->lst_tot;

	if ( top>0 )
		top--;

	GT_SetGadgetAttrs( ListEditorGadgets[ GadID ],ListEditorWnd,0,
		GTSC_Top,  top,	GTSC_Total,tot,	TAG_DONE );
}









/**********************************************************************
 *
 *						InteliRefreshHorizSlider
 *
 *	Description : horizontal slider of the List is redrawn depending on which
 *				  lists is being edited.
 *	Returns		: void
 *	Globals		:
 *
 */
void InteliRefreshHorizSlider(
	struct WindowStruct *window
){
static	ULONG	top, tot;
		ULONG	set;

	switch( BIGMODE(window->listhead) ){
		case MF_BIGTAPEMODE:
		case MF_BIGSHOTMODE:
		case MF_BIGEDLMODE:
			tot = Project.Head[ MODE( window->listhead ) ].HeadNode.HeadData->lst_tot;
			top = 0;
			set = TRUE;
			break;
		case MF_REELMODE:
			tot=0;
			top=0;
			set = TRUE;
			break;
		case MF_TAPEMODE:
		case MF_SHOTMODE:
		case MF_EDLMODE:
			tot = Project.Head[ MODE( window->listhead ) ].HeadNode.HeadData->lst_tot;
			top = GetHeadNum( window->listhead );
			set = TRUE;
			break;
		default			:
			set = FALSE;
			break;
	}

	if( set ){
		if( window == &Project.LeftWin )
			GT_SetGadgetAttrs( ListEditorGadgets[ GD_LEFT_LISTSEL ],ListEditorWnd,0,
				GTSL_Level, top,
				GTSL_Max, tot,
				GTSL_Min,0,
				TAG_DONE );
		else
		if( window == &Project.RightWin )
			GT_SetGadgetAttrs( ListEditorGadgets[ GD_RIGHT_LISTSEL ],ListEditorWnd,0,
				GTSL_Level, top,
				GTSL_Max, tot,
				GTSL_Min,0,
				TAG_DONE );
	}
}

















// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -











// replace old screen region from temp bitmap
#define	BlitBuf2Scr	BltBitMap( tempbmap, 0,0, Scr->RastPort.BitMap, blitx,blity, \
						BLITBOB_XSIZE,BLITBOB_YSIZE,0xc0,0xffff,NULL);

// place new screen region to temp bitbap
#define	BlitScr2Buf	BltBitMap( Scr->RastPort.BitMap, blitx,blity,tempbmap,0,0,	\
						BLITBOB_XSIZE,BLITBOB_YSIZE,0xc0,0xffff,NULL);

// place element bitmap to new screen region
#define	BlitObj2Scr	BltBitMap( mybmap, 0,0, Scr->RastPort.BitMap, blitx,blity,	\
						BLITBOB_XSIZE,BLITBOB_YSIZE,0xc0,0xffff,NULL);

// place element bitmap to buffer
#define	BlitScr2Obj	BltBitMap( Scr->RastPort.BitMap, blitx,blity,mybmap,0,0,	\
		BLITBOB_XSIZE,BLITBOB_YSIZE,0xc0,0xffff,NULL);


/**********************************************************************
 *
 *						MouseButtonHandler
 *
 *	Description : does all mouse-to-user I/O functions (click etc..)
 *	Returns		: void
 *	Globals		:
 *
 */
void MouseButtonHandler(
	UWORD event,					// mouse button event
	UWORD qual						// qualifier
){
static	ULONG	down=0,
				lastmode=0,last_elno=0,
				pressededitmode=0, pressed_elno=0;

static	union NODE *pressedlisthead;
static	UWORD	mx,my,				// temp mousex/y
				blitx,blity,		// any x y
				origx,origy,		// original pointer x y
				origcx, origcy;		// original corner x y
struct	WindowStruct *window, *lastwindow, *pressedwindow;
	UWORD		Selected_el_no,		// element no. of element clicked on.
				Selected_scr_no,	// element no. of element clicked on.
				Selected_scr_pos;	// element line on screen.
	WORD		delta,dir,bothwindows=FALSE;



	window = Project.CurrentWindow;
	mx = ListEditorWnd->MouseX;
	my = ListEditorWnd->MouseY;
	Selected_scr_no = CalcLine( my ) + PRESENTLISTDATA_PTR->scr_top;
	Selected_el_no = ListNo( PRESENTLIST_PTR, Selected_scr_no );
	Selected_scr_pos = Selected_scr_no - PRESENTLISTDATA_PTR->scr_top; //screen pos

//DEBUG
// printf("EDITOR: (L1732) my = %d\n",my);
// printf("EDITOR: (L1733) CalcLine(my) = %d\n",CalcLine(my) );
// printf("EDITOR: (L1734) SelScrno = %d\n",Selected_scr_no);

	if ( Project.PreviousWindow->listhead == window->listhead )
		bothwindows = TRUE;


	if ( 1 ){
		if (qual & IEQUALIFIER_LSHIFT ||
			qual & IEQUALIFIER_RSHIFT ||
			qual & IEQUALIFIER_CAPSLOCK ){
			switch ( event ) {
				case MB_PRESSED :
					if ( down == 0 ){	// first time left pressed with shift toggle it
						RefreshCursorClr( window );
						if ( pressed_elno == Selected_el_no )
							lastmode = CurrElementSet( CES_MARK|CES_MAKEPRESENT, PRESENTLIST_PTR, Selected_el_no );
						else {
							lastmode = CurrElementSet( CES_TOGGLEMARK|CES_MAKEPRESENT, PRESENTLIST_PTR, Selected_el_no );
							pressed_elno = Selected_el_no;
						}

						RefreshXElement( window, Selected_el_no );
						RefreshAllTotals();

						origcx = window->x + 7;
						origcy = window->y + (Selected_scr_pos * (window->ELEMENT_YSIZE));
						origx = mx;
						origy = my;
						blitx = origcx;
						blity = origcy;
						BlitScr2Obj
						BlitScr2Buf
						down++;
						pressedwindow = window;
						lastwindow = window;
					}
									// while holding down left & shift
					else
					if ( down ) {
						if ( last_elno != Selected_el_no ||
							 lastwindow != window )
							RefreshCursorClr( window );

						// mark elements inbetween
						delta = Selected_el_no - last_elno;
						if ( delta < 0){
							dir = -1;
							delta = -delta;	}
						else
							dir =  1;
						while ( delta-- ){
							last_elno+= dir;
							if (lastmode & MARKF_SELECT )
								CurrElementSet( CES_MARK, PRESENTLIST_PTR, last_elno );
							else
								CurrElementSet( CES_UNMARK, PRESENTLIST_PTR, last_elno );
							RefreshXElement( window, last_elno );
						}

						SetPresent( window, Selected_el_no );		// cursor follows pointer position

						RefreshCursor( window );

						RefreshAllTotals();
					}

					last_elno = Selected_el_no;
					lastwindow = window;
					break;

				case MB_RELEASED:
					// refresh on first release of button while shift
					if ( down ){
						BlitBuf2Scr
						RefreshCursorClr( window );
						if ( PRESENTLISTDATA_PTR->sel_elements == 0 ){
							CurrElementSet( CES_MAKEPRESENT, PRESENTLIST_PTR, Selected_el_no );
							RefreshElementAll( window );
						}
						RefreshCursor( window );
					}
					Node2Gads( window->listhead );
					down = 0;
					last_elno = 0;
					pressed_elno = 0;
					break;
			}
		}

	// - - - - - - - -
	// UNSHIFTED DRAG (NORMAL DRAG) or shall we say just clicking on item
	// - - - - - - - -
		else {
			switch ( event ){
				case MB_PRESSED:
							 		// first press of object
					if ( down == 0 &&
						 PRESENTLISTDATA_PTR->scr_tot &&
						 Selected_scr_no <= PRESENTLISTDATA_PTR->scr_tot ) {

						pressededitmode = CURRENTBIGMODE;
						pressedlisthead = window->listhead;
						pressed_elno = Selected_el_no;
						pressedwindow = window;

						if ( PRESENTLISTDATA_PTR->sel_elements > 1 ){
							UnMarkAll( window->listhead , MARKF_SELECT );
							SetPresent( window, Selected_el_no );
							RefreshCurrWindow();
						}
						else
						if ( PRESENTLISTDATA_PTR->lst_pos != Selected_el_no ){
							UnHiLightPresent( window );
							SetPresent( window, Selected_el_no );
							HiLightPresent( window );	//draw element selected
						}

						origcx = window->x + 7;
						origcy = window->y + (Selected_scr_pos * (window->ELEMENT_YSIZE));
						origx = mx;
						origy = my;
						blitx = origcx;
						blity = origcy;
						BlitScr2Obj
						BlitScr2Buf
						down++;
					}
					else			// held down press of object
					if ( down ) {
						WaitTOF();
						BlitBuf2Scr
						blitx = origcx + (mx - origx);
						blity = origcy + (my - origy);

						Selected_scr_no = ( ((((window->ELEMENT_YSIZE)>>1)+blity)-WIN_POSY) / (window->ELEMENT_YSIZE) ) + PRESENTLISTDATA_PTR->scr_top;
						Selected_el_no = ListNo( PRESENTLIST_PTR, Selected_scr_no );

						if ( last_elno != Selected_el_no ){
							RefreshCursorClr( window );
							SetPresent( window, Selected_el_no );		// cursor follows pointer position
							RefreshCursor( window );
						}
						BlitScr2Buf
						BlitObj2Scr
					}
					last_elno = Selected_el_no;
					lastwindow = window;
					break;

										// let go of object

				case MB_RELEASED :
					if ( down ) {
						BlitBuf2Scr

						// XFER shot
						if ( (pressededitmode == MF_SHOTMODE) &&
							 (CURRENTBIGMODE == MF_EDLMODE ) ) {
							SetPresent( Project.PreviousWindow, pressed_elno );
							SetPresent( window, Selected_el_no );
							Xfer2EDL( pressededitmode );
							EDLInfoRender();
						}
						// MOVE shot
						else {
							if ( pressededitmode == CURRENTBIGMODE ){
								if ( (Selected_el_no != pressed_elno) ){
									MoveElement( pressedlisthead, window->listhead,
										GetNode(pressedlisthead,pressed_elno), PRESENTNODE_PTR,
										pressed_elno );
									if ( CURRENTMODE == MF_EDLMODE ) {
										RecalcEDLTimes( window->listhead, window->listhead );
										EDLInfoRender();
									}
									if ( Selected_el_no == pressed_elno-1 )
										SetPresent( window, pressed_elno );
									RefreshAllWindows();		//used to be CurrWindow
								}
							}
						}
					}
					Node2Gads( window->listhead );

					down = 0;
					last_elno = 0;
					break;

				case MB_DUBCLICK :
					if ( Selected_el_no == pressed_elno ) {
						switch( CURRENTBIGMODE ) {
							case MF_BIGTAPEMODE :
							case MF_BIGSHOTMODE :
							case MF_BIGEDLMODE  :
								SetWindowList( window, Selected_el_no );
								InteliRefreshHorizSlider( window );
								RefreshCurrWindow();
								break;
						}
					}
					else {
						MouseButtonHandler( MB_PRESSED , qual );
						return;
					}
					break;
			}
		}
	}
}








/**********************************************************************
 *
 *						MouseHandler
 *
 *	Description : finds out which region on screen it is on SHOT/EDL/TRANS
 *					and does all mouse-to-user I/O functions (click etc..)
 *	Returns		: void
 *	Globals		:
 *
 */
void MouseHandler(
	UWORD event,			// mouse button event

	UWORD qual				// qualifier
//	UWORD mx,
//	UWORD my
){
static ULONG lastbigmode=0;


	switch( MouseRegion() ) {
		case REG_SHOTLIST	:
						if ( event == MB_PRESSED )
							CurrentModeNext( CM_SETMODE, 0 );
						MouseButtonHandler( event,qual );
						break;

		case REG_EDLLIST	:
						if ( event == MB_PRESSED )
							CurrentModeNext( CM_SETMODE, 1 );
						MouseButtonHandler( event,qual );
						break;

		case REG_TRANSLIST	:
						if ( event == MB_PRESSED )
							CurrentModeNext( CM_SETMODE, 2 );
						break;

		case REG_ANYLIST	:
						if ( event == MB_PRESSED )
							MouseButtonHandler( event,qual );
						break;
	}

	if ( event == MB_PRESSED &&
		 lastbigmode != CURRENTBIGMODE )
		ShowCurrentMode();

	if ( event == MB_RELEASED )
		MouseButtonHandler( event,qual );

	lastbigmode = CURRENTBIGMODE;
}






/**********************************************************************
 *
 *						CurrentModeNext
 *
 *	Description : switches the current edit mode to the next one
 *	Returns		: void
 *	Globals		:
 *
 */
void CurrentModeNext(
	UWORD	type,
	WORD	dir
){
#define	MAXMODES 2
static	cmode=0;
struct WindowStruct	*windows[] = {&Project.LeftWin, &Project.RightWin };

	// decide what to do a:nextmode b:specifiedmode
	switch (type){
		case CM_INCMODE :
				cmode+=dir;
			break;
		case CM_SETMODE :
				cmode = dir;
			break;
	}
	//range check
	if (cmode < 0)
		cmode = 0;
	if (cmode >= MAXMODES)
		cmode = MAXMODES-1;

	if( Project.CurrentWindow != windows[cmode] ){
		Project.PreviousWindow = Project.CurrentWindow;
		Project.CurrentWindow = windows[cmode];
		Project.CurrentList[ MODE(Project.CurrentWindow->listhead) ] = Project.CurrentWindow->listhead;
		ShowCurrentMode();
	}
}







/**********************************************************************
 *
 *						SetWindowListNext
 *
 *	Description : sets the windows database list to next and refresh (edl1/edl2/...)
 *	Returns		: void
 *	Globals		:
 *
 */
void SetWindowListNext(
	struct WindowStruct *window
){
	SetWindowList( window, LS_PLUS );
}






/**********************************************************************
 *
 *						SetWindowListPrev
 *
 *	Description : sets the windows database previous list and refresh (edl1/edl2/...)
 *	Returns		: void
 *	Globals		:
 *
 */
void SetWindowListPrev(
	struct WindowStruct *window
){
	SetWindowList( window, LS_MINUS );
}






/**********************************************************************
 *
 *						SetWindowList
 *
 *	Description : sets the windows database list and refresh (edl1/edl2/...)
 *	Returns		: void
 *	Globals		:
 *
 */
void SetWindowList(
	struct WindowStruct *window,
	UWORD  value					// 0=index 1=main 2=scratch 3=1st
){
	UBYTE	*string={"LIST:000  "};
	WORD	max,cpos,active;

	switch( BIGMODE( Project.CurrentWindow->listhead) ){
		case MF_REELMODE:
			max = 0;
			cpos = 0;
			break;
		default:
			max = Project.Head[ MODE(Project.CurrentWindow->listhead) ].HeadNode.HeadData->scr_tot;
			cpos = Ptr2ListNo( &Project.Head[ MODE(Project.CurrentWindow->listhead) ], Project.CurrentWindow->listhead );
			break;
	}

//DEBUG
 printf("EDITOR: SetWindowList(), max=%d cpos=%d\n",max,cpos);

	switch( value ){
		case LS_SAME	:
			active = FALSE;
			break;
		case LS_BEGIN	:
			value = LS_INDEX;
			active = TRUE;
			break;
		case LS_END		:
			value =	max;
			active = TRUE;
			break;
		case LS_MINUS	:
			if ( cpos > 0 ) {
				value = cpos-1;
				active = TRUE;
			}
			else
				active = FALSE;
			break;
		case LS_PLUS	:
			if ( cpos < max ) {
				value = cpos+1;
				active = TRUE;
			}
			else
				active = FALSE;
			break;
		case LS_INDEX:
		case LS_MAIN:
		case LS_SCRATCH:
			active = TRUE;
			break;
		default:
			active = TRUE;
			break;
	}

	if ( active ){
		Project.CurrentList[ MODE(Project.CurrentWindow->listhead) ] = Project.CurrentWindow->listhead;
		sprintf(string+5,"%3d",value);
		PutsXY( window->x+20, WIN_POSY2+DEF_FONT_Y+2, string, 1, 4 );
		SetCurrentList( window->listhead, value );
		ShowCurrentMode();
		InteliRefreshWindow( window, IRW_REFRESHALL);
	}
}







/**********************************************************************
 *
 *						SetWindowDBaseNext
 *
 *	Description : sets the windows data base head list to next one
 *	Returns		: void
 *	Globals		:
 *
 */
void SetWindowDBaseNext(
	struct WindowStruct *window
){

	WORD	dbasenum;

	dbasenum = MODE( window->listhead );

	if ( dbasenum++ > MAXMODE-1) dbasenum = MAXMODE-1;

	if ( dbasenum < MINMODE) dbasenum = MINMODE;

//	SetWindowDBase( window, dbasenum-1 );

	RefreshSelectors();
}






/**********************************************************************
 *
 *						SetWindowDBasePrev
 *
 *	Description : sets the windows data base head list to previous
 *	Returns		: void
 *	Globals		:
 *
 */
void SetWindowDBasePrev(
	struct WindowStruct *window
){

	WORD	dbasenum;

	dbasenum = MODE( window->listhead );

	if ( dbasenum-- < MINMODE) dbasenum = MINMODE;

	if ( dbasenum > MAXMODE-1) dbasenum = MAXMODE-1;

//	SetWindowDBase( window, dbasenum-1 );

	RefreshSelectors();

}






/**********************************************************************
 *
 *						SetWindowDBase
 *
 *	Description : sets the windows data base head list (edl/shot/reels)
 *	Returns		: void
 *	Globals		:
 *
 */
void SetWindowDBase(
	struct WindowStruct *window,
	UWORD	value					// cycle gadget number
){

	switch( value ){
		// SHOT LIST
		case 0 :	window->listhead = Project.CurrentList[ MF_SHOTMODE ];
					break;
		// EDIT LIST
		case 1 :	window->listhead = Project.CurrentList[ MF_EDLMODE ];
					break;
		// REEL LIST
		case 2 :	window->listhead = Project.CurrentList[ MF_REELMODE ];
					break;
		// TAPES
		case 3 :	window->listhead = Project.CurrentList[ MF_TAPEMODE ];
					break;

	}
	ShowCurrentMode();

	InteliRefreshWindow( window, IRW_REFRESHALL);
}




/**********************************************************************
 *
 *						RefreshSelectors
 *
 *	Description : re-draw the gadgets that select the list type depending
 *				  on the current window list types.
 *	Returns		: void
 *	Globals		:
 *
 */
void RefreshSelectors(){

}





/**********************************************************************
 *
 *						ShowCurrentMode
 *
 *	Description : shows on screen which edit mode is active
 *	Returns		: void
 *	Globals		:
 *
 */
void ShowCurrentMode( void ){

#define	DISABLEGAD(gad)	GT_SetGadgetAttrs( ListEditorGadgets[gad], ListEditorWnd,0,GA_Disabled,TRUE,TAG_DONE )
#define	ENABLEGAD(gad)	GT_SetGadgetAttrs( ListEditorGadgets[gad], ListEditorWnd,0,GA_Disabled,FALSE,TAG_DONE )


	switch ( CURRENTBIGMODE ){
		case MF_TAPEMODE	:
				ENABLEGAD( GD_ED_NEW);
				DISABLEGAD( GD_ED_CUT);
				DISABLEGAD( GD_f2 );
				ENABLEGAD( GD_f1 );
				ENABLEGAD( GD_SourceIn );
				ENABLEGAD( GD_SourceOut );
				ENABLEGAD( GD_Duration );
				ENABLEGAD( GD_KeyFrame );
				break;
		case MF_SHOTMODE	:
				ENABLEGAD(GD_ED_NEW);
				DISABLEGAD(GD_ED_CUT);
				ENABLEGAD( GD_f2 );
				ENABLEGAD( GD_f1 );
				ENABLEGAD( GD_SourceIn );
				ENABLEGAD( GD_SourceOut );
				ENABLEGAD( GD_Duration );
				ENABLEGAD( GD_KeyFrame );
				break;
		case MF_EDLMODE		:
				DISABLEGAD(GD_ED_NEW);
				DISABLEGAD(GD_ED_CUT);
				ENABLEGAD( GD_f2 );
				ENABLEGAD( GD_f1 );
				ENABLEGAD( GD_SourceIn );
				ENABLEGAD( GD_SourceOut );
				ENABLEGAD( GD_Duration );
				ENABLEGAD( GD_KeyFrame );
				break;

		case MF_BIGTAPEMODE	:
				ENABLEGAD(GD_ED_NEW);
				DISABLEGAD(GD_ED_CUT);
				DISABLEGAD( GD_f2 );
				ENABLEGAD( GD_f1 );
				DISABLEGAD( GD_SourceIn );
				DISABLEGAD( GD_SourceOut );
				DISABLEGAD( GD_Duration );
				DISABLEGAD( GD_KeyFrame );
				break;
		case MF_BIGSHOTMODE	:
				ENABLEGAD(GD_ED_NEW);
				DISABLEGAD(GD_ED_CUT);
				ENABLEGAD( GD_f2 );
				ENABLEGAD( GD_f1 );
				DISABLEGAD( GD_SourceIn );
				DISABLEGAD( GD_SourceOut );
				DISABLEGAD( GD_Duration );
				DISABLEGAD( GD_KeyFrame );
				break;
		case MF_BIGEDLMODE	:
				ENABLEGAD(GD_ED_NEW);
				DISABLEGAD(GD_ED_CUT);
				ENABLEGAD( GD_f2 );
				ENABLEGAD( GD_f1 );
				DISABLEGAD( GD_SourceIn );
				DISABLEGAD( GD_SourceOut );
				DISABLEGAD( GD_Duration );
				DISABLEGAD( GD_KeyFrame );
				break;

		case MF_REELMODE	:
				ENABLEGAD( GD_ED_NEW );
				ENABLEGAD( GD_ED_CUT );
				DISABLEGAD( GD_f2 );
				DISABLEGAD( GD_f1 );
				ENABLEGAD( GD_SourceIn );
				ENABLEGAD( GD_SourceOut );
				ENABLEGAD( GD_Duration );
				ENABLEGAD( GD_KeyFrame );
				break;
	}

	RefreshAllTitles();
	RefreshAllListTitles();
	RefreshCurrTotal();
}






















// - - - - - - - - - - - - - - - - - - - - - - - - -







/**********************************************************************
 *
 *						OpenShotList
 *
 *	Description : Open a new SHOTLIST file to edit!
 *	Returns		: void
 *	Globals		:
 *
 *
void OpenList( void ){
	UBYTE	Error[80],FileName[180], *path;
	BPTR	fh;
	FILE	*fp;

	if ( path=AslFileRequest( 'l' ,"Open SHOT List file", "ShotLists:", "#?.shl" ) ){
		// process reading code here
		strcpy(FileName,path);
		fp = fopen( FileName,"rb");
		if (fp){
			BUSYLIST
			LoadList( fp, &Project.LeftWin );
			fclose(fp);
			NORMALLIST
		}
	}
	else {
		Fault( IoErr(), "File Error:\n", Error, 79 );
		AskRequest( Error ,1);
	}
}
 */





/**********************************************************************
 *
 *						OpenProject
 *
 *	Description : Open a new Project file to edit!
 *	Returns		: void
 *	Globals		:
 *
 */
void OpenProject( void ){
	UBYTE	Error[80],FileName[210], *path;
	BPTR	fh;
	ULONG	io_err;

	if ( path=AslFileRequest( 'l' ,"Open Project file", "Projects:", "#?" ) ){
		// process reading code here
		strcpy(FileName,path);
		fh = Open( FileName, MODE_OLDFILE );
		if (fh){
			BUSYLIST
			LoadProject( fh );
			Close( fh );
			NORMALLIST
		}
	}
	else {
		io_err = IoErr();
		if ( io_err ) {
			Fault( io_err, "File Error:\n", Error, 79 );
			AskRequest( Error ,1 );
		}
	}
}


/**********************************************************************
 *
 *						SaveProject
 *
 *	Description : Save a Project file to filesystem!
 *	Returns		: void
 *	Globals		:
 *
 */
void WriteProject( void ){
	UBYTE	Error[80],FileName[210], *path;
	BPTR	fh;
	ULONG	io_err;

	if ( path=AslFileRequest( 's' ,"Save Project file", "Projects:", "#?" ) ){
		// process reading code here
		strcpy(FileName,path);
		fh = Open( FileName, MODE_NEWFILE );
		if (fh){
			BUSYLIST
			SaveProject( fh );
			Close( fh );
			NORMALLIST
		}
	}
	else {
		io_err = IoErr();
		if ( io_err ) {
			Fault( io_err, "File Error:\n", Error, 79 );
			AskRequest( Error ,1 );
		}
	}
}





/**********************************************************************
 *
 *						SaveCMXList
 *
 *	Description : Write CMX EDLLIST file with requester
 *	Returns		: void
 *	Globals		:
 *
 */
void SaveCMXRequester(
	struct WindowStruct *window
){
	UBYTE	Error[80],FileName[180], *path;
	ULONG	io_err;
	FILE	*fp;

	if ( path=AslFileRequest( 's' ,"Write CMX file", "CMXlists:", "#?.cmx" ) ){
		// process reading code here
		strcpy(FileName,path);
		if ( fp = fopen( FileName,"wb") ){
			BUSYLIST
			SaveCMXList( fp, window->listhead );
			fclose(fp);
			NORMALLIST
		}
	}
	else {
		io_err = IoErr();
		if ( io_err ) {
			Fault( io_err, "File Error:\n", Error, 79 );
			AskRequest( Error ,1 );
		}
	}
}















// -----------------------------------




/**********************************************************************
 *
 *						NewElement
 *
 *	Description : add a node to list
 *	Returns		: void
 *	Globals		:
 *
 */
void NewElement( void ){

	union	NODE	*NewReel;

		switch ( CURRENTBIGMODE ){
			case MF_REELMODE	:
								BUSYLIST
								UnHiLightPresent( Project.CurrentWindow );
								NewReel = CreateElement( PRESENTLIST_PTR, PRESENTNODE_PTR );
								CreateAttachedShotList(NewReel);
								RefreshAllWindows();
								//ScreenToFront( InputReqScreen );
								NORMALLIST
								break;
			case MF_EDLMODE		:
								BUSYLIST
							//	UnHiLightPresent( Project.CurrentWindow );
							//	CreateElement( PRESENTLIST_PTR, PRESENTNODE_PTR );
								EDLInfoRender();
								InteliRefreshAllWindows();
								NORMALLIST
								break;

				default			:
								BUSYLIST
								UnHiLightPresent( Project.CurrentWindow );
								CreateElement( PRESENTLIST_PTR, PRESENTNODE_PTR );
								RefreshCurrWindow();
								NORMALLIST
								break;
		}
}





/*****************************************************************
 *
 * 							DupElement
 *
 * 	Description : duplicate current element
 * 	Returns  	: void
 * 	Globals 	:
 *
 */
void DupElement( void ){
		switch ( CURRENTBIGMODE ){
			case MF_TAPEMODE :
			case MF_REELMODE :
			case MF_SHOTMODE :
								BUSYLIST
								UnHiLightPresent( Project.CurrentWindow );
								DupNode( PRESENTLIST_PTR );
								InteliRefreshCurrWindow();
								NORMALLIST
								break;

			case MF_EDLMODE  :	BUSYLIST
								UnHiLightPresent( Project.CurrentWindow );
								DupNode( PRESENTLIST_PTR );
								InteliRefreshAllWindows();
								EDLInfoRender();
								NORMALLIST
								break;
		}
}




/**********************************************************************
 *
 *						DelElements
 *
 *	Description : delete marked nodes from list
 *	Returns		: void
 *	Globals		:
 *
 */
void DelElements( void ){
		switch ( CURRENTBIGMODE ){
			case MF_TAPEMODE :
			case MF_REELMODE :
			case MF_SHOTMODE :
								BUSYLIST
								DeleteElements( PRESENTLIST_PTR, MARKF_SELECT );
								InteliRefreshAllWindows();
								NORMALLIST
								break;
			case MF_EDLMODE  :
								BUSYLIST
								DeleteElements( PRESENTLIST_PTR, MARKF_SELECT);
								InteliRefreshAllWindows();
								EDLInfoRender();
								NORMALLIST
								break;
		}
}





/*****************************************************************
 *
 * 							SelectAll
 *
 * 	Description : marks all elements in current list selected
 * 	Returns  	: nil.
 * 	Globals 	: CurrentEditMode, list headers
 *
 */
void SelectAll( void ){
		BUSYLIST
		MarkAll( PRESENTLIST_PTR, MARKF_SELECT );
		RefreshCurrWindow();
		NORMALLIST
}




/*****************************************************************
 *
 * 							UnSelectAll
 *
 * 	Description : unmarks all elements in current list selected
 * 	Returns  	: nil.
 * 	Globals 	: CurrentEditMode, list headers
 *
 */
void UnSelectAll( void ){
		BUSYLIST
		UnMarkAll( PRESENTLIST_PTR, MARKF_SELECT );
		RefreshCurrWindow();
		NORMALLIST
}




/*****************************************************************
 *
 * 							InvertAll
 *
 * 	Description : inverts all elements in current list selected
 * 	Returns  	: nil.
 * 	Globals 	: CurrentEditMode, list headers
 *
 */
void InvertAll( void ){
		BUSYLIST
		ToggleAll( PRESENTLIST_PTR, MARKF_SELECT );
		RefreshCurrWindow();
		NORMALLIST
}







/*****************************************************************
 *
 * 							Cut
 *
 * 	Description : Cuts from any list to Cliplist
 * 	Returns  	: nil.
 * 	Globals 	: CurrentEditMode, list headers
 *
 */
void Cut( void ){
		BUSYLIST
		CutElements( PRESENTLIST_PTR );
		UnMarkAll( RELCLIPLIST_PTR(PRESENTLIST_PTR), MARKF_SELECT );
		InteliRefreshAllWindows();
		EDLInfoRender();
		NORMALLIST
}




/*****************************************************************
 *
 * 							Copy
 *
 * 	Description : Copies from any list to Cliplist
 * 	Returns  	: nil.
 * 	Globals 	: CurrentEditMode, list headers
 *
 */
void Copy( void ){
		BUSYLIST
		CopyElements( PRESENTLIST_PTR );
		UnMarkAll( RELCLIPLIST_PTR(PRESENTLIST_PTR), MARKF_SELECT );
		InteliRefreshCurrWindow();
		NORMALLIST
}




/*****************************************************************
 *
 * 							Paste
 *
 * 	Description : Pastes from Cliplist to any list
 * 	Returns  	: nil.
 * 	Globals 	: CurrentEditMode, list headers
 *
 */
void Paste( void ){
		BUSYLIST
		PasteElements( PRESENTLIST_PTR );
		InteliRefreshCurrWindow();
		if ( CURRENTMODE == MF_EDLMODE )
			EDLInfoRender();
		NORMALLIST
}





/*****************************************************************
 *
 * 							Xfer2EDL
 *
 * 	Description : trasfers elements hilighted in source-list to dest-list
 * 	Returns  	: nil.
 * 	Globals 	:
 *
 *				!!! Gary was 'ere !!!
 */
void Xfer2EDL(
	LONG lastmode			// previous window
){
	BUSYLIST
	TransferShots2EDL( Project.CurrentList[MF_SHOTMODE], Project.CurrentList[MF_EDLMODE], MARKF_SELECT );	//:-)
	RefreshAllWindows();
	EDLInfoRender();
	NORMALLIST


	/*	LONG currmode = CURRENTBIGMODE;

		switch ( lastmode ){
			case MF_SHOTMODE :
				if ( currmode == MF_EDLMODE ){
					BUSYLIST
					TransferShots2EDL( Project.PreviousWindow->listhead, Project.CurrentWindow->listhead, MARKF_SELECT );	//:-(
					RefreshAllWindows();
					EDLInfoRender();
					NORMALLIST
				}
				break;
			case MF_EDLMODE :
				if ( currmode == MF_SHOTMODE ){
					BUSYLIST
					TransferShots2EDL( Project.CurrentWindow->listhead, Project.PreviousWindow->listhead, MARKF_SELECT );	//:-(
					RefreshAllWindows();
					EDLInfoRender();
					NORMALLIST
				}
				break;
		}
	*/
}








/*****************************************************************
 *
 * 							OpenDebug
 *
 * 	Description : trasfers elements hilighted in shotlist to edl
 * 	Returns  	: nil.
 * 	Globals 	: CurrentEditMode, list headers
 *
 */
void OpenDebug( void ){

	debugwin = Open( "CON:400/300/300/200/DebugWindow", MODE_OLDFILE);
	SelectInput( debugwin);
	SelectOutput( debugwin);
	printf("Debug Window:\n");

}
void CloseDebug( void ){
	Close( debugwin );
}







/*****************************************************************
 *
 * 							SetPresent
 *
 * 	Description : sets passed pos element position using cursor
 *					and if its off screen the screen,it is re-aligned
 *					so it is where the cursor is located at
 * 	Returns  	: current screen position
 * 	Globals 	:
 *
 */
WORD SetPresent(
	struct	WindowStruct *window,
	WORD pos
){
	WORD	screen_total;

	if ( pos ){
		if ( window->listhead->HeadNode.HeadData->lst_tot < MAX_ELEMENT_LINES )
			screen_total = window->listhead->HeadNode.HeadData->lst_tot;
		else
			screen_total = MAX_ELEMENT_LINES-1;

		// make sure the position cant go out of bounds
		if ( pos < 1 )
			pos = 1;
		if ( pos > window->listhead->HeadNode.HeadData->lst_tot )
			pos = window->listhead->HeadNode.HeadData->lst_tot;

		// if cursor travels off screen, reposition screen coords
		// ie. screen follows cursor position
		if ( (pos > (window->listhead->HeadNode.HeadData->scr_top + screen_total) ) )
			window->listhead->HeadNode.HeadData->scr_top = pos - screen_total;
		if ( pos < window->listhead->HeadNode.HeadData->scr_top )
			window->listhead->HeadNode.HeadData->scr_top = pos;

		CurrElementSet( CES_MAKEPRESENT, window->listhead, pos );

		return ( (WORD)(window->listhead->HeadNode.HeadData->lst_pos -
				 window->listhead->HeadNode.HeadData->scr_top) );
	}
}




/*****************************************************************
 *
 * 							ReAlignPresent
 *
 * 	Description :	if present is off screen the cursor is re-aligned
 *					so it is where the screen-view is located at
 * 	Returns  	: VOID
 * 	Globals 	: CurrentList
 *
 */
WORD ReAlignPresent(
	struct WindowStruct *window
){
	WORD	pos = window->listhead->HeadNode.HeadData->lst_pos;
	WORD	screen_total;

//DEBUG
// printf("EDITOR: @ReAlignPresent(), pos = %d\n",pos);

	if ( pos ){
		// finds out the total elements displayed
		if ( window->listhead->HeadNode.HeadData->lst_tot < MAX_ELEMENT_LINES )
			screen_total = window->listhead->HeadNode.HeadData->lst_tot;
		else
			screen_total = MAX_ELEMENT_LINES-1;

		// if cursor travels off the screen, reposition cursor coords
		// ie. cursor follows screen-view position
		if ( (pos > (window->listhead->HeadNode.HeadData->scr_top + screen_total) ) )
			pos = window->listhead->HeadNode.HeadData->scr_top + screen_total;
		if ( pos < window->listhead->HeadNode.HeadData->scr_top )
			pos = window->listhead->HeadNode.HeadData->scr_top;

		CurrElementSet( CES_MAKEPRESENT, window->listhead, pos );
	}

	return pos;
}





/*****************************************************************
 *
 * 							MoveCursor
 *
 * 	Description : Moves cursor +/- vector number
 * 	Returns  	: current screen position
 * 	Globals 	:
 *
 */
WORD MoveCursor(
	struct WindowStruct *window,
	WORD vector
){

	if ( window->listhead->HeadNode.HeadData->sel_elements > 1 ){
		UnMarkAll( window->listhead , MARKF_SELECT );
		SetPresent( window, window->listhead->HeadNode.HeadData->lst_pos + vector );
		InteliRefreshWindow( window, IRW_REFRESHALL );
	}
	else {
		UnHiLightPresent( window );
		SetPresent( window, window->listhead->HeadNode.HeadData->lst_pos + vector );
		HiLightPresent( window );
	}

	return ( (WORD)(window->listhead->HeadNode.HeadData->lst_pos -
		window->listhead->HeadNode.HeadData->scr_top) );
}



/*****************************************************************
 *
 * 							SetCursor
 *
 * 	Description : Places cursor @ vector number
 * 	Returns  	: current screen position
 * 	Globals 	:
 *
 */
WORD SetCursor(
	struct WindowStruct *window,
	WORD vector
){

	if ( window->listhead->HeadNode.HeadData->sel_elements > 1 ){
		UnMarkAll( window->listhead , MARKF_SELECT );
		vector = SetPresent( window, vector );
		InteliRefreshWindow( window, IRW_REFRESHALL );
	}
	else {
		UnHiLightPresent( window );
		vector = SetPresent( window, vector );
		HiLightPresent( window );
	}
	return ( (WORD) vector );
}








/*****************************************************************
 *
 * 							HiLightPresent
 *
 * 	Description : marks the present node and cursor |> is drawn
 * 	Returns  	:
 * 	Globals 	:
 *
 */
void HiLightPresent(
	struct WindowStruct *window
){

	if ( window->listhead->HeadNode.HeadData->lst_pos ) {
		CurrElementSet( CES_MARK, window->listhead, window->listhead->HeadNode.HeadData->lst_pos );
		RefreshXElement( window, window->listhead->HeadNode.HeadData->lst_pos );
		RefreshCursor( window );
	}
}




/*****************************************************************
 *
 * 							UnHiLightPresent
 *
 * 	Description : unmarks the present node and cursor |> is undrawn
 * 	Returns  	: current screen position
 * 	Globals 	:
 *
 */
void UnHiLightPresent(
	struct WindowStruct *window
){

	if ( window->listhead->HeadNode.HeadData->lst_pos ) {
		CurrElementSet( CES_UNMARK, window->listhead, window->listhead->HeadNode.HeadData->lst_pos );
		RefreshXElement( window, window->listhead->HeadNode.HeadData->lst_pos );
		RefreshCursorClr( window );
	}
}






/**********************************************************************
 *
 *						RefreshCursor
 *	Description : o this displays the cursor icon at the new position
 *					in the list window
 *  Returns		: NULL
 *	Globals		: usual
 *
 */
void RefreshCursor(
	struct WindowStruct *window
){

	WORD	sline;
	static UBYTE	*mark = {" "};

	*(mark) = GFXCHAR_RIGHTP;

	UseWindowFontSize();

	// calc new line pos
	sline = window->listhead->HeadNode.HeadData->lst_pos - window->listhead->HeadNode.HeadData->scr_top;

	// place arrow selector symbol in new position
	PutsXY( window->x, window->y+(sline*(window->ELEMENT_YSIZE))+window->Font_Height, mark, PEN_CURSOR, PEN_CURSOR_BACK );

	SetFont( ListEditorWnd->RPort, mainfont10 );
}


void RefreshCursorClr(
	struct WindowStruct *window
){

	WORD	sline;

//DEBUG
// printf("EDITOR: @RefreshCursorClr()\n");
	sline = window->listhead->HeadNode.HeadData->lst_pos - window->listhead->HeadNode.HeadData->scr_top;

	UseWindowFontSize();

	//erase symbol from old position
	PutsXY( window->x, window->y+(sline*(window->ELEMENT_YSIZE))+window->Font_Height, " ", PEN_CURSOR, PEN_CURSOR_BACK );

	SetFont( ListEditorWnd->RPort, mainfont10 );
}




/**********************************************************************
 *
 *						GotoPage
 *	Description : o this refreshes at a new page position
 *					in the list window
 *  Returns		: NULL
 *	Globals		: usual
 *
 */
void GotoPage(
	struct WindowStruct *window,
	UWORD option
){
	switch ( option ){
		case GP_TOP : 	SetCursor( window, 1 );
						InteliRefreshWindow( window, IRW_REFRESHALL );
						break;
		case GP_END	:	SetCursor( window, window->listhead->HeadNode.HeadData->lst_tot );
						InteliRefreshWindow( window, IRW_REFRESHALL );
						break;
		case GP_UP	:	MoveCursor( window, -((MAX_ELEMENT_LINES)-1) );
						InteliRefreshWindow( window, IRW_REFRESHALL );
						break;
		case GP_DOWN:	MoveCursor( window, +((MAX_ELEMENT_LINES)-1) );
						InteliRefreshWindow( window, IRW_REFRESHALL );
						break;
		case GP_SET	:	//SetCursor( window, (MAX_ELEMENT_LINES) * rqGetLong(req,max=lst_tot/MAX_ELEMENT_LINES,min=1,"Jump to page#) );
						InteliRefreshWindow( window, IRW_REFRESHALL );
						break;
	}
}





/**********************************************************************
 *
 *						BusyPointer
 *	Description : o this places a busy pointer on window specified
 *  Returns		: NULL
 *	Globals		: usual
 *
 */
void BusyPointer( struct Window *win ){
	SetWindowPointer( win, WA_BusyPointer, TRUE, TAG_DONE );
}




/**********************************************************************
 *
 *						NormalPointer
 *	Description : o this places a normal pointer on window specified
 *  Returns		: NULL
 *	Globals		: usual
 *
 */
void NormalPointer( struct Window *win ){
	SetWindowPointer( win, TAG_DONE );
}






/**********************************************************************
 *
 *						Draw3DOutBox
 *	Description : just like normal bevelbox except inside color is selectable
 *				  plus it looks MONUMENTAL!
 *								¯¯¯¯¯¯¯¯¯¯¯
 *  Returns		: NULL
 *	Globals		: usual
 *
 */
void Draw3DBox(
	struct	RastPort *rp,		// window rastport to draw into
	UWORD	xpos,				// X coordinate to place MEGA-BEVEL
	UWORD	ypos,				// Y coordinate to place MEGA-BEVEL
	UWORD	xsize,				// X size of MEGA-BEVEL
	UWORD	ysize,				// X size of MEGA-BEVEL
	UWORD	shine_pen,			// top-left pen
	UWORD	body_pen,			// central pen
	UWORD	shadow_pen			// bot-right pen
){

#define	LineDraw(x,y,x2,y2)	Move( rp, x , y); \
							Draw( rp, x2, y2 )

	xsize--;ysize--;

	SetAPen( rp, body_pen );
	RectFill( rp, xpos, ypos, xpos+xsize, ypos+ysize);

	LineDraw( xpos, ypos, xpos+1, ypos+1 );
	LineDraw( xpos+xsize-1, ypos+ysize-1, xpos+xsize, ypos+ysize );

	SetAPen( rp, shine_pen );
	LineDraw( xpos+0, ypos+0, xpos+xsize-1, ypos );
	LineDraw( xpos+0, ypos+1, xpos+xsize-2, ypos+1 );
	LineDraw( xpos+0, ypos+0, xpos, ypos+ysize-1 );
	LineDraw( xpos+1, ypos+0, xpos+1, ypos+ysize-2 );

	SetAPen( rp, shadow_pen );
	LineDraw( xpos+1, ypos+ysize-0, xpos+xsize-1, ypos+ysize-0 );
	LineDraw( xpos+2, ypos+ysize-1, xpos+xsize-2, ypos+ysize-1 );
	LineDraw( xpos+xsize-0, ypos+1, xpos+xsize-0, ypos+ysize-1 );
	LineDraw( xpos+xsize-1, ypos+2, xpos+xsize-1, ypos+ysize-2 );

	SetAPen( rp, 2 );
	if ( shine_pen > shadow_pen ){
		LineDraw( xpos+0, ypos+0, xpos+2, ypos+0 );
		LineDraw( xpos+0, ypos+0, xpos+0, ypos+2 );
	}
	else {
		LineDraw( xpos+xsize, ypos+ysize, xpos+xsize-2, ypos+ysize );
		LineDraw( xpos+xsize, ypos+ysize, xpos+xsize-0, ypos+ysize-2 );
	}
}






/**********************************************************************
 *
 *						Draw3DLine
 *	Description : just like normal line except its 3D (sorta!) (horizontal)
 *				  plus it looks MONUMENTAL!
 *								¯¯¯¯¯¯¯¯¯¯¯
 *  Returns		: NULL
 *	Globals		: usual
 *
 */
void Draw3DLine(
	struct	RastPort *rp,		// window rastport to draw into
	UWORD	xpos,				// X coordinate to place MEGA-BEVEL
	UWORD	ypos,				// Y coordinate to place MEGA-BEVEL
	UWORD	xsize,				// X size of MEGA-BEVEL
	UWORD	shine_pen,			// top-left pen
	UWORD	shadow_pen			// bot-right pen
){

	xsize--;


	SetAPen( rp, shine_pen );
	LineDraw( xpos+0, ypos+0, xpos+xsize, ypos+0 );
	SetAPen( rp, shadow_pen );
	LineDraw( xpos+0, ypos+1, xpos+xsize, ypos+1 );
}


/**********************************************************************
 *
 *						Draw3DVLine
 *	Description : just like normal line except its 3D (sorta!) (vertical)
 *				  plus it looks MONUMENTAL!
 *								¯¯¯¯¯¯¯¯¯¯¯
 *  Returns		: NULL
 *	Globals		: usual
 *
 */
void Draw3DVLine(
	struct	RastPort *rp,		// window rastport to draw into
	UWORD	xpos,				// X coordinate to place MEGA-BEVEL
	UWORD	ypos,				// Y coordinate to place MEGA-BEVEL
	UWORD	ysize,				// X size of MEGA-BEVEL
	UWORD	shine_pen,			// top-left pen
	UWORD	shadow_pen			// bot-right pen
){

//	ysize--;

	SetAPen( rp, shine_pen );
	LineDraw( xpos+1, ypos, xpos+1, ypos+ysize );
	SetAPen( rp, shadow_pen );
	LineDraw( xpos+0, ypos, xpos+0, ypos+ysize );
}








void RefreshSEMX(
	struct Gadget *gadget
){
	if(!(gadget->Flags & GFLG_SELECTED))
		UnPressGadget( ListEditorWnd, gadget );
	else

	if ( gadget == ListEditorGadgets[ GD_ED_S_E ] ) {
		PressGadget( ListEditorWnd, ListEditorGadgets[ GD_ED_S_S ] );
		PressGadget( ListEditorWnd, ListEditorGadgets[ GD_ED_E_E ] );
	}
	else
	if ( gadget == ListEditorGadgets[ GD_ED_S_S ] ) {
		PressGadget( ListEditorWnd, ListEditorGadgets[ GD_ED_S_E ] );
		PressGadget( ListEditorWnd, ListEditorGadgets[ GD_ED_E_E ] );
	}
	else
	if ( gadget == ListEditorGadgets[ GD_ED_E_E ] ) {
		PressGadget( ListEditorWnd, ListEditorGadgets[ GD_ED_S_E ] );
		PressGadget( ListEditorWnd, ListEditorGadgets[ GD_ED_S_S ] );
	}
}



void PressGadget(
	struct Window *win,
	struct Gadget *gadget
){
	if(gadget->Flags & GFLG_SELECTED) {
		gadget->Flags ^= GFLG_SELECTED;
		RefreshGList(gadget, win, NULL, 1);
	}
}


void UnPressGadget(
	struct Window *win,
	struct Gadget *gadget
){
	gadget->Flags |= GFLG_SELECTED;
	RefreshGList(gadget, win, NULL, 1);
}





