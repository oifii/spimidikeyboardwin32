/*
 * Copyright (c) 2015-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "spimidikeyboardwin32.h"
#include "FreeImage.h"
#include <shellapi.h> //for CommandLineToArgW()
#include <mmsystem.h> //for timeSetEvent()
#include <stdio.h> //for swprintf()
#include <assert.h>
#include "spiwavsetlib.h"

#include "porttime.h"
#include "portmidi.h"
#include <map>
#include <list>

#include "spiutility.h"
#include "spimidiutility.h"

// Global Variables:

CHAR pCHAR[1024];
WCHAR pWCHAR[1024];

//UINT global_TimerId=0;
UINT global_nIDEvent=1;
PmStream* global_pPmStreamMIDIOUT = NULL; // midi output
PmStream* global_pPmStreamMIDIIN;      // midi input 
bool global_active = false;     // set when global_pPmStreamMIDIIN is ready for reading
bool global_inited = false;     // suppress printing during command line parsing 
//bool global_playflag = true;
bool global_midiready = false;

int global_inputmidideviceid =  11; //alesis q49 midi port id (when midi yoke installed)
//int inputmididevice =  1; //midi yoke 1 (when midi yoke installed)
int global_outputmidideviceid = 13; //device id 13, for "Out To MIDI Yoke:  1", when 8 yoke installed for spi
int global_inputmidichannel = -1; //-1 for all 16 channels otherwise from 0 to 15
int global_outputmidichannel = -1; //-1, will be set in the code
int global_outputmidichannel1 = 0; //0 for first channel
int global_outputmidichannel2 = 0; //0 for first channel
int global_outputmidichannel3 = 0; //0 for first channel
int global_outputmidichannel4 = 0; //0 for first channel
//float global_tempo_bpm = 60.0f;


map<string,int> global_inputmididevicemap;
map<string,int> global_outputmididevicemap;
string global_inputmididevicename = "Q49";
string global_outputmididevicename = "Out To MIDI Yoke:  1";


#define MAX_LOADSTRING 100
FIBITMAP* global_dib;
HFONT global_hFont;
HWND global_hwnd=NULL;
MMRESULT global_timer=0;
#define MAX_GLOBALTEXT	4096
WCHAR global_text[MAX_GLOBALTEXT+1];
int global_x=100;
int global_y=200;
int global_xwidth=400;
int global_yheight=400;
BYTE global_alpha=200;
int global_fontheight=24;
int global_fontwidth=-1; //will be computed within WM_PAINT handler
BYTE global_fontcolor_r=255;
BYTE global_fontcolor_g=255;
BYTE global_fontcolor_b=255;
int global_staticalignment = 0; //0 for left, 1 for center and 2 for right
int global_staticheight=-1; //will be computed within WM_SIZE handler
int global_staticwidth=-1; //will be computed within WM_SIZE handler 
//spi, begin
int global_imageheight=-1; //will be computed within WM_SIZE handler
int global_imagewidth=-1; //will be computed within WM_SIZE handler 
//spi, end
int global_titlebardisplay=1; //0 for off, 1 for on
int global_acceleratoractive=1; //0 for off, 1 for on
int global_menubardisplay=0; //0 for off, 1 for on
FILE* global_pfile=NULL;
#define IDC_MAIN_EDIT	100
#define IDC_MAIN_STATIC	101

HINSTANCE global_hInstance;								// current instance
//TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
//TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szTitle[1024]={L"spimidikeyboardwin32title"};					// The title bar text
TCHAR szWindowClass[1024]={L"spimidikeyboardwin32class"};				// the main window class name

//new parameters
string global_begin="begin.ahk";
string global_end="end.ahk";

bool global_visibility = true;

//#define StatusAddText StatusAddTextW

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


CSpiMidiUtility mySpiMidiUtility;

void receive_poll(PtTimestamp timestamp, void *userData)
{
    PmEvent event;
    int count; 
    if (!global_active) return;
    while ((count = Pm_Read(global_pPmStreamMIDIIN, &event, 1))) 
	{
        if (count == 1) 
		{
			//1) output message
			mySpiMidiUtility.output(event.message);
			
		}
        else            
		{
			sprintf(pCHAR, "%s\n", Pm_GetErrorText((PmError)count));StatusAddTextA(pCHAR); //spi a cast as (PmError)
		}
    }
}

HHOOK hKeyboardHook;
/*
bool alreadydown_AtoZ = false;
*/
bool alreadydown_A = false;
bool alreadydown_B = false;
bool alreadydown_C = false;
bool alreadydown_D = false;
bool alreadydown_E = false;
bool alreadydown_F = false;
bool alreadydown_G = false;
bool alreadydown_H = false;
bool alreadydown_I = false;
bool alreadydown_J = false;
bool alreadydown_K = false;
bool alreadydown_L = false;
bool alreadydown_M = false;
bool alreadydown_N = false;
bool alreadydown_O = false;
bool alreadydown_P = false;
bool alreadydown_Q = false;
bool alreadydown_R = false;
bool alreadydown_S = false;
bool alreadydown_T = false;
bool alreadydown_U = false;
bool alreadydown_V = false;
bool alreadydown_W = false;
bool alreadydown_X = false;
bool alreadydown_Y = false;
bool alreadydown_Z = false;

/*
bool alreadydown_0to9 = false;
*/
bool alreadydown_0 = false;
bool alreadydown_1 = false;
bool alreadydown_2 = false;
bool alreadydown_3 = false;
bool alreadydown_4 = false;
bool alreadydown_5 = false;
bool alreadydown_6 = false;
bool alreadydown_7 = false;
bool alreadydown_8 = false;
bool alreadydown_9 = false;

bool alreadydown_lshift = false;
bool alreadydown_capslock = false;
bool alreadydown_tab = false;
bool alreadydown_ascii96 = false; //`
bool alreadydown_backspace = false;
bool alreadydown_minus = false;
bool alreadydown_equals = false;
bool alreadydown_leftbrace = false;
bool alreadydown_rightbrace = false;
bool alreadydown_backslash = false;
bool alreadydown_semicolon = false;
bool alreadydown_apostrophe = false;
bool alreadydown_return = false;
bool alreadydown_comma = false;
bool alreadydown_period = false;
bool alreadydown_slash = false;
bool alreadydown_rshift = false;
bool alreadydown_uparrow = false;
bool alreadydown_downarrow = false;
bool alreadydown_leftarrow = false;
bool alreadydown_rightarrow = false;

PmEvent global_PmEvent;
int global_notenumber=0;
int global_velocity=127;
int local_notenumber = 0; //will be set for each key
int local_octaveshift = 0; //will be set for each key
int global_octaveshift = 3; //integer between 0 and 9, will be set with arrow up or down

void noteon()
{
	global_notenumber = global_octaveshift*12 + local_octaveshift*12 + local_notenumber;
	if(global_notenumber>=0 && global_notenumber<128)
	{
		global_PmEvent.timestamp = 0;
		global_PmEvent.message = Pm_Message(MIDI_ON_NOTE+global_outputmidichannel, global_notenumber, global_velocity); 
		Pm_Write(global_pPmStreamMIDIOUT, &global_PmEvent, 1);
	}
}

void noteoff()
{
	global_notenumber = global_octaveshift*12 + local_octaveshift*12 + local_notenumber;
	if(global_notenumber>=0 && global_notenumber<128)
	{
		global_PmEvent.timestamp = 0;
		global_PmEvent.message = Pm_Message(MIDI_OFF_NOTE+global_outputmidichannel, global_notenumber, 0); 
		Pm_Write(global_pPmStreamMIDIOUT, &global_PmEvent, 1);
	}
}

//__declspec(dllexport) LRESULT CALLBACK KeyboardEvent (int nCode, WPARAM wParam, LPARAM lParam)
LRESULT CALLBACK KeyboardEvent (int nCode, WPARAM wParam, LPARAM lParam)
{
	if(!global_midiready) return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);

    DWORD SHIFT_key=0;
    DWORD CTRL_key=0;
    DWORD ALT_key=0;
    DWORD LSHIFT_key=0;
    DWORD LCTRL_key=0;
    DWORD LALT_key=0;
    DWORD RSHIFT_key=0;
    DWORD RCTRL_key=0;
    DWORD RALT_key=0;

    DWORD LWIN_key=0;
    DWORD RWIN_key=0;
    DWORD APPS_key=0;


    if  ( (nCode == HC_ACTION) && ((wParam == WM_SYSKEYDOWN) || (wParam == WM_KEYDOWN) || (wParam == WM_SYSKEYUP) || (wParam == WM_KEYUP)) )      
    {
        KBDLLHOOKSTRUCT hooked_key = *((KBDLLHOOKSTRUCT*)lParam);

        int key = hooked_key.vkCode;
		DWORD extendedflag = hooked_key.flags & LLKHF_EXTENDED;
		DWORD upflag = hooked_key.flags & LLKHF_UP;

        SHIFT_key = GetAsyncKeyState(VK_SHIFT);
        CTRL_key = GetAsyncKeyState(VK_CONTROL);
        ALT_key = GetAsyncKeyState(VK_MENU);
        LSHIFT_key = GetAsyncKeyState(VK_LSHIFT);
        LCTRL_key = GetAsyncKeyState(VK_LCONTROL);
        LALT_key = GetAsyncKeyState(VK_LMENU);
        RSHIFT_key = GetAsyncKeyState(VK_RSHIFT);
        RCTRL_key = GetAsyncKeyState(VK_RCONTROL);
        RALT_key = GetAsyncKeyState(VK_RMENU);

		LWIN_key = GetAsyncKeyState(VK_LWIN);
		RWIN_key = GetAsyncKeyState(VK_RWIN);
		APPS_key = GetAsyncKeyState(VK_APPS);

		//////////////////////////////
		//////////////////////////////
		// DOWN
		//////////////////////////////
		//////////////////////////////
		if (wParam == WM_SYSKEYDOWN)   
        {
			return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
		}
        else if ( (wParam == WM_KEYDOWN) && (key==VK_LSHIFT) )  //lshift
		{
			if(alreadydown_lshift) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_lshift=true;

			//printf("Keydown = lshift\n");
			global_outputmidichannel = global_outputmidichannel1;
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==VK_CAPITAL) )  //caps lock 
		{
			if(alreadydown_capslock) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_capslock=true;

			//printf("Keydown = capslock\n");
			global_outputmidichannel = global_outputmidichannel2;
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==VK_TAB) )  //tab 
		{
			if(alreadydown_tab) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_tab=true;

			//printf("Keydown = tab\n");
			global_outputmidichannel = global_outputmidichannel3;
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==192) )  //` 
		{
			if(alreadydown_ascii96) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_ascii96=true;

			//printf("Keydown = `\n");
			global_outputmidichannel = global_outputmidichannel4;
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==VK_UP) )  //up arrow 
		{
			if(alreadydown_uparrow) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_uparrow=true;

			//printf("Keydown = uparrow\n");
			global_octaveshift += 1;
			if(global_octaveshift>9) global_octaveshift=9;
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==VK_DOWN) )  //down arrow 
		{
			if(alreadydown_downarrow) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_downarrow=true;

			//printf("Keydown = downarrow\n");
			global_octaveshift -= 1;
			if(global_octaveshift<0) global_octaveshift=0;
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==VK_LEFT) )  //left arrow 
		{
			if(alreadydown_leftarrow) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_leftarrow=true;

			//printf("Keydown = leftarrow\n");
			global_octaveshift -= 1;
			if(global_octaveshift<0) global_octaveshift=0;
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==VK_RIGHT) )  //right arrow 
		{
			if(alreadydown_rightarrow) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_rightarrow=true;

			//printf("Keydown = rightarrow\n");
			global_octaveshift += 1;
			if(global_octaveshift>9) global_octaveshift=9;
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key >= '0' && key <= '9') )   
        {
			/*
			if(alreadydown_0to9) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_0to9=true;
			*/

			//printf("Keydown = %c\n",key);
			if(key=='1')
			{
				if(alreadydown_1) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_1=true;
				//no behavior
			}
			else if(key=='2')
			{
				if(alreadydown_2) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_2=true;
				local_notenumber = 1; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteon();
			}
			else if(key=='3')
			{
				if(alreadydown_3) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_3=true;
				local_notenumber = 3; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteon();
			}
			else if(key=='4')
			{
				if(alreadydown_4) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_4=true;
				//no behavior
			}
			else if(key=='5')
			{
				if(alreadydown_5) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_5=true;
				local_notenumber = 6; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteon();
			}
			else if(key=='6')
			{
				if(alreadydown_6) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_6=true;
				local_notenumber = 8; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteon();
			}
			else if(key=='7')
			{
				if(alreadydown_7) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_7=true;
				local_notenumber = 10; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteon();
			}
			else if(key=='8')
			{
				if(alreadydown_8) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_8=true;
				//no behavior
			}
			else if(key=='9')
			{
				if(alreadydown_9) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_9=true;
				local_notenumber = 1; //ranging between 0 and 11
				local_octaveshift = 2; //either 0, 1 or 2
				noteon();
			}
			else if(key=='0')
			{
				if(alreadydown_0) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_0=true;
				local_notenumber = 3; //ranging between 0 and 11
				local_octaveshift = 2; //either 0, 1 or 2
				noteon();
			}
			return 1;
		}
		else if ( (wParam == WM_KEYDOWN) && (key >= 'A' && key <= 'Z') )   
        {
			/*
			if(alreadydown_AtoZ) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_AtoZ=true;
			*/
			//QWERTYUIOP
			if(key=='Q')
			{
				if(alreadydown_Q) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_Q=true;
				local_notenumber = 0; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteon();
			}
			else if(key=='W')
			{
				if(alreadydown_W) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_W=true;
				local_notenumber = 2; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteon();
			}
			else if(key=='E')
			{
				if(alreadydown_E) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_E=true;
				local_notenumber = 4; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteon();
			}
			else if(key=='R')
			{
				if(alreadydown_R) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_R=true;
				local_notenumber = 5; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteon();
			}
			else if(key=='T')
			{
				if(alreadydown_T) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_T=true;
				local_notenumber = 7; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteon();
			}
			else if(key=='Y')
			{
				if(alreadydown_Y) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_Y=true;
				local_notenumber = 9; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteon();
			}
			else if(key=='U')
			{
				if(alreadydown_U) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_U=true;
				local_notenumber = 11; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteon();
			}
			else if(key=='I')
			{
				if(alreadydown_I) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_I=true;
				local_notenumber = 0; //ranging between 0 and 11
				local_octaveshift = 2; //either 0, 1 or 2
				noteon();
			}
			else if(key=='O')
			{
				if(alreadydown_O) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_O=true;
				local_notenumber = 2; //ranging between 0 and 11
				local_octaveshift = 2; //either 0, 1 or 2
				noteon();
			}
			else if(key=='P')
			{
				if(alreadydown_P) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_P=true;
				local_notenumber = 4; //ranging between 0 and 11
				local_octaveshift = 2; //either 0, 1 or 2
				noteon();
			}

			//ASDFGHJKL
			else if(key=='A')
			{
				if(alreadydown_A) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_A=true;
				//no behavior
			}
			else if(key=='S')
			{
				if(alreadydown_S) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_S=true;
				local_notenumber = 1; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteon();
			}
			else if(key=='D')
			{
				if(alreadydown_D) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_D=true;
				local_notenumber = 3; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteon();
			}
			else if(key=='F')
			{
				if(alreadydown_F) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_F=true;
				//no behavior
			}
			else if(key=='G')
			{
				if(alreadydown_G) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_G=true;
				local_notenumber = 6; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteon();
			}
			else if(key=='H')
			{
				if(alreadydown_H) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_H=true;
				local_notenumber = 8; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteon();
			}
			else if(key=='J')
			{
				if(alreadydown_J) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_J=true;
				local_notenumber = 10; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteon();
			}
			else if(key=='K')
			{
				if(alreadydown_K) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_K=true;
				//no behavior
			}
			else if(key=='L')
			{
				if(alreadydown_L) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_L=true;
				local_notenumber = 1; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteon();
			}

			//ZXCVBNM
			else if(key=='Z')
			{
				if(alreadydown_Z) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_Z=true;
				local_notenumber = 0; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteon();
			}
			else if(key=='X')
			{
				if(alreadydown_X) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_X=true;
				local_notenumber = 2; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteon();
			}
			else if(key=='C')
			{
				if(alreadydown_C) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_C=true;
				local_notenumber = 4; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteon();
			}
			else if(key=='V')
			{
				if(alreadydown_V) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_V=true;
				local_notenumber = 5; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteon();
			}
			else if(key=='B')
			{
				if(alreadydown_B) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_B=true;
				local_notenumber = 7; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteon();
			}
			else if(key=='N')
			{
				if(alreadydown_N) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_N=true;
				local_notenumber = 9; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteon();
			}
			else if(key=='M')
			{
				if(alreadydown_M) return 1; //CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				  else alreadydown_M=true;
				local_notenumber = 11; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteon();
			}

            //if  (GetAsyncKeyState(VK_SHIFT)>= 0) key +=32;
			/*
            if (CTRL_key !=0 && key == 'Y' )
            {
               MessageBoxA(NULL, "CTRL-y was pressed\nLaunch your app here", "H O T K E Y", MB_OK); 
               //CTRL_key=0;

               //do stuff here

			   return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
            }

            if (CTRL_key !=0 && key == 'Q' )
            {
                //MessageBoxA(NULL, "Shutting down", "H O T K E Y", MB_OK); 
				//ShellExecuteA(NULL, "open", global_end.c_str(), "", NULL, 0);
               PostQuitMessage(0);
            }
			*/

			//printf("Keydown = %c\n",key);
			return 1;
			/*
            SHIFT_key = 0;
            CTRL_key = 0;
            ALT_key = 0;
			LSHIFT_key=0;
			LCTRL_key=0;
			LALT_key=0;
			RSHIFT_key=0;
			RCTRL_key=0;
			RALT_key=0;
			*/
        }
        else if ( (wParam == WM_KEYDOWN) && (key==189) )  //minus with underscore 
		{
			if(alreadydown_minus) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_minus=true;

			printf("Keydown = minus\n");
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==187) )  //equals with plus 
		{
			if(alreadydown_equals) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_equals=true;

			//printf("Keydown = equal\n");
			local_notenumber = 6; //ranging between 0 and 11
			local_octaveshift = 2; //either 0, 1 or 2
			noteon();
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==VK_BACK) )  //backspace 
		{
			if(alreadydown_backspace) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_backspace=true;

			//printf("Keydown = backspace\n");
			local_notenumber = 8; //ranging between 0 and 11
			local_octaveshift = 2; //either 0, 1 or 2
			noteon();
			return 1;
		}

        else if ( (wParam == WM_KEYDOWN) && (key==219) )  //left brace 
		{
			if(alreadydown_leftbrace) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_leftbrace=true;

			//printf("Keydown = leftbrace\n");
			local_notenumber = 5; //ranging between 0 and 11
			local_octaveshift = 2; //either 0, 1 or 2
			noteon();
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==221) )  //right brace 
		{
			if(alreadydown_rightbrace) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_rightbrace=true;

			//printf("Keydown = rightbrace\n");
			local_notenumber = 7; //ranging between 0 and 11
			local_octaveshift = 2; //either 0, 1 or 2
			noteon();
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==220) )  //backslash 
		{
			if(alreadydown_backslash) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_backslash=true;

			//printf("Keydown = backslash\n");
			local_notenumber = 9; //ranging between 0 and 11
			local_octaveshift = 2; //either 0, 1 or 2
			noteon();
			return 1;
		}

        else if ( (wParam == WM_KEYDOWN) && (key==186) )  //semicolon 
		{
			if(alreadydown_semicolon) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_semicolon=true;

			//printf("Keydown = semicolon\n");
			local_notenumber = 3; //ranging between 0 and 11
			local_octaveshift = 1; //either 0, 1 or 2
			noteon();
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==222) )  //apostrophe 
		{
			if(alreadydown_apostrophe) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_apostrophe=true;

			//printf("Keydown = apostrophe\n");
			//no behavior
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==VK_RETURN) && (extendedflag==0) )  //return 
		{
			if(alreadydown_return) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_return=true;

			//printf("Keydown = return\n");
			local_notenumber = 6; //ranging between 0 and 11
			local_octaveshift = 1; //either 0, 1 or 2
			noteon();
			return 1;
		}

        else if ( (wParam == WM_KEYDOWN) && (key==188) )  //comma 
		{
			if(alreadydown_comma) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_comma=true;

			//printf("Keydown = comma\n");
			local_notenumber = 0; //ranging between 0 and 11
			local_octaveshift = 1; //either 0, 1 or 2
			noteon();
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==190) )  //period 
		{
			if(alreadydown_period) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_period=true;

			//printf("Keydown = period\n");
			local_notenumber = 2; //ranging between 0 and 11
			local_octaveshift = 1; //either 0, 1 or 2
			noteon();
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==191) )  //slash (forward slash) 
		{
			if(alreadydown_slash) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_slash=true;

			//printf("Keydown = slash\n");
			local_notenumber = 4; //ranging between 0 and 11
			local_octaveshift = 1; //either 0, 1 or 2
			noteon();
			return 1;
		}
        else if ( (wParam == WM_KEYDOWN) && (key==VK_RSHIFT) )  //rshift 
		{
			if(alreadydown_rshift) return 1;//CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
			  else alreadydown_rshift=true;

			//printf("Keydown = rshift\n");
			local_notenumber = 5; //ranging between 0 and 11
			local_octaveshift = 1; //either 0, 1 or 2
			noteon();
			return 1;
		}
		//////////////////////////////
		//////////////////////////////
		// UP
		//////////////////////////////
		//////////////////////////////
        else if (wParam == WM_SYSKEYUP)   
        {
			return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
		}
        else if ( (wParam == WM_KEYUP) && (key==VK_LSHIFT) )  //lshift
		{
			alreadydown_lshift=false;

			//printf("Keyup = lshift\n");
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==VK_CAPITAL) )  //caps lock 
		{
			alreadydown_capslock=false;

			//printf("Keyup = capslock\n");
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==VK_TAB) )  //tab 
		{
			alreadydown_tab=false;

			//printf("Keyup = tab\n");
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==192) )  //` 
		{
			alreadydown_ascii96=false;

			//printf("Keyup = `\n");
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==VK_UP) )  //up arrow 
		{
			alreadydown_uparrow=false;

			//printf("Keyup = uparrow\n");
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==VK_DOWN) )  //down arrow 
		{
			alreadydown_downarrow=false;

			//printf("Keyup = downarrow\n");
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==VK_LEFT) )  //left arrow 
		{
			alreadydown_leftarrow=false;

			//printf("Keyup = leftarrow\n");
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==VK_RIGHT) )  //right arrow 
		{
			alreadydown_rightarrow=false;

			//printf("Keyup = rightarrow\n");
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key >= '0' && key <= '9') )   
        {
			/*
			alreadydown_0to9= false;
			*/
			//printf("Keyup = %c\n",key);
			if(key=='1')
			{
				alreadydown_1 = false;
				//no behavior
			}
			else if(key=='2')
			{
				alreadydown_2 = false;
				local_notenumber = 1; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='3')
			{
				alreadydown_3 = false;
				local_notenumber = 3; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='4')
			{
				alreadydown_4 = false;
				//no behavior
			}
			else if(key=='5')
			{
				alreadydown_5 = false;
				local_notenumber = 6; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='6')
			{
				alreadydown_6 = false;
				local_notenumber = 8; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='7')
			{
				alreadydown_7 = false;
				local_notenumber = 10; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='8')
			{
				alreadydown_8 = false;
				//no behavior
			}
			else if(key=='9')
			{
				alreadydown_9 = false;
				local_notenumber = 1; //ranging between 0 and 11
				local_octaveshift = 2; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='0')
			{
				alreadydown_0 = false;
				local_notenumber = 3; //ranging between 0 and 11
				local_octaveshift = 2; //either 0, 1 or 2
				noteoff();
			}
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key >= 'A' && key <= 'Z') )   
        {
			/*
			alreadydown_AtoZ= false;
			*/
			//QWERTYUIOP
			if(key=='Q')
			{
				alreadydown_Q= false;
				local_notenumber = 0; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='W')
			{
				alreadydown_W= false;
				local_notenumber = 2; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='E')
			{
				alreadydown_E= false;
				local_notenumber = 4; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='R')
			{
				alreadydown_R= false;
				local_notenumber = 5; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='T')
			{
				alreadydown_T= false;
				local_notenumber = 7; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='Y')
			{
				alreadydown_Y= false;
				local_notenumber = 9; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='U')
			{
				alreadydown_U= false;
				local_notenumber = 11; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='I')
			{
				alreadydown_I= false;
				local_notenumber = 0; //ranging between 0 and 11
				local_octaveshift = 2; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='O')
			{
				alreadydown_O= false;
				local_notenumber = 2; //ranging between 0 and 11
				local_octaveshift = 2; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='P')
			{
				alreadydown_P= false;
				local_notenumber = 4; //ranging between 0 and 11
				local_octaveshift = 2; //either 0, 1 or 2
				noteoff();
			}

			//ASDFGHJKL
			else if(key=='A')
			{
				alreadydown_A= false;
				//no behavior
			}
			else if(key=='S')
			{
				alreadydown_S= false;
				local_notenumber = 1; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='D')
			{
				alreadydown_D= false;
				local_notenumber = 3; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='F')
			{
				alreadydown_F= false;
				//no behavior
			}
			else if(key=='G')
			{
				alreadydown_G= false;
				local_notenumber = 6; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='H')
			{
				alreadydown_H= false;
				local_notenumber = 8; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='J')
			{
				alreadydown_J= false;
				local_notenumber = 10; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='K')
			{
				alreadydown_K= false;
				//no behavior
			}
			else if(key=='L')
			{
				alreadydown_L= false;
				local_notenumber = 1; //ranging between 0 and 11
				local_octaveshift = 1; //either 0, 1 or 2
				noteoff();
			}

			//ZXCVBNM
			else if(key=='Z')
			{
				alreadydown_Z= false;
				local_notenumber = 0; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='X')
			{
				alreadydown_X= false;
				local_notenumber = 2; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='C')
			{
				alreadydown_C= false;
				local_notenumber = 4; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='V')
			{
				alreadydown_V= false;
				local_notenumber = 5; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='B')
			{
				alreadydown_B= false;
				local_notenumber = 7; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='N')
			{
				alreadydown_N= false;
				local_notenumber = 9; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteoff();
			}
			else if(key=='M')
			{
				alreadydown_M= false;
				local_notenumber = 11; //ranging between 0 and 11
				local_octaveshift = 0; //either 0, 1 or 2
				noteoff();
			}
			//printf("Keyup = %c\n",key);
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==189) )  //minus with underscore 
		{
			alreadydown_minus=false;

			printf("Keyup = minus\n");
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==187) )  //equals with plus 
		{
			alreadydown_equals=false;

			//printf("Keyup = equal\n");
			local_notenumber = 6; //ranging between 0 and 11
			local_octaveshift = 2; //either 0, 1 or 2
			noteoff();
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==VK_BACK) )  //backspace 
		{
			alreadydown_backspace=false;

			//printf("Keyup = backspace\n");
			local_notenumber = 8; //ranging between 0 and 11
			local_octaveshift = 2; //either 0, 1 or 2
			noteoff();
			return 1;
		}

        else if ( (wParam == WM_KEYUP) && (key==219) )  //left brace 
		{
			alreadydown_leftbrace=false;

			//printf("Keyup = leftbrace\n");
			local_notenumber = 5; //ranging between 0 and 11
			local_octaveshift = 2; //either 0, 1 or 2
			noteoff();
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==221) )  //right brace 
		{
			alreadydown_rightbrace=false;

			//printf("Keyup = rightbrace\n");
			local_notenumber = 7; //ranging between 0 and 11
			local_octaveshift = 2; //either 0, 1 or 2
			noteoff();
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==220) )  //backslash 
		{
			alreadydown_backslash=false;

			//printf("Keyup = backslash\n");
			local_notenumber = 9; //ranging between 0 and 11
			local_octaveshift = 2; //either 0, 1 or 2
			noteoff();
			return 1;
		}

        else if ( (wParam == WM_KEYUP) && (key==186) )  //semicolon 
		{
			alreadydown_semicolon=false;

			//printf("Keyup = semicolon\n");
			local_notenumber = 3; //ranging between 0 and 11
			local_octaveshift = 1; //either 0, 1 or 2
			noteoff();
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==222) )  //apostrophe 
		{
			alreadydown_apostrophe=false;

			//printf("Keyup = apostrophe\n");
			//no behavior
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==VK_RETURN) && (extendedflag==0) )  //return 
		{
			alreadydown_return=false;

			//printf("Keyup = return\n");
			local_notenumber = 6; //ranging between 0 and 11
			local_octaveshift = 1; //either 0, 1 or 2
			noteoff();
			return 1;
		}

        else if ( (wParam == WM_KEYUP) && (key==188) )  //comma 
		{
			alreadydown_comma=false;

			//printf("Keyup = comma\n");
			local_notenumber = 0; //ranging between 0 and 11
			local_octaveshift = 1; //either 0, 1 or 2
			noteoff();
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==190) )  //period 
		{
			alreadydown_period=false;

			//printf("Keyup = period\n");
			local_notenumber = 2; //ranging between 0 and 11
			local_octaveshift = 1; //either 0, 1 or 2
			noteoff();
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==191) )  //slash (forward slash) 
		{
			alreadydown_slash=false;

			//printf("Keyup = slash\n");
			local_notenumber = 4; //ranging between 0 and 11
			local_octaveshift = 1; //either 0, 1 or 2
			noteoff();
			return 1;
		}
        else if ( (wParam == WM_KEYUP) && (key==VK_RSHIFT) )  //rshift 
		{
			alreadydown_rshift=false;

			//printf("Keyup = rshift\n");
			local_notenumber = 5; //ranging between 0 and 11
			local_octaveshift = 1; //either 0, 1 or 2
			noteoff();
			return 1;
		}

	}
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}


void CALLBACK StartGlobalProcess(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	//WavSetLib_Initialize(global_hwnd, IDC_MAIN_STATIC, global_staticwidth, global_staticheight, global_fontwidth, global_fontheight);
	global_pfile = fopen("output.txt","w");
	WavSetLib_Initialize(global_hwnd, IDC_MAIN_STATIC, global_staticwidth, global_staticheight, global_fontwidth, global_fontheight, global_staticalignment, global_pfile);


	//////////////////////////
	//initialize random number
	//////////////////////////
	//srand((unsigned)time(0));
	srand((unsigned int)GetTickCount());

	/////////////////////
	//initialize portmidi
	/////////////////////
    PmError err;
	Pm_Initialize(); 

	if ( !global_inputmididevicename.empty() )
	{
		/////////////////////////////
		//input midi device selection
		/////////////////////////////
		const PmDeviceInfo* deviceInfo;
		int numDevices = Pm_CountDevices();
		for( int i=0; i<numDevices; i++ )
		{
			deviceInfo = Pm_GetDeviceInfo( i );
			if (deviceInfo->input)
			{
				string devicenamestring = deviceInfo->name;
				global_inputmididevicemap.insert(pair<string,int>(devicenamestring,i));
			}
		}
		map<string,int>::iterator it;
		it = global_inputmididevicemap.find(global_inputmididevicename);
		if(it!=global_inputmididevicemap.end())
		{
			global_inputmidideviceid = (*it).second;
			sprintf(pCHAR, "%s maps to %d\n", global_inputmididevicename.c_str(), global_inputmidideviceid);StatusAddTextA(pCHAR);
			deviceInfo = Pm_GetDeviceInfo(global_inputmidideviceid);
		}
		else
		{
			assert(false);
			for(it=global_inputmididevicemap.begin(); it!=global_inputmididevicemap.end(); it++)
			{
				sprintf(pCHAR, "%s maps to %d\n", (*it).first.c_str(), (*it).second);StatusAddTextA(pCHAR);
			}
			swprintf(pWCHAR, L"input midi device not found\n");StatusAddText(pWCHAR);
			return;
		}
	}

	//////////////////////////////
	//output midi device selection
	//////////////////////////////
	const PmDeviceInfo* deviceInfo;
    int numDevices = Pm_CountDevices();
    for( int i=0; i<numDevices; i++ )
    {
        deviceInfo = Pm_GetDeviceInfo( i );
		if (deviceInfo->output)
		{
			string devicenamestring = deviceInfo->name;
			global_outputmididevicemap.insert(pair<string,int>(devicenamestring,i));
		}
	}
	map<string,int>::iterator it;
	it = global_outputmididevicemap.find(global_outputmididevicename);
	if(it!=global_outputmididevicemap.end())
	{
		global_outputmidideviceid = (*it).second;
		sprintf(pCHAR, "%s maps to %d\n", global_outputmididevicename.c_str(), global_outputmidideviceid);StatusAddTextA(pCHAR);
		deviceInfo = Pm_GetDeviceInfo(global_outputmidideviceid);
	}
	else
	{
		assert(false);
		for(it=global_outputmididevicemap.begin(); it!=global_outputmididevicemap.end(); it++)
		{
			sprintf(pCHAR, "%s maps to %d\n", (*it).first.c_str(), (*it).second);StatusAddTextA(pCHAR);
		}
		swprintf(pWCHAR, L"output midi device not found\n");StatusAddText(pWCHAR);
	}

    // list device information 
    swprintf(pWCHAR, L"MIDI output devices:\n");StatusAddText(pWCHAR);
    for (int i = 0; i < Pm_CountDevices(); i++) 
	{
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if (info->output) 
		{
			sprintf(pCHAR, "%d: %s, %s\n", i, info->interf, info->name);StatusAddTextA(pCHAR);
		}
    }
	swprintf(pWCHAR, L"device %d selected\n", global_outputmidideviceid);StatusAddText(pWCHAR);
    //err = Pm_OpenInput(&midi_in, inp, NULL, 512, NULL, NULL);
    err = Pm_OpenOutput(&global_pPmStreamMIDIOUT, global_outputmidideviceid, NULL, 512, NULL, NULL, 0); //0 latency
    if (err) 
	{
        sprintf(pCHAR, "%s\n", Pm_GetErrorText(err));StatusAddTextA(pCHAR);
        //Pt_Stop();
        //mmexit(1);
		return;
    }

	if ( !global_inputmididevicename.empty() )
	{
		//////////////////////////
		//start polling midi input
		//////////////////////////

		// use porttime callback to empty midi queue and print 
		Pt_Start(1, receive_poll, 0); 
		// list device information 
		swprintf(pWCHAR, L"MIDI input devices:\n");StatusAddText(pWCHAR);
		for (int i = 0; i < Pm_CountDevices(); i++) 
		{
			const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
			if (info->input) 
			{
				sprintf(pCHAR, "%d: %s, %s\n", i, info->interf, info->name);StatusAddTextA(pCHAR);
			}
		}
		//inputmididevice = get_number("Type input device number: ");
		swprintf(pWCHAR, L"device %d selected\n", global_inputmidideviceid);StatusAddText(pWCHAR);

		err = Pm_OpenInput(&global_pPmStreamMIDIIN, global_inputmidideviceid, NULL, 512, NULL, NULL);
		if (err) 
		{
			/*
			sprintf(pCHAR, Pm_GetErrorText(err));StatusAddTextA(pCHAR);
			Pt_Stop();
			return;
			*/
			//disconnect portmidi input
			global_active = false;
			swprintf(pWCHAR, L"****************************\n");StatusAddText(pWCHAR);
			swprintf(pWCHAR, L"midi input port DISCONNECTED\n");StatusAddText(pWCHAR);
			swprintf(pWCHAR, L"****************************\n");StatusAddText(pWCHAR);
		}
		else
		{
			Pm_SetFilter(global_pPmStreamMIDIIN, mySpiMidiUtility.filter);
			global_inited = true; // now can document changes, set filter 
			//swprintf(pWCHAR, L"spimidikeyboardwin32 ready.\n");StatusAddText(pWCHAR);
			global_active = true;
			swprintf(pWCHAR, L"*************************\n");StatusAddText(pWCHAR);
			swprintf(pWCHAR, L"midi input port CONNECTED\n");StatusAddText(pWCHAR);
			swprintf(pWCHAR, L"*************************\n");StatusAddText(pWCHAR);
		}
	}

	global_midiready = true;
	/* //spinote: hook was not working when inserted in here 
	HINSTANCE hInstance = GetModuleHandle(NULL);
    //hKeyboardHook = SetWindowsHookEx (  WH_KEYBOARD_LL, (HOOKPROC) KeyboardEvent,   global_hInstance,  NULL    );
    hKeyboardHook = SetWindowsHookEx (  WH_KEYBOARD_LL, (HOOKPROC) KeyboardEvent,   hInstance,  NULL    );
    //hKeyboardHook = SetWindowsHookEx (  WH_KEYBOARD_LL, (HOOKPROC) KeyboardEvent,   NULL,  NULL    );
	*/
	//PostMessage(global_hwnd, WM_DESTROY, 0, 0);
}




int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//LPWSTR *szArgList;
	LPSTR *szArgList;
	int nArgs;
	int i;

	//szArgList = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	szArgList = CommandLineToArgvA(GetCommandLineA(), &nArgs);
	if( NULL == szArgList )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}
	LPWSTR *szArgListW;
	int nArgsW;
	szArgListW = CommandLineToArgvW(GetCommandLineW(), &nArgsW);
	if( NULL == szArgListW )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}

	if(nArgs>1)
	{
		//input midi device
		//inputmididevice=atoi(argv[1]);
		global_inputmididevicename=szArgList[1];
	}
	if(nArgs>2)
	{
		//output midi channel
		global_inputmidichannel = atoi(szArgList[2]);
	}
	if(nArgs>3)
	{
		//output midi device
		//outputmididevice = atoi(argv[3]);
		global_outputmididevicename = szArgList[3];
	}
	if(nArgs>4)
	{
		//output midi channel
		global_outputmidichannel1 = atoi(szArgList[4]);
	}
	if(nArgs>5)
	{
		//output midi channel
		global_outputmidichannel2 = atoi(szArgList[5]);
	}
	if(nArgs>6)
	{
		//output midi channel
		global_outputmidichannel3 = atoi(szArgList[6]);
	}
	if(nArgs>7)
	{
		//output midi channel
		global_outputmidichannel4 = atoi(szArgList[7]);
	}
	if(nArgs>8)
	{
		global_x = atoi(szArgList[8]);
	}
	if(nArgs>9)
	{
		global_y = atoi(szArgList[9]);
	}
	if(nArgs>10)
	{
		global_xwidth = atoi(szArgList[10]);
	}
	if(nArgs>11)
	{
		global_yheight = atoi(szArgList[11]);
	}
	if(nArgs>12)
	{
		global_alpha = atoi(szArgList[12]);
	}
	if(nArgs>13)
	{
		global_titlebardisplay = atoi(szArgList[13]);
	}
	if(nArgs>14)
	{
		global_menubardisplay = atoi(szArgList[14]);
	}
	if(nArgs>15)
	{
		global_acceleratoractive = atoi(szArgList[15]);
	}
	if(nArgs>16)
	{
		global_fontheight = atoi(szArgList[16]);
	}
	if(nArgs>17)
	{
		global_fontcolor_r = atoi(szArgList[17]);
	}
	if(nArgs>18)
	{
		global_fontcolor_g = atoi(szArgList[18]);
	}
	if(nArgs>19)
	{
		global_fontcolor_b = atoi(szArgList[19]);
	}
	if(nArgs>20)
	{
		global_staticalignment = atoi(szArgList[20]);
	}
	//new parameters
	if(nArgs>21)
	{
		wcscpy(szWindowClass, szArgListW[21]); 
	}
	if(nArgs>22)
	{
		wcscpy(szTitle, szArgListW[22]); 
	}
	if(nArgs>23)
	{
		global_begin = szArgList[23]; 
	}
	if(nArgs>24)
	{
		global_end = szArgList[24]; 
	}
	if(nArgs>25)
	{
		global_octaveshift = atoi(szArgList[25]); 
	}
	if(nArgs>26)
	{
		global_velocity = atoi(szArgList[26]); 
	}
	if(nArgs>27)
	{
		global_visibility = atoi(szArgList[27]);
	}
	LocalFree(szArgList);
	LocalFree(szArgListW);

	int nShowCmd = false;
	//ShellExecuteA(NULL, "open", "begin.bat", "", NULL, nShowCmd);
	ShellExecuteA(NULL, "open", global_begin.c_str(), "", NULL, nCmdShow);

	if(global_outputmidichannel1<0 || global_outputmidichannel1>15)
	{
		swprintf(pWCHAR, L"invalid outputmidichannel1, midi channel must range from 0 to 15 inclusively.\n");StatusAddText(pWCHAR);
		return FALSE;
	}
	if(global_outputmidichannel2<0 || global_outputmidichannel2>15)
	{
		swprintf(pWCHAR, L"invalid outputmidichannel2, midi channel must range from 0 to 15 inclusively.\n");StatusAddText(pWCHAR);
		return FALSE;
	}
	if(global_outputmidichannel3<0 || global_outputmidichannel3>15)
	{
		swprintf(pWCHAR, L"invalid outputmidichannel3, midi channel must range from 0 to 15 inclusively.\n");StatusAddText(pWCHAR);
		return FALSE;
	}
	if(global_outputmidichannel4<0 || global_outputmidichannel4>15)
	{
		swprintf(pWCHAR, L"invalid outputmidichannel4, midi channel must range from 0 to 15 inclusively.\n");StatusAddText(pWCHAR);
		return FALSE;
	}
	global_outputmidichannel = global_outputmidichannel1;


	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_SPIWAVWIN32, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	if(global_acceleratoractive)
	{
		hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SPIWAVWIN32));
	}
	else
	{
		hAccelTable = NULL;
	}
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	//wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPIWAVWIN32));
	wcex.hIcon			= (HICON)LoadImage(NULL, L"background_32x32x16.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);

	if(global_menubardisplay)
	{
		wcex.lpszMenuName = MAKEINTRESOURCE(IDC_SPIWAVWIN32); //original with menu
	}
	else
	{
		wcex.lpszMenuName = NULL; //no menu
	}
	wcex.lpszClassName	= szWindowClass;
	//wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.hIconSm		= (HICON)LoadImage(NULL, L"background_16x16x16.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	global_hInstance = hInstance; // Store instance handle in our global variable

	global_dib = FreeImage_Load(FIF_JPEG, "background.jpg", JPEG_DEFAULT);

	//global_hFont=CreateFontW(32,0,0,0,FW_BOLD,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
	global_hFont=CreateFontW(global_fontheight,0,0,0,FW_NORMAL,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");

	if(global_titlebardisplay)
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, //original with WS_CAPTION etc.
			global_x, global_y, global_xwidth, global_yheight, NULL, NULL, hInstance, NULL);
	}
	else
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP | WS_VISIBLE, //no WS_CAPTION etc.
			global_x, global_y, global_xwidth, global_yheight, NULL, NULL, hInstance, NULL);
	}
	if (!hWnd)
	{
		return FALSE;
	}
	global_hwnd = hWnd;

	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, global_alpha, LWA_ALPHA);

	if(global_visibility) ShowWindow(hWnd, nCmdShow);
	  else ShowWindow(hWnd, SW_HIDE);
	UpdateWindow(hWnd);

	HMODULE hModule = GetModuleHandle(NULL);
    //hKeyboardHook = SetWindowsHookEx (  WH_KEYBOARD_LL, (HOOKPROC) KeyboardEvent,   global_hInstance,  NULL    );
    hKeyboardHook = SetWindowsHookEx (  WH_KEYBOARD_LL, (HOOKPROC) KeyboardEvent,   hModule,  NULL    );
    //hKeyboardHook = SetWindowsHookEx (  WH_KEYBOARD_LL, (HOOKPROC) KeyboardEvent,   NULL,  NULL    );

	return TRUE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HGDIOBJ hOldBrush;
	HGDIOBJ hOldPen;
	int iOldMixMode;
	COLORREF crOldBkColor;
	COLORREF crOldTextColor;
	int iOldBkMode;
	HFONT hOldFont, hFont;
	TEXTMETRIC myTEXTMETRIC;

	/////////////////////////
	//midi message definition
	/////////////////////////
	PmEvent myPmEvent[2];
	//note on
	myPmEvent[0].timestamp = 0;
	myPmEvent[0].message = Pm_Message(0x90+global_outputmidichannel, 60, 100); //channel 0
	//myPmEvent[0].message = Pm_Message(0x91, 60, 100); //channel 1
	//note off
	myPmEvent[1].timestamp = 0;
	myPmEvent[1].message = Pm_Message(0x90+global_outputmidichannel, 60, 0); //channel 0
	//myPmEvent[1].message = Pm_Message(0x91, 60, 0); //channel 1

	PmEvent tempPmEvent;

	switch (message)
	{
	case WM_CREATE:
		{
			//HWND hStatic = CreateWindowEx(WS_EX_TRANSPARENT, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_CENTER,  
			HWND hStatic = CreateWindowEx(WS_EX_TRANSPARENT, L"STATIC", L"", WS_CHILD | WS_VISIBLE | global_staticalignment, 
				0, 100, 100, 100, hWnd, (HMENU)IDC_MAIN_STATIC, GetModuleHandle(NULL), NULL);
			if(hStatic == NULL)
				MessageBox(hWnd, L"Could not create static text.", L"Error", MB_OK | MB_ICONERROR);
			SendMessage(hStatic, WM_SETFONT, (WPARAM)global_hFont, MAKELPARAM(FALSE, 0));



			global_timer=timeSetEvent(1000,25,(LPTIMECALLBACK)&StartGlobalProcess,0,TIME_ONESHOT);
		}
		break;
	case WM_SIZE:
		{
			RECT rcClient;

			GetClientRect(hWnd, &rcClient);
			/*
			HWND hEdit = GetDlgItem(hWnd, IDC_MAIN_EDIT);
			SetWindowPos(hEdit, NULL, 0, 0, rcClient.right/2, rcClient.bottom/2, SWP_NOZORDER);
			*/
			HWND hStatic = GetDlgItem(hWnd, IDC_MAIN_STATIC);
			global_staticwidth = rcClient.right - 0;
			//global_staticheight = rcClient.bottom-(rcClient.bottom/2);
			global_staticheight = rcClient.bottom-0;
			//spi, begin
			global_imagewidth = rcClient.right - 0;
			global_imageheight = rcClient.bottom - 0; 
			WavSetLib_Initialize(global_hwnd, IDC_MAIN_STATIC, global_staticwidth, global_staticheight, global_fontwidth, global_fontheight, global_staticalignment, global_pfile);
			//spi, end
			//SetWindowPos(hStatic, NULL, 0, rcClient.bottom/2, global_staticwidth, global_staticheight, SWP_NOZORDER);
			SetWindowPos(hStatic, NULL, 0, 0, global_staticwidth, global_staticheight, SWP_NOZORDER);
		}
		break;
	case WM_CTLCOLOREDIT:
		{
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, RGB(0xFF, 0xFF, 0xFF));
			return (INT_PTR)::GetStockObject(NULL_PEN);
		}
		break;
	case WM_CTLCOLORSTATIC:
		{
			SetBkMode((HDC)wParam, TRANSPARENT);
			//SetTextColor((HDC)wParam, RGB(0xFF, 0xFF, 0xFF));
			SetTextColor((HDC)wParam, RGB(global_fontcolor_r, global_fontcolor_g, global_fontcolor_b));
			return (INT_PTR)::GetStockObject(NULL_PEN);
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(global_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		SetStretchBltMode(hdc, COLORONCOLOR);
		//spi, begin
		/*
		StretchDIBits(hdc, 0, 0, global_xwidth, global_yheight,
						0, 0, FreeImage_GetWidth(global_dib), FreeImage_GetHeight(global_dib),
						FreeImage_GetBits(global_dib), FreeImage_GetInfo(global_dib), DIB_RGB_COLORS, SRCCOPY);
		*/
		StretchDIBits(hdc, 0, 0, global_imagewidth, global_imageheight,
						0, 0, FreeImage_GetWidth(global_dib), FreeImage_GetHeight(global_dib),
						FreeImage_GetBits(global_dib), FreeImage_GetInfo(global_dib), DIB_RGB_COLORS, SRCCOPY);
		//spi, end
		hOldBrush = SelectObject(hdc, (HBRUSH)GetStockObject(GRAY_BRUSH));
		hOldPen = SelectObject(hdc, (HPEN)GetStockObject(WHITE_PEN));
		//iOldMixMode = SetROP2(hdc, R2_MASKPEN);
		iOldMixMode = SetROP2(hdc, R2_MERGEPEN);
		//Rectangle(hdc, 100, 100, 200, 200);

		crOldBkColor = SetBkColor(hdc, RGB(0xFF, 0x00, 0x00));
		crOldTextColor = SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
		iOldBkMode = SetBkMode(hdc, TRANSPARENT);
		//hFont=CreateFontW(70,0,0,0,FW_BOLD,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
		//hOldFont=(HFONT)SelectObject(hdc,global_hFont);
		hOldFont=(HFONT)SelectObject(hdc,global_hFont);
		GetTextMetrics(hdc, &myTEXTMETRIC);
		global_fontwidth = myTEXTMETRIC.tmAveCharWidth;
		//TextOutW(hdc, 100, 100, L"test string", 11);

		SelectObject(hdc, hOldBrush);
		SelectObject(hdc, hOldPen);
		SetROP2(hdc, iOldMixMode);
		SetBkColor(hdc, crOldBkColor);
		SetTextColor(hdc, crOldTextColor);
		SetBkMode(hdc, iOldBkMode);
		SelectObject(hdc,hOldFont);
		//DeleteObject(hFont);
		EndPaint(hWnd, &ps);
		break;
	case WM_KEYDOWN:
		break;
	case WM_TIMER:
		break;
	case WM_DESTROY:
		{
			//release hook
			UnhookWindowsHookEx(hKeyboardHook);

			//send note off
			PmEvent tempPmEvent;
			for(int notenumber=0; notenumber<128; notenumber++)
			{
				tempPmEvent.timestamp = 0;
				tempPmEvent.message = Pm_Message(MIDI_OFF_NOTE+global_outputmidichannel1, notenumber, 0); //note off, channel global_outputmidichannel1
				Pm_Write(global_pPmStreamMIDIOUT, &tempPmEvent, 1);

				tempPmEvent.timestamp = 0;
				tempPmEvent.message = Pm_Message(MIDI_OFF_NOTE+global_outputmidichannel2, notenumber, 0); //note off, channel global_outputmidichannel2
				Pm_Write(global_pPmStreamMIDIOUT, &tempPmEvent, 1);

				tempPmEvent.timestamp = 0;
				tempPmEvent.message = Pm_Message(MIDI_OFF_NOTE+global_outputmidichannel3, notenumber, 0); //note off, channel global_outputmidichannel3
				Pm_Write(global_pPmStreamMIDIOUT, &tempPmEvent, 1);

				tempPmEvent.timestamp = 0;
				tempPmEvent.message = Pm_Message(MIDI_OFF_NOTE+global_outputmidichannel4, notenumber, 0); //note off, channel global_outputmidichannel4
				Pm_Write(global_pPmStreamMIDIOUT, &tempPmEvent, 1);
			}

			//terminate portmidi
			global_active = false;
			Pm_Close(global_pPmStreamMIDIIN);
			Pm_Close(global_pPmStreamMIDIOUT);
			Pt_Stop();
			Pm_Terminate();
			//terminate portaudio
			//Pa_Terminate();
			//delete all memory allocations
			//if(global_pInstrument) delete global_pInstrument;
			//close file
			if(global_pfile) fclose(global_pfile);
			//terminate wavset library
			WavSetLib_Terminate();
			//terminate win32 app.
			if (global_timer) timeKillEvent(global_timer);
			FreeImage_Unload(global_dib);
			DeleteObject(global_hFont);

			int nShowCmd = false;
			//ShellExecuteA(NULL, "open", "end.bat", "", NULL, nShowCmd);
			ShellExecuteA(NULL, "open", global_end.c_str(), "", NULL, 0);
			PostQuitMessage(0);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
