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

private:
	static const std::array<PieceType, 62> initial_pieces_;
	static const std::array<int, 9> col_offsets_;
	static const std::array<int, 7> row_offsets_;

private:
	SDL2pp::Renderer& renderer_;

	// Textures
	SDL2pp::Texture background_;
	SDL2pp::Texture pieces_inactive_;
	SDL2pp::Texture pieces_active_;

private:
	std::array<PieceType, 62> pieces_;
	std::array<bool, 62> active_;

	int activated_systems_;

private:
	PieceType RotatePiece(PieceType type, bool clockwise = true);
	void RecalculateActivePieces();
	void PropagateActivity(int x, int y, Direction dir);

	int PieceNum(int x, int y);

public:
	ArtemisPuzzle(SDL2pp::Renderer& renderer, const DataManager& datamanager);
	virtual ~ArtemisPuzzle();

	bool ProcessEvent(const SDL_Event& event) override;
	bool Update(unsigned int ticks) override;
	void Render() override;
};

#endif // ARTEMISPUZZLE_HH
