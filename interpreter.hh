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

#ifndef INTERPRETER_HH
#define INTERPRETER_HH

#include <map>

#include "nodfile.hh"
#include "gameeventlistener.hh"

class DataManager;
class GameInterface;
class MovPlayer;

class Interpreter : private GameEventListener {
protected:
	typedef std::map<std::string, NodFile> NodFileMap;
	typedef std::pair<std::string, int> NodPointer;

protected:
	NodFileMap nod_files_;

	const DataManager& data_manager_;
	GameInterface& interface_;
	MovPlayer& player_;

	NodPointer current_node_;

	bool awaiting_event_;

protected:
	void InterruptAndGoto(int offset);

public:
	Interpreter(const DataManager& data_manager, GameInterface& interface, MovPlayer& player, const std::string& startnod, int numentry = 0);
	virtual ~Interpreter();

	void Update();
};

#endif // INTERPRETER_HH
