<controllers>
<controller name="PlayerController">
	<![CDATA[
	
		//This controller allows a human user (or another program, via piping)
		//to play the game by passing in moves via stdin. The format is
		//[move name] [argument 1] [argument 2] ...
	
		//Prompt and get input
		std::cout<<"msg: Waiting for inpput, Player "<<_game->current_pid<<": ";
		std::string s;
		getline(std::cin, s);
		
		//Parse into move name and arguments
		std::string fName;
		std::vector<int> args;
		size_t pos = s.find(" ");
		if (pos != std::string::npos)
		{
			fName = s.substr(0, pos);
			s.erase(0, pos + 1);
		
			std::string token;
			while ((pos = s.find(" ")) != std::string::npos) {
				token = s.substr(0, pos);
				args.push_back(atoi(token.c_str()));
				s.erase(0, pos + 1);
			}
			args.push_back(atoi(s.c_str()));
		}
		else
		{
			fName = s;
			s = "";
		}
		
		//Attempt to execute the move
		if (_game->validate_move(_game->player_types[_game->current_pid], fName, &args[0]))
		{
			_game->execute_move(_game->player_types[_game->current_pid], fName, &args[0]);
			_game->winner = _game->win_check();
			if (_game->winner >= 0)
			{
				std::cout<<"win: "<<_game->winner<<std::endl;
				common::quit();
			}
			else if (_game->winner == -2)
			{
				std::cout<<"draw"<<std::endl;
				common::quit();
			}
		}
		else
		{
			std::cout<<"msg: invalid move."<<std::endl;
		}
	]]>
</controller>
<controller name="RandomController">
	<![CDATA[
	
		//This controller will execute random moves. It doesn't keep track of what moves
		//have been invalidated, so it may try the same invalid move multiple times
		//It should work fine unless the ratio of valid moves to invalid moves is
		//incredibly small
	
		//Get all the move types
		std::vector < std::string >  moveTypes = _game->move_map[_game->player_types[_game->current_pid]];

		//Keep trying until we get a valid move
		while (true)
		{
			//Pick a random move type, and get the domain of that move
			std::string fName = moveTypes[rand() % moveTypes.size()];
			std::vector < std::vector < int >  >  domain = _game->domain_map[_game->player_types[_game->current_pid]][fName];
			
			//Pick some random arguments from the domain
			std::vector<int> moveArgs;
			for (unsigned int i = 0; i  <  domain.size(); i++)
			{
				moveArgs.push_back(domain[i][rand() % domain[i].size()]);
			}
			
			//Attempt to execute the move
			if (_game->validate_move(_game->player_types[_game->current_pid], fName, &moveArgs[0]))
			{
				_game->execute_move(_game->player_types[_game->current_pid], fName, &moveArgs[0]);
				_game->winner = _game->win_check();
				if (_game->winner >= 0)
				{
					std::cout<<"win: "<<_game->winner<<std::endl;
					common::quit();
				}
				else if (_game->winner == -2)
				{
					std::cout<<"draw"<<std::endl;
					common::quit();
				}
				break;
			}
			
		}
	]]>
</controller>
<controller name = "AcyclicMinimax">
	<![CDATA[
	
	//This controller uses the Minimax algorithm (see http://en.wikipedia.org/wiki/Minimax )
	//to choose its moves. An important property of the minimax algorithm is that it will
	//Always force a win if possible, and if not, force a draw if possible.
	
	//Enumerate all possible moves
	std::vector<controller_util::move> move_space = controller_util::enumerate_moves(_game);
	
	//Hold the game, move, and score of the move for each branch
	std::vector<game*> branches;
	std::vector<controller_util::move> branch_moves;
	std::vector<int> branch_scores;
	
	//Validate the move for each branch, throwing away invalid moves
	for (int i = 0; i < move_space.size(); i++)
	{
		int* args = &move_space[i].params[0];
		game* newGame = _game->copy();
		if(newGame->validate_move_unsafe(_game->player_types[_game->current_pid], move_space[i].name, args))
		{
			newGame->execute_move(_game->player_types[_game->current_pid], move_space[i].name, args);
			newGame->winner = newGame->win_check();
			branches.push_back(newGame);
			branch_moves.push_back(move_space[i]);
		}
	}
	
	//Recursively perform minimax to get the score for each branch
	for (int j = 0; j < branches.size(); j++)
	{
		branch_scores.push_back(controller_util::minimax_a(branches[j], _game->current_pid));
	}
	
	//Find the branch with the highest score, and choose that move
	int index = std::max_element(branch_scores.begin(), branch_scores.end()) - branch_scores.begin();
	controller_util::move best_move = branch_moves[index];
	_game->try_move(_game->player_types[_game->current_pid], best_move.name, &best_move.params[0]);
	_game->winner = _game->win_check();
	if (_game->winner >= 0)
	{
		std::cout<<"win: "<<_game->winner<<std::endl;
		common::quit();
	}
	else if (_game->winner == -2)
	{
		std::cout<<"draw"<<std::endl;
		common::quit();
	}
	
	]]>
</controller>
<controller name = "Statistical">
	<![CDATA[
	
	//This controller will simulate a number of random games for each possible branche
	//Then choose the branch in which it won the most games.
	//It will spend 1 second running the simulations, allowing for a constant-time evaluation regardles
	//of the game being played
	
	//Enumerate all possible moves
	std::vector<controller_util::move> move_space = controller_util::enumerate_moves(_game);
	
	//Hold the game, move, and score of the move for each branch
	std::vector<game*> branches;
	std::vector<controller_util::move> branch_moves;
	std::vector<int> branch_scores;
	
	//Validate the move for each branch, throwing away invalid moves
	for (int i = 0; i < move_space.size(); i++)
	{
		int* args = &move_space[i].params[0];
		game* newGame = _game->copy();
		if(newGame->validate_move_unsafe(_game->player_types[_game->current_pid], move_space[i].name, args))
		{
			newGame->execute_move(_game->player_types[_game->current_pid], move_space[i].name, args);
			newGame->winner = newGame->win_check();
			branches.push_back(newGame);
			branch_moves.push_back(move_space[i]);
			branch_scores.push_back(0);
		}
	}
	
	//Simulate games for 1s
	auto startTime = std::chrono::high_resolution_clock::now();
	auto endTime = std::chrono::high_resolution_clock::now();
	while(endTime - startTime < std::chrono::seconds(1))
	{
		for (int i = 0; i < branches.size(); i++)
		{
			if(branches[i]->winner == -1)
			{
				game* sim_game = branches[i]->copy();
				controller_util::simulate_random_game(sim_game);
				if(sim_game->winner == _game->current_pid) { branch_scores[i]++; }
				else if (sim_game->winner != -2) { branch_scores[i]--; }
			}
		}
		endTime = std::chrono::high_resolution_clock::now();
	}
	
	//Execute the branch that won the most games
	int index = std::max_element(branch_scores.begin(), branch_scores.end()) - branch_scores.begin();
	controller_util::move best_move = branch_moves[index];
	_game->try_move(_game->player_types[_game->current_pid], best_move.name, &best_move.params[0]);
	_game->winner = _game->win_check();
	if (_game->winner >= 0)
	{
		std::cout<<"win: "<<_game->winner<<std::endl;
		common::quit();
	}
	else if (_game->winner == -2)
	{
		std::cout<<"draw"<<std::endl;
		common::quit();
	}
	
	]]>
</controller>
<controller name = "StatSim">
	<![CDATA[
	
	//This controller works similarly to the Statistical controller, except that the simulated games are not
	//random, but instead consider that both players are themselves running statistical algorithms (with smaller
	//sample sizes as you progress deeper, to prevent balooning of the number of simulations.
	
	//Enumerate all possible moves
	std::vector<controller_util::move> move_space = controller_util::enumerate_moves(_game);
	
	//Hold the game, move, and score of the move for each branch
	std::vector<game*> branches;
	std::vector<controller_util::move> branch_moves;
	std::vector<int> branch_scores;
	
	//Validate the move for each branch, throwing away invalid moves
	for (int i = 0; i < move_space.size(); i++)
	{
		int* args = &move_space[i].params[0];
		game* newGame = _game->copy();
		if(newGame->validate_move_unsafe(_game->player_types[_game->current_pid], move_space[i].name, args))
		{
		//std::cout<<"#:"<<i<<"  feed:"<<args[0]<<std::endl;
			newGame->execute_move(_game->player_types[_game->current_pid], move_space[i].name, args);
			newGame->winner = newGame->win_check();
			branches.push_back(newGame);
			branch_moves.push_back(move_space[i]);
			branch_scores.push_back(0);
		}
	}
	
	//Perform the simulation for each branch
	for (int i = 0; i < branches.size(); i++)
	{
		if(branches[i]->winner == -1)
		{
			branch_scores[i] = controller_util::statsim(branches[i], _game->current_pid, 8,2);
		}
	}
	
	//Execute the branch that got the highest score
	int index = std::max_element(branch_scores.begin(), branch_scores.end()) - branch_scores.begin();
	controller_util::move best_move = branch_moves[index];
	_game->try_move(_game->player_types[_game->current_pid], best_move.name, &best_move.params[0]);
	_game->winner = _game->win_check();
	if (_game->winner >= 0)
	{
		std::cout<<"win: "<<_game->winner<<std::endl;
		common::quit();
	}
	else if (_game->winner == -2)
	{
		std::cout<<"draw"<<std::endl;
		common::quit();
	}
	
	]]>
</controller>
</controllers>
<util>
	<![CDATA[
	
#include <algorithm>
#include <chrono>
#include <random>
	
namespace controller_util
{
	//This structure defines a distinct move -- a name and a set of values to pass in as arguments
	typedef struct {std::string name; std::vector<int> params; } move;
	
	//A method for scoring a game state, from the perspective of a specific player
	int score(game* g, int pid)
	{
		if(g->winner < 0) { return 0; }
		else if(g->winner == pid) { return 1; }
		else { return -1; }
	}
	
	//Enumerates all moves available to the current player of a game
	//This uses an iterative combinatoric method in which counting is done with a "multi-base"
	//number, where each digit has base equal to the size of one of the vectors to be combined
	std::vector<move> enumerate_moves(game* g)
	{
		std::string p_type = g->player_types[g->current_pid];
		
		std::vector<move> ret;
		
		for (int i = 0; i < g->move_map[p_type].size(); i++)
		{
			std::string m_name = g->move_map[p_type][i];
			std::vector<std::vector<int>> domain = g->domain_map[p_type][m_name];
			
			std::vector<int> digits;
			for (int j = 0; j < domain.size(); j++) { digits.push_back(0); }
			while (digits[0] < domain[0].size())
			{
				move m;
				m.name = m_name;
				for (int k = 0; k < digits.size(); k++)
				{
					m.params.push_back(domain[k][digits[k]]);
				}
				ret.push_back(m);
				
				digits[digits.size()-1]++;
				for (int k = digits.size()-1; k > 0; k--)
				{
					if(digits[k] > domain[k].size()-1)
					{
						digits[k] = 0;
						digits[k-1] += 1;
					}
					else { break; }
				}
			}
		}
		
		return ret;
	}
	
	//Simulates a game with random moves until a terminal state
	void simulate_random_game(game* g)
	{
		std::vector<controller_util::move> move_space = controller_util::enumerate_moves(g);
		std::random_shuffle(move_space.begin(), move_space.end());
		for (int i = 0; i < move_space.size(); i++)
		{
			int* args = &move_space[i].params[0];
			if(g->validate_move_unsafe(g->player_types[g->current_pid], move_space[i].name, args))
			{
				g->execute_move(g->player_types[g->current_pid], move_space[i].name, args);
				g->winner = g->win_check();
				break;
			}
		}
		if (g->winner == -1)
		{
			simulate_random_game(g);
		}
	}
	
	//Function version of the statsim controller to allow for recursion
	//fact is the amount of sub-simualtions to do
	//decay is the factor by which to decrease the number of sub-simulations at every depth
	int statsim(game* g, int pid, int fact, int decay)
	{
		g->winner = g->win_check();
		if(g->winner != -1) { return score(g, pid); }
		std::vector<controller_util::move> move_space = controller_util::enumerate_moves(g);
		std::vector<game*> branches;
		std::vector<controller_util::move> branch_moves;
		std::vector<int> branch_scores;
		for (int i = 0; i < move_space.size(); i++)
		{
			int* args = &move_space[i].params[0];
			game* newGame = g->copy();
			if(newGame->validate_move_unsafe(g->player_types[g->current_pid], move_space[i].name, args))
			{
				newGame->execute_move(g->player_types[g->current_pid], move_space[i].name, args);
				newGame->winner = newGame->win_check();
				branches.push_back(newGame);
				branch_moves.push_back(move_space[i]);
				branch_scores.push_back(0);
			}
		}
		
		for (int itt = 0; itt < fact; itt++)
		{
			for (int i = 0; i < branches.size(); i++)
			{
				if(branches[i]->winner == -1)
				{
					game* sim_game = branches[i]->copy();
					if (decay < fact)
					{
						int newfact = fact/decay;
						int numwins = statsim(sim_game, pid, newfact, decay);
						branch_scores[i] += numwins;
					}
					else
					{
						simulate_random_game(sim_game);
						if(sim_game->winner == pid) { branch_scores[i]++; }
						else if (sim_game->winner != -2) { branch_scores[i]--; }
					}
				}
				else
				{
					branch_scores[i] += score(branches[i], pid);
				}
			}
		}
		
		if (g->current_pid == pid)
		{
			return *std::max_element(branch_scores.begin(), branch_scores.end());
		}
		else
		{
			return *std::min_element(branch_scores.begin(), branch_scores.end());
		}
	}
	
	//Function version of the minimax controller to allow for recursion.
	//The "_a" indicates that it is acyclic (ie: does not handle cycles).
	int minimax_a(game* g, int pid)
	{
		if(g->winner != -1) { return score(g, pid); }
		
		std::vector<move> move_space = enumerate_moves(g);
		
		std::vector<int> scores = {};
		
		for (int i = 0; i < move_space.size(); i++)
		{
			int* args = &move_space[i].params[0];
			
			game* newGame = g->copy();
			if(newGame->validate_move_unsafe(g->player_types[g->current_pid], move_space[i].name, args))
			{
				newGame->execute_move(g->player_types[g->current_pid], move_space[i].name, args);
				newGame->winner = newGame->win_check();
				int moveVal = minimax_a(newGame, pid);
				
				if (moveVal == 1 && g->current_pid == pid) { return 1; }
				if (moveVal == -1 && g->current_pid != pid) { return -1; }
				scores.push_back( moveVal );
				//moves.push(/*...*/);
			}
		}
		if (scores.size() == 0) { std::cout<<"Error: No valid moves."; }
		if (g->current_pid == pid)
		{
			return *std::max_element(scores.begin(), scores.end());
		}
		else
		{
			return *std::min_element(scores.begin(), scores.end());
		}
		
	}
	
}
	]]>
</util>