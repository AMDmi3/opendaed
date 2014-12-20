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
		Entry e;
		stream >> e.frameno;

		if (stream.fail())
			break;

		for (int i = 0; i < 8; i++) {
			int x1, y1, x2, y2;

			stream >> x1 >> y1 >> x2 >> y2;

			if (stream.fail())
				break;

			if (x1 != 0 || y1 != 0 || x2 != 0 && y2 != 0)
				e.rects.emplace_back(x1, y1, x2 - x1, y2 - y1);
		}

		entries_.push_back(std::move(e));
	}
}

HotFile::~HotFile() {
}

const HotFile::Entry& HotFile::GetEntry(int index) const {
	if (index < 0 || index >= (int)entries_.size())
		throw std::runtime_error("entry does not exists in hot file");
	return entries_[index];
}

int HotFile::GetNumEntries() const {
	return entries_.size();
}
