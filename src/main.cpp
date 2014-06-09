#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <Windows.h>
#include <direct.h>

#include "common.h"
#include "game.h"
#include "controller.h"

#include "gdfParser.h"
#include "cdfParser.h"

using namespace std;

//Generate the game source files
void gen_game(ifstream &gdf, ifstream &cdf)
{
	CreateDirectory("intermediary", NULL);
	
	//Open files
	ofstream game_h("intermediary/game.h");
	ofstream game_cpp("intermediary/game.cpp");
	ofstream controller_h("intermediary/controller.h");
	ofstream common_h("intermediary/common.h");
	ofstream main_cpp("intermediary/main.cpp");
	ofstream fast_delegate_h("intermediary/FastDelegate.h");
	ofstream fast_delegate_bind_h("intermediary/FastDelegateBind.h");
	ifstream infile;

	cout<<"Temp files ready for writing.\n";

	//Parse input files
	game gameDesc = parse_GDF(gdf);
	cout<<"GDF parsed.\n";
	vector<controller> controllers = parseControllers(cdf);
	string controller_util = parseUtil(cdf);
	cout<<"CDF parsed.\n";

	
	//Game
	infile.open("data/game.h");
	std::string game_hs((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
	boost::algorithm::replace_all(game_hs,"//[[BOARDS]]", gen_boards_h(gameDesc));
	boost::algorithm::replace_all(game_hs,"//[[MOVES]]", gen_moves_h(gameDesc));
	game_h << game_hs;
	infile.close();

	cout<<"game.h generated."<<endl;

	infile.open("data/game.cpp");
	std::string game_cs((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
	boost::algorithm::replace_all(game_cs,"//[[STATIC-INIT]]", gen_static_cpp(gameDesc));
	boost::algorithm::replace_all(game_cs,"//[[BOARDS-INIT]]", gen_boards_cpp(gameDesc));
	boost::algorithm::replace_all(game_cs,"//[[MAPS-INIT]]", gen_maps_cpp(gameDesc));
	boost::algorithm::replace_all(game_cs,"//[[WIN-INIT]]", gen_win_cpp(gameDesc));
	boost::algorithm::replace_all(game_cs,"//[[MOVES-INIT]]", gen_moves_cpp(gameDesc));
	boost::algorithm::replace_all(game_cs,"//[[COPY-INIT]]", gen_copy_cpp(gameDesc));
	game_cpp << game_cs;
	infile.close();

	cout<<"game.cpp generated."<<endl;
	
	//Controller
	controller_h << controller_headers + genControllers(controllers) + genControllerMap(controllers);

	cout<<"controller.h generated."<<endl;
  
	//Common
	infile.open("data/common.h");
	std::string common_hs((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
	boost::algorithm::replace_all(common_hs,"//[[GAME-UTIL]]", gameDesc.control);
	boost::algorithm::replace_all(common_hs,"//[[CONTROLLER-UTIL]]", controller_util);
	common_h << common_hs;
	infile.close();

	cout<<"common.h generated."<<endl;
  
	//Main
	infile.open("data/main.cpp");
	main_cpp << infile.rdbuf();
	infile.close();

	cout<<"main.cpp generated."<<endl;

	//FastDelegate library
	infile.open("data/FastDelegate.h");
	fast_delegate_h << infile.rdbuf();
	infile.close();
	infile.open("data/FastDelegateBind.h");
	fast_delegate_bind_h << infile.rdbuf();
	infile.close();

	cout<<"FastDelegate files transferred."<<endl;
	
	//Close files
	game_h.close();
	game_cpp.close();
	controller_h.close();
	common_h.close();
	main_cpp.close();
}

//Removes  temporary files
void cleanup()
{	
	
	if(
		!remove("intermediary/game.h")&&
		!remove("intermediary/game.cpp")&&
		!remove("intermediary/controller.h")&&
		!remove("intermediary/common.h")&&
		!remove("intermediary/main.cpp")&&
		!remove("intermediary/FastDelegate.h")&&
		!remove("intermediary/FastDelegateBind.h")
	  )
	{
		_rmdir("intermediary");
	}
}


//Entry point for the program
int main(int argc, char* argv[])
{
	//Handle arguments
	if (argc != 4) {cout << "Invalid number of arguments provided.\n"; return 1;}

	string gdf_name = argv[1];
	string cdf_name = argv[2];
	string out_name = argv[3];
	
	//Open files
	ifstream gdf_file(gdf_name, ios_base::in);
	ifstream cdf_file(cdf_name, ios_base::in);
	
	if (!gdf_file.is_open()) {cout << "File not found: "<<gdf_name.c_str()<<endl; return 2;}
	if (!cdf_file.is_open()) {cout << "File not found: "<<cdf_name.c_str()<<endl; return 2;}
	cout<<"Opened files.\n";

	//Generate the game source files
	gen_game(gdf_file, cdf_file);
	cout<<"Done generating files.\n";
	cout<<"Compiling results...\n";

	//Compile
	string cmd = "g++ -O2 -std=c++0x -enable-auto-import -o " + out_name + " intermediary/main.cpp";
	int result = system(cmd.c_str());
	if (result == 0)
	{
		cout<<"Compilation done.";
	}

	//Cleanup and exit
	cleanup();
	return 0;
}