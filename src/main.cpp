//main.cpp
//Usage: ./court_connect [path/to/players.csv]
//OWNER: Zachary Martus

#include "court_connect.h"
#include "menu.h"

#include <iostream>

int main(int argc, char* argv[]) {
    //use the path from the command line if one was given, otherwise fall back to the default csv
    std::string csv_path = "data/players.csv";
    if (argc > 1) {
        csv_path = argv[1];
    }

    std::vector<Player> players = load_players(csv_path);
    if (players.empty()) {
        std::cerr << "Could not load any players from " << csv_path << "\n";
        return 1;
    }

    run_app(players);
    return 0;
}