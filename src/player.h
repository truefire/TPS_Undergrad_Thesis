#pragma once
#include <iostream>
#include <vector>
#include "move.h"

using namespace std;

//encapsulates a <player> tag in a GDF file
class player
{
public:
	string name;			//name of the player type
	vector<gMove> moves;	//list of the player's moves
};