/*
 * tkWinCEMenu.h --
 *
 *	This file contains the menu emulation headers for Tk on Windows CE.
 *	It is adapted with minor modifications from Rainer Keuchel's
 *	celib-based port.
 *
 * Copyright (c) 2003 ActiveState Corporation
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: Exp $
 */

#ifndef WCEMENU_H
#define WCEMENU_H 1

#include <shellapi.h>

#define SHCMBF_EMPTYBAR      0x0001
#define SHCMBF_HIDDEN        0x0002
#define SHCMBF_HIDESIPBUTTON 0x0004

typedef struct tagSHMENUBARINFO
{
    DWORD cbSize; 
    HWND hwndParent;
    DWORD dwFlags;  
    UINT nToolBarId;
    HINSTANCE hInstRes;
    int nBmpId;
    int cBmpImages;
    HWND hwndMB;
} SHMENUBARINFO, *PSHMENUBARINFO;

WINSHELLAPI BOOL SHCreateMenuBar(SHMENUBARINFO *pmbi);

#define SHCMBM_SETSUBMENU   (WM_USER + 400)
#define SHCMBM_GETSUBMENU   (WM_USER + 401)
#define SHCMBM_GETMENU      (WM_USER + 402)

#define SH_GetMenu(hWndMB) \
 (HMENU)SendMessageW((hWndMB), SHCMBM_GETMENU, (WPARAM)0, (LPARAM)0);
#define SH_GetSubMenu(hWndMB,ID_MENU) \
 (HMENU)SendMessageW((hWndMB), SHCMBM_GETSUBMENU, (WPARAM)0, (LPARAM)ID_MENU);
#define SH_SetSubMenu(hWndMB,ID_MENU, HM) \
 (HMENU)SendMessageW((hWndMB), SHCMBM_SETSUBMENU, (WPARAM)ID_MENU, (LPARAM)HM);

#define Menu_GetMenu(hWndCB) \
 (HMENU)CommandBar_GetMenu(hWndCB, 0)
#define Menu_GetSubMenu(hWndCB,ID_MENU) \
 (HMENU)GetSubMenu((HMENU)CommandBar_GetMenu(hWndCB, 0), ID_MENU)

typedef struct
{
    // in
    HWND hwndParent;
    int type;
    HINSTANCE hInst;
    WORD resid;
    // out
    HMENU hMenu;
    HWND hwndBar;
    HMENU hMenuOrig;
} WCE_MENU_INFO;

// loads aygshell.dll if there
BOOL WCEInitMenu();

// needs a dummy menu from a resource... pinfo must static/malloc'ed!
HMENU WCECreateMenu(WCE_MENU_INFO *pInfo);

// emulation calls
HMENU WCEGetMenu(HWND hwnd);
BOOL WCESetMenu(HWND hwnd, HMENU hMenu);
int WCEGetMenuItemCount(HMENU hMenu);
BOOL WCEDrawMenuBar(HWND hwnd);

// misc
BOOL WCEIsMenuAYG();
int WCEGetMenuHeight(HWND hwnd);
HWND WCEGetMenuWindow(HWND hwnd);
BOOL WCECopyMenu(HMENU src, HMENU dest);

#endif
