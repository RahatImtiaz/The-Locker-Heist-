// ============================================================================
// heist.cpp - THE LOCKER HEIST : implementation of every class in heist.h
// ============================================================================
#define _CRT_SECURE_NO_WARNINGS   // lets Visual Studio accept localtime()
#include "heist.h"
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cctype>

// ============================================================================
// UTILITY HELPERS
// ============================================================================

// Removes leading and trailing white space (also strips a stray '\r')
string trimText(const string & s) {
    size_t b = 0, e = s.size();
    while (b < e && isspace((unsigned char)s[b])) b++;
    while (e > b && isspace((unsigned char)s[e - 1])) e--;
    return s.substr(b, e - b);
}

// Converts a text to capital letters
string toUpperText(const string & s) {
    string out = s;
    for (size_t i = 0; i < out.size(); i++) out[i] = (char)toupper((unsigned char)out[i]);
    return out;
}

// Converts a non-negative or negative integer to text (no streams needed)
string intToText(int value) {
    if (value == 0) return "0";
    bool negative = value < 0;
    long long val = value;
    if (negative) val = -val;
    string out = "";
    while (val > 0) { out = (char)('0' + val % 10) + out; val /= 10; }
    return negative ? "-" + out : out;
}

// Safely converts text such as "-5" or "25" to int; returns false if invalid
bool parseInt(const string& text, int& result) {
    string t = trimText(text);
    if (t.empty() || t.size() > 9) return false;
    size_t i = 0;
    bool negative = false;
    if (t[0] == '-' || t[0] == '+') { negative = (t[0] == '-'); i = 1; }
    if (i >= t.size()) return false;
    int value = 0;
    for (; i < t.size(); i++) {
        if (!isdigit((unsigned char)t[i])) return false;
        value = value * 10 + (t[i] - '0');
    }
    result = negative ? -value : value;
    return true;
}

// Splits "a|b|c" into parts[]; a leading/trailing '|' is ignored.
// Returns the real number of fields (only maxParts are stored).
int splitLine(const string& line, char delim, string parts[], int maxParts) {
    string work = trimText(line);
    if (!work.empty() && work[0] == delim) work = work.substr(1);
    if (!work.empty() && work[work.size() - 1] == delim) work = work.substr(0, work.size() - 1);
    int count = 0;
    string current = "";
    for (size_t i = 0; i <= work.size(); i++) {
        if (i == work.size() || work[i] == delim) {
            if (count < maxParts) parts[count] = trimText(current);
            count++;
            current = "";
        } else {
            current += work[i];
        }
    }
    return count;
}

// Returns "Word 1" or "Word 2"
string rowLabel(WordRow row) { return row == WordRow::Word1 ? "Word 1" : "Word 2"; }

// Local helper: replaces '_' with ' ' for nicer printing (Bedroom_1 -> Bedroom 1)
static string prettyName(const string& s) {
    string out = s;
    for (size_t i = 0; i < out.size(); i++) if (out[i] == '_') out[i] = ' ';
    return out;
}

// Local helper: pads text with spaces to a fixed width (for tables)
static string padRight(const string& s, int width) {
    string out = s;
    while ((int)out.size() < width) out += ' ';
    return out;
}

// Local helper: reads a data file, skips the header line and blank lines.
// Throws FileException if the file is missing or has too many lines.
static int readDataLines(const string& path, string lines[], int maxLines) {
    ifstream in(path.c_str());
    if (!in) throw FileException("File not found: " + path);
    string line;
    bool headerSkipped = false;
    int count = 0;
    while (getline(in, line)) {
        line = trimText(line);
        if (line.empty()) continue;
        if (!headerSkipped) { headerSkipped = true; continue; }
        if (count >= maxLines) throw FileException(path + " is corrupted (too many lines).");
        lines[count++] = line;
    }
    return count;
}

// ============================================================================
// STRUCT OPERATORS (Module 11)
// ============================================================================

// Two tiles are equal when their letters are equal
bool operator==(const Tile& a, const Tile& b) { return a.tileLetter == b.tileLetter; }

// Two clues are equal when letter, position and row are all equal
bool operator==(const Clue& a, const Clue& b) {
    return a.clueLetter == b.clueLetter && a.cluePosition == b.cluePosition && a.clueRow == b.clueRow;
}

// Prints a tile as [A] or [ ]
ostream& operator<<(ostream& os, const Tile& t) {
    os << "[" << (t.filled ? t.tileLetter : ' ') << "]";
    return os;
}

// Reads one letter into a tile
istream& operator>>(istream& is, Tile& t) {
    char c;
    if (is >> c) { t.tileLetter = (char)toupper((unsigned char)c); t.filled = true; }
    return is;
}

// Prints a location with its search status
ostream& operator<<(ostream& os, const Location& loc) {
    os << prettyName(loc.locName) << (loc.locSearchFlag ? " -- Searched" : " -- NOT Searched");
    return os;
}

// Reads a location name from a stream
istream& operator>>(istream& is, Location& loc) {
    is >> loc.locName;
    return is;
}

// ============================================================================
// TileRack
// ============================================================================

// Constructor: creates an empty rack with a limit
TileRack::TileRack(int limit) : tiles(limit), positions(limit), maxTiles(limit) {
    // earlier raw DMA: tiles = new Tile[maxTiles];
}

// Copy constructor: DynamicArray members make a deep copy
TileRack::TileRack(const TileRack& other) : tiles(other.tiles), positions(other.positions), maxTiles(other.maxTiles) {}

// Assignment operator: deep copy with self-assignment check
TileRack& TileRack::operator=(const TileRack& other) {
    if (this != &other) {
        tiles = other.tiles;
        positions = other.positions;
        maxTiles = other.maxTiles;
    }
    return *this;
}

// Destructor: DynamicArray frees its own memory
TileRack::~TileRack() {
    // earlier raw DMA: delete[] tiles;
}

// Stores a letter with its board position; throws if the rack is full
void TileRack::addTile(char letter, int position) {
    if (isFull()) throw RackFullException("The rack is full. It can hold only " + intToText(maxTiles) + " tiles.");
    tiles.push(Tile(letter, true));
    positions.push(position);
}

// Removes the first tile having the given letter
void TileRack::removeTile(char letter) {
    if (isEmpty()) throw RackEmptyException("The rack is empty. Nothing to remove.");
    for (int i = 0; i < tiles.size(); i++) {
        if (tiles[i] == Tile(letter, true)) {
            tiles.removeAt(i);
            positions.removeAt(i);
            return;
        }
    }
    throw WrongLetterException(string("Letter ") + letter + " is not in the rack.");
}

// Number of letters stored
int TileRack::getCount() const { return tiles.size(); }

// True when no more tile fits
bool TileRack::isFull() const { return tiles.size() >= maxTiles; }

// True when no tile is stored
bool TileRack::isEmpty() const { return tiles.size() == 0; }

// Removes every tile
void TileRack::clear() { tiles.clear(); positions.clear(); }

// Prints letters with their positions, e.g. A(3) P(1)
void TileRack::display() const {
    if (isEmpty()) { cout << "(empty)"; return; }
    for (int i = 0; i < tiles.size(); i++) {
        if (tiles[i].tileLetter == ' ') cout << " |  ";
        else cout << tiles[i].tileLetter << "(" << positions[i] << ") ";
    }
}

// operator+ merges two racks with a space tile between them (one big clue)
TileRack TileRack::operator+(const TileRack& other) const {
    TileRack merged(maxTiles + other.maxTiles + 1);
    for (int i = 0; i < tiles.size(); i++) merged.addTile(tiles[i].tileLetter, positions[i]);
    merged.addTile(' ', 0);
    for (int i = 0; i < other.tiles.size(); i++) merged.addTile(other.tiles[i].tileLetter, other.positions[i]);
    return merged;
}

// ============================================================================
// Scrabble
// ============================================================================

// Constructor: two empty rows of the right size
Scrabble::Scrabble() : row1(WORD1_LEN), row2(WORD2_LEN), word1(""), word2(""), solved1(false), solved2(false) {
    for (int i = 0; i < WORD1_LEN; i++) row1.push(Tile());
    for (int i = 0; i < WORD2_LEN; i++) row2.push(Tile());
    // earlier raw DMA: grid[0] = new Tile[7]; grid[1] = new Tile[8];
}

// Copy constructor: deep copy
Scrabble::Scrabble(const Scrabble& other)
    : row1(other.row1), row2(other.row2), word1(other.word1), word2(other.word2),
      solved1(other.solved1), solved2(other.solved2) {}

// Assignment operator: deep copy
Scrabble& Scrabble::operator=(const Scrabble& other) {
    if (this != &other) {
        row1 = other.row1; row2 = other.row2;
        word1 = other.word1; word2 = other.word2;
        solved1 = other.solved1; solved2 = other.solved2;
    }
    return *this;
}

// Destructor: members free themselves
Scrabble::~Scrabble() {}

// Returns the row object for Word 1 or Word 2
DynamicArray<Tile>& Scrabble::rowOf(WordRow r) { return (r == WordRow::Word1) ? row1 : row2; }
const DynamicArray<Tile>& Scrabble::rowOf(WordRow r) const { return (r == WordRow::Word1) ? row1 : row2; }

// Returns the hidden target word of a row
const string& Scrabble::targetOf(WordRow r) const { return (r == WordRow::Word1) ? word1 : word2; }

// Stores the target words and empties both rows
void Scrabble::loadWords(const string& w1, const string& w2) {
    if ((int)w1.size() != WORD1_LEN) throw InvalidWordException("Word 1 must have exactly 7 letters.");
    if ((int)w2.size() != WORD2_LEN) throw InvalidWordException("Word 2 must have exactly 8 letters.");
    word1 = w1; word2 = w2;
    solved1 = solved2 = false;
    row1.clear(); row2.clear();
    for (int i = 0; i < WORD1_LEN; i++) row1.push(Tile());
    for (int i = 0; i < WORD2_LEN; i++) row2.push(Tile());
}

// Places and reveals a letter; checks position, filled slot and wrong letter
void Scrabble::placeLetter(const Clue& clue) {
    DynamicArray<Tile>& row = rowOf(clue.clueRow);
    int len = row.size();
    if (clue.cluePosition < 1 || clue.cluePosition > len)
        throw InvalidPositionException("Position " + intToText(clue.cluePosition) + " does not exist in " + rowLabel(clue.clueRow) + ".");
    Tile& slot = row[clue.cluePosition - 1];
    if (slot.filled)
        throw SlotFilledException("Position " + intToText(clue.cluePosition) + " of " + rowLabel(clue.clueRow) + " is already filled.");
    Tile expected(targetOf(clue.clueRow)[clue.cluePosition - 1], true);
    Tile given(clue.clueLetter, true);
    if (!(expected == given))
        throw WrongLetterException(string("Letter ") + clue.clueLetter + " does not fit position " + intToText(clue.cluePosition) + ".");
    slot = given;
}

// True when every slot of the row is filled
bool Scrabble::isRowComplete(WordRow r) const {
    const DynamicArray<Tile>& row = rowOf(r);
    for (int i = 0; i < row.size(); i++) if (!row[i].filled) return false;
    return true;
}

// True when the detective already typed this word correctly
bool Scrabble::isRowSolved(WordRow r) const { return r == WordRow::Word1 ? solved1 : solved2; }

// True when both words are solved
bool Scrabble::bothSolved() const { return solved1 && solved2; }

// Compares the typed word with the hidden target letter by letter
bool Scrabble::checkWord(WordRow r, const string& typed) const {
    const string& target = targetOf(r);
    if (typed.size() != target.size()) return false;
    for (size_t i = 0; i < target.size(); i++)
        if (!(Tile(target[i], true) == Tile(typed[i], true))) return false;
    return true;
}

// Marks a row as solved
void Scrabble::markSolved(WordRow r) { if (r == WordRow::Word1) solved1 = true; else solved2 = true; }

// Returns the hidden target word
string Scrabble::getTarget(WordRow r) const { return targetOf(r); }

// Prints one row such as:  Word 1: [A] [ ] [ ]
void Scrabble::displayRow(ostream& os, WordRow r) const {
    const DynamicArray<Tile>& row = rowOf(r);
    os << "  " << rowLabel(r) << ": ";
    for (int i = 0; i < row.size(); i++) os << row[i] << " ";
    if (isRowSolved(r)) os << " -> SOLVED";
    else if (isRowComplete(r)) os << " -> COMPLETE";
}

// Prints the whole board
ostream& operator<<(ostream& os, const Scrabble& s) {
    os << "Scrabble:\n";
    s.displayRow(os, WordRow::Word1); os << "\n";
    s.displayRow(os, WordRow::Word2); os << "\n";
    return os;
}

// ============================================================================
// Cottage
// ============================================================================

// Constructor: empty cache that can hold the 7 locations
Cottage::Cottage() : locCache(LOCATION_COUNT) {
    // earlier raw DMA: locCache = new Location[7];
}

// Copy constructor: deep copy
Cottage::Cottage(const Cottage& other) : locCache(other.locCache) {}

// Assignment operator: deep copy
Cottage& Cottage::operator=(const Cottage& other) {
    if (this != &other) locCache = other.locCache;
    return *this;
}

// Destructor: DynamicArray deletes the locations and the array itself
Cottage::~Cottage() {
    // earlier raw DMA: delete[] locCache;
}

// Adds a location (always belongs to Word 1)
void Cottage::addLocation(const string& name, char letter, int position) {
    Location loc;
    loc.locName = name; loc.letterClueLoc = letter; loc.boardPosLoc = position;
    loc.locRow = WordRow::Word1; loc.locSearchFlag = false;
    locCache.push(loc);
}

// Number of locations stored
int Cottage::getLocationCount() const { return locCache.size(); }

// True if the location was searched earlier
bool Cottage::isSearched(int index) const { return locCache[index].locSearchFlag; }

// Searches the location: sets the flag and returns the clue
Clue Cottage::searchLocation(int index) {
    Location& loc = locCache[index];
    loc.locSearchFlag = true;
    return Clue(loc.letterClueLoc, loc.boardPosLoc, loc.locRow);
}

// Returns the clue again without changing anything
Clue Cottage::peekClue(int index) const {
    const Location& loc = locCache[index];
    return Clue(loc.letterClueLoc, loc.boardPosLoc, loc.locRow);
}

// Name of a location
string Cottage::getLocationName(int index) const { return locCache[index].locName; }

// Prints the numbered location menu with search status
void Cottage::displayLocations() const {
    cout << "\n====== Location Menu ======\n";
    for (int i = 0; i < locCache.size(); i++) cout << "[" << (i + 1) << "] " << locCache[i] << "\n";
    cout << "[0] Back\n==========================\n";
}

// Removes all locations (used when a new game starts)
void Cottage::reset() { locCache.clear(); }

// ============================================================================
// CHARACTER HIERARCHY
// ============================================================================

// Character constructor
Character::Character(const string& name, const string& role, const string& nation)
    : characterName(name), characterRole(role), nationality(nation), isCulprit(false), interviewed(false) {}

// Virtual destructor (so derived objects are destroyed correctly via Character*)
Character::~Character() {}

// Default clue: nothing (Detective gives no clue)
Clue Character::giveClue() const { return Clue('-', 0, WordRow::Word2); }

// Default interview requirement: none
int Character::minCredibility() const { return 0; }

// Only Suspect overrides this to return true
bool Character::isSuspect() const { return false; }

string Character::getName() const { return characterName; }
string Character::getRole() const { return characterRole; }
string Character::getNationality() const { return nationality; }
bool   Character::getIsCulprit() const { return isCulprit; }
bool   Character::isInterviewed() const { return interviewed; }

// Insertion operator calls the polymorphic display()
ostream& operator<<(ostream& os, const Character& c) { c.display(os); return os; }

// ---- Victim ----
Victim::Victim(const string& name, const string& nation, char letter, int position)
    : Character(name, "Victim", nation), clueRowChar(WordRow::Word2), letterClueChar(letter), boardPosChar(position) {}
void Victim::display(ostream& os) const {
    os << characterName << " (Business Owner / Victim, " << nationality << ")\n"
       << "  \"My locker was hidden behind the bookshelf... only a tilted book opens it!\"";
}
Clue Victim::giveClue() const { return Clue(letterClueChar, boardPosChar, clueRowChar); }
int  Victim::minCredibility() const { return 80; }

// ---- Detective ----
Detective::Detective(const string& name, int startCredibility)
    : Character(name, "Detective", "Malay"), credibility(startCredibility), turns(0),
      rack1(Scrabble::WORD1_LEN), rack2(Scrabble::WORD2_LEN) {}
void Detective::display(ostream& os) const {
    os << characterName << " (Detective) | Credibility: " << credibility << " | Turns: " << turns;
}
void Detective::loseCredibility(int amount) { credibility -= amount; if (credibility < 0) credibility = 0; }
void Detective::useTurn() { turns++; }
int  Detective::getCredibility() const { return credibility; }
int  Detective::getTurns() const { return turns; }
TileRack& Detective::getRack1() { return rack1; }
TileRack& Detective::getRack2() { return rack2; }

// ---- Suspect and its middle classes ----
Suspect::Suspect(const string& name, const string& role, const string& nation, char letter, int position)
    : Character(name, role, nation), clueRowChar(WordRow::Word2), letterClueChar(letter), boardPosChar(position) {}
Clue Suspect::giveClue() const { return Clue(letterClueChar, boardPosChar, clueRowChar); }
bool Suspect::isSuspect() const { return true; }

BusinessRelations::BusinessRelations(const string& name, const string& role, const string& nation, char letter, int position)
    : Suspect(name, role, nation, letter, position) {}
HouseHelp::HouseHelp(const string& name, const string& role, const string& nation, char letter, int position)
    : Suspect(name, role, nation, letter, position) {}
CloseRelations::CloseRelations(const string& name, const string& role, const string& nation, char letter, int position)
    : Suspect(name, role, nation, letter, position) {}

// ---- Leaf suspects ----
// Character (and Suspect for Partner/PA/Child) are VIRTUAL bases, so the most
// derived class must construct them directly.
Partner::Partner(const string& name, const string& role, const string& nation, char letter, int position)
    : Character(name, role, nation),
      Suspect(name, role, nation, letter, position),
      BusinessRelations(name, role, nation, letter, position) {}
void Partner::display(ostream& os) const {
    os << characterName << " (" << characterRole << ", " << nationality << ")\n"
       << "  \"The owner and I argued about money. But I never touched that locker!\"";
}
int Partner::minCredibility() const { return 50; }

PA::PA(const string& name, const string& nation, char letter, int position)
    : Character(name, "PA", nation),
      Suspect(name, "PA", nation, letter, position),
      BusinessRelations(name, "PA", nation, letter, position),
      CloseRelations(name, "PA", nation, letter, position) {}
void PA::display(ostream& os) const {
    os << characterName << " (PA, " << nationality << ")\n"
       << "  \"I know all of the boss's secrets, detective... and I hired you, remember?\"";
}
int PA::minCredibility() const { return 35; }

Cook::Cook(const string& name, const string& nation, char letter, int position)
    : Character(name, "Cook", nation),
      HouseHelp(name, "Cook", nation, letter, position) {}
void Cook::display(ostream& os) const {
    os << characterName << " (Cook, " << nationality << ")\n"
       << "  \"I was in the kitchen all night, the sauce needed me.\"";
}

Guard::Guard(const string& name, const string& nation, char letter, int position)
    : Character(name, "Guard", nation),
      HouseHelp(name, "Guard", nation, letter, position) {}
void Guard::display(ostream& os) const {
    os << characterName << " (Guard, " << nationality << ")\n"
       << "  \"I watched the gate. Nobody came in or out... or so I thought.\"";
}

Servant::Servant(const string& name, const string& nation, char letter, int position)
    : Character(name, "Servant", nation),
      HouseHelp(name, "Servant", nation, letter, position) {}
void Servant::display(ostream& os) const {
    os << characterName << " (Servant, " << nationality << ")\n"
       << "  \"I only served drinks, sir. I saw nothing, I swear.\"";
}

Child::Child(const string& name, const string& nation, char letter, int position)
    : Character(name, "Child", nation),
      Suspect(name, "Child", nation, letter, position),
      CloseRelations(name, "Child", nation, letter, position) {}
void Child::display(ostream& os) const {
    os << characterName << " (Owner's Child, " << nationality << ")\n"
       << "  \"I'm scared... I heard footsteps upstairs during the party!\"";
}
int Child::minCredibility() const { return 75; }

// ============================================================================
// ScoreBoard
// ============================================================================

// Stores the path of the score file
ScoreBoard::ScoreBoard(const string& path) : filePath(path) {}

// Today's date as dd-mm-yy
string ScoreBoard::todayDate() {
    time_t now = time(0);
    tm* local = localtime(&now);
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%d-%m-%y", local);
    return string(buffer);
}

// Appends one result line to the score file (creates header if file is new)
void ScoreBoard::addScore(const string& detectiveName, bool won, int credibility, int turnsLeft, const string& culprit) {
    ifstream probe(filePath.c_str());
    bool exists = probe.good();
    probe.close();
    ofstream out(filePath.c_str(), ios::app);
    if (!out) throw FileException("Cannot write to " + filePath + " (does the 'leader' folder exist?)");
    if (!exists) out << "Detective|Result|Credibility|Turns_Left|Culprit|Date\n";
    string left = (turnsLeft < 10 ? "0" : "") + intToText(turnsLeft);
    string culpritText = culprit;
    for (size_t i = 0; i < culpritText.size(); i++) if (culpritText[i] == ' ') culpritText[i] = '_';
    out << detectiveName << "|" << (won ? "WIN" : "LOSE") << "|" << credibility << "|" << left << "|"
        << culpritText << "|" << todayDate() << "\n";
}

// Reads the file and prints all previous scores as a table
void ScoreBoard::displayAll() const {
    ifstream in(filePath.c_str());
    if (!in) throw FileException("Score file not found: " + filePath);
    string line;
    bool headerSeen = false;
    int rowNo = 0;
    cout << "\n";
    while (getline(in, line)) {
        line = trimText(line);
        if (line.empty()) continue;
        string p[6];
        if (splitLine(line, '|', p, 6) != 6) throw FileException(filePath + " is corrupted.");
        if (!headerSeen) {
            headerSeen = true;
            cout << padRight("#", 4) << padRight("Detective", 18) << padRight("Result", 8) << padRight("Cred.", 7)
                 << padRight("TurnsLeft", 11) << padRight("Culprit", 14) << "Date\n";
            cout << "--------------------------------------------------------------------\n";
            continue;
        }
        rowNo++;
        cout << padRight(intToText(rowNo), 4) << padRight(p[0], 18) << padRight(p[1], 8) << padRight(p[2], 7)
             << padRight(p[3], 11) << padRight(p[4], 14) << p[5] << "\n";
    }
    if (!headerSeen) throw FileException(filePath + " is empty or corrupted.");
    if (rowNo == 0) cout << "(no games played yet)\n";
}

// ============================================================================
// GameEngine (Singleton)
// ============================================================================
GameEngine* GameEngine::instance = 0;

// Private constructor: sets default rules and empty character list
GameEngine::GameEngine()
    : detective(), cottage(), scrabble(), mergedRack(17), peopleCount(0), scoreBoard(0),
      state(GameState::Menu), titleShown(false), maxTurns(25), startCredibility(100),
      wrongTurnPenalty(5), wrongWordPenalty(10),
      winCondition("Accuse Correct Culprit"), loseCondition("Credibility Zero Or Turns Exhausted"), culpritRole("") {
    for (int i = 0; i < MAX_PEOPLE; i++) people[i] = 0;
}

// Destructor: frees all characters (virtual destructors do the rest)
GameEngine::~GameEngine() { clearPeople(); }

// Singleton access: creates the engine on first call
GameEngine* GameEngine::getInstance() {
    if (!instance) instance = new GameEngine();
    return instance;
}

// Deletes the singleton
void GameEngine::destroyInstance() { delete instance; instance = 0; }

// Association: the engine only uses the score board, it does not own it
void GameEngine::attachScoreBoard(ScoreBoard* board) { scoreBoard = board; }

// Deletes every Character object from the heap
void GameEngine::clearPeople() {
    for (int i = 0; i < MAX_PEOPLE; i++) { delete people[i]; people[i] = 0; }
    peopleCount = 0;
}

// ---------------------------------------------------------------- I/O helpers

// Reads one line; throws InputClosedException when input ends (Ctrl+Z / Ctrl+D)
string GameEngine::readLine() {
    string s;
    if (!getline(cin, s)) throw InputClosedException();
    return trimText(s);
}

// Prints a prompt and reads a number between low and high
int GameEngine::readInt(const string& prompt, int low, int high) {
    cout << prompt;
    string s = readLine();
    int v;
    if (!parseInt(s, v)) throw InvalidInputException("\"" + s + "\" is not a number. Please type a number.");
    if (v < low || v > high)
        throw InvalidInputException("Choice must be between " + intToText(low) + " and " + intToText(high) + ".");
    return v;
}

// Prints a prompt and reads Y or N
bool GameEngine::readYesNo(const string& prompt) {
    cout << prompt;
    string s = toUpperText(readLine());
    if (s == "Y" || s == "YES") return true;
    if (s == "N" || s == "NO") return false;
    throw InvalidInputException("Please answer with Y or N.");
}

// Waits for Enter
void GameEngine::pauseScreen() {
    cout << "\nPress Enter to continue...";
    readLine();
}

// Clears the console screen (cls on Windows, clear elsewhere)
void GameEngine::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    cout << "\n\n";   // keeps output readable when piped; real terminals scroll
    system("clear > /dev/null 2>&1");
#endif
}

// Prints a fancy ASCII box with one or two text lines
void GameEngine::printBox(const string& line1, const string& line2) {
    const int W = 62;
    string bar((size_t)W, '=');
    cout << "\n+" << bar << "+\n";
    int pad = W - (int)line1.size(); if (pad < 0) pad = 0;
    cout << "|" << string((size_t)(pad / 2), ' ') << line1 << string((size_t)(pad - pad / 2), ' ') << "|\n";
    if (!line2.empty()) {
        pad = W - (int)line2.size(); if (pad < 0) pad = 0;
        cout << "|" << string((size_t)(pad / 2), ' ') << line2 << string((size_t)(pad - pad / 2), ' ') << "|\n";
    }
    cout << "+" << bar << "+\n";
}

// Title screen shown once at startup
void GameEngine::showTitle() {
    cout << "\n  ===========================================================\n";
    cout << "      T H E   L O C K E R   H E I S T\n";
    cout << "      A Detective Game backed by Principles of OOP\n";
    cout << "  ===========================================================\n";
    cout << "  A locker hidden behind a bookshelf in a wealthy owner's cottage\n";
    cout << "  was robbed during a party. Eight people were present and one of\n";
    cout << "  them is the culprit. Find the letters, solve two words, and\n";
    cout << "  accuse the right suspect before your credibility runs out!\n";
}

// Prints the main menu
void GameEngine::showMainMenu() {
    printBox("THE LOCKER HEIST - MAIN MENU");
    cout << "   [1] New Game\n   [2] View Leader/Score Board\n   [3] About\n   [4] Rules\n   [5] Exit\n";
    cout << "--------------------------------------------------------------\n";
}

// ---------------------------------------------------------------- main loop

// Runs (or resumes after an exception) the game until the player exits
void GameEngine::run() {
    if (!titleShown) { showTitle(); titleShown = true; }
    while (state != GameState::Exit) {
        if (state == GameState::Menu) {
            showMainMenu();
            int c = readInt("Choice: ", 1, 5);
            handleMainChoice(static_cast<MenuChoice>(c));
        } else if (state == GameState::Playing) {
            checkEndConditions();
            if (state == GameState::Playing) playTurn();
        } else {
            state = GameState::Menu;   // Won/Lost leftovers (e.g. after a file error) -> back to menu
        }
    }
}

// Main menu dispatcher
void GameEngine::handleMainChoice(MenuChoice choice) {
    switch (choice) {
        case MenuChoice::NewGame:     startNewGame(); break;
        case MenuChoice::LeaderBoard: showScoreBoard(); break;
        case MenuChoice::About:       showAbout(); break;
        case MenuChoice::Rules:       showRules(); break;
        case MenuChoice::Exit:        state = GameState::Exit; cout << "\nGoodbye, Detective!\n"; break;
    }
}

// Shows the leader/score board
void GameEngine::showScoreBoard() {
    if (!scoreBoard) throw FileException("Score board is not attached.");
    printBox("LEADER / SCORE BOARD");
    scoreBoard->displayAll();
    pauseScreen();
}

// Prints about.txt
void GameEngine::showAbout() {
    ifstream in("about.txt");
    if (!in) throw FileException("File not found: about.txt");
    string line;
    cout << "\n";
    while (getline(in, line)) cout << line << "\n";
    pauseScreen();
}

// Loads rules.txt (falls back to defaults if missing or corrupt)
void GameEngine::loadRules() {
    int tMax = 25, tCred = 100, tTurn = 5, tWord = 10;
    string tWin = "Accuse Correct Culprit", tLose = "Credibility Zero Or Turns Exhausted";
    bool gotMax = false, gotCred = false, gotTurn = false, gotWord = false, corrupt = false;
    ifstream in("rules.txt");
    if (!in) {
        cout << "[rules.txt not found - using default rules]\n";
    } else {
        string line;
        while (getline(in, line) && !corrupt) {
            line = trimText(line);
            if (line.empty()) continue;
            size_t eq = line.find('=');
            if (eq == string::npos) { corrupt = true; break; }
            string key = trimText(line.substr(0, eq)), val = trimText(line.substr(eq + 1));
            int v = 0;
            if (key == "Max_Turns")                { corrupt = !(parseInt(val, v) && v > 0 && v <= 200); tMax = v; gotMax = true; }
            else if (key == "Starting_Credibility"){ corrupt = !(parseInt(val, v) && v > 0 && v <= 1000); tCred = v; gotCred = true; }
            else if (key == "Every_Wrong_Turn")    { corrupt = !parseInt(val, v); tTurn = v < 0 ? -v : v; gotTurn = true; }
            else if (key == "Every_Wrong_Word")    { corrupt = !parseInt(val, v); tWord = v < 0 ? -v : v; gotWord = true; }
            else if (key == "Win_Condition")       tWin = prettyName(val);
            else if (key == "Lose_Condition")      tLose = prettyName(val);
        }
        if (corrupt || !gotMax || !gotCred || !gotTurn || !gotWord) {
            cout << "[rules.txt is corrupt - using default rules]\n";
            tMax = 25; tCred = 100; tTurn = 5; tWord = 10;
            tWin = "Accuse Correct Culprit"; tLose = "Credibility Zero Or Turns Exhausted";
        }
    }
    maxTurns = tMax; startCredibility = tCred; wrongTurnPenalty = tTurn; wrongWordPenalty = tWord;
    winCondition = tWin; loseCondition = tLose;
}

// Shows the rules (read from rules.txt)
void GameEngine::showRules() {
    loadRules();
    printBox("RULES OF THE LOCKER HEIST");
    cout << "  Max turns            : " << maxTurns << "\n";
    cout << "  Starting credibility : " << startCredibility << "\n";
    cout << "  Every wrong turn     : -" << wrongTurnPenalty << " credibility\n";
    cout << "  Every wrong word     : -" << wrongWordPenalty << " credibility\n";
    cout << "  Win condition        : " << winCondition << "\n";
    cout << "  Lose condition       : " << loseCondition << "\n\n";
    cout << "  Interview limits: Owner needs credibility >= 80, Child >= 75,\n";
    cout << "  Partners >= 50, PA >= 35, House Help (Cook/Guard/Servant) anytime.\n";
    cout << "  Viewing the Scrabble board is free (no turn used).\n";
    pauseScreen();
}

// ---------------------------------------------------------------- new game

// Asks for the detective name; spaces can become '_' or '.'
string GameEngine::askDetectiveName() {
    cout << "Enter detective name: ";
    string name = readLine();
    if (name.empty()) throw InvalidInputException("Detective name cannot be empty.");
    if (name.find(' ') != string::npos) {
        cout << "Name contains spaces. Replace spaces with underscore (_) or dot (.)? Type _ or . : ";
        string ch = readLine();
        if (ch != "_" && ch != ".") throw InvalidInputException("Please type only _ or . to replace spaces.");
        for (size_t i = 0; i < name.size(); i++) if (name[i] == ' ') name[i] = ch[0];
    }
    return name;
}

// Starts a fresh game: new Detective, Cottage, Scrabble, random culprit
void GameEngine::startNewGame() {
    printBox("THE LOCKER HEIST - NEW GAME");
    string name = askDetectiveName();
    loadRules();
    loadGameData();                               // throws FileException on problems
    detective = Detective(name, startCredibility);
    mergedRack.clear();
    state = GameState::Playing;
    printBox(name + ", a culprit has been secretly chosen!", "The game begins!");
    cout << "  Max turns: " << maxTurns << " | Starting credibility: " << startCredibility
         << " | Wrong turn: -" << wrongTurnPenalty << " | Wrong word: -" << wrongWordPenalty << "\n";
    cout << "  Win: " << winCondition << " | Lose: " << loseCondition << "\n";
}

// Reads clue.txt, picks the culprit, then loads locations, characters and words
void GameEngine::loadGameData() {
    clearPeople();
    cottage.reset();

    string lines[16];
    int n = readDataLines("clue.txt", lines, 16);
    if (n != 7) throw FileException("clue.txt is corrupted (it must contain 7 culprit profiles).");
    string roles[7], w1[7], w2[7];
    for (int i = 0; i < 7; i++) {
        string p[3];
        if (splitLine(lines[i], '|', p, 3) != 3) throw FileException("clue.txt is corrupted at line " + intToText(i + 2) + ".");
        roles[i] = p[0]; w1[i] = toUpperText(p[1]); w2[i] = toUpperText(p[2]);
        if ((int)w1[i].size() != Scrabble::WORD1_LEN || (int)w2[i].size() != Scrabble::WORD2_LEN)
            throw FileException("clue.txt has a word with a wrong length at line " + intToText(i + 2) + ".");
    }
    int culprit = rand() % 7;                     // random culprit profile
    culpritRole = roles[culprit];

    loadLocations(culprit, w1[culprit]);
    try { loadCharacters(culprit, w2[culprit]); }
    catch (...) { clearPeople(); throw; }         // never leak half-built characters
    scrabble.loadWords(w1[culprit], w2[culprit]);

    // mark the culprit (GameEngine is a friend of Character)
    bool found = false;
    for (int i = 0; i < peopleCount; i++)
        if (people[i]->isSuspect() && people[i]->characterRole == culpritRole) { people[i]->isCulprit = true; found = true; }
    if (!found) { clearPeople(); throw FileException("characters.txt has no suspect with role " + culpritRole + "."); }
}

// Reads locations.txt: column (2 + columnIndex) holds the letters for this culprit
void GameEngine::loadLocations(int columnIndex, const string& word1) {
    string lines[16];
    int n = readDataLines("locations.txt", lines, 16);
    if (n != Cottage::LOCATION_COUNT) throw FileException("locations.txt is corrupted (it must contain 7 locations).");
    bool used[8] = {false, false, false, false, false, false, false, false};
    for (int i = 0; i < n; i++) {
        string p[9];
        if (splitLine(lines[i], '|', p, 9) != 9) throw FileException("locations.txt is corrupted at line " + intToText(i + 2) + ".");
        int pos;
        if (!parseInt(p[1], pos) || pos < 1 || pos > 7 || used[pos]) throw FileException("locations.txt has a bad W1Pos at line " + intToText(i + 2) + ".");
        used[pos] = true;
        string letter = toUpperText(p[2 + columnIndex]);
        if (letter.size() != 1 || letter[0] != word1[pos - 1]) throw FileException("locations.txt letters do not match clue.txt at line " + intToText(i + 2) + ".");
        cottage.addLocation(p[0], letter[0], pos);
    }
}

// Reads characters.txt: column (4 + columnIndex) holds the letters for this culprit
void GameEngine::loadCharacters(int columnIndex, const string& word2) {
    string lines[16];
    int n = readDataLines("characters.txt", lines, 16);
    if (n != MAX_PEOPLE) throw FileException("characters.txt is corrupted (it must contain 8 characters).");
    bool used[9] = {false, false, false, false, false, false, false, false, false};
    for (int i = 0; i < n; i++) {
        string p[12];
        if (splitLine(lines[i], '|', p, 12) != 12) throw FileException("characters.txt is corrupted at line " + intToText(i + 2) + ".");
        int pos;
        if (!parseInt(p[3], pos) || pos < 1 || pos > 8 || used[pos]) throw FileException("characters.txt has a bad W2Pos at line " + intToText(i + 2) + ".");
        used[pos] = true;
        string letter = toUpperText(p[4 + columnIndex]);
        if (letter.size() != 1 || letter[0] != word2[pos - 1]) throw FileException("characters.txt letters do not match clue.txt at line " + intToText(i + 2) + ".");
        people[peopleCount] = createCharacter(p[0], p[1], p[2], letter[0], pos);
        peopleCount++;
    }
}

// Factory: builds the right derived object for a role (stored as Character*)
Character* GameEngine::createCharacter(const string& name, const string& role, const string& nation, char letter, int pos) {
    if (role == "Victim")    return new Victim(name, nation, letter, pos);
    if (role == "Cook")      return new Cook(name, nation, letter, pos);
    if (role == "Guard")     return new Guard(name, nation, letter, pos);
    if (role == "Servant")   return new Servant(name, nation, letter, pos);
    if (role == "Child")     return new Child(name, nation, letter, pos);
    if (role == "PA")        return new PA(name, nation, letter, pos);
    if (role == "Partner 1" || role == "Partner 2") return new Partner(name, role, nation, letter, pos);
    throw FileException("characters.txt contains an unknown role: " + role);
}

// Returns the culprit object
Character* GameEngine::findCulprit() {
    for (int i = 0; i < peopleCount; i++) if (people[i]->isCulprit) return people[i];
    return 0;
}

// ---------------------------------------------------------------- gameplay

// Prints credibility, turns, rack and board
void GameEngine::showStatus() {
    cout << "\nCredibility: " << detective.getCredibility() << " | Turns used: " << detective.getTurns() << "/" << maxTurns << "\n";
    cout << "Rack: ";
    if (detective.getRack1().isEmpty() && detective.getRack2().isEmpty()) cout << "(empty)";
    else {
        mergedRack = detective.getRack1() + detective.getRack2();   // operator+
        mergedRack.display();
    }
    cout << "\n" << scrabble;
}

// One round: show status + game menu, then run the chosen action
void GameEngine::playTurn() {
    showStatus();
    cout << "\n==== Game Menu ====\n[1] Visit a Location\n[2] Interview a Person\n"
         << "[3] View Scrabble Board (free - no turn)\n[4] Form a Word\n[5] Accuse a Suspect\n[6] Quit\n";
    cout << "\n--- TURN " << (detective.getTurns() + 1) << " ---\n";
    int c = readInt("Choice: ", 1, 6);
    switch (c) {
        case 1: visitLocation(); break;
        case 2: interviewPerson(); break;
        case 3: viewBoard(); break;
        case 4: formWord(); break;
        case 5: accuseSuspect(); break;
        case 6: quitGame(); break;
    }
}

// Places a clue on the board and stores it in the right rack
void GameEngine::collectClue(const Clue& clue) {
    scrabble.placeLetter(clue);                                   // may throw
    if (clue.clueRow == WordRow::Word1) detective.getRack1().addTile(clue.clueLetter, clue.cluePosition);
    else                                detective.getRack2().addTile(clue.clueLetter, clue.cluePosition);
}

// Visit a location: first visit collects a clue, later visits ask for confirmation
void GameEngine::visitLocation() {
    while (true) {
        cottage.displayLocations();
        int pick = readInt("Pick: ", 0, cottage.getLocationCount());
        if (pick == 0) return;
        int idx = pick - 1;
        string name = prettyName(cottage.getLocationName(idx));
        if (cottage.isSearched(idx)) {
            cout << "\nYou have already searched the " << name << ".\n";
            if (!readYesNo("Search again? (Y/N): ")) continue;       // NO -> back to location menu
            Clue c = cottage.peekClue(idx);
            cout << "Letter " << c.clueLetter << " is at position " << c.cluePosition << " of " << rowLabel(c.clueRow) << " (already placed).\n";
            detective.useTurn();
            return;
        }
        cout << "\nYou have NOT searched the " << name << " yet.\n";
        if (!readYesNo("Search? (Y/N): ")) continue;
        Clue c = cottage.peekClue(idx);
        collectClue(c);                           // throws before the flag is set if something is wrong
        cottage.searchLocation(idx);
        detective.useTurn();
        cout << "Letter " << c.clueLetter << " collected at position " << c.cluePosition << " of " << rowLabel(c.clueRow) << ".\n";
        cout << "Letter has been placed.\n";
        return;
    }
}

// Interview a person: credibility rules differ per class (polymorphism)
void GameEngine::interviewPerson() {
    while (true) {
        cout << "\n====== Character Menu ======\n";
        for (int i = 0; i < peopleCount; i++)
            cout << "[" << (i + 1) << "] " << people[i]->getName() << " (" << people[i]->getRole() << ") -- "
                 << (people[i]->isInterviewed() ? "Interviewed" : "NOT Interviewed") << "\n";
        cout << "[0] Back\n==========================\n";
        int pick = readInt("Pick: ", 0, peopleCount);
        if (pick == 0) return;
        Character* who = people[pick - 1];
        if (who->isInterviewed()) {
            cout << "\nYou have already interviewed " << who->getName() << ".\n";
            if (!readYesNo("Interview again? (Y/N): ")) continue;
            Clue c = who->giveClue();
            cout << *who << "\n";
            cout << "Letter " << c.clueLetter << " is at position " << c.cluePosition << " of " << rowLabel(c.clueRow) << " (already placed).\n";
            detective.useTurn();
            return;
        }
        cout << "\nYou have NOT interviewed " << who->getName() << " yet.\n";
        if (!readYesNo("Interview? (Y/N): ")) continue;
        if (detective.getCredibility() < who->minCredibility()) {   // virtual call
            detective.loseCredibility(wrongTurnPenalty);
            detective.useTurn();
            throw RuleViolationException(who->getName() + " refuses to talk! You need credibility >= " +
                                         intToText(who->minCredibility()) + ". (-" + intToText(wrongTurnPenalty) + " credibility)");
        }
        cout << "\n" << *who << "\n";                               // polymorphic display
        Clue c = who->giveClue();
        collectClue(c);
        who->interviewed = true;                                    // friend access
        detective.useTurn();
        cout << "Letter " << c.clueLetter << " collected at position " << c.cluePosition << " of " << rowLabel(c.clueRow) << ".\n";
        cout << "Placed.\n";
        return;
    }
}

// Shows the board (free action, no turn)
void GameEngine::viewBoard() {
    printBox("SCRABBLE BOARD");
    cout << scrabble;
    cout << "Rack: ";
    if (detective.getRack1().isEmpty() && detective.getRack2().isEmpty()) cout << "(empty)";
    else { mergedRack = detective.getRack1() + detective.getRack2(); mergedRack.display(); }
    cout << "\n";
}

// Form a word: the row must be complete; a wrong word costs credibility
void GameEngine::formWord() {
    if (!scrabble.isRowComplete(WordRow::Word1) && !scrabble.isRowComplete(WordRow::Word2))
        throw InvalidWordException("No row is complete yet. Collect all letters of a row first.");
    cout << "\n[1] Word 1 (7 letters)\n[2] Word 2 (8 letters)\n";
    int r = readInt("Which word? ", 1, 2);
    WordRow row = (r == 1) ? WordRow::Word1 : WordRow::Word2;
    if (!scrabble.isRowComplete(row)) throw InvalidWordException(rowLabel(row) + " is not complete yet.");
    if (scrabble.isRowSolved(row))    throw InvalidWordException(rowLabel(row) + " is already solved.");
    cout << "Type the word: ";
    string typed = toUpperText(readLine());
    detective.useTurn();
    if (scrabble.checkWord(row, typed)) {
        scrabble.markSolved(row);
        cout << "Correct! " << rowLabel(row) << " = " << typed << "\n";
        if (scrabble.bothSolved()) {
            Character* c = findCulprit();
            if (c) {
                cout << "\nBoth words solved:\n  " << scrabble.getTarget(WordRow::Word1) << " + " << scrabble.getTarget(WordRow::Word2)
                     << " = " << c->getNationality() << " -> " << c->getRole() << " is the culprit!\n";
            } else {
                cout << "\nBoth words solved:\n  " << scrabble.getTarget(WordRow::Word1) << " + " << scrabble.getTarget(WordRow::Word2)
                     << " -> " << culpritRole << " is the culprit!\n";
            }
            cout << "Now use [5] Accuse a Suspect.\n";
        }
    } else {
        detective.loseCredibility(wrongWordPenalty);
        throw InvalidWordException("Wrong word! -" + intToText(wrongWordPenalty) + " credibility. Try again.");
    }
}

// Accuse one of the seven suspects
void GameEngine::accuseSuspect() {
    cout << "\n====== Accuse ======\n";
    int map[MAX_PEOPLE];
    int shown = 0;
    for (int i = 0; i < peopleCount; i++) {
        if (people[i]->isSuspect()) { map[shown++] = i; cout << "[" << shown << "] " << people[i]->getName() << " (" << people[i]->getRole() << ")\n"; }
    }
    cout << "[0] Back\n====================\n";
    int pick = readInt("Pick: ", 0, shown);
    if (pick == 0) return;
    Character* accused = people[map[pick - 1]];
    detective.useTurn();
    cout << "Accuse: " << accused->getName() << " (" << accused->getRole() << ")\n";
    if (accused->getIsCulprit()) {
        cout << accused->getName() << " is the culprit!\n";
        finishGame(true, "");
    } else {
        detective.loseCredibility(wrongTurnPenalty);
        throw WrongAccusationException(accused->getName() + " is innocent! -" + intToText(wrongTurnPenalty) + " credibility.");
    }
}

// Quit to main menu (result is not saved)
void GameEngine::quitGame() {
    if (readYesNo("Quit this investigation? (Y/N): ")) {
        cout << "Investigation abandoned. Returning to main menu.\n";
        state = GameState::Menu;
    }
}

// Checks the lose conditions at the start of every round
void GameEngine::checkEndConditions() {
    if (detective.getCredibility() <= 0) finishGame(false, "Your credibility reached zero.");
    else if (detective.getTurns() >= maxTurns) finishGame(false, "You ran out of turns.");
}

// Shows the result, saves it to the score board and returns to the main menu
void GameEngine::finishGame(bool won, const string& reason) {
    state = won ? GameState::Won : GameState::Lost;
    Character* c = findCulprit();
    string culpritText = c ? (c->getRole() + " " + c->getName()) : culpritRole;
    int turnsLeft = maxTurns - detective.getTurns();
    if (turnsLeft < 0) turnsLeft = 0;
    printBox(detective.getName() + (won ? " WIN!" : " LOSE!"),
             "Final credibility: " + intToText(detective.getCredibility()) + " | Turns used: " +
             intToText(detective.getTurns()) + " | Culprit: " + culpritText);
    if (!won) cout << "  " << reason << "\n";
    if (scoreBoard) scoreBoard->addScore(detective.getName(), won, detective.getCredibility(), turnsLeft, culpritRole);
    pauseScreen();
    clearScreen();
    cout << "Screen Cleared!\n";
    state = GameState::Menu;
}