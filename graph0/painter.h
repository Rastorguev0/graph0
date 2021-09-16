#pragma once
#include "windows.h"
#include "gdiplus.h"

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <deque>

namespace Paint {

  /* the coordinates of the following structures are notional,
  that is, they are the coordinates of the
  notional window 1000x1000 */

  struct Point {
    int x = 0;
    int y = 0;
  };

  struct Ellipse {
    Point center;
  };

  struct Line{
    Point from;
    Point to;
  };

  struct Text {
    std::string text;
    Point center;
  };

  using Object = std::variant<Ellipse, Line, Text>;

  enum class Layer {
    LINE, ELLIPSE, TEXT,
  };


  class Painter
  {
  public:
    void Draw(HDC hdc, int wndW, int wndH);
    void Update(int wndW, int wndH);
    void Reset();

    Painter& AddObject(Object o);
  private:

    struct Settings {
      Settings();

      std::vector<Layer> queue = { Layer::LINE, Layer::ELLIPSE, Layer::TEXT };
      double paddingW = 1 / 8.;
      double paddingH = 1 / 8.;
      Gdiplus::Color vertex_color;
      Gdiplus::Color edge_color;
      Gdiplus::Color label_color;
      Gdiplus::Color bg_color;
      int vertex_r = 4;
      const WCHAR* font = L"Arial";
    };
    const Settings settings;

  private:
    std::map<Layer, std::vector<Object>> objects;
    double scaleX = 0;
    double scaleY = 0;
    int paddingW = 0;
    int paddingH = 0;
    int R = 0;
    double edge_width = 0;
  };

}