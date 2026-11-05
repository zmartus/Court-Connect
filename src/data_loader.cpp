//data_loader.cpp
//OWNER: Shrish Arun

#include "court_connect.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
vector<Player> load_players(const string& csv_path) {
    vector<Player> players;
    ifstream file(csv_path);
    if (file.is_open() == false) {
        return players;
    }
    string line;
    getline(file, line);
    while (getline(file, line)){
        for (unsigned int i = 0; i < line.size(); i++) {
            if (line[i] == ',') {
                line[i] = ' ';
            }
        }
    stringstream input(line);
    Player player;
    input >> player.playerID >> player.skillLevel >> player.latitude >> player.longitude >> player.availableDay >> player.startMin >> player.endMin >> player.practiceType;
    if (input.fail()) {
            continue;
        }

        players.push_back(player);
    }

    file.close();
    return players;
}