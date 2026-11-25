// ============================================================================
// main.cpp - Entry point for THE LOCKER HEIST
// ============================================================================
#include "heist.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

int main() {
    // Seed random number generator for culprit selection
    srand(static_cast<unsigned int>(time(0)));

    ScoreBoard board("leader/scores.txt");
    GameEngine* engine = GameEngine::getInstance();
    engine->attachScoreBoard(&board);

    while (true) {
        try {
            engine->run();
            break; // Normal exit from main loop
        }
        catch (const InputClosedException& e) {
            cout << "\n[Input closed. Exiting game.]\n";
            break;
        }
        catch (const GameException& e) {
            cout << "\n[ERROR]: " << e.what() << "\n";
            // Exception caught; engine state resets to Menu on next loop iteration
        }
        catch (const std::exception& e) {
            cout << "\n[STANDARD ERROR]: " << e.what() << "\n";
        }
        catch (...) {
            cout << "\n[UNKNOWN ERROR OCCURRED]\n";
        }
    }

    GameEngine::destroyInstance();
    return 0;
}
