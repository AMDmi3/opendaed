/*
 * Copyright (C) 2014 Dmitry Marakasov
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

#include <iostream>

#include <SDL2/SDL.h>

#include <SDL2pp/SDL2pp.hh>

#include "datamanager.hh"

void usage(const char* progname) {
	std::cerr << "Usage: " << progname << " <path to data directory>" << std::endl;
}

int realmain(int argc, char** argv) {
	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}

	// Data manager
	DataManager data_manager;
	data_manager.ScanDir(argv[1]);

	// SDL stuff
	SDL2pp::SDL sdl(SDL_INIT_VIDEO);
	SDL2pp::Window window("OpenDaed", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);
	SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

	while (1) {
		// Process events
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				return 0;
			} else if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE: case SDLK_q:
					return 0;
				}
			}
		}

		// Render
		renderer.SetDrawColor(0, 0, 0);
		renderer.Clear();

		renderer.Present();

		// Frame limiter
		SDL_Delay(1);
	}

	return 0;
}

int main(int argc, char** argv) {
	try {
		return realmain(argc, argv);
	} catch (SDL2pp::Exception& e) {
		std::cerr << "Error: " << e.what() << " (" << e.GetSDLError() << ")" << std::endl;
	} catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 1;
}

