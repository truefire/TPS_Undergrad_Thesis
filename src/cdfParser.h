#pragma once

#include <vector>
#include <iostream>
#include <stdlib.h>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <Windows.h>

#include "common.h"
#include "controller.h"

using namespace std;

//==========================================================================
//This file contains methods for parsing and generating code from a CDF file
//==========================================================================

//Header for the controller.h file
string controller_headers = "#pragma once\n\n#include \"common.h\"\n#include <map>\n#include \"game.cpp\"\n\n";

//Generates the controller map (maps controller names to control function) for a list of controllers, for controllers.h
string genControllerMap( vector<controller> controllers )
{
	string ret = "std::map<std::string, common::controller> controller_map = {";

	for (unsigned int i = 0; i < controllers.size(); i++)
	{
		ret += "{\"" + controllers[i].name + "\", controller_" + controllers[i].name + "}";
		
		if (i !=controllers.size() - 1) { ret += ", "; }
	}
	ret += "};";
	return ret;
}

//Parses the controller list from a CDF file
vector<controller> parseControllers(ifstream &file)
{
	using namespace boost::property_tree;

	vector<controller> controllers;
	
	ptree tree;
	read_xml(file, tree);

	BOOST_FOREACH( ptree::value_type &v, tree.get_child("controllers"))
	{
		controller c;
		c.eval = v.second.data();
		c.name = v.second.get<string>("<xmlattr>.name");
		controllers.push_back(c);
	}

	return controllers;
}

//Generates control function code from a list of controllers, for controllers.h
string genControllers( vector<controller> controllers)
{
	
	string ret = "";

	for (unsigned int i = 0; i < controllers.size(); i++)
	{
		ret += "void controller_" + controllers[i].name + "(game* _game)\n{" + controllers[i].eval + "}\n";
	}
	return ret;
}

//Parses the util code from a CDF file
string parseUtil(ifstream &file)
{
	using namespace boost::property_tree;

	file.clear();
	file.seekg(0, ios::beg);

	ptree tree;
	read_xml(file, tree);

	return tree.get<string>("util");
}