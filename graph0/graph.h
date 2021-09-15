#pragma once
#include <vector>

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
    Graph(vector<Vertex>&& vertexes, vector<Edge>&& edges);

    void Render(Painter& p) const;

    void Scale(double rate);

    //moves data to main graph
    //WARNING: vertex and edges data will be lost
    Graph& JoinTo(Graph& main, int offsetX, int offsetY);

    static Graph& Join(std::vector<Graph>& graphs);

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

  protected:

    //connectivity components by DFS
    void CC_DFS(u_int start, vector<bool>& visited, vector<u_int>& cc_vertexes) const;

  protected:
    vector<vector<u_int>> adj_list;
    //converts vertexes id
    vector<u_int> converter;
    bool convert = false;
  };


  class ConnectedGraph : public Graph {
  public:
    using Math::Graph::Graph;
    Paint::Graph Lay() const;
  };

}

