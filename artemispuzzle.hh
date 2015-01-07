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

private:
	static const std::array<PieceType, 62> initial_pieces_;
	static const std::array<int, 9> col_offsets_;
	static const std::array<int, 7> row_offsets_;

private:
	SDL2pp::Renderer& renderer_;

	// Textures
	SDL2pp::Texture background_;
	SDL2pp::Texture pieces_inactive_;

private:
	std::array<PieceType, 62> pieces_;

private:
	PieceType RotatePiece(PieceType type, bool clockwise = true);

public:
	ArtemisPuzzle(SDL2pp::Renderer& renderer, const DataManager& datamanager);
	virtual ~ArtemisPuzzle();

	void ProcessEvent(const SDL_Event& event) override;
	void Update(unsigned int ticks) override;
	void Render() override;
};

#endif // ARTEMISPUZZLE_HH
