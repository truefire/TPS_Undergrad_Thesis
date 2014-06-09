#pragma once
#include <iostream>
#include <vector>

using namespace std;

//encapsulates a <controller> tag in a CDF file
class controller
{
public:
	string name;	//name of the controller
	string eval;	//text of the controller function
};