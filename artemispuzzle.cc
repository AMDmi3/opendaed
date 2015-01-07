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

ArtemisPuzzle::ArtemisPuzzle(SDL2pp::Renderer& renderer, const DataManager& datamanager)
	: renderer_(renderer),
	  background_(renderer, datamanager.GetPath("images/party/backgrnd.rle")) {
}

ArtemisPuzzle::~ArtemisPuzzle() {
}

void ArtemisPuzzle::ProcessEvent(const SDL_Event& event) {
}

void ArtemisPuzzle::Update(unsigned int ticks) {
}

void ArtemisPuzzle::Render() {
	renderer_.Copy(background_, SDL2pp::NullOpt, SDL2pp::Rect(0, 0, 640, 480));
}
