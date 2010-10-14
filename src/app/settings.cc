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

#include "stdafx.hh"
#include <ckcore/directory.hh>
#include "main_frm.hh"
#include "toolbar_manager.hh"
#include "settings.hh"

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

bool CLanguageSettings::Save(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	pXml->AddElement(_T("Language"),_T(""),true);
		pXml->AddElement(_T("LanguageFile"),m_szLanguageFile);
	pXml->LeaveElement();

	return true;
}

bool CLanguageSettings::Load(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	if (!pXml->EnterElement(_T("Language")))
		return false;

	pXml->GetSafeElementData(_T("LanguageFile"),m_szLanguageFile,MAX_PATH - 1);

	// Calculate full path.
	TCHAR szFullPath[MAX_PATH];
	::GetModuleFileName(NULL,szFullPath,MAX_PATH - 1);
	ExtractFilePath(szFullPath);
	lstrcat(szFullPath,_T("languages\\"));
	lstrcat(szFullPath,m_szLanguageFile);

	if (ckcore::File::exist(szFullPath))
	{
		m_pLngProcessor = new CLngProcessor(szFullPath);
		m_pLngProcessor->Load();
	}

	pXml->LeaveElement();
	return true;
}

bool CGlobalSettings::Save(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	pXml->AddElement(_T("Global"),_T(""),true);
		pXml->AddElement(_T("AutoRunCheck"),m_bAutoRunCheck);
		pXml->AddElement(_T("Log"),m_bLog);
		pXml->AddElement(_T("RememberShell"),m_bRememberShell);
		pXml->AddElement(_T("CopyWarning"),m_bCopyWarning);
		pXml->AddElement(_T("RawImageInfo"),m_bRawImageInfo);
		pXml->AddElement(_T("CharSetWarning"),m_bCharSetWarning);
		pXml->AddElement(_T("WriteSpeedWarning"),m_bWriteSpeedWarning);
		pXml->AddElement(_T("CodecWarning"),m_bCodecWarning);
		pXml->AddElement(_T("FixateWarning"),m_bFixateWarning);
		pXml->AddElement(_T("NoDevWarning"),m_bNoDevWarning);
		pXml->AddElement(_T("Smoke"),m_bSmoke);
		pXml->AddElement(_T("Wizard"),m_bShowWizard);
		pXml->AddElement(_T("GraceTime"),m_iGraceTime);
		pXml->AddElement(_T("FIFO"),m_iFIFOSize);
		if (m_iFIFOSize > FIFO_MAX)
			m_iFIFOSize = FIFO_MAX;
		else if (m_iFIFOSize < FIFO_MIN)
			m_iFIFOSize = FIFO_MIN;

		// Temporary folder.
		pXml->AddElement(_T("TempPath"),m_szTempPath);

		// Shell extension.
		pXml->AddElement(_T("ShellExtension"),_T(""),true);
			pXml->AddElementAttr(_T("submenu"),m_bShellExtSubMenu);
			pXml->AddElementAttr(_T("icons"),m_bShellExtIcon);
			pXml->AddElementAttr(_T("count"),(int)m_szShellExt.size());

			TCHAR szItemName[32];
			int iItemCount = 0;
			std::list<tstring>::iterator itNodeObject;
			for (itNodeObject = m_szShellExt.begin(); itNodeObject != m_szShellExt.end(); itNodeObject++)
			{
				lsprintf(szItemName,_T("Item%i"),iItemCount++);
				pXml->AddElement(szItemName,(*itNodeObject).c_str());
			}
		pXml->LeaveElement();
	pXml->LeaveElement();

	return true;
}

bool CGlobalSettings::Load(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	if (!pXml->EnterElement(_T("Global")))
		return false;

	pXml->GetSafeElementData(_T("AutoRunCheck"),&m_bAutoRunCheck);
	pXml->GetSafeElementData(_T("Log"),&m_bLog);
	pXml->GetSafeElementData(_T("RememberShell"),&m_bRememberShell);
	pXml->GetSafeElementData(_T("CopyWarning"),&m_bCopyWarning);
	pXml->GetSafeElementData(_T("RawImageInfo"),&m_bRawImageInfo);
	pXml->GetSafeElementData(_T("CharSetWarning"),&m_bCharSetWarning);
	pXml->GetSafeElementData(_T("WriteSpeedWarning"),&m_bWriteSpeedWarning);
	pXml->GetSafeElementData(_T("CodecWarning"),&m_bCodecWarning);
	pXml->GetSafeElementData(_T("FixateWarning"),&m_bFixateWarning);
	pXml->GetSafeElementData(_T("NoDevWarning"),&m_bNoDevWarning);
	pXml->GetSafeElementData(_T("Smoke"),&m_bSmoke);
	pXml->GetSafeElementData(_T("Wizard"),&m_bShowWizard);
	pXml->GetSafeElementData(_T("GraceTime"),&m_iGraceTime);
	pXml->GetSafeElementData(_T("FIFO"),&m_iFIFOSize);

	// Temporary folder.
	pXml->GetSafeElementData(_T("TempPath"),m_szTempPath,MAX_PATH - 1);
	if (!ckcore::Directory::exist(m_szTempPath))
	{
		// If the folder does not exist, create it.
		if (m_szTempPath == NULL || lstrlen(m_szTempPath) < 3 || m_szTempPath[1] != ':')
		{
			IncludeTrailingBackslash(m_szTempPath);
			ckcore::Directory::create(m_szTempPath);
		}
		else
		{
			GetTempPath(MAX_PATH - 1,m_szTempPath);
		}
	}

	// Shell extension.
	if (pXml->EnterElement(_T("ShellExtension")))
	{
		pXml->GetSafeElementAttrValue(_T("submenu"),&m_bShellExtSubMenu);
		pXml->GetSafeElementAttrValue(_T("icons"),&m_bShellExtIcon);

		int iItemCount = 0;
		TCHAR szItemName[32];
		TCHAR szBuffer[128];

		pXml->GetSafeElementAttrValue(_T("count"),&iItemCount);
		m_szShellExt.clear();

		for (int i = 0; i < iItemCount; i++)
		{
			lsprintf(szItemName,_T("Item%i"),i);

			pXml->GetSafeElementData(szItemName,szBuffer,127);
			m_szShellExt.push_back(szBuffer);
		}

		pXml->LeaveElement();
	}

	pXml->LeaveElement();
	return true;
}

bool CDynamicSettings::Save(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	pXml->AddElement(_T("Dynamic"),_T(""),true);
		pXml->AddElement(_T("ProjectListViewStyle"),m_iPrjListViewStyle);
		pXml->AddElement(_T("ToolBar"),m_bViewToolBar,true);
			pXml->AddElementAttr(_T("text"),m_iToolBarText);
			pXml->AddElementAttr(_T("icon"),m_iToolBarIcon);
		pXml->LeaveElement();
		pXml->AddElement(_T("StatusBar"),m_bViewStatusBar);

		pXml->AddElement(_T("WindowLeft"),m_rcWindow.left);
		pXml->AddElement(_T("WindowRight"),m_rcWindow.right);
		pXml->AddElement(_T("WindowTop"),m_rcWindow.top);
		pXml->AddElement(_T("WindowBottom"),m_rcWindow.bottom);
		pXml->AddElement(_T("WindowMaximized"),m_bWinMaximized);

		pXml->AddElement(_T("ShellDir"),m_szShellDir);

		// Save the toolbar button configuration.
		g_ToolBarManager.Save(pXml);
	pXml->LeaveElement();

	return true;
}

bool CDynamicSettings::Load(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	if (!pXml->EnterElement(_T("Dynamic")))
		return false;

	pXml->GetSafeElementData(_T("ProjectListViewStyle"),&m_iPrjListViewStyle);

	if (pXml->EnterElement(_T("ToolBar")))
	{
		pXml->GetSafeElementData(&m_bViewToolBar);
		pXml->GetSafeElementAttrValue(_T("text"),&m_iToolBarText);
		pXml->GetSafeElementAttrValue(_T("icon"),&m_iToolBarIcon);
		pXml->LeaveElement();

		// Validate the values.
		if (m_iToolBarText > TOOLBAR_TEXT_DONTSHOW || m_iToolBarText < 0)
			m_iToolBarText = TOOLBAR_TEXT_DONTSHOW;
		if (m_iToolBarIcon > TOOLBAR_ICON_LARGE || m_iToolBarIcon < 0)
			m_iToolBarIcon = TOOLBAR_ICON_SMALL;
	}

	pXml->GetSafeElementData(_T("StatusBar"),&m_bViewStatusBar);

	pXml->GetSafeElementData(_T("WindowLeft"),&m_rcWindow.left);
	pXml->GetSafeElementData(_T("WindowRight"),&m_rcWindow.right);
	pXml->GetSafeElementData(_T("WindowTop"),&m_rcWindow.top);
	pXml->GetSafeElementData(_T("WindowBottom"),&m_rcWindow.bottom);
	pXml->GetSafeElementData(_T("WindowMaximized"),&m_bWinMaximized);

	pXml->GetSafeElementData(_T("ShellDir"),m_szShellDir,MAX_PATH - 1);

	// Save the toolbar button configuration.
	g_ToolBarManager.Load(pXml);

	pXml->LeaveElement();
	return true;
}

void CDynamicSettings::Apply()
{
	// Apply the project list view style.
	switch (m_iPrjListViewStyle)
	{
		case LISTVIEWSTYLE_LARGEICONS:
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_LARGEICONS,MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_SMALLICONS,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_LIST,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_DETAILS,MF_BYCOMMAND | MF_UNCHECKED);

			g_pMainFrame->UISetCheck(ID_VIEW_LARGEICONS,true);
			g_pMainFrame->UISetCheck(ID_VIEW_SMALLICONS,false);
			g_pMainFrame->UISetCheck(ID_VIEW_LIST,false);
			g_pMainFrame->UISetCheck(ID_VIEW_DETAILS,false);
			break;

		case LISTVIEWSTYLE_SMALLICONS:
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_LARGEICONS,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_SMALLICONS,MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_LIST,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_DETAILS,MF_BYCOMMAND | MF_UNCHECKED);

			g_pMainFrame->UISetCheck(ID_VIEW_LARGEICONS,false);
			g_pMainFrame->UISetCheck(ID_VIEW_SMALLICONS,true);
			g_pMainFrame->UISetCheck(ID_VIEW_LIST,false);
			g_pMainFrame->UISetCheck(ID_VIEW_DETAILS,false);
			break;

		case LISTVIEWSTYLE_LIST:
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_LARGEICONS,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_SMALLICONS,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_LIST,MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_DETAILS,MF_BYCOMMAND | MF_UNCHECKED);

			g_pMainFrame->UISetCheck(ID_VIEW_LARGEICONS,false);
			g_pMainFrame->UISetCheck(ID_VIEW_SMALLICONS,false);
			g_pMainFrame->UISetCheck(ID_VIEW_LIST,true);
			g_pMainFrame->UISetCheck(ID_VIEW_DETAILS,false);
			break;

		case LISTVIEWSTYLE_DETAILS:
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_LARGEICONS,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_SMALLICONS,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_LIST,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(g_pMainFrame->m_hProjListNoSelMenu,ID_VIEW_DETAILS,MF_BYCOMMAND | MF_CHECKED);

			g_pMainFrame->UISetCheck(ID_VIEW_LARGEICONS,false);
			g_pMainFrame->UISetCheck(ID_VIEW_SMALLICONS,false);
			g_pMainFrame->UISetCheck(ID_VIEW_LIST,false);
			g_pMainFrame->UISetCheck(ID_VIEW_DETAILS,true);
			break;
	};

	g_pMainFrame->m_ProjectListView.SetViewStyle(m_iPrjListViewStyle);
}

bool CEraseSettings::Save(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	pXml->AddElement(_T("Erase"),_T(""),true);
		pXml->AddElement(_T("Mode"),m_iMode);
		pXml->AddElement(_T("Force"),m_bForce);
		pXml->AddElement(_T("Eject"),m_bEject);
		pXml->AddElement(_T("Simulate"),m_bSimulate);
	pXml->LeaveElement();

	return true;
}

bool CEraseSettings::Load(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	if (!pXml->EnterElement(_T("Erase")))
		return false;

	pXml->GetSafeElementData(_T("Mode"),&m_iMode);
	pXml->GetSafeElementData(_T("Force"),&m_bForce);
	pXml->GetSafeElementData(_T("Eject"),&m_bEject);
	pXml->GetSafeElementData(_T("Simulate"),&m_bSimulate);

	pXml->LeaveElement();
	return true;
}

bool CFixateSettings::Save(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	pXml->AddElement(_T("Fixate"),_T(""),true);
		pXml->AddElement(_T("Eject"),m_bEject);
		pXml->AddElement(_T("Simulate"),m_bSimulate);
	pXml->LeaveElement();

	return true;
}

bool CFixateSettings::Load(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	if (!pXml->EnterElement(_T("Fixate")))
		return false;

	pXml->GetSafeElementData(_T("Eject"),&m_bEject);
	pXml->GetSafeElementData(_T("Simulate"),&m_bSimulate);

	pXml->LeaveElement();
	return true;
}

bool CBurnImageSettings::Save(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	pXml->AddElement(_T("BurnImage"),_T(""),true);
		pXml->AddElement(_T("OnFly"),m_bOnFly);
		pXml->AddElement(_T("Verify"),m_bVerify);
		pXml->AddElement(_T("Eject"),m_bEject);
		pXml->AddElement(_T("Simulate"),m_bSimulate);
		pXml->AddElement(_T("BUP"),m_bBUP);
		pXml->AddElement(_T("PadTracks"),m_bPadTracks);
		pXml->AddElement(_T("Fixate"),m_bFixate);
	pXml->LeaveElement();

	return true;
}

bool CBurnImageSettings::Load(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	if (!pXml->EnterElement(_T("BurnImage")))
		return false;

	pXml->GetSafeElementData(_T("OnFly"),&m_bOnFly);
	pXml->GetSafeElementData(_T("Verify"),&m_bVerify);
	pXml->GetSafeElementData(_T("Eject"),&m_bEject);
	pXml->GetSafeElementData(_T("Simulate"),&m_bSimulate);
	pXml->GetSafeElementData(_T("BUP"),&m_bBUP);
	pXml->GetSafeElementData(_T("PadTracks"),&m_bPadTracks);
	pXml->GetSafeElementData(_T("Fixate"),&m_bFixate);

	pXml->LeaveElement();
	return true;
}

bool CProjectSettings::Save(CXmlProcessor *pXml)
{
	return false;
}

bool CProjectSettings::Load(CXmlProcessor *pXml)
{
	return false;
}

bool CCopyDiscSettings::Save(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	pXml->AddElement(_T("CopyDisc"),_T(""),true);
		pXml->AddElement(_T("OnFly"),m_bOnFly);
		pXml->AddElement(_T("Clone"),m_bClone);
	pXml->LeaveElement();

	return true;
}

bool CCopyDiscSettings::Load(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	if (!pXml->EnterElement(_T("CopyDisc")))
		return false;

	pXml->GetSafeElementData(_T("OnFly"),&m_bOnFly);
	pXml->GetSafeElementData(_T("Clone"),&m_bClone);

	pXml->LeaveElement();
	return true;
}

bool CBurnAdvancedSettings::Save(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	pXml->AddElement(_T("BurnAdvanced"),_T(""),true);
		pXml->AddElement(_T("Overburn"),m_bOverburn);
		pXml->AddElement(_T("Swab"),m_bSwab);
		pXml->AddElement(_T("IgnoreSize"),m_bIgnoreSize);
		pXml->AddElement(_T("Immed"),m_bImmed);
		pXml->AddElement(_T("AudioMaster"),m_bAudioMaster);
		pXml->AddElement(_T("ForceSpeed"),m_bForceSpeed);
		pXml->AddElement(_T("VariRec"),m_bVariRec,true);
			pXml->AddElementAttr(_T("value"),m_iVariRec);
		pXml->LeaveElement();
	pXml->LeaveElement();

	return true;
}

bool CBurnAdvancedSettings::Load(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	if (!pXml->EnterElement(_T("BurnAdvanced")))
		return false;

	pXml->GetSafeElementData(_T("Overburn"),&m_bOverburn);
	pXml->GetSafeElementData(_T("Swab"),&m_bSwab);
	pXml->GetSafeElementData(_T("IgnoreSize"),&m_bIgnoreSize);
	pXml->GetSafeElementData(_T("Immed"),&m_bImmed);
	pXml->GetSafeElementData(_T("AudioMaster"),&m_bAudioMaster);
	pXml->GetSafeElementData(_T("ForceSpeed"),&m_bForceSpeed);

	if (pXml->EnterElement(_T("VariRec")))
	{
		pXml->GetSafeElementData(&m_bVariRec);
		pXml->GetSafeElementAttrValue(_T("value"),&m_iVariRec);
		
		pXml->LeaveElement();
	}

	pXml->LeaveElement();
	return true;
}

bool CSaveTracksSettings::Save(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	pXml->AddElement(_T("SaveTracks"),_T(""),true);
		pXml->AddElement(_T("Target"),m_szTarget);
	pXml->LeaveElement();

	return true;
}

bool CSaveTracksSettings::Load(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	if (!pXml->EnterElement(_T("SaveTracks")))
		return false;

	pXml->GetSafeElementData(_T("Target"),m_szTarget,MAX_PATH - 1);

	pXml->LeaveElement();
	return true;
}

bool CReadSettings::Save(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	pXml->AddElement(_T("Read"),_T(""),true);
		pXml->AddElement(_T("IgnoreErr"),m_bIgnoreErr);
		pXml->AddElement(_T("Clone"),m_bClone);
	pXml->LeaveElement();

	return true;
}

bool CReadSettings::Load(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	if (!pXml->EnterElement(_T("Read")))
		return false;

	pXml->GetSafeElementData(_T("IgnoreErr"),&m_bIgnoreErr);
	pXml->GetSafeElementData(_T("Clone"),&m_bClone);

	pXml->LeaveElement();
	return true;
}