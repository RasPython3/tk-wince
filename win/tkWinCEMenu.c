/*
 * tkWinCEMenu.c --
 *
 *	This file contains the menu emulation code for Tk on Windows CE.
 *	It is adapted with minor modifications from Rainer Keuchel's
 *	celib-based port.
 *
 * Dynamic menues in WinCE are a pain!
 *
 * Seems that toplevel SHMenu-Texts cannot be dynamically changed...
 * And the menu cannot be dynamically expanded. And there were other
 * problems...
 *
 * We work-around by using only one top-level popup, which is returned
 * as the menu handle. It's not very nice, but makes life easier.
 *
 * How to use:
 * Call CreateMenu() to create a menu
 * Call WCECreateMenu() to create the menubar.
 * Call WCESetMenu to copy items from the first menu to the menubar
 * Call WCEDrawMenuBar to copy items from the first menu to the menubar
 * after changes.
 *
 * Be careful when calling DestroyMenu().
 *
 * Copyright (c) 2003 ActiveState Corporation
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: Exp $
 */

#include <windows.h>
#include "celib.h"
#include "tkWinCEMenu.h"
#include <commctrl.h>

#define WCE_MAXMENUES 10

WCE_MENU_INFO *menu_tab[WCE_MAXMENUES];

static BOOL has_ayg = FALSE;
static FARPROC pfn_SHCreateMenuBar = NULL;

BOOL 
WCEInitMenu()
{
    static HMODULE hMod = NULL;

    if (hMod == NULL) {
	hMod = LoadLibraryW(L"aygshell.dll");
	if (hMod) {
	    pfn_SHCreateMenuBar = GetProcAddressW(hMod, L"SHCreateMenuBar");
	    if (pfn_SHCreateMenuBar) {
		has_ayg = TRUE;
	    }
	}
    }

    return (hMod != NULL);
}

BOOL 
WCEIsMenuAYG()
{
    return has_ayg;
}

BOOL
WCEAddMenuInfo(WCE_MENU_INFO *p)
{
    int i;

    for (i = 0; i < WCE_MAXMENUES; i++) {
	if (menu_tab[i] == NULL) {
	    menu_tab[i] = p;
	    return TRUE;
	}
    }
    return FALSE;
}

WCE_MENU_INFO *
WCEFindMenuInfoByHwnd(HWND hwnd)
{
    int i;

    for (i = 0; i < WCE_MAXMENUES, menu_tab[i] != NULL; i++) {
	if (menu_tab[i]->hwndParent == hwnd) {
	    return menu_tab[i];
	}
    }
    return NULL;
}

WCE_MENU_INFO *
WCEFindMenuInfoByHmenu(HMENU hMenu)
{
    int i;

    for (i = 0; i < WCE_MAXMENUES, menu_tab[i] != NULL; i++) {
	if (menu_tab[i]->hMenu == hMenu) {
	    return menu_tab[i];
	}
    }
    return NULL;
}

HMENU
WCECreateMenu(WCE_MENU_INFO *pInfo)
{
    HMENU hMenu;

    XCETrace("MENU: WCECreateMenu");

    if (has_ayg) {
	SHMENUBARINFO mbi;

	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize     = sizeof(SHMENUBARINFO);
	mbi.dwFlags    = 0;
	mbi.hwndParent = pInfo->hwndParent;
	mbi.nToolBarId = pInfo->resid;
	mbi.hInstRes   = pInfo->hInst;
	mbi.nBmpId     = 0;
	mbi.cBmpImages = 0;	

	if ((*pfn_SHCreateMenuBar)(&mbi) == FALSE)
	    return NULL;

	if (mbi.hwndMB == NULL)
	    return NULL;

	pInfo->hwndBar = mbi.hwndMB;

	hMenu = SH_GetMenu(pInfo->hwndBar);
	hMenu = CreatePopupMenu();
	SH_SetSubMenu(pInfo->hwndBar, 0, hMenu);
	pInfo->hMenu = SH_GetSubMenu(pInfo->hwndBar, 0);
    } else {
	pInfo->hwndBar = CommandBar_Create(pInfo->hInst, pInfo->hwndParent, 1);
	CommandBar_InsertMenubar(pInfo->hwndBar, pInfo->hInst, pInfo->resid, 0);
	// Button does not resize!
	//CommandBar_AddAdornments(pInfo->hwndBar, 0, 0);

	pInfo->hMenu = Menu_GetMenu(pInfo->hwndBar);
    }

    // remove dummy entry
    DeleteMenu(pInfo->hMenu, 0, MF_BYPOSITION);

    WCEAddMenuInfo(pInfo);

    return pInfo->hMenu;
}

// Emulates SetMenu

BOOL
WCESetMenu(HWND hwnd, HMENU hMenu)
{
    WCE_MENU_INFO *pInfo;

    XCETrace("MENU: WCESetMenu");

    if ((pInfo = WCEFindMenuInfoByHwnd(hwnd)) == NULL) {
	return FALSE;
    }

    pInfo->hMenuOrig = hMenu;

    WCECopyMenu(pInfo->hMenuOrig, pInfo->hMenu);

    return TRUE;
}

// Emulate GetMenu()

HMENU
WCEGetMenu(HWND hwnd)
{
    WCE_MENU_INFO *pInfo;

    XCETrace("MENU: WCEGetMenu");

    if ((pInfo = WCEFindMenuInfoByHwnd(hwnd)) == NULL)
	return NULL;

    return pInfo->hMenuOrig;
}

HMENU
WCEGetRealMenu(HWND hwnd)
{
    WCE_MENU_INFO *pInfo;

    XCETrace("MENU: WCEGetMenu");

    if ((pInfo = WCEFindMenuInfoByHwnd(hwnd)) == NULL)
	return NULL;

    return pInfo->hMenu;
}

int
WCEGetMenuHeight(HWND hwnd)
{
    WCE_MENU_INFO *pInfo;
    RECT r;

    if ((pInfo = WCEFindMenuInfoByHwnd(hwnd)) == NULL)
	return -1;

    GetClientRect(pInfo->hwndBar, &r);

    return r.bottom;
}

HWND
WCEGetMenuWindow(HWND hwnd)
{
    WCE_MENU_INFO *pInfo;

    if ((pInfo = WCEFindMenuInfoByHwnd(hwnd)) == NULL)
	return NULL;

    return pInfo->hwndBar;
}

int
WCEGetMenuItemCount(HMENU hMenu)
{
    int i;
    MENUITEMINFOW mi;
    WCHAR text[126];

    XCETrace("MENU: GetMenuItemCount(%x)", hMenu);

    for (i = 0;; i++) {
	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_DATA|MIIM_TYPE;
	mi.dwTypeData = (WCHAR*) text;
	mi.cch = sizeof(text)/2;

	text[0] = 0;

	SetLastError(0);

	if (!GetMenuItemInfoW(hMenu, i, TRUE, &mi)) {
	    XCETrace("MENU: GetMenuItemInfo(%d): %d", i, GetLastError());
	    break;
	} else {
	    XCETrace("MENU: Menu %x, item %d: Data %ls Type %x", 
		    hMenu, i, text, mi.fType);
	}
    }

    return i;
}

BOOL
WCEDrawMenuBar(HWND hwnd)
{
    WCE_MENU_INFO *pInfo;
    RECT r;

    XCETrace("MENU: WCEDrawMenuBar");

    if ((pInfo = WCEFindMenuInfoByHwnd(hwnd)) == NULL) {
	return FALSE;
    }

    if (pInfo->hMenuOrig == NULL) {
	XCETrace("MENU: No original menu");
	return TRUE;
    }

    WCECopyMenu(pInfo->hMenuOrig, pInfo->hMenu);

    // Force an update. Else the menu might not popup...
    // How can this be done better?
    GetWindowRect(pInfo->hwndBar, &r);

    // This generates a WM_NOTIFY with NM_CUSTOMDRAW (-12)
    // Seems this is needed to update the menubar...
    MoveWindow(pInfo->hwndBar, r.left+1, r.top+1, r.right - r.left, 
	    r.bottom - r.top, TRUE);
    MoveWindow(pInfo->hwndBar, r.left, r.top, r.right - r.left, 
	    r.bottom - r.top, TRUE);

    return TRUE;
}

BOOL
WCECopyMenu(HMENU hsrc, HMENU hdest)
{
    int i = 0;
    int n;
    HMENU hMenuTmp;

    n = WCEGetMenuItemCount(hdest);

    XCETrace("MENU: Removing %d items in dest menu %x", n, hdest);

    for (i = 0; i < n; i++) {
	// delete destroys the menu, but we need it...
	//DeleteMenu(hdest, 0, MF_BYPOSITION);
	RemoveMenu(hdest, 0, MF_BYPOSITION);
    }

    n = WCEGetMenuItemCount(hsrc);

    XCETrace("MENU: Copying %d items from menu %x to menu %x", n, hsrc, hdest);

    for (i = 0; i < n; i++) {
	SetLastError(0);

	if ((hMenuTmp = GetSubMenu(hsrc, i)) != NULL) {
	    MENUITEMINFOW mi;
	    WCHAR text[126];

	    memset(&mi, 0, sizeof(mi));
	    mi.cbSize = sizeof(mi);
	    mi.fMask = MIIM_DATA|MIIM_TYPE;
	    mi.dwTypeData = (WCHAR*) text;
	    mi.cch = sizeof(text)/2;

	    SetLastError(0);

	    if (!GetMenuItemInfoW(hsrc, i, TRUE, &mi)) {
		XCETrace("MENU: GetMenuItemInfo(%x, %d): Error %d", 
			hsrc, i, GetLastError());
	    } else {
		int nsub = WCEGetMenuItemCount(hMenuTmp);

		XCETrace("MENU: Appending popup %x (%ls) with %d entries to %x", 
			hMenuTmp, text, nsub, hdest);
	      
		AppendMenuW(hdest, MF_POPUP, (UINT) hMenuTmp, text);
	    }
	} else {
	    XCETrace("MENU: GetSubMenu(%x, %d): Error %d", 
		    hsrc, i, GetLastError());
	    return FALSE;
	}
    }

    return TRUE;
}
