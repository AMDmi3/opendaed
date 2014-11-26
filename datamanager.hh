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

#ifndef DATAMANAGER_HH
#define DATAMANAGER_HH

#include <string>
#include <map>
#include <functional>

class DataManager {
protected:
	typedef std::map<std::string, std::string> PathMap;

protected:
	PathMap data_files_;

private:
	void ScanDir(const std::string& path, std::function<void(const std::string&, const std::string&)> processor);

public:
	DataManager();
	~DataManager();

	void ScanDir(const std::string& datapath);
	std::string GetPath(const std::string& path) const;
};

#endif // DATAMANAGER_HH
