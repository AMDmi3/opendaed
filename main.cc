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

#include <SDL2pp/SDL.hh>
#include <SDL2pp/Exception.hh>
#include <SDL2pp/Window.hh>
#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Texture.hh>

#include "datamanager.hh"
#include "gameinterface.hh"
#include "interpreter.hh"
#include "movplayer.hh"

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

	// Script interpreter
	Interpreter script(data_manager, "encountr.nod");

	// SDL stuff
	SDL2pp::SDL sdl(SDL_INIT_VIDEO);
	SDL2pp::Window window("OpenDaed", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);
	SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

	GameInterface interface(renderer, data_manager);

	MovPlayer mov;

	mov.Play(data_manager.GetPath("tm_01.mov"), SDL_GetTicks(), 1119, 1456, [](){ std::cerr << "Movie ended" << std::endl; });

	while (1) {
		unsigned int frame_ticks = SDL_GetTicks();

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

			interface.ProcessEvent(event);
		}

		// Update logic
		interface.Update(frame_ticks);

		// Render
		renderer.SetDrawColor(0, 0, 0);
		renderer.Clear();

		interface.Render();
		mov.UpdateFrame(renderer, frame_ticks);
		renderer.Copy(mov.GetTexture(), SDL2pp::Rect::Null(), SDL2pp::Rect(295, 16, 320, 240));

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

