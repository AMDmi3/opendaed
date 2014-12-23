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

#include <list>
#include <algorithm>

#include "logger.hh"

#include "datamanager.hh"
#include "movplayer.hh"
#include "gameinterface.hh"
#include "hotfile.hh"

#include "interpreter.hh"

namespace {

static GameInterface::ControlEvent ConditionToEvent(int cond) {
	switch (cond) {
	case (int)NodFile::Condition::YES: return GameInterface::ControlEvent::YES;
	case (int)NodFile::Condition::NO: return GameInterface::ControlEvent::NO;
	case (int)NodFile::Condition::STARTUP: return GameInterface::ControlEvent::STARTUP;
	case (int)NodFile::Condition::DIAGNOSTICS: return GameInterface::ControlEvent::DIAGNOSTICS;
	case (int)NodFile::Condition::DEPLOY: return GameInterface::ControlEvent::DEPLOY;
	case (int)NodFile::Condition::ANALYSIS: return GameInterface::ControlEvent::ANALYSIS;
	case (int)NodFile::Condition::FLOODLIGHT: return GameInterface::ControlEvent::FLOODLIGHT;
	default:
		throw std::logic_error("condition type not implemented");
	}
}

}

Interpreter::Interpreter(const DataManager& data_manager, GameInterface& interface, MovPlayer& player, const std::string& startnod, int startentry) : data_manager_(data_manager), interface_(interface), player_(player), awaiting_event_(false) {
	std::list<std::string> loading_queue;
	loading_queue.push_back(startnod);

	while (!loading_queue.empty()) {
		Log("interp") << "loading script " << loading_queue.front();
		NodFileMap::iterator just_added = nod_files_.emplace(loading_queue.front(), data_manager.GetPath(loading_queue.front())).first;

		// collect all .nod files referenced by recently
		// loaded one which were not yet visited
		just_added->second.ForEach([this, &loading_queue](const NodFile::Entry& e) {
				std::string file = e.GetName();
				std::transform(file.begin(), file.end(), file.begin(), ::tolower);
				if (file.rfind(".nod") == file.length() - 4 && file != "gate.nod" && nod_files_.find(file) == nod_files_.end())
					loading_queue.push_back(file);
			});

		loading_queue.pop_front();
	}

	current_node_ = std::make_pair(startnod, startentry);

	interface_.SetListener(this);
	player_.SetListener(this);
}

Interpreter::~Interpreter() {
	interface_.SetListener(nullptr);
	player_.SetListener(nullptr);
}

void Interpreter::InterruptAndGoto(int offset) {
	if (!awaiting_event_)
		return;

	Log("interp") << "interrupt received";

	player_.Stop();

	current_node_.second += offset;
	awaiting_event_ = false;
}

void Interpreter::Update() {
	if (awaiting_event_)
		return;

	while (1) {
		ResetHandlers();
		interface_.ResetMode();

		NodFileMap::const_iterator nodfile = nod_files_.find(current_node_.first);
		if (nodfile == nod_files_.end())
			throw std::logic_error("nod file not found"); // shouldn't happend as all files are preloaded in constructor
		const NodFile::Entry* current_entry = &nodfile->second.GetEntry(current_node_.second);
		Log("interp") << "interpreting entry " << current_node_.second << " from " << current_node_.first << ": type=" << current_entry->GetType();
		switch (current_entry->GetType()) {
		case 0: // death
			{
				Log("interp") << "  death: should exit to menu here, but it's not implemented yet";
				throw std::logic_error("death not implemented");
			}
		case 1: // no-op, mostly used by "gate" entries
			{
				Log("interp") << "  nop, skipping";
				current_node_.second += current_entry->GetDefaultOffset();
				break;
			}
		case 2: // simple "play movie" command
		case 61: // play movie with analysis as result?
			{
				// save frame limits on player actions
				// e.g. int a 20 second movie, the game will accept player
				// input only between 10 and 18 seconds
				int actionstartframe = current_entry->GetActionStartFrame();
				int actionendframe = current_entry->GetActionEndFrame();

				// install event handlers for this scene
				for (auto& condition : current_entry->GetConditions()) {
					Log("interp") << "  installing interface control handler: condition=" << condition.first;
					AddControlEventHandler([=](GameInterface::ControlEvent event){
							if (event == ConditionToEvent(condition.first) &&
									player_.GetCurrentFrame() >= actionstartframe &&
									player_.GetCurrentFrame() <= actionendframe)
								InterruptAndGoto(condition.second);
						});
				}

				// install end of clip handler for this scene
				int offset = current_entry->GetDefaultOffset();
				AddEndOfClipEventHandler([=](){
						InterruptAndGoto(offset);
					});

				// play movie
				Log("interp") << "  playing a movie";
				player_.Play(
						data_manager_.GetPath(current_entry->GetName()),
						current_entry->GetStartFrame(),
						current_entry->GetEndFrame()
					);

				// yield
				awaiting_event_ = true;
				return;
			}
		case 3:
			{
				// install event handlers for this scene
				for (auto& condition : current_entry->GetConditions()) {
					Log("interp") << "  installing interface control handler: condition=" << condition.first;
					AddControlEventHandler([=](GameInterface::ControlEvent event){
							if (event == ConditionToEvent(condition.first))
								InterruptAndGoto(condition.second);
						});
				}

				// check if we have a hot zone (is this correct?)
				std::string hotname = current_entry->GetName();
				hotname.replace(hotname.length() - 3, std::string::npos, "hot");
				if (data_manager_.HasPath(hotname)) {
					Log("interp") << "  found hotzone " << hotname;
					HotFile hot(data_manager_.GetPath(hotname));

					std::vector<int> offsets_for_rect;

					for (int i = 0; i < 8; i++)
						offsets_for_rect.push_back(current_entry->GetCondition(i).second);

					AddPointEventHandler([=](const SDL2pp::Point& point) {
							int nrect = 0;
							for (auto& rect: hot.GetRectsForFrame(player_.GetCurrentFrame())) {
								if (rect.Contains(point)) {
									InterruptAndGoto(offsets_for_rect[nrect]);
									return;
								}
								nrect++;
							}
						});

					interface_.EnableLaserMode();
				}

				// play our single frame
				Log("interp") << "  playing a single frame: ";
				player_.PlaySingleFrame(
						data_manager_.GetPath(current_entry->GetName()),
						current_entry->GetStartFrame()
					);

				// yield
				awaiting_event_ = true;
				return;
			}
		case 5:
			{
				Log("interp") << "  enable user interface (not implemented yet)";
				current_node_.second += current_entry->GetDefaultOffset();
				break;
			}
		case 30:
			{
				Log("interp") << "  something hotzone-related, skipping";
				current_node_.second += current_entry->GetDefaultOffset();
				break;
			}
		case 33:
			{
				Log("interp") << "  unknown, skipping";
				current_node_.second += current_entry->GetDefaultOffset();
				break;
			}
		default:
			throw std::logic_error("node type processing not implemented");
		}
	}
}
