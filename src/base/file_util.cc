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

#include <ckcore/types.hh>
#include <ckcore/string.hh>
#include <ckcore/exception.hh>
#include "file_util.hh"

void WriteString(ckcore::File &File,const ckcore::tchar *szString)
{
	const ckcore::tuint32 uiByteCount =
		static_cast<ckcore::tuint32>(ckcore::string::astrlen(szString) * sizeof(ckcore::tchar));
	
	if (uiByteCount != File.write(szString,uiByteCount))
		throw ckcore::Exception2(ckT("Error writing to the file."));	// FIXME: Improve message output.
}