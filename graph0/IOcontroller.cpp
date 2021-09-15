#include "framework.h"
#include <fstream>
#include <cassert>

#include "IOcontroller.h"

using namespace std;

vector<vector<int>> IOcontroller::ReadMatrix(const string& f_name)
{
  ifstream input(f_name);
  if (!input) throw 0;
  int v_count = 0;
  input >> v_count;

  vector<vector<int>> adj_matrix(v_count);
  for (int i = 0; i < v_count * v_count; ++i) {
    int flag;
    if (input >> flag && flag == 0 || flag == 1) {
      adj_matrix[i / v_count].push_back(flag);
    }
    else throw 0;
  }
  return adj_matrix;
}
