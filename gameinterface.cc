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

#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_events.h>

#include "gameinterface.hh"

constexpr unsigned int GameInterface::Constants::ControlDelayMs;

const GameInterface::ControlMap GameInterface::controls_ = {
	{ GameInterface::Control::ANALYSIS, { GameInterface::Texture::FNHILITE, { 10, 101, 90, 18 }, { 0, 0, 90, 18 } } },
	{ GameInterface::Control::DIAGNOSTICS, { GameInterface::Texture::FNHILITE, { 39, 141, 80, 18 }, { 0, 19, 80, 18 } } },
	{ GameInterface::Control::YES, { GameInterface::Texture::FNHILITE, { 26, 181, 36, 16 }, { 0, 38, 36, 16 } } },
	{ GameInterface::Control::NO, { GameInterface::Texture::FNHILITE, { 74, 181, 25, 16 }, { 48, 38, 25, 16 } } },
	{ GameInterface::Control::STATUS, { GameInterface::Texture::FNHILITE, { 49, 214, 53, 16 }, { 0, 55, 53, 16 } } },

	{ GameInterface::Control::STARTUP, { GameInterface::Texture::FNHILITE, { 37, 276, 69, 19 }, { 0, 72, 69, 19 } } },
	{ GameInterface::Control::DEPLOY, { GameInterface::Texture::FNHILITE, { 43, 314, 57, 17 }, { 0, 92, 57, 17 } } },
	{ GameInterface::Control::GRAPPLE_ARM, { GameInterface::Texture::FNHILITE, { 34, 345, 90, 20 }, { 0, 110, 90, 20 } } },
	{ GameInterface::Control::FLOODLIGHT,  { GameInterface::Texture::FNHILITE, { 35, 383, 75, 20 }, { 0, 151, 75, 20 } } },

	{ GameInterface::Control::WOUND, { GameInterface::Texture::FNHILITE, { 170, 0, 90, 51 }, { 0, 217, 90, 51 } } },

	{ GameInterface::Control::PATTERN_PREV, { GameInterface::Texture::MLHILITE, { 171, 326, 38, 33 }, { 1, 90, 38, 33 } } },
	{ GameInterface::Control::PATTERN_SEND, { GameInterface::Texture::MLHILITE, { 209, 334, 48, 25 }, { 40, 90, 48, 25 } } },
	{ GameInterface::Control::PATTERN_NEXT, { GameInterface::Texture::MLHILITE, { 257, 340, 42, 33 }, { 89, 90, 42, 33 } } },

	{ GameInterface::Control::COLORS_IR, { GameInterface::Texture::MLHILITE, { 408, 288, 38, 17 }, { 81, 1, 38, 17 } } },
	{ GameInterface::Control::COLORS_VIS, { GameInterface::Texture::MLHILITE, { 470, 285, 38, 19 }, { 41, 1, 38, 19 } } },
	{ GameInterface::Control::COLORS_UV, { GameInterface::Texture::MLHILITE, { 531, 283, 38, 19 }, { 1, 1, 38, 19 } } },

	{ GameInterface::Control::COLORS_1, { GameInterface::Texture::MLHILITE, { 402, 312, 22, 23 }, { 1, 21, 22, 23 } } },
	{ GameInterface::Control::COLORS_2, { GameInterface::Texture::MLHILITE, { 434, 311, 21, 23 }, { 25, 21, 21, 23 } } },
	{ GameInterface::Control::COLORS_3, { GameInterface::Texture::MLHILITE, { 462, 309, 22, 25 }, { 48, 21, 22, 25 } } },
	{ GameInterface::Control::COLORS_4, { GameInterface::Texture::MLHILITE, { 492, 308, 23, 25 }, { 72, 21, 23, 25 } } },
	{ GameInterface::Control::COLORS_5, { GameInterface::Texture::MLHILITE, { 522, 307, 23, 25 }, { 97, 21, 23, 25 } } },
	{ GameInterface::Control::COLORS_6, { GameInterface::Texture::MLHILITE, { 553, 307, 23, 25 }, { 122, 21, 23, 25 } } },
};

GameInterface::GameInterface(SDL2pp::Renderer& renderer, const DataManager& datamanager)
	: renderer_(renderer),
	  background_(renderer, datamanager.GetPath("images/intrface.bmp")),
	  fnhighlights_(renderer, datamanager.GetPath("images/fnhilite.rle")),
	  mlhighlights_(renderer, datamanager.GetPath("images/mlhilite.bmp")),
	  patterns_(renderer, datamanager.GetPath("images/patterns.bmp")),
	  currently_activated_control_(GameInterface::Control::NONE),
	  ui_enabled_(true),
	  fullscreen_video_(false),
	  colors_mode_(ColorsMode::VIS),
	  selected_pattern_(-1),
	  laser_enabled_(false),
	  navigation_mask_(0),
	  listener_(nullptr) {
}

GameInterface::~GameInterface() {
}

void GameInterface::Render(SDL2pp::Texture* video) {
	if (fullscreen_video_) {
		// fullscreen video is enabled, we only need to render it
		if (video)
			renderer_.Copy(*video, SDL2pp::Rect::Null(), SDL2pp::Rect(0, 0, 640, 480));
		return;
	} else if (!ui_enabled_) {
		// fullscreen video is not enabled, but there's no UI (start
		// of the game): only render video in the center
		if (video)
			renderer_.Copy(*video, SDL2pp::Rect::Null(), SDL2pp::Rect(160, 120, 320, 240));
		return;
	}

	renderer_.Copy(background_, SDL2pp::Rect::Null(), SDL2pp::Rect(0, 0, 640, 480));

	// render currently active control
	ControlMap::const_iterator active_control_info = controls_.find(currently_activated_control_);
	if (active_control_info != controls_.end())
		renderer_.Copy(
				(active_control_info->second.texture == Texture::MLHILITE) ? mlhighlights_ : fnhighlights_,
				active_control_info->second.source_rect,
				active_control_info->second.rect
			);

	// render ir/vis/vis color bar
	switch (colors_mode_) {
	case ColorsMode::IR:
		renderer_.Copy(mlhighlights_, SDL2pp::Rect(1, 48, 176, 19), SDL2pp::Rect(404, 335, 176, 19));
		break;
	case ColorsMode::UV:
		renderer_.Copy(mlhighlights_, SDL2pp::Rect(1, 69, 176, 19), SDL2pp::Rect(404, 335, 176, 19));
		break;
	default:
		break;
	}

	// laser indicator
	if (laser_enabled_)
		renderer_.Copy(fnhighlights_, SDL2pp::Rect(0, 173, 55, 43), SDL2pp::Rect(28, 18, 55, 43));

	// pattern
	if (selected_pattern_ >= 0)
		renderer_.Copy(patterns_, SDL2pp::Rect(0, selected_pattern_ * 36, 105, 36), SDL2pp::Rect(162, 412, 105, 36));

	// video
	if (video)
		renderer_.Copy(*video, SDL2pp::Rect::Null(), SDL2pp::Rect(295, 16, 320, 240));

	// navigation marks
	renderer_.SetDrawColor(104, 191, 136);
	if (navigation_mask_ & (int)NavMode::LEFT) {
		renderer_.DrawLine(298, 120, 298, 151);
		renderer_.DrawLine(298, 136, 304, 136);
	}
	if (navigation_mask_ & (int)NavMode::RIGHT) {
		renderer_.DrawLine(611, 120, 611, 151);
		renderer_.DrawLine(605, 136, 611, 136);
	}
	// TODO: top
	// TODO: bottom
}

void GameInterface::TryActivateControl(Control control) {
	if (currently_activated_control_ != Control::NONE)
		return;

	currently_activated_control_ = control;
	control_activation_time_ = SDL_GetTicks();
}

void GameInterface::ProcessControlAction(Control control) {
	switch (control) {
	// Left button mapping
	case Control::ANALYSIS: EmitControlEvent(ControlEvent::ANALYSIS); break;
	case Control::DIAGNOSTICS: EmitControlEvent(ControlEvent::DIAGNOSTICS); break;
	case Control::YES: EmitControlEvent(ControlEvent::YES); break;
	case Control::NO: EmitControlEvent(ControlEvent::NO); break;
	case Control::STATUS: EmitControlEvent(ControlEvent::STATUS); break;

	case Control::STARTUP: EmitControlEvent(ControlEvent::STARTUP); break;
	case Control::DEPLOY: EmitControlEvent(ControlEvent::DEPLOY); break;
	case Control::GRAPPLE_ARM: EmitControlEvent(ControlEvent::GRAPPLE_ARM); break;
	case Control::FLOODLIGHT: EmitControlEvent(ControlEvent::FLOODLIGHT); break;

	// Color mode selection
	case Control::COLORS_IR:
		colors_mode_ = ColorsMode::IR;
		break;
	case Control::COLORS_VIS:
		colors_mode_ = ColorsMode::VIS;
		break;
	case Control::COLORS_UV:
		colors_mode_ = ColorsMode::UV;
		break;

	// Color buttons processing
	case Control::COLORS_1: case Control::COLORS_2: case Control::COLORS_3:
	case Control::COLORS_4: case Control::COLORS_5: case Control::COLORS_6:
		if (colors_mode_ == ColorsMode::IR) {
			EmitControlEvent(ControlEvent::INFRARED);
		} else if (colors_mode_ == ColorsMode::UV) {
			EmitControlEvent(ControlEvent::ULTRAVIOLET);
		} else switch (control) {
			case Control::COLORS_1: EmitControlEvent(ControlEvent::RED); break;
			case Control::COLORS_2: EmitControlEvent(ControlEvent::ORANGE); break;
			case Control::COLORS_3: EmitControlEvent(ControlEvent::YELLOW); break;
			case Control::COLORS_4: EmitControlEvent(ControlEvent::GREEN); break;
			case Control::COLORS_5: EmitControlEvent(ControlEvent::BLUE); break;
			case Control::COLORS_6: EmitControlEvent(ControlEvent::PURPLE); break;
			default: break;
		}
		break;
	default: break;
	}
}

void GameInterface::EmitControlEvent(GameInterface::ControlEvent event) {
	if (listener_)
		listener_->ProcessControlEvent(event);
}

void GameInterface::EmitPointEvent(const SDL2pp::Point& point) {
	if (listener_)
		listener_->ProcessPointEvent(point);
}

void GameInterface::Update(unsigned int ticks) {
	if (ticks > control_activation_time_ + GameInterface::Constants::ControlDelayMs) {
		ProcessControlAction(currently_activated_control_);
		currently_activated_control_ = Control::NONE;
	}
}

void GameInterface::ProcessEvent(const SDL_Event& event) {
	if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
		ProcessMouseDown(event.button);
	else if (event.type == SDL_KEYUP)
		ProcessKeyUp(event.key);
}

void GameInterface::ProcessKeyUp(const SDL_KeyboardEvent& key) {
	if (!ui_enabled_)
		return;

	if (key.keysym.mod == KMOD_NONE) {
		switch (key.keysym.sym) {
		case SDLK_a: TryActivateControl(Control::ANALYSIS); break;
		case SDLK_d: TryActivateControl(Control::DIAGNOSTICS); break;
		case SDLK_y: TryActivateControl(Control::YES); break;
		case SDLK_n: TryActivateControl(Control::NO); break;
		case SDLK_t: TryActivateControl(Control::STATUS); break;
		case SDLK_s: TryActivateControl(Control::STARTUP); break;
		case SDLK_p: TryActivateControl(Control::DEPLOY); break;
		case SDLK_g: TryActivateControl(Control::GRAPPLE_ARM); break;
		case SDLK_f: TryActivateControl(Control::FLOODLIGHT); break;
		case SDLK_SPACE: fullscreen_video_ = !fullscreen_video_; break;
		default: break;
		}
	} else if (key.keysym.mod == KMOD_LCTRL || key.keysym.mod == KMOD_RCTRL) {
		switch (key.keysym.sym) {
		case SDLK_i: // TODO: infrared // XXX: not used in the game, may ignore
		case SDLK_r: // TODO: red
		case SDLK_o: // TODO: orange
		case SDLK_y: // TODO: yellow
		case SDLK_g: // TODO: green
		case SDLK_b: // TODO: blue
		case SDLK_p: // TODO: purple
		case SDLK_u: // TODO: ultraviolet // XXX: not used in the game, may ignore
		default: break;
		}
	}
}

void GameInterface::ProcessMouseDown(const SDL_MouseButtonEvent& button) {
	if (!ui_enabled_)
		return;

	if (!fullscreen_video_)
		for (auto& control : controls_)
			if (control.second.rect.Contains(SDL2pp::Point(button.x, button.y)))
				TryActivateControl(control.first);

	SDL2pp::Point target(0, 0);
	if (fullscreen_video_) {
		target.SetX(button.x / 2);
		target.SetY(button.y / 2);
	} else {
		target.SetX(button.x - 295);
		target.SetY(button.y - 16);
	}

	if (target.GetX() >= 0 && target.GetY() >= 0 && target.GetX() < 320 && target.GetY() < 240)
		EmitPointEvent(target);
}

void GameInterface::SetListener(GameInterface::EventListener* listener) {
	listener_ = listener;
}

void GameInterface::EnableLaserMode() {
	laser_enabled_ = true;
}

void GameInterface::EnableNavigationMode(int navigation_mask) {
	navigation_mask_ = navigation_mask;
}

void GameInterface::ResetMode() {
	laser_enabled_ = false;
	navigation_mask_ = 0;
}
