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

#ifndef HEXAGONSPUZZLE_HH
#define HEXAGONSPUZZLE_HH

#include <array>
#include <vector>
#include <set>

#include <SDL2pp/Texture.hh>
#include <SDL2pp/Renderer.hh>

#include "screen.hh"

class DataManager;

class HexagonsPuzzle : public Screen {
private:
	typedef std::pair<int, int> LineDesc;
	typedef std::vector<LineDesc> LineSet;


private:
	static const std::array<SDL2pp::Point, 7> piece_locations_;
	static const std::array<SDL2pp::Point, 6> vertex_coords_;

	static const std::array<std::array<LineSet, 6>, 6> level_hex_lines_;

private:
	SDL2pp::Renderer& renderer_;

	// Textures
	SDL2pp::Texture background_;
	SDL2pp::Texture levels_;
	SDL2pp::Texture levels_hl_;
	SDL2pp::Texture pieces_;
	SDL2pp::Texture chaser_;

private:
	int level_;
	int last_touched_piece_;

	std::array<LineSet, 6> hex_lines_;

	std::set<LineDesc> last_touched_lines_;
	std::set<LineDesc> all_lines_;

private:
	void ThickLine(const SDL2pp::Point& v1, const SDL2pp::Point& v2);
	void SetupLevel(int level);
	void RecalculateSummary();

public:
	HexagonsPuzzle(SDL2pp::Renderer& renderer, const DataManager& datamanager);
	virtual ~HexagonsPuzzle();

	bool ProcessEvent(const SDL_Event& event) override;
	bool Update() override;
	void Render() override;
};

#endif // HEXAGONSPUZZLE_HH
