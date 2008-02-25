/*
 * Copyright (C) 2006-2008 Christian Kindahl, christian dot kindahl at gmail dot com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "stdafx.h"
#include "Settings.h"
#include "MainFrm.h"
#include "ToolBarManager.h"
#include "../../Common/FileManager.h"

CLanguageSettings g_LanguageSettings;
CGlobalSettings g_GlobalSettings;
CDynamicSettings g_DynamicSettings;
CEraseSettings g_EraseSettings;
CFixateSettings g_FixateSettings;
CBurnImageSettings g_BurnImageSettings;
CProjectSettings g_ProjectSettings;
CCopyDiscSettings g_CopyDiscSettings;
CBurnAdvancedSettings g_BurnAdvancedSettings;
CSaveTracksSettings g_SaveTracksSettings;
CReadSettings g_ReadSettings;

bool CLanguageSettings::Save(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	pXML->AddElement(_T("Language"),_T(""),true);
		pXML->AddElement(_T("LanguageFile"),m_szLanguageFile);
	pXML->LeaveElement();

	return true;
}

bool CLanguageSettings::Load(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	if (!pXML->EnterElement(_T("Language")))
		return false;

	pXML->GetSafeElementData(_T("LanguageFile"),m_szLanguageFile,MAX_PATH - 1);

	// Calculate full path.
	TCHAR szFullPath[MAX_PATH];
	::GetModuleFileName(NULL,szFullPath,MAX_PATH - 1);
	ExtractFilePath(szFullPath);
	lstrcat(szFullPath,_T("Languages\\"));
	lstrcat(szFullPath,m_szLanguageFile);

	if (fs_fileexists(szFullPath))
	{
		m_pLNGProcessor = new CLNGProcessor();
		m_pLNGProcessor->Load(szFullPath);
	}

	pXML->LeaveElement();
	return true;
}

bool CGlobalSettings::Save(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	pXML->AddElement(_T("Global"),_T(""),true);
		pXML->AddElement(_T("AutoRunCheck"),m_bAutoRunCheck);
		pXML->AddElement(_T("AutoCheckBus"),m_bAutoCheckBus);
		pXML->AddElement(_T("Log"),m_bLog);
		pXML->AddElement(_T("RememberShell"),m_bRememberShell);
		pXML->AddElement(_T("CopyWarning"),m_bCopyWarning);
		pXML->AddElement(_T("RawImageInfo"),m_bRawImageInfo);
		pXML->AddElement(_T("CharSetWarning"),m_bCharSetWarning);
		pXML->AddElement(_T("WriteSpeedWarning"),m_bWriteSpeedWarning);
		pXML->AddElement(_T("CodecWarning"),m_bCodecWarning);
		pXML->AddElement(_T("FixateWarning"),m_bFixateWarning);
		pXML->AddElement(_T("Smoke"),m_bSmoke);
		pXML->AddElement(_T("GraceTime"),m_iGraceTime);
		pXML->AddElement(_T("FIFO"),m_iFIFOSize);
		if (m_iFIFOSize > FIFO_MAX)
			m_iFIFOSize = FIFO_MAX;
		else if (m_iFIFOSize < FIFO_MIN)
			m_iFIFOSize = FIFO_MIN;

		// Temporary folder.
		pXML->AddElement(_T("TempPath"),m_szTempPath);

		// Shell extension.
		pXML->AddElement(_T("ShellExtension"),_T(""),true);
			pXML->AddElementAttr(_T("submenu"),m_bShellExtSubMenu);
			pXML->AddElementAttr(_T("icons"),m_bShellExtIcon);
			pXML->AddElementAttr(_T("count"),(int)m_szShellExt.size());

			TCHAR szItemName[32];
			int iItemCount = 0;
			std::list<tstring>::iterator itNodeObject;
			for (itNodeObject = m_szShellExt.begin(); itNodeObject != m_szShellExt.end(); itNodeObject++)
			{
				lsprintf(szItemName,_T("Item%i"),iItemCount++);
				pXML->AddElement(szItemName,(*itNodeObject).c_str());
			}
		pXML->LeaveElement();
	pXML->LeaveElement();

	return true;
}

bool CGlobalSettings::Load(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	if (!pXML->EnterElement(_T("Global")))
		return false;

	pXML->GetSafeElementData(_T("AutoRunCheck"),&m_bAutoRunCheck);
	pXML->GetSafeElementData(_T("AutoCheckBus"),&m_bAutoCheckBus);
	pXML->GetSafeElementData(_T("Log"),&m_bLog);
	pXML->GetSafeElementData(_T("RememberShell"),&m_bRememberShell);
	pXML->GetSafeElementData(_T("CopyWarning"),&m_bCopyWarning);
	pXML->GetSafeElementData(_T("RawImageInfo"),&m_bRawImageInfo);
	pXML->GetSafeElementData(_T("CharSetWarning"),&m_bCharSetWarning);
	pXML->GetSafeElementData(_T("WriteSpeedWarning"),&m_bWriteSpeedWarning);
	pXML->GetSafeElementData(_T("CodecWarning"),&m_bCodecWarning);
	pXML->GetSafeElementData(_T("FixateWarning"),&m_bFixateWarning);
	pXML->GetSafeElementData(_T("Smoke"),&m_bSmoke);
	pXML->GetSafeElementData(_T("GraceTime"),&m_iGraceTime);
	pXML->GetSafeElementData(_T("FIFO"),&m_iFIFOSize);

	// Temporary folder.
	pXML->GetSafeElementData(_T("TempPath"),m_szTempPath,MAX_PATH - 1);
	if (!fs_directoryexists(m_szTempPath))
	{
		// If the folder does not exist, create it.
		if (fs_validpath(m_szTempPath))
		{
			IncludeTrailingBackslash(m_szTempPath);
			fs_createpath(m_szTempPath);
		}
		else
		{
			GetTempPath(MAX_PATH - 1,m_szTempPath);
		}
	}

	// Shell extension.
	if (pXML->EnterElement(_T("ShellExtension")))
	{
		pXML->GetSafeElementAttrValue(_T("submenu"),&m_bShellExtSubMenu);
		pXML->GetSafeElementAttrValue(_T("icons"),&m_bShellExtIcon);

		int iItemCount = 0;
		TCHAR szItemName[32];
		TCHAR szBuffer[128];

		pXML->GetSafeElementAttrValue(_T("count"),&iItemCount);
		m_szShellExt.clear();

		for (int i = 0; i < iItemCount; i++)
		{
			lsprintf(szItemName,_T("Item%i"),i);

			pXML->GetSafeElementData(szItemName,szBuffer,127);
			m_szShellExt.push_back(szBuffer);
		}

		pXML->LeaveElement();
	}

	pXML->LeaveElement();
	return true;
}

bool CDynamicSettings::Save(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	pXML->AddElement(_T("Dynamic"),_T(""),true);
		pXML->AddElement(_T("ProjectListViewStyle"),m_iPrjListViewStyle);
		pXML->AddElement(_T("ToolBar"),m_bViewToolBar,true);
			pXML->AddElementAttr(_T("text"),m_iToolBarText);
			pXML->AddElementAttr(_T("icon"),m_iToolBarIcon);
		pXML->LeaveElement();
		pXML->AddElement(_T("StatusBar"),m_bViewStatusBar);
		pXML->AddElement(_T("QuickHelp"),m_bViewQuickHelp);

		pXML->AddElement(_T("WindowLeft"),m_rcWindow.left);
		pXML->AddElement(_T("WindowRight"),m_rcWindow.right);
		pXML->AddElement(_T("WindowTop"),m_rcWindow.top);
		pXML->AddElement(_T("WindowBottom"),m_rcWindow.bottom);
		pXML->AddElement(_T("WindowMaximized"),m_bWinMaximized);

		pXML->AddElement(_T("ShellDir"),m_szShellDir);

		// Save the toolbar button configuration.
		g_ToolBarManager.Save(pXML);
	pXML->LeaveElement();

	return true;
}

bool CDynamicSettings::Load(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	if (!pXML->EnterElement(_T("Dynamic")))
		return false;

	pXML->GetSafeElementData(_T("ProjectListViewStyle"),&m_iPrjListViewStyle);

	if (pXML->EnterElement(_T("ToolBar")))
	{
		pXML->GetSafeElementData(&m_bViewToolBar);
		pXML->GetSafeElementAttrValue(_T("text"),&m_iToolBarText);
		pXML->GetSafeElementAttrValue(_T("icon"),&m_iToolBarIcon);
		pXML->LeaveElement();

		// Validate the values.
		if (m_iToolBarText > TOOLBAR_TEXT_DONTSHOW || m_iToolBarText < 0)
			m_iToolBarText = TOOLBAR_TEXT_DONTSHOW;
		if (m_iToolBarIcon > TOOLBAR_ICON_LARGE || m_iToolBarIcon < 0)
			m_iToolBarIcon = TOOLBAR_ICON_SMALL;
	}

	pXML->GetSafeElementData(_T("StatusBar"),&m_bViewStatusBar);
	pXML->GetSafeElementData(_T("QuickHelp"),&m_bViewQuickHelp);

	pXML->GetSafeElementData(_T("WindowLeft"),&m_rcWindow.left);
	pXML->GetSafeElementData(_T("WindowRight"),&m_rcWindow.right);
	pXML->GetSafeElementData(_T("WindowTop"),&m_rcWindow.top);
	pXML->GetSafeElementData(_T("WindowBottom"),&m_rcWindow.bottom);
	pXML->GetSafeElementData(_T("WindowMaximized"),&m_bWinMaximized);

	pXML->GetSafeElementData(_T("ShellDir"),m_szShellDir,MAX_PATH - 1);

	// Save the toolbar button configuration.
	g_ToolBarManager.Load(pXML);

	pXML->LeaveElement();
	return true;
}

void CDynamicSettings::Apply()
{
	// Apply the project list view style.
	switch (m_iPrjListViewStyle)
	{
		case LISTVIEWSTYLE_LARGEICONS:
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_LARGEICONS,MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_SMALLICONS,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_LIST,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_DETAILS,MF_BYCOMMAND | MF_UNCHECKED);

			g_MainFrame.UISetCheck(ID_VIEW_LARGEICONS,true);
			g_MainFrame.UISetCheck(ID_VIEW_SMALLICONS,false);
			g_MainFrame.UISetCheck(ID_VIEW_LIST,false);
			g_MainFrame.UISetCheck(ID_VIEW_DETAILS,false);
			break;

		case LISTVIEWSTYLE_SMALLICONS:
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_LARGEICONS,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_SMALLICONS,MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_LIST,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_DETAILS,MF_BYCOMMAND | MF_UNCHECKED);

			g_MainFrame.UISetCheck(ID_VIEW_LARGEICONS,false);
			g_MainFrame.UISetCheck(ID_VIEW_SMALLICONS,true);
			g_MainFrame.UISetCheck(ID_VIEW_LIST,false);
			g_MainFrame.UISetCheck(ID_VIEW_DETAILS,false);
			break;

		case LISTVIEWSTYLE_LIST:
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_LARGEICONS,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_SMALLICONS,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_LIST,MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_DETAILS,MF_BYCOMMAND | MF_UNCHECKED);

			g_MainFrame.UISetCheck(ID_VIEW_LARGEICONS,false);
			g_MainFrame.UISetCheck(ID_VIEW_SMALLICONS,false);
			g_MainFrame.UISetCheck(ID_VIEW_LIST,true);
			g_MainFrame.UISetCheck(ID_VIEW_DETAILS,false);
			break;

		case LISTVIEWSTYLE_DETAILS:
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_LARGEICONS,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_SMALLICONS,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_LIST,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_MainFrame.m_hProjListNoSelMenu,ID_VIEW_DETAILS,MF_BYCOMMAND | MF_CHECKED);

			g_MainFrame.UISetCheck(ID_VIEW_LARGEICONS,false);
			g_MainFrame.UISetCheck(ID_VIEW_SMALLICONS,false);
			g_MainFrame.UISetCheck(ID_VIEW_LIST,false);
			g_MainFrame.UISetCheck(ID_VIEW_DETAILS,true);
			break;
	};

	g_MainFrame.m_ProjectListView.SetViewStyle(m_iPrjListViewStyle);
}

bool CEraseSettings::Save(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	pXML->AddElement(_T("Erase"),_T(""),true);
		pXML->AddElement(_T("Mode"),m_iMode);
		pXML->AddElement(_T("Force"),m_bForce);
		pXML->AddElement(_T("Eject"),m_bEject);
		pXML->AddElement(_T("Simulate"),m_bSimulate);
	pXML->LeaveElement();

	return true;
}

bool CEraseSettings::Load(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	if (!pXML->EnterElement(_T("Erase")))
		return false;

	pXML->GetSafeElementData(_T("Mode"),&m_iMode);
	pXML->GetSafeElementData(_T("Force"),&m_bForce);
	pXML->GetSafeElementData(_T("Eject"),&m_bEject);
	pXML->GetSafeElementData(_T("Simulate"),&m_bSimulate);

	pXML->LeaveElement();
	return true;
}

bool CFixateSettings::Save(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	pXML->AddElement(_T("Fixate"),_T(""),true);
		pXML->AddElement(_T("Eject"),m_bEject);
		pXML->AddElement(_T("Simulate"),m_bSimulate);
	pXML->LeaveElement();

	return true;
}

bool CFixateSettings::Load(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	if (!pXML->EnterElement(_T("Fixate")))
		return false;

	pXML->GetSafeElementData(_T("Eject"),&m_bEject);
	pXML->GetSafeElementData(_T("Simulate"),&m_bSimulate);

	pXML->LeaveElement();
	return true;
}

bool CBurnImageSettings::Save(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	pXML->AddElement(_T("BurnImage"),_T(""),true);
		pXML->AddElement(_T("OnFly"),m_bOnFly);
		pXML->AddElement(_T("Verify"),m_bVerify);
		pXML->AddElement(_T("Eject"),m_bEject);
		pXML->AddElement(_T("Simulate"),m_bSimulate);
		pXML->AddElement(_T("BUP"),m_bBUP);
		pXML->AddElement(_T("PadTracks"),m_bPadTracks);
		pXML->AddElement(_T("Fixate"),m_bFixate);
	pXML->LeaveElement();

	return true;
}

bool CBurnImageSettings::Load(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	if (!pXML->EnterElement(_T("BurnImage")))
		return false;

	pXML->GetSafeElementData(_T("OnFly"),&m_bOnFly);
	pXML->GetSafeElementData(_T("Verify"),&m_bVerify);
	pXML->GetSafeElementData(_T("Eject"),&m_bEject);
	pXML->GetSafeElementData(_T("Simulate"),&m_bSimulate);
	pXML->GetSafeElementData(_T("BUP"),&m_bBUP);
	pXML->GetSafeElementData(_T("PadTracks"),&m_bPadTracks);
	pXML->GetSafeElementData(_T("Fixate"),&m_bFixate);

	pXML->LeaveElement();
	return true;
}

bool CProjectSettings::Save(CXMLProcessor *pXML)
{
	return false;
}

bool CProjectSettings::Load(CXMLProcessor *pXML)
{
	return false;
}

bool CCopyDiscSettings::Save(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	pXML->AddElement(_T("CopyDisc"),_T(""),true);
		pXML->AddElement(_T("OnFly"),m_bOnFly);
		pXML->AddElement(_T("Clone"),m_bClone);
	pXML->LeaveElement();

	return true;
}

bool CCopyDiscSettings::Load(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	if (!pXML->EnterElement(_T("CopyDisc")))
		return false;

	pXML->GetSafeElementData(_T("OnFly"),&m_bOnFly);
	pXML->GetSafeElementData(_T("Clone"),&m_bClone);

	pXML->LeaveElement();
	return true;
}

bool CBurnAdvancedSettings::Save(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	pXML->AddElement(_T("BurnAdvanced"),_T(""),true);
		pXML->AddElement(_T("Overburn"),m_bOverburn);
		pXML->AddElement(_T("Swab"),m_bSwab);
		pXML->AddElement(_T("IgnoreSize"),m_bIgnoreSize);
		pXML->AddElement(_T("Immed"),m_bImmed);
		pXML->AddElement(_T("AudioMaster"),m_bAudioMaster);
		pXML->AddElement(_T("ForceSpeed"),m_bForceSpeed);
		pXML->AddElement(_T("VariRec"),m_bVariRec,true);
			pXML->AddElementAttr(_T("value"),m_iVariRec);
		pXML->LeaveElement();
	pXML->LeaveElement();

	return true;
}

bool CBurnAdvancedSettings::Load(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	if (!pXML->EnterElement(_T("BurnAdvanced")))
		return false;

	pXML->GetSafeElementData(_T("Overburn"),&m_bOverburn);
	pXML->GetSafeElementData(_T("Swab"),&m_bSwab);
	pXML->GetSafeElementData(_T("IgnoreSize"),&m_bIgnoreSize);
	pXML->GetSafeElementData(_T("Immed"),&m_bImmed);
	pXML->GetSafeElementData(_T("AudioMaster"),&m_bAudioMaster);
	pXML->GetSafeElementData(_T("ForceSpeed"),&m_bForceSpeed);

	if (pXML->EnterElement(_T("VariRec")))
	{
		pXML->GetSafeElementData(&m_bVariRec);
		pXML->GetSafeElementAttrValue(_T("value"),&m_iVariRec);
		
		pXML->LeaveElement();
	}

	pXML->LeaveElement();
	return true;
}

bool CSaveTracksSettings::Save(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	pXML->AddElement(_T("SaveTracks"),_T(""),true);
		pXML->AddElement(_T("Target"),m_szTarget);
	pXML->LeaveElement();

	return true;
}

bool CSaveTracksSettings::Load(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	if (!pXML->EnterElement(_T("SaveTracks")))
		return false;

	pXML->GetSafeElementData(_T("Target"),m_szTarget,MAX_PATH - 1);

	pXML->LeaveElement();
	return true;
}

bool CReadSettings::Save(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	pXML->AddElement(_T("Read"),_T(""),true);
		pXML->AddElement(_T("IgnoreErr"),m_bIgnoreErr);
		pXML->AddElement(_T("Clone"),m_bClone);
	pXML->LeaveElement();

	return true;
}

bool CReadSettings::Load(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	if (!pXML->EnterElement(_T("Read")))
		return false;

	pXML->GetSafeElementData(_T("IgnoreErr"),&m_bIgnoreErr);
	pXML->GetSafeElementData(_T("Clone"),&m_bClone);

	pXML->LeaveElement();
	return true;
}
