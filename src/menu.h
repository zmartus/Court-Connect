//menu.h
//OWNER: Zachary Martus
//all the terminal screens: Main Menu, Search Form, Match Results, Algorithm Comparison
 
#pragma once
 
#include "court_connect.h"
 
#include <iostream>
#include <vector>
 
//keeps the menu going until the user picks Exit or the input runs out
//the streams are parameters so the tests can feed the menu scripted input instead of typing
void run_app(const std::vector<Player>& players, std::istream& in = std::cin, std::ostream& out = std::cout);
 