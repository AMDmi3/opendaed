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

#ifndef LOGGER_HH
#define LOGGER_HH

#ifdef NDEBUG

class Log {
public:
	Log(const char*) {
	}

	template<class T>
	Log& operator<<(const T& t) {
		return *this;
	}
};

#else

#include <iostream>
#include <iomanip>
#include <sstream>

class Log {
private:
	const char* ident_;
	std::stringstream stream_;

public:
	Log(const char* ident) : ident_(ident) {
	}

	~Log() {
		try {
			std::cerr << "[" << std::setw(8) << ident_ << "] " << stream_.str() << std::endl;
		} catch (...) {
		}
	}

	template<class T>
	Log& operator<<(const T& t) {
		stream_ << t;
		return *this;
	}
};

#endif

#endif // LOGGER_HH
