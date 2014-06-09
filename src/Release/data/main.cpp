#include <vector>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <chrono>

#include "common.h"
#include "FastDelegate.h"
#include "game.h"
#include "controller.h"

using namespace std;
using namespace common;

namespace main_execution
{

	vector<controller> player_controllers;	//holds the controllers that play the game
	game main_game;							//holds the main game instance

	//entry point
	void start(int argc, char* argv[])
	{
		//seed RNG for convenience of controllers
		srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());

		//read arguments
		for (int i = 1; i < argc; i++) { player_controllers.push_back(controller_map[argv[i]]); }
		
		//Set game to active
		main_game.active = true;
		
		//main loop
		while(true)
		{
			player_controllers[main_game.current_pid](&main_game);
		}
	}
}

//formal entry point
int main(int argc, char* argv[])
{
	//Check number of args
	if (argc != game::num_players + 1) {cout << "Invalid number of arguments provided. Got " << (argc-1) << ", require " << game::num_players << ".\n"; return 1;}

	//Start execution
	main_execution::start(argc, argv);
	return 0;
}