#pragma once
#include <iostream>
#include <vector>
#include "param.h"

using namespace std;

//encapsulates a <move> tag in a GDF file
class gMove
{
public:
	string name;					//name of the move
	vector<param> parameters;		//list of the parameters to the move
	string validate;				//text of the <validate> tag
	string execute;					//text of the <execute> tag
};