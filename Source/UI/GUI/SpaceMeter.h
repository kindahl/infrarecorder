/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2010 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <atlcrack.h>	// COMMAND_RANGE_HANDLER_EX
#include "VisualStyles.h"
#include "CtrlMessages.h"

#define SPACEMETER_BAR_HEIGHT				10
#define SPACEMETER_BARINDENT_NORMAL			2
#define SPACEMETER_BARINDENT_THEMED			4
#define SPACEMETER_BARINDENT_BOTTOM			16

#define SPACEMETER_METER_SPACING			2
#define SPACEMETER_METER_HEIGHT				2
#define SPACEMETER_METER_COUNT				10
#define SPACEMETER_METERTEXT_SIZE			16
#define SPACEMETER_METERTEXT_INDENT_LEFT	4

/*
	New Tango colors.
*/
// Normal colors.
#define SPACEMETER_BORDERCOLOR				RGB(32,74,135)
#define SPACEMETER_BARCOLOR_FREETOP			RGB(114,159,205)
#define SPACEMETER_BARCOLOR_FREEBOTTOM		RGB(255,255,255)
#define SPACEMETER_BARCOLOR_TOP				RGB(114,159,205)
#define SPACEMETER_BARCOLOR_BOTTOM			RGB(52,101,164)

// Warning colors (when overburning will most probably work).
#define SPACEMETER_WARNBORDERCOLOR			RGB(206,92,0)
#define SPACEMETER_WARNBARCOLOR_FREETOP		RGB(252,175,62)
#define SPACEMETER_WARNBARCOLOR_FREEBOTTOM	RGB(255,255,255)
#define SPACEMETER_WARNBARCOLOR_TOP			RGB(252,175,62)
#define SPACEMETER_WARNBARCOLOR_BOTTOM		RGB(245,121,0)

// Disc full colors.
#define SPACEMETER_FULLBORDERCOLOR			RGB(164,0,0)
#define SPACEMETER_FULLBARCOLOR_FREETOP		RGB(239,41,41)
#define SPACEMETER_FULLBARCOLOR_FREEBOTTOM	RGB(255,255,255)
#define SPACEMETER_FULLBARCOLOR_TOP			RGB(239,41,41)
#define SPACEMETER_FULLBARCOLOR_BOTTOM		RGB(204,0,0)

// How many bytes is it safe to overburn.
#define SPACEMETER_OVERBURNSIZE				13516800	// 6600 * 2048 = 13 MiB.
#define SPACEMETER_OVERBURNLENGTH			88 * 1000	// 6600 sectors = 88 seconds.

#define SPACEMETER_DMSIZE					0
#define SPACEMETER_DMLENGTH					1

// Different draw states.
#define SPACEMETER_DRAWSTATE_NORMAL			0
#define SPACEMETER_DRAWSTATE_WARN			1
#define SPACEMETER_DRAWSTATE_FULL			2
#define SPACEMETER_DRAWSTATE_OUTOFSCOPE		3

// Disc sizes (that should be passed to SetDiscSize).
#define SPACEMETER_SIZE_DLDVD				0
#define SPACEMETER_SIZE_DVD					1
#define SPACEMETER_SIZE_870MB				2
#define SPACEMETER_SIZE_791MB				3
#define SPACEMETER_SIZE_703MB				4
#define SPACEMETER_SIZE_650MB				5
#define SPACEMETER_SIZE_210MB				6
#define SPACEMETER_SIZE_185MB				7
#define SPACEMETER_SIZE_1020MIN				8
#define SPACEMETER_SIZE_510MIN				9
#define SPACEMETER_SIZE_99MIN				10
#define SPACEMETER_SIZE_90MIN				11
#define SPACEMETER_SIZE_80MIN				12
#define SPACEMETER_SIZE_74MIN				13
#define SPACEMETER_SIZE_24MIN				14
#define SPACEMETER_SIZE_21MIN				15

// Specifies which ID the first recent menu item will have, the second will have + 1 and so on.
#define SPACEMETER_POPUPMENU_IDBASE			1000
#define SPACEMETER_POPUPMENU_COUNT			8

#define SPACEMETER_TOOLTIP_ID				1042

class CSpaceMeter : public CWindowImpl<CSpaceMeter,CWindow>
{
private:
	// These values are for rendering, they should never be altered manually.
	int m_iMeterPosition;
	int m_iMeterSegmentSpacing;
	TCHAR m_uiMeterSegments[SPACEMETER_METER_COUNT][SPACEMETER_METERTEXT_SIZE];

	HBRUSH m_hBarBorderBrush;
	HBRUSH m_hWarnBarBorderBrush;
	HBRUSH m_hFullBarBorderBrush;

	// Used for drawing a Vista meter.
	HTHEME m_hProgressTheme;

	// Horizontal indentation in pixels.
	int m_iHorIndent;

	int m_iDisplayMode;
	int m_iDrawState;

	// The following values can be changed from the outside using public functions.
	unsigned __int64 m_uiAllocatedSize;
	unsigned __int64 m_uiDiscSize;
	unsigned __int64 m_uiMeterSize;			// How many bytes does the meter display.

	// Tooltip.
	CToolTipCtrl m_ToolTip;
	ckcore::tstring m_ToolTipText;

	// Popup menu.
	HMENU m_hPopupMenu;
	void FillPopupMenu();

	void DrawBar(HDC hDC,RECT *pClientRect);
	void DrawFullBar(HDC hDC,RECT *pClientRect);
	void DrawMeter(HDC hDC,RECT *pClientRect,RECT *pBarRect);

	void UpdateMeter(int iClientWidth);
	void UpdateToolTip();

	bool m_bIsUpdatePending;

public:
	DECLARE_WND_CLASS(_T("ckSpaceMeter"));

	CSpaceMeter();
	~CSpaceMeter();

	//void SetDiskSize(unsigned __int64 uiDiskSize);
	void SetDiscSize(unsigned int uiDiscSize);
	void SetAllocatedSize(unsigned __int64 uiAllocatedSize);
	void IncreaseAllocatedSize(unsigned __int64 uiSize);
	void DecreaseAllocatedSize(unsigned __int64 uiSize);
	unsigned __int64 GetAllocatedSize();

	void SetDisplayMode(int iDisplayMode);
	
	void Initialize();

private:
#if _ATL_VER <= 0x0300
	BEGIN_MSG_MAP_EX(CSpaceMeter)
#else
	BEGIN_MSG_MAP(CSpaceMeter)
#endif
		MESSAGE_HANDLER(WM_CREATE,OnCreate)
		MESSAGE_HANDLER(WM_SIZE,OnSize)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkgnd)
		MESSAGE_HANDLER(WM_RBUTTONDOWN,OnRButtonDown)
		//MESSAGE_HANDLER(WM_MOUSEMOVE,OnMouseMove)
		MESSAGE_HANDLER(WMU_SPACE_METER_DELAYED_UPDATE,OnDelayedUpdate)
		MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseMove)
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO,OnGetDispInfo)

		COMMAND_RANGE_HANDLER_EX(SPACEMETER_POPUPMENU_IDBASE,SPACEMETER_POPUPMENU_IDBASE + SPACEMETER_POPUPMENU_COUNT - 1,OnPopupMenuClick)
    END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnRButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnDelayedUpdate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnGetDispInfo(int idCtrl,LPNMHDR pnmh,BOOL &bHandled);

	LRESULT OnPopupMenuClick(UINT uNotifyCode,int nID,CWindow wnd);

	void RequestDelayedUpdate();
};