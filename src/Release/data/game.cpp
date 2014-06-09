#pragma once

#include "common.h"
#include <map>
#include <stdlib.h>
#include <vector>

//Template for final game.cpp
//Sections marked with [[]] will be replaced programatically

//[[STATIC-INIT]]

game::game()
{
	winner = -1;
	current_pid = 0;
	active = false;
	
//[[BOARDS-INIT]]

//[[MAPS-INIT]]
}

//[[WIN-INIT]]

//[[MOVES-INIT]]
		
game* game::copy()
{
	game* ret = new game;
	
	ret->winner = winner;
	ret->current_pid = current_pid;
	
//[[COPY-INIT]]
	
	return ret;
}
		
std::string game::enumerate_board(int i)
{
	return boards_list[i] + " | " + common::to_string(board_width[i]) + " | [" + common::to_str_v(*boards_map[boards_list[i]])+ "]";
}
std::string game::enumerate_boards()
{
	std::string ret = "";
	
	for (int i = 0; i < boards_list.size(); i++)
	{
		ret += enumerate_board(i) + "\n";
	}
	
	return ret;
}
	
bool game::validate_move_unsafe(std::string playerType, std::string moveName, int* args)
{
	return validator_map[playerType][moveName](args);
}	
bool game::validate_move(std::string playerType, std::string moveName, int* args)
{
	std::map<std::string, std::map <std::string, common::validator>>::iterator it1 = validator_map.find(playerType);
	if(it1 == validator_map.end() ) { return false; }
	std::map<std::string, common::validator>::iterator it2 = validator_map[playerType].find(moveName);
	if(it2 == validator_map[playerType].end() ) { return false; }

	try{
		std::vector<std::vector<int>> parDomain = domain_map[playerType][moveName];
		int numPar = parDomain.size();
		
		for (unsigned int i = 0; i < numPar; i++)
		{
			if (!common::containsVec(parDomain[i], args[i]))
			{
				return false;
			}
		}
		
		return validator_map[playerType][moveName](args);
	}
	catch(int e)
	{
		return false;
	}
}
bool game::try_move(std::string playerType, std::string moveName, int* args)
{
	bool b = validate_move(playerType, moveName, args);	
	if (!b) { return false; }
	else { execute_move(playerType, moveName, args); return true; }
}
void game::execute_move(std::string playerType, std::string moveName, int* args)
{
	if (active) { 
		std::cout << "move: " << current_pid << " | " << moveName << "(";
		for (int i = 0; i < domain_map[playerType][moveName].size(); i++)
		{
			std::cout<<args[i];
			if (i != domain_map[playerType][moveName].size() - 1) { std::cout<<", "; }
		}
		std::cout<<")\n";
	}
	executor_map[playerType][moveName](args);
	if (active) { std::cout  << enumerate_boards(); }
}