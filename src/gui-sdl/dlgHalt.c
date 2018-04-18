/*
  Hatari - dlgHalt.c - Emulation halt + reset handling alertbox

  Copyright (C) 2015 by Eero Tamminen

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.
*/
const char DlgHalt_fileid[] = "Hatari dlgHalt.c : " __DATE__ " " __TIME__;

#include <string.h>

#include "main.h"
#include "reset.h"
#include "debugui.h"
#include "dialog.h"
#include "screen.h"
#include "sdlgui.h"
#include "m68000.h"

#define DLGHALT_WARM	2
#define DLGHALT_COLD	3
#define DLGHALT_DEBUG	4
#define DLGHALT_QUIT	5

#define DLGHALT_MSG "Detected double bus/address error => CPU halted!"

/* The "Halt"-dialog: */
static SGOBJ haltdlg[] = {
	{ SGBOX,  0, 0, 0,0, 52,7, NULL },
	{ SGTEXT, 0, 0, 2,1, 48,1, DLGHALT_MSG },
	{ SGBUTTON, SG_DEFAULT, 0,  6,3, 12,1, "_Warm reset" },
	{ SGBUTTON, 0,          0,  6,5, 12,1, "_Cold reset" },
	{ SGBUTTON, 0,          0, 28,3, 18,1, "Console _debugger" },
	{ SGBUTTON, SG_CANCEL,  0, 28,5, 18,1, "_Quit Hatari" },
	{ SGSTOP, 0, 0, 0,0, 0,0, NULL }
};


/*-----------------------------------------------------------------------*/
/**
 * Make Hatari quit
 */
static void do_quit(void)
{
	if (bQuitProgram) {
		/* got here again, cold reset emulation to make sure we actually can exit */
		fputs("Halt dialog invoked during Hatari shutdown, doing emulation cold reset...\n", stderr);
		Reset_Cold();
	} else {
		bQuitProgram = true;
	}
	M68000_SetSpecial(SPCFLAG_BRK);
}

/*-----------------------------------------------------------------------*/
/**
 * Show the "halt" dialog
 */
void Dialog_HaltDlg(void)
{
	bool show = SDL_ShowCursor(SDL_QUERY);
#if WITH_SDL2
	bool mode = SDL_GetRelativeMouseMode();
	SDL_SetRelativeMouseMode(SDL_FALSE);
#endif
	SDL_ShowCursor(SDL_ENABLE);

	/* if we get halt with --run-vbls, just quit right away */
	if (Main_SetRunVBLs(0))
	{
		Log_Printf(LOG_ERROR, DLGHALT_MSG);
		do_quit();
		return;
	}

	SDLGui_CenterDlg(haltdlg);
	switch (SDLGui_DoDialog(haltdlg, NULL, false)) {

	case DLGHALT_WARM:
		/* Reset to exit 'halt' state (resets CPU and regs.spcflags) */
		Reset_Warm();
		break;
	case DLGHALT_COLD:
		/* Warm reset isn't always enough to restore emulated system to working state */
		Reset_Cold();
		break;
	case DLGHALT_DEBUG:
		/* Call the debugger, restore screen so user sees whats on it */
		SDL_UpdateRect(sdlscrn, 0,0, 0,0);
		DebugUI(REASON_CPU_EXCEPTION);
		break;
	default:
		/* DLGHALTQUIT, SDLGUI_QUIT and GUI errors */
		do_quit();
	}
	SDL_ShowCursor(show);
#if WITH_SDL2
	SDL_SetRelativeMouseMode(mode);
#endif
}
