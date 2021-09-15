#pragma once
#include <vector>
#include <string>

class IOcontroller {
public:
  static std::vector<std::vector<int>> ReadMatrix(const std::string& f_name);
};