#pragma once

#include "Map.hpp"

class Actor
{
private:
  int x;
  int y;
  char sym;
  std::string color;
  bool alive;
  std::string originalColor;
public:
  Actor(char s, std::string scolor);
  Point getPos();
  char getSym() const;
  std::string& getColor();
  void setColor(std::string scolor);
  bool canWalk(int dx, int dy, Map& map) const;
  bool move(int dx, int dy, Map& map);
  void move(Point pos, Map& map);
  void changeColor(std::string scolor);
  void revertColor();
  void fade();
  void unfade();
  bool isAlive() const;
  void kill();
  void revive();
  void render();
};

Actor::Actor(char s, std::string scolor)
{
  x = 0;
  y = 0;
  sym = s;
  color = scolor;
  originalColor = scolor;
  alive = true;
}

Point Actor::getPos()
{
  return Point(x, y);
}

char Actor::getSym() const
{
  return sym;
}

std::string& Actor::getColor()
{
  return color;
}

void Actor::setColor(std::string scolor)
{
  color = scolor;
  originalColor = scolor;
}

bool Actor::canWalk(int cx, int cy, Map& map) const
{
  if (!alive)
    return false;

  if (map.inBounds(cx, cy) && !map.board[cy][cx].blocking)
    return true;

  return false;
}

bool Actor::move(int dx, int dy, Map& map)
{
  if (!alive)
    return false;

  int nx = x + dx;
  int ny = y + dy;
  if (canWalk(nx, ny, map))
  {
    x = nx;
    y = ny;
    return true;
  }

  return false;
}

void Actor::move(Point pos, Map& map)
{
  if (!alive)
    return;

  if (canWalk(pos.x, pos.y, map))
  {
    x = pos.x;
    y = pos.y;
  }
}

void Actor::changeColor(std::string scolor)
{
  if (!alive)
    return;

  color = scolor;
}

void Actor::revertColor()
{
  if (!alive)
    return;

  color = originalColor;
}

void Actor::fade()
{
  if (!alive)
    return;

  color = "darker " + color;
}

void Actor::unfade()
{
  if (!alive)
    return;
  color = color.substr(7);
}

bool Actor::isAlive() const
{
  return alive;
}

void Actor::kill()
{
  alive = false;
}

void Actor::revive()
{
  alive = true;
}

void Actor::render()
{
  if (!alive)
    return;

  terminal_color(color_from_name(color.c_str()));
  terminal_put(x, y, sym);
}
