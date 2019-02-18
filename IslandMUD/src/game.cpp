﻿/* Jim Viebke
May 15 2015 */

#include <iostream>
#include <memory>

#include "game.h"

Game::Game()
{
	// Calling start() makes sure the "running" flag is set to true.
	// Other threads periodically check this flag, and will cleanly terminate themselves if the flag is ever disabled.
	Server::start();

	world = std::make_unique<World>();

	// start the threads for listening on port numbers
	std::thread(&Game::networking_thread, this, C::GAME_PORT_NUMBER, &Game::client_thread).detach();
	std::thread(&Game::networking_thread, this, C::GAME_MAP_PORT_NUMBER, &Game::client_map_thread).detach();

	// start the thread responsible for dispatching output to connected clients
	std::thread(&Game::outbound_thread, this).detach();

	// start the thread responsible for all things NPC
	std::thread(&Game::NPC_thread, this).detach();
}

Update_Messages Game::execute_command(const Character::ID & actor_id, const std::vector<std::string> & command)
{
	// "help"
	if (command.size() == 1 && command[0] == C::SHOW_HELP_COMMAND)
	{
		return Update_Messages(std::string("help:\n") +
			"\nlegend" +
			"\ninventory / inv / i" +
			"\nlook / l" +
			"\nrecipes" +
			"\nrecipes [search keyword]" +
			"\nmove [compass direction]" +
			"\ntake / drop / craft / mine / equip / dequip / chop / smash [item]" +
			"\nequipped" +
			"\nadd / place / put / drop [item] into chest" +
			"\ntake [item] from chest" +
			"\nchest" +
			"\nattack [compass direction] wall / door" +
			"\nattack [name]" +
			"\nconstruct [material] ceiling / floor" +
			"\nconstruct [compass direction] [material] wall" +
			"\nconstruct [compass direction] [material] wall with [material] door");
	}
	// "legend"
	else if (command.size() == 1 && command[0] == C::LEGEND_COMMAND)
	{
		return Update_Messages(std::string("legend:\n") +
			"\n " + C::FOREST_CHAR + "     forest" +
			"\n " + C::WATER_CHAR + "     water" +
			"\n " + C::LAND_CHAR + "     land" +
			"\n " + C::GENERIC_MINERAL_CHAR + "     a mineral deposit" +
			"\n" +
			"\n " + C::PLAYER_CHAR + "     you" +
			"\n 1     number of militants" +
			"\n " + C::NPC_NEUTRAL_CHAR + "     one or more neutral inhabitants" +
			"\n" +
			"\n " + C::ITEM_CHAR + "     one or more items" +
			"\n " + C::CHEST_CHAR + "     a chest" +
			"\n " + C::TABLE_CHAR + "     a table" +
			"\n" +
			"\n " + C::WALL_CHAR + C::WALL_CHAR + C::WALL_CHAR + "   a wall" +
			"\n " + C::WALL_CHAR + C::DOOR_CHAR + C::WALL_CHAR + "   a wall with a door" +
			"\n " + C::WALL_CHAR + C::RUBBLE_CHAR + C::WALL_CHAR + "   a smashed door or wall (traversable)");
	}
	// "i", "inv", "inventory"
	else if (command.size() == 1 && command[0] == C::INVENTORY_COMMAND)
	{
		// if the actor is a Player_Character
		if (const std::shared_ptr<PC> & actor = U::convert_to<PC>(actors.find(actor_id)->second))
		{
			return Update_Messages(actor->get_inventory_info());
		}
	}
	// "l", "look"
	else if (command.size() == 1 && command[0] == C::LOOK_COMMAND)
	{
		// if the actor is a Player_Character
		if (const std::shared_ptr<PC> & player = U::convert_to<PC>(actors.find(actor_id)->second))
		{
			return Update_Messages(world->room_at(player->get_location())->summary(player->get_ID()));
		}
	}
	// moving: "move northeast" OR "northeast"
	else if ((command.size() == 2 && command[0] == C::MOVE_COMMAND)
		|| command.size() == 1 && U::contains(C::direction_ids, command[0]))
	{
		// get a shared_ptr to the acting charater
		std::shared_ptr<Character> character = actors.find(actor_id)->second;

		// get the result of making the move
		Update_Messages result = character->move(command[command.size() - 1]); // passes direction (last element in command) and world

		// if the acting character is a player character
		if (const std::shared_ptr<PC> player = U::convert_to<PC>(character))
			// append a summary of the new area
			result.to_user += world->room_at(player->get_location())->summary(player->get_ID());

		return result;
	}
	// take: "take branch"
	else if (command.size() == 2 && command[0] == C::TAKE_COMMAND)
	{
		return actors.find(actor_id)->second->take(command[1]); // (item)
	}
	// take: "take [n] branches"
	else if (command.size() == 3 && command[0] == C::TAKE_COMMAND)
	{
		return actors.find(actor_id)->second->take(command[2], command[1]); // (item, count)
	}
	// dropping item: "drop staff"
	else if (command.size() == 2 && command[0] == C::DROP_COMMAND)
	{
		return actors.find(actor_id)->second->drop(command[1]); // (item)
	}
	// dropping item: drop n staffs" (the tokenizer will have already converted plurals to singular)
	else if (command.size() == 3 && command[0] == C::DROP_COMMAND)
	{
		return actors.find(actor_id)->second->drop(command[2], command[1]);
	}
	// crafting: "craft sword"
	else if (command.size() == 2 && command[0] == C::CRAFT_COMMAND)
	{
		return actors.find(actor_id)->second->craft(command[1]); // (item_id)
	}
	// mining: "mine iron"
	else if (command.size() == 2 && command[0] == C::MINE_COMMAND)
	{
		return actors.find(actor_id)->second->craft(command[1]); // (item_id)
	}
	// making ceiling/floor: "construct stone floor/ceiling"
	else if (command.size() == 3 && command[0] == C::CONSTRUCT_COMMAND)
	{
		return actors.find(actor_id)->second->construct_surface(command[1], U::to_surface(command[2])); // material, direction
	}
	// making walls: "construct west stone wall"
	else if (command.size() == 4 && command[0] == C::CONSTRUCT_COMMAND && command[3] == C::WALL)
	{
		return actors.find(actor_id)->second->construct_surface(command[2], U::to_surface(command[1])); // material, direction
	}
	// "construct west stone wall with stick door"
	else if (command.size() == 7 && command[0] == C::CONSTRUCT_COMMAND && command[3] == C::WALL && command[4] == C::WITH_COMMAND && command[6] == C::DOOR)
	{
		return actors.find(actor_id)->second->construct_surface_with_door(command[2], U::to_surface(command[1]), command[5]); // material, direction
	}
	// printing out the full library of recipes: "recipes"
	else if (command.size() == 1 && command[0] == C::PRINT_RECIPES_COMMAND)
	{
		return Update_Messages(Character::recipes->get_recipes()); // (item_id)
	}
	// print out any recipes where the name of the recipe contains the 2nd command
	else if (command.size() > 1 && command[0] == C::PRINT_RECIPES_COMMAND)
	{
		return Update_Messages(Character::recipes->get_recipes_matching(command[1]));
	}
	// the player is attacking a wall "smash west wall"
	else if (command.size() >= 3 && command[0] == C::ATTACK_COMMAND && U::contains(C::surface_ids, command[1])
		&& command[2] == C::WALL)
	{
		return actors.find(actor_id)->second->attack_surface(command[1]);
	}
	// the player is attacking a door "smash west door"
	else if (command.size() >= 3 && command[0] == C::ATTACK_COMMAND && U::contains(C::surface_ids, command[1])
		&& command[2] == C::DOOR)
	{
		return actors.find(actor_id)->second->attack_door(command[1]);
	}
	// the player is attacking a person or item
	else if (command.size() == 2 && command[0] == C::ATTACK_COMMAND)
	{
		// extract a shared pointer to the player
		const auto & player = actors.find(actor_id)->second;

		// if the target exists
		const auto target_iterator = actor_ids.find(command[1]);
		if (target_iterator != actor_ids.cend())
		{
			std::shared_ptr<Character> target = actors.find(target_iterator->second)->second;

			// if the target is in this room
			if (world->room_at(player->get_location())->is_occupied_by(target->get_ID()))
			{
				if (U::is<NPC>(target)) // the player exists, and can be attacked
				{
					return player->attack_character(target);
				}
				else // the player exists, but is a human
				{
					return Update_Messages(command[1] + " is friendly.");
				}
			}
		}
		else // the second argument is not a nearby NPC
		{
			return actors.find(actor_id)->second->attack_item(command[1]);
		}
	}
	// save
	else if (command.size() == 1 && command[0] == C::SAVE_COMMAND)
	{
		return actors.find(actor_id)->second->save();
	}
	// equip [item]
	else if (command.size() == 2 && command[0] == C::EQUIP_COMMAND)
	{
		return actors.find(actor_id)->second->equip(command[1]);
	}
	// dequip (2nd arg is optional and ignored)
	else if ((command.size() == 1 || command.size() == 2) && command[0] == C::DEQUIP_COMMAND)
	{
		return actors.find(actor_id)->second->unequip();
	}
	// put [item] in chest
	else if (command.size() == 4 && command[0] == C::DROP_COMMAND && command[2] == C::INSERT_COMMAND && command[3] == C::CHEST_ID)
	{
		return actors.find(actor_id)->second->add_to_chest(command[1]);
	}
	// put [n] [items] in chest
	else if (command.size() == 5 && command[0] == C::DROP_COMMAND && command[3] == C::INSERT_COMMAND && command[4] == C::CHEST_ID)
	{
		return actors.find(actor_id)->second->add_to_chest(command[2], command[1]);
	}
	// look at chest: "chest"
	else if (command.size() == 1 && command[0] == C::CHEST_ID)
	{
		return actors.find(actor_id)->second->look_inside_chest();
	}
	// take [item] from chest
	else if (command.size() == 4 && command[0] == C::TAKE_COMMAND && command[2] == C::FROM_COMMAND && command[3] == C::CHEST_ID)
	{
		return actors.find(actor_id)->second->take_from_chest(command[1]);
	}
	// take [n] [item] from chest
	else if (command.size() == 5 && command[0] == C::TAKE_COMMAND && command[3] == C::FROM_COMMAND && command[4] == C::CHEST_ID)
	{
		return actors.find(actor_id)->second->take_from_chest(command[2], command[1]);
	}
	// put [item] on table
	else if (command.size() == 4 && command[0] == C::DROP_COMMAND && command[2] == C::INSERT_COMMAND && command[3] == C::TABLE_ID)
	{
		return actors.find(actor_id)->second->add_to_table(command[1]);
	}
	// put [n] [item] on table
	else if (command.size() == 5 && command[0] == C::DROP_COMMAND && command[3] == C::INSERT_COMMAND && command[4] == C::TABLE_ID)
	{
		return actors.find(actor_id)->second->add_to_table(command[2], command[1]);
	}
	// look at table: "table"
	else if (command.size() == 1 && command[0] == C::TABLE_ID)
	{
		return actors.find(actor_id)->second->look_at_table();
	}
	// take [item] from table
	else if (command.size() == 4 && command[0] == C::TAKE_COMMAND && command[2] == C::FROM_COMMAND && command[3] == C::TABLE_ID)
	{
		return actors.find(actor_id)->second->take_from_table(command[1]);
	}
	// take [n] [items] from table
	else if (command.size() == 5 && command[0] == C::TAKE_COMMAND && command[3] == C::FROM_COMMAND && command[4] == C::TABLE_ID)
	{
		return actors.find(actor_id)->second->take_from_table(command[2], command[1]);
	}
	// get equipped item name
	else if (command.size() == 1 && (command[0] == C::EQUIP_COMMAND || command[0] == C::ITEM_COMMAND))
	{
		// if the actor is a Player_Character
		if (const std::shared_ptr<PC> & actor = U::convert_to<PC>(actors.find(actor_id)->second))
		{
			// convert the actor to a Player_Character
			return Update_Messages(actor->get_equipped_item_info());
		}
	}
	// debugging
	else if (command.size() == 1 && command[0] == "coord")
	{
		if (const std::shared_ptr<PC> player = U::convert_to<PC>(actors.find(actor_id)->second))
		{
			std::stringstream coord;
			coord << "Your coordinates are " << player->get_location().to_string() << ".";
			return Update_Messages(coord.str());
		}
	}
	// debugging
	else if (command.size() == 1 && command[0] == "shutdown")
	{
		Server::shutdown();
		return Update_Messages("Server is shutting down.");
	}

	return Update_Messages("Nothing happens.");
}

void Game::run()
{
	std::thread(&Game::processing_thread, this).detach();

	shutdown_listener();
}

void Game::generate_outbound_messages(const character_id & user_ID, const Update_Messages & update_messages)
{
	// Make sure the calling function has a lock on the actors_mutex, because this function does not acquire it.

	// create a stringstream to assemble the return message
	std::stringstream action_result;

	// get the user that triggered the action
	const std::shared_ptr<Character> character = actors.find(user_ID)->second;

	// save the player's coordinates in case a map update is required
	const int player_x = character->get_location().get_x();
	const int player_y = character->get_location().get_y();

	// add the update message to the end of the outbound message
	action_result << update_messages.to_user;

	// if a map update is required for all players within view distance
	if (update_messages.map_update_required)
	{
		// for each room within view distance
		for (int cx = player_x - (int)C::VIEW_DISTANCE; cx <= player_x + (int)C::VIEW_DISTANCE; ++cx)
		{
			for (int cy = player_y - (int)C::VIEW_DISTANCE; cy <= player_y + (int)C::VIEW_DISTANCE; ++cy)
			{
				Coordinate current(cx, cy);

				// skip if the room is out of bounds
				if (!current.is_valid()) continue;

				// for each user in the room
				for (const character_id & user : world->room_at(current)->get_actor_ids())
				{
					// if the user is a player character
					if (const std::shared_ptr<PC> player = U::convert_to<PC>(actors.find(user)->second))
					{
						// get the player's map socket
						network::connection_ptr connection = players.get_map_connection(user);
						// if the player does not have a map socket, send the map to the player's main socket
						if (connection == nullptr) connection = players.get_connection(user);

						// generate an area map from the current player's perspective, send it to the correct socket
						outbound_queue.put(Message(connection, "\n\n" + player->generate_area_map()));
					}
				}
			}
		}
	}

	// if the update requires a map update for one or more players
	if (update_messages.additional_map_update_users != nullptr)
	{
		// for each player that requires an update
		for (const auto & player_id : *update_messages.additional_map_update_users)
		{
			// if the referenced character is a player character
			if (const std::shared_ptr<PC> player = U::convert_to<PC>(actors.find(player_id)->second))
			{
				// get the player's map socket
				network::connection_ptr connection = players.get_map_connection(player_id);
				// if the player does not have a map socket, send the map to the player's main socket
				if (connection == nullptr) connection = players.get_connection(player_id);

				// generate an area map from the current player's perspective, send it to the correct socket
				outbound_queue.put(Message(connection, player->generate_area_map()));
			}
		}
	}

	// if the user that made the action is a human
	if (U::is<PC>(character))
	{
		// create an outbound message to the client in question
		const network::connection_ptr connection = players.get_connection(user_ID);
		if (connection != nullptr) outbound_queue.put(Message(connection, user_ID + ": " + action_result.str()));
	}

	// if a message needs to be sent to all other player characters in the room
	if (update_messages.to_room != nullptr)
	{
		// get a list of all players in the room
		for (const auto & actor_id : world->room_at(character->get_location())->get_actor_ids()) // for each player in the room
		{
			// if the player is not "self" and the player is a human
			if (actor_id != user_ID && U::is<PC>(actors.find(actor_id)->second))
			{
				// send the room update to the player
				outbound_queue.put(Message(players.get_connection(actor_id), *update_messages.to_room));
			}
		}
	}

	// if a custom message needs to be sent to a specific user
	if (update_messages.custom_message)
	{
		// add the message to the queue
		outbound_queue.put(Message(players.get_connection(update_messages.custom_message->first), update_messages.custom_message->second));
	}
}

// private member functions

void Game::networking_thread(const unsigned & listening_port, client_thread_type client_thread_pointer)
{
	network::port_ptr port;

	try
	{
		port = network::Port::open_port(listening_port);
	}
	catch (std::exception & ex)
	{
		std::stringstream ss;
		ss << "Failed to listen on port " << listening_port << ". Reason: " << ex.what() << "\n";
		std::cout << ss.str();
		return;
	}

	{
		std::stringstream ss;
		ss << "Listening on port " << listening_port << ".\n";
		std::cout << ss.str();
	}

	for (;;)
	{
		// execution pauses inside of accept() until an incoming connection is received
		network::connection_ptr connection = port->get_connection(); // blocking

		// start a thread to receive messages from this client
		std::thread(client_thread_pointer, this, connection).detach(); // detach() because the thread is responsible for destroying itself
	}
}

void Game::client_thread(network::connection_ptr connection)
{
	outbound_queue.put(Message(connection, "Welcome to IslandMUD!\n\n"));

	{
		const std::string user_ID = login_or_signup(connection); // execution stays in here until the user is signed in
		if (user_ID == "") return; // the user disconnected before signing in, the function above already cleaned up

		// finish other login/connection details

		std::lock_guard<std::mutex> lock(game_state); // lock the actors structure while we modify it

		// create the player
		std::shared_ptr<PC> player = std::make_shared<PC>(user_ID, this);

		// save the player's connection
		players.set_connection(player->get_ID(), connection);

		// add the player to the actor list
		actors[player->get_ID()] = player;

		// save the player's ID to the lookup
		actor_ids[user_ID] = player->get_ID();

		// send a welcome message through the outbound queue
		outbound_queue.put(Message(connection, "Welcome to IslandMUD! You are playing as \"" + user_ID + "\".\n\n"));
	}

	// continuously read data from the client until the client disconnects
	for (;;)
	{
		// create a holder for the next message
		std::string data;

		try
		{
			// read data from the user (blocking)
			data = connection->read();
		}
		catch (std::exception &)
		{
			connection->close();

			// lock the game state so we can clean up
			std::lock_guard<std::mutex> lock(game_state);

			// get the user's ID
			const character_id user_ID = players.get_user_ID(connection); // find username
			if (user_ID == -1) return; // should never happen

			// create a reference to the user's Character object
			std::shared_ptr<Character> user = actors.find(user_ID)->second; // save the user's data

			// clean up the world
			world->attempt_unload_radius(user->get_location(), user_ID);

			// clean up the user
			user->save(); // save the user's data
			actors.erase(user->get_ID()); // erase the user from the actor's map
			actor_ids.erase(user->name); // erase the user from the ID lookup

			// clean up networking stuff
			players.erase(user_ID); // erase the client record

			// clean up the user's thread
			return;
		}

		inbound_queue.put(Message(connection, data));
	}
}


void Game::processing_thread()
{
	// the processing thread handles input from users who are already logged in

	for (;;)
	{
		// destructively get the next inbound message
		const Message inbound_message = inbound_queue.get(); // blocking

															 // don't allow the actors structure to be modified
		std::unique_lock<std::mutex> lock(game_state);

		// get the user ID for the inbound socket
		const character_id user_ID = this->players.get_user_ID(inbound_message.connection);

		// if the target is another player or NPC in the room
		const auto actor_it = actors.find(user_ID);
		if (actor_it == actors.cend())
		{
			// This player is no longer online. Drop the command.
		}
		else
		{
			// execute the user's parsed command against the world
			const Update_Messages update_messages = execute_command(user_ID, Parse::tokenize(inbound_message.data));

			// generate_outbound_messages uses an Update_Messages object to generate all 
			generate_outbound_messages(actor_it->second->get_ID(), update_messages);
		}
	}
}

void Game::client_map_thread(network::connection_ptr map_connection)
{
	outbound_queue.put(Message(map_connection, "Welcome to IslandMUD!\n\n"));

	std::string user_ID;

	// loop in here until the user logs in
	for (;;)
	{
		const std::string login_instructions =
			"This is your map view. Log in on your main client, then log in here using \"username\" \"password\".\n"
			"To create an account or play the game, use port " + U::to_string(C::GAME_PORT_NUMBER) + " in another window.\n\n";

		// set instructions
		outbound_queue.put(Message(map_connection, login_instructions));

		// a holder for the next message
		std::stringstream input;

		// execution pauses inside of recv() until the user sends data (one of these for each user in their own thread)
		try
		{
			input << map_connection->read();
		}
		catch (std::exception &)
		{
			return; // the user lost connection before logging in. return (connection and thread clean themselves up)
		}

		// tokenize user input
		const std::istream_iterator<std::string> begin(input);
		const std::vector<std::string> commands(begin, std::istream_iterator<std::string>());

		if (commands.size() == 2) // only valid input size
		{
			{ // temporary scope to destroy mutex as soon as possible
				std::lock_guard<std::mutex> lock(game_state);
				if (actor_ids.find(commands[0]) == actor_ids.cend()) continue; // repeat message if the user is not logged in
			}

			const std::string user_file = C::user_data_directory + "/" + commands[0] + ".xml";

			if (!U::file_exists(user_file))
			{
				outbound_queue.put(Message(map_connection, "User \"" + commands[0] + "\" does not exist."));
				continue;
			}

			pugi::xml_document user_data_xml;
			user_data_xml.load_file(user_file.c_str());

			if (commands[1] != user_data_xml
				.child(C::XML_USER_ACCOUNT.c_str())
				.attribute(C::XML_USER_PASSWORD.c_str())
				.as_string())
			{
				outbound_queue.put(Message(map_connection, "Incorrect password for user \"" + commands[0] + "\"."));
				continue;
			}

			// the password was correct, save the user's name (their ID)
			user_ID = commands[0];
			break;
		}

		// wrong input length, loop
	}

	// finish other login/connection details
	{
		// add user to the lookup
		players.set_map_connection(actor_ids.find(user_ID)->second, map_connection);

		// generate an area map from the current player's perspective, send it to the newly connect map socket
		std::lock_guard<std::mutex> lock(game_state);
		if (const std::shared_ptr<PC> player = U::convert_to<PC>(actors.find(actor_ids.find(user_ID)->second)->second))
		{
			outbound_queue.put(Message(map_connection, "\n\n" + player->generate_area_map()));
		}
	}

	// loop in here forever
	for (;;)
	{
		outbound_queue.put(Message(map_connection, "\n\nThis is your overhead map client. Use your other client on port " + U::to_string(C::GAME_PORT_NUMBER) + " to play."));

		std::string input;

		try
		{
			input = map_connection->read();
		}
		catch (std::exception &)
		{
			// close the connection
			map_connection->close();
			// reset the player's map connection
			players.set_map_connection(actor_ids.find(user_ID)->second, nullptr);
			return; // destroy this thread
		}
	}
}

void Game::NPC_thread()
{
	// perform a one-time spawning of some NPCs for debugging purposes
	NPC_spawn_startup_logic();

	// std::this_thread::sleep_for(std::chrono::seconds(15)); // put a delay between server startup and NPCs' first action

	std::cout << std::endl;

	auto next_tick = U::current_time_in_ms();

	for (;;)
	{
		const auto tick_start = U::current_time_in_ms();

		{
			std::stringstream ss;
			ss << "NPC thread starting tick at " << tick_start << ".\n";
			std::cout << ss.str();
		}

		//std::cout << "NPC thread running.\n";

		const auto update_start = U::current_time_in_ms();
		NPC_update_logic();
		const auto update_end = U::current_time_in_ms();

		const auto spawn_start = U::current_time_in_ms();
		NPC_spawn_logic();
		const auto spawn_end = U::current_time_in_ms();

		const auto count_start = U::current_time_in_ms();
		size_t loaded_rooms;
		{
			std::unique_lock<std::mutex> lock(game_state);
			loaded_rooms = world->count_loaded_rooms();
		}
		const auto count_end = U::current_time_in_ms();

		const auto audit_start = U::current_time_in_ms();
		{
			std::unique_lock<std::mutex> lock(game_state);
			audit_world_data();
		}
		const auto audit_end = U::current_time_in_ms();



		const auto tick_end = U::current_time_in_ms();
		next_tick += C::MS_PER_TICK;

		std::stringstream ss;
		ss << "NPC tick: " << tick_end - tick_start << " ms (logic " <<
			update_end - update_start << " ms, spawn " <<
			spawn_end - spawn_start << " ms, counted "
			<< loaded_rooms << " rooms in " <<
			count_end - count_start << " ms, audit " <<
			audit_end - audit_start << " ms)\n";
		std::cout << ss.str();

		// if the start of the next tick is in the future, that is,
		// if this tick finished on time
		if (tick_end < next_tick)
		{
			const auto sleep_for = next_tick - tick_end;
			std::stringstream ss;
			ss << "Sleeping " << sleep_for << " ms.\n";
			std::cout << ss.str();
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_for));
		}
	}
}

void Game::NPC_spawn_startup_logic()
{
	// use booleans to toggle spawning of certain NPCs for debugging purposes

	// create four corporals, each with a bodyguard
	if (true)
	{
		const std::vector<std::string> corporals = { "Jeb", "Bill", "Bob", "Gene", "Val" };
		const std::vector<std::string> bodyguards = { "Alpha", "Beta", "Gamma", "Delta", "Epsilon" };

		for (unsigned i = 0; i < corporals.size(); ++i)
		{
			std::lock_guard<std::mutex> lock(game_state);

			std::shared_ptr<Hostile_NPC_Corporal> corporal = std::make_shared<Hostile_NPC_Corporal>(corporals[i], this);
			actors[corporal->get_ID()] = corporal;
			actor_ids[corporal->name] = corporal->get_ID();

			std::shared_ptr<Hostile_NPC_Bodyguard> bodyguard = std::make_shared<Hostile_NPC_Bodyguard>(bodyguards[i], corporal->get_ID(), this);
			actors[bodyguard->get_ID()] = bodyguard;
			actor_ids[bodyguard->name] = bodyguard->get_ID();
		}
	}

	// add a corporal and a bodyguard
	if (true)
	{
		std::lock_guard<std::mutex> lock(game_state);

		std::shared_ptr<Hostile_NPC_Corporal> hunter = std::make_shared<Hostile_NPC_Corporal>("Hunter", this);
		actors[hunter->get_ID()] = hunter;
		actor_ids[hunter->name] = hunter->get_ID();

		if (true)
		{
			std::shared_ptr<Hostile_NPC_Bodyguard> guardian = std::make_shared<Hostile_NPC_Bodyguard>("Guardian", hunter->get_ID(), this);
			actors[guardian->get_ID()] = guardian;
			actor_ids[guardian->name] = guardian->get_ID();
		}
	}

	// add a bunch more corporals
	if (false)
	{
		for (char name = 'A'; name <= 'z'; ++name) // test code
		{
			std::shared_ptr<Hostile_NPC_Corporal> corporal = std::make_shared<Hostile_NPC_Corporal>(std::string() + name, this);
			actors[corporal->get_ID()] = corporal;
			actor_ids[corporal->name] = corporal->get_ID();
		}
	}

	if (false)
	{
		std::shared_ptr<Hostile_NPC_Worker> worker = std::make_shared<Hostile_NPC_Worker>("Rupert", this);
		actors[worker->get_ID()] = worker;
		actor_ids[worker->name] = worker->get_ID();
	}
}

void Game::NPC_spawn_logic()
{
	// std::cout << "Running NPC spawn logic...\n";

	std::unique_lock<std::mutex> lock(game_state);

	int player_count = 0;
	int NPC_corporal_count = 0;
	int NPC_worker_count = 0;

	// count players and hostile NPC corporals
	for (auto & actor : actors) // for each actor
	{
		if (U::is<Player_Character>(actor.second))
			++player_count;
		else if (U::is<Hostile_NPC_Corporal>(actor.second))
			++NPC_corporal_count;
		else if (U::is<Hostile_NPC_Worker>(actor.second))
			++NPC_worker_count;
	}

	// if players outnumber NPC corporals, spawn more NPC corporals, each with 3-5 followers
	if (player_count > NPC_corporal_count)
	{
		const unsigned difference = player_count - NPC_corporal_count;

		std::stringstream ss;
		ss << "There are " << difference << " more players than NPC corproals. Spawning " << difference << " corporal NPCs with followers.\n";
		std::cout << ss.str();

		for (unsigned i = 0; i < difference; ++i)
		{
			// spawn a corporal

			// generate an ID
			const std::string corporal_ID = U::to_string(U::random_int_from(0u, (unsigned)-2));

			auto corporal = std::make_shared<Hostile_NPC_Corporal>(corporal_ID, this);
			actors[corporal->get_ID()] = corporal;
			actor_ids[corporal->name] = corporal->get_ID();

			// spawn and assigned 1-4 followers
			const unsigned number_of_followers = U::random_int_from(1u, 4u);
			for (unsigned j = 0; j < number_of_followers; ++j)
			{
				const std::string name = U::to_string(U::random_int_from(0u, (unsigned)-2));

				auto NPC = std::make_shared<Hostile_NPC_Bodyguard>(name, corporal->get_ID(), this);
				actors[NPC->get_ID()] = NPC;
				actor_ids[NPC->name] = NPC->get_ID();
			}
		}
	}

	// if players outnumber NPC workers, spawn more NPC workers, each with one bodyguard
	if (player_count > NPC_worker_count)
	{
		const unsigned difference = player_count - NPC_worker_count;

		std::stringstream ss;
		ss << "There are " << difference << " more players than NPC workers. Spawning " << difference << " worker NPCs with bodyguards.\n";
		std::cout << ss.str();

		for (unsigned i = 0; i < difference; ++i)
		{
			// spawn a worker

			// generate an ID
			const std::string worker_ID = U::to_string(U::random_int_from(0u, (unsigned)-2));

			auto worker = std::make_shared<Hostile_NPC_Worker>(worker_ID, this);
			actors[worker->get_ID()] = worker;
			actor_ids[worker->name] = worker->get_ID();

			// spawn a bodyguard
			{ // temporary scope to erase the local reference after creation
				const std::string follower_ID = U::to_string(U::random_int_from(0u, (unsigned)-2));

				auto NPC = std::make_shared<Hostile_NPC_Bodyguard>(follower_ID, worker->get_ID(), this);
				actors[NPC->get_ID()] = NPC;
				actor_ids[NPC->name] = NPC->get_ID();
			}
		}
	}
}

void Game::NPC_update_logic()
{
	std::unique_lock<std::mutex> lock(game_state);

	std::vector<character_id> npc_ids;
	npc_ids.reserve(actors.size());

	for (const auto & actor : actors) // for each actor
	{
		if (const std::shared_ptr<NPC> npc = U::convert_to<NPC>(actor.second)) // if the actor is an NPC
		{
			npc_ids.push_back(npc->get_ID());
		}
	}

	while (npc_ids.size() > 0)
	{
		lock.unlock();
		// here other threads are given a chance to claim the mutex
		std::this_thread::sleep_for(std::chrono::milliseconds(1)); // hacky hack because mutexes are unfair
		lock.lock();

		const auto next_id = npc_ids.back();
		npc_ids.pop_back();

		const auto next_actor = actors.find(next_id);

		if (next_actor == actors.cend()) continue;

		if (std::shared_ptr<NPC> npc = U::convert_to<NPC>(next_actor->second)) // if the actor is an NPC
		{
			{
				std::stringstream ss;
				ss << "Calling NPC::update() on " << npc->name << ", located at "
					<< npc->get_location().to_string() << "...\n";
				std::cout << ss.str();
			}

			const Update_Messages update_messages = npc->update(); // call update, passing in the world and actors

			//std::cout << "Done NPC::update(). Generating outbound messages...\n";

			generate_outbound_messages(npc->get_ID(), update_messages);
			//std::cout << "Done generating outbound messages.\n";
		}
	}
}

void Game::outbound_thread()
{
	for (;;)
	{
		const Message message = outbound_queue.get(); // blocking

		message.connection->send(message.data);
	}
}

// helper functions

std::string Game::login_or_signup(const network::connection_ptr & connection)
{
	// return the user_ID of a user after they log in or sign up

	for (;;) // return after the user logs in or creates an account
	{
		const std::string login_instructions =
			"Type \"username\" \"password\" to log in.\n"
			"Type \"username\" \"password\" \"repeat password\" to create an account.\n"
			"*** Password encryption has not yet been implemented. ***";

		// set instructions
		outbound_queue.put(Message(connection, login_instructions));

		std::stringstream user_input;

		try
		{
			user_input << connection->read();
		}
		catch (std::exception &)
		{
			connection->close();
			return ""; // the user lost connection before logging in
		}

		const std::istream_iterator<std::string> begin(user_input);
		const std::vector<std::string> input(begin, std::istream_iterator<std::string>());

		if (input.size() == 3) // new account
		{
			const std::string user_file = C::user_data_directory + "/" + input[0] + ".xml";

			// check if the username is taken
			if (U::file_exists(user_file))
			{
				outbound_queue.put(Message(connection, "User \"" + input[0] + "\" already exists."));
				continue;
			}

			// check if the passwords match
			if (input[1] != input[2])
			{
				outbound_queue.put(Message(connection, "Passwords don't match."));
				continue;
			}

			// the user's file does not exist yet, create it using the username and password

			pugi::xml_document user_data_xml;
			user_data_xml
				.append_child(C::XML_USER_ACCOUNT.c_str())
				.append_attribute(C::XML_USER_PASSWORD.c_str())
				.set_value(input[1].c_str());

			user_data_xml.save_file(user_file.c_str());

			return input[0]; // account created
		}
		else if (input.size() == 2) // returning user
		{
			const std::string user_file = C::user_data_directory + "/" + input[0] + ".xml";

			if (!U::file_exists(user_file))
			{
				outbound_queue.put(Message(connection, "User \"" + input[0] + "\" does not exist.\n"));
				continue;
			}

			// check if the user is already logged in
			{ // temporary scope to delete mutex lock
				std::lock_guard<std::mutex> lock(game_state);
				if (actor_ids.find(input[0]) != actor_ids.cend())
				{
					outbound_queue.put(Message(connection, "User \"" + input[0] + "\" already logged in.\n"));
					continue; // try again
				}
			}

			pugi::xml_document user_data_xml;
			user_data_xml.load_file(user_file.c_str());

			if (input[1] != user_data_xml
				.child(C::XML_USER_ACCOUNT.c_str())
				.attribute(C::XML_USER_PASSWORD.c_str())
				.as_string())
			{
				outbound_queue.put(Message(connection, "Incorrect password for user \"" + input[0] + "\".\n"));
				continue;
			}

			// the password was correct
			return input[0]; // the username of the user that just logged in or signed up
		}

		// wrong input length, loop
	}
}

void Game::shutdown_listener()
{
	// Check for a server shutdown. Since the get() call is blocking, do this check here, likely for free.
	Server::wait_for_shutdown();

	// lock game state
	std::unique_lock<std::mutex> lock(game_state);

	// for each actor, remove the room around them
	for (auto & actor : actors) world->attempt_unload_radius(actor.second->get_location(), actor.first);

	// for each actor, save the actor's data
	for (auto & actor : actors) actor.second->save();

	// this is on the main thread, so "return;" destroys the game object
	return;
}

void Game::audit_world_data()
{
	std::stringstream ss;

	std::vector<Coordinate> player_coordinates;
	player_coordinates.reserve(actors.size());

	for (auto actor : actors)
		player_coordinates.push_back(actor.second->get_location());

	for (int x = 0; x < C::WORLD_X_DIMENSION; ++x)
	{
		for (int y = 0; y < C::WORLD_Y_DIMENSION; ++y)
		{
			if (world->room_at(Coordinate(x, y)) == nullptr) continue;
			
			// else, the room is loaded

			for (Coordinate coordinate : player_coordinates)
			{
				if (coordinate.diagonal_distance_to(Coordinate(x, y)) <= C::VIEW_DISTANCE)
					goto next_room;
			}

			ss << x << ", " << y << " is loaded, but no players are within view distance.\n";

		next_room: ;
		}
	}

	if (ss.str().empty())
	{
		std::cout << "World audit passed.\n";
	}
	else
	{
		std::cout << "World audit failed:\n";
		std::cout << ss.str();
		std::cout.flush();
	}
}
