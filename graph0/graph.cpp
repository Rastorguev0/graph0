#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <algorithm>

#include "graph.h"

using namespace std;

Math::Graph::Graph()
{
}

Math::Graph::Graph(vector<vector<u_int>> adj_list)
  : adj_list(move(adj_list))
{
}

vector<vector<u_int>> Math::Graph::AdjListFromMatrix(const vector<vector<int>>& adj_matrix)
{
  vector<vector<u_int>> result(adj_matrix.size());
  for (u_int v = 0; v < adj_matrix.size(); ++v) {
    for (u_int n = 0; n < adj_matrix[v].size(); ++n) {
      if (adj_matrix[v][n] == 1) result[v].push_back(n);
    }
  }
  return result;
}

Paint::Graph Math::Graph::Lay() const
{
  vector<Paint::Graph> ccs;
  //run DFS to get connectivity components (ccs)
  vector<bool> visited(adj_list.size(), false);
  for (u_int v = 0; v < adj_list.size(); ++v) {
    if (!visited[v]) {
      vector<u_int> ccv;
      CC_DFS(v, visited, ccv);

      //nextly create cc with converter
      sort(ccv.begin(), ccv.end());
      vector<vector<u_int>> adj(ccv.size());
      for (u_int v = 0; v < adj.size(); ++v) {
        for (u_int n : adj_list[ccv[v]]) {
          adj[v].push_back(
            distance(ccv.begin(), lower_bound(ccv.begin(), ccv.end(), n))
          );
        }
      }
      
      const size_t cc_vertexes = ccv.size();
      Math::ConnectedGraph cg(adj);
      cg.ConvertOn(move(ccv));
      //laying the cc graph and scale it by vertex count
      ccs.emplace_back(cg.Lay());
      ccs.back().Scale(
        sqrt(cc_vertexes / static_cast<double>(adj_list.size()))
      );
    }
  }

  //join connectivity components
  return Paint::Graph::Join(ccs);
}

void Math::Graph::CC_DFS(u_int start, vector<bool>& visited, vector<u_int>& cc_vertexes) const
{
  visited[start] = true;
  cc_vertexes.push_back(start);
  for (u_int n : adj_list[start]) {
    if (!visited[n]) {
      CC_DFS(n, visited, cc_vertexes);
    }
  }
}

void Math::Graph::ConvertOn()
{
  convert = true;
}

void Math::Graph::ConvertOn(vector<u_int> c)
{
  converter = move(c);
  convert = true;
}

void Math::Graph::ConvertOff()
{
  convert = false;
}

Paint::Graph Math::ConnectedGraph::Lay() const
{
  vector<Paint::Vertex> vertexes;
  vector<Paint::Edge> edges;
  const int R = Paint::Graph::AREA_SIZE / 2;

  for (u_int v = 0; v < adj_list.size(); ++v) {
    double alpha = 2 * M_PI * v / adj_list.size();
    const Paint::Point p{
      static_cast<int>(R * (cos(alpha) + 1.)),
      static_cast<int>(R * (sin(alpha) + 1.))
    };
    vertexes.emplace_back(convert ? converter[v] : v, p);

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

void Paint::Graph::Scale(double rate)
{
  if (rate == 1.) return;
  for (auto& v : vertexes) {
    v.p.x *= rate;
    v.p.y *= rate;
  }
  for (auto& e : edges) {
    e.from.x *= rate;
    e.from.y *= rate;
    e.to.x *= rate;
    e.to.y *= rate;
  }
  area_size = static_cast<int>(area_size * rate);
}

Paint::Graph& Paint::Graph::JoinTo(Paint::Graph& main, int offsetX, int offsetY)
{
  for (auto v = main.vertexes.insert(main.vertexes.end(),
    make_move_iterator(vertexes.begin()), make_move_iterator(vertexes.end())
  );
    v < main.vertexes.end();
    ++v)
  {
    v->p.x += offsetX;
    v->p.y += offsetY;
  }
  for (auto e = main.edges.insert(main.edges.end(),
    make_move_iterator(edges.begin()), make_move_iterator(edges.end())
  );
    e < main.edges.end();
    ++e)
  {
    e->from.x += offsetX;
    e->from.y += offsetY;
    e->to.x += offsetX;
    e->to.y += offsetY;
  }
  main.area_size = max(main.area_size, max(offsetX, offsetY) + area_size);
  vertexes.clear();
  edges.clear();
  return main;
}

Paint::Graph& Paint::Graph::Join(std::vector<Paint::Graph>& graphs)
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
    graphs[counter].JoinTo(main, size + padding, size + padding);
    int corner_area_size = main.GetAreaSize() - size - padding;
    if (++counter < graphs.size()) {
      auto& joined = graphs[counter];
      joined.JoinTo(main,
        size + padding + (corner_area_size - joined.GetAreaSize()) / 2,
        (size - joined.GetAreaSize()) / 2
      );

      if (++counter < graphs.size()) {
        auto& joined = graphs[counter];
        joined.JoinTo(main,
          (size - joined.GetAreaSize()) / 2,
          size + padding + (corner_area_size - joined.GetAreaSize()) / 2
        );
      }
    }
  }
  main.Scale(Paint::Graph::AREA_SIZE / static_cast<double>(main.area_size));
  return main;
}

int Paint::Graph::GetAreaSize() const
{
  return area_size;
}