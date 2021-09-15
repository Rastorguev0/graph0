#define _USE_MATH_DEFINES
#include <cmath>
#include <string>

#include "graph.h"

using namespace std;

const double R = Paint::WND_SIZE / 2;

Math::Graph::Graph(const vector<vector<int>>& adj_matrix)
  : adj_list(adj_matrix.size())
{
  for (u_int v = 0; v < adj_matrix.size(); ++v) {
    for (u_int n = 0; n < adj_matrix[v].size(); ++n) {
      if (adj_matrix[v][n] == 1) adj_list[v].push_back(n);
    }
  }
}

Paint::Graph Math::Graph::Lay() const
{
  vector<Paint::Vertex> vertexes;
  vector<Paint::Edge> edges;

  for (u_int v = 0; v < adj_list.size(); ++v) {
    double alpha = 2 * M_PI * v / adj_list.size();
    const Paint::Point p{
      static_cast<int>(R * (cos(alpha) + 1.)),
      static_cast<int>(R * (sin(alpha) + 1.))
    };
    vertexes.emplace_back(v, p);

    for (const auto n : adj_list[v]) {
      if (v < n) {
        alpha = 2 * M_PI * n / adj_list.size();
        const Paint::Point p2{
      static_cast<int>(R * (cos(alpha) + 1.)),
      static_cast<int>(R * (sin(alpha) + 1.))
        };
        edges.emplace_back(p, p2);
      }
    }

  }
  return Paint::Graph(move(vertexes), move(edges));
}

Paint::Graph::Graph(vector<Vertex>&& vertexes, vector<Edge>&& edges)
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