#pragma once
#include <vector>
#include <utility>

#include "painter.h"

using std::vector;

namespace Paint {

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
    Graph(vector<Vertex> vertexes, vector<Edge> edges);

    void Render(Painter& p) const;

    void Scale(double rate);

    Graph& Absorb(Graph&& absorbed, int offsetX, int offsetY);

    static Graph& Join(std::vector<Graph>&& graphs);

    void Align();

    int GetAreaSize() const;

    static const int AREA_SIZE = 1000;

  private:
    vector<Vertex> vertexes;
    vector<Edge> edges;
    int area_size = AREA_SIZE;
  };

}

namespace Math {

  class ConnectedGraph;
  class Tree;

  class Graph
  {
  public:
    Graph();
    Graph(vector<vector<u_int>> adj_list);

    static vector<vector<u_int>> AdjListFromMatrix(const vector<vector<int>>& adj_matrix);

    void ConvertOn();
    void ConvertOn(vector<u_int> c);
    void ConvertOff();

    Paint::Graph Lay() const;

    bool HasCycle() const;

    template<typename Proc>
    void DFS(u_int start, vector<bool>& visited, Proc proc) const;

  protected:
    ConnectedGraph TurnIntoConGraph(bool move);
    Tree TurnIntoTree(bool move);

  protected:
    vector<vector<u_int>> adj_list;
    //converts vertexes id
    vector<u_int> converter;
    bool convert = false;

  private:
    bool CycleDFS(u_int start, u_int parent, vector<bool>& visited) const;
  };


  class ConnectedGraph : public Graph {
  public:
    using Math::Graph::Graph;
    Paint::Graph Lay() const;
  };


  class Tree : public ConnectedGraph {
  public:
    using Math::ConnectedGraph::ConnectedGraph;

    Paint::Graph Lay() const;
    bool HasCycle() const;
    std::pair<u_int, u_int> GetCenter() const;

  private:
    u_int GetChieldCount(u_int v, u_int parent) const;
  };

}

