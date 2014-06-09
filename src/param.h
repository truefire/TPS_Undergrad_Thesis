#pragma once
#include <iostream>
#include <vector>

using namespace std;

//encapsulates a <param> tag in a GDF file
class param
{
public:
	string name;			//name of the parameter
	vector<int> values;		//possible values that can be passed in
};