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

#include <vector>
#include <stdexcept>

#include "quicktime.hh"

QuickTime::QuickTime(const std::string& path) {
	qt_ = quicktime_open(path.c_str(), 1, 0);
	if (qt_ == nullptr)
		throw std::runtime_error("quicktime_open failed");
}

QuickTime::~QuickTime() {
	quicktime_close(qt_);
}

quicktime_t* QuickTime::Get() const {
	return qt_;
}

bool QuickTime::HasVideo() const {
	return quicktime_has_video(qt_);
}

int QuickTime::GetWidth(int track) const {
	return quicktime_video_width(qt_, track);
}

int QuickTime::GetHeight(int track) const {
	return quicktime_video_height(qt_, track);
}

int QuickTime::SetVideoPosition(int64_t frame, int track) {
	return quicktime_set_video_position(qt_, frame, track);
}

int QuickTime::GetTimeScale(int track) {
	return lqt_video_time_scale(qt_, track);
}

int QuickTime::GetFrameDuration(int track) {
	int constant;
	int duration = lqt_frame_duration(qt_, track, &constant);
	if (constant != 1)
		throw std::runtime_error("video with non-constant framerate detected, not supported");
	return duration;
}

int QuickTime::DecodeVideo(unsigned char** row_pointers, int track) {
	return quicktime_decode_video(qt_, row_pointers, track);
}

int QuickTime::DecodeVideo(unsigned char* pixels, int pitch, int track) {
	const int height = GetHeight(track);

	std::vector<unsigned char*> row_pointers(height);
	for (int i = 0; i < height; i++)
		row_pointers[i] = pixels + pitch * i;

	return quicktime_decode_video(qt_, row_pointers.data(), track);
}
