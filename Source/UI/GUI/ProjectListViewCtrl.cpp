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
#include "ProjectListViewCtrl.h"
#include "ProjectManager.h"
#include "Settings.h"
#include "MainFrm.h"
#include "StringTable.h"
#include "ProjectDropSource.h"
#include "ProjectDataObject.h"

CProjectListViewDropTarget::CProjectListViewDropTarget(CProjectListViewCtrl *pHost)
{
	m_pHost = pHost;
}

bool CProjectListViewDropTarget::OnDragOver(POINTL ptCursor)
{
	LVHITTESTINFO lvHit;
	lvHit.pt.x = ptCursor.x;
	lvHit.pt.y = ptCursor.y;

	ScreenToClient(m_pHost->m_hWnd,&lvHit.pt);

	// See we're dragging above a folder, in that case highlight it.
	int iTarget = m_pHost->HitTest(&lvHit);
	if (iTarget != -1 && (((CItemData *)m_pHost->GetItemData(iTarget))->ucFlags & PROJECTITEM_FLAG_ISFOLDER))
		m_pHost->SelectDropTarget(iTarget);
	else
		m_pHost->SelectDropTarget(-1);

	return true;
}

bool CProjectListViewDropTarget::OnDrop(POINTL ptCursor,IDataObject *pDataObject)
{
	LVHITTESTINFO lvHit;
	lvHit.pt.x = ptCursor.x;
	lvHit.pt.y = ptCursor.y;

	ScreenToClient(m_pHost->m_hWnd,&lvHit.pt);

	CProjectNode *pTargetNode = NULL;

	int iTarget = m_pHost->HitTest(&lvHit);
	if (iTarget != -1 && (((CItemData *)m_pHost->GetItemData(iTarget))->ucFlags & PROJECTITEM_FLAG_ISFOLDER))
	{
		m_pHost->SelectDropTarget(-1);

		pTargetNode = g_TreeManager.ResolveNode(g_TreeManager.GetCurrentNode(),
			(CItemData *)m_pHost->GetItemData(iTarget));
	}

	// Handle the data.
	HandleDropData(pDataObject,pTargetNode,ptCursor);
	return true;
}

void CProjectListViewDropTarget::OnDragLeave()
{
	m_pHost->SelectDropTarget(-1);
}

bool CProjectListViewDropTarget::HandleDropData(IDataObject *pDataObject,CProjectNode *pTargetNode,
												POINTL ptCursor)
{
	// Construct a FORMATETC object.
	FORMATETC FormatEtc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM StgMedium;

	if (pDataObject->QueryGetData(&FormatEtc) == S_OK)
	{
		if (pDataObject->GetData(&FormatEtc,&StgMedium) == S_OK)
		{
			// Prepare a project file transaction.
			CProjectManager::CFileTransaction Transaction;

			HDROP hDrop = (HDROP)GlobalLock(StgMedium.hGlobal);

			unsigned int uiNumFiles = DragQueryFile(hDrop,0xFFFFFFFF,NULL,NULL);
			TCHAR szFullName[MAX_PATH];

			for (unsigned int i = 0; i < uiNumFiles; i++)
			{
				// Add each file to the project.
				if (DragQueryFile(hDrop,i,szFullName,MAX_PATH - 1))
					Transaction.AddFile(szFullName,pTargetNode);
			}

			GlobalUnlock(StgMedium.hGlobal);
			ReleaseStgMedium(&StgMedium);
		}
	}
	else
	{
		// Move to another folder if possible, otherwise rearrange in the list.
		if (pTargetNode != NULL)
		{
			FormatEtc.cfFormat = m_uiClipFormat;
			if (pDataObject->QueryGetData(&FormatEtc) == S_OK)
			{
				if (pDataObject->GetData(&FormatEtc,&StgMedium) == S_OK)
				{
					// Prepare a project file transaction.
					CProjectManager::CFileTransaction Transaction;

					CProjectDropData *pDropData = (CProjectDropData *)GlobalLock(StgMedium.hGlobal);
					if (pDropData->m_pParent == NULL)
						return true;

					SIZE_T uiNumFiles = (GlobalSize(StgMedium.hGlobal) - sizeof(CProjectDropData)) / sizeof(CItemData *);

					for (SIZE_T i = 0; i < uiNumFiles; i++)
					{
						CItemData *pItemData = pDropData->m_ppData[i];

						Transaction.MoveFile(pDropData->m_pParent,pItemData,pTargetNode);
					}

					GlobalUnlock(StgMedium.hGlobal);
					ReleaseStgMedium(&StgMedium);
				}
			}
		}
		else
		{
			/*
			*/
			// Determine the dropped item.
			LVHITTESTINFO lvHit;
			lvHit.pt.x = ptCursor.x;
			lvHit.pt.y = ptCursor.y;

			ScreenToClient(m_pHost->m_hWnd,&lvHit.pt);

			ListView_HitTest(m_pHost->m_hWnd,&lvHit);

			// Not inside the list view?
			if (lvHit.iItem == -1)
				return true;

			// Make sure that we can't drop a file above a folder.
			CItemData *pDropItemData = (CItemData *)m_pHost->GetItemData(lvHit.iItem);

			while (pDropItemData->ucFlags & PROJECTITEM_FLAG_ISFOLDER)
				pDropItemData = (CItemData *)m_pHost->GetItemData(++lvHit.iItem);

			CProjectNode *pCurrentNode = g_TreeManager.GetCurrentNode();

			// Locate the dropped item (wee need an iterator so we know where to insert
			// the dropped files.
			std::list <CItemData *>::iterator itFileObject;

			for (itFileObject = pCurrentNode->m_Files.begin(); itFileObject !=
				pCurrentNode->m_Files.end(); itFileObject++)
			{
				if (*itFileObject == pDropItemData)
					break;
			}

			// Move the selected items.
			int iItemIndex = -1;
			iItemIndex = m_pHost->GetNextItem(iItemIndex,LVNI_SELECTED);

			while (iItemIndex != -1)
			{
				// Move the internal item data pointer.
				CItemData *pItemData = (CItemData *)m_pHost->GetItemData(iItemIndex);

				pCurrentNode->m_Files.remove(pItemData);
				pCurrentNode->m_Files.insert(itFileObject,pItemData);

				// Move the actual list item.
				LVITEM lvi = { 0 };
				lvi.iItem = iItemIndex;
				lvi.iSubItem = 0;
				lvi.pszText = LPSTR_TEXTCALLBACK;
				lvi.stateMask = ~LVIS_SELECTED;
				lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
				m_pHost->GetItem(&lvi);

				lvi.iItem = lvHit.iItem;

				// Re-insert the item.
				int iResult = m_pHost->InsertItem(&lvi);
				if (lvi.iItem < iItemIndex)
					lvHit.iItem++;
				if (iResult <= iItemIndex)
					iItemIndex++;

				// Delete the old item.
				m_pHost->DeleteItem(iItemIndex);

				iItemIndex = m_pHost->GetNextItem(-1,LVNI_SELECTED);
			}
			/*
			*/
		}
	}

	return true;
}
/*
*/

CProjectListViewCtrl::CProjectListViewCtrl()
{
	m_hDragImageList = NULL;

	// Drop target.
	m_pDropTarget = new CProjectListViewDropTarget(this);
	CoLockObjectExternal(m_pDropTarget,TRUE,FALSE);
}

CProjectListViewCtrl::~CProjectListViewCtrl()
{
	// Drop target.
	if (m_pDropTarget != NULL)
	{
		CoLockObjectExternal(m_pDropTarget,FALSE,TRUE);

		m_pDropTarget->Release();
		m_pDropTarget = NULL;
	}

	if (m_hDragImageList != NULL)
		ImageList_Destroy(m_hDragImageList);
}

LRESULT CProjectListViewCtrl::OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	LRESULT lResult = DefWindowProc(uMsg,wParam,lParam);

	if (wParam == VK_RETURN)
	{
		if (GetSelectedCount() > 0)
		{
			// This is not safe, I know.
			BOOL bDummy;
			g_MainFrame.OnPLVDblClk(IDC_PROJECTLISTVIEW,NULL,bDummy);
		}
	}

	return lResult;
}

LRESULT CProjectListViewCtrl::OnDropFiles(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	HDROP hDrop = (HDROP)wParam;

	POINT ptDrop;
	TCHAR szFullName[MAX_PATH];

	if (DragQueryPoint(hDrop,&ptDrop) > 0)
	{
		// Prepare a project file transaction.
		CProjectManager::CFileTransaction Transaction;

		unsigned int uiNumFiles = DragQueryFile(hDrop,0xFFFFFFFF,NULL,NULL);

		for (unsigned int i = 0; i < uiNumFiles; i++)
		{
			if (DragQueryFile(hDrop,i,szFullName,MAX_PATH - 1))
				Transaction.AddFile(szFullName);
		}
	}

	DragFinish(hDrop);

	bHandled = false;
	return 0;
}

LRESULT CProjectListViewCtrl::OnSetFocus(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	g_ProjectManager.ListSetActive();
	g_ProjectManager.NotifyListSelChanged(GetSelectedCount());

	bHandled = false;
	return 0;
}

LRESULT CProjectListViewCtrl::OnCustomDraw(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	LPNMLVCUSTOMDRAW lpNMCustomDraw = (LPNMLVCUSTOMDRAW)lParam;

	switch (lpNMCustomDraw->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			return CDRF_NOTIFYITEMDRAW;

		case CDDS_ITEMPREPAINT:
			CItemData *pItemData = (CItemData *)lpNMCustomDraw->nmcd.lItemlParam;

			if (pItemData->ucFlags & PROJECTITEM_FLAG_ISIMPORTED)
			{
				lpNMCustomDraw->clrText = PROJECTLISTVIEW_COLOR_IMPORTED;
				return CDRF_NEWFONT;
			}
			
			if (pItemData->ucFlags & PROJECTITEM_FLAG_ISLOCKED)
			{
				lpNMCustomDraw->clrText = GetSysColor(COLOR_GRAYTEXT); 
				return CDRF_NEWFONT;
			}

			bHandled = false;
			return CDRF_DODEFAULT;
	}

	bHandled = false;
	return CDRF_DODEFAULT;
}

void CProjectListViewCtrl::BeginDrag(LPNMLISTVIEW pNMListView)
{
	CProjectDropSource *pDropSource = new CProjectDropSource();
	CProjectDataObject *pDataObject = new CProjectDataObject();

	// Add all file names to the data object.
	int iItemIndex = -1;
	iItemIndex = GetNextItem(iItemIndex,LVNI_SELECTED);

	while (iItemIndex != -1)
	{
		CItemData *pItemData = (CItemData *)GetItemData(iItemIndex);
		pDataObject->AddFile(pItemData);

		iItemIndex = GetNextItem(iItemIndex,LVNI_SELECTED);
	}

	pDataObject->SetParent(g_TreeManager.GetCurrentNode());

	DWORD dwEffect = 0;
	DWORD dwResult = ::DoDragDrop(pDataObject,pDropSource,DROPEFFECT_MOVE,&dwEffect);

	pDropSource->Release();
	pDataObject->Release();	
}

void CProjectListViewCtrl::SetViewStyle(int iViewStyle)
{
	unsigned long ulStyle = 0;

	switch (iViewStyle)
	{
		case LISTVIEWSTYLE_LARGEICONS:
			ulStyle = LVS_ICON;
			break;

		case LISTVIEWSTYLE_SMALLICONS:
			ulStyle = LVS_SMALLICON;
			break;

		case LISTVIEWSTYLE_LIST:
			ulStyle = LVS_LIST;
			break;

		case LISTVIEWSTYLE_DETAILS:
			ulStyle = LVS_REPORT;
			break;
	};

	unsigned long ulOldStyle = GetWindowLong(GWL_STYLE);
	if ((ulOldStyle & LVS_TYPEMASK) != ulStyle)
	{
		unsigned long ulNewStyle = (ulOldStyle & ~LVS_TYPEMASK) | ulStyle;
		SetWindowLong(GWL_STYLE,ulNewStyle);
	}
}

LRESULT CProjectListViewCtrl::OnViewLargeIcons(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_DynamicSettings.m_iPrjListViewStyle = LISTVIEWSTYLE_LARGEICONS;
	g_DynamicSettings.Apply();
	return 0;
}

LRESULT CProjectListViewCtrl::OnViewSmallIcons(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_DynamicSettings.m_iPrjListViewStyle = LISTVIEWSTYLE_SMALLICONS;
	g_DynamicSettings.Apply();
	return 0;
}

LRESULT CProjectListViewCtrl::OnViewList(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_DynamicSettings.m_iPrjListViewStyle = LISTVIEWSTYLE_LIST;
	g_DynamicSettings.Apply();
	return 0;
}

LRESULT CProjectListViewCtrl::OnViewDetails(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_DynamicSettings.m_iPrjListViewStyle = LISTVIEWSTYLE_DETAILS;
	g_DynamicSettings.Apply();
	return 0;
}

LRESULT CProjectListViewCtrl::OnNewFolder(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_ProjectManager.ListAddNewFolder();
	return 0;
}

/*
	CProjectListViewCtrl::SelectDropTarget
	--------------------------------------
	Marks the specified item as drop target, if iDropItemIndex the drop target
	items will be cleared but no new target selected.
*/
void CProjectListViewCtrl::SelectDropTarget(int iDropItemIndex)
{
	// Move the selected items.
	int iItemIndex = -1;
	iItemIndex = GetNextItem(iItemIndex,LVNI_DROPHILITED);

	while (iItemIndex != -1)
	{
		SetItemState(iItemIndex,0,LVNI_DROPHILITED);

		iItemIndex = GetNextItem(-1,LVNI_DROPHILITED);
	}

	if (iDropItemIndex != -1)
		SetItemState(iDropItemIndex,LVNI_DROPHILITED,LVNI_DROPHILITED);
}

void CProjectListViewCtrl::ForceRedraw()
{
	RECT rcClient;
	GetClientRect(&rcClient);
	InvalidateRect(&rcClient,true);
}
