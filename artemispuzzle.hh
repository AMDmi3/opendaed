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

#ifndef ARTEMISPUZZLE_HH
#define ARTEMISPUZZLE_HH

#include <array>

#include <SDL2pp/Texture.hh>
#include <SDL2pp/Renderer.hh>

#include "screen.hh"

class DataManager;

class ArtemisPuzzle : public Screen {
private:
	enum PieceType {
		DR = 0,
		DL = 1,
		UL = 2,
		UR = 3,
		HO = 4,
		VE = 5,
	};

	enum Direction {
		LEFT,
		RIGHT,
		UP,
		DOWN,
	};

	enum Systems {
		AUX_BIO_SYSTEMS = 1,
		AUX_CONTROL_SYSTEM = 2,
		AUX_AIR_REFILTRATION = 4,
		AUX_POWER_GRID = 8,

		ALL_SYSTEMS = AUX_BIO_SYSTEMS | AUX_CONTROL_SYSTEM | AUX_AIR_REFILTRATION | AUX_POWER_GRID,
	};

	enum Constants {
		NUM_COLUMNS = 9,
		NUM_ROWS = 7,

		NUM_PIECES = NUM_COLUMNS * NUM_ROWS,
		CENTRAL_PIECE_NUMBER = NUM_PIECES / 2,

		NUM_HORIZONTAL_LINES = (NUM_COLUMNS - 1) * NUM_ROWS,
		NUM_VERTICAL_LINES = NUM_COLUMNS * (NUM_ROWS - 1),

		CENTRAL_COLUMN = NUM_COLUMNS / 2,
		CENTRAL_ROW = NUM_ROWS / 2,

		TIME_LIMIT_TICKS = 280 * 1000,
	};

private:
	static const std::array<PieceType, NUM_PIECES> initial_pieces_;
	static const std::array<int, NUM_COLUMNS> col_offsets_;
	static const std::array<int, NUM_ROWS> row_offsets_;
	static const std::array<SDL2pp::Point, 13> light_locations_;

private:
	SDL2pp::Renderer& renderer_;

	// Textures
	SDL2pp::Texture background_;
	SDL2pp::Texture pieces_inactive_;
	SDL2pp::Texture pieces_active_;
	SDL2pp::Texture line_horiz_;
	SDL2pp::Texture line_vert_;
	SDL2pp::Texture core_;
	SDL2pp::Texture lights_;
	SDL2pp::Texture aux1_;
	SDL2pp::Texture aux2_;
	SDL2pp::Texture aux3_;
	SDL2pp::Texture aux4_;
	SDL2pp::Texture main1_;
	SDL2pp::Texture main2_;
	SDL2pp::Texture main3_;
	SDL2pp::Texture main4_;
	SDL2pp::Texture greyblit_;

private:
	std::array<PieceType, NUM_PIECES> pieces_;
	std::array<bool, NUM_PIECES> active_;

	std::array<bool, NUM_HORIZONTAL_LINES> horizontal_lines_;
	std::array<bool, NUM_VERTICAL_LINES> vertical_lines_;

	int activated_systems_;
	int time_left_[4];
	unsigned int last_frame_time_;

private:
	PieceType RotatePiece(PieceType type, bool clockwise = true);
	void RecalculateActivePieces();
	void PropagateActivity(int x, int y, Direction dir);

public:
	ArtemisPuzzle(SDL2pp::Renderer& renderer, const DataManager& datamanager);
	virtual ~ArtemisPuzzle();

	bool ProcessEvent(const SDL_Event& event) override;
	bool Update() override;
	void Render() override;
};

#endif // ARTEMISPUZZLE_HH
