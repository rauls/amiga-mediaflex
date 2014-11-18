/**********************************************************************
 *
 *						EventHandler.c
 *	Description : handles all incoming events and routes them to functions
 *  Returns		:
 *	Globals		:
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuitionbase.h>
#include <intuition/classes.h>

#include <proto/exec.h>				// use amiga library stuff
#include <proto/dos.h>
#include <proto/gadtools.h>
#include <proto/intuition.h>

#include "globals.h"

#include "eventhandler.h"
#include "database.h"
#include "funcs.h"
#include "Editor.h"
#include "EditDisplay.h"
#include "SetSegments.h"
#include "VTR_Ctrl.h"
#include "AUDIO_Ctrl.h"
#include "KeyEditor.h"
#include "SearchReq.h"
#include "InputProj.h"


#include "DTAGSGUI.h"			// GUI code
#include "EditorGUI.h"
#include "VTR_CtrlGUI.h"
#include "EditDisplayGUI.h"
#include "SetSegmentsGUI.h"
#include "DCGUI.h"
#include "TimeLineGUI.h"
#include "SearchReqGUI.h"
#include "InputProjGUI.h"

//#include "InputReqHandle.h"		// handler code
#include "DTAGSrequestor.h"
#include "TransitionGUI.h"


#define	System(x)	SystemTagList( x,TAG_DONE )


		ULONG	TimerSeconds=0;
		ULONG	TimerMicros=0;
static	LONG	n=1;
		WORD	temp;


/**********************************************************************
 *
 *						EventHandler
 *	Description : handles al incoming events from Intuition and then
 *					process them to various function
 *  Returns		:
 *	Globals		:
 *
 */
BOOL EventHandler( void ){
	BOOL running = TRUE;
	BOOL Busy = FALSE;
	ULONG Mask, ReturnSignal,
		ListEditorSignal,
		DTAGSReqSignal,
		EditDisplaySignal,
		VTR_CtrlSignal,
		DCSignal,
		TimeLineSignal,
		TimeLinePrefsSignal,
		SetSegmentsSignal,
		SearchReqSignal,
		InputProjSignal,
		TransSignal;

		if( Audio_State || Video_State )
			Busy = TRUE;
		else
			Busy = FALSE;


		if(SetSegmentsWnd) {
			SetSegmentsSignal = 1L<<SetSegmentsWnd->UserPort->mp_SigBit;
			Mask = SetSegmentsSignal;
		}
		else {

			if(ListEditorWnd) {
				ListEditorSignal = 1L<<ListEditorWnd->UserPort->mp_SigBit;
				Mask = ListEditorSignal;
			}

			if(DTAGS0Wnd) {
				DTAGSReqSignal = 1L<<DTAGS0Wnd->UserPort->mp_SigBit;
				Mask |= DTAGSReqSignal;
			}

			if(VTR_CtrlWnd) {
				VTR_CtrlSignal = 1L<<VTR_CtrlWnd->UserPort->mp_SigBit;
				Mask |= VTR_CtrlSignal;
			}

			if(EditDisplayWnd) {
				EditDisplaySignal = 1L<<EditDisplayWnd->UserPort->mp_SigBit;
				Mask |= EditDisplaySignal;
			}

			if(DCWnd) {
				DCSignal = 1L<<DCWnd->UserPort->mp_SigBit;
				Mask |= DCSignal;
			}

			if(TimeLineWnd) {
				TimeLineSignal = 1L<<TimeLineWnd->UserPort->mp_SigBit;
				Mask |= TimeLineSignal;
			}

			if(TimeLinePrefsWnd) {
				TimeLinePrefsSignal = 1L<<TimeLinePrefsWnd->UserPort->mp_SigBit;
				Mask |= TimeLinePrefsSignal;
			}

			if(SearchReqWnd) {
				SearchReqSignal = 1L<<SearchReqWnd->UserPort->mp_SigBit;
				Mask |= SearchReqSignal;
			}

			if(InputProjWnd) {
				InputProjSignal = 1L<<InputProjWnd->UserPort->mp_SigBit;
				Mask |= InputProjSignal;
			}

			if(TransWnd) {
				TransSignal = 1L<<TransWnd->UserPort->mp_SigBit;
				Mask |= TransSignal;
			}
		}

		ReturnSignal = Wait(Mask);

		if( Audio_State || Video_State )
			Busy = TRUE;
		else
			Busy = FALSE;

		if(SetSegmentsWnd) {
			if(ReturnSignal & SetSegmentsSignal)  running = HandleSetSegments();
		}
		else {
			if(ListEditorWnd && !Busy)
				if(ReturnSignal & ListEditorSignal)  running = HandleListEditorEvent();

			if(DTAGS0Wnd && !Busy) {
				SetActiveDTAG();
				if(ReturnSignal & DTAGSReqSignal) HandleDTAGSrequestor();
			}

			if(VTR_CtrlWnd)
				if(ReturnSignal & VTR_CtrlSignal)	HandleVTR_Ctrl();

			if(EditDisplayWnd && !Busy)
				if(ReturnSignal & EditDisplaySignal)	HandleEditDisplay();

			if(DCWnd && !Busy)
				if(ReturnSignal & DCSignal)	HandleDCWindow();

			if(TimeLineWnd && !Busy)
				if(ReturnSignal & TimeLineSignal)	HandleTimeLineWindow();

			if(TimeLinePrefsWnd && !Busy)
				if(ReturnSignal & TimeLinePrefsSignal)	HandleTimeLineWindow();

			if(SearchReqWnd && !Busy)
				if(ReturnSignal & SearchReqSignal)  HandleSearchReq();

			if(InputProjWnd)
				if(ReturnSignal & InputProjSignal)  HandleInputProj();

			if(TransWnd && !Busy)
				if(ReturnSignal & TransSignal)  HandleTransRequestor();

		}
	return running;
}




	ULONG	glob_Code = NULL;

	ULONG	glob_Qual = NULL;

/**********************************************************************
 *
 *						HandleListEditorEvent
 *	Description : all incomming events are processed here for this window
 *  Returns		:
 *	Globals		:
 *
 */
BOOL HandleListEditorEvent( void ){
static	UWORD	buttonevent=0;

struct	IntuiMessage
				*m,ListEditorMsg;
struct	Gadget	*gadget;
		BOOL	running = TRUE;


//==========>  handle all "GadTools" messages  <===========
	while( m = GT_GetIMsg( ListEditorWnd->UserPort )) {
		CopyMem(( char * )m, ( char * )&ListEditorMsg, (long)sizeof( struct IntuiMessage ));
		GT_ReplyIMsg( m );

		gadget = (struct Gadget *)ListEditorMsg.IAddress;

		glob_Code = ListEditorMsg.Code;
		glob_Qual = ListEditorMsg.Qualifier;

		switch ( ListEditorMsg.Class ) {

			case	STRINGIDCMP|CHECKBOXIDCMP|BUTTONIDCMP:

				switch ( gadget->GadgetID ) {
//					case 0 : ; break;
//				};

//			case	SCROLLERIDCMP:
					// string gadgets
					case GD_Note	:	NoteGad2Node( PRESENTLIST_PTR );
										ErrorHandler(	ERRH_Type,		ERRH_OK,
														TAG_DONE );
										break;

					case GD_IO_REQ	:
						OpenInputProjWin();
						break;

//			case	BUTTONIDCMP:
				// function gadgets in-between two editors
					case GD_ED_S_E	:	RefreshSEMX( gadget );
										ShowStatus("Scene List / EDL mode.");
										SetWindowDBase( &Project.LeftWin, DB_SHOTLIST );
										SetWindowDBase( &Project.RightWin, DB_EDLLIST );
										ShowCurrentMode();
										EDLInfoRender();
										break;
					case GD_ED_S_S	:	RefreshSEMX( gadget );
										ShowStatus("Scene List / Scene List mode.");
										SetWindowDBase( &Project.LeftWin, DB_SHOTLIST );
										SetWindowDBase( &Project.RightWin, DB_SHOTLIST );
										ShowCurrentMode();
										EDLInfoRender();
										break;
					case GD_ED_E_E	:	RefreshSEMX( gadget );
										ShowStatus("EDL / EDL mode.");
										SetWindowDBase( &Project.LeftWin, DB_EDLLIST );
										SetWindowDBase( &Project.RightWin, DB_EDLLIST );
										ShowCurrentMode();
										EDLInfoRender();
										break;
					case GD_ED_T_R	:	RefreshSEMX( gadget );
										ShowStatus("Tape List / Reel List mode.");
										SetWindowDBase( &Project.LeftWin, DB_TAPELIST );
										SetWindowDBase( &Project.RightWin, DB_REELLIST );
										ShowCurrentMode();
										EDLInfoRender();
										break;
				//	case GD_ED_T_T	:	RefreshSEMX( gadget );
				//						ShowStatus("Tape List / Tape List mode.");
				//						SetWindowDBase( &Project.LeftWin, 3 );
				//						SetWindowDBase( &Project.RightWin, 3 );
				//						break;

					case GD_ED_NEW	:	NewElement();
										break;
					case GD_ED_DEL	:	DelElements();
										break;
					case GD_ED_DUP	:
										DupnElements( n );
										break;
					case GD_ED_DUPN	:
										GT_GetGadgetAttrs( ListEditorGadgets[ GD_ED_DUPN ], ListEditorWnd, 0,
											GTIN_Number, &n, TAG_DONE );
										sprintf(txtstr, "New repeat number is %d.", n );
										ShowStatus( txtstr );
										break;
					case GD_ED_CUT 	:	Cut();
										break;
					case GD_ED_COPY	:	Copy();
										break;
					case GD_ED_PASTE:	Paste();
										break;
					case GD_ED_UNDO	:
										break;
					case GD_ED_ALL	:	SelectAll();
										EDLInfoRender();
										break;
					case GD_ED_INV	:	InvertAll();
										EDLInfoRender();
										break;
					case GD_ED_NONE	:	UnSelectAll();
										EDLInfoRender();
										break;
					case GD_XFER	:
							temp = TransferElements( Project.LeftWin.listhead, Project.RightWin.listhead, 'C' );
							if ( temp == -1) {
								ErrorHandler(	ERRH_Type,		ERRH_ERROR,
												ERRH_Options,	ERRH_ErrorText,
												ERRH_Text,		ERRH_CANTXFERTXT,
												TAG_DONE );
							}
							else {
								if ( temp > 1 )
									sprintf(txtstr,"%d shots have been transfered to the EDL",temp);
								else
									sprintf(txtstr,"%d shot has been transfered to the EDL",temp);
								ErrorHandler(	ERRH_Type,		ERRH_MSG,
												ERRH_Options,	ERRH_StatusText,
												ERRH_Text,		txtstr,
												TAG_DONE );
								RefreshAllWindows();
								EDLInfoRender();
							}
										break;


					case GD_ED_SPLIT :
										if( SplitSelected( Project.CurrentWindow->listhead ) ) {
											if ( BIGMODE( Project.CurrentWindow->listhead ) == MF_EDLMODE ){
												EDLInfoRender();
												RefreshAllWindows();
											}
											else
												RefreshCurrWindow();
										}
										break;
					case GD_ED_SYNC	 :
										if( SyncSelected( Project.CurrentWindow->listhead ) ) {
											if ( BIGMODE( Project.CurrentWindow->listhead ) == MF_EDLMODE )
												EDLInfoRender();
											RefreshCurrWindow();
										}
										break;
					case GD_ED_UNSYNC:
										if( UnsyncSelected( Project.CurrentWindow->listhead ) ) {
											if ( BIGMODE( Project.CurrentWindow->listhead ) == MF_EDLMODE )
												EDLInfoRender();
											RefreshCurrWindow();
										}
										break;
					case GD_ED_LOCK	 :
										if( LockSelected( Project.CurrentWindow->listhead ) ) {
											if ( BIGMODE( Project.CurrentWindow->listhead ) == MF_EDLMODE )
												EDLInfoRender();
											RefreshCurrWindow();
										}
										break;
					case GD_ED_UNLOCK:
										if( UnlockSelected( Project.CurrentWindow->listhead ) ) {
											if ( BIGMODE( Project.CurrentWindow->listhead ) == MF_EDLMODE )
												EDLInfoRender();
											RefreshCurrWindow();
										}
										break;



					case GD_ED_PAGE	:
										break;

					case GD_ED_PREV	:	GotoPage( Project.CurrentWindow,GP_UP );
										break;
					case GD_ED_NEXT :	GotoPage( Project.CurrentWindow,GP_DOWN );
										break;
					case GD_ED_TOP	:	GotoPage( Project.CurrentWindow,GP_TOP );
										break;
					case GD_ED_END	:	GotoPage( Project.CurrentWindow,GP_END );
										break;



					case GD_ED_PICS	:
										HandleKeyEditor();
								//		ShowStatus("Key Frame display ON.");
								//		System("run ViewIFF dunk0:graphics/iff_images/elle.iff24");
										break;

					case GD_ED_VTR	:	if(VTR_CtrlWnd)
											CloseVTR_CtrlWin();
										else {
											OpenVTR_CtrlWin();
											Node2Gads( PRESENTLIST_PTR );
											RefreshLiveVTR();
											ShowCurrentMode();
										}
										break;
					case GD_ED_TIME :
										if(TimeLineWnd)
											CloseTimeLineWindow();
										else
											OpenTimeLineWindow();
										break;
					case GD_ED_TAGS :
										if(DTAGS0Wnd)
											CloseDTAGSrequestor();
										else
											OpenDTAGSrequestor();
										break;
					case GD_ED_PREFS:
										if(EditDisplayWnd)
											CloseEditDisplayWin();
										else
											OpenEditDisplayWin();
										break;
				}
				break;



//=========================  MENUS ============================

			case	IDCMP_MENUPICK:
				running = HandleListEditorMenus( &ListEditorMsg );
				break;


//==========>  handle all "Intuition" messages  <===========

			case	IDCMP_MOUSEBUTTONS :			// mouse coords
				switch ( ListEditorMsg.Code ){
					case SELECTUP:
						buttonevent = MB_RELEASED;
						break;
					case SELECTDOWN:
						if ( DoubleClick( TimerSeconds, TimerMicros, ListEditorMsg.Seconds, ListEditorMsg.Micros ) )
							buttonevent = MB_DUBCLICK;
						else {
							TimerSeconds = ListEditorMsg.Seconds;
							TimerMicros = ListEditorMsg.Micros;
							buttonevent = MB_PRESSED;
						}
						break;
				}
				ShowTime();
				MouseHandler( buttonevent, ListEditorMsg.Qualifier );
				if ( buttonevent == MB_RELEASED ||
					 buttonevent == MB_DUBCLICK )
					buttonevent = 0;
				break;

			case	IDCMP_MOUSEMOVE:
				if ( buttonevent )
					MouseHandler( buttonevent, ListEditorMsg.Qualifier );
				break;

			case	IDCMP_REFRESHWINDOW:
			    GT_BeginRefresh( ListEditorWnd );
				ListEditorRender();
			    GT_EndRefresh( ListEditorWnd, TRUE);
	    		break;

			case	IDCMP_ACTIVEWINDOW:
//				ClearMsgPort( ListEditorWnd->UserPort );
				break;

			case	IDCMP_VANILLAKEY:				// Key short cuts
				running = ListEditorVanillaKey( &ListEditorMsg );
//DEBUG
// printf("CODE=%ld QUAL=%ld\n",ListEditorMsg.Code,ListEditorMsg.Qualifier);
				break;

			case	IDCMP_RAWKEY:
				running = ListEditorRawKey( &ListEditorMsg );
				break;

		}
	}

	return running;
}






/**********************************************************************
 *
 *						ListEditorMenus()
 *	Description : Handles all menu events
 *  Returns		: BOOL
 *	Globals		:
 *
 */
BOOL HandleListEditorMenus(
	struct IntuiMessage   *ListEditorMsg
){
	struct	MenuItem		*Next;
	UWORD	menu, item, sub;
	BOOL running = TRUE;


			    while  ( ListEditorMsg->Code != MENUNULL ) {
		            Next = ItemAddress(  ListEditorMenus, ListEditorMsg->Code );
			        menu = MENUNUM( ListEditorMsg->Code );
			        item = ITEMNUM( ListEditorMsg->Code );
			        sub  = SUBNUM( ListEditorMsg->Code );
#ifdef DEBUG
	printf("m=%d i=%d s=%d\n",menu,item,sub);
#endif
			        switch ( menu ) {
			            case    0:
			                switch ( item ) {
								case	0:
									switch ( sub ) {
										case	0:	OpenProject();
													RefreshAllListTitles();
													RefreshAllTitles();
													RefreshAllTotals();
													ClearAllWindows();
													RefreshAllWindows();
											break;
										case	1:	WriteProject();
											break;
													// NEW
										case	2:	//NewProject();
													RefreshAllListTitles();
													RefreshAllTitles();
													RefreshAllTotals();
													RefreshAllWindows();
											break;
									}
									break;

								case	1:	break; // ~~~~~~~~~~~ line!
								// EDL / SHOTLIST / ANYLIST
								case	2:
									switch ( sub ) {
										case	3:	SaveCMXRequester( Project.CurrentWindow );
											break;
										case	5:
													RefreshCurrWindow();
											break;
									}
									break;

								// Select a font size
								case	3:
									switch ( sub ) {
										case	0:	SetWindowFontSize(8) ;
													ClearAllWindows();
													RefreshAllWindows();
													break;
										case	1:	SetWindowFontSize(10) ;
													ClearAllWindows();
													RefreshAllWindows();
													break;
										case	2:	SetWindowFontSize(12) ;
													ClearAllWindows();
													RefreshAllWindows();
													break;
										case	3:	SetWindowFontSize(14) ;
													ClearAllWindows();
													RefreshAllWindows();
													break;
										case	4:	SetWindowFontSize(16) ;
													ClearAllWindows();
													RefreshAllWindows();
													break;
										case	5:	SetWindowFontSize(18) ;
													ClearAllWindows();
													RefreshAllWindows();
													break;
									}
									break;

								case	5:	if ( AskRequestLow("URGENT!","QUIT!! Are you sure?","QUIT|STAY") )
												running=FALSE;
									break;
							}
							break;

			            case    1:
			                switch ( item ) {
								case	0:	NewElement();
											break;
								case	1:	DelElements();
											break;
								case	2:	DupnElements( n );
											break;
								case	3:	Cut();
											break;
								case	4:	Copy();
											break;
								case	5:	Paste();
											break;


#define	MENU2MARKER(a,x)	if ( Next->Flags&CHECKED )\
								a |= x; \
							else \
								a &= (x^0xffffffff); \
							break; \


								case	7:			//view element type for shotlist
									switch ( sub ) {
										case	0:	MENU2MARKER( VIEW_SHOT, MARKF_VISIBLE )
										case	1:	MENU2MARKER( VIEW_SHOT, MARKF_SELECT )
										case	2:	MENU2MARKER( VIEW_SHOT, MARKF_FOUND )
										case	3:	MENU2MARKER( VIEW_SHOT, MARKF_HIDDEN )
										case	4:	MENU2MARKER( VIEW_SHOT, MARKF_AUDIO )
										case	5:	MENU2MARKER( VIEW_SHOT, MARKF_VIDEO )
										case	6:	MENU2MARKER( VIEW_SHOT, DT_GRAPHICS )
									}
									ReCalcWindow( &Project.LeftWin );
									ReCalcWindow( &Project.RightWin );
									RefreshAllWindows();
									break;

								case	8:			// view element type for edl
									switch ( sub ) {
										case	0:	MENU2MARKER( VIEW_EDL, MARKF_VISIBLE )
										case	1:	MENU2MARKER( VIEW_EDL, MARKF_SELECT )
										case	2:	MENU2MARKER( VIEW_EDL, MARKF_FOUND )
										case	3:	MENU2MARKER( VIEW_EDL, MARKF_HIDDEN )
										case	4:	MENU2MARKER( VIEW_EDL, MARKF_AUDIO )
										case	5:	MENU2MARKER( VIEW_EDL, MARKF_VIDEO )
										case	6:	MENU2MARKER( VIEW_EDL, DT_GRAPHICS )
									}
									ReCalcWindow( &Project.LeftWin );
									ReCalcWindow( &Project.RightWin );
									RefreshAllWindows();
									break;

								case	10:			// define segment
											UnHiLightPresent( Project.CurrentWindow );
											CreateSegmentHeader( Project.CurrentWindow->listhead );
											RefreshCurrWindow();
											break;

								case	11:			// define comment
											UnHiLightPresent( Project.CurrentWindow );
											CreateCommentNode( Project.CurrentWindow->listhead,
											   Project.CurrentWindow->listhead->HeadNode.present );
											RefreshCurrWindow();
											break;

								case	12:			// lock
										if( LockSelected( Project.CurrentWindow->listhead ) ) {
											if ( BIGMODE( Project.CurrentWindow->listhead ) == MF_EDLMODE )
												EDLInfoRender();
											RefreshCurrWindow();
										}
											break;
								case	13:			// unlock
										if( UnlockSelected( Project.CurrentWindow->listhead ) ) {
											if ( BIGMODE( Project.CurrentWindow->listhead ) == MF_EDLMODE )
												EDLInfoRender();
											RefreshCurrWindow();
										}
											break;

								case	14:			// protect
										if( MarkAllSelected( Project.CurrentWindow->listhead, MARKF_PROTECT ) ) {
											if ( BIGMODE( Project.CurrentWindow->listhead ) == MF_EDLMODE )
												EDLInfoRender();
											RefreshCurrWindow();
										}
											break;
								case	15:			// unprotect
										if( UnMarkAllSelected( Project.CurrentWindow->listhead, MARKF_PROTECT ) ) {
											if ( BIGMODE( Project.CurrentWindow->listhead ) == MF_EDLMODE )
												EDLInfoRender();
											RefreshCurrWindow();
										}
											break;

								case	16:			// sync
										if( SyncSelected( Project.CurrentWindow->listhead ) ) {
											if ( BIGMODE( Project.CurrentWindow->listhead ) == MF_EDLMODE )
												EDLInfoRender();
											RefreshCurrWindow();
										}
											break;
								case	17:			// unsync
										if( UnsyncSelected( Project.CurrentWindow->listhead ) ) {
											if ( BIGMODE( Project.CurrentWindow->listhead ) == MF_EDLMODE )
												EDLInfoRender();
											RefreshCurrWindow();
										}
											break;
							}
							break;

			            case    2:
			                switch ( item ) {
								case 0 :
								case 1 :
											break;
								case 3 :
											AddEXTVideoReels( PRESENTLIST_PTR );
											RefreshRightWindow();
											break;
								case 4 :
											AddEXTAudioReels( PRESENTLIST_PTR );
											RefreshRightWindow();
											break;
							}
							break;

			            case    3:
			                switch ( item ) {
								case	0:	SystemTagList("run SetPalette",TAG_DONE);
											break;
								case	1:	SystemTagList("run NewShell window=CON:200/200/360/120/MediaFlex_Shell/CLOSE/SCREENMediaFlex",TAG_DONE);
											break;
							}
							break;

			            case    4:
			                switch ( item ) {
								case	0:	DebugShell();
											break;
							}
					}
    			ListEditorMsg->Code = Next->NextSelect;
				}
	return running;

}






/**********************************************************************
 *
 *						ListEditorVanillaKey()
 *	Description : goes here on every keymap key pressed
 *  Returns		: BOOL
 *	Globals		:
 *
 */
BOOL ListEditorVanillaKey(
	struct IntuiMessage   *ListEditorMsg
){
	BOOL running = TRUE,
		 active;
	WORD	pel;


	switch( CURRENTBIGMODE ){
		case MF_REELMODE:
		case MF_EDLMODE:
		case MF_SHOTMODE:
				active = TRUE;
				break;
		default	:
				active = FALSE;
				break;
	}


		// shift keypad
	if ( ListEditorMsg->Qualifier & IEQUALIFIER_NUMERICPAD &&
		 ListEditorMsg->Qualifier & (IEQUALIFIER_RSHIFT|IEQUALIFIER_LSHIFT) ){
		switch ( ListEditorMsg->Code ) {
			case '7'	:
						if ( Project.PreviousWindow->listhead == Project.CurrentWindow->listhead )
							GotoPage( Project.PreviousWindow, GP_TOP );
						GotoPage( Project.CurrentWindow, GP_TOP );
						break;
			case '1'	:
						if ( Project.PreviousWindow->listhead == Project.CurrentWindow->listhead )
							GotoPage( Project.PreviousWindow, GP_END );
						GotoPage( Project.CurrentWindow, GP_END );
						break;
			case '9'	:
						if ( Project.PreviousWindow->listhead == Project.CurrentWindow->listhead )
							GotoPage( Project.PreviousWindow, GP_UP );
						GotoPage( Project.CurrentWindow, GP_UP );
						break;
			case '3'	:
						if ( Project.PreviousWindow->listhead == Project.CurrentWindow->listhead )
							GotoPage( Project.PreviousWindow, GP_DOWN );
						GotoPage( Project.CurrentWindow, GP_DOWN );
						break;
		}
	}
	else

	if ( ListEditorMsg->Qualifier & IEQUALIFIER_NUMERICPAD ){
		switch ( ListEditorMsg->Code ) {
			case '0'	:
						RefreshVTRKeys( GD_VT_PLAYF );
						VTR_Handler( PRESENTLIST_PTR, GD_VT_PLAYF );
						break;
			case '9'	:
						RefreshVTRKeys( GD_VT_FF );
						VTR_Handler( PRESENTLIST_PTR, GD_VT_FF );
						break;
			case '6'	:
						RefreshVTRKeys( GD_VT_PLAYF );
						VTR_Handler( PRESENTLIST_PTR, GD_VT_PLAYF );
						break;
			case '3'	:
						RefreshVTRKeys( GD_VT_TOGF );
						VTR_Handler( PRESENTLIST_PTR, GD_VT_TOGF );
						break;
			case '8'	:
						RefreshVTRKeys( GD_VT_BEG );
						VTR_Handler( PRESENTLIST_PTR, GD_VT_BEG );
						break;
			case '5'	:
						RefreshVTRKeys( GD_VT_STOP );
						VTR_Handler( PRESENTLIST_PTR, GD_VT_STOP );
						break;
			case '2'	:
						RefreshVTRKeys( GD_VT_END );
						VTR_Handler( PRESENTLIST_PTR, GD_VT_END );
						break;
			case '7'	:
						RefreshVTRKeys( GD_VT_REW );
						VTR_Handler( PRESENTLIST_PTR, GD_VT_REW );
						break;
			case '4'	:
						RefreshVTRKeys( GD_VT_PLAYB );
						VTR_Handler( PRESENTLIST_PTR, GD_VT_PLAYB );
						break;
			case '1'	:
						RefreshVTRKeys( GD_VT_TOGB );
						VTR_Handler( PRESENTLIST_PTR, GD_VT_TOGB );
						break;
		}
	}

	else

	switch ( toupper(ListEditorMsg->Code)) {
	//======  KeyBoard activated gadgets ======
		case	'N':
						ShowStatus("Type in notes for the item selected.");
						ActivateGadget( ListEditorGadgets[GD_Note], ListEditorWnd, 0 );
						break;
		case	'I':
						if ( active ){
							if ( VTR_CtrlWnd )
								ActivateWindow( VTR_CtrlWnd );
							else {
								OpenVTR_CtrlWin();
								TCodes2Gads( PRESENTLIST_PTR );
								ShowCurrentMode();
							}
							ShowStatus("Type in your shot Source In time code.");
							if( !VTR_CtrlGadgets[GD_SourceIn]->Flags & GFLG_DISABLED )
							ActivateGadget( VTR_CtrlGadgets[GD_SourceIn], VTR_CtrlWnd, 0 );
						}
						break;
		case	'O':
						if ( active ){
							if ( VTR_CtrlWnd )
								ActivateWindow( VTR_CtrlWnd );
							else {
								OpenVTR_CtrlWin();
								TCodes2Gads( PRESENTLIST_PTR );
								ShowCurrentMode();
							}
							ShowStatus("Type in your shot Source Out time code.");
							if( !VTR_CtrlGadgets[GD_SourceOut]->Flags & GFLG_DISABLED )
							ActivateGadget( VTR_CtrlGadgets[GD_SourceOut], VTR_CtrlWnd, 0 );
						}
						break;
		case	'D':
						if ( active ){
							if ( VTR_CtrlWnd )
								ActivateWindow( VTR_CtrlWnd );
							else {
								OpenVTR_CtrlWin();
								TCodes2Gads( PRESENTLIST_PTR );
								ShowCurrentMode();
							}
							ShowStatus("Type in your shot Duration time code.");
							if( !VTR_CtrlGadgets[GD_Duration]->Flags & GFLG_DISABLED )
							ActivateGadget( VTR_CtrlGadgets[GD_Duration], VTR_CtrlWnd, 0 );
						}
						break;
		case	'K':
						if ( active ){
							if ( VTR_CtrlWnd )
								ActivateWindow( VTR_CtrlWnd );
							else {
								OpenVTR_CtrlWin();
								TCodes2Gads( PRESENTLIST_PTR );
								ShowCurrentMode();
							}
							ShowStatus("Type in your shot KeyFrame time code.");
							if( !VTR_CtrlGadgets[GD_KeyFrame]->Flags & GFLG_DISABLED )
							ActivateGadget( VTR_CtrlGadgets[GD_KeyFrame], VTR_CtrlWnd, 0 );
						}
						break;
	}

	switch ( ListEditorMsg->Code ) {
		case	'×':	running=FALSE;
						break;							// ALT - x
		case	 1 :
						sprintf( txtstr, "Free RAM = %ld/%ld, Total = %ld",
								AvailMem( MEMF_CHIP|MEMF_LARGEST ),
								AvailMem( MEMF_FAST|MEMF_LARGEST ),
								AvailMem( MEMF_CHIP|MEMF_LARGEST )+AvailMem( MEMF_FAST|MEMF_LARGEST ) );
						ShowStatus( txtstr );
						break;
		case	'„':
						ShowStatus("DEBUG SHELL ACTIVE.");
						DebugShell();
						ShowStatus("OK.");
						break;
		case	13 :
						pel = PRESENTLISTDATA_PTR->lst_pos;
						if ( (PRESENTNODE_PTR->ListNode.marker&DT_COMMENT)!=DT_COMMENT  &&
							 (PRESENTNODE_PTR->ListNode.marker&DT_SEGHEAD)!=DT_SEGHEAD ) {
							switch( CURRENTBIGMODE ) {
								case MF_TAPEMODE :
								case MF_REELMODE :
								case MF_SHOTMODE :
									if ( glob_Qual&IEQUALIFIER_LSHIFT || glob_Qual&IEQUALIFIER_RSHIFT ) {
										CurrElementSet( CES_TOGGLEMARK, Project.CurrentWindow->listhead, Project.CurrentWindow->listhead->HeadNode.HeadData->lst_pos );
										RefreshXElement( Project.CurrentWindow, Project.CurrentWindow->listhead->HeadNode.HeadData->lst_pos );
										RefreshCursor( Project.CurrentWindow );
									}
									break;
								case MF_EDLMODE :
									if ( glob_Qual&IEQUALIFIER_LSHIFT || glob_Qual&IEQUALIFIER_RSHIFT ) {
										CurrElementSet( CES_TOGGLEMARK, Project.CurrentWindow->listhead, Project.CurrentWindow->listhead->HeadNode.HeadData->lst_pos );
										RefreshXElement( Project.CurrentWindow, Project.CurrentWindow->listhead->HeadNode.HeadData->lst_pos );
										RefreshCursor( Project.CurrentWindow );
										SetTimeLineNode();
									}
									break;

								case MF_BIGTAPEMODE :
								case MF_BIGSHOTMODE :
									SetWindowList( Project.CurrentWindow, pel );
									RefreshCurrWindow();
									break;
								case MF_BIGEDLMODE  :
									SetWindowList( Project.CurrentWindow, pel );
									RefreshCurrWindow();
									EDLInfoRender();
									break;
							}
						}
						break;

		// TAB key
		case 9	:
					temp = TransferElements( Project.LeftWin.listhead, Project.RightWin.listhead, 'C' );
					if ( temp == -1) {
						ErrorHandler(	ERRH_Type,		ERRH_ERROR,
										ERRH_Options,	ERRH_ErrorText,
										ERRH_Text,		ERRH_CANTXFERTXT,
										TAG_DONE );
					}
					else {
						if ( temp > 1 )
							sprintf(txtstr,"%d shots have been transfered to the EDL",temp);
						else
							sprintf(txtstr,"%d shot has been transfered to the EDL",temp);
						ErrorHandler(	ERRH_Type,		ERRH_MSG,
										ERRH_Options,	ERRH_StatusText,
										ERRH_Text,		txtstr,
										TAG_DONE );
						RefreshAllWindows();
						EDLInfoRender();
					}
								break;

	}
	return running;
}







/**********************************************************************
 *
 *						ListEditorRawKey()
 *	Description : goes here if any key is pressed  (hardware key matrix)
 *  Returns		:
 *	Globals		:
 *
 */
BOOL ListEditorRawKey(
	struct IntuiMessage   *ListEditorMsg
){

	glob_Code = ListEditorMsg->Code;
	glob_Qual = ListEditorMsg->Qualifier;

		// shifted keys
	if ( ListEditorMsg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT) ){
		switch ( ListEditorMsg->Code ) {
			case KEY_UP		:
								GotoPage( Project.CurrentWindow, GP_CURSUP );
								break;
			case KEY_DOWN	:
								GotoPage( Project.CurrentWindow, GP_CURSDOWN );
								break;

			case KEY_F1		:
								SetWindowDBase( Project.CurrentWindow, DB_TAPELIST );
								ShowCurrentMode();
								break;
			case KEY_F2		:
								SetWindowDBase( Project.CurrentWindow, DB_REELLIST );
								ShowCurrentMode();
								break;

			case KEY_F5		:	break;
			case KEY_F6		:	break;
			case KEY_F7		:	break;
			case KEY_F8		:	break;
			case KEY_F9		:	break;
			case KEY_F10	:	break;
		}
	}

	else

		// alted keys
	if ( ListEditorMsg->Qualifier & IEQUALIFIER_LALT ||
		 ListEditorMsg->Qualifier & IEQUALIFIER_RALT ){
		switch ( ListEditorMsg->Code ) {
			case KEY_UP		:
								if ( Project.PreviousWindow->listhead == Project.CurrentWindow->listhead )
									GotoPage( Project.PreviousWindow, GP_TOP );
								GotoPage( Project.CurrentWindow, GP_TOP );
								if ( MODE( Project.CurrentWindow->listhead) == MF_EDLMODE )
									SetTimeLineNode();
								break;
			case KEY_DOWN	:
								if ( Project.PreviousWindow->listhead == Project.CurrentWindow->listhead )
									GotoPage( Project.PreviousWindow, GP_END );
								GotoPage( Project.CurrentWindow, GP_END );
								if ( MODE( Project.CurrentWindow->listhead) == MF_EDLMODE )
									SetTimeLineNode();
								break;

			case KEY_RIGHT	:
								break;
			case KEY_LEFT	:
								break;
		}
	}

	else


		switch ( ListEditorMsg->Code ) {
						// function keys
			case KEY_F1		:
								SetWindowList( Project.CurrentWindow, LS_INDEX );
								break;
			case KEY_F2		:
								SetWindowList( Project.CurrentWindow, LS_MAIN );
								break;
			case KEY_F3		:
								SetWindowList( Project.CurrentWindow, LS_SCRATCH );
								break;
			case KEY_F4		:
								SetWindowList( Project.CurrentWindow, LS_MINUS );
								break;
			case KEY_F5		:
								SetWindowList( Project.CurrentWindow, LS_PLUS );
								break;



			case KEY_F6		:	if(VTR_CtrlWnd)
									CloseVTR_CtrlWin();
								else {
									OpenVTR_CtrlWin();
									Node2Gads( PRESENTLIST_PTR );
									RefreshLiveVTR();
									ShowCurrentMode();
								}
								break;

			case KEY_F7		:
								HandleKeyEditor();
								break;

			case KEY_F8		:	if(TimeLineWnd)
									CloseTimeLineWindow();
								else
									printf("%d\n",OpenTimeLineWindow());
								break;

			case KEY_F9		:	if(DTAGS0Wnd)
									CloseDTAGSrequestor();
								else
									OpenDTAGSrequestor();
								break;

			case KEY_F10	:	if(EditDisplayWnd)
									CloseEditDisplayWin();
								else
									OpenEditDisplayWin();
								break;



						// movement
			case KEY_RIGHT	:	CurrentModeNext( CM_INCMODE, 1 );
								Node2Gads( Project.CurrentWindow->listhead );
								break;

			case KEY_LEFT	:	CurrentModeNext( CM_INCMODE, -1 );
								Node2Gads( Project.CurrentWindow->listhead );
								break;

			case KEY_DOWN	:
								GotoPage( Project.CurrentWindow, GP_CURSDOWN );
								break;
			case KEY_UP		:
								GotoPage( Project.CurrentWindow, GP_CURSUP );
								break;

						// misc
			case KEY_HELP	:
								ShowStatus("Help Activated.");
								IssueHelp( 0, 0 );
								break;
			case KEY_RMOUSE :	Forbid();
								Permit();
								break;
		}

	return TRUE;
}










/**********************************************************************
 *
 *						ClearMsgPort
 *
 *	Description : Clears all message from a message port.
 *	Returns		:
 *	Globals		:
 *
 */
void ClearMsgPort(
	struct MsgPort *mport
){
    struct IntuiMessage  *msg;

    while ( msg = GT_GetIMsg( mport ) )
		GT_ReplyIMsg( msg );
}



