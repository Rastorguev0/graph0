#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <algorithm>
#include <optional>
#include <queue>

#include "graph.h"

using namespace std;

/* Math::Graph */

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
      DFS(v, visited, [&ccv](u_int v) { ccv.push_back(v); });

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
      Math::Graph g(move(adj));
      g.ConvertOn(move(ccv));
      if (g.HasCycle()) {
        Math::ConnectedGraph cg = g.TurnIntoConGraph(true);
        ccs.emplace_back(cg.Lay());
      }
      else {
        Math::Tree t = g.TurnIntoTree(true);
        ccs.emplace_back(t.Lay());
      }

      //scale cc by vertex count
      ccs.back().Scale(
        sqrt(cc_vertexes / static_cast<double>(adj_list.size()))
      );
    }
  }

  //join connectivity components
  return Paint::Graph::Join(move(ccs));
}

template<typename Proc>
void Math::Graph::DFS(u_int start, vector<bool>& visited, Proc proc) const
{
  visited[start] = true;
  proc(start);
  for (u_int n : adj_list[start]) {
    if (!visited[n]) {
      DFS(n, visited, proc);
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

bool Math::Graph::HasCycle() const
{
  vector<bool> visited(adj_list.size(), false);
  for (int v = 0; v < adj_list.size(); ++v) {
    if (!visited[v] && CycleDFS(0, -1, visited))
      return true;
  }
  return false;
}

bool Math::Graph::CycleDFS(u_int start, u_int parent, vector<bool>& visited) const
{
  visited[start] = true;
  for (u_int n : adj_list[start]) {
    if (visited[n] && n != parent) {
      return true;
    }
    else if (!visited[n] && CycleDFS(n, start, visited)) {
      return true;
    }
  }
  return false;
}

Math::ConnectedGraph Math::Graph::TurnIntoConGraph(bool move)
{
  ConnectedGraph result;
  if (move) {
    result = ConnectedGraph(std::move(adj_list));
    result.ConvertOn(std::move(converter));
    if (!convert) result.ConvertOff();
  }
  else {
    result = ConnectedGraph(adj_list);
    result.ConvertOn(converter);
    if (!convert) result.ConvertOff();
  }
  return result;
}

Math::Tree Math::Graph::TurnIntoTree(bool move)
{
  Tree result;
  if (move) {
    result = Tree(std::move(adj_list));
    result.ConvertOn(std::move(converter));
    if (!convert) result.ConvertOff();
  }
  else {
    result = Tree(adj_list);
    result.ConvertOn(converter);
    if (!convert) result.ConvertOff();
  }
  return result;
}


/* Math::ConnectedGraph */


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
    const int v_id = convert ? converter[v] : v;
    vertexes.emplace_back(v_id, p);

    for (const auto n : adj_list[v]) {
      if (v < n) {
        alpha = 2 * M_PI * n / adj_list.size();
        const Paint::Point p2{
      static_cast<int>(R * (cos(alpha) + 1.)),
      static_cast<int>(R * (sin(alpha) + 1.))
        };
        const int n_id = convert ? converter[n] : n;
        edges.emplace_back(v_id, n_id);
      }
    }

  }
  return Paint::Graph(vertexes, move(edges));
}


/* Math::Tree */


Paint::Graph Math::Tree::Lay() const
{
  vector<Paint::Vertex> vertexes;
  vector<Paint::Edge> edges;

  const auto [C, R] = GetCenter();

  //for every vertex keep number of it's child vertexes'
  //for central vertex it's all vertex count - 1
  vector<u_int> ch_count(adj_list.size());
  ch_count[C] = adj_list.size() - 1;
  //for every vertex keep it's depth
  //for central vertex it's 0
  vector<u_int> depth(adj_list.size());
  depth[C] = 0;
  //for every vertex keep it's outcoming angle sector for child vertexes
  //for central vertex it's sector is [0; 2 * M_PI]
  vector<pair<double, double>> sectors(adj_list.size());
  sectors[C] = { 0, 2 * M_PI };
  //for every vertex keep it's coordinates
  //for central vertex it's {AREA_SIZE / 2; AREA_SIZE / 2}
  vector<Paint::Point> points(adj_list.size());
  points[C] = Paint::Point{ Paint::Graph::AREA_SIZE / 2, Paint::Graph::AREA_SIZE / 2 };
  //lay the central vertex
  vertexes.emplace_back(convert ? converter[C] : C, points[C]);

  //nextly use BFS to lay the tree radialy
  vector<bool> visited(adj_list.size(), false);
  queue<u_int> q;
  q.push(C);
  while (!q.empty()) {
    u_int u = q.front();
    q.pop();
    visited[u] = true;
    double sector_begin = sectors[u].first;
    for (u_int n : adj_list[u]) {
      if (!visited[n]) {
        q.push(n);
        const auto [ps_begin, ps_end] = sectors[u];
        ch_count[n] = GetChieldCount(n, u);
        double alpha = (ps_end - ps_begin) * (1 + ch_count[n]) / static_cast<double>(ch_count[u]);
        sectors[n] = { sector_begin, sector_begin + alpha };
        sector_begin += alpha;

        depth[n] = depth[u] + 1u;
        double r = depth[n] * Paint::Graph::AREA_SIZE / R / 2.;
        points[n] = Paint::Point{
        static_cast<int>(r * cos(sectors[n].first + alpha / 2) + Paint::Graph::AREA_SIZE / 2),
        static_cast<int>(r * sin(sectors[n].first + alpha / 2) + Paint::Graph::AREA_SIZE / 2)
        };
        const int u_id = convert ? converter[u] : u;
        const int n_id = convert ? converter[n] : n;
        vertexes.emplace_back(n_id, points[n]);
        edges.emplace_back(u_id, n_id);
      }
    }
  }

  return Paint::Graph(vertexes, move(edges));
}

bool Math::Tree::HasCycle() const
{
  return false;
}

std::pair<u_int, u_int> Math::Tree::GetCenter() const
{
  if (adj_list.empty())
    return make_pair(0, 0);

  vector<vector<int>> distances(
    adj_list.size(), vector<int>(adj_list.size(), -1)
  );
  //using BFS for every vertex to get distance
  for (int v = 0; v < distances.size(); ++v) {
    distances[v][v] = 0;
    queue<u_int> q;
    vector<bool> visietd(distances.size(), false);
    q.push(v);
    while (!q.empty()) {
      u_int u = q.front();
      q.pop();
      visietd[u] = true;
      for (u_int n : adj_list[u]) {
        if (!visietd[n]) {
          q.push(n);
          distances[v][n] = distances[v][u] + 1;
        }
      }
    }
  }

  //counting the eccentricity of the vertices
  vector<int> e(distances.size());
  for (int v = 0; v < distances.size(); ++v) {
    e[v] = *max_element(distances[v].begin(), distances[v].end());
  }

  auto v = min_element(e.begin(), e.end());
  return make_pair(distance(e.begin(), v), *v);
}

u_int Math::Tree::GetChieldCount(u_int v, u_int parent) const
{
  vector<bool> visited(adj_list.size(), false);
  visited[parent] = true;
  int counter = -1;
  DFS(v, visited, [&counter](u_int _) { counter++; });
  return static_cast<u_int>(counter);
}
