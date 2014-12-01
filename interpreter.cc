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

#include "interpreter.hh"

Interpreter::Interpreter(const DataManager& data_manager, GameInterface& interface, MovPlayer& player, const std::string& startnod, int startentry) : data_manager_(data_manager), interface_(interface), player_(player), awaiting_event_(false) {
	std::list<std::string> loading_queue;
	loading_queue.push_back(startnod);

	while (!loading_queue.empty()) {
		Log("interp") << "Loading script " << loading_queue.front();
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
}

Interpreter::~Interpreter() {
}

void Interpreter::InterruptAndGoto(int offset) {
	Log("interp") << "Interrupt received";

	player_.Stop();
	interface_.ResetHandlers();

	current_node_.second += offset;
	awaiting_event_ = false;
}

void Interpreter::Update(Uint32 current_ticks) {
	if (awaiting_event_)
		return;

	while (1) {
		NodFileMap::const_iterator nodfile = nod_files_.find(current_node_.first);
		if (nodfile == nod_files_.end())
			throw std::logic_error("nod file not found"); // shouldn't happend as all files are preloaded in constructor
		const NodFile::Entry* current_entry = &nodfile->second.GetEntry(current_node_.second);
		Log("interp") << "Interpreting entry " << current_node_.second << " from " << current_node_.first << ": type=" << current_entry->GetType();
		switch (current_entry->GetType()) {
		case 0: // death
			{
				Log("interp") << "  Death: should exit to menu here, but it's not implemented yet";
				throw std::logic_error("death not implemented");
			}
		case 1: // no-op, mostly used by "gate" entries
			{
				Log("interp") << "  Nop, skipping";
				current_node_.second += current_entry->GetDefaultOffset();
				break;
			}
		case 2: // simple "play movie" command
		case 61: // play movie with analysis as result?
			{
				interface_.ResetHandlers();
				for (auto& condition : current_entry->GetConditions()) {
					Log("interp") << "  Installing interface control handler: condition=" << condition.first;
					switch (condition.first) {
					case (int)NodFile::Condition::YES:
						interface_.InstallHandler(GameInterface::Control::YES, [=]() { InterruptAndGoto(condition.second); } );
						break;
					case (int)NodFile::Condition::NO:
						interface_.InstallHandler(GameInterface::Control::NO, [=]() { InterruptAndGoto(condition.second); } );
						break;
					case (int)NodFile::Condition::STARTUP:
						interface_.InstallHandler(GameInterface::Control::STARTUP, [=]() { InterruptAndGoto(condition.second); } );
						break;
					case (int)NodFile::Condition::DIAGNOSTICS:
						interface_.InstallHandler(GameInterface::Control::DIAGNOSTICS, [=]() { InterruptAndGoto(condition.second); } );
						break;
					case (int)NodFile::Condition::DEPLOY:
						interface_.InstallHandler(GameInterface::Control::DEPLOY, [=]() { InterruptAndGoto(condition.second); } );
						break;
					case (int)NodFile::Condition::ANALYSIS:
						interface_.InstallHandler(GameInterface::Control::ANALYSIS, [=]() { InterruptAndGoto(condition.second); } );
						break;
					default:
						throw std::logic_error("condition type not implemented");
					}
				}

				Log("interp") << "  Playing a movie";
				int offset = current_entry->GetDefaultOffset();
				player_.Play(
						data_manager_.GetPath(current_entry->GetName()),
						current_ticks,
						current_entry->GetStartFrame(),
						current_entry->GetEndFrame(),
						[=]() {
							InterruptAndGoto(offset);
						}
					);
				awaiting_event_ = true;
				return;
			}
		case 5:
			{
				Log("interp") << "  Enable user interface (not implemented yet)";
				current_node_.second += current_entry->GetDefaultOffset();
				break;
			}
		default:
			throw std::logic_error("node type processing not implemented");
		}
	}
}
