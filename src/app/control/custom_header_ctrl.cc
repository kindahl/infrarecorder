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

#include "stdafx.h"
#include "custom_header_ctrl.hh"
#include "ProjectManager.h"
#include "TreeManager.h"

CCustomHeaderCtrl::CCustomHeaderCtrl()
{
	m_bSortUp = true;
	m_iSortCol = 0;
}

void CCustomHeaderCtrl::SetSortColumn(unsigned int uiColIndex,bool bSortUp)
{
	m_bSortUp = bSortUp;
	m_iSortCol = uiColIndex;

	HDITEM hdColumn;
	hdColumn.mask = HDI_FORMAT;
	GetItem(m_iSortCol,&hdColumn);

	if (m_bSortUp)
	{
		hdColumn.fmt |= HDF_SORTUP;
		hdColumn.fmt &= ~HDF_SORTDOWN;
	}
	else
	{
		hdColumn.fmt |= HDF_SORTDOWN;
		hdColumn.fmt &= ~HDF_SORTUP;
	}

	SetItem(m_iSortCol,&hdColumn);
}

LRESULT CCustomHeaderCtrl::OnSetSortColumn(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	SetSortColumn((unsigned int)wParam,lParam == 0);
	return 0;
}

void CCustomHeaderCtrl::ColumnClick(unsigned int uiColIndex)
{
	if (m_iSortCol == uiColIndex)
	{
		// Set the new sort arrow.
		SetSortColumn(uiColIndex,!m_bSortUp);
	}
	else
	{
		// Remove the sort arrow from the previous column.
		HDITEM hdColumn;
		hdColumn.mask = HDI_FORMAT;
		GetItem(m_iSortCol,&hdColumn);
		hdColumn.fmt &= ~(m_bSortUp ? HDF_SORTUP : HDF_SORTDOWN);
		SetItem(m_iSortCol,&hdColumn);

		// Set the new sort arrow.
		SetSortColumn(uiColIndex,true);
	}

	// Do the sorting.
	g_TreeManager.GetCurrentNode()->Sort(uiColIndex,m_bSortUp,g_ProjectManager.GetViewType() != PROJECTVIEWTYPE_DATA);
	g_TreeManager.Refresh();
}
