// tkWinCE.c
//
// Time-stamp: <20/03/02 08:40:26 keuchel@netwave.de>

#include "tkWinInt.h"
#include <stdio.h>
#include <celib.h>
#include <time.h>

#define MAXWINMAPS 20

typedef struct {
    HWND hWndOld;
    HWND hWndNew;
} WINMAPENT;

WINMAPENT winmaptab[MAXWINMAPS];

// Store/get window handles for recreated windows...
// This is currently used for getting the right window handle for
// fonts. See tkWinFont.c.

// Stupid names...

void
wince_putwinhandles(HWND hWndOld, HWND hWndNew)
{
    int i;

    for(i = 0; i < MAXWINMAPS; i++) {
	if(winmaptab[i].hWndOld == NULL) {
	    winmaptab[i].hWndOld = hWndOld;	  
	    winmaptab[i].hWndNew = hWndNew;	  
	    return;
	}
    }

    XCEShowMessageA("No more winhandle map slots");
}

HWND
wince_getwinhandle(HWND hWndOld)
{
    int i;

    for(i = 0; i < MAXWINMAPS; i++) {
	if(winmaptab[i].hWndOld == hWndOld)
	    return winmaptab[i].hWndNew;
    }

    return NULL;
}

//////////////////////////////////////////////////////////////////////
// Emulate CreateDIBitmap

HBITMAP 
CreateDIBitmap(HDC  hdc,
	BITMAPINFOHEADER *lpbmih,
	DWORD  fdwInit,
	VOID *lpBits,
	BITMAPINFO *lpBitsInfo,
	UINT  iUsage
    )
{
    HBITMAP hbitmap;
    LPBYTE lpDestBits;
    DWORD dwLen;
    LPBITMAPINFOHEADER lpH;
    int nPadWidth = 4;
    int pad = 0;
    DWORD dwError;
    DWORD i;
    BYTE *d;
    WORD *s;
    WORD *s0;
    int orig_bitcount;
    XImage *image = (XImage *)lpBits;
    lpBits = image->data;

    lpH = lpbmih;

    if (lpH->biWidth % nPadWidth) {
	pad = nPadWidth - lpH->biWidth % nPadWidth;
    }

    // there are problems with padding. i dont know the rules
    // anymore...
    pad = 0;

    // This is wrong when the infoheader is followed by color data...
    // We hope that biSizeImage is set then...
    dwLen = ((lpH->biWidth+pad) * abs(lpH->biHeight) * lpH->biBitCount) / 8;

    // includes colordata, if any...
    if (lpH->biSizeImage != 0) {
	dwLen = lpH->biSizeImage;
    }

    orig_bitcount = lpH->biBitCount;

    if (lpH->biBitCount == 16) {
	lpH->biBitCount = 24;
    }

    hbitmap = CreateDIBSection(hdc, lpBitsInfo, iUsage, &lpDestBits, NULL, 0);
    if (hbitmap != NULL) {
	if (orig_bitcount == 16) {
	    if (lpH->biCompression == BI_RGB) {
		s = (WORD *) lpBits;
		d = (BYTE *) lpDestBits;
		s0 = (WORD *) lpDestBits;
		// There is a bug in this code when padding was used!

		// how do you get the full color range from 5 bits???
		// shifting left seems to be ok...
#if 1
		dwLen = dwLen >> 1;
		for (i = 0; i < dwLen; i++) {
		    *d++ = (((*s) >> 0) & 0x1F) << 3;
		    *d++ = (((*s) >> 5) & 0x1F) << 3;
		    *d++ = (((*s) >> 10) & 0x1F) << 3;
		    s++;
		}
#endif
	    } else {
		XCEShowMessageA("Cannot create bitmap: wrong number of colors");
		//DebugBreak();
		return (HBITMAP) GDI_ERROR;
	    }
	} else {
	    memcpy(lpDestBits, lpBits, dwLen);
	}

	return hbitmap;
    }

    dwError = GetLastError();

    XCEShowMessageA("Cannot create bitmap: %d", dwError);
    //DebugBreak();

    return (HBITMAP) GDI_ERROR;
}

//////////////////////////////////////////////////////////////////////
#if CE_OLD
static int cursor_x, cursor_y;

// Dummy... How emulate correctly...

// The original function on ARM/HPC/WCE300 also returns error 120:
// unsupported function!

BOOL
WinCEGetCursorPos(POINT *p)
{
    p->x = cursor_x;
    p->y = cursor_y;

    return TRUE;
}

BOOL
WinCESetCursorPos(int x, int y)
{
    cursor_x = x;
    cursor_y = y;

    return TRUE;
}
#endif
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

/*
 *  Arc, Chord and Pie compatability functions for CE
 *  http://www.experts-exchange.com/Programming/Programming_Platforms/Win_Prog/Q_11141686.html
 */

BOOL CeArc(
    HDC hdc,         // handle to device context
    int nLeftRect,   // x-coord of upper-left corner of rectangle
    int nTopRect,    // y-coord of upper-left corner of rectangle
    int nRightRect,  // x-coord of lower-right corner of rectangle
    int nBottomRect, // y-coord of lower-right corner of rectangle
    int nXStartArc,  // x-coord of first radial's endpoint
    int nYStartArc,  // y-coord of first radial's endpoint
    int nXEndArc,    // x-coord of second radial's endpoint
    int nYEndArc     // y-coord of second radial's endpoint
    );

BOOL CeChord(
    HDC hdc,         // handle to device context
    int nLeftRect,   // x-coord of upper-left corner of rectangle
    int nTopRect,    // y-coord of upper-left corner of rectangle
    int nRightRect,  // x-coord of lower-right corner of rectangle
    int nBottomRect, // y-coord of lower-right corner of rectangle
    int nXRadial1,   // x-coord of first radial's endpoint
    int nYRadial1,   // y-coord of first radial's endpoint
    int nXRadial2,   // x-coord of second radial's endpoint
    int nYRadial2    // y-coord of second radial's endpoint
    );

BOOL CePie(
    HDC hdc,         // handle to device context
    int nLeftRect,   // x-coord of upper-left corner of rectangle
    int nTopRect,    // y-coord of upper-left corner of rectangle
    int nRightRect,  // x-coord of lower-right corner of rectangle
    int nBottomRect, // y-coord of lower-right corner of rectangle
    int nXRadial1,   // x-coord of first radial's endpoint
    int nYRadial1,   // y-coord of first radial's endpoint
    int nXRadial2,   // x-coord of second radial's endpoint
    int nYRadial2    // y-coord of second radial's endpoint
    );

static LPPOINT
GetArcPoints(
    int nLeftRect,   // [IN] x-coord of upper-left corner of rectangle
    int nTopRect,    // [IN] y-coord of upper-left corner of rectangle
    int nRightRect,  // [IN] x-coord of lower-right corner of rectangle
    int nBottomRect, // [IN] y-coord of lower-right corner of rectangle
    int nXStartArc,  // [IN] x-coord of first radial's endpoint
    int nYStartArc,  // [IN] y-coord of first radial's endpoint
    int nXEndArc,    // [IN] x-coord of second radial's endpoint
    int nYEndArc,    // [IN] y-coord of second radial's endpoint
    int *Points     // [IN] zero: Arc, Chord; nonzero: Pie. [OUT] # of points
    )
{
    // The drawing direction is always counterclockwise.
    int nPoints = *Points;

    // drawing a pie?
    const BOOL bPie = (nPoints != 0);

    // PI
    const double PI = 3.14159265358979;

    // delta angle
    const double dDeltaAngle = 0.01;

    // rectangle center
    const int nCX = (nLeftRect + nRightRect) / 2,
	nCY = (nTopRect + nBottomRect) / 2;

    // ellipse radius 
    const int nRX = abs(nLeftRect - nRightRect) / 2,
	nRY = abs(nTopRect - nBottomRect) / 2;

    // array of points
    LPPOINT lppt;

    // begin and end angles
    double dBeginAngle, dEndAngle, dCurAngle;

    int ni;

    if (nRX == 0 || nRY == 0)
	return NULL;

    if (nYStartArc == nCY) {
	dBeginAngle = nXStartArc < nCX ? PI : 0.0;
    } else {
	// -PI < dBeginAngle && dBeginAngle < PI
	dBeginAngle = atan2((nCY - nYStartArc) * nRX / nRY, nXStartArc - nCX);
    }

    if (nYEndArc == nCY) {
	dEndAngle = nXEndArc < nCX ? PI : 0.0;
    } else {
	// -PI < dEndAngle && dEndAngle < PI
	dEndAngle = atan2((nCY - nYEndArc) * nRX / nRY, nXEndArc - nCX);
    }

    // normalization
    if (dBeginAngle >= dEndAngle) {
	dEndAngle += PI * 2.0;
    }

    ASSERT(dBeginAngle < dEndAngle);

    // number of points
    nPoints = (int) ((dEndAngle - dBeginAngle) / dDeltaAngle) + 1;
    *Points = nPoints;

    if (nPoints < 2)
	return NULL;

    // array of points
    lppt = (LPPOINT) malloc((nPoints + (bPie ? 1 : 0)) * sizeof(POINT));

    if (lppt == NULL)
	return NULL;

    dCurAngle = dBeginAngle;
    for (ni = 0; ni < nPoints; ni++) {
	lppt[ni].x = (int) (nRX * cos(dCurAngle)) + nCX;
	lppt[ni].y = (int) (-nRY * sin(dCurAngle)) + nCY;

	dCurAngle += dDeltaAngle;
    }

    if (bPie) {
	// a pie's last point is the center.
	lppt[nPoints].x = nCX;
	lppt[nPoints].y = nCY;
	*Points = nPoints + 1;
    }

    return lppt;
}

static void ReleaseArcPoints(LPPOINT lppt)
{
    if (lppt != NULL) {
	free(lppt);
    }
}

BOOL CeArc(
    HDC hdc,         // handle to device context
    int nLeftRect,   // x-coord of upper-left corner of rectangle
    int nTopRect,    // y-coord of upper-left corner of rectangle
    int nRightRect,  // x-coord of lower-right corner of rectangle
    int nBottomRect, // y-coord of lower-right corner of rectangle
    int nXStartArc,  // x-coord of first radial's endpoint
    int nYStartArc,  // y-coord of first radial's endpoint
    int nXEndArc,    // x-coord of second radial's endpoint
    int nYEndArc     // y-coord of second radial's endpoint
    )
{
    int nPoints = 0;  // Arc
    const LPPOINT lppt = GetArcPoints(nLeftRect, nTopRect,
	    nRightRect, nBottomRect,
	    nXStartArc, nYStartArc,
	    nXEndArc, nYEndArc,
	    &nPoints);

    if (lppt != NULL) {
	//ASSERT(nPoints >= 2);

	// drawing 
	const BOOL bRet = Polyline(hdc, lppt, nPoints);

	ReleaseArcPoints(lppt);

	return bRet;
    } else {
	return FALSE;
    }
}

BOOL CeChord(
    HDC hdc,         // handle to device context
    int nLeftRect,   // x-coord of upper-left corner of rectangle
    int nTopRect,    // y-coord of upper-left corner of rectangle
    int nRightRect,  // x-coord of lower-right corner of rectangle
    int nBottomRect, // y-coord of lower-right corner of rectangle
    int nXRadial1,   // x-coord of first radial's endpoint
    int nYRadial1,   // y-coord of first radial's endpoint
    int nXRadial2,   // x-coord of second radial's endpoint
    int nYRadial2    // y-coord of second radial's endpoint
    )
{
    int nPoints = 0;  // Chord
    const LPPOINT lppt = GetArcPoints(nLeftRect, nTopRect,
	    nRightRect, nBottomRect,
	    nXRadial1, nYRadial1,
	    nXRadial2, nYRadial2,
	    &nPoints);

    if (lppt != NULL) {
	//ASSERT(nPoints >= 2);

	// drawing 
	const BOOL bRet = Polygon(hdc, lppt, nPoints);

	ReleaseArcPoints(lppt);

	return bRet;
    } else {
	return FALSE;
    }
}

BOOL CePie(
    HDC hdc,         // handle to device context
    int nLeftRect,   // x-coord of upper-left corner of rectangle
    int nTopRect,    // y-coord of upper-left corner of rectangle
    int nRightRect,  // x-coord of lower-right corner of rectangle
    int nBottomRect, // y-coord of lower-right corner of rectangle
    int nXRadial1,   // x-coord of first radial's endpoint
    int nYRadial1,   // y-coord of first radial's endpoint
    int nXRadial2,   // x-coord of second radial's endpoint
    int nYRadial2    // y-coord of second radial's endpoint
    )
{
    int nPoints = 1;  // Pie
    const LPPOINT lppt = GetArcPoints(nLeftRect, nTopRect,
	    nRightRect, nBottomRect,
	    nXRadial1, nYRadial1,
	    nXRadial2, nYRadial2,
	    &nPoints);

    if (lppt != NULL) {
	//ASSERT(nPoints >= 2);

	// drawing 
	const BOOL bRet = Polygon(hdc, lppt, nPoints);

	ReleaseArcPoints(lppt);

	return bRet;
    } else {
	return FALSE;
    }
}
