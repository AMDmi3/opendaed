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

#include <stdexcept>

#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_render.h>

#include <SDL2pp/AudioSpec.hh>

#include "logger.hh"

#include "movplayer.hh"

MovPlayer::MovPlayer() : state_(STOPPED), listener_(nullptr) {
}

MovPlayer::~MovPlayer() {
}

void MovPlayer::SetListener(MovPlayer::EventListener* listener) {
	listener_ = listener;
}

void MovPlayer::UpdateMovieFile(const std::string& filename, bool need_audio) {
	has_audio_ = false;
	if (filename != current_file_ || qt_.get() == nullptr) {
		// open new qt video
		qt_.reset(new QuickTime(filename));

		if (!qt_->HasVideo())
			throw std::runtime_error("no video track");

		if (!qt_->SupportedVideo())
			throw std::runtime_error("video track not supported");

		if (need_audio && qt_->HasAudio()) {
			if (!qt_->SupportedAudio())
				throw std::runtime_error("audio track not supported");
			has_audio_ = true;
		}

		current_frame_ = -1;
		next_frame_ = 0;
	}
}

void MovPlayer::UpdateFrameTexture(SDL2pp::Renderer& renderer, int frame) {
	// we already have wanted frame loaded, do nothing
	if (current_frame_ == frame)
		return;

	// is seek required?
	if (next_frame_ != frame) {
		qt_->SetVideoPosition(frame);
		next_frame_ = frame;
	}

	int width = qt_->GetWidth();
	int height = qt_->GetHeight();

	// is texture rebuild required?
	if (texture_.get() == nullptr ||
			texture_->GetFormat() != SDL_PIXELFORMAT_RGB24 ||
			texture_->GetAccess() != SDL_TEXTUREACCESS_STREAMING ||
			texture_->GetWidth() != width ||
			texture_->GetHeight() != height)
		texture_.reset(new SDL2pp::Texture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height));

	// decode next frame
	{
		SDL2pp::Texture::LockHandle lock = texture_->Lock(SDL2pp::NullOpt);
		qt_->DecodeVideo(static_cast<unsigned char*>(lock.GetPixels()), lock.GetPitch());
	}

	current_frame_ = next_frame_++;
}

void MovPlayer::ResetPlayback() {
	start_frame_ = end_frame_ = 0;
	start_frame_ticks_ = 0;
	state_ = STOPPED;
	audio_.reset(nullptr);
}

void MovPlayer::EmitEndOfClipEvent() {
	if (listener_)
		listener_->ProcessEndOfClipEvent();
}

void MovPlayer::Play(const std::string& filename, int startframe, int endframe) {
	Log("player") << "playing " << filename << " at [" << startframe << ".." << endframe << "]";

	ResetPlayback();

	UpdateMovieFile(filename, true);

	// print some info
	Log("player") << "  video:";
	Log("player") << "    dimensions: " << qt_->GetWidth() << "x" << qt_->GetHeight();
	Log("player") << "    time scale: " << qt_->GetTimeScale();
	Log("player") << "    frame dur.: " << qt_->GetFrameDuration();
	Log("player") << "    frame rate: " << (float)qt_->GetTimeScale() / (float)qt_->GetFrameDuration() << " fps";
	Log("player") << "    pts offset: " << qt_->GetVideoPtsOffset() << " (" << (float)qt_->GetVideoPtsOffset() / (float)qt_->GetFrameDuration() << " frames)";

	if (has_audio_) {
		Log("player") << "  audio:";
		Log("player") << "    sample rate: " << qt_->GetSampleRate();
		std::string format = "unknown";
		switch (qt_->GetSampleFormat()) {
		case LQT_SAMPLE_INT8: format = "s8"; break;
		case LQT_SAMPLE_UINT8: format = "u8"; break;
		case LQT_SAMPLE_INT16: format = "s16"; break;
		case LQT_SAMPLE_INT32: format = "s32"; break;
		case LQT_SAMPLE_FLOAT: format = "float"; break;
		case LQT_SAMPLE_DOUBLE: format = "double"; break;
		default: break;
		}
		Log("player") << "    sample format: " << format;
		Log("player") << "    audio bits: " << qt_->GetAudioBits();
		Log("player") << "    channels: " << qt_->GetTrackChannels();
		Log("player") << "    pts offset: " << qt_->GetAudioPtsOffset() << " samples";
	}

	// setup video timing
	start_frame_ = startframe - qt_->GetVideoPtsOffset() / qt_->GetFrameDuration();
	end_frame_ = endframe - qt_->GetVideoPtsOffset() / qt_->GetFrameDuration();

	// setup audio
	if (has_audio_) {
		SDL2pp::AudioSpec spec(qt_->GetSampleRate(), AUDIO_U8, qt_->GetTrackChannels(), 16);
		audio_.reset(new SDL2pp::AudioDevice(SDL2pp::NullOpt, false, spec,
				[this](Uint8* stream, int len) {
					qt_->DecodeAudioRaw(stream, len / qt_->GetTrackChannels());
				}
			));

		int audiopos = (int)((float)start_frame_ * (float)qt_->GetFrameDuration() / (float)qt_->GetTimeScale() * qt_->GetSampleRate());
		qt_->SetAudioPosition(audiopos);

		audio_->Pause(false);
	}

	start_frame_ticks_ = SDL_GetTicks();

	state_ = PLAYING;
}

void MovPlayer::PlaySingleFrame(const std::string& filename, int frame) {
	Log("player") << "playing " << filename << " single frame " << frame;

	ResetPlayback();

	UpdateMovieFile(filename, false);

	// print some info
	Log("player") << "  video:";
	Log("player") << "    dimensions: " << qt_->GetWidth() << "x" << qt_->GetHeight();

	start_frame_ = frame;

	state_ = SINGLE_FRAME;
}

void MovPlayer::Stop() {
	Log("player") << "stopping";
	state_ = STOPPED;
}

bool MovPlayer::UpdateFrame(SDL2pp::Renderer& renderer) {
	// no movie loaded -> nothing to do
	if (!qt_.get())
		return false;

	// ensure audio callback is not called while processing this frame
	SDL2pp::AudioDevice::LockHandle lock;
	if (has_audio_)
		lock = audio_->Lock();

	// calculate wanted frame from given time
	int wanted_frame;
	switch (state_) {
	case STOPPED:
		wanted_frame = current_frame_;
		break;
	case PLAYING:
		wanted_frame = start_frame_ +
			(SDL_GetTicks() - start_frame_ticks_) * qt_->GetTimeScale() / (qt_->GetFrameDuration() * 1000) -
			qt_->GetVideoPtsOffset() / qt_->GetFrameDuration();
		if (wanted_frame > end_frame_)
			wanted_frame = end_frame_;
		break;
	case SINGLE_FRAME:
		wanted_frame = start_frame_;
		break;
	}

	// don't go past end frame
	if (wanted_frame < 0)
		wanted_frame = 0;

	UpdateFrameTexture(renderer, wanted_frame);

	if (state_ == PLAYING) {
		if (current_frame_ >= end_frame_) {
			Log("player") << "movie finished";
			if (audio_.get())
				audio_->Pause(true);
			state_ = STOPPED;
			EmitEndOfClipEvent();
		}
	}

	return true;
}

int MovPlayer::GetCurrentFrame() const {
	return current_frame_;
}

SDL2pp::Texture* MovPlayer::GetTexture() {
	return texture_.get();
}
