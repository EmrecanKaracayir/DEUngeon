#pragma once

constexpr int ROOM_BUFFER{ 2 };

enum class TERRAIN : uint8_t
{
  NONE,
  CAVE,
  ROCK,
  TUNNEL,
  BOMBED,
};

struct Edge
{
  int u, v;
  double weight;
  Edge(int _u, int _v, double _w) : u(_u), v(_v), weight(_w) {}
  bool operator<(const Edge& other) const
  {
    return weight > other.weight;
  }
};

class Point
{
public:
  int x{};
  int y{};
  bool blocking{};
  TERRAIN terrain{};
  Point(int X, int Y, bool b, TERRAIN t) : x(X), y(Y), blocking(b), terrain(t) {}
  Point(int X, int Y) : x(X), y(Y), blocking(false), terrain(TERRAIN::NONE) {}
  Point() {}

  bool operator==(const Point& other) const
  {
    return x == other.x && y == other.y;
  }
};

class Rect
{
public:
  int left{};
  int right{};
  int top{};
  int bottom{};
  int centX{};
  int centY{};
  Rect(int X1, int X2, int Y1, int Y2) : left(X1), right(X2), top(Y1), bottom(Y2)
  {
    centX = (left + right) / 2; centY = (top + bottom) / 2;
  }
  Rect() {}
};

class Map
{
public:
  std::vector<std::vector<Point>> board;
  int map_w;
  int map_h;
  Map(int mw, int mh);
  Map() {}
  bool inBounds(int x, int y) const;
  void Dig(int sx, int sy, int w, int h, TERRAIN terr);
  void makeRooms(int numRooms);
  void tunnel(std::vector<Rect>& rooms);
  void render() const;
  Point getStartCoords(bool isPlayer);
  Point getRandomCoords();
private:
  void createTunnel(Rect& start, Rect& fin);
};

Map::Map(int mw, int mh)
{
  map_w = mw;
  map_h = mh;
  board.resize(mh, std::vector<Point>(mw));
  for (int y = 0; y < mh; y++)
  {
    for (int x = 0; x < mw; x++)
    {
      board[y][x] = Point(x, y, true, TERRAIN::ROCK);
    }
  }
}

//verifies a given coordinate is on the map
bool Map::inBounds(int x, int y) const
{
  return x > 0 && x < map_w && y > 0 && y < map_h;
}

void Map::Dig(int left, int top, int right, int bottom, TERRAIN terr)
{
  int stopY = bottom;
  int stopX = right;
  if (bottom > map_h) stopY = map_h - 2;
  if (right > map_w) stopX = map_w - 2;
  for (int y = top; y < stopY; y++)
  {
    for (int x = left; x < stopX; x++)
    {
      board[y][x].blocking = false;
      board[y][x].terrain = terr;
    }
  }
}

bool overlaps(Rect a, Rect b)
{
  // Check if a is to the right of b or b is to the right of a
  if (a.left > b.right + ROOM_BUFFER || b.left > a.right + ROOM_BUFFER)
  {
    return false;
  }

  // Check if a is above b or b is above a
  if (a.top > b.bottom + ROOM_BUFFER || b.top > a.bottom + ROOM_BUFFER)
  {
    return false;
  }

  // If neither of the above conditions are met, the rooms overlap
  return true;
}

void Map::makeRooms(int numRooms)
{
  const unsigned int MAX_SIZE = 12;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> randRoomSize(6, MAX_SIZE);
  std::uniform_int_distribution<int> randRoomX(3, map_w - MAX_SIZE - 3);
  std::uniform_int_distribution<int> randRoomY(3, map_h - MAX_SIZE - 3);
  int left{}, top{}, right{}, bottom{}, roomWidth{}, roomHeight{}, roomSize{};
  Rect room;
  std::vector<Rect> rooms;
  while (rooms.size() < numRooms)
  {
    roomSize = randRoomSize(gen);
    left = randRoomX(gen);
    top = randRoomY(gen);
    right = left + roomSize;
    bottom = top + roomSize;
    room = Rect(left, right, top, bottom);
    if (rooms.empty())
      rooms.push_back(room);
    else
    {
      bool overlap = false;
      for (auto& r : rooms)
      {
        if (overlaps(r, room))
        {
          overlap = true;
          break;
        }
      }
      if (!overlap)
      {
        rooms.push_back(room);
      }
    }
    for (auto& r : rooms)
      Dig(r.left, r.top, r.right, r.bottom, TERRAIN::CAVE);
  }
  tunnel(rooms);
}

void Map::tunnel(std::vector<Rect>& rooms)
{
  auto n = static_cast<int>(rooms.size());
  std::vector<std::vector<std::pair<int, double>>> adj(n);

  // Step 1: Create a graph
  for (int i = 0; i < n; ++i)
  {
    for (int j = i + 1; j < n; ++j)
    {
      double dist = std::hypot(rooms[i].centX - rooms[j].centX, rooms[i].centY - rooms[j].centY);
      adj[i].push_back({ j, dist });
      adj[j].push_back({ i, dist });
    }
  }

  // Step 2: Use Prim's algorithm to find the MST
  std::vector<bool> visited(n, false);
  std::priority_queue<Edge> pq;
  pq.push(Edge(-1, 0, 0.0)); // start from the first room

  while (!pq.empty())
  {
    Edge edge = pq.top();
    pq.pop();
    int u = edge.v;
    if (visited[u]) continue;
    visited[u] = true;

    // Step 3: Create a tunnel for each edge in the MST
    if (edge.u != -1)
    {
      createTunnel(rooms[edge.u], rooms[edge.v]);
    }

    for (auto& [v, w] : adj[u])
    {
      if (!visited[v])
      {
        pq.push(Edge(u, v, w));
      }
    }
  }

  // Step 4: Add some additional random edges
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, n - 1);

  int extraEdges = n * 3 / 4;
  for (int i = 0; i < extraEdges; ++i)
  {
    int u = dis(gen);
    int v = dis(gen);
    if (u != v)
    {
      // Use BFS to find the shortest path
      std::vector<bool> visited(n, false);
      std::queue<int> q;
      q.push(u);
      int pathLength = 0;

      while (!q.empty())
      {
        int current = q.front();
        q.pop();
        if (current == v) break;
        if (visited[current]) continue;
        visited[current] = true;
        pathLength++;

        for (auto& [next, w] : adj[current])
        {
          if (!visited[next])
          {
            q.push(next);
          }
        }
      }

      if (pathLength > 2)
      { // if the shortest path is longer than 1
        createTunnel(rooms[u], rooms[v]);
      }
      else
      {
        i--;
      }
    }
  }
}

void Map::createTunnel(Rect& start, Rect& fin)
{
  int ax = start.centX;
  int ay = start.centY;
  int bx = fin.centX;
  int by = fin.centY;

  // Create a horizontal tunnel from start to fin
  if (ax <= bx)
  {
    for (; ax <= bx; ++ax)
    {
      if (ax < map_w && ay < map_h)
      {
        board[ay][ax].blocking = false;
        board[ay][ax].terrain = TERRAIN::TUNNEL;
      }
    }
  }
  else
  {
    for (; bx <= ax; ++bx)
    {
      if (bx < map_w && by < map_h)
      {
        board[by][bx].blocking = false;
        board[by][bx].terrain = TERRAIN::TUNNEL;
      }
    }
  }

  // Create a vertical tunnel from start to fin
  if (ay <= by)
  {
    for (; ay <= by; ++ay)
    {
      if (ax < map_w && ay < map_h)
      {
        board[ay][ax].blocking = false;
        board[ay][ax].terrain = TERRAIN::TUNNEL;
      }
    }
  }
  else
  {
    for (; by <= ay; ++by)
    {
      if (bx < map_w && by < map_h)
      {
        board[by][bx].blocking = false;
        board[by][bx].terrain = TERRAIN::TUNNEL;
      }
    }
  }
}

void Map::render() const
{
  char terrsym{};
  for (int y = 0; y < map_h; y++)
  {
    for (int x = 0; x < map_w; x++)
    {
      switch (board[y][x].terrain)
      {
        case TERRAIN::ROCK:
          terminal_color(color_from_name("grey"));
          terrsym = '#';
          break;
        case TERRAIN::CAVE:
          terminal_color(color_from_name("darker grey"));
          terrsym = ',';
          break;
        case TERRAIN::TUNNEL:
          terminal_color(color_from_name("darker grey"));
          terrsym = ',';
          break;
        case TERRAIN::BOMBED:
          terminal_color(color_from_name("darker grey"));
          terrsym = '.';
          break;
        default:
          break;
      }
      terminal_put(x, y, terrsym);
    }
  }
}

Point Map::getStartCoords(bool isPlayer)
{
  // Point must be in a room
  Point p;

  if (isPlayer)
  {
    for (int y = 0; y < map_h; y++)
    {
      for (int x = 0; x < map_w; x++)
      {
        if (board[y][x].terrain != TERRAIN::ROCK)
        {
          p.x = x;
          p.y = y;
          return p;
        }
      }
    }
  }
  else
  {
    for (int y = map_h - 1; y > 0; y--)
    {
      for (int x = map_w - 1; x > 0; x--)
      {
        if (board[y][x].terrain != TERRAIN::ROCK)
        {
          p.x = x;
          p.y = y;
          return p;
        }
      }
    }
  }

  return p;
}

Point Map::getRandomCoords()
{
  Point p;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> randX(3, map_w - 4);
  std::uniform_int_distribution<int> randY(3, map_h - 4);
  int x = randX(gen);
  int y = randY(gen);
  while (board[y][x].terrain == TERRAIN::ROCK)
  {
    x = randX(gen);
    y = randY(gen);
  }
  p.x = x;
  p.y = y;
  return p;
}
