# Build:  make      Run:  ./locker_heist      Clean:  make clean
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic

locker_heist: main.cpp heist.cpp heist.h
	$(CXX) $(CXXFLAGS) -o locker_heist main.cpp heist.cpp

clean:
	rm -f locker_heist
