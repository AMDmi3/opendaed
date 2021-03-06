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

#ifndef GAMEINTERFACE_HH
#define GAMEINTERFACE_HH

#include <map>

#include <SDL2/SDL_events.h>

#include <SDL2pp/Texture.hh>
#include <SDL2pp/Renderer.hh>

#include "datamanager.hh"

class GameInterface {
public:
	enum class ControlEvent {
		ANALYSIS,
		DIAGNOSTICS,
		YES,
		NO,
		STATUS,

		STARTUP,
		DEPLOY,
		GRAPPLE_ARM,
		FLOODLIGHT,

		INFRARED,

		RED,
		ORANGE,
		YELLOW,
		GREEN,
		BLUE,
		PURPLE,

		ULTRAVIOLET,
	};

	class EventListener {
	public:
		virtual ~EventListener() {
		}

		virtual void ProcessControlEvent(ControlEvent) {
		}

		virtual void ProcessPointEvent(const SDL2pp::Point&) {
		}
	};

protected:
	enum class Control {
		NONE,

		ANALYSIS,
		DIAGNOSTICS,
		YES,
		NO,
		STATUS,

		STARTUP,
		DEPLOY,
		GRAPPLE_ARM,
		FLOODLIGHT,

		WOUND,

		PATTERN_PREV,
		PATTERN_SEND,
		PATTERN_NEXT,

		COLORS_IR,
		COLORS_VIS,
		COLORS_UV,

		COLORS_1,
		COLORS_2,
		COLORS_3,
		COLORS_4,
		COLORS_5,
		COLORS_6,
	};

protected:
	struct Constants {
		static constexpr unsigned int ControlDelayMs = 500;
	};

	enum class Texture {
		FNHILITE,
		MLHILITE,
	};

	enum class ColorsMode {
		IR,
		VIS,
		UV,
	};

	enum class NavMode {
		LEFT = 0x1,
		RIGHT = 0x2,
		UP = 0x4,
		DOWN = 0x8,
	};

	struct ControlInfo {
		Texture texture;
		SDL2pp::Rect rect;
		SDL2pp::Rect source_rect;
	};

	typedef std::map<Control, ControlInfo> ControlMap;

	typedef std::map<Control, std::function<void()>> ControlHandlerMap;

protected:
	static const ControlMap controls_;

protected:
	SDL2pp::Renderer& renderer_;

	// Textures
	SDL2pp::Texture background_;
	SDL2pp::Texture fnhighlights_;
	SDL2pp::Texture mlhighlights_;
	SDL2pp::Texture patterns_;

	// Click processing
	Control currently_activated_control_;
	unsigned int control_activation_time_;

	bool ui_enabled_;
	bool fullscreen_video_;
	ColorsMode colors_mode_;
	int selected_pattern_;
	bool laser_enabled_;
	int navigation_mask_;

	EventListener* listener_;

protected:
	void TryActivateControl(Control control);
	void ProcessControlAction(Control control);

	void EmitControlEvent(ControlEvent event);
	void EmitPointEvent(const SDL2pp::Point& point);

public:
	GameInterface(SDL2pp::Renderer& renderer, const DataManager& datamanager);
	~GameInterface();

	void ProcessEvent(const SDL_Event& event);
	void ProcessMouseDown(const SDL_MouseButtonEvent& button);
	void ProcessKeyUp(const SDL_KeyboardEvent& key);
	void Update(unsigned int ticks);
	void Render(SDL2pp::Texture* video);

	void SetListener(EventListener* listener);

	void EnableLaserMode();
	void EnableNavigationMode(int navmask);
	void ResetMode();
};

#endif // GAMEINTERFACE_HH
