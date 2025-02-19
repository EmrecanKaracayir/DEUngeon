#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <queue>
#include <random>
#include <thread>
#include <vector>

#include <BearLibTerminal.h>

#include "Actor.hpp"
#include "Engine.hpp"
#include "Map.hpp"
#include "PathFinding.hpp"

using namespace std;

static void initBearLib(int wx, int wy)
{
  terminal_open();
  std::string windowSize = "window: size=" + std::to_string(wx) + "x" + std::to_string(wy) + ";";
  terminal_set(windowSize.c_str());
  terminal_refresh();
}

int main()
{
  int wx = 100;
  int wy = 50;
  initBearLib(wx, wy);
  while (true)
  {
    Engine eng(wx, wy, 15);
    eng.gameLoop();
  }
  terminal_close();
  return 0;
}
