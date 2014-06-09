#pragma once
#include "FastDelegate.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <vector>

//Template for final common.h
//Sections marked with [[]] will be replaced programatically

namespace common
{

	typedef fastdelegate::FastDelegate1<int*,bool> validator;
	typedef fastdelegate::FastDelegate1<int*> executor;

	template <typename T>
	bool containsVec( std::vector<T> v, T x )
	{
		for (unsigned int i = 0; i < v.size(); i++)
		{
			if (x == v[i]) { return true; }
		}
		return false;
	}

	template <typename T>
	std::string to_string(T x)
	{
		std::ostringstream os ;
		os << x ;
		return os.str() ;
	}

	template <typename T>
	std::string to_str_v (std::vector<T> x)
	{
		std::string ret = "";
		for (unsigned int i=0; i < x.size(); i++)
		{	
			ret += to_string(x[i]);
			if (i != x.size() - 1) { ret += ", "; }
		}
		return ret;
	}

	void quit()
	{
		exit(0);
	}

}

#include "game.h"

namespace common
{
	typedef void (*controller)(game*);
}

//[[GAME-UTIL]]

//[[CONTROLLER-UTIL]]