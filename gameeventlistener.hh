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

#ifndef GAMEEVENTLISTENER_HH
#define GAMEEVENTLISTENER_HH

#include <list>
#include <stdexcept>

#include "movplayer.hh"
#include "gameinterface.hh"

class GameEventListener : public MovPlayer::EventListener, public GameInterface::EventListener {
public:
	typedef std::function<void(GameInterface::ControlEvent)> ControlEventHandler;
	typedef std::function<void(const SDL2pp::Point&)> PointEventHandler;
	typedef std::function<void()> EndOfClipEventHandler;

private:
	std::list<ControlEventHandler> control_event_handlers_;
	std::list<PointEventHandler> point_event_handlers_;
	std::list<EndOfClipEventHandler> endofclip_event_handlers_;

	struct StackEntry {
		size_t control_events;
		size_t point_events;
		size_t endofclip_events;
	};

	std::list<StackEntry> handler_state_stack_;

public:
	virtual ~GameEventListener() {
	}

	void AddControlEventHandler(ControlEventHandler&& handler) {
		control_event_handlers_.emplace_back(std::move(handler));
	}

	void AddPointEventHandler(PointEventHandler&& handler) {
		point_event_handlers_.emplace_back(std::move(handler));
	}

	void AddEndOfClipEventHandler(EndOfClipEventHandler&& handler) {
		endofclip_event_handlers_.emplace_back(std::move(handler));
	}

	virtual void ProcessControlEvent(GameInterface::ControlEvent event) override {
		for (auto i = control_event_handlers_.begin(); i != control_event_handlers_.end(); i++) {
			std::cerr << "X\n";
			(*i)(event);
		}
	}

	virtual void ProcessPointEvent(const SDL2pp::Point& point) override {
		for (auto i = point_event_handlers_.rbegin(); i != point_event_handlers_.rend(); i++)
			(*i)(point);
	}

	virtual void ProcessEndOfClipEvent() override {
		for (auto i = endofclip_event_handlers_.rbegin(); i != endofclip_event_handlers_.rend(); i++)
			(*i)();
	}

	void PushHandlerState() {
		handler_state_stack_.push_back({
				control_event_handlers_.size(),
				point_event_handlers_.size(),
				endofclip_event_handlers_.size()
			});
	}

	void PopHandlerState() {
		if (handler_state_stack_.empty())
			throw std::runtime_error("event handler stack underflow");

		control_event_handlers_.resize(handler_state_stack_.back().control_events);
		point_event_handlers_.resize(handler_state_stack_.back().point_events);
		endofclip_event_handlers_.resize(handler_state_stack_.back().endofclip_events);

		handler_state_stack_.pop_back();
	}

	void ResetHandlers() {
		control_event_handlers_.clear();
		point_event_handlers_.clear();
		endofclip_event_handlers_.clear();
	}
};

#endif // GAMEEVENTLISTENER_HH
