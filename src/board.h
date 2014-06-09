#pragma once
#include <iostream>
#include <vector>

using namespace std;

//encapsulates a <board> tag in a GDF file
class board
{
public:
	string name;		//name of the board
	int width;			//width of the board
	vector<int> init;	//init board data
};


