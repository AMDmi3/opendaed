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
#include <random>

#include <SDL2/SDL_timer.h>

#include "artemispuzzle.hh"

#include "datamanager.hh"
#include "logger.hh"

const std::array<ArtemisPuzzle::PieceType, ArtemisPuzzle::NUM_PIECES> ArtemisPuzzle::initial_pieces_ = { {
	DR, UL, DR, VE, UL, DR, UL, HO, UL,
	DR, DL, DR, UR, VE, HO, DR, DR, UL,
	UL, DR, VE, DR, UR, DR, UL, VE, HO,
	DR, HO, DR, UL, DR, UR, HO, HO, DL, // central piece is not used
	UR, VE, UL, UR, DR, UL, VE, UR, DL,
	DR, UL, HO, UL, VE, DR, DR, DR, VE,
	DR, HO, DR, UR, UL, UL, HO, DR, UR,
} };

// this is bug2bug compatibility, but actually alignment may be
// improved; look at connection lines between pieces
const std::array<int, ArtemisPuzzle::NUM_COLUMNS> ArtemisPuzzle::col_offsets_ = { {
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

const std::array<int, ArtemisPuzzle::NUM_ROWS> ArtemisPuzzle::row_offsets_ = { {
	36 + 61 * 0,
	36 + 61 * 1,
	36 + 61 * 2 + 1,
	36 + 61 * 3 + 1,
	36 + 61 * 4 + 1,
	36 + 61 * 5 + 1,
	36 + 61 * 6 + 2,
} };

const std::array<SDL2pp::Point, 13> ArtemisPuzzle::light_locations_ = { {
#ifdef BUG2BUG
	// original lights are a bit displaced
	{ 10, 264 },
	{ 83, 195 },
	{ 84, 318 },
	{ 148, 257 },
	{ 212, 73 },
	{ 276, 379 },
	{ 339, 73 },
	{ 339, 135 },
	{ 404, 377 },
	{ 468, 195 },
	{ 519, 6 },
	{ 532, 134 },
	{ 531, 256 },
#else
	{ 11, 265 },
	{ 84, 196 },
	{ 84, 318 },
	{ 148, 257 },
	{ 212, 73 },
	{ 275, 379 },
	{ 339, 73 },
	{ 339, 135 },
	{ 403, 379 },
	{ 468, 195 },
	{ 519, 6 },
	{ 531, 135 },
	{ 531, 257 },
#endif
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

	PropagateActivity(CENTRAL_COLUMN, CENTRAL_ROW, LEFT);
	PropagateActivity(CENTRAL_COLUMN, CENTRAL_ROW, RIGHT);
	PropagateActivity(CENTRAL_COLUMN, CENTRAL_ROW, UP);
	PropagateActivity(CENTRAL_COLUMN, CENTRAL_ROW, DOWN);
}

void ArtemisPuzzle::PropagateActivity(int x, int y, Direction dir) {
	while (1) {
		switch (dir) {
		case LEFT:  x--; break;
		case RIGHT: x++; break;
		case UP:    y--; break;
		case DOWN:  y++; break;
		}

		if (x == -1 && y == CENTRAL_ROW)
			activated_systems_ |= AUX_BIO_SYSTEMS;
		if (x == NUM_COLUMNS && y == CENTRAL_ROW)
			activated_systems_ |= AUX_CONTROL_SYSTEM;
		if (x == CENTRAL_COLUMN && y == -1)
			activated_systems_ |= AUX_AIR_REFILTRATION;
		if (x == CENTRAL_COLUMN && y == NUM_ROWS)
			activated_systems_ |= AUX_POWER_GRID;

		if (x < 0 || y < 0 || x >= NUM_COLUMNS || y >= NUM_ROWS)
			return; // out of bounds
		if (x == CENTRAL_COLUMN && y == CENTRAL_ROW)
			return; // back into center

		PieceType type = pieces_[y * NUM_COLUMNS + x];

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
		active_[y * NUM_COLUMNS + x] = true;

		// activate line from prev piece
		switch (prev_dir) {
		case LEFT:  horizontal_lines_[y * (NUM_COLUMNS - 1) + x] = true; break;
		case RIGHT: horizontal_lines_[y * (NUM_COLUMNS - 1) + x - 1] = true; break;
		case UP:    vertical_lines_[y * NUM_COLUMNS + x] = true; break;
		case DOWN:  vertical_lines_[(y - 1) * NUM_COLUMNS + x] = true; break;
		}
	}
}

ArtemisPuzzle::ArtemisPuzzle(SDL2pp::Renderer& renderer, const DataManager& datamanager)
	: renderer_(renderer),
	  background_(renderer, datamanager.GetPath("images/party/backgrnd.rle")),
	  pieces_inactive_(renderer, datamanager.GetPath("images/party/ctrw.rle")),
	  pieces_active_(renderer, datamanager.GetPath("images/party/ctrr.rle")),
	  line_horiz_(renderer, datamanager.GetPath("images/party/horz.rle")),
	  line_vert_(renderer, datamanager.GetPath("images/party/vert.bmp")),
	  core_(renderer, datamanager.GetPath("images/party/p1circ.bmp")),
	  lights_(renderer, datamanager.GetPath("images/party/lights.bmp")),
	  aux1_(renderer, datamanager.GetPath("images/party/aux1.rle")),
	  aux2_(renderer, datamanager.GetPath("images/party/aux2.rle")),
	  aux3_(renderer, datamanager.GetPath("images/party/aux3.rle")),
	  aux4_(renderer, datamanager.GetPath("images/party/aux4.rle")),
	  main1_(renderer, datamanager.GetPath("images/party/main1.rle")),
	  main2_(renderer, datamanager.GetPath("images/party/main2.rle")),
	  main3_(renderer, datamanager.GetPath("images/party/main3.rle")),
	  main4_(renderer, datamanager.GetPath("images/party/main4.rle")),
	  greyblit_(renderer, datamanager.GetPath("images/party/greyblit.bmp")),
	  pieces_(initial_pieces_) {
	RecalculateActivePieces();

	last_frame_time_ = SDL_GetTicks();
	time_left_[0] = TIME_LIMIT_TICKS;
	time_left_[1] = TIME_LIMIT_TICKS;
	time_left_[2] = TIME_LIMIT_TICKS;
	time_left_[3] = TIME_LIMIT_TICKS;

	Log("puzzle") << "starting artemis puzzle";
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
		int column = col_offset - col_offsets_.begin();
		int row = row_offset - row_offsets_.begin();
		if (column >= NUM_COLUMNS || row >= NUM_ROWS)
			return true;
		int npiece = row * NUM_COLUMNS + column;

		// rotate piece
#ifdef BUG2BUG
		bool clockwise = true;
#else
		bool clockwise = event.button.button != SDL_BUTTON_RIGHT;
#endif
		if (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT || event.button.button == SDL_BUTTON_MIDDLE)
			pieces_[npiece] = RotatePiece(pieces_[npiece], clockwise);

		RecalculateActivePieces();
	}

	if (activated_systems_ == ALL_SYSTEMS) {
		Log("puzzle") << "  all systems connected, congratulations!";
		return false;
	}

	return true;
}

bool ArtemisPuzzle::Update() {
	unsigned int ticks = SDL_GetTicks();
	unsigned int delta = ticks - last_frame_time_;
	last_frame_time_ = ticks;

	if (!(activated_systems_ & AUX_BIO_SYSTEMS))
		time_left_[3] -= delta;
	if (!(activated_systems_ & AUX_POWER_GRID))
		time_left_[2] -= delta;
	if (!(activated_systems_ & AUX_CONTROL_SYSTEM))
		time_left_[1] -= delta;
	if (!(activated_systems_ & AUX_AIR_REFILTRATION))
		time_left_[0] -= delta;

	for (int i = 0; i < 4; i++) {
		if (time_left_[i] <= 0) {
			Log("puzzle") << "  your time is out, you're dead";
			return false;
		}
	}

	return true;
}

void ArtemisPuzzle::Render() {
	// Background
	renderer_.Copy(background_, SDL2pp::NullOpt, SDL2pp::Rect(0, 0, 640, 480));

	// Pieces
	int n = 0;
	for (int y = 0; y < NUM_ROWS; y++) {
		for (int x = 0; x < NUM_COLUMNS; x++, n++) {
			if (n == CENTRAL_PIECE_NUMBER)
				continue;

			// this is also bug2bug compatible; the thing is that ctrr.bmp
			// has vertical and horizontal lines misplaced a bit
			renderer_.Copy(
					active_[n] ? pieces_active_ : pieces_inactive_,
					SDL2pp::Rect(32 * (int)pieces_[n], 0, 32, 32),
					SDL2pp::Rect(col_offsets_[x], row_offsets_[y], 32, 32)
				);
		}
	}

	// Vertical lines
	n = 0;
	for (int y = 0; y < NUM_ROWS - 1; y++) {
		for (int x = 0; x < NUM_COLUMNS; x++, n++) {
			if (n == NUM_VERTICAL_LINES / 2 || n == NUM_VERTICAL_LINES / 2 - 1) // central lines
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
	for (int y = 0; y < NUM_ROWS; y++) {
		for (int x = 0; x < NUM_COLUMNS - 1; x++, n++) {
			if (n == NUM_HORIZONTAL_LINES / 2 || n == NUM_HORIZONTAL_LINES / 2 - 1) // central lines
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

	// connected systems indication
	if (activated_systems_ & AUX_BIO_SYSTEMS)
		renderer_.Copy(aux4_, SDL2pp::NullOpt, SDL2pp::Rect(38, 460, 120, 12));
	if (activated_systems_ & AUX_POWER_GRID)
		renderer_.Copy(aux3_, SDL2pp::NullOpt, SDL2pp::Rect(177, 460, 120, 12));
	if (activated_systems_ & AUX_CONTROL_SYSTEM)
		renderer_.Copy(aux2_, SDL2pp::NullOpt, SDL2pp::Rect(320, 460, 120, 12));
	if (activated_systems_ & AUX_AIR_REFILTRATION)
		renderer_.Copy(aux1_, SDL2pp::NullOpt, SDL2pp::Rect(469, 460, 120, 12));

	// animated stuff: core
	int seconds = SDL_GetTicks() / 1000;

	int corephase = seconds % 15;
	renderer_.Copy(
			core_,
			SDL2pp::Rect(96 * (corephase % 5), 92 * (corephase / 5), 96, 92),
			SDL2pp::Rect(270, 191, 96, 92)
		);

	// animated stuff: blinking lighs
	// fun fact: if you use minstd_rand0 or minstd_rand PRNG here
	// you'll notice some lights not changing their color (or changing
	// with long period), likely because of poor quality of PRNG
	std::mt19937 rnd;
	rnd.seed(seconds);
	for (auto& coords : light_locations_) {
		renderer_.Copy(
				lights_,
				SDL2pp::Rect(18 * (rnd() % 3), 0, 18, 18),
				SDL2pp::Rect(coords.x, coords.y, 18, 18)
			);
	}

	// animated stuff: useless messages
	if (activated_systems_ == ALL_SYSTEMS) {
		renderer_.Copy(main4_, SDL2pp::NullOpt, SDL2pp::Rect(40, 6, 250, 12));
	} else {
		switch (seconds % 4) {
		case 0: break;
		case 1: renderer_.Copy(main1_, SDL2pp::NullOpt, SDL2pp::Rect(40, 6, 250, 12)); break;
		case 2: renderer_.Copy(main2_, SDL2pp::NullOpt, SDL2pp::Rect(40, 6, 250, 12)); break;
		case 3: renderer_.Copy(main3_, SDL2pp::NullOpt, SDL2pp::Rect(40, 6, 250, 12)); break;
		}
	}

	// time indication
	for (int sys = 0; sys < 4; sys++) {
		int numfills = std::min(34, 35 * (TIME_LIMIT_TICKS - time_left_[sys]) / TIME_LIMIT_TICKS);
		for (int fill = 0; fill < numfills; fill++)
			renderer_.Copy(greyblit_, SDL2pp::NullOpt, SDL2pp::Rect(7 + sys * 5, 11 + fill * 6, 5, 5));
	}
}
