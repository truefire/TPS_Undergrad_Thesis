#pragma once
#include "common.h"
#include <stdlib.h>
#include <vector>
#include <map>
#include <string>

//Template for final game.h
//Sections marked with [[]] will be replaced programatically

class game
{
public:
	game();

	int winner;
	int current_pid;
	bool active;
	
//[[BOARDS]]
	
	std::map<std::string, std::vector<int>*> boards_map;
	
	static std::vector<std::string> boards_list;
	static std::vector<int> board_width;
	static std::string player_types[];
	static int num_players;

	static std::map <std::string, std::vector<std::string>> move_map;
	static std::map <std::string, std::map <std::string, std::vector<std::vector<int>>>> domain_map;
	
	int win_check();
	
//[[MOVES]]
	
	std::map <std::string, std::map <std::string, common::validator>> validator_map;
	std::map <std::string, std::map <std::string, common::executor>> executor_map;
	
	game* copy();
	std::string enumerate_board(int);
	std::string enumerate_boards();
	void execute_move(std::string, std::string, int*);
	bool validate_move(std::string, std::string, int*);
	bool validate_move_unsafe(std::string, std::string, int*);
	bool try_move(std::string, std::string, int*);
};