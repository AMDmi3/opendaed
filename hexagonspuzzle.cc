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

#include "hexagonspuzzle.hh"

#include "datamanager.hh"
#include "logger.hh"

// pieces are numbered clockwise from top right
const std::array<SDL2pp::Point, 7> HexagonsPuzzle::piece_locations_ = { {
	{ 328, 111 },
	{ 382, 202 },
	// those two are not really a piece location (one from puz2piec.bmp)
	// but a tweak for lines, which are offset from hexagon by 2 pixels
	{ 328, 293 + 2 },
	{ 229, 293 + 2 },
	{ 177, 202 },
	{ 229, 111 },
	{ 281, 202 },
} };

// vertices are numbered clockwise from top one
const std::array<SDL2pp::Point, 6> HexagonsPuzzle::vertex_coords_ = { {
	{ 39, 2 },
	{ 71, 20 },
	{ 71, 57 },
	{ 39, 75 },
	{ 7, 57 },
	{ 7, 20 },
} };

// this is level data:
// array of per-level
//   array of per-hexagon
//     vector of vertex number pairs
// each pair represents one line on a hexagon
const std::array<std::array<HexagonsPuzzle::LineSet, 6>, 6> HexagonsPuzzle::level_hex_lines_ = { {
	// level 0
	{ {
		{ },
		{ { 5, 0 } },
		{ { 5, 0 }, { 4, 0 } },
		{ { 5, 0 }, { 4, 0 }, { 3, 0 } },
		{ { 5, 0 }, { 4, 0 }, { 3, 0 }, { 2, 0 } },
		{ { 5, 0 }, { 4, 0 }, { 3, 0 }, { 2, 0 }, { 1, 0 } },
	} },
	// level 1
	{ {
		{ { 4, 5 }, { 5, 0 }, { 4, 0 }, { 3, 0 } },
		{ { 4, 5 }, { 5, 0 }, { 4, 0 }, { 2, 5 } },
		{ { 4, 5 }, { 5, 0 }, { 4, 0 }, { 1, 4 } },
		{ { 4, 5 }, { 5, 0 }, { 4, 0 } },
		{ { 4, 5 }, { 5, 0 }, { 4, 0 } },
		{ { 4, 5 }, { 5, 0 }, { 4, 0 } },
	} },
	// level 2
	{ {
		{ { 5, 1 }, { 1, 4 }, { 4, 5 } },
		{ { 5, 1 }, { 1, 4 }, { 4, 5 } },
		{ { 5, 1 }, { 1, 4 }, { 4, 5 } },
		{ { 5, 1 }, { 1, 4 }, { 4, 5 } },
		{ { 5, 1 }, { 1, 4 }, { 4, 5 } },
		{ { 5, 1 }, { 1, 4 }, { 4, 5 } },
	} },
	// level 3
	{ {
		{ { 0, 1 } },
		{ { 0, 1 }, { 1, 3 }, { 3, 5 }, { 5, 1 } },
		{ { 0, 1 } },
		{ { 0, 1 }, { 1, 3 }, { 3, 5 }, { 5, 1 } },
		{ { 0, 1 } },
		{ { 0, 1 }, { 0, 3 }, { 1, 4 }, { 2, 5 } },
	} },
	// level 4
	{ {
		{ { 0, 2 }, { 2, 4 }, { 4, 0 } },
		{ { 0, 3 }, { 2, 5 } },
		{ { 1, 2 }, { 4, 5 }, { 5, 0 } },
		{ { 0, 2 }, { 2, 4 }, { 4, 0 } },
		{ { 0, 3 } },
		{ { 2, 3 }, { 4, 5 }, { 5, 0 } },
	} },
	// level 5
	{ {
		{ { 5, 2 }, { 2, 4 }, { 4, 5 } },
		{ { 5, 2 }, { 2, 4 }, { 4, 5 } },
		{ { 5, 0 }, { 0, 1 }, { 1, 5 } },
		{ { 0, 2 }, { 2, 4 }, { 4, 0 } },
		{ { 1, 2 }, { 2, 4 }, { 4, 1 } },
		{ { 1, 2 }, { 2, 4 }, { 4, 1 } },
	} },
} };

void HexagonsPuzzle::SetupLevel(int level) {
	level_ = level;
	last_touched_piece_ = -1;

	hex_lines_ = level_hex_lines_[level];

	RecalculateSummary();
}

void HexagonsPuzzle::ThickLine(const SDL2pp::Point& v1, const SDL2pp::Point& v2) {
	renderer_.DrawLine(v1, v2);
	renderer_.DrawLine(v1 + SDL2pp::Point(1, 0), v2 + SDL2pp::Point(1, 0));
	renderer_.DrawLine(v1 + SDL2pp::Point(0, 1), v2 + SDL2pp::Point(0, 1));
	renderer_.DrawLine(v1 + SDL2pp::Point(1, 1), v2 + SDL2pp::Point(1, 1));
}

void HexagonsPuzzle::RecalculateSummary() {
	all_lines_.clear();
	last_touched_lines_.clear();

	for (int p = 0; p < 6; p++) {
		for (auto& line : hex_lines_[p]) {
			all_lines_.insert(std::make_pair(std::min(line.first, line.second), std::max(line.first, line.second)));
			if (last_touched_piece_ == p)
				last_touched_lines_.insert(std::make_pair(std::min(line.first, line.second), std::max(line.first, line.second)));
		}
	}
}

HexagonsPuzzle::HexagonsPuzzle(SDL2pp::Renderer& renderer, const DataManager& datamanager)
	: renderer_(renderer),
	  background_(renderer, datamanager.GetPath("images/plaphex/puzz2bg.bmp")),
	  levels_(renderer, datamanager.GetPath("images/plaphex/puz2goal.bmp")),
	  levels_hl_(renderer, datamanager.GetPath("images/plaphex/puz2glhl.bmp")),
	  pieces_(renderer, datamanager.GetPath("images/plaphex/puz2peic.bmp")),
	  chaser_(renderer, datamanager.GetPath("images/plaphex/chaser.rle")) {
	Log("puzzle") << "starting hexagons puzzle";

	SetupLevel(0);
}

HexagonsPuzzle::~HexagonsPuzzle() {
}

bool HexagonsPuzzle::ProcessEvent(const SDL_Event& event) {
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		int npiece;
		for (auto pieceloc = piece_locations_.begin(); pieceloc != piece_locations_.end(); pieceloc++) {
			if (!SDL2pp::Rect(pieceloc->x, pieceloc->y, 80, 80).Contains(SDL2pp::Point(event.button.x, event.button.y)))
				continue;

			npiece = pieceloc - piece_locations_.begin();
			break;
		}

		if (npiece > 5)
			return true;

		for (auto& line : hex_lines_[npiece]) {
			line.first = (line.first + 1) % 6;
			line.second = (line.second + 1) % 6;
		}

		last_touched_piece_ = npiece;

		RecalculateSummary();
	}

	return true;
}

bool HexagonsPuzzle::Update() {
	return true;
}

void HexagonsPuzzle::Render() {
	// background
	renderer_.Copy(background_, SDL2pp::NullOpt, SDL2pp::Rect(0, 0, 640, 480));

	// pieces
	renderer_.SetDrawColor(0, 255, 0);
	for (int p = 0; p < 6; p++) {
		for (auto& line : hex_lines_[p]) {
			SDL2pp::Point v1 = piece_locations_[p] + vertex_coords_[line.first];
			SDL2pp::Point v2 = piece_locations_[p] + vertex_coords_[line.second];

			ThickLine(v1, v2);
		}
	}

	// central piece
	for (auto& line : all_lines_) {
		SDL2pp::Point v1 = piece_locations_[6] + vertex_coords_[line.first];
		SDL2pp::Point v2 = piece_locations_[6] + vertex_coords_[line.second];

		if (last_touched_lines_.find(line) != last_touched_lines_.end())
			renderer_.SetDrawColor(255, 0, 0);
		else
			renderer_.SetDrawColor(0, 255, 0);

		ThickLine(v1, v2);
	}
}
