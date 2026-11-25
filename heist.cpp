#include "heist.h"
#include <cstdlib>
#include <ctime>
#include <limits>
#include <cctype>

ostream& operator<<(ostream& out, const Tile& t) { out << (t.filled ? t.tileLetter : '_'); return out; }
istream& operator>>(istream& in, Tile& t) { in >> t.tileLetter; t.filled = true; return in; }
ostream& operator<<(ostream& out, const Location& l) { out << l.locName << " [" << l.letterClueLoc << "]"; return out; }
istream& operator>>(istream& in, Location& l) { in >> l.locName >> l.letterClueLoc; return in; }

void TileRack::add(char c) { letters.push(c); }
bool TileRack::contains(char c) const { for (int i = 0;i < letters.size();++i) if (letters[i] == c) return true; return false; }
int TileRack::size() const { return letters.size(); }
char TileRack::operator[](int i) const { return letters[i]; }
TileRack operator+(const TileRack& a, const TileRack& b) {
    TileRack result;
    for (int i = 0;i < a.size();++i) result.add(a[i]);
    result.add(' ');
    for (int i = 0;i < b.size();++i) result.add(b[i]);
    return result;
}

Scrabble::Scrabble() { words[0] = ""; words[1] = ""; solved[0] = solved[1] = false; }
void Scrabble::setWords(const string& a, const string& b) { words[0] = a; words[1] = b; }
void Scrabble::addClue(WordRow row, char c) { collected[(int)row].push(c); }
void Scrabble::display() const {
    cout << "\n----------- SCRABBLE BOARD -----------\n";
    for (int r = 0;r < 2;++r) {
        cout << (r == 0 ? "Word 1: " : "Word 2: ");
        for (unsigned int i = 0;i < words[r].size();++i) {
            char shown = '?';
            for (int j = 0;j < collected[r].size();++j) if (j == (int)i) shown = collected[r][j];
            if (solved[r]) cout << words[r][i] << ' ';
            else cout << shown << ' ';
        }
        cout << "\n";
    }
    cout << "Letters collected: " << collected[0].size() << " / " << collected[1].size() << "\n";
}
bool Scrabble::formWord(WordRow row) {
    int r = (int)row;
    if (words[r].empty()) return false;
    if (collected[r].size() < (int)words[r].size()) return false;
    // The word is validated against the clue letters in their supplied order.
    for (unsigned int i = 0;i < words[r].size();++i) {
        if (i >= (unsigned int)collected[r].size() ||
            toupper(collected[r][(int)i]) != toupper(words[r][i])) return false;
    }
    solved[r] = true;
    return true;
}
bool Scrabble::allSolved() const { return solved[0] && solved[1]; }
string Scrabble::getWord(WordRow row) const { return words[(int)row]; }

Character::Character() : characterName("Unknown"), characterRole("Unknown"), nationality("Unknown"), isCulprit(false), interviewed(false) {}
Character::Character(const string& n, const string& r, const string& nation)
    : characterName(n), characterRole(r), nationality(nation), isCulprit(false), interviewed(false) {
}
string Character::getName() const { return characterName; }
string Character::getRole() const { return characterRole; }
bool Character::culprit() const { return isCulprit; }
void Character::setCulprit(bool v) { isCulprit = v; }
bool Character::wasInterviewed() const { return interviewed; }
void Character::setInterviewed(bool v) { interviewed = v; }
ostream& operator<<(ostream& out, const Character& c) { c.display(); return out; }

Suspect::Suspect(const string& n, const string& r, const string& nation, char l, int p)
    : Character(n, r, nation), wordLetter(l), wordPosition(p) {
}
void Suspect::display() const { cout << characterName << " (" << characterRole << "), " << nationality; }
char Suspect::getLetter() const { return wordLetter; }
int Suspect::getPosition() const { return wordPosition; }
BusinessRelations::BusinessRelations(const string& n, const string& r, const string& nation, char l, int p)
    : Character(n, r, nation), Suspect(n, r, nation, l, p) {
}
CloseRelations::CloseRelations(const string& n, const string& r, const string& nation, char l, int p)
    : Character(n, r, nation), Suspect(n, r, nation, l, p) {
}
HouseHelp::HouseHelp(const string& n, const string& r, const string& nation, char l, int p)
    : Character(n, r, nation), Suspect(n, r, nation, l, p) {
}
Partner::Partner(const string& n, const string& nation, char l, int p)
    : Character(n, "Partner", nation), Suspect(n, "Partner", nation, l, p), BusinessRelations(n, "Partner", nation, l, p) {
}
PA::PA(const string& n, const string& nation, char l, int p)
    : Character(n, "PA", nation), Suspect(n, "PA", nation, l, p), BusinessRelations(n, "PA", nation, l, p), CloseRelations(n, "PA", nation, l, p) {
}
Child::Child(const string& n, const string& nation, char l, int p)
    : Character(n, "Child", nation), Suspect(n, "Child", nation, l, p), CloseRelations(n, "Child", nation, l, p) {
}
Victim::Victim() : Character("Business Owner", "Victim", "French") {}
void Victim::display() const { cout << characterName << " (" << characterRole << "), " << nationality; }
Detective::Detective(const string& n, int c) : Character(n, "Detective", "Player"), credibility(c), turns(0) {}
void Detective::display() const { cout << characterName << " (Detective) | Credibility: " << credibility << " | Turns: " << turns; }
int Detective::getCredibility() const { return credibility; }
int Detective::getTurns() const { return turns; }
void Detective::addTurn() { ++turns; }
void Detective::penalize(int a) { credibility -= a; if (credibility < 0) credibility = 0; }

void Cottage::addLocation(const Location& l) { locations.push(l); }
int Cottage::size() const { return locations.size(); }
Location& Cottage::at(int i) { return locations[i]; }
void Cottage::display() const { for (int i = 0;i < locations.size();++i) cout << i + 1 << ". " << locations[i].locName << (locations[i].locSearchFlag ? " (searched)" : "") << "\n"; }

void ScoreBoard::show() const {
    ifstream in("leader/scoreBoard.txt");
    if (!in) { cout << "No scoreboard records yet.\n"; return; }
    string line; cout << "\n--------- LEADER / SCORE BOARD ---------\n";
    while (getline(in, line)) cout << line << "\n";
}
void ScoreBoard::add(const string& d, const string& result, int c, int t, const string& culprit) {
    ofstream out("leader/scoreBoard.txt", ios::app);
    if (out) out << d << "|" << result << "|" << c << "|" << t << "|" << culprit << "|2026\n";
}

GameEngine* GameEngine::instance = 0;
GameEngine::GameEngine() : state(Menu), detective(0), culpritName(""), culpritRole(""), maxTurns(25) { srand((unsigned int)time(0)); loadData(); }
GameEngine::~GameEngine() { delete detective; }
GameEngine* GameEngine::getInstance() { if (!instance) instance = new GameEngine(); return instance; }
void GameEngine::destroyInstance() { delete instance; instance = 0; }

void GameEngine::loadData() {
    ifstream rules("rules.txt");
    string line;
    while (getline(rules, line)) {
        if (line.find("Max_Turns=") == 0) maxTurns = atoi(line.substr(10).c_str());
    }
    ifstream loc("locations.txt");
    if (loc) {
        getline(loc, line);
        while (getline(loc, line)) {
            string fields[8]; int f = 0; string part = "";
            for (unsigned int i = 0;i < line.size();++i) {
                if (line[i] == '|' && f < 7) { fields[f++] = part; part = ""; }
                else part += line[i];
            }
            fields[f] = part;
            if (f >= 2) cottage.addLocation(Location(fields[0], fields[1].empty() ? '?' : fields[1][0], f, Word1));
        }
    }
}
void GameEngine::pause() { cout << "\nPress Enter to continue..."; cin.ignore(numeric_limits<streamsize>::max(), '\n'); }

void GameEngine::newGame() {
    string name;
    cout << "\nEnter detective name (no spaces; use _ or .): ";
    cin >> name;
    if (name.empty()) throw GameException("Detective name cannot be empty.");
    delete detective; detective = new Detective(name, 100);
    ifstream clues("clue.txt");
    string header; getline(clues, header);
    string roles[7], w1[7], w2[7];
    int count = 0;
    while (count < 7 && getline(clues, header)) {
        int a = header.find('|'), b = header.find('|', a + 1);
        if (a < 0 || b < 0) continue;
        roles[count] = header.substr(0, a);
        w1[count] = header.substr(a + 1, b - a - 1);
        w2[count] = header.substr(b + 1);
        ++count;
    }
    if (count == 0) { roles[0] = "Servant"; w1[0] = "LOOTERS"; w2[0] = "HERITAGE"; count = 1; }
    int chosen = rand() % count;
    culpritRole = roles[chosen]; culpritName = "Unknown";
    scrabble.setWords(w1[chosen], w2[chosen]);
    state = Playing;
    play();
}
void GameEngine::play() {
    cout << "\nWelcome, Agent " << detective->getName() << ". Find the culprit!\n";
    while (state == Playing) {
        cout << "\nTurns left: " << (maxTurns - detective->getTurns()) << " | Credibility: " << detective->getCredibility() << "\n";
        cout << "1. Visit a location\n2. Interview a person\n3. View Scrabble board\n4. Form Word 1\n5. Form Word 2\n6. Accuse a suspect\n7. Quit to main menu\nChoice: ";
        int choice; if (!(cin >> choice)) { cin.clear(); cin.ignore(10000, '\n'); throw GameException("Invalid input."); }
        if (choice == 1) {
            cottage.display();
            if (cottage.size() == 0) { cout << "locations.txt is missing or empty.\n"; continue; }
            cout << "Choose location: "; int n; cin >> n;
            if (n<1 || n>cottage.size()) throw GameException("Invalid location number.");
            Location& l = cottage.at(n - 1);
            if (l.locSearchFlag) throw GameException("This location has already been searched.");
            l.locSearchFlag = true;
            scrabble.addClue(Word1, l.letterClueLoc);
            detective->addTurn();
            cout << "You found clue letter: " << l.letterClueLoc << "\n";
        }
        else if (choice == 2) {
            cout << "People: 1. Business Owner  2. Partner  3. PA  4. Cook  5. Guard  6. Servant  7. Child\n";
            cout << "Select person: "; int p; cin >> p;
            const char* people[] = { "","Business Owner","Partner","PA","Cook","Guard","Servant","Child" };
            if (p < 1 || p>7) throw GameException("Invalid person.");
            detective->addTurn();
            cout << "Interviewed: " << people[p] << ". They shared a clue about the case.\n";
            if (p == 1) { if (detective->getCredibility() < 80) cout << "The owner refuses: credibility must be at least 80.\n"; }
            else if (p == 2 && detective->getCredibility() < 50) cout << "Partner refuses: credibility must be at least 50.\n";
            else if (p == 3 && detective->getCredibility() < 35) cout << "PA refuses: credibility must be at least 35.\n";
            else if (p == 7 && detective->getCredibility() < 75) cout << "Child refuses: credibility must be at least 75.\n";
            else { cout << "You gained one credibility point.\n"; }
        }
        else if (choice == 3) scrabble.display();
        else if (choice == 4 || choice == 5) {
            WordRow row = (choice == 4 ? Word1 : Word2);
            cout << "Enter the complete word: "; string word; cin >> word;
            if (word == scrabble.getWord(row) && scrabble.formWord(row)) cout << "Correct! Word solved.\n";
            else { detective->penalize(10); detective->addTurn(); cout << "Incorrect word. Credibility -10.\n"; }
            if (scrabble.allSolved()) cout << "Both words are solved. You can now accuse!\n";
        }
        else if (choice == 6) accuse();
        else if (choice == 7) { state = Menu; }
        else cout << "Choose a valid option.\n";
        if (detective->getTurns() >= maxTurns || detective->getCredibility() <= 0) {
            state = Lost;
            cout << "\nCase failed. You ran out of turns or credibility.\n";
            scoreBoard.add(detective->getName(), "Lost", detective->getCredibility(), detective->getTurns(), culpritRole);
            state = Menu;
        }
    }
}
void GameEngine::accuse() {
    cout << "Enter suspect role (Partner / PA / Cook / Guard / Servant / Child): ";
    string role; cin >> role;
    detective->addTurn();
    if (role == culpritRole) {
        cout << "\n*** CASE SOLVED! You identified the culprit: " << culpritRole << " ***\n";
        scoreBoard.add(detective->getName(), "Won", detective->getCredibility(), detective->getTurns(), culpritRole);
        state = Won;
    }
    else {
        detective->penalize(5);
        cout << "Wrong accusation. Credibility -5.\n";
        state = Playing;
    }
    if (state == Won) state = Menu;
}
void GameEngine::run() {
    while (state != Exit) {
        cout << "\n========================================\n";
        cout << "          THE LOCKER HEIST\n";
        cout << "========================================\n";
        cout << "1. New Game\n2. Leader / Score Board\n3. About\n4. Rules\n5. Exit\nChoice: ";
        int choice;
        if (!(cin >> choice)) { cin.clear(); cin.ignore(10000, '\n'); cout << "Invalid input.\n"; continue; }
        try {
            switch (choice) {
            case NewGame: newGame(); break;
            case LeaderBoard: scoreBoard.show(); break;
            case About: {
                ifstream in("about.txt"); string line;
                while (getline(in, line)) cout << line << "\n";
                if (!in) cout << "Locker Heist: a detective mystery game.\n";
                break;
            }
            case Rules: {
                ifstream in("rules.txt"); string line;
                while (getline(in, line)) cout << line << "\n";
                if (!in) cout << "Default: 25 turns, 100 starting credibility.\n";
                break;
            }
            case Quit: state = Exit; break;
            default: cout << "Please choose 1 to 5.\n";
            }
        }
        catch (const GameException& e) { cout << e.what() << "\n"; }
    }
    cout << "\nThank you for playing Locker Heist!\n";
}
