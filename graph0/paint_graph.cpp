#include <algorithm>

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

Paint::Graph& Paint::Graph::Join(std::vector<Paint::Graph>&& graphs)
{
  for (auto& g : graphs) {
    g.Cut();
  }
  //TODO REWORK
  sort(graphs.begin(), graphs.end(),
    [](const Paint::Graph& c1, const Paint::Graph& c2) {
      return c2.GetAreaSize() < c1.GetAreaSize();
    }
  );
  const int padding = static_cast<int>(Paint::Graph::AREA_SIZE / pow(graphs.size(), 0.5));

  Paint::Graph& main = graphs.front();
  for (int counter = 1; counter < graphs.size(); ++counter) {
    int size = main.GetAreaH();
    main.Absorb(move(graphs[counter]), size + padding, size + padding);
    int corner_area_size = main.GetAreaH() - size - padding;
    if (++counter < graphs.size()) {
      auto& absorbed = graphs[counter];
      main.Absorb(move(absorbed),
        size + padding + (corner_area_size - absorbed.GetAreaH()) / 2,
        (size - absorbed.GetAreaH()) / 2
      );

      if (++counter < graphs.size()) {
        auto& absorbed = graphs[counter];
        main.Absorb(move(absorbed),
          (size - absorbed.GetAreaH()) / 2,
          size + padding + (corner_area_size - absorbed.GetAreaH()) / 2
        );
      }
    }
  }
  //not actually areaW
  main.Scale(Paint::Graph::AREA_SIZE / static_cast<double>(main.areaW));
  return main;
}

void Paint::Graph::Cut()
{
  //TODO cut can make zero areaH or areaW. need to improve it
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
    v.p.x -= minX;
    v.p.y -= minY;
  }
  areaW = maxX - minX;
  areaH = maxY - minY;
}

int Paint::Graph::GetAreaSize() const
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
