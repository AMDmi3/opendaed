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

#ifndef MOVPLAYER_HH
#define MOVPLAYER_HH

#include <string>
#include <memory>
#include <functional>

#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Texture.hh>
#include <SDL2pp/Audio.hh>

#include "quicktime.hh"

class MovPlayer {
public:
	typedef std::function<void()> Callback;

protected:
	enum State {
		PLAYING,
		STOPPED,
		SINGLE_FRAME,
	};

protected:
	std::unique_ptr<QuickTime> qt_;

	std::unique_ptr<SDL2pp::Texture> texture_;
	std::unique_ptr<SDL2pp::AudioDevice> audio_;

	// state of the currently loaded movie clip
	std::string current_file_;
	bool has_audio_;
	int current_frame_;
	int next_frame_;

	// state of the player
	State state_;
	unsigned int start_frame_ticks_;
	int start_frame_;
	int end_frame_;
	Callback finish_callback_;

protected:
	void UpdateMovieFile(const std::string& name, bool need_audio);
	void UpdateFrameTexture(SDL2pp::Renderer& renderer, int frame);

	void ResetPlayback();

public:
	MovPlayer();
	~MovPlayer();

	void Play(const std::string& filename, int startframe, int endframe, Callback&& finish_callback = Callback());
	void PlaySingleFrame(const std::string& filename, int frame);
	void Stop();

	bool UpdateFrame(SDL2pp::Renderer& renderer);
	SDL2pp::Texture* GetTexture();
};

#endif // MOVPLAYER_HH
