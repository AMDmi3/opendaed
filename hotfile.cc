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

#include <iostream>
#include <fstream>

#include "hotfile.hh"

HotFile::HotFile(const std::string& path) {
	std::ifstream stream(path, std::ios_base::in);
	stream.exceptions(std::ifstream::badbit);

	while (!stream.eof()) {
		int frameno;
		stream >> frameno;

		if (stream.fail())
			break;

		RectVector rects;
		for (int i = 0; i < 8; i++) {
			int x1, y1, x2, y2;

			stream >> x1 >> y1 >> x2 >> y2;

			if (stream.fail())
				break;

			if (x1 != 0 || y1 != 0 || x2 != 0 || y2 != 0)
				rects.emplace_back(x1, y1, x2 - x1, y2 - y1);
		}

		entries_.emplace(std::make_pair(frameno, std::move(rects)));
	}
}

HotFile::~HotFile() {
}

const HotFile::RectVector& HotFile::GetRectsForFrame(int frame) const {
	EntryMap::const_iterator entry = entries_.find(frame);
	if (entry == entries_.end())
		throw std::runtime_error("no hotspots for requested frame");
	return entry->second;
}
