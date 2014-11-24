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

#ifndef NODFILE_HH
#define NODFILE_HH

#include <string>
#include <vector>

class NodFile {
public:
	struct Entry {
		int number;
		std::string file;
		int fields[24];
	};

protected:
	typedef std::vector<Entry> EntryVector;

protected:
	EntryVector entries_;

public:
	NodFile(const std::string& path);
	~NodFile();

	const Entry& GetEntry(int index) const;
	int GetNumEntries() const;

	template<class F>
	void ForEach(const F& processor) const {
		for (auto& e : entries_)
			processor(e);
	}
};

#endif // NODFILE_HH
