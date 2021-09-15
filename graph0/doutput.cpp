#include "doutput.h"

template<>
void dout(const std::string str) {
  OutputDebugStringA((" " + str).c_str());
}

template<>
void dout(const char* str) {
  OutputDebugStringA(" ");
  OutputDebugStringA(str);
}

