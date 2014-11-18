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

#include "NetStatGUI.h"

#include "editor.h"

struct Window         *NetStatWnd = NULL;
struct Gadget         *NetStatGList = NULL;
struct Gadget         *NetStatGadgets[NetStat_CNT];
UWORD                  NetStatLeft = 65;
UWORD                  NetStatTop = 131;
UWORD                  NetStatWidth = 500;
UWORD                  NetStatHeight = 220;
UBYTE                 *NetStatWdt = (UBYTE *)"NetStat";





UWORD NetStatGTypes[] = {

};

struct NewGadget NetStatNGad[] = {

};

ULONG NetStatGTags[] = {

};





void NetStatRender( void )
{
	UWORD		offx, offy;

	offx = NetStatWnd->BorderLeft;
	offy = NetStatWnd->BorderTop;

	Draw3DBox(  NetStatWnd->RPort, offx + 0, offy +  0, NetStatWidth-0, NetStatHeight, PEN_MBEV_SHINE, PEN_MBEV_BODY, PEN_MBEV_SHADOW );

//	PrintIText( NetStatWnd->RPort, NetStatIText, offx, offy );
}





int OpenNetStatWindow( void )
{
	struct NewGadget	ng;
	struct Gadget	*g;
	UWORD		lc, tc;
	UWORD		offx = Scr->WBorLeft, offy = Scr->WBorTop + Scr->RastPort.TxHeight + 1;

	if ( ! ( g = CreateContext( &NetStatGList )))
		return( 1L );

	for( lc = 0, tc = 0; lc < NetStat_CNT; lc++ ) {

		CopyMem((char * )&NetStatNGad[ lc ], (char * )&ng, (long)sizeof( struct NewGadget ));

		ng.ng_VisualInfo = VisualInfo;
		ng.ng_TextAttr   = &helvetica13;
		ng.ng_LeftEdge  += offx;
		ng.ng_TopEdge   += offy;

		NetStatGadgets[ lc ] = g = CreateGadgetA((ULONG)NetStatGTypes[ lc ], g, &ng, ( struct TagItem * )&NetStatGTags[ tc ] );

		while( NetStatGTags[ tc ] ) tc += 2;
		tc++;

		if ( NOT g )
			return( 2L );
	}

	if ( ! ( NetStatWnd = OpenWindowTags( NULL,
				WA_Left,		NetStatLeft,
				WA_Top,			NetStatTop,
				WA_InnerWidth,	NetStatWidth,
				WA_InnerHeight,	NetStatHeight,
				WA_IDCMP,		CHECKBOXIDCMP|CYCLEIDCMP|IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|IDCMP_RAWKEY|IDCMP_VANILLAKEY|IDCMP_MENUPICK,
				WA_Flags,		WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_CLOSEGADGET|WFLG_SMART_REFRESH|WFLG_ACTIVATE,
				WA_Title,		NetStatWdt,
				WA_PubScreen,	Scr,
				TAG_DONE )))
	return( 4L );

	NetStatRender();

//	AddGList(NetStatWnd, NetStatGadgets[0],NULL,NULL,NULL);
//	RefreshGList(NetStatGadgets[0], NetStatWnd,NULL,NULL);

	return( 0L );
}

void CloseNetStatWindow( void )
{
	if ( NetStatWnd        ) {
		CloseWindow( NetStatWnd );
		NetStatWnd = NULL;
	}

	if ( NetStatGList      ) {
		FreeGadgets( NetStatGList );
		NetStatGList = NULL;
	}
}



#define	IC_XS	40
#define	IC_YS	48

	LONG	net_details[10][5];
	LONG	numof_servers=0;
	LONG	numof_managers[10]=0;
void GetNetDetails( void ){
	LONG	most_managers=0;

//	if( numof_servers ){
//		LONG lp;
//		for( lp=0; lp< numof_servers )
//			if( numof_managers[lp] > most_managers )
//				most_managers=numof_managers[lp];
//	}
//	height = (most_managers*40*2) + 40
//	width = (numof_servers*2) + 40
}


void DrawNetStat( void ){
	struct Image *im;
	LONG	x,y;

	if( x )
	for( x=0;x<numof_servers;x++ ){
		if( y )
		for(y=0;y<numof_managers[y];y++ ){
			// show server icon
			if( x && y==0) {
				if( im = LoadImage( "mediaflex:iff/server.iff" ) ){
					DrawImage( im , 20+(x*IC_XS*2), 20 );
					FreeImage( im );
					//ShowServerName
				}
			}
			// show horiz arrow
			if( x<(numof_servers-1) && y==0) {
				if( im = LoadImage( "mediaflex:iff/netlink.iff" ) ){
					DrawImage( im , 20+(x*IC_XS*2)+IC_XS, 20 );
					FreeImage( im );
				}
			}

			// show vert arrow
			if( y>0) {
				if( im = LoadImage( "mediaflex:iff/varrow.iff" ) ){
					DrawImage( im , 20+(x*IC_XS*2), 20+(y*IC_YS*2)+IC_YS );
					FreeImage( im );
				}
			}
			// show manager icon
			if( y>0 ) {
				if( im = LoadImage( "mediaflex:iff/video.iff" ) ){
					DrawImage( im , 20+(x*IC_XS*2), 20+(y*IC_YS*2) );
					FreeImage( im );
					//ShowManagerName
				}
			}
			
		}
	}
}



