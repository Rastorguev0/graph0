#pragma once
#include <vector>

#include "painter.h"

namespace Paint {

  const int WND_SIZE = 1000;

  struct Vertex {
    int label = 0;
    Point p;
  };

  struct Edge {
    Point from;
    Point to;
  };

  class Graph
  {
  public:
    Graph(std::vector<Vertex>&& vertexes, std::vector<Edge>&& edges);

    void Render(Painter& p) const;

  private:
    std::vector<Vertex> vertexes;
    std::vector<Edge> edges;
  };

}

namespace Math {

  class Graph
  {
  public:
    Graph(const std::vector<std::vector<int>>& adj_matrix);

    Paint::Graph Lay() const;

  private:
    std::vector<std::vector<u_int>> adj_list;
  };

}

