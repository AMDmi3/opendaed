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
#include <map>

class NodFile {
public:
	enum class Condition {
		NO = 104,
		YES = 103,
	};

	struct Entry {
		typedef std::map<int, int> ConditionMap;

		int number;
		std::string name;
		int fields[24];

		int GetType() const { return fields[0]; }
		int GetStartFrame() const { return fields[1]; }
		int GetEndFrame() const { return fields[2]; }
		const std::string GetName() const { return name; }

		int GetDefaultOffset() const { return fields[5]; }

		ConditionMap GetConditions() const {
			ConditionMap conds;
			for (int i = 6; i <= 20; i += 2)
				if (fields[i+1] != 0)
					conds[fields[i+1]] = fields[i];
			return conds;
		}
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
