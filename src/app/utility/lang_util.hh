/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2012 Christian Kindahl
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

#define TRANSLATION_ID_AUTHOR			1
#define TRANSLATION_ID_DATE				2
#define TRANSLATION_ID_VERSION			3
#define TRANSLATION_ID_MANUAL			4

const TCHAR *lngGetString(unsigned int uiID);
const TCHAR *lngGetManual();
int lngMessageBox(HWND hWnd,unsigned int uiTextID,unsigned int uiCaptionID,unsigned int uiType);
void lngTranslateTables();
ckcore::tstring lngSlowFormatStr(const eStringTable TranslatedFormatStr,...);
