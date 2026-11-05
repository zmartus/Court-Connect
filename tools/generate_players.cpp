//generate_players.cpp
//Owner: Shrish Arun 
//makes 100,000 fake players clustered around Florida cities and writes them to data/players.csv

#include "court_connect.h"

#include <fstream>
#include <iostream>
#include <random>
#include <string>

using namespace std;

int random_int(mt19937& generator, int min_value, int max_value) {
    int range = max_value - min_value + 1;
    return min_value + generator() % range;
} 
double random_offset(mt19937& generator) {
    int number = generator() % 1001;
    return number / 1000.0 - 0.5;
}
int main() {
    ofstream file("data/players.csv");
    if (file.is_open() == false) {
        cout << "Could not open data/players.csv, make the data folder first" << endl;
        return 1;
    }
    mt19937 generator(42);
file << "playerID,skillLevel,latitude,longitude,availableDay,startMin,endMin,practiceType" << endl;

for (int i = 1; i <= 100000; i++) {
    Player player;
    player.playerID = i;
    player.skillLevel = random_int(generator, 1, 10);
    int city_index = random_int(generator, 0, FLORIDA_CITIES.size() - 1);
    player.latitude = FLORIDA_CITIES[city_index].latitude + random_offset(generator);
    player.longitude = FLORIDA_CITIES[city_index].longitude + random_offset(generator);
    player.availableDay = random_int(generator, 0, 6);
    player.startMin = 360 + random_int(generator, 0, 30) * 30;
    player.endMin = player.startMin + random_int(generator, 60, 180);
    if (player.endMin > 1439) {
        player.endMin = 1439;
}
int type_index = random_int(generator, 0, PRACTICE_TYPES.size() - 1);
        player.practiceType = PRACTICE_TYPES[type_index];
        file << player.playerID << ","
             << player.skillLevel << ","
             << player.latitude << ","
             << player.longitude << ","
             << player.availableDay << ","
             << player.startMin << ","
             << player.endMin << ","
             << player.practiceType << endl;
    }
    file.close();
    cout << "Wrote 100000 players to data/players.csv" << endl;
    return 0;
}
