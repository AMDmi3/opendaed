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

class DataManager;

class Interpreter {
protected:
	typedef std::map<std::string, NodFile> NodFileMap;

protected:
	NodFileMap nod_files_;

public:
	Interpreter(const DataManager& data_manager, const std::string& startnod);
	~Interpreter();
};

#endif // INTERPRETER_HH
