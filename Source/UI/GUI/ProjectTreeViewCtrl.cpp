/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
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

#include "stdafx.h"
#include "ProjectTreeViewCtrl.h"
#include "ProjectManager.h"
#include "ProjectDataObject.h"
#include "ProjectDropSource.h"

CProjectTreeViewDropTarget::CProjectTreeViewDropTarget(CTreeViewCtrlEx *pHost)
{
	m_pHost = pHost;
}

bool CProjectTreeViewDropTarget::OnDragOver(POINTL ptCursor)
{
	TVHITTESTINFO tvHit;
	tvHit.pt.x = ptCursor.x;
	tvHit.pt.y = ptCursor.y;

	ScreenToClient(m_pHost->m_hWnd,&tvHit.pt);

	HTREEITEM hTarget = m_pHost->HitTest(&tvHit);
	if (hTarget)
	{
		m_pHost->SelectDropTarget(hTarget);
		return true;
	}

	m_pHost->SelectDropTarget(NULL);
	return false;
}

bool CProjectTreeViewDropTarget::OnDrop(POINTL ptCursor,IDataObject *pDataObject)
{
	TVHITTESTINFO tvHit;
	tvHit.pt.x = ptCursor.x;
	tvHit.pt.y = ptCursor.y;

	ScreenToClient(m_pHost->m_hWnd,&tvHit.pt);

	HTREEITEM hTarget = m_pHost->HitTest(&tvHit);
	if (hTarget)
	{
		m_pHost->SelectDropTarget(NULL);

		HandleDropData(pDataObject,(CProjectNode *)m_pHost->GetItemData(hTarget));
		return true;
	}

	return false;
}

void CProjectTreeViewDropTarget::OnDragLeave()
{
	m_pHost->SelectDropTarget(NULL);
}

bool CProjectTreeViewDropTarget::HandleDropData(IDataObject *pDataObject,CProjectNode *pTargetNode)
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

	return true;
}

CProjectTreeViewCtrl::CProjectTreeViewCtrl()
{
	m_pDropTarget = new CProjectTreeViewDropTarget(this);

	CoLockObjectExternal(m_pDropTarget,TRUE,FALSE);
}

CProjectTreeViewCtrl::~CProjectTreeViewCtrl()
{
	if (m_pDropTarget != NULL)
	{
		CoLockObjectExternal(m_pDropTarget,FALSE,TRUE);

		m_pDropTarget->Release();
		m_pDropTarget = NULL;
	}
}

LRESULT CProjectTreeViewCtrl::OnSetFocus(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CProjectNode *pNode = (CProjectNode *)GetSelectedItem().GetData();

	g_ProjectManager.TreeSetActive();
	g_ProjectManager.NotifyTreeSelChanged(pNode);

	bHandled = false;
	return 0;
}

LRESULT CProjectTreeViewCtrl::OnCustomDraw(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	LPNMTVCUSTOMDRAW lpNMCustomDraw = (LPNMTVCUSTOMDRAW)lParam;

	switch (lpNMCustomDraw->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			return CDRF_NOTIFYITEMDRAW;			

		case CDDS_ITEMPREPAINT:
			CProjectNode *pNode = (CProjectNode *)lpNMCustomDraw->nmcd.lItemlParam;

			/*if (pNode == NULL || !(pNode->pItemData->ucFlags & PROJECTITEM_FLAG_ISLOCKED))
			{
				bHandled = false;
				return CDRF_DODEFAULT;
			}

			lpNMCustomDraw->clrText = GetSysColor(COLOR_GRAYTEXT); 
			return CDRF_NEWFONT;*/

			if (pNode != NULL && pNode->pItemData->ucFlags & PROJECTITEM_FLAG_ISIMPORTED)
			{
				lpNMCustomDraw->clrText = PROJECTTREEVIEW_COLOR_IMPORTED; 
				return CDRF_NEWFONT;
			}

			if (pNode != NULL && pNode->pItemData->ucFlags & PROJECTITEM_FLAG_ISLOCKED)
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

void CProjectTreeViewCtrl::BeginDrag(LPNMTREEVIEW pNMTreeView)
{
	if (((CProjectNode *)GetItemData(pNMTreeView->itemNew.hItem))->pItemData->ucFlags & PROJECTITEM_FLAG_ISPROJECTROOT)
		return;

	CProjectDropSource *pDropSource = new CProjectDropSource();
	CProjectDataObject *pDataObject = new CProjectDataObject();

	// Add all file names to the data object.
	pDataObject->AddFile(((CProjectNode *)GetItemData(pNMTreeView->itemNew.hItem))->pItemData);
	pDataObject->SetParent((CProjectNode *)GetItemData(GetParentItem(pNMTreeView->itemNew.hItem)));

	DWORD dwEffect = 0;
	DWORD dwResult = ::DoDragDrop(pDataObject,pDropSource,DROPEFFECT_MOVE,&dwEffect);

	pDropSource->Release();
	pDataObject->Release();
}

void CProjectTreeViewCtrl::ForceRedraw()
{
	RECT rcClient;
	GetClientRect(&rcClient);
	InvalidateRect(&rcClient,true);
}
