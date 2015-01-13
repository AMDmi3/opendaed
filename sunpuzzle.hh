/*
 * Copyright (C) 2015 Dmitry Marakasov
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

#ifndef SUNPUZZLE_HH
#define SUNPUZZLE_HH

#include <array>

#include <SDL2pp/Texture.hh>
#include <SDL2pp/Renderer.hh>

#include "screen.hh"

class DataManager;

class SunPuzzle : public Screen {
private:
	static const std::array<SDL2pp::Point, 6> button_locations_;

private:
	SDL2pp::Renderer& renderer_;

	// Textures
	SDL2pp::Texture background_;
	SDL2pp::Texture buttons_;
	SDL2pp::Texture temperature_;

private:
	std::array<int, 6> states_;

public:
	SunPuzzle(SDL2pp::Renderer& renderer, const DataManager& datamanager);
	virtual ~SunPuzzle();

	bool ProcessEvent(const SDL_Event& event) override;
	bool Update() override;
	void Render() override;
};

#endif // SUNPUZZLE_HH
