#include <algorithm>

#include "graph.h"

using namespace std;

Paint::Graph::Graph(vector<Vertex> vertexes, vector<Edge> edges)
  : vertexes(move(vertexes)), edges(move(edges))
{
}

void Paint::Graph::Render(Painter& p) const
{
  for (const auto& vertex : vertexes) {
    p.AddObject(
      Paint::Ellipse{ .center = vertex.p }
    ).AddObject(
      Paint::Text{ .text = to_string(vertex.label), .center = vertex.p }
    );
  }

  for (const auto& edge : edges) {
    p.AddObject(
      Paint::Line{ .from = edge.from, .to = edge.to }
    );
  }
}

void Paint::Graph::Scale(double rate)
{
  if (rate == 1.) return;
  for (auto& v : vertexes) {
    v.p.x = static_cast<int>(v.p.x * rate);
    v.p.y = static_cast<int>(v.p.y * rate);
  }
  for (auto& e : edges) {
    e.from.x = static_cast<int>(e.from.x * rate);
    e.from.y = static_cast<int>(e.from.y * rate);
    e.to.x = static_cast<int>(e.to.x * rate);
    e.to.y = static_cast<int>(e.to.y * rate);
  }
  area_size = static_cast<int>(area_size * rate);
}

Paint::Graph& Paint::Graph::Absorb(Paint::Graph&& absorbed, int offsetX, int offsetY)
{
  for (auto v = vertexes.insert(vertexes.end(),
    make_move_iterator(absorbed.vertexes.begin()),
    make_move_iterator(absorbed.vertexes.end())
  );
    v < vertexes.end();
    ++v)
  {
    v->p.x += offsetX;
    v->p.y += offsetY;
  }
  for (auto e = edges.insert(edges.end(),
    make_move_iterator(absorbed.edges.begin()),
    make_move_iterator(absorbed.edges.end())
  );
    e < edges.end();
    ++e)
  {
    e->from.x += offsetX;
    e->from.y += offsetY;
    e->to.x += offsetX;
    e->to.y += offsetY;
  }
  area_size = max(area_size, max(offsetX, offsetY) + absorbed.area_size);
  return *this;
}

Paint::Graph& Paint::Graph::Join(std::vector<Paint::Graph>&& graphs)
{
  sort(graphs.begin(), graphs.end(),
    [](const Paint::Graph& c1, const Paint::Graph& c2) {
      return c2.GetAreaSize() < c1.GetAreaSize();
    }
  );
  const int padding = Paint::Graph::AREA_SIZE / 4;

  Paint::Graph& main = graphs.front();
  for (int counter = 1; counter < graphs.size(); ++counter) {
    int size = main.GetAreaSize();
    main.Absorb(move(graphs[counter]), size + padding, size + padding);
    int corner_area_size = main.GetAreaSize() - size - padding;
    if (++counter < graphs.size()) {
      auto& absorbed = graphs[counter];
      main.Absorb(move(absorbed),
        size + padding + (corner_area_size - absorbed.GetAreaSize()) / 2,
        (size - absorbed.GetAreaSize()) / 2
      );

      if (++counter < graphs.size()) {
        auto& absorbed = graphs[counter];
        main.Absorb(move(absorbed),
          (size - absorbed.GetAreaSize()) / 2,
          size + padding + (corner_area_size - absorbed.GetAreaSize()) / 2
        );
      }
    }
  }
  main.Scale(Paint::Graph::AREA_SIZE / static_cast<double>(main.area_size));
  return main;
}

void Paint::Graph::Align()
{

}

int Paint::Graph::GetAreaSize() const
{
  return area_size;
}