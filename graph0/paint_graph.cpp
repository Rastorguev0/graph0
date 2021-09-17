#include <algorithm>
#include <list>

#include "graph.h"

using namespace std;

Paint::Graph::Graph(const vector<Vertex>& vs, vector<Edge> edges)
  : edges(move(edges))
{
  for (const auto& v : vs) {
    vertexes[v.id] = v;
  }
}

void Paint::Graph::Render(Painter& p) const
{
  for (const auto& [id, vertex] : vertexes) {
    p.AddObject(
      Paint::Ellipse{ .center = vertex.p }
    ).AddObject(
      Paint::Text{ .text = to_string(id), .center = vertex.p }
    );
  }

  for (const auto& edge : edges) {
    p.AddObject(
      Paint::Line{ .from = vertexes.at(edge.from).p, .to = vertexes.at(edge.to).p }
    );
  }
}

void Paint::Graph::Scale(double rate)
{
  if (rate == 1.) return;
  for (auto& [id, v] : vertexes) {
    v.p.x = static_cast<int>(v.p.x * rate);
    v.p.y = static_cast<int>(v.p.y * rate);
  }
  areaW = static_cast<int>(areaW * rate);
  areaH = static_cast<int>(areaH * rate);
}

void Paint::Graph::ScaleX(double rate)
{
  if (rate == 1.) return;
  for (auto& [id, v] : vertexes) {
    v.p.x = static_cast<int>(v.p.x * rate);
  }
  areaW = static_cast<int>(areaW * rate);
}

void Paint::Graph::ScaleY(double rate)
{
  if (rate == 1.) return;
  for (auto& [id, v] : vertexes) {
    v.p.y = static_cast<int>(v.p.y * rate);
  }
  areaH = static_cast<int>(areaH * rate);
}

Paint::Graph& Paint::Graph::Absorb(Paint::Graph&& absorbed, int offsetX, int offsetY)
{
  for (auto& [id, v] : absorbed.vertexes) {
    v.p.x += offsetX;
    v.p.y += offsetY;
  }
  vertexes.merge(move(absorbed.vertexes));
  edges.insert(edges.end(),
    make_move_iterator(absorbed.edges.begin()),
    make_move_iterator(absorbed.edges.end())
  );

  areaW = max(areaW, offsetX + absorbed.areaW);
  areaH = max(areaH, offsetX + absorbed.areaH);
  return *this;
}

struct Rect {
  int left;
  int top;
  int right;
  int bottom;
  bool Fits(int w, int h) const {
    return (w <= right - left) && (h <= top - bottom);
  }
  bool Intersect(const Rect& other) const {
    return !(right <= other.left || other.right <= left || top <= other.bottom || other.top <= bottom);
  }
};

void UpdateAreas(list<Rect>& areas, const Rect& added) {
  for (auto& area : areas) {
    if (area.Intersect(added)) {
      if (area.left < added.left) {
        area.right = added.left;
      }
      if (area.bottom < added.bottom) {
        area.top = added.bottom;
      }
      //area.left < added.left && area.bottom < added.bottom ???
    }
  }
}

Paint::Graph& Paint::Graph::Join(std::vector<Paint::Graph>&& graphs)
{
  int TOTAL_AREA = 0;
  for (auto& graph : graphs) {
    graph.Cut();
    TOTAL_AREA += graph.GetArea();
  }
  const int padding =
    pow(Paint::Graph::AREA_SIZE, 2) / static_cast<double>(TOTAL_AREA)
    * 100 / static_cast<double>(graphs.size());

  //sort by descending order of area
  sort(graphs.begin(), graphs.end(),
    [](const Paint::Graph& c1, const Paint::Graph& c2) {
      return c2.GetArea() < c1.GetArea();
    }
  );

  const int     INF = TOTAL_AREA;
  list<Rect>  available_areas;
  list<Rect>  added_areas;
  Paint::Graph& main = graphs.front();
  available_areas.push_back({ 0, INF, INF, main.areaH });
  available_areas.push_back({ main.areaW, INF, INF, 0 });
  added_areas.push_back({ 0, main.areaH, main.areaW, 0 });

  for (int n = 1; n < graphs.size(); ++n) {
    auto& graph = graphs[n];
    //necessary place
    int w = graph.areaW + 2 * padding;
    int h = graph.areaH + 2 * padding;
    //need to minimize elongation
    int elong = INF;
    auto area_it = available_areas.begin();
    bool rotated = false;
    for (auto area = available_areas.begin(); area != available_areas.end(); ++area) {
      //rotated graph
      if (area->Fits(h, w)) {
        int new_elong = abs(
          max(main.areaW, area->left + h) - max(main.areaH, area->bottom + w)
        );
        if (new_elong <= elong) {
          elong = new_elong;
          area_it = area;
          rotated = true;
        }
      }
      //default graph
      if (area->Fits(w, h)) {
        int new_elong = abs(
          max(main.areaW, area->left + w) - max(main.areaH, area->bottom + h)
        );
        if (new_elong <= elong) {
          elong = new_elong;
          area_it = area;
          rotated = false;
        }
      }
    }
    //now we know how to add graph to main
    if (rotated) {
      graph.Rotate();
      swap(w, h);
    }
    main.Absorb(
      move(graph), area_it->left + padding, area_it->bottom + padding
    );
    Rect added{ area_it->left, area_it->bottom + h, area_it->left + w, area_it->bottom };
    added_areas.push_back(added);

    //then we need to update available areas
    available_areas.erase(area_it);
    UpdateAreas(available_areas, added);
    Rect lu{ added.left, INF, INF, added.top };
    Rect rd{ added.right, INF, INF, added.bottom };
    list<Rect> new_areas;
    new_areas.push_back(move(lu));
    new_areas.push_back(move(rd));
    for (const auto& added : added_areas) {
      UpdateAreas(new_areas, added);
    }
    available_areas.insert(available_areas.end(), begin(new_areas), end(new_areas));
  }

  main.Cut();
  main.ScaleX(Paint::Graph::AREA_SIZE / static_cast<double>(main.areaW));
  main.ScaleY(Paint::Graph::AREA_SIZE / static_cast<double>(main.areaH));
  return main;
}

void Paint::Graph::Cut()
{
  const int min_size = max(1, static_cast<int>(
    1.5 * pow(areaH * areaW / 300 / (1 + 0.3 * vertexes.size()), 0.45)
  ));
  int minX = areaW;
  int maxX = 0;
  int minY = areaH;
  int maxY = 0;
  for (const auto& [id, v] : vertexes) {
    minX = min(minX, v.p.x);
    maxX = max(maxX, v.p.x);
    minY = min(minY, v.p.y);
    maxY = max(maxY, v.p.y);
  }
  for (auto& [id, v] : vertexes) {
    v.p.x = minX == maxX ? min_size / 2 : v.p.x - minX;
    v.p.y = minY == maxY ? min_size / 2 : v.p.y - minY;
  }
  areaW = minX == maxX ? min_size : maxX - minX;
  areaH = minY == maxY ? min_size : maxY - minY;
}

void Paint::Graph::Rotate()
{
  for (auto& [id, v] : vertexes) {
    swap(v.p.x, v.p.y);
  }
}

int Paint::Graph::GetArea() const
{
  return areaW * areaH;
}

int Paint::Graph::GetAreaW() const
{
  return areaW;
}

int Paint::Graph::GetAreaH() const
{
  return areaH;
}
