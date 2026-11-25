#include "heist.h"

int main() {
    try {
        GameEngine* game = GameEngine::getInstance();
        game->run();
        GameEngine::destroyInstance();
    }
    catch (const GameException& error) {
        cout << error.what() << endl;
        GameEngine::destroyInstance();
        return 1;
    }
    catch (...) {
        cout << "HEIST ERROR: An unexpected error occurred." << endl;
        GameEngine::destroyInstance();
        return 1;
    }
    return 0;
}
