/*
 * Copyright (C) 2014 Dmitry Marakasov
 *
 * This file is part of opendaed.
 *
 * opendaed is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * opendaed is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with opendaed.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HOTFILE_HH
#define HOTFILE_HH

#include <string>
#include <vector>
#include <map>

#include <SDL2pp/Rect.hh>

class HotFile {
public:
	typedef std::vector<SDL2pp::Rect> RectVector;

protected:
	typedef std::map<int, RectVector> EntryMap;

protected:
	EntryMap entries_;

public:
	HotFile(const std::string& path);
	~HotFile();

	const RectVector& GetRectsForFrame(int frame) const;
};

#endif // HOTFILE_HH
