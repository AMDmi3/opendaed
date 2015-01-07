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

#include "artemispuzzle.hh"

#include "datamanager.hh"

const std::vector<ArtemisPuzzle::PieceType> ArtemisPuzzle::initial_pieces_ = {
	DR, UL, DR, VE, UL, DR, UL, HO, UL,
	DR, DL, DR, UR, VE, HO, DR, DR, UL,
	UL, DR, VE, DR, UR, DR, UL, VE, HO,
	DR, HO, DR, UL,     UR, HO, HO, DL,
	UR, VE, UL, UR, DR, UL, VE, UR, DL,
	DR, UL, HO, UL, VE, DR, DR, DR, VE,
	DR, HO, DR, UR, UL, UL, HO, DR, UR,
};

ArtemisPuzzle::ArtemisPuzzle(SDL2pp::Renderer& renderer, const DataManager& datamanager)
	: renderer_(renderer),
	  background_(renderer, datamanager.GetPath("images/party/backgrnd.rle")),
	  pieces_inactive_(renderer, datamanager.GetPath("images/party/ctrw.rle")),
	  pieces_(initial_pieces_) {
}

ArtemisPuzzle::~ArtemisPuzzle() {
}

void ArtemisPuzzle::ProcessEvent(const SDL_Event& event) {
}

void ArtemisPuzzle::Update(unsigned int ticks) {
}

void ArtemisPuzzle::Render() {
	// Background
	renderer_.Copy(background_, SDL2pp::NullOpt, SDL2pp::Rect(0, 0, 640, 480));

	// Pieces
	int n = 0;
	for (int y = 0; y < 7; y++) {
		// Uneven space between rows; this is actuaaly more correct than
		// origial game - it contains single pixel errors in piece placements
		int y_extra_offset = 0;
		if (y > 1)
			y_extra_offset++;
		if (y > 3)
			y_extra_offset++;

		for (int x = 0; x < 9; x++) {
			if (x == 4 && y == 3) // central piece
				continue;

			renderer_.Copy(
					pieces_inactive_,
					SDL2pp::Rect(0 + 32 * (int)pieces_[n], 0, 32, 32),
					SDL2pp::Rect(45 + x * 64, 36 + y_extra_offset + y * 61, 32, 32)
				);

			n++;
		}
	}
}
