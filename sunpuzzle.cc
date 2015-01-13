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

#include <algorithm>

#include <SDL2/SDL_timer.h>

#include "sunpuzzle.hh"

#include "datamanager.hh"
#include "logger.hh"

// TODO: no sounds

const std::array<SDL2pp::Point, 6> SunPuzzle::button_locations_ = { {
	{ 360, 58 },
	{ 424, 144 },
	{ 404, 274 },
	{ 184, 274 },
	{ 166, 144 },
	{ 228, 58},
} };

SunPuzzle::SunPuzzle(SDL2pp::Renderer& renderer, const DataManager& datamanager)
	: renderer_(renderer),
	  background_(renderer, datamanager.GetPath("images/psun/od_bg2.bmp")),
	  buttons_(renderer, datamanager.GetPath("images/psun/od_buttb.rle")),
	  temperature_(renderer, datamanager.GetPath("images/psun/bigtempa.bmp")) {
	Log("puzzle") << "starting sun puzzle";

	std::fill(states_.begin(), states_.end(), 0);
}

SunPuzzle::~SunPuzzle() {
}

bool SunPuzzle::ProcessEvent(const SDL_Event& event) {
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		int nbutton = -1;
		for (auto buttonloc = button_locations_.begin(); buttonloc != button_locations_.end(); buttonloc++) {
			if (!SDL2pp::Rect(buttonloc->x, buttonloc->y, 42, 42).Contains(SDL2pp::Point(event.button.x, event.button.y)))
				continue;

			nbutton = buttonloc - button_locations_.begin();
			break;
		}

		if (nbutton < 0 || nbutton > 5)
			return true;

		// upgrade n'th and n+2'th buttons
		states_[nbutton] = (states_[nbutton] + 1) % 3;
		nbutton = (nbutton + 2) % 6;
		states_[nbutton] = (states_[nbutton] + 1) % 3;

		for (int i = 0; i < 6; i++)
			if (states_[i] != 2)
				return true;

		Log("puzzle") << "  congrats, puzzle is solved";
			return false;
	}

	return true;
}

bool SunPuzzle::Update() {
	return true;
}

void SunPuzzle::Render() {
	// background
	renderer_.Copy(background_, SDL2pp::NullOpt, SDL2pp::Rect(0, 0, 640, 480));

	// buttons
	for (int i = 0; i < 6; i++) {
		renderer_.Copy(
				buttons_,
				SDL2pp::Rect(42 * i, 42 * states_[i], 42, 42),
				SDL2pp::Rect(button_locations_[i].x, button_locations_[i].y, 42, 42)
			);
	}

	// temperature
	int temperature = 0;
	for (int i = 0; i < 6; i++)
		temperature += states_[i];

	temperature = std::min(temperature * 5 / 12, 5);

	int phase = (SDL_GetTicks() / 1000) % 6;

	renderer_.Copy(
			temperature_,
			SDL2pp::Rect(100 * temperature, 120 * phase, 100, 120),
			SDL2pp::Rect(264, 192, 100, 120)
		);
}
