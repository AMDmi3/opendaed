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

#include <dirent.h>

#include <algorithm>
#ifndef NDEBUG
#	include <iostream>
#endif

#include "datamanager.hh"

void DataManager::ScanDir(const std::string& path, std::function<void(const std::string&, const std::string&)> processor) {
	DIR* dirp = opendir(path.c_str());
	if (dirp == nullptr)
		throw std::runtime_error("cannot read data directory");

	try {
		struct dirent* de;
		while ((de = readdir(dirp)) != nullptr) {
			if (de->d_name[0] == '.')
				continue;

			if (de->d_type == DT_DIR) {
				ScanDir(path + "/" + de->d_name, processor);
			} else if (de->d_type == DT_REG) {
				processor(path, de->d_name);
			}
		}
	} catch (...) {
		closedir(dirp);
		throw;
	}
	closedir(dirp);
}

DataManager::DataManager() {
}

DataManager::~DataManager() {
}

void DataManager::ScanDir(const std::string& datapath) {
	PathMap new_files;

	// Note: for the sake of simplicity, we assume that file
	// with specific name is only present on a single disk once
	// and if it's present on different disks, all copies are
	// identical
	ScanDir(datapath, [this, &new_files](const std::string& dir, const std::string& file){
			std::string fullpath = dir + "/" + file;
			std::string lcfile = file;
			std::transform(lcfile.begin(), lcfile.end(), lcfile.begin(), ::tolower);
			new_files.insert(std::make_pair(lcfile, fullpath));
		});

	data_files_.swap(new_files);

#ifndef NDEBUG
	std::cerr << "Found data files in " << datapath << ":" << std::endl;
	for (auto& file: data_files_)
		std::cerr << "    " << file.first << " -> " << file.second << std::endl;
#endif
}

std::string DataManager::GetPath(const std::string& path) const {
	// Though full path may be provided (e.g. "images/intrface.bmp",
	// only file name matters, see assumption described in ScanDir()
	std::string name = path;
	size_t slashpos = name.rfind('/');
	if (slashpos != std::string::npos)
		name = name.substr(slashpos + 1);
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);

	PathMap::const_iterator file = data_files_.find(name);
	if (file == data_files_.end())
		throw std::runtime_error("required data file not found");

	return file->second;
}
