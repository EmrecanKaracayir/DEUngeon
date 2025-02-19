#pragma once

#include "Actor.hpp"
#include "Map.hpp"
#include "PathFinding.hpp"

static long long getCurrentTimeInMilliseconds()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::high_resolution_clock::now().time_since_epoch()
  ).count();
}

enum class GameState : uint8_t
{
  RUNNING,
  PAUSED,
  STOPPED
};

struct Enemy
{
  Actor actor;
  int moveDelay;
  long long moveTimer;
  Enemy(Actor a, int md, long long mt)
    : actor(a)
    , moveDelay(md)
    , moveTimer(mt)
  {
    originalMoveDelay = moveDelay;
  }
  void stun()
  {
    actor.fade();
    moveDelay = 2000;
  }
  void unstun()
  {
    actor.unfade();
    moveDelay = originalMoveDelay;
  }
  bool isStunned() const
  {
    return moveDelay != originalMoveDelay;
  }
  void rage(std::string color)
  {
    originalMoveDelay = moveDelay = static_cast<int>(originalMoveDelay * 0.8F);
    actor.setColor(color);
  }
private:
  int originalMoveDelay;
};


struct Player
{
  Actor actor;
  int moveDelay;
  long long moveTimer;
  int dashes;
  int destroys;
  Player(Actor actor, int moveDelay, long long moveTimer)
    : actor(actor)
    , moveDelay(moveDelay)
    , moveTimer(moveTimer)
  {
    dashes = 0;
    destroys = 0;
    dashing = false;
  }
  bool isDashing() const
  {
    return dashing;
  }
  void dash()
  {
    if (dashes > 0)
    {
      actor.changeColor("white");
      dashing = true;
      dashes--;
    }
  }
  void destroy(Map& map, std::vector<Enemy>& enemies)
  {
    if (destroys > 0)
    {
      actor.changeColor("red");
      Point pos = actor.getPos();
      for (int dx = -5; dx <= 5; dx++)
      {
        for (int dy = -5; dy <= 5; dy++)
        {
          if (dx == 0 && dy == 0) continue;

          int newX = pos.x + dx;
          int newY = pos.y + dy;

          if (map.inBounds(newX, newY))
          {
            map.board[newY][newX].terrain = TERRAIN::BOMBED;
            map.board[newY][newX].blocking = false;
          }

          for (auto& enemy : enemies)
          {
            if (enemy.actor.getPos() == Point(newX, newY))
            {
              enemy.actor.kill();
              // Other enemies get speed boost
              for (auto& otherEnemy : enemies)
              {
                if (&otherEnemy != &enemy)
                {
                  otherEnemy.rage(enemy.actor.getColor());
                }
              }
            }
          }
        }
      }
      actor.revertColor();
      destroys--;
    }
  }
  void stopDash()
  {
    actor.revertColor();
    dashing = false;
  }
private:
  bool dashing;
};

class Engine
{
private:
  int m_maxX;
  int m_maxY;
  Map m_map;
  Player m_player;
  std::vector<Enemy> m_enemies;
  std::vector<Actor> m_powerUps;
  GameState m_state;
  AStar m_astar;
  long long gameTimer;
  int gameTime;
public:
  Engine(int wx, int wy, int numRooms);
  bool gameLoop();
  void render();
private:
  void enemyMove();
  bool actorDied();
  void collectPowerUp();
  void printGameTime();
  void printGameOver() const;
  void printDashes();
  void printDestroys();
  void printGameState();
};

Engine::Engine(int wx, int wy, int numRooms)
  : m_maxX(wx)
  , m_maxY(wy)
  , m_map(Map(m_maxX, m_maxY))
  , m_player(Player(Actor('@', "cyan"), 75, getCurrentTimeInMilliseconds()))
  , m_powerUps()
  , m_enemies()
  , m_state(GameState::PAUSED)
  , m_astar(AStar(m_map))
  , gameTimer(getCurrentTimeInMilliseconds())
  , gameTime(30)
{
  // Prepare map
  m_map.makeRooms(numRooms);

  // Place player
  Point pStartCoords = m_map.getStartCoords(true);
  m_player.actor.move(pStartCoords.x, pStartCoords.y, m_map);

  // Create power-ups
  for (int i = 0; i < 5; i++)
  {
    Point puCoords = m_map.getRandomCoords();
    m_powerUps.emplace_back('>', "dark cyan");
    m_powerUps.back().move(puCoords.x, puCoords.y, m_map);
  }
  for (int i = 0; i < 5; i++)
  {
    Point puCoords = m_map.getRandomCoords();
    m_powerUps.emplace_back('x', "dark cyan");
    m_powerUps.back().move(puCoords.x, puCoords.y, m_map);
  }


  // Create enemies
  auto currentTime = getCurrentTimeInMilliseconds();
  m_enemies.emplace_back(Actor('?', "blue"), 350, currentTime);
  m_enemies.emplace_back(Actor('$', "green"), 300, currentTime);
  m_enemies.emplace_back(Actor('&', "yellow"), 250, currentTime);
  m_enemies.emplace_back(Actor('%', "orange"), 200, currentTime);
  m_enemies.emplace_back(Actor('#', "red"), 150, currentTime);

  // Place enemies
  Point eStartCoords = m_map.getStartCoords(false);
  for (auto& enemy : m_enemies)
  {
    enemy.actor.move(eStartCoords.x--, eStartCoords.y, m_map);
  }

  // Render start screen
  render();
}

bool Engine::gameLoop()
{
  char keypress{};
  char lastDir{};
  while (m_state != GameState::STOPPED)
  {
    if (terminal_has_input())
    {
      keypress = terminal_read();

      if (keypress == TK_ENTER)
      {
        m_state = m_state == GameState::PAUSED ? GameState::RUNNING : GameState::PAUSED;
      }
      else if (keypress == TK_ESCAPE)
      {
        m_state = GameState::STOPPED;
      }
    }

    if (m_state != GameState::RUNNING)
    {
      render();
      continue;
    }

    auto currentTime = getCurrentTimeInMilliseconds();
    if (currentTime >= m_player.moveTimer + m_player.moveDelay
        || keypress != lastDir
        || m_player.isDashing())
    {
      if (m_player.isDashing())
      {
        switch (lastDir)
        {
          case TK_UP:
          case TK_W:
            if (!m_player.actor.move(0, -1, m_map))
            {
              m_player.stopDash();
            }
            break;
          case TK_DOWN:
          case TK_S:
            if (!m_player.actor.move(0, 1, m_map))
            {
              m_player.stopDash();
            }
            break;
          case TK_LEFT:
          case TK_A:
            if (!m_player.actor.move(-1, 0, m_map))
            {
              m_player.stopDash();
            }
            break;
          case TK_RIGHT:
          case TK_D:
            if (!m_player.actor.move(1, 0, m_map))
            {
              m_player.stopDash();
            }
            break;
        }
      }
      else
      {
        switch (keypress)
        {
          case TK_UP:
          case TK_W:
            m_player.actor.move(0, -1, m_map);
            lastDir = keypress;
            break;
          case TK_DOWN:
          case TK_S:
            m_player.actor.move(0, 1, m_map);
            lastDir = keypress;
            break;
          case TK_LEFT:
          case TK_A:
            m_player.actor.move(-1, 0, m_map);
            lastDir = keypress;
            break;
          case TK_RIGHT:
          case TK_D:
            m_player.actor.move(1, 0, m_map);
            lastDir = keypress;
            break;
          case TK_SHIFT:
            m_player.dash();
            break;
          case TK_SPACE:
            m_player.destroy(m_map, m_enemies);
            break;
          case TK_ESCAPE:
            m_state = GameState::STOPPED;
            break;
          default:
            break;
        }
      }
      keypress = 0;
      m_player.moveTimer = currentTime;
    }

    enemyMove();

    if (!actorDied())
    {
      collectPowerUp();

      if (getCurrentTimeInMilliseconds() >= gameTimer + 1000)
      {
        gameTime--;
        gameTimer = getCurrentTimeInMilliseconds();
      }
      if (gameTime == 0)
      {
        m_state = GameState::STOPPED;
      }
    }

    render();
  }

  return true;
}

void Engine::enemyMove()
{
  auto currentTime = getCurrentTimeInMilliseconds();
  for (auto& enemy : m_enemies)
  {
    if (currentTime >= enemy.moveTimer + enemy.moveDelay)
    {
      if (enemy.isStunned())
      {
        enemy.unstun();
      }
      auto path = m_astar.findPath(enemy.actor.getPos(), m_player.actor.getPos());
      if (path.size() != 0)
      {
        enemy.actor.move(
          path[0], m_map
        );
      }
      enemy.moveTimer = currentTime;
    }
  }
}

bool Engine::actorDied()
{
  std::vector<std::vector<Enemy>::iterator> enemiesToRemove;
  for (auto& enemy : m_enemies)
  {
    if (m_player.actor.getPos() == enemy.actor.getPos())
    {
      if (m_player.isDashing())
      {
        enemy.stun();
      }
      else if (!enemy.isStunned())
      {
        m_state = GameState::STOPPED;
        return true;
      }
    }
  }
  return false;
}

void Engine::collectPowerUp()
{
  for (auto& powerUp : m_powerUps)
  {
    if (powerUp.isAlive())
    {
      if (m_player.actor.getPos() == powerUp.getPos())
      {
        if (powerUp.getSym() == '>')
        {
          m_player.dashes++;
        }
        else if (powerUp.getSym() == 'x')
        {
          m_player.destroys++;
        }
        powerUp.kill();
        break;
      }
    }
  }
}

void Engine::render()
{
  terminal_clear();

  m_map.render();

  for (auto& powerUp : m_powerUps)
  {
    powerUp.render();
  }

  m_player.actor.render();

  for (auto& enemy : m_enemies)
  {
    enemy.actor.render();
  }

  printGameTime();

  printDashes();

  printDestroys();

  printGameState();


  if (m_state == GameState::STOPPED)
  {
    printGameOver();
  }

  terminal_refresh();
}

void Engine::printGameTime()
{
  if (gameTime < 5)
  {
    terminal_color(color_from_name("green"));
  }
  else if (gameTime < 10)
  {
    terminal_color(color_from_name("yellow"));
  }
  else if (gameTime < 15)
  {
    terminal_color(color_from_name("orange"));
  }
  else
  {
    terminal_color(color_from_name("red"));
  }
  terminal_print(0, 0, ("Time: " + std::to_string(gameTime)).c_str());
}

void Engine::printGameOver() const
{
  if (gameTime > 0)
  {
    terminal_color(color_from_name("red"));
    terminal_print(0, 1, "GAME OVER! YOU LOST!");
  }
  else
  {
    terminal_color(color_from_name("green"));
    terminal_print(0, 1, "GAME OVER! YOU WON!");
  }
}

void Engine::printDashes()
{
  if (m_player.dashes > 1)
  {
    terminal_color(color_from_name("green"));
  }
  else if (m_player.dashes > 0)
  {
    terminal_color(color_from_name("yellow"));
  }
  else
  {
    terminal_color(color_from_name("red"));
  }
  terminal_print(0, m_maxY - 2, ("Dashes: " + std::to_string(m_player.dashes)).c_str());
}

void Engine::printDestroys()
{
  if (m_player.destroys > 1)
  {
    terminal_color(color_from_name("green"));
  }
  else if (m_player.destroys > 0)
  {
    terminal_color(color_from_name("yellow"));
  }
  else
  {
    terminal_color(color_from_name("red"));
  }
  terminal_print(9, m_maxY - 2, (", Destroys: " + std::to_string(m_player.destroys)).c_str());
}



void Engine::printGameState()
{
  switch (m_state)
  {
    case GameState::RUNNING:
      terminal_color(color_from_name("green"));
      terminal_print(0, m_maxY - 1, "State: RUNNING");
      break;
    case GameState::PAUSED:
      terminal_color(color_from_name("yellow"));
      terminal_print(0, m_maxY - 1, "State: PAUSED");
      break;
    case GameState::STOPPED:
      terminal_color(color_from_name("red"));
      terminal_print(0, m_maxY - 1, "State: STOPPED");
      break;
  }
}
