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

#include <list>
#ifndef NDEBUG
#	include <iostream>
#endif

#include "datamanager.hh"

#include "interpreter.hh"

Interpreter::Interpreter(const DataManager& data_manager, const std::string& startnod) {
	std::list<std::string> loading_queue;
	loading_queue.push_back(startnod);

	while (!loading_queue.empty()) {
#ifndef NDEBUG
		std::cerr << "Loading script " << loading_queue.front() << std::endl;
#endif
		NodFileMap::iterator just_added = nod_files_.emplace(loading_queue.front(), data_manager.GetPath(loading_queue.front())).first;

		// collect all .nod files referenced by recently
		// loaded one which were not yet visited
		just_added->second.ForEach([this, &loading_queue](const NodFile::Entry& e) {
				std::string file = e.GetName();
				std::transform(file.begin(), file.end(), file.begin(), ::tolower);
				if (file.rfind(".nod") == file.length() - 4 && file != "gate.nod" && nod_files_.find(file) == nod_files_.end())
					loading_queue.push_back(file);
			});

		loading_queue.pop_front();
	}
}

Interpreter::~Interpreter() {
}
