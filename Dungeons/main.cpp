#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <random>
#include <ctime>
#include <map>
#include <algorithm>
#include <limits>

using namespace std;

struct Item {
    string name;
    string slot;
    int attack;
    int defense;
    int hp;
    int price;
};

struct Enemy {
    string name;
    int hp;
    int attack;
    int xp;
    int gold;
};

struct Achievement {
    string name;
    string description;
    string type;
    string difficulty;
    bool unlocked;
};

struct GameMap {
    string name;
    string difficulty;
    int width;
    int height;
    vector<string> tiles;
};

class Player {
public:
    int hp = 100;
    int maxHp = 100;
    int baseAttack = 8;
    int baseDefense = 2;
    int gold = 50;
    int xp = 0;
    int level = 1;

    vector<Item> inventory;
    map<string, Item> equipped;
    map<string, bool> hasEquipped;

    Player() {
        hasEquipped["Weapon"] = false;
        hasEquipped["Shield"] = false;
        hasEquipped["Head"] = false;
        hasEquipped["Chest"] = false;
        hasEquipped["Boots"] = false;
    }

    int getAttack() {
        int total = baseAttack;
        for (auto& e : equipped) total += e.second.attack;
        return total;
    }

    int getDefense() {
        int total = baseDefense;
        for (auto& e : equipped) total += e.second.defense;
        return total;
    }

    int xpNeeded() {
        return level * 100;
    }

    void addXp(int amount) {
        xp += amount;
        cout << "Gavai " << amount << " XP.\n";

        while (xp >= xpNeeded()) {
            xp -= xpNeeded();
            level++;
            maxHp += 15;
            hp = maxHp;
            baseAttack += 2;
            baseDefense += 1;
            gold += 25;

            cout << "\n*** LEVEL UP! ***\n";
            cout << "Pasiekei " << level << " lygi.\n";
            cout << "Max HP +15, Attack +2, Defense +1, +25 aukso.\n";
        }
    }
};

class Game {
private:
    Player player;
    GameMap currentMap;
    bool hasMap = false;

    int px = 1;
    int py = 1;

    int enemiesKilled = 0;
    int lootCollected = 0;
    int levelsCleared = 0;
    int itemsBought = 0;
    int itemsSold = 0;
    int itemsEquipped = 0;

    vector<Achievement> achievements;

    mt19937 rng;

public:
    Game() {
        rng.seed((unsigned)time(nullptr));
        createAchievements();
    }

    void run() {
        while (true) {
            cout << "\n=============================\n";
            cout << "      DUNGEON EXPLORER\n";
            cout << "=============================\n";
            cout << "1. Pradeti nauja zaidima\n";
            cout << "2. Generuoti nauja zemelapi\n";
            cout << "3. Importuoti zemelapi\n";
            cout << "4. Inventorius\n";
            cout << "5. Parduotuve\n";
            cout << "6. Pasiekimai\n";
            cout << "7. HUD\n";
            cout << "0. Iseiti\n";
            cout << "Pasirinkimas: ";

            int choice = readInt();

            if (choice == 1) {
                currentMap = generateMap(12, 8, "easy");
                hasMap = true;
                startGame();
            }
            else if (choice == 2) generateMapMenu();
            else if (choice == 3) importMap();
            else if (choice == 4) inventoryMenu();
            else if (choice == 5) storeMenu();
            else if (choice == 6) achievementMenu();
            else if (choice == 7) printHud();
            else if (choice == 0) break;
            else cout << "Blogas pasirinkimas.\n";
        }
    }

private:
    int readInt() {
        int value;
        while (!(cin >> value)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Ivesk skaiciu: ";
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return value;
    }

    int rnd(int a, int b) {
        uniform_int_distribution<int> dist(a, b);
        return dist(rng);
    }

    void createAchievements() {
        achievements = {
            {"First Blood", "Nugalek pirma priesa", "Combat", "easy", false},
            {"Goblin Hunter", "Nugalek 3 priesus", "Combat", "easy", false},
            {"Monster Slayer", "Nugalek 5 priesus", "Combat", "medium", false},
            {"Dungeon Cleaner", "Nugalek 10 priesu", "Combat", "hard", false},
            {"First Loot", "Surink pirma loot", "Loot", "easy", false},
            {"Loot Hunter", "Surink 5 loot", "Loot", "medium", false},
            {"Treasure Master", "Surink 10 loot", "Loot", "hard", false},
            {"First Level", "Pereik pirma lygi", "Progress", "easy", false},
            {"Dungeon Runner", "Pereik 3 lygius", "Progress", "medium", false},
            {"Level 2", "Pasiek 2 lygi", "XP", "easy", false},
            {"Level 5", "Pasiek 5 lygi", "XP", "hard", false},
            {"Rich Hero", "Turek 200 aukso", "Gold", "medium", false},
            {"Shopper", "Nusipirk pirma daikta", "Store", "easy", false},
            {"Merchant", "Parduok pirma daikta", "Store", "easy", false},
            {"Equipped", "Uzsidek pirma daikta", "Inventory", "easy", false},
            {"Fully Ready", "Uzsidek 5 daiktus", "Inventory", "hard", false},
            {"Survivor", "Laimedamas kova turek 10 arba maziau HP", "Combat", "veteran", false}
        };
    }

    void unlock(string name) {
        for (auto& a : achievements) {
            if (a.name == name && !a.unlocked) {
                a.unlocked = true;
                cout << "\n>>> PASIEKIMAS GAUTAS: " << a.name << " <<<\n";
                cout << a.description << "\n";
            }
        }
    }

    void checkAchievements() {
        if (enemiesKilled >= 1) unlock("First Blood");
        if (enemiesKilled >= 3) unlock("Goblin Hunter");
        if (enemiesKilled >= 5) unlock("Monster Slayer");
        if (enemiesKilled >= 10) unlock("Dungeon Cleaner");

        if (lootCollected >= 1) unlock("First Loot");
        if (lootCollected >= 5) unlock("Loot Hunter");
        if (lootCollected >= 10) unlock("Treasure Master");

        if (levelsCleared >= 1) unlock("First Level");
        if (levelsCleared >= 3) unlock("Dungeon Runner");

        if (player.level >= 2) unlock("Level 2");
        if (player.level >= 5) unlock("Level 5");

        if (player.gold >= 200) unlock("Rich Hero");

        if (itemsBought >= 1) unlock("Shopper");
        if (itemsSold >= 1) unlock("Merchant");
        if (itemsEquipped >= 1) unlock("Equipped");
        if (itemsEquipped >= 5) unlock("Fully Ready");
    }

    void printHud() {
        cout << "\n-----------------------------\n";
        cout << "HP: " << player.hp << "/" << player.maxHp;
        cout << " | Gold: " << player.gold;
        cout << " | XP: " << player.xp << "/" << player.xpNeeded();
        cout << " | Level: " << player.level << "\n";
        cout << "Attack: " << player.getAttack();
        cout << " | Defense: " << player.getDefense() << "\n";
        cout << "-----------------------------\n";
    }

    void printMap() {
        for (int y = 0; y < currentMap.height; y++) {
            for (int x = 0; x < currentMap.width; x++) {
                if (x == px && y == py) cout << "P";
                else cout << currentMap.tiles[y][x];
            }
            cout << "\n";
        }
    }

    GameMap generateMap(int width, int height, string difficulty) {
        GameMap map;
        map.name = "Generated_" + difficulty + "_" + to_string(rnd(1000, 9999));
        map.difficulty = difficulty;
        map.width = width;
        map.height = height;
        map.tiles = vector<string>(height, string(width, '.'));

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (y == 0 || y == height - 1 || x == 0 || x == width - 1) {
                    map.tiles[y][x] = '#';
                }
            }
        }

        int wallChance = 10;
        int enemyCount = 4;
        int treasureCount = 3;

        if (difficulty == "medium") {
            wallChance = 15;
            enemyCount = 6;
            treasureCount = 4;
        }
        else if (difficulty == "hard") {
            wallChance = 20;
            enemyCount = 8;
            treasureCount = 5;
        }
        else if (difficulty == "veteran") {
            wallChance = 25;
            enemyCount = 10;
            treasureCount = 6;
        }

        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                if (!(x == 1 && y == 1) && rnd(1, 100) <= wallChance) {
                    map.tiles[y][x] = '#';
                }
            }
        }

        placeTile(map, 'S');
        placeTile(map, 'X');

        for (int i = 0; i < enemyCount; i++) placeTile(map, 'E');
        for (int i = 0; i < treasureCount; i++) placeTile(map, 'T');

        map.tiles[1][1] = '.';
        return map;
    }

    void placeTile(GameMap& map, char tile) {
        for (int i = 0; i < 500; i++) {
            int x = rnd(1, map.width - 2);
            int y = rnd(1, map.height - 2);

            if (map.tiles[y][x] == '.' && !(x == 1 && y == 1)) {
                map.tiles[y][x] = tile;
                return;
            }
        }
    }

    void generateMapMenu() {
        cout << "\nPasirink sudetinguma:\n";
        cout << "1. easy\n";
        cout << "2. medium\n";
        cout << "3. hard\n";
        cout << "4. veteran\n";
        cout << "Pasirinkimas: ";

        int c = readInt();

        string difficulty = "easy";
        int w = 12;
        int h = 8;

        if (c == 2) {
            difficulty = "medium";
            w = 16;
            h = 10;
        }
        else if (c == 3) {
            difficulty = "hard";
            w = 20;
            h = 12;
        }
        else if (c == 4) {
            difficulty = "veteran";
            w = 24;
            h = 14;
        }

        currentMap = generateMap(w, h, difficulty);
        hasMap = true;
        px = 1;
        py = 1;

        cout << "\nSugeneruotas zemelapis:\n";
        printMap();

        cout << "\nIssaugoti zemelapi i JSON? 1 - taip, 0 - ne: ";
        if (readInt() == 1) saveMap();

        cout << "Startuoti zaidima? 1 - taip, 0 - ne: ";
        if (readInt() == 1) startGame();
    }

    void saveMap() {
        string filename = currentMap.name + ".json";
        ofstream file(filename);

        file << "{\n";
        file << "  \"name\": \"" << currentMap.name << "\",\n";
        file << "  \"difficulty\": \"" << currentMap.difficulty << "\",\n";
        file << "  \"width\": " << currentMap.width << ",\n";
        file << "  \"height\": " << currentMap.height << ",\n";
        file << "  \"tiles\": [\n";

        for (int i = 0; i < currentMap.height; i++) {
            file << "    \"" << currentMap.tiles[i] << "\"";
            if (i != currentMap.height - 1) file << ",";
            file << "\n";
        }

        file << "  ]\n";
        file << "}\n";

        cout << "Issaugota: " << filename << "\n";
    }

    void importMap() {
        cout << "Ivesk failo pavadinima, pvz. Generated_easy_1234.json: ";
        string filename;
        getline(cin, filename);

        ifstream file(filename);

        if (!file) {
            cout << "Failas nerastas.\n";
            return;
        }

        stringstream buffer;
        buffer << file.rdbuf();
        string text = buffer.str();

        vector<string> rows;
        size_t tilesPos = text.find("\"tiles\"");
        size_t start = text.find("[", tilesPos);
        size_t end = text.find("]", start);

        if (tilesPos == string::npos || start == string::npos || end == string::npos) {
            cout << "Blogas JSON formatas.\n";
            return;
        }

        string tileBlock = text.substr(start, end - start);

        size_t pos = 0;
        while (true) {
            size_t q1 = tileBlock.find("\"", pos);
            if (q1 == string::npos) break;

            size_t q2 = tileBlock.find("\"", q1 + 1);
            if (q2 == string::npos) break;

            rows.push_back(tileBlock.substr(q1 + 1, q2 - q1 - 1));
            pos = q2 + 1;
        }

        if (rows.empty()) {
            cout << "Nepavyko ikelti zemelapio.\n";
            return;
        }

        currentMap.name = filename;
        currentMap.difficulty = "imported";
        currentMap.tiles = rows;
        currentMap.height = rows.size();
        currentMap.width = rows[0].size();

        hasMap = true;
        px = 1;
        py = 1;
        currentMap.tiles[py][px] = '.';

        cout << "Zemelapis ikeltas.\n";
        printMap();

        cout << "Startuoti zaidima? 1 - taip, 0 - ne: ";
        if (readInt() == 1) startGame();
    }

    Item randomItem() {
        vector<Item> items = {
            {"Rusty Sword", "Weapon", 3, 0, 0, 30},
            {"Iron Sword", "Weapon", 6, 0, 0, 80},
            {"Steel Axe", "Weapon", 9, 0, 0, 130},
            {"Wooden Shield", "Shield", 0, 3, 0, 40},
            {"Iron Shield", "Shield", 0, 6, 0, 90},
            {"Leather Cap", "Head", 0, 2, 5, 35},
            {"Iron Helmet", "Head", 0, 4, 10, 75},
            {"Leather Armor", "Chest", 0, 4, 10, 60},
            {"Knight Armor", "Chest", 0, 8, 20, 140},
            {"Old Boots", "Boots", 0, 1, 5, 25},
            {"Swift Boots", "Boots", 1, 2, 10, 95},
            {"Health Potion", "Potion", 0, 0, 35, 30}
        };

        return items[rnd(0, items.size() - 1)];
    }

    Enemy randomEnemy() {
        vector<Enemy> enemies = {
            {"Goblin", 25, 6, 25, 15},
            {"Rat", 18, 4, 15, 8},
            {"Skeleton", 30, 7, 35, 20},
            {"Orc", 45, 10, 55, 35},
            {"Troll", 70, 15, 90, 60}
        };

        return enemies[rnd(0, enemies.size() - 1)];
    }

    void startGame() {
        if (!hasMap) {
            cout << "Pirma reikia sugeneruoti arba importuoti zemelapi.\n";
            return;
        }

        px = 1;
        py = 1;

        bool playing = true;

        while (playing && player.hp > 0) {
            checkAchievements();
            printHud();
            printMap();

            cout << "\nVeiksmas: W/A/S/D judeti, I inventorius, H pasiekimai, Q meniu\n";
            cout << "Ivesk: ";

            string input;
            getline(cin, input);

            if (input.empty()) continue;

            char c = toupper(input[0]);

            if (c == 'Q') playing = false;
            else if (c == 'I') inventoryMenu();
            else if (c == 'H') achievementMenu();
            else if (c == 'W' || c == 'A' || c == 'S' || c == 'D') move(c, playing);
            else cout << "Nezinomas veiksmas.\n";

            if (player.hp <= 0) {
                cout << "Tu mirei. Zaidimas baigtas.\n";
                player.hp = player.maxHp;
                playing = false;
            }
        }
    }

    void move(char dir, bool& playing) {
        int nx = px;
        int ny = py;

        if (dir == 'W') ny--;
        if (dir == 'S') ny++;
        if (dir == 'A') nx--;
        if (dir == 'D') nx++;

        char tile = currentMap.tiles[ny][nx];

        if (tile == '#') {
            cout << "Siena. Negali eiti.\n";
            return;
        }

        px = nx;
        py = ny;

        if (tile == 'E') {
            currentMap.tiles[ny][nx] = '.';
            fight();
        }
        else if (tile == 'T') {
            currentMap.tiles[ny][nx] = '.';
            openTreasure();
        }
        else if (tile == 'S') {
            storeMenu();
        }
        else if (tile == 'X') {
            cout << "\nPerejai lygi!\n";
            levelsCleared++;
            player.addXp(50);
            player.gold += 40;
            checkAchievements();

            cout << "Generuoti kita lygi? 1 - taip, 0 - meniu: ";
            if (readInt() == 1) {
                currentMap = generateMap(currentMap.width, currentMap.height, "medium");
                px = 1;
                py = 1;
            } else {
                playing = false;
            }
        }
    }

    void fight() {
        Enemy enemy = randomEnemy();

        cout << "\nSutikai priesa: " << enemy.name << "\n";

        while (enemy.hp > 0 && player.hp > 0) {
            cout << enemy.name << " HP: " << enemy.hp << "\n";
            cout << "1. Pulti\n";
            cout << "2. Naudoti potion\n";
            cout << "Pasirinkimas: ";

            int c = readInt();

            if (c == 2) {
                usePotion();
            }
            else {
                int dmg = max(1, player.getAttack() + rnd(-2, 4));
                enemy.hp -= dmg;
                cout << "Padarei " << dmg << " zalos.\n";
            }

            if (enemy.hp > 0) {
                int dmg = max(1, enemy.attack - player.getDefense() + rnd(-2, 3));
                player.hp -= dmg;
                cout << enemy.name << " tau padare " << dmg << " zalos.\n";
            }
        }

        if (player.hp > 0) {
            cout << "Nugalejai " << enemy.name << "!\n";

            enemiesKilled++;
            player.gold += enemy.gold;
            player.addXp(enemy.xp);

            cout << "Gavai " << enemy.gold << " aukso.\n";

            if (rnd(1, 100) <= 60) {
                Item item = randomItem();
                player.inventory.push_back(item);
                lootCollected++;

                cout << "Gavai item: " << item.name << "\n";
            }

            if (player.hp <= 10) unlock("Survivor");

            checkAchievements();
        }
    }

    void openTreasure() {
        cout << "\nRadai lobi!\n";

        int gold = rnd(20, 70);
        player.gold += gold;

        Item item = randomItem();
        player.inventory.push_back(item);
        lootCollected++;

        cout << "Gavai " << gold << " aukso.\n";
        cout << "Gavai item: " << item.name << "\n";

        checkAchievements();
    }

    bool usePotion() {
        for (int i = 0; i < player.inventory.size(); i++) {
            if (player.inventory[i].slot == "Potion") {
                int heal = player.inventory[i].hp;
                player.hp = min(player.maxHp, player.hp + heal);
                player.inventory.erase(player.inventory.begin() + i);
                cout << "Panaudojai potion. HP +" << heal << "\n";
                return true;
            }
        }

        cout << "Neturi potion.\n";
        return false;
    }

    void inventoryMenu() {
        while (true) {
            cout << "\nINVENTORIUS\n";
            cout << "Gold: " << player.gold << "\n";

            cout << "\nUzsideti daiktai:\n";
            for (string slot : {"Weapon", "Shield", "Head", "Chest", "Boots"}) {
                cout << slot << ": ";
                if (player.hasEquipped[slot]) cout << player.equipped[slot].name << "\n";
                else cout << "None\n";
            }

            cout << "\nDaiktai:\n";
            if (player.inventory.empty()) {
                cout << "Inventorius tuscias.\n";
            } else {
                for (int i = 0; i < player.inventory.size(); i++) {
                    Item it = player.inventory[i];
                    cout << i + 1 << ". " << it.name
                         << " [" << it.slot << "] "
                         << "ATK +" << it.attack
                         << " DEF +" << it.defense
                         << " HP +" << it.hp
                         << " Price " << it.price << "\n";
                }
            }

            cout << "\n1. Uzsideti daikta\n";
            cout << "2. Naudoti potion\n";
            cout << "3. Parduoti daikta\n";
            cout << "0. Atgal\n";
            cout << "Pasirinkimas: ";

            int c = readInt();

            if (c == 1) equipItem();
            else if (c == 2) usePotion();
            else if (c == 3) sellItem();
            else if (c == 0) break;
        }
    }

    void equipItem() {
        if (player.inventory.empty()) {
            cout << "Inventorius tuscias.\n";
            return;
        }

        cout << "Pasirink item numeri: ";
        int index = readInt() - 1;

        if (index < 0 || index >= player.inventory.size()) {
            cout << "Blogas numeris.\n";
            return;
        }

        Item item = player.inventory[index];

        if (item.slot == "Potion") {
            cout << "Potion negalima uzsideti.\n";
            return;
        }

        if (player.hasEquipped[item.slot]) {
            player.inventory.push_back(player.equipped[item.slot]);
        }

        player.equipped[item.slot] = item;
        player.hasEquipped[item.slot] = true;

        player.maxHp += item.hp;
        player.hp = min(player.maxHp, player.hp + item.hp);

        player.inventory.erase(player.inventory.begin() + index);

        itemsEquipped++;
        cout << "Uzsidejai: " << item.name << "\n";

        checkAchievements();
    }

    void sellItem() {
        if (player.inventory.empty()) {
            cout << "Inventorius tuscias.\n";
            return;
        }

        cout << "Pasirink item numeri pardavimui: ";
        int index = readInt() - 1;

        if (index < 0 || index >= player.inventory.size()) {
            cout << "Blogas numeris.\n";
            return;
        }

        Item item = player.inventory[index];
        int sellPrice = item.price / 2;

        player.gold += sellPrice;
        player.inventory.erase(player.inventory.begin() + index);

        itemsSold++;
        cout << "Pardavei " << item.name << " uz " << sellPrice << " aukso.\n";

        checkAchievements();
    }

    void storeMenu() {
        vector<Item> shop = {
            {"Iron Sword", "Weapon", 6, 0, 0, 80},
            {"Iron Shield", "Shield", 0, 6, 0, 90},
            {"Iron Helmet", "Head", 0, 4, 10, 75},
            {"Knight Armor", "Chest", 0, 8, 20, 140},
            {"Swift Boots", "Boots", 1, 2, 10, 95},
            {"Health Potion", "Potion", 0, 0, 35, 30}
        };

        while (true) {
            cout << "\n========== PARDUOTUVE ==========\n";
            cout << "Tavo gold: " << player.gold << "\n";

            for (int i = 0; i < shop.size(); i++) {
                Item it = shop[i];
                cout << i + 1 << ". " << it.name
                     << " [" << it.slot << "] "
                     << "Kaina: " << it.price << "\n";
            }

            cout << "7. Parduoti daikta\n";
            cout << "0. Atgal\n";
            cout << "Pasirinkimas: ";

            int c = readInt();

            if (c == 0) break;
            else if (c == 7) sellItem();
            else if (c >= 1 && c <= shop.size()) {
                Item item = shop[c - 1];

                if (player.gold >= item.price) {
                    player.gold -= item.price;
                    player.inventory.push_back(item);
                    itemsBought++;

                    cout << "Nusipirkai: " << item.name << "\n";
                    checkAchievements();
                } else {
                    cout << "Nepakanka aukso.\n";
                }
            }
        }
    }

    void achievementMenu() {
        while (true) {
            cout << "\nPASIEKIMAI\n";
            cout << "1. Visi\n";
            cout << "2. Gauti\n";
            cout << "3. Negauti\n";
            cout << "4. Pagal tipa\n";
            cout << "5. Pagal sudetinguma\n";
            cout << "0. Atgal\n";
            cout << "Pasirinkimas: ";

            int c = readInt();

            if (c == 0) break;
            else if (c == 1) printAchievements("all", "");
            else if (c == 2) printAchievements("unlocked", "");
            else if (c == 3) printAchievements("locked", "");
            else if (c == 4) {
                cout << "Tipai: Combat, Loot, Progress, XP, Gold, Store, Inventory\n";
                cout << "Ivesk tipa: ";
                string type;
                getline(cin, type);
                printAchievements("type", type);
            }
            else if (c == 5) {
                cout << "Sudetingumai: easy, medium, hard, veteran\n";
                cout << "Ivesk sudetinguma: ";
                string diff;
                getline(cin, diff);
                printAchievements("difficulty", diff);
            }
        }
    }

    void printAchievements(string filter, string value) {
        for (auto& a : achievements) {
            bool show = false;

            if (filter == "all") show = true;
            else if (filter == "unlocked" && a.unlocked) show = true;
            else if (filter == "locked" && !a.unlocked) show = true;
            else if (filter == "type" && a.type == value) show = true;
            else if (filter == "difficulty" && a.difficulty == value) show = true;

            if (show) {
                cout << (a.unlocked ? "[GAUTA] " : "[-----] ");
                cout << a.name << " | " << a.type << " | " << a.difficulty << "\n";
                cout << "        " << a.description << "\n";
            }
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}