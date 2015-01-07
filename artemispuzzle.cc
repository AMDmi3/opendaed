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

#include "artemispuzzle.hh"

#include "datamanager.hh"

const std::array<ArtemisPuzzle::PieceType, 62> ArtemisPuzzle::initial_pieces_ = { {
	DR, UL, DR, VE, UL, DR, UL, HO, UL,
	DR, DL, DR, UR, VE, HO, DR, DR, UL,
	UL, DR, VE, DR, UR, DR, UL, VE, HO,
	DR, HO, DR, UL,     UR, HO, HO, DL,
	UR, VE, UL, UR, DR, UL, VE, UR, DL,
	DR, UL, HO, UL, VE, DR, DR, DR, VE,
	DR, HO, DR, UR, UL, UL, HO, DR, UR,
} };

// this is bug2bug compatibility, but actually alignment may be
// improved; look at connection lines between pieces
const std::array<int, 9> ArtemisPuzzle::col_offsets_ = { {
	44 + 64 * 0,
	44 + 64 * 1,
	44 + 64 * 2,
	44 + 64 * 3,
	44 + 64 * 4,
	44 + 64 * 5,
	44 + 64 * 6,
	44 + 64 * 7,
	44 + 64 * 8,
} };

const std::array<int, 7> ArtemisPuzzle::row_offsets_ = { {
	36 + 61 * 0,
	36 + 61 * 1,
	36 + 61 * 2 + 1,
	36 + 61 * 3 + 1,
	36 + 61 * 4 + 1,
	36 + 61 * 5 + 1,
	36 + 61 * 6 + 2,
} };

ArtemisPuzzle::PieceType ArtemisPuzzle::RotatePiece(PieceType type, bool clockwise) {
	switch (type) {
	case DR: return clockwise ? DL : UR;
	case DL: return clockwise ? UL : DR;
	case UL: return clockwise ? UR : DL;
	case UR: return clockwise ? DR : UL;
	case HO: return VE;
	case VE: return HO;
	}
}

void ArtemisPuzzle::RecalculateActivePieces() {
	std::fill(active_.begin(), active_.end(), false);
	std::fill(horizontal_lines_.begin(), horizontal_lines_.end(), false);
	std::fill(vertical_lines_.begin(), vertical_lines_.end(), false);
	activated_systems_ = 0;

	PropagateActivity(4, 3, LEFT);
	PropagateActivity(4, 3, RIGHT);
	PropagateActivity(4, 3, UP);
	PropagateActivity(4, 3, DOWN);
}

void ArtemisPuzzle::PropagateActivity(int x, int y, Direction dir) {
	while (1) {
		switch (dir) {
		case LEFT:  x--; break;
		case RIGHT: x++; break;
		case UP:    y--; break;
		case DOWN:  y++; break;
		}

		if (x == -1 && y == 3)
			activated_systems_ |= AUX_BIO_SYSTEMS;
		if (x == 9 && y == 3)
			activated_systems_ |= AUX_CONTROL_SYSTEM;
		if (x == 4 && y == -1)
			activated_systems_ |= AUX_AIR_REFILTRATION;
		if (x == 4 && y == 7)
			activated_systems_ |= AUX_POWER_GRID;

		if (x < 0 || y < 0 || x > 8 || y > 6)
			return; // out of bounds
		if (x == 4 && y == 3)
			return; // back into center

		PieceType type = pieces_[PieceNum(x, y)];

		Direction prev_dir = dir;

		switch (dir) {
		case LEFT:
			switch (type) {
			case DR: dir = DOWN; break;
			case UR: dir = UP;   break;
			case HO:             break;
			default:
				return;
			}
			break;
		case RIGHT:
			switch (type) {
			case DL: dir = DOWN; break;
			case UL: dir = UP;   break;
			case HO:             break;
			default:
				return;
			}
			break;
		case UP:
			switch (type) {
			case DR: dir = RIGHT; break;
			case DL: dir = LEFT;  break;
			case VE:              break;
			default:
				return;
			}
			break;
		case DOWN:
			switch (type) {
			case UL: dir = LEFT;  break;
			case UR: dir = RIGHT; break;
			case VE:              break;
			default:
				return;
			}
			break;
		}

		// activate piece
		active_[PieceNum(x, y)] = true;

		// activate line from prev piece
		switch (prev_dir) {
		case LEFT:  horizontal_lines_[y * 8 + x] = true; break;
		case RIGHT: horizontal_lines_[y * 8 + x - 1] = true; break;
		case UP:    vertical_lines_[y * 9 + x] = true; break;
		case DOWN:  vertical_lines_[(y - 1) * 9 + x] = true; break;
		}
	}
}

int ArtemisPuzzle::PieceNum(int x, int y) {
	int npiece = x + y * 9;

	// ignore central piece
	if (npiece == 31)
		return -1;

	if (npiece > 31)
		npiece--;

	assert(npiece >= 0 && npiece < 62);

	return npiece;
}

ArtemisPuzzle::ArtemisPuzzle(SDL2pp::Renderer& renderer, const DataManager& datamanager)
	: renderer_(renderer),
	  background_(renderer, datamanager.GetPath("images/party/backgrnd.rle")),
	  pieces_inactive_(renderer, datamanager.GetPath("images/party/ctrw.rle")),
	  pieces_active_(renderer, datamanager.GetPath("images/party/ctrr.rle")),
	  line_horiz_(renderer, datamanager.GetPath("images/party/horz.rle")),
	  line_vert_(renderer, datamanager.GetPath("images/party/vert.bmp")),
	  pieces_(initial_pieces_) {
	RecalculateActivePieces();
}

ArtemisPuzzle::~ArtemisPuzzle() {
}

bool ArtemisPuzzle::ProcessEvent(const SDL_Event& event) {
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		// get closest next row/column
		auto col_offset = std::upper_bound(col_offsets_.begin(), col_offsets_.end(), event.button.x);
		auto row_offset = std::upper_bound(row_offsets_.begin(), row_offsets_.end(), event.button.y);

		// fix row/col and check we're in bounds
		if (col_offset-- == col_offsets_.begin() || event.button.x - *col_offset >= 32)
			return true;
		if (row_offset-- == row_offsets_.begin() || event.button.y - *row_offset >= 32)
			return true;

		// calculate piece number
		int npiece = PieceNum(col_offset - col_offsets_.begin(), row_offset - row_offsets_.begin());
		if (npiece == -1)
			return true;

		// rotate piece
		if (event.button.button == SDL_BUTTON_LEFT)
			pieces_[npiece] = RotatePiece(pieces_[npiece], true);
		if (event.button.button == SDL_BUTTON_RIGHT)
			pieces_[npiece] = RotatePiece(pieces_[npiece], false);

		RecalculateActivePieces();
	}

	return activated_systems_ != ALL_SYSTEMS;
}

bool ArtemisPuzzle::Update(unsigned int) {
	return true;
}

void ArtemisPuzzle::Render() {
	// Background
	renderer_.Copy(background_, SDL2pp::NullOpt, SDL2pp::Rect(0, 0, 640, 480));

	// Pieces
	int n = 0;
	for (int y = 0; y < 7; y++) {
		for (int x = 0; x < 9; x++) {
			if (x == 4 && y == 3) // skip central piece
				continue;

			// this is also bug2bug compatible; the thing is that ctrr.bmp
			// has vertical and horizontal lines misplaced a bit
			renderer_.Copy(
					active_[n] ? pieces_active_ : pieces_inactive_,
					SDL2pp::Rect(32 * (int)pieces_[n], 0, 32, 32),
					SDL2pp::Rect(col_offsets_[x], row_offsets_[y], 32, 32)
				);

			n++;
		}
	}

	// Vertical lines
	n = 0;
	for (int y = 0; y < 6; y++) {
		for (int x = 0; x < 9; x++, n++) {
			if (x == 4 && (y == 2 || y == 3)) // line to core
				continue;
			if (vertical_lines_[n]) {
				renderer_.Copy(
						line_vert_,
						SDL2pp::Rect(6, 0, 6, row_offsets_[y + 1] - row_offsets_[y] - 32),
						SDL2pp::Rect(col_offsets_[x] + 13, row_offsets_[y] + 32, 6, row_offsets_[y + 1] - row_offsets_[y] - 32)
					);
			}
		}
	}

	// Horizontal lines
	n = 0;
	for (int y = 0; y < 7; y++) {
		for (int x = 0; x < 8; x++, n++) {
			if (y == 3 && (x == 3 || x == 4)) // line to core
				continue;
			if (horizontal_lines_[n]) {
				renderer_.Copy(
						line_horiz_,
						SDL2pp::Rect(0, 6, col_offsets_[x + 1] - col_offsets_[x] - 32, 6),
						SDL2pp::Rect(col_offsets_[x] + 32, row_offsets_[y] + 13, col_offsets_[y + 1] - col_offsets_[y] - 32, 6)
					);
			}
		}
	}
}
