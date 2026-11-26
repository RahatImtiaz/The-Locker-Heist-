#ifndef HEIST_H
#define HEIST_H

#include <iostream>
#include <string>
#include <exception>

using namespace std;

// ============================================================================
// ENUMS
// ============================================================================
enum class WordRow { Word1, Word2 };
enum class GameState { Menu, Playing, Won, Lost, Exit };
enum class MenuChoice { NewGame = 1, LeaderBoard, About, Rules, Exit };

// ============================================================================
// CUSTOM EXCEPTIONS
// ============================================================================
class GameException : public exception {
protected:
    string msg;
public:
    GameException(const string& m) : msg(m) {}
    virtual const char* what() const throw() { return msg.c_str(); }
    virtual ~GameException() throw() {}
};

class FileException : public GameException {
public:
    FileException(const string& m) : GameException(m) {}
};

class InvalidInputException : public GameException {
public:
    InvalidInputException(const string& m) : GameException(m) {}
};

class InputClosedException : public GameException {
public:
    InputClosedException() : GameException("Input stream closed.") {}
};

class RackFullException : public GameException {
public:
    RackFullException(const string& m) : GameException(m) {}
};

class RackEmptyException : public GameException {
public:
    RackEmptyException(const string& m) : GameException(m) {}
};

class WrongLetterException : public GameException {
public:
    WrongLetterException(const string& m) : GameException(m) {}
};

class InvalidWordException : public GameException {
public:
    InvalidWordException(const string& m) : GameException(m) {}
};

class InvalidPositionException : public GameException {
public:
    InvalidPositionException(const string& m) : GameException(m) {}
};

class SlotFilledException : public GameException {
public:
    SlotFilledException(const string& m) : GameException(m) {}
};

class RuleViolationException : public GameException {
public:
    RuleViolationException(const string& m) : GameException(m) {}
};

class WrongAccusationException : public GameException {
public:
    WrongAccusationException(const string& m) : GameException(m) {}
};

// ============================================================================
// TEMPLATE DYNAMIC ARRAY
// ============================================================================
template <typename T>
class DynamicArray {
private:
    T* data;
    int capacity;
    int count;

    void resize(int newCap) {
        T* newData = new T[newCap];
        for (int i = 0; i < count; i++) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
        capacity = newCap;
    }

public:
    DynamicArray(int initialCap = 10) : capacity(initialCap), count(0) {
        if (capacity <= 0) capacity = 10;
        data = new T[capacity];
    }

    DynamicArray(const DynamicArray& other) : capacity(other.capacity), count(other.count) {
        data = new T[capacity];
        for (int i = 0; i < count; i++) data[i] = other.data[i];
    }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            delete[] data;
            capacity = other.capacity;
            count = other.count;
            data = new T[capacity];
            for (int i = 0; i < count; i++) data[i] = other.data[i];
        }
        return *this;
    }

    ~DynamicArray() { delete[] data; }

    void push(const T& val) {
        if (count >= capacity) resize(capacity * 2);
        data[count++] = val;
    }

    void removeAt(int index) {
        if (index < 0 || index >= count) return;
        for (int i = index; i < count - 1; i++) {
            data[i] = data[i + 1];
        }
        count--;
    }

    void clear() { count = 0; }
    int size() const { return count; }

    T& operator[](int index) { return data[index]; }
    const T& operator[](int index) const { return data[index]; }
};

// ============================================================================
// GAME STRUCTS & UTILITIES
// ============================================================================
struct Tile {
    char tileLetter;
    bool filled;
    Tile(char l = ' ', bool f = false) : tileLetter(l), filled(f) {}
};

struct Clue {
    char clueLetter;
    int cluePosition;
    WordRow clueRow;
    Clue(char l = ' ', int p = 0, WordRow r = WordRow::Word1)
        : clueLetter(l), cluePosition(p), clueRow(r) {}
};

struct Location {
    string locName;
    char letterClueLoc;
    int boardPosLoc;
    WordRow locRow;
    bool locSearchFlag;
    Location() : locName(""), letterClueLoc(' '), boardPosLoc(0), locRow(WordRow::Word1), locSearchFlag(false) {}
};

bool operator==(const Tile& a, const Tile& b);
bool operator==(const Clue& a, const Clue& b);
ostream& operator<<(ostream& os, const Tile& t);
istream& operator>>(istream& is, Tile& t);
ostream& operator<<(ostream& os, const Location& loc);
istream& operator>>(istream& is, Location& loc);

string trimText(const string& s);
string toUpperText(const string& s);
string intToText(int value);
bool parseInt(const string& text, int& result);
int splitLine(const string& line, char delim, string parts[], int maxParts);
string rowLabel(WordRow row);

// ============================================================================
// TileRack & Scrabble & Cottage
// ============================================================================
class TileRack {
private:
    DynamicArray<Tile> tiles;
    DynamicArray<int> positions;
    int maxTiles;

public:
    TileRack(int limit = 10);
    TileRack(const TileRack& other);
    TileRack& operator=(const TileRack& other);
    ~TileRack();

    void addTile(char letter, int position);
    void removeTile(char letter);
    int getCount() const;
    bool isFull() const;
    bool isEmpty() const;
    void clear();
    void display() const;

    TileRack operator+(const TileRack& other) const;
};

class Scrabble {
private:
    DynamicArray<Tile> row1;
    DynamicArray<Tile> row2;
    string word1;
    string word2;
    bool solved1;
    bool solved2;

    DynamicArray<Tile>& rowOf(WordRow r);
    const DynamicArray<Tile>& rowOf(WordRow r) const;
    const string& targetOf(WordRow r) const;

public:
    static const int WORD1_LEN = 7;
    static const int WORD2_LEN = 8;

    Scrabble();
    Scrabble(const Scrabble& other);
    Scrabble& operator=(const Scrabble& other);
    ~Scrabble();

    void loadWords(const string& w1, const string& w2);
    void placeLetter(const Clue& clue);
    bool isRowComplete(WordRow r) const;
    bool isRowSolved(WordRow r) const;
    bool bothSolved() const;
    bool checkWord(WordRow r, const string& typed) const;
    void markSolved(WordRow r);
    string getTarget(WordRow r) const;
    void displayRow(ostream& os, WordRow r) const;

    friend ostream& operator<<(ostream& os, const Scrabble& s);
};

class Cottage {
private:
    DynamicArray<Location> locCache;

public:
    static const int LOCATION_COUNT = 7;

    Cottage();
    Cottage(const Cottage& other);
    Cottage& operator=(const Cottage& other);
    ~Cottage();

    void addLocation(const string& name, char letter, int position);
    int getLocationCount() const;
    bool isSearched(int index) const;
    Clue searchLocation(int index);
    Clue peekClue(int index) const;
    string getLocationName(int index) const;
    void displayLocations() const;
    void reset();
};

// ============================================================================
// CHARACTER HIERARCHY
// ============================================================================
class Character {
protected:
    string characterName;
    string characterRole;
    string nationality;
    bool isCulprit;
    bool interviewed;

public:
    Character(const string& name, const string& role, const string& nation);
    virtual ~Character();

    virtual void display(ostream& os) const = 0;
    virtual Clue giveClue() const;
    virtual int minCredibility() const;
    virtual bool isSuspect() const;

    string getName() const;
    string getRole() const;
    string getNationality() const;
    bool getIsCulprit() const;
    bool isInterviewed() const;

    friend class GameEngine;
    friend ostream& operator<<(ostream& os, const Character& c);
};

class Victim : public Character {
private:
    WordRow clueRowChar;
    char letterClueChar;
    int boardPosChar;

public:
    Victim(const string& name, const string& nation, char letter, int position);
    virtual void display(ostream& os) const;
    virtual Clue giveClue() const;
    virtual int minCredibility() const;
};

class Detective : public Character {
private:
    int credibility;
    int turns;
    TileRack rack1;
    TileRack rack2;

public:
    Detective(const string& name = "Detective", int startCredibility = 100);
    virtual void display(ostream& os) const;
    void loseCredibility(int amount);
    void useTurn();
    int getCredibility() const;
    int getTurns() const;
    TileRack& getRack1();
    TileRack& getRack2();
};

class Suspect : virtual public Character {
protected:
    WordRow clueRowChar;
    char letterClueChar;
    int boardPosChar;

public:
    Suspect(const string& name, const string& role, const string& nation, char letter, int position);
    virtual Clue giveClue() const;
    virtual bool isSuspect() const;
};

class BusinessRelations : virtual public Suspect {
public:
    BusinessRelations(const string& name, const string& role, const string& nation, char letter, int position);
};

class HouseHelp : public Suspect {
public:
    HouseHelp(const string& name, const string& role, const string& nation, char letter, int position);
};

class CloseRelations : virtual public Suspect {
public:
    CloseRelations(const string& name, const string& role, const string& nation, char letter, int position);
};

class Partner : public BusinessRelations {
public:
    Partner(const string& name, const string& role, const string& nation, char letter, int position);
    virtual void display(ostream& os) const;
    virtual int minCredibility() const;
};

class PA : public BusinessRelations, public CloseRelations {
public:
    PA(const string& name, const string& nation, char letter, int position);
    virtual void display(ostream& os) const;
    virtual int minCredibility() const;
};

class Cook : public HouseHelp {
public:
    Cook(const string& name, const string& nation, char letter, int position);
    virtual void display(ostream& os) const;
};

class Guard : public HouseHelp {
public:
    Guard(const string& name, const string& nation, char letter, int position);
    virtual void display(ostream& os) const;
};

class Servant : public HouseHelp {
public:
    Servant(const string& name, const string& nation, char letter, int position);
    virtual void display(ostream& os) const;
};

class Child : public CloseRelations {
public:
    Child(const string& name, const string& nation, char letter, int position);
    virtual void display(ostream& os) const;
    virtual int minCredibility() const;
};

// ============================================================================
// SCOREBOARD & GAMEENGINE
// ============================================================================
class ScoreBoard {
private:
    string filePath;
    static string todayDate();

public:
    ScoreBoard(const string& path = "leader/scoreBoard.txt");
    void addScore(const string& detectiveName, bool won, int credibility, int turnsLeft, const string& culprit);
    void displayAll() const;
};

class GameEngine {
private:
    static const int MAX_PEOPLE = 8;

    static GameEngine* instance;

    Detective detective;
    Cottage cottage;
    Scrabble scrabble;
    TileRack mergedRack;

    Character* people[MAX_PEOPLE];
    int peopleCount;

    ScoreBoard* scoreBoard;

    GameState state;
    bool titleShown;

    int maxTurns;
    int startCredibility;
    int wrongTurnPenalty;
    int wrongWordPenalty;
    string winCondition;
    string loseCondition;
    string culpritRole;

    GameEngine();
    ~GameEngine();
    GameEngine(const GameEngine&);
    GameEngine& operator=(const GameEngine&);

    void clearPeople();
    string readLine();
    int readInt(const string& prompt, int low, int high);
    bool readYesNo(const string& prompt);
    void pauseScreen();
    void clearScreen();
    void printBox(const string& line1, const string& line2 = "");

    void showTitle();
    void showMainMenu();
    void handleMainChoice(MenuChoice choice);
    void showScoreBoard();
    void showAbout();
    void loadRules();
    void showRules();

    string askDetectiveName();
    void startNewGame();
    void loadGameData();
    void loadLocations(int columnIndex, const string& word1);
    void loadCharacters(int columnIndex, const string& word2);
    Character* createCharacter(const string& name, const string& role, const string& nation, char letter, int pos);
    Character* findCulprit();

    void showStatus();
    void playTurn();
    void collectClue(const Clue& clue);
    void visitLocation();
    void interviewPerson();
    void viewBoard();
    void formWord();
    void accuseSuspect();
    void quitGame();
    void checkEndConditions();
    void finishGame(bool won, const string& reason);

public:
    static GameEngine* getInstance();
    static void destroyInstance();

    void attachScoreBoard(ScoreBoard* board);
    void run();
};

#endif // HEIST_H
