<controllers>
<controller name = "FeedThePig_Standard">
	<![CDATA[
	//This basic strategy is to feed the pig so that he has eaten a multiple of 5
	//This is always possible unless he is already at a multiple of 5, thus your opponent
	//will not be able to do this if you can.
	//He will eventually reach 20 at the end of one of your turns, after which
	//your opponent will be forced to lose the game
	
	//Ammount to feed, ideally
	int feed_amt = 5 - ((*(_game->boards_food))[0] % 5);
	int prev = (*(_game->boards_prev))[0];
	
	//If that doesn't work, default to 1 or 2
	if (feed_amt == 5 || feed_amt == prev)
	{
		if (prev == 1) { feed_amt = 2; }
		else { feed_amt = 1; }
	}
	
	//Try the move, then evaluate win conditions
	_game->try_move(_game->player_types[_game->current_pid], "feed", &feed_amt);
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
</util>