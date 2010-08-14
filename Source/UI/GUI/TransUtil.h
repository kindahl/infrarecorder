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

#define TRANSUTIL_STATICEDIT_SPACING		6

int UpdateStaticWidth(HWND hParentWnd,HWND hCtrlWnd,const TCHAR *szText);
int UpdateStaticWidth(HWND hParentWnd,int iStaticID,const TCHAR *szText);
void UpdateEditPos(HWND hParentWnd,int iEditID,int iLeft,bool bMove = false);
int UpdateStaticHeight(HWND hParentWnd,int iStaticID,const TCHAR *szText);
