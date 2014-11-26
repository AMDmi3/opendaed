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

#include <SDL2/SDL_render.h>

#include "logger.hh"

#include "movplayer.hh"

MovPlayer::MovPlayer() : playing_(false) {
}

MovPlayer::~MovPlayer() {
}

void MovPlayer::Play(const std::string& filename, unsigned int startticks, int startframe, int endframe, Callback&& finish_callback) {
	Log("player") << "playing " << filename << " at [" << startframe << ".." << endframe << "]";
	if (filename != current_file_ || qt_.get() == nullptr) {
		// open new qt video
		qt_.reset(new QuickTime(filename));

		if (!qt_->HasVideo())
			throw std::runtime_error("no video track");

		current_frame_ = -1;
		next_frame_ = 0;
	}

	finish_callback_ = finish_callback;

	start_frame_ = startframe;
	end_frame_ = endframe;

	start_frame_ticks_ = startticks;

	playing_ = true;
}

void MovPlayer::Stop() {
	Log("player") << "Stopping";
	playing_ = false;
}

void MovPlayer::UpdateFrame(SDL2pp::Renderer& renderer, unsigned int ticks) {
	// not playing -> do nothing
	if (!playing_)
		return;

	// calculate wanted frame from given time
	int wanted_frame = start_frame_ + (ticks - start_frame_ticks_) * qt_->GetTimeScale() / (qt_->GetFrameDuration() * 1000);

	// don't go past end frame
	if (wanted_frame > end_frame_)
		wanted_frame = end_frame_;

	// we already have wanted frame loaded, do nothing
	if (current_frame_ == wanted_frame)
		return;

	// seek required
	if (next_frame_ != wanted_frame) {
		qt_->SetVideoPosition(wanted_frame);
		next_frame_ = wanted_frame;
	}

	int width = qt_->GetWidth();
	int height = qt_->GetHeight();

	// texture rebuild required
	if (texture_.get() == nullptr ||
			texture_->GetFormat() != SDL_PIXELFORMAT_RGB24 ||
			texture_->GetAccess() != SDL_TEXTUREACCESS_STREAMING ||
			texture_->GetWidth() != width ||
			texture_->GetHeight() != height)
		texture_.reset(new SDL2pp::Texture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height));

	// decode next frame
	{
		SDL2pp::Texture::LockHandle lock = texture_->Lock(SDL2pp::Rect::Null());
		qt_->DecodeVideo(static_cast<unsigned char*>(lock.GetPixels()), lock.GetPitch());
	}

	current_frame_ = next_frame_++;

	if (current_frame_ >= end_frame_) {
		Log("player") << "Movie finished";
		finish_callback_();
		playing_ = false;
	}
}

SDL2pp::Texture& MovPlayer::GetTexture() {
	return *texture_.get();
}
