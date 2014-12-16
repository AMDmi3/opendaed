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

bool QuickTime::SupportedVideo(int track) const {
	return quicktime_supported_video(qt_, track);
}

int QuickTime::GetWidth(int track) const {
	return quicktime_video_width(qt_, track);
}

int QuickTime::GetHeight(int track) const {
	return quicktime_video_height(qt_, track);
}

int QuickTime::GetTimeScale(int track) const {
	return lqt_video_time_scale(qt_, track);
}

int QuickTime::GetFrameDuration(int track) const {
	int constant;
	int duration = lqt_frame_duration(qt_, track, &constant);
	if (constant != 1)
		throw std::runtime_error("video with non-constant framerate detected, not supported");
	return duration;
}

int64_t QuickTime::GetVideoPtsOffset(int track) const {
	return lqt_get_video_pts_offset(qt_, track);
}

lqt_sample_format_t QuickTime::GetSampleFormat(int track) const {
	return lqt_get_sample_format(qt_, track);
}

int QuickTime::SetVideoPosition(int64_t frame, int track) {
	return quicktime_set_video_position(qt_, frame, track);
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

bool QuickTime::HasAudio() const {
	return quicktime_has_audio(qt_);
}

bool QuickTime::SupportedAudio(int track) const {
	return quicktime_supported_audio(qt_, track);
}

long QuickTime::GetSampleRate(int track) const {
	return quicktime_sample_rate(qt_, track);
}

int QuickTime::GetAudioBits(int track) const {
	return quicktime_audio_bits(qt_, track);
}

int QuickTime::GetTrackChannels(int track) const {
	return quicktime_track_channels(qt_, track);
}

int64_t QuickTime::GetAudioPtsOffset(int track) const {
	return lqt_get_audio_pts_offset(qt_, track);
}

int QuickTime::SetAudioPosition(int64_t sample, int track) {
	return quicktime_set_audio_position(qt_, sample, track);
}

int64_t QuickTime::LastAudioPosition(int track) const {
	return lqt_last_audio_position(qt_, track);
}

int QuickTime::DecodeAudioTrack(int16_t** output_i, float** output_f, long samples, int track) {
	return lqt_decode_audio_track(qt_, output_i, output_f, samples, track);
}

int QuickTime::DecodeAudioTrackInterleaved(int16_t* output_i, float* output_f, long samples, int track) {
	std::vector<int16_t> temp_i(output_i ? GetTrackChannels(track) * samples : 0, 0);
	std::vector<float> temp_f(output_f ? GetTrackChannels(track) * samples : 0, 0.0f);

	std::vector<int16_t*> temp_p_i;
	if (output_i)
		for (int i = 0; i < GetTrackChannels(track); i++)
			temp_p_i.push_back(temp_i.data() + i * samples);
	std::vector<float*> temp_p_f;
	if (output_f)
		for (int i = 0; i < GetTrackChannels(track); i++)
			temp_p_f.push_back(temp_f.data() + i * samples);

	int retval = lqt_decode_audio_track(qt_, output_i ? temp_p_i.data() : nullptr, output_f ? temp_p_f.data() : nullptr, samples, track);

	if (output_i)
		for (int s = 0; s < samples; s++)
			for (int c = 0; c < GetTrackChannels(track); c++)
				*(output_i++) = temp_i[c * samples + s];

	if (output_f)
		for (int s = 0; s < samples; s++)
			for (int c = 0; c < GetTrackChannels(track); c++)
				*(output_f++) = temp_f[c * samples + s];

	return retval;
}

int QuickTime::DecodeAudioRaw(void* output, long samples, int track) {
	return lqt_decode_audio_raw(qt_, output, samples, track);
}
