#pragma once
#include <iostream>
#include <vector>
#include "board.h"
#include "player.h"

using namespace std;

//encapsulates a <game> tag in a GDF file
class game
{
public:
	string name;				//name of the game
	vector<board> boards;		//list of the game boards
	vector<player> players;		//list of the player types
	string playerTypes;			//text of the <playerInstances> tag. Defines the type of each pid
	string control;				//GDF util text
	string win;					//text of the <win> tag
};