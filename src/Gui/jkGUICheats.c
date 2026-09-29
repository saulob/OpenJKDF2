#include "jkGUICheats.h"

#include "General/stdBitmap.h"
#include "General/stdFont.h"
#include "Engine/rdMaterial.h" // TODO move tVBuffer
#include "globals.h"
#include "stdPlatform.h"
#include "jk.h"
#include "Gui/jkGUIRend.h"
#include "Gui/jkGUI.h"
#include "Main/jkDev.h"
#include "Main/sithCommand.h"
#include "types_enums.h"

#ifdef QOL_IMPROVEMENTS

enum jkGuiCheatsButton_t
{
    JKGUICHEATS_BACK           = 1,
    JKGUICHEATS_HEAL           = 10,
    JKGUICHEATS_ALLWEAPONS     = 11,
    JKGUICHEATS_ALLITEMS       = 12,
    JKGUICHEATS_REFILLFORCE    = 13,
    JKGUICHEATS_ALLFORCEPOWERS = 14,
};

enum jkGuiCheatsElement_t
{
    JKGUICHEATS_ELMT_TITLE          = 0,
    JKGUICHEATS_ELMT_PLAYER         = 1,
    JKGUICHEATS_ELMT_GODMODE        = 2,
    JKGUICHEATS_ELMT_NOCLIP         = 3,
    JKGUICHEATS_ELMT_FLY            = 4,
    JKGUICHEATS_ELMT_WORLD          = 5,
    JKGUICHEATS_ELMT_DISABLEAI      = 6,
    JKGUICHEATS_ELMT_SLOWMO         = 7,
    JKGUICHEATS_ELMT_REVEALMAP      = 8,
    JKGUICHEATS_ELMT_ACTIONS        = 9,
    JKGUICHEATS_ELMT_HEAL           = 10,
    JKGUICHEATS_ELMT_ALLWEAPONS     = 11,
    JKGUICHEATS_ELMT_ALLITEMS       = 12,
    JKGUICHEATS_ELMT_REFILLFORCE    = 13,
    JKGUICHEATS_ELMT_ALLFORCEPOWERS = 14,
    JKGUICHEATS_ELMT_BACK           = 15,
};

#define JKGUICHEATS_MAPMODE_REVEAL (SITHMAPMODE_SHOWALLTHINGS | SITHMAPMODE_SHOWALLSECTORS)

// Same cheat-table entries jkDev_Startup registers for jediwannabe/boinga, whiteflag/statuesque and slowmo/gospeedgo:
// sithCommand_DebugMode picks the flag from `extra` and prints `cmdStr` in its "%s_IS_%s" feedback.
static stdDebugConsoleCmd jkGuiCheats_cmdInvul     = {"Invul",      jkDev_CmdDebugFlags2, 5};
static stdDebugConsoleCmd jkGuiCheats_cmdDisableAi = {"Disable AI", jkDev_CmdDebugFlags,  0};
static stdDebugConsoleCmd jkGuiCheats_cmdSlowmo    = {"Slowmo",     jkDev_CmdDebugFlags3, 7};

static int jkGuiCheats_ToggleClick(jkGuiElement *pElement, jkGuiMenu *pMenu, int32_t mouseX, int32_t mouseY, BOOL bRedraw);

static jkGuiElement jkGuiCheats_aElements[17] = {
    { ELEMENT_TEXT,       0,                           6, "GUIEXT_CHEATS",                  3, { 20,  20, 600, 40}, 1, 0, NULL, NULL, NULL,                    NULL, {0}, 0},

    { ELEMENT_TEXT,       0,                           2, "GUIEXT_CHEATS_PLAYER",           2, { 60,  90, 240, 30}, 1, 0, NULL, NULL, NULL,                    NULL, {0}, 0},
    { ELEMENT_CHECKBOX,   0,                           0, "GUIEXT_CHEATS_GOD_MODE",         0, { 70, 125, 230, 20}, 1, 0, NULL, NULL, jkGuiCheats_ToggleClick, NULL, {0}, 0},
    { ELEMENT_CHECKBOX,   0,                           0, "GUIEXT_CHEATS_NOCLIP",           0, { 70, 150, 230, 20}, 1, 0, NULL, NULL, jkGuiCheats_ToggleClick, NULL, {0}, 0},
    { ELEMENT_CHECKBOX,   0,                           0, "GUIEXT_CHEATS_FLY",              0, { 70, 175, 230, 20}, 1, 0, NULL, NULL, jkGuiCheats_ToggleClick, NULL, {0}, 0},

    { ELEMENT_TEXT,       0,                           2, "GUIEXT_CHEATS_WORLD",            2, { 60, 215, 240, 30}, 1, 0, NULL, NULL, NULL,                    NULL, {0}, 0},
    { ELEMENT_CHECKBOX,   0,                           0, "GUIEXT_CHEATS_DISABLE_AI",       0, { 70, 250, 230, 20}, 1, 0, NULL, NULL, jkGuiCheats_ToggleClick, NULL, {0}, 0},
    { ELEMENT_CHECKBOX,   0,                           0, "GUIEXT_CHEATS_SLOW_MOTION",      0, { 70, 275, 230, 20}, 1, 0, NULL, NULL, jkGuiCheats_ToggleClick, NULL, {0}, 0},
    { ELEMENT_CHECKBOX,   0,                           0, "GUIEXT_CHEATS_REVEAL_MAP",       0, { 70, 300, 230, 20}, 1, 0, NULL, NULL, jkGuiCheats_ToggleClick, NULL, {0}, 0},

    { ELEMENT_TEXT,       0,                           2, "GUIEXT_CHEATS_ACTIONS",          2, {340,  90, 260, 30}, 1, 0, NULL, NULL, NULL,                    NULL, {0}, 0},
    { ELEMENT_TEXTBUTTON, JKGUICHEATS_HEAL,            2, "GUIEXT_CHEATS_HEAL",             2, {350, 120, 250, 30}, 1, 0, NULL, NULL, NULL,                    NULL, {0}, 0},
    { ELEMENT_TEXTBUTTON, JKGUICHEATS_ALLWEAPONS,      2, "GUIEXT_CHEATS_ALL_WEAPONS",      2, {350, 155, 250, 30}, 1, 0, NULL, NULL, NULL,                    NULL, {0}, 0},
    { ELEMENT_TEXTBUTTON, JKGUICHEATS_ALLITEMS,        2, "GUIEXT_CHEATS_ALL_ITEMS",        2, {350, 190, 250, 30}, 1, 0, NULL, NULL, NULL,                    NULL, {0}, 0},
    { ELEMENT_TEXTBUTTON, JKGUICHEATS_REFILLFORCE,     2, "GUIEXT_CHEATS_REFILL_FORCE",     2, {350, 225, 250, 30}, 1, 0, NULL, NULL, NULL,                    NULL, {0}, 0},
    { ELEMENT_TEXTBUTTON, JKGUICHEATS_ALLFORCEPOWERS,  2, "GUIEXT_CHEATS_ALL_FORCE_POWERS", 2, {350, 260, 250, 30}, 1, 0, NULL, NULL, NULL,                    NULL, {0}, 0},

    { ELEMENT_TEXTBUTTON, JKGUICHEATS_BACK,            2, "GUIEXT_CHEATS_BACK",             3, {220, 430, 200, 40}, 1, 0, NULL, NULL, NULL,                    NULL, {0}, 0},
    { ELEMENT_END,        0,                           0, NULL,                             0, {0},                 0, 0, NULL, NULL, NULL,                    NULL, {0}, 0},
};

static jkGuiMenu jkGuiCheats_menu = { jkGuiCheats_aElements, -1, 0xFF, 0xE1, JKGUI_BM_CHECK, 0, 0, jkGui_stdBitmaps, jkGui_stdFonts, 0, 0, "thermloop01.wav", "thrmlpu2.wav", 0, 0, 0, 0, 0, 0 };

// Single-player only, and the existing cheats dereference the local player without null checks
static int jkGuiCheats_IsAvailable()
{
    return !sithNet_isMulti && sithWorld_g_pCurrentWorld && sithPlayer_g_pLocalPlayerThing;
}

// The checkboxes never hold their own state: they are always re-read from the engine
static void jkGuiCheats_RefreshToggles(jkGuiMenu *pMenu)
{
    SithThing *pPlayer = sithPlayer_g_pLocalPlayerThing;

    jkGuiCheats_aElements[JKGUICHEATS_ELMT_GODMODE].selectedTextEntry   = !!(pPlayer->actorParams.flags & SITH_AF_INVULNERABLE);
    jkGuiCheats_aElements[JKGUICHEATS_ELMT_NOCLIP].selectedTextEntry    = !!(g_debugmodeFlags & DEBUGFLAG_NOCLIP);
    jkGuiCheats_aElements[JKGUICHEATS_ELMT_FLY].selectedTextEntry       = !!(pPlayer->physicsParams.flags & SITH_PF_FLY);
    jkGuiCheats_aElements[JKGUICHEATS_ELMT_DISABLEAI].selectedTextEntry = !!(g_debugmodeFlags & DEBUGFLAG_NO_AIEVENTS);
    jkGuiCheats_aElements[JKGUICHEATS_ELMT_SLOWMO].selectedTextEntry    = !!(g_debugmodeFlags & DEBUGFLAG_SLOWMO);
    jkGuiCheats_aElements[JKGUICHEATS_ELMT_REVEALMAP].selectedTextEntry = (g_mapModeFlags & JKGUICHEATS_MAPMODE_REVEAL) == JKGUICHEATS_MAPMODE_REVEAL;

    if (!pMenu)
        return;

    for (int i = JKGUICHEATS_ELMT_GODMODE; i <= JKGUICHEATS_ELMT_REVEALMAP; i++)
    {
        if (jkGuiCheats_aElements[i].type == ELEMENT_CHECKBOX)
            jkGuiRend_UpdateAndDrawClickable(&jkGuiCheats_aElements[i], pMenu, 1);
    }
}

static void jkGuiCheats_SetDebugMode(stdDebugConsoleCmd *pCmd, int bEnable)
{
    pCmd->cmdFunc(pCmd, bEnable ? "on" : "off");
}

static int jkGuiCheats_ToggleClick(jkGuiElement *pElement, jkGuiMenu *pMenu, int32_t mouseX, int32_t mouseY, BOOL bRedraw)
{
    if (!jkGuiCheats_IsAvailable())
        return 0;

    // Flip whatever the engine currently has, then show the result
    jkGuiCheats_RefreshToggles(NULL);
    int bEnable = !pElement->selectedTextEntry;

    switch (pElement - jkGuiCheats_aElements)
    {
        case JKGUICHEATS_ELMT_GODMODE:
            jkGuiCheats_SetDebugMode(&jkGuiCheats_cmdInvul, bEnable);
            break;

        case JKGUICHEATS_ELMT_NOCLIP:
            jkDev_CmdNoclip(NULL, NULL);
            break;

        case JKGUICHEATS_ELMT_FLY:
            sithCommand_Fly(NULL, NULL);
            break;

        case JKGUICHEATS_ELMT_DISABLEAI:
            jkGuiCheats_SetDebugMode(&jkGuiCheats_cmdDisableAi, bEnable);
            break;

        case JKGUICHEATS_ELMT_SLOWMO:
            jkGuiCheats_SetDebugMode(&jkGuiCheats_cmdSlowmo, bEnable);
            break;

        case JKGUICHEATS_ELMT_REVEALMAP:
            // jkDev_CmdAllMap XORs both bits, so a half-set state (ie from a COG) would never become fully revealed
            if (bEnable)
                g_mapModeFlags &= ~JKGUICHEATS_MAPMODE_REVEAL;
            jkDev_CmdAllMap(NULL, NULL);
            break;

        default:
            break;
    }

    jkGuiCheats_RefreshToggles(pMenu);
    return 0;
}

void jkGuiCheats_Startup()
{
    jkGui_InitMenu(&jkGuiCheats_menu, jkGui_stdBitmaps[JKGUI_BM_BK_SETUP]);
}

void jkGuiCheats_Shutdown()
{
    stdPlatform_Printf("OpenJKDF2: %s\n", __func__);
}

void jkGuiCheats_Show()
{
    int32_t clicked;

    if (!jkGuiCheats_IsAvailable())
        return;

    while (1)
    {
        jkGuiCheats_RefreshToggles(NULL);
        jkGuiRend_MenuSetEscapeKeyShortcutElement(&jkGuiCheats_menu, &jkGuiCheats_aElements[JKGUICHEATS_ELMT_BACK]);

        clicked = jkGuiRend_DisplayAndReturnClicked(&jkGuiCheats_menu);
        if (clicked == JKGUICHEATS_BACK || clicked == -1 || !jkGuiCheats_IsAvailable())
            return;

        switch (clicked)
        {
            case JKGUICHEATS_HEAL:
                jkDev_CmdHeal(NULL, NULL);
                break;

            case JKGUICHEATS_ALLWEAPONS:
                jkDev_CmdAllWeapons(NULL, NULL);
                break;

            case JKGUICHEATS_ALLITEMS:
                jkDev_CmdAllItems(NULL, NULL);
                break;

            case JKGUICHEATS_REFILLFORCE:
                jkDev_CmdMana(NULL, NULL);
                break;

            case JKGUICHEATS_ALLFORCEPOWERS:
                jkDev_CmdUberJedi(NULL, NULL);
                break;

            default:
                break;
        }
    }
}

#endif // QOL_IMPROVEMENTS
