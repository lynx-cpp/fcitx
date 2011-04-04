/***************************************************************************
 *   Copyright (C) 2010~2010 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <X11/keysym.h>
#include <libintl.h>
#include <errno.h>

#include "core/fcitx.h"
#include "configfile.h"
#include "fcitx-config/xdg.h"
#include "fcitx-config/cutils.h"
#include <core/keys.h>
static boolean IsReloadConfig = false;

static FcitxConfig fc;
ConfigFileDesc* fcitxConfigDesc = NULL;
static ConfigFileDesc* GetConfigDesc();
static void FilterSwitchKey(ConfigGroup *group, ConfigOption *option, void* value, ConfigSync sync, void* arg);
static void FilterTriggerKey(ConfigGroup *group, ConfigOption *option, void* value, ConfigSync sync, void* arg);
static void Filter2nd3rdKey(ConfigGroup *group, ConfigOption *option, void* value, ConfigSync sync, void* arg);

CONFIG_BINDING_BEGIN(FcitxConfig);
CONFIG_BINDING_REGISTER("Program", "DelayStart", iDelayStart);
CONFIG_BINDING_REGISTER("Output", "HalfPuncAfterNumber", bEngPuncAfterNumber);
CONFIG_BINDING_REGISTER("Output", "EnterAction", enterToDo);
CONFIG_BINDING_REGISTER("Output", "SemiColonAction", semicolonToDo);
CONFIG_BINDING_REGISTER("Output", "InputEngByCapitalChar", bEngAfterCap);
CONFIG_BINDING_REGISTER("Output", "ConvertPunc", bConvertPunc);
CONFIG_BINDING_REGISTER("Output", "LegendModeDisablePaging", bDisablePagingInLegend);
CONFIG_BINDING_REGISTER("Output", "SendTextWhenSwitchEng", bSendTextWhenSwitchEng);
CONFIG_BINDING_REGISTER("Output", "CandidateWordNumber", iMaxCandWord);
CONFIG_BINDING_REGISTER("Appearance", "CenterInputWindow", bCenterInputWindow);
CONFIG_BINDING_REGISTER("Appearance", "ShowInputWindowAfterTriggering", bShowInputWindowTriggering);
CONFIG_BINDING_REGISTER("Appearance", "ShowPointAfterIndex", bPointAfterNumber);
CONFIG_BINDING_REGISTER("Appearance", "ShowInputSpeed", bShowUserSpeed);
CONFIG_BINDING_REGISTER("Appearance", "ShowVersion", bShowVersion);
CONFIG_BINDING_REGISTER_WITH_FILTER("Hotkey", "TriggerKey", hkTrigger, FilterTriggerKey);
CONFIG_BINDING_REGISTER_WITH_FILTER("Hotkey", "SwitchKey", iSwitchKey, FilterSwitchKey);
CONFIG_BINDING_REGISTER("Hotkey", "DoubleSwitchKey", bDoubleSwitchKey);
CONFIG_BINDING_REGISTER("Hotkey", "TimeInterval", iTimeInterval);
CONFIG_BINDING_REGISTER("Hotkey", "FollowCursorKey", hkTrack);
CONFIG_BINDING_REGISTER("Hotkey", "HideMainWindowKey", hkHideMainWindow);
CONFIG_BINDING_REGISTER("Hotkey", "VKSwitchKey", hkVK);
CONFIG_BINDING_REGISTER("Hotkey", "LegendSwitchKey", hkLegend);
CONFIG_BINDING_REGISTER("Hotkey", "FullWidthSwitchKey", hkCorner);
CONFIG_BINDING_REGISTER("Hotkey", "PuncSwitchKey", hkPunc);
CONFIG_BINDING_REGISTER("Hotkey", "PrevPageKey", hkPrevPage);
CONFIG_BINDING_REGISTER("Hotkey", "NextPageKey", hkNextPage);
CONFIG_BINDING_REGISTER_WITH_FILTER("Hotkey", "SecondThirdCandWordKey", str2nd3rdCand, Filter2nd3rdKey);
CONFIG_BINDING_REGISTER("Hotkey", "SaveAllKey", hkSaveAll);
#ifdef _ENABLE_RECORDING
CONFIG_BINDING_REGISTER("Hotkey", "SetRecordingKey", hkRecording);
CONFIG_BINDING_REGISTER("Hotkey", "ResetRecordingKey", hkResetRecording);
#endif
CONFIG_BINDING_REGISTER("InputMethod", "PhraseTips", bPhraseTips);
CONFIG_BINDING_END()

void Filter2nd3rdKey(ConfigGroup *group, ConfigOption *option, void* value, ConfigSync sync, void* arg)
{
    char *pstr = *(char**) value;

    if(sync == Raw2Value){
        if (!strcasecmp (pstr, "SHIFT")) {
            fc.i2ndSelectKey[0].sym = fc.i2ndSelectKey[1].sym = XK_Shift_L;        //左SHIFT的扫描码
            fc.i2ndSelectKey[0].state = KEY_SHIFT_COMP ;        //左SHIFT的扫描码
            fc.i2ndSelectKey[1].state = KEY_NONE ;        //左SHIFT的扫描码
            fc.i3rdSelectKey[0].sym = fc.i3rdSelectKey[1].sym = XK_Shift_R;        //右SHIFT的扫描码
            fc.i3rdSelectKey[0].state = KEY_SHIFT_COMP ;        //左SHIFT的扫描码
            fc.i3rdSelectKey[1].state = KEY_NONE;
        }
        else if (!strcasecmp (pstr, "CTRL")) {
            fc.i2ndSelectKey[0].sym = fc.i2ndSelectKey[1].sym = XK_Control_L;        //左CTRL的扫描码
            fc.i2ndSelectKey[0].state = KEY_CTRL_COMP ;        //左SHIFT的扫描码
            fc.i2ndSelectKey[1].state = KEY_NONE ;        //左SHIFT的扫描码
            fc.i3rdSelectKey[0].sym = fc.i3rdSelectKey[1].sym = XK_Control_R;       //右CTRL的扫描码
            fc.i3rdSelectKey[0].state = KEY_CTRL_COMP ;        //左SHIFT的扫描码
            fc.i3rdSelectKey[1].state = KEY_NONE;
        }
        else {
            if (pstr[0] && pstr[0]!='0')
            {
                fc.i2ndSelectKey[0].sym = pstr[0] & 0xFF;
                fc.i2ndSelectKey[0].state = KEY_NONE;
            }
            else
            {
                fc.i2ndSelectKey[0].sym = 0;
                fc.i2ndSelectKey[0].state = KEY_NONE;
            }
            if (pstr[1] && pstr[1]!='0')
            {
                fc.i3rdSelectKey[0].sym = pstr[1] & 0xFF;
                fc.i3rdSelectKey[0].state = KEY_NONE;
            }
            else
            {
                fc.i3rdSelectKey[0].sym = 0;
                fc.i3rdSelectKey[0].state = KEY_NONE;
            }
        }
    }
}

void FilterTriggerKey(ConfigGroup *group, ConfigOption *option, void* value, ConfigSync sync, void* arg)
{
    HOTKEYS *hotkey = (HOTKEYS*)value;
    if (sync == Raw2Value)
    {
        if (hotkey[0].desc == NULL && hotkey[1].desc == NULL)
        {
            hotkey[0].desc = strdup("CTRL_SPACE");
            hotkey[0].sym = XK_space;
            hotkey[0].state = KEY_CTRL_COMP;
        }
        char *strkey[2];
        int i = 0;
        for (;i<2;i++)
        {
            if (hotkey[i].desc == NULL)
                break;
            strkey[i] = hotkey[i].desc;
        }
    }
}

void FilterSwitchKey(ConfigGroup *group, ConfigOption *option, void* value, ConfigSync sync, void* arg)
{
    HOTKEYS* hkey = NULL;
    if (sync == Raw2Value)
    {
        SWITCHKEY *s = (SWITCHKEY*)value;
        switch(*s)
        {
            case S_R_CTRL:
                hkey = FCITX_RCTRL;
                break;
            case S_R_SHIFT:
                hkey = FCITX_RSHIFT;
                break;
            case S_L_SHIFT:
                hkey = FCITX_LSHIFT;
                break;
            case S_L_CTRL:
                hkey = FCITX_LCTRL;
                break;
        }
    }
    if (hkey != NULL)
    {
        fc.switchKey[0] = hkey[0];
        fc.switchKey[1] = hkey[1];
    }
}

void LoadConfig()
{
    FILE *fp;
    char *file;
    fp = GetXDGFileUser( "config", "rt", &file);
    FcitxLog(INFO, _("Load Config File %s"), file);
    free(file);
    if (!fp) {
        if (errno == ENOENT)
        {
            SaveConfig();
            LoadConfig();
        }
        return;
    }
    
    ConfigFileDesc* configDesc = GetConfigDesc();
    ConfigFile *cfile = ParseConfigFileFp(fp, configDesc);
    
    FcitxConfigConfigBind(&fc, cfile, configDesc);
    ConfigBindSync((GenericConfig*)&fc);

    IsReloadConfig = True;
    
    fclose(fp);
/*#ifndef _ENABLE_PANGO
    CreateFont();
#endif*/
}

ConfigFileDesc* GetConfigDesc()
{
    if (!fcitxConfigDesc)
    {
        FILE *tmpfp;
        tmpfp = GetXDGFileData("config.desc", "r", NULL);
        fcitxConfigDesc = ParseConfigFileDescFp(tmpfp);
		fclose(tmpfp);
    }

    return fcitxConfigDesc;
}

void SaveConfig()
{
    ConfigFileDesc* configDesc = GetConfigDesc();
    char *file;
    FILE *fp = GetXDGFileUser("config", "wt", &file);
    FcitxLog(INFO, "Save Config to %s", file);
    SaveConfigFileFp(fp, fc.gconfig.configFile, configDesc);
    free(file);
    fclose(fp);
}

GenericConfig* GetConfig()
{
    return &fc.gconfig;
}


int ConfigGetMaxCandWord()
{
    return fc.iMaxCandWord;
}

boolean ConfigGetPointAfterNumber()
{
    return fc.bPointAfterNumber;
}

boolean IsCenterInputWindow()
{
    return fc.bCenterInputWindow;
}