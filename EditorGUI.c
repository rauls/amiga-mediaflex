/*
 *  Source machine generated by GadToolsBox V2.0
 *  which is (c) Copyright 1991-1993 Jaba Development
 *
 *  GUI Designed by : CCS
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxbase.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <string.h>

#include "editorGUI.h"

#include "Editor.h"

struct Screen         *Scr = NULL;
APTR                   VisualInfo = NULL;
struct Window         *ListEditorWnd = NULL;
struct Window         *TempWnd = NULL;
struct Window         *ObjWnd = NULL;
struct Gadget         *ListEditorGList = NULL;
struct Menu           *ListEditorMenus = NULL;
struct Gadget         *ListEditorGadgets[33];
UWORD                  ListEditorLeft = 0;
UWORD                  ListEditorTop = 0;
UWORD                  ListEditorWidth = 1280;
UWORD                  ListEditorHeight = 755;
UBYTE                 *ListEditorWdt = (UBYTE *)"ListEditor";

struct IntuiText ListEditorIText[] = {
	13, 0, JAM1,23, 689, &mflex10, (UBYTE *)"Descriptor", &ListEditorIText[1],
	13, 0, JAM1,37, 710, &mflex10, (UBYTE *)"Comments", &ListEditorIText[2],
	1, 0, JAM1,500, 113, &mflex10, (UBYTE *)"Edit", NULL };

struct NewMenu ListEditorNewMenu[] = {
	NM_TITLE, (STRPTR)"System", NULL, 0, NULL, NULL,
	NM_ITEM, (STRPTR)"Project", NULL, 0, NULL, NULL,
	NM_SUB, (STRPTR)"Open", NULL, 0, 0L, NULL,
	NM_SUB, (STRPTR)"Save As", NULL, 0, 0L, NULL,
	NM_SUB, (STRPTR)"New", NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"List", NULL, 0, NULL, NULL,
	NM_SUB, (STRPTR)"Open CMX", NULL, 0, 0L, NULL,
	NM_SUB, (STRPTR)"Open List", NULL, 0, 0L, NULL,
	NM_SUB, (STRPTR)"Open from Project", NULL, 0, 0L, NULL,
	NM_SUB, (STRPTR)"Save CMX", NULL, 0, 0L, NULL,
	NM_SUB, (STRPTR)"Save List", NULL, 0, 0L, NULL,
	NM_SUB, (STRPTR)"New", NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Font Height", NULL, 0, NULL, NULL,
	NM_SUB, (STRPTR)" Small  (8)", NULL, CHECKIT, 62L, NULL,
	NM_SUB, (STRPTR)" Medium (10)", NULL, CHECKIT, 61L, NULL,
	NM_SUB, (STRPTR)" Large  (12)", NULL, CHECKIT|CHECKED, 59L, NULL,
	NM_SUB, (STRPTR)" Larger (14)", NULL, CHECKIT, 55L, NULL,
	NM_SUB, (STRPTR)" Largest(16)", NULL, CHECKIT, 47L, NULL,
	NM_SUB, (STRPTR)" HUGE!  (18)", NULL, CHECKIT, 31L, NULL,
	NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Exit", (STRPTR)"q", 0, 0L, NULL,
	NM_TITLE, (STRPTR)"Edit", NULL, 0, NULL, NULL,
	NM_ITEM, (STRPTR)"New", (STRPTR)"n", 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Delete", (STRPTR)"d", 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Duplicate", (STRPTR)"z", 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Cut", (STRPTR)"x", 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Copy", (STRPTR)"c", 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Paste", (STRPTR)"v", 0, 0L, NULL,

	NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL,

	NM_ITEM, (STRPTR)"Scene View", NULL, CHECKED|MENUTOGGLE, NULL, NULL,
	NM_SUB, (STRPTR)"All", NULL, CHECKIT|CHECKED, 0L, NULL,
	NM_SUB, (STRPTR)"Selected", NULL, CHECKIT, 0L, NULL,
	NM_SUB, (STRPTR)"Found", NULL, CHECKIT, 0L, NULL,
	NM_SUB, (STRPTR)"Hidden", NULL, CHECKIT, 0L, NULL,
	NM_SUB, (STRPTR)"Audio", NULL, CHECKIT, 0L, NULL,
	NM_SUB, (STRPTR)"Video", NULL, CHECKIT, 0L, NULL,
	NM_SUB, (STRPTR)"Graphics", NULL, CHECKIT, 0L, NULL,

	NM_ITEM, (STRPTR)"EDL View", NULL, CHECKED|MENUTOGGLE, NULL, NULL,
	NM_SUB, (STRPTR)"All", NULL, CHECKIT|CHECKED, 0L, NULL,
	NM_SUB, (STRPTR)"Selected", NULL, CHECKIT, 0L, NULL,
	NM_SUB, (STRPTR)"Found", NULL, CHECKIT, 0L, NULL,
	NM_SUB, (STRPTR)"Hidden", NULL, CHECKIT, 0L, NULL,
	NM_SUB, (STRPTR)"Audio", NULL, CHECKIT, 0L, NULL,
	NM_SUB, (STRPTR)"Video", NULL, CHECKIT, 0L, NULL,
	NM_SUB, (STRPTR)"Graphics", NULL, CHECKIT, 0L, NULL,

	NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Add Segment", NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Add Comment", NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Lock Item", NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"UnLock Item", NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Protect Item", NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"UnProtect Item", NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Sync Item", NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"UnSync Item", NULL, 0, 0L, NULL,

	NM_TITLE, (STRPTR)"Post", NULL, 0, NULL, NULL,
	NM_ITEM, (STRPTR)"Input Video Stream", NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Input Audio Reel", NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)NM_BARLABEL, NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Import Video Reels", NULL, 0, 0L, NULL,
	NM_ITEM, (STRPTR)"Import Audio Reels", NULL, 0, 0L, NULL,
	NM_TITLE, (STRPTR)"Misc", NULL, 0, NULL, NULL,
	NM_ITEM, (STRPTR)"Palette", (STRPTR)"p", 0, 0L, NULL,
	NM_ITEM, (STRPTR)"DOS Shell", (STRPTR)"s", 0, 0L, NULL,
	NM_TITLE, (STRPTR)" ", NULL, 0, NULL, NULL,
	NM_ITEM, (STRPTR)"DBase Debugger", NULL, 0, 0L, NULL,
	NM_END, NULL, NULL, 0, 0L, NULL };

UWORD ListEditorGTypes[] = {
	STRING_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	INTEGER_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND,
	BUTTON_KIND
};

struct NewGadget ListEditorNGad[] = {
	101, 730, 916, 16, (UBYTE *)"Notes", NULL, GD_Note, PLACETEXT_LEFT|NG_HIGHLABEL, NULL, NULL,
	491, 345, 43, 15, (UBYTE *)"����", NULL, GD_XFER, PLACETEXT_IN, NULL, NULL,
	491, 328, 43, 15, (UBYTE *)"NONE", NULL, GD_ED_NONE, PLACETEXT_IN, NULL, NULL,
	491, 294, 43, 15, (UBYTE *)"ALL", NULL, GD_ED_ALL, PLACETEXT_IN, NULL, NULL,
	491, 311, 43, 15, (UBYTE *)"INV", NULL, GD_ED_INV, PLACETEXT_IN, NULL, NULL,
	491, 506, 43, 15, (UBYTE *)"PREV", NULL, GD_ED_PREV, PLACETEXT_IN, NULL, NULL,
	491, 523, 43, 16, (UBYTE *)"NEXT", NULL, GD_ED_NEXT, PLACETEXT_IN, NULL, NULL,
	491, 472, 43, 15, (UBYTE *)"TOP", NULL, GD_ED_TOP, PLACETEXT_IN, NULL, NULL,
	491, 489, 43, 15, (UBYTE *)"END", NULL, GD_ED_END, PLACETEXT_IN, NULL, NULL,
	491, 197, 43, 15, (UBYTE *)"CUT", NULL, GD_ED_CUT, PLACETEXT_IN, NULL, NULL,
	491, 162, 43, 16, (UBYTE *)"DUP#", NULL, GD_ED_DUP, PLACETEXT_IN, NULL, NULL,
	491, 145, 43, 15, (UBYTE *)"DEL", NULL, GD_ED_DEL, PLACETEXT_IN, NULL, NULL,
	491, 214, 43, 15, (UBYTE *)"COPY", NULL, GD_ED_COPY, PLACETEXT_IN, NULL, NULL,
	491, 128, 43, 15, (UBYTE *)"NEW", NULL, GD_ED_NEW, PLACETEXT_IN, NULL, NULL,
	491, 265, 43, 15, (UBYTE *)"UNDO", NULL, GD_ED_UNDO, PLACETEXT_IN, NULL, NULL,
	491, 231, 43, 15, (UBYTE *)"PASTE", NULL, GD_ED_PASTE, PLACETEXT_IN, NULL, NULL,
	491, 76, 43, 15, (UBYTE *)"S � S", NULL, GD_ED_S_S, PLACETEXT_IN, NULL, NULL,
	491, 59, 43, 15, (UBYTE *)"E � E", NULL, GD_ED_E_E, PLACETEXT_IN, NULL, NULL,
	491, 42, 43, 15, (UBYTE *)"S � E", NULL, GD_ED_S_E, PLACETEXT_IN, NULL, NULL,
	491, 620, 43, 15, (UBYTE *)"PREFS", NULL, GD_ED_PREFS, PLACETEXT_IN, NULL, NULL,
	491, 603, 43, 15, (UBYTE *)"TAGS", NULL, GD_ED_TAGS, PLACETEXT_IN, NULL, NULL,
	491, 586, 43, 15, (UBYTE *)"TIME", NULL, GD_ED_TIME, PLACETEXT_IN, NULL, NULL,
	491, 569, 43, 15, (UBYTE *)"PICS", NULL, GD_ED_PICS, PLACETEXT_IN, NULL, NULL,
	491, 93, 43, 15, (UBYTE *)"T � R", NULL, GD_ED_T_R, PLACETEXT_IN, NULL, NULL,
	491, 552, 43, 15, (UBYTE *)"VTR", NULL, GD_ED_VTR, PLACETEXT_IN, NULL, NULL,
	491, 455, 43, 15, (UBYTE *)"PAGE#", NULL, GD_ED_PAGE, PLACETEXT_IN, NULL, NULL,
	7, -10, 102, 14, (UBYTE *)"Project", NULL, GD_IO_REQ, PLACETEXT_IN, NULL, NULL,
	491, 179, 43, 17, NULL, NULL, GD_ED_DUPN, 0, NULL, NULL,
	491, 248, 43, 15, (UBYTE *)"SPLIT", NULL, GD_ED_SPLIT, PLACETEXT_IN, NULL, NULL,
	491, 409, 43, 16, (UBYTE *)"LOCK", NULL, GD_ED_LOCK, PLACETEXT_IN, NULL, NULL,
	491, 427, 43, 17, (UBYTE *)"UNLOC", NULL, GD_ED_UNLOCK, PLACETEXT_IN, NULL, NULL,
	491, 373, 43, 16, (UBYTE *)"SYNC", NULL, GD_ED_SYNC, PLACETEXT_IN, NULL, NULL,
	491, 391, 43, 16, (UBYTE *)"UNSYN", NULL, GD_ED_UNSYNC, PLACETEXT_IN, NULL, NULL
};

ULONG ListEditorGTags[] = {
	(GTST_MaxChars), 128, (TAG_DONE),
	(TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (GA_Disabled), TRUE, (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GTIN_Number), 1, (GTIN_MaxChars), 3, (STRINGA_Justification), (GACT_STRINGRIGHT), (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE),
	(GT_Underscore), '_', (TAG_DONE)
};

struct ColorSpec ScreenColors[] = {
	 0, 0x08, 0x08, 0x08,
	 1, 0x00, 0x00, 0x00,
	 2, 0x00, 0x08, 0x08,
	 3, 0x05, 0x05, 0x05,
	 4, 0x0A, 0x0A, 0x0A,
	 5, 0x00, 0x0C, 0x0F,
	 6, 0x02, 0x07, 0x0A,
	 7, 0x0F, 0x0F, 0x0F,
	 8, 0x06, 0x02, 0x00,
	 9, 0x0E, 0x05, 0x00,
	10, 0x00, 0x07, 0x08,
	11, 0x0E, 0x0B, 0x00,
	12, 0x0B, 0x0F, 0x00,
	13, 0x0F, 0x0F, 0x00,
	14, 0x00, 0x0F, 0x08,
	15, 0x0C, 0x0C, 0x0C,
	~0, 0x00, 0x00, 0x00 };

UWORD DriPens[] = {
	2,1,1,7,3,6,7,4,13,7};

int SetupScreen( void )
{
	if ( ! ( Scr = OpenScreenTags( NULL, SA_Left,	0,
					SA_Top,		0,
					SA_Width,	1280,
					SA_Height,	768,
					SA_Depth,	4,
					SA_Colors,	&ScreenColors[0],
					SA_Font,	&mflex10,
					SA_Type,	CUSTOMSCREEN,
					SA_DisplayID,	PAL_MONITOR_ID|LORESLACE_KEY,
					SA_AutoScroll,	TRUE,
					SA_Overscan,	OSCAN_TEXT,
					SA_Pens,	&DriPens[0],
					SA_Title,	"MF � 93",
					TAG_DONE )))
		return( 1L );

	if ( ! ( VisualInfo = GetVisualInfo( Scr, TAG_DONE )))
		return( 2L );

	return( 0L );
}

void CloseDownScreen( void )
{
	if ( VisualInfo ) {
		FreeVisualInfo( VisualInfo );
		VisualInfo = NULL;
	}

	if ( Scr        ) {
		CloseScreen( Scr );
		Scr = NULL;
	}
}

void ListEditorRender( void )
{
	UWORD		offx, offy;

	offx = 0;
	offy = Scr->WBorTop + Scr->Font->ta_YSize + 1 -8;

	// MAIN BIG WIN PART I
	Draw3DBox( ListEditorWnd->RPort, offx +    0, offy + -12, 1280, 767, PEN_MBEV_SHINE, PEN_MBEV_BODY, PEN_MBEV_SHADOW);
	// horizontal lines @ bottom
	Draw3DLine(ListEditorWnd->RPort, offx +    1, offy + 678, 1278, PEN_MBEV_SHADOW, PEN_MBEV_SHINE);
	// lines @ top
	Draw3DLine(ListEditorWnd->RPort, offx +    1, offy +  05, 1278, PEN_MBEV_SHADOW, PEN_MBEV_SHINE);
//	Draw3DLine(ListEditorWnd->RPort, offx +    1, offy +  28, 1278, PEN_MBEV_SHADOW, PEN_MBEV_SHINE);

	// LEFT WIN
	Draw3DBox( ListEditorWnd->RPort, offx +    6, offy +  42,  484, 604, PEN_MBEV_SHADOW, PEN_LISTS_BACK, PEN_MBEV_SHINE);
	// RIGHT WIN
	Draw3DBox( ListEditorWnd->RPort, offx +  536, offy +  42,  485, 604, PEN_MBEV_SHADOW, PEN_LISTS_BACK, PEN_MBEV_SHINE);
	// SMALL WIN
//	Draw3DBox( ListEditorWnd->RPort, offx + 1028, offy +  42,  243, 604, PEN_MBEV_SHADOW, PEN_LISTS_BACK, PEN_MBEV_SHINE);

	RefreshGadgets( ListEditorGadgets[0],ListEditorWnd, NULL );
	PrintIText( ListEditorWnd->RPort, ListEditorIText, offx, offy );

	PrintIText( ListEditorWnd->RPort, ListEditorIText, offx, offy );
}

int OpenListEditorWindow( void )
{
	struct NewGadget	ng;
	struct Gadget	*g;
	UWORD		lc, tc;
	UWORD		offx = 0, offy = Scr->WBorTop + Scr->RastPort.TxHeight + 1-8;

	if ( ! ( g = CreateContext( &ListEditorGList )))
		return( 1L );

	for( lc = 0, tc = 0; lc < ListEditor_CNT; lc++ ) {

		CopyMem((char * )&ListEditorNGad[ lc ], (char * )&ng, (long)sizeof( struct NewGadget ));

		ng.ng_VisualInfo = VisualInfo;
		ng.ng_TextAttr   = &mflex12;
		ng.ng_LeftEdge  += offx;
		ng.ng_TopEdge   += offy;

		ListEditorGadgets[ lc ] = g = CreateGadgetA((ULONG)ListEditorGTypes[ lc ], g, &ng, ( struct TagItem * )&ListEditorGTags[ tc ] );

		while( ListEditorGTags[ tc ] ) tc += 2;
		tc++;

		if ( NOT g )
			return( 2L );
	}

	if ( ! ( ListEditorMenus = CreateMenus( ListEditorNewMenu,
					GTMN_FrontPen,		2L,
					TAG_DONE )))
		return( 3L );

	LayoutMenus( ListEditorMenus, VisualInfo,
					GTMN_TextAttr, 		&helvetica18,
					GTMN_NewLookMenus,	TRUE,
					TAG_DONE );

	if ( ! ( TempWnd = OpenWindowTags( NULL,
				WA_Left,	0,
				WA_Top,		0,
				WA_Width,	1280,
				WA_Height,	60,
				WA_IDCMP,	0,
				WA_Flags,	WFLG_SMART_REFRESH|WFLG_BACKDROP|WFLG_BORDERLESS,
				WA_CustomScreen,	Scr,
				TAG_DONE )))
	return( 4L );

	if ( ! ( ObjWnd = OpenWindowTags( NULL,
				WA_Left,	40,
				WA_Top,		0,
				WA_Width,	640,
				WA_Height,	30,
				WA_IDCMP,	0,
				WA_Flags,	WFLG_SMART_REFRESH|WFLG_BACKDROP|WFLG_BORDERLESS,
				WA_CustomScreen,	Scr,
				TAG_DONE )))
	return( 4L );

	if ( ! ( ListEditorWnd = OpenWindowTags( NULL,
				WA_Left,	ListEditorLeft,
				WA_Top,		ListEditorTop,
				WA_Width,	ListEditorWidth,
				WA_Height,	ListEditorHeight + offy,
				WA_IDCMP,	IDCMP_MOUSEBUTTONS|STRINGIDCMP|CHECKBOXIDCMP|SCROLLERIDCMP|
							BUTTONIDCMP|SLIDERIDCMP|CYCLEIDCMP|MXIDCMP|
							IDCMP_MENUPICK|IDCMP_RAWKEY|IDCMP_VANILLAKEY|
							IDCMP_REFRESHWINDOW,
				WA_Flags,	WFLG_NEWLOOKMENUS|WFLG_SMART_REFRESH|WFLG_BACKDROP|
							WFLG_BORDERLESS|WFLG_REPORTMOUSE|WFLG_ACTIVATE,
			//	WA_Gadgets,	ListEditorGList,
				WA_ScreenTitle,	"MediaFlex",
				WA_CustomScreen,	Scr,
				TAG_DONE )))
	return( 4L );

	SetMenuStrip( ListEditorWnd, ListEditorMenus );

	ListEditorRender();

	AddGList(ListEditorWnd, ListEditorGadgets[0],NULL,NULL,NULL);
	RefreshGList(ListEditorGadgets[0], ListEditorWnd,NULL,NULL);

	return( 0L );
}

void CloseListEditorWindow( void )
{
	if ( ListEditorMenus      ) {
		ClearMenuStrip( ListEditorWnd );
		FreeMenus( ListEditorMenus );
		ListEditorMenus = NULL;	}

	if ( ListEditorWnd        ) {
		CloseWindow( ListEditorWnd );
		ListEditorWnd = NULL;
	}

	if ( ListEditorGList      ) {
		FreeGadgets( ListEditorGList );
		ListEditorGList = NULL;
	}
}

