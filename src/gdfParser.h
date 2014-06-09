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
#include "game.h"

using namespace std;

//==========================================================================
//This file contains methods for parsing and generating code from a GDF file
//==========================================================================

//Parse a GDF file into a game object, for use in code generation.
game parse_GDF(ifstream &file)
{
	game gameDesc;

	using namespace boost::property_tree;

	//Read the XML tree
	ptree tree;
	read_xml(file, tree);
	ptree game = tree.get_child("game");
	
	//Top-level elements
	gameDesc.name = tree.get<string>("game.<xmlattr>.name");
	gameDesc.win = tree.get<string>("game.win");
	gameDesc.control = tree.get<string>("game.util");
	gameDesc.playerTypes = tree.get<string>("game.playerInstances");
	
	//Parse boards
	BOOST_FOREACH( ptree::value_type &v, game.get_child("boards"))
	{
		board b;
		b.width = v.second.get<int>("width");
		b.name = v.second.get<string>("<xmlattr>.label");

		string init = v.second.get<string>("init");
		boost::algorithm::trim(init);
		boost::algorithm::replace_all(init, "\n", "");
		boost::algorithm::replace_all(init, " ", "");

		b.init= parseInts(init);
		gameDesc.boards.push_back(b);
	}
	
	//Parse players
	BOOST_FOREACH( ptree::value_type &v, game.get_child("players"))
	{
		player p;
		p.name = v.second.get<string>("<xmlattr>.type");
		
		//Parse player moves
		BOOST_FOREACH( ptree::value_type &m, v.second.get_child("moves"))
		{
				gMove mv;
				mv.execute = m.second.get<string>("execute");
				mv.validate = m.second.get<string>("validate");
				mv.name = m.second.get<string>("<xmlattr>.name");
				
				//Parse move parameters
				BOOST_FOREACH( ptree::value_type &pr, m.second.get_child("params"))
				{
						param par;
						par.name = pr.second.get<string>("<xmlattr>.name");
						string values = pr.second.data();
						boost::algorithm::trim(values);
						boost::replace_all(values, "\n", "");
						boost::replace_all(values, " ", "");

						par.values = parseInts(values);

						mv.parameters.push_back(par);
				}

				p.moves.push_back(mv);
		}
		gameDesc.players.push_back(p);
	}

	return gameDesc;
}

//Generates a signiature which abstracts the fact that we're really passing an array of ints to every move function
//By assigning each of the elements in that array to a local variable, as specified by the parameters section in the GDF
string mk_signature(vector<param> params)
{
	string ret = "(int* _args){\n";
	for (unsigned int i = 0; i < params.size(); i++)
	{
		ret += "int " + params[i].name + " = _args[" + to_string(static_cast<long long>(i)) + "];\n";
	}
	return ret;
}

//Generates the header information for board data, for game.h
string gen_boards_h(game g)
{
	string ret = "";
	for (unsigned int i = 0; i < g.boards.size(); i++)
	{
		board b = g.boards[i];
		ret += "\tstd::vector<int> " + b.name + "_data;\n";
		ret += "\tstd::vector<int>* boards_" + b.name + ";\n";
	}
	return ret;
}

//Generates the header information for players moves, for game.h
string gen_moves_h(game g)
{
	string ret = "";
	for (unsigned int i = 0; i < g.players.size(); i++)
	{
		player p = g.players[i];
		for (unsigned int j = 0; j < p.moves.size(); j++)
		{
			gMove m = p.moves[j];
			ret += "\tbool " + p.name + "_validate_" + m.name + "(int*);\n";
			ret += "\tvoid " + p.name + "_execute_" + m.name + "(int*);\n";
		}
	}
	return ret;
}

//Generates static variable initalizations for game.cpp
string gen_static_cpp(game g)
{
	string ret = "";

	//Player types and number of players
	ret += "std::string game::player_types[] = {" + g.playerTypes + "};\n";
	ret += "int game::num_players = " + to_string(static_cast<long long>(countChars(g.playerTypes, ',')) + 1) + ";\n";
		
	//list of all the board names
	ret += "std::vector<std::string> game::boards_list = {";
	for (unsigned int i = 0; i < g.boards.size(); i++)
	{
		board b = g.boards[i];
		ret += "\"" + b.name + "\""; 

		if (i !=g.boards.size() - 1) { ret += ", "; }
	}
	ret += "};\n";

	//widths of each board
	ret += "std::vector<int> game::board_width = {";
	for (unsigned int i = 0; i < g.boards.size(); i++)
	{
		board b = g.boards[i];
		ret += to_string(static_cast<long long>(b.width)); 

		if (i !=g.boards.size() - 1) { ret += ", "; }
	}
	ret += "};\n";

	//map from player types to movesets
	ret += "std::map <std::string, std::vector<std::string>> game::move_map = {";
	for (unsigned int i = 0; i < g.players.size(); i++)
	{
		player p = g.players[i];

		ret += "{\"" + p.name + "\", {";
		
		for (unsigned int j = 0; j < p.moves.size(); j++)
		{
			gMove m = p.moves[j];
			ret += "\"" + m.name + "\"";
			if (j !=p.moves.size() - 1) { ret += ", "; }
		}
		
		ret += "}}";

		if (i !=g.players.size() - 1) { ret += ", "; }
	}
	ret += "};\n";

	//map from move names to possible argument values
	ret += "std::map <std::string, std::map <std::string, std::vector<std::vector<int>>>> game::domain_map = {";
	for (unsigned int i = 0; i < g.players.size(); i++)
	{
		player p = g.players[i];
		ret += "{\"" + p.name + "\", {";
		for (unsigned int j = 0; j < p.moves.size(); j++)
		{
			gMove m = p.moves[j];
			ret += "{\"" + m.name + "\", {";
			for (unsigned int k = 0; k < m.parameters.size(); k++)
			{
				param par = m.parameters[i];
				ret += "{" + to_str_v(par.values) + "}";
				if (k !=m.parameters.size () - 1) { ret += ", "; }
			}
			ret += "}}";
			if (j !=p.moves.size() - 1) { ret += ", "; }
		}
		ret += "}}";
		if (i !=g.players.size() - 1) { ret += ", "; }
	}
	ret += "};\n";

	return ret;
}

//Generates board initialization code, for the constructor in game.cpp
string gen_boards_cpp(game g)
{
	string ret = "";

	//Initialize the board data
	for (unsigned int i = 0; i < g.boards.size(); i++)
	{
		board b = g.boards[i];
		ret += "\t" + b.name + "_data = {";
		for (unsigned int j = 0; j < b.init.size(); j++)
		{
			ret += to_string(static_cast<long long>(b.init[j]));
			if (j != b.init.size() - 1) { ret += ", "; }
		}
		ret += "};\n";
		ret += "\tboards_" + b.name + " = &" + b.name + "_data;\n";
	}
	ret += "\n";
	
	//Initialize the boards map
	ret += "\tboards_map = {";
	for (unsigned int i = 0; i < g.boards.size(); i++)
	{
		board b = g.boards[i];
		ret += "{\"" + b.name + "\", boards_" + b.name + "}"; 

		if (i !=g.boards.size() - 1) { ret += ", "; }
	}
	ret += "};\n";

	return ret;
}

//Generates the maps from function names to validator/executor function pointers, for game.cpp
string gen_maps_cpp(game g)
{
	string ret = "";

	//The validator map
	ret += " \tvalidator_map = {";
	for (unsigned int i = 0; i < g.players.size(); i++)
	{
		player p = g.players[i];
		ret += "{\"" + p.name + "\", {";
		for (unsigned int j = 0; j < p.moves.size(); j++)
		{
			gMove m = p.moves[j];
			ret += "{\"" + m.name + "\", fastdelegate::MakeDelegate(this, &game::" + p.name + "_validate_" + m.name + ")}";
			if (j !=p.moves.size() - 1) { ret += ", "; }
		}
		ret += "}}";
		if (i !=g.players.size() - 1) { ret += ", "; }
	}
	ret += "};\n";

	//The executor map
	ret += "\texecutor_map = {";
	for (unsigned int i = 0; i < g.players.size(); i++)
	{
		player p = g.players[i];
		ret += "{\"" + p.name + "\", {";
		for (unsigned int j = 0; j < p.moves.size(); j++)
		{
			gMove m = p.moves[j];
			ret += "{\"" + m.name + "\", fastdelegate::MakeDelegate(this, &game::" +  p.name + "_execute_" + m.name + ")}";
			if (j !=p.moves.size() - 1) { ret += ", "; }
		}
		ret += "}}";
		if (i !=g.players.size() - 1) { ret += ", "; }
	}
	ret += "};\n";

	return ret;
}

//Generates the win function for game.cpp
string gen_win_cpp(game g)
{
	return "int game::win_check(){\n" + g.win + "\n}\n";
}

//Generates the validator and executor functions for each player's moves, for game.cpp
string gen_moves_cpp(game g)
{
	string ret = "";

	for (unsigned int i = 0; i < g.players.size(); i++)
	{
		player p = g.players[i];
		for (unsigned int j = 0; j < p.moves.size(); j++)
		{
			gMove m = p.moves[j];
			ret += "bool game::" + p.name + "_validate_" + m.name + mk_signature(m.parameters) + m.validate + "\n}\n";
			ret += "void game::" + p.name + "_execute_" + m.name + mk_signature(m.parameters) + m.execute + "\n}\n\n";
		}
	}

	return ret;
}

//Generates the copy function for game.cpp
string gen_copy_cpp(game g)
{
	string ret = "";
	
	//Copy the board data
	for (unsigned int i = 0; i < g.boards.size(); i++)
	{
		board b = g.boards[i];
		ret += "\tret->" + b.name + "_data = " + b.name + "_data;\n";
		ret += "\tret->boards_" + b.name + " = &(ret->" + b.name + "_data);\n";
	}
	ret += "\n";
	
	//Copy the boards map
	ret += "\tret->boards_map = {";
	for (unsigned int i = 0; i < g.boards.size(); i++)
	{
		board b = g.boards[i];
		ret += "{\"" + b.name + "\", ret->boards_" + b.name + "}"; 

		if (i !=g.boards.size() - 1) { ret += ", "; }
	}
	ret += "};\n\n";

	//Copy the validator map
	ret += " \tret->validator_map = {";
	for (unsigned int i = 0; i < g.players.size(); i++)
	{
		player p = g.players[i];
		ret += "{\"" + p.name + "\", {";
		for (unsigned int j = 0; j < p.moves.size(); j++)
		{
			gMove m = p.moves[j];
			ret += "{\"" + m.name + "\", fastdelegate::MakeDelegate(ret, &game::" + p.name + "_validate_" + m.name + ")}";
			if (j !=p.moves.size() - 1) { ret += ", "; }
		}
		ret += "}}";
		if (i !=g.players.size() - 1) { ret += ", "; }
	}
	ret += "};\n";

	//Copy the executor map
	ret += "\tret->executor_map = {";
	for (unsigned int i = 0; i < g.players.size(); i++)
	{
		player p = g.players[i];
		ret += "{\"" + p.name + "\", {";
		for (unsigned int j = 0; j < p.moves.size(); j++)
		{
			gMove m = p.moves[j];
			ret += "{\"" + m.name + "\", fastdelegate::MakeDelegate(ret, &game::" +  p.name + "_execute_" + m.name + ")}";
			if (j !=p.moves.size() - 1) { ret += ", "; }
		}
		ret += "}}";
		if (i !=g.players.size() - 1) { ret += ", "; }
	}
	ret += "};\n";

	return ret;
}