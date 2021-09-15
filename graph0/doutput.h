#pragma once
#include "framework.h"
#include <string>

template<typename Item>
void dout(const Item item);

template<>
void dout(const std::string str);

template<>
void dout(const char* str);

template<typename ... Args>
void DOUT(Args&& ... args);

template<typename Item>
void dout(const Item item) {
  OutputDebugStringA((" " + std::to_string(item)).c_str());
}

template<typename ... Args>
void DOUT(Args&& ... args) {
  (dout(std::forward<Args>(args)), ...);
  OutputDebugStringA("\n");
}