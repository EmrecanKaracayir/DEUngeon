#pragma once

struct ComparePair
{
  bool operator()(const std::pair<Point, double>& a, const std::pair<Point, double>& b) const
  {
    return a.second > b.second;
  }
};


class AStar
{
public:
  AStar(Map& map) : m_map(map) {}
  std::vector<Point> findPath(Point start, Point end);

private:
  Map& m_map;
  std::vector<std::vector<bool>> m_visitedArr;
  std::vector<std::vector<Point>> m_cameFromArr;
  std::vector<std::vector<double>> m_gScoreArr;
  std::vector<std::vector<double>> m_fScoreArr;

  void init();
  std::vector<Point> reconstructPath(Point start, Point end);
  double heuristic(Point a, Point b);
};

void AStar::init()
{
  // Reset all data structures
  m_visitedArr.clear();
  m_cameFromArr.clear();
  m_gScoreArr.clear();
  m_fScoreArr.clear();

  // Resize all data structures to the size of the map
  m_visitedArr.resize(m_map.map_h, std::vector<bool>(m_map.map_w, false));
  m_cameFromArr.resize(m_map.map_h, std::vector<Point>(m_map.map_w));
  m_gScoreArr.resize(m_map.map_h, std::vector<double>(m_map.map_w, std::numeric_limits<double>::infinity()));
  m_fScoreArr.resize(m_map.map_h, std::vector<double>(m_map.map_w, std::numeric_limits<double>::infinity()));
}

std::vector<Point> AStar::findPath(Point start, Point end)
{
  init();
  std::priority_queue<std::pair<Point, double>, std::vector<std::pair<Point, double>>, ComparePair> queue;
  queue.push({ start, 0 });
  m_visitedArr[start.y][start.x] = true;
  m_gScoreArr[start.y][start.x] = 0;
  m_fScoreArr[start.y][start.x] = heuristic(start, end);

  while (!queue.empty())
  {
    Point current = queue.top().first;
    queue.pop();

    if (current.x == end.x && current.y == end.y)
    {
      return reconstructPath(start, end);
    }

    for (int dx = -1; dx <= 1; dx++)
    {
      for (int dy = -1; dy <= 1; dy++)
      {
        if (dx == 0 && dy == 0) continue;

        // Not allowing diagonal movement
        if (dx != 0 && dy != 0) continue;

        int newX = current.x + dx;
        int newY = current.y + dy;

        if (newX >= 0 && newX < m_map.map_w && newY >= 0 && newY < m_map.map_h)
        {
          if (!m_visitedArr[newY][newX] && !m_map.board[newY][newX].blocking)
          {
            double tentative_gScore = m_gScoreArr[current.y][current.x] + 1;
            if (tentative_gScore < m_gScoreArr[newY][newX])
            {
              m_cameFromArr[newY][newX] = current;
              m_gScoreArr[newY][newX] = tentative_gScore;
              m_fScoreArr[newY][newX] = m_gScoreArr[newY][newX] + heuristic(Point(newX, newY), end);
              if (!m_visitedArr[newY][newX])
              {
                queue.push({ Point(newX, newY), m_fScoreArr[newY][newX] });
                m_visitedArr[newY][newX] = true;
              }
            }
          }
        }
      }
    }
  }

  return {};  // return empty path if no path found
}

std::vector<Point> AStar::reconstructPath(Point start, Point end)
{
  std::vector<Point> path;
  for (Point p = end; p.x != start.x || p.y != start.y; p = m_cameFromArr[p.y][p.x])
  {
    path.push_back(p);
  }
  std::reverse(path.begin(), path.end());
  return path;
}

double AStar::heuristic(Point a, Point b)
{
  // Using Euclidean distance as heuristic
  return std::sqrt(std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2));
}
