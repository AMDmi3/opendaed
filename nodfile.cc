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

#include "nodfile.hh"

NodFile::NodFile(const std::string& path) {
	std::ifstream stream(path, std::ios_base::in);
	stream.exceptions(std::ifstream::badbit);

	int index = 0;
	while (!stream.eof()) {
		Entry e;
		stream >> e.number >> e.name;

		for (int i = 0; i < 24; i++)
			stream >> e.fields[i];

		if (stream.fail())
			break;

		if (index != e.number)
			throw std::runtime_error("entry index in file != real index, this is unexpected");

		entries_.push_back(std::move(e));

		index++;
	}
}

NodFile::~NodFile() {
}

const NodFile::Entry& NodFile::GetEntry(int index) const {
	return entries_[index];
}

int NodFile::GetNumEntries() const {
	return entries_.size();
}
