#pragma once

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <boost/algorithm/string.hpp>

//==================================================================
//This file contains utility functions for use elsewhere in the code
//==================================================================

//parses a comma delimited list of ints to a vector
std::vector<int> parseInts(std::string s)
{
	std::vector<int> v;

	std::string s2 = boost::algorithm::trim_copy(s);

	std::stringstream ss(s2);
	int i;

	while (ss >> i)
	{
		v.push_back(i);

		if (ss.peek() == ',')
			ss.ignore();
	}
	return v;
}

//counts the number of occurances of a given character in a string
int countChars(std::string s, char c)
{
	int n = 0;
	for(unsigned int i = 0; i < s.length(); i++)
	{
		if(s[i] == c) {n++;}
	}
	return n;
}

//converts a vector of ints to a comma delimited string of ints
std::string to_str_v (std::vector<int> x)
{
	std::string ret = "";
	for (unsigned int i=0; i < x.size(); i++)
	{	
		ret += std::to_string(static_cast<long long>(x[i]));
		if (i != x.size() - 1) { ret += ", "; }
	}
	return ret;
}