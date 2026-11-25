#ifndef HEIST_H
#define HEIST_H

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// DynamicArray utilizes dynamic memory to store objects without STL containers.
template <class T>
class DynamicArray {
    T* data;
    int count;
    int capacity;
    void grow() {
        int newCapacity = capacity * 2;
        T* temp = new T[newCapacity];
        for (int i = 0; i < count; ++i) temp[i] = data[i];
        delete[] data;
        data = temp;
        capacity = newCapacity;
    }
public:
    DynamicArray(int cap = 4) : data(new T[cap]), count(0), capacity(cap) {}
    DynamicArray(const DynamicArray& other) : data(new T[other.capacity]), count(other.count), capacity(other.capacity) {
        for (int i = 0; i < count; ++i) data[i] = other.data[i];
    }
    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            delete[] data;
            capacity = other.capacity; count = other.count;
            data = new T[capacity];
            for (int i = 0; i < count; ++i) data[i] = other.data[i];
        }
        return *this;
    }
    ~DynamicArray() { delete[] data; }
    void push(const T& value) { if (count == capacity) grow(); data[count++] = value; }
    int size() const { return count; }
    bool empty() const { return count == 0; }
    T& operator[](int i) { return data[i]; }
    const T& operator[](int i) const { return data[i]; }
    void clear() { count = 0; }
};

enum WordRow { Word1, Word2 };
enum GameState { Menu, Playing, Won, Lost, Exit };
enum MenuChoice { NewGame = 1, LeaderBoard, About, Rules, Quit };

class GameException {
    string message;
public:
    GameException(const string& m) : message(m) {}
    string what() const { return "HEIST ERROR: " + message; }
};

struct Tile {
    char tileLetter;
    bool filled;
    Tile(char c = '_', bool f = false) : tileLetter(c), filled(f) {}
};
struct Clue {
    char clueLetter;
    int cluePosition;
    WordRow clueRow;
    Clue(char c = '?', int p = 0, WordRow r = Word1) : clueLetter(c), cluePosition(p), clueRow(r) {}
};
struct Location {
    string locName;
    char letterClueLoc;
    int boardPosLoc;
    WordRow locRow;
    bool locSearchFlag;
    Location(string n = "", char c = '?', int p = 0, WordRow r = Word1)
        : locName(n), letterClueLoc(c), boardPosLoc(p), locRow(r), locSearchFlag(false) {
    }
};

ostream& operator<<(ostream& out, const Tile& t);
istream& operator>>(istream& in, Tile& t);
ostream& operator<<(ostream& out, const Location& l);
istream& operator>>(istream& in, Location& l);

class TileRack {
    DynamicArray<char> letters;
public:
    void add(char c);
    bool contains(char c) const;
    int size() const;
    char operator[](int i) const;
    friend TileRack operator+(const TileRack& a, const TileRack& b);
};
TileRack operator+(const TileRack& a, const TileRack& b);

class Scrabble {
    string words[2];
    bool solved[2];
    DynamicArray<char> collected[2];
public:
    Scrabble();
    void setWords(const string& a, const string& b);
    void addClue(WordRow row, char c);
    void display() const;
    bool formWord(WordRow row);
    bool allSolved() const;
    string getWord(WordRow row) const;
};

class Character {
protected:
    string characterName, characterRole, nationality;
    bool isCulprit, interviewed;
public:
    Character();
    Character(const string& n, const string& r, const string& nation);
    virtual ~Character() {}
    virtual void display() const = 0;
    string getName() const;
    string getRole() const;
    bool culprit() const;
    void setCulprit(bool v);
    bool wasInterviewed() const;
    void setInterviewed(bool v);
    friend ostream& operator<<(ostream& out, const Character& c);
};

class Suspect : virtual public Character {
protected:
    char wordLetter;
    int wordPosition;
public:
    Suspect(const string& n = "", const string& r = "", const string& nation = "", char l = '?', int p = 0);
    void display() const override;
    char getLetter() const;
    int getPosition() const;
};
class BusinessRelations : virtual public Suspect {
public:
    BusinessRelations(const string& n = "", const string& r = "", const string& nation = "", char l = '?', int p = 0);
};
class CloseRelations : virtual public Suspect {
public:
    CloseRelations(const string& n = "", const string& r = "", const string& nation = "", char l = '?', int p = 0);
};
class HouseHelp : public Suspect {
public:
    HouseHelp(const string& n = "", const string& r = "", const string& nation = "", char l = '?', int p = 0);
};
class Partner : public BusinessRelations {
public:
    Partner(const string& n, const string& nation, char l, int p);
};
class PA : public BusinessRelations, public CloseRelations {
public:
    PA(const string& n, const string& nation, char l, int p);
};
class Child : public CloseRelations {
public:
    Child(const string& n, const string& nation, char l, int p);
};
class Victim : public Character {
public:
    Victim();
    void display() const override;
};
class Detective : public Character {
    int credibility, turns;
public:
    Detective(const string& n = "Detective", int c = 100);
    void display() const override;
    int getCredibility() const;
    int getTurns() const;
    void addTurn();
    void penalize(int amount);
};

class Cottage {
    DynamicArray<Location> locations;
public:
    void addLocation(const Location& l);
    int size() const;
    Location& at(int i);
    void display() const;
};

class ScoreBoard {
public:
    void show() const;
    void add(const string& detective, const string& result, int credibility, int turns, const string& culprit);
};

class GameEngine {
    static GameEngine* instance;
    GameState state;
    Detective* detective;
    Cottage cottage;
    Scrabble scrabble;
    ScoreBoard scoreBoard;
    string culpritName, culpritRole;
    int maxTurns;
    GameEngine();
    void loadData();
    void newGame();
    void play();
    void accuse();
    void pause();
public:
    static GameEngine* getInstance();
    static void destroyInstance();
    ~GameEngine();
    void run();
};

#endif
