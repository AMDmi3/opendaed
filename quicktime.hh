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

#ifndef QUICKTIME_HH
#define QUICKTIME_HH

#include <string>

#include <lqt/lqt.h>

class QuickTime {
protected:
	quicktime_t* qt_;

public:
	QuickTime(const std::string& path);
	~QuickTime();

	quicktime_t* Get() const;

	// video
	bool HasVideo() const;

	int GetWidth(int track = 0) const;
	int GetHeight(int track = 0) const;

	int SetVideoPosition(int64_t frame, int track = 0);

	int GetTimeScale(int track = 0);

	int GetFrameDuration(int track = 0);

	int DecodeVideo(unsigned char** row_pointers, int track = 0);
	int DecodeVideo(unsigned char* pixels, int pitch, int track = 0);
};

#endif // QUICKTIME_HH
