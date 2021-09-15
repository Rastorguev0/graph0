#include "resource.h"
#include "windows.h"
#include "gdiplus.h"
#include <cmath>

#include "painter.h"
#include "doutput.h"
#include "graph.h"

using namespace std;

void Paint::Painter::Draw(HDC hdc, int wndW, int wndH)
{
  using namespace Gdiplus;
  Graphics graphics(hdc);
  graphics.SetSmoothingMode(SmoothingModeHighSpeed);

  const SolidBrush bgBrush(settings.bg_color);
  graphics.FillRectangle(&bgBrush, 0, 0, wndW, wndH);

  if (settings.queue.size() > objects.size())
    return;
  Update(wndW, wndH);

  for (auto layer : settings.queue) {
    for (const auto& o : objects.at(layer)) {

      if (layer == Paint::Layer::ELLIPSE) {
        const auto& e = get<Paint::Ellipse>(o);

        Gdiplus::SolidBrush brush(settings.vertex_color);
        graphics.FillEllipse(&brush,
          static_cast<int>(scaleX * e.center.x) - R + paddingW,
          static_cast<int>(scaleY * e.center.y) - R + paddingH,
          2 * R, 2 * R);
      }
      else if (layer == Paint::Layer::LINE) {
        const auto& l = get<Paint::Line>(o);

        Gdiplus::Pen pen(settings.edge_color, edge_width);
        graphics.DrawLine(&pen,
          static_cast<int>(scaleX * l.from.x) + paddingW,
          static_cast<int>(scaleY * l.from.y) + paddingH,
          static_cast<int>(scaleX * l.to.x) + paddingW,
          static_cast<int>(scaleY * l.to.y) + paddingH
        );
      }
      else if (layer == Paint::Layer::TEXT) {
        const auto& t = get<Paint::Text>(o);

        const std::wstring buf = std::wstring(t.text.begin(), t.text.end());
        const WCHAR*       label = buf.c_str();
        const Font         font(settings.font, R);
        const RectF        box(
          static_cast<int>(scaleX * t.center.x) - R + paddingW,
          static_cast<int>(scaleY * t.center.y) - R + paddingH,
          2 * R, 2 * R);
        SolidBrush         brush(settings.label_color);
        StringFormat       format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);
        graphics.DrawString(label, buf.size(), &font, box, &format, &brush);
      }
      else throw 0;

    }
  }
}

void Paint::Painter::Update(int wndW, int wndH)
{
  paddingW = static_cast<int>(wndW * settings.paddingW);
  paddingH = static_cast<int>(wndH * settings.paddingH);
  double w = wndW - 2 * paddingW;
  double h = wndH - 2 * paddingH;
  scaleX = w / static_cast<double>(Paint::Graph::AREA_SIZE);
  scaleY = h / static_cast<double>(Paint::Graph::AREA_SIZE);

  R = settings.vertex_r + static_cast<int>(
    pow(wndW * wndH / 300 / (1 + 0.1 * objects[Paint::Layer::ELLIPSE].size()), 0.45)
    );
  edge_width = 2. + pow(wndW * wndH / 10000, 0.3) -
    min(2., log(objects[Paint::Layer::ELLIPSE].size()));
}

void Paint::Painter::Reset()
{
  objects.clear();
  scaleX = scaleY = edge_width = 0;
  paddingW = paddingH = R = 0;
}

Paint::Painter& Paint::Painter::AddObject(Object o)
{
  if (holds_alternative<Paint::Ellipse>(o)) {
    objects[Paint::Layer::ELLIPSE].push_back(move(o));
  }
  else if (holds_alternative<Paint::Line>(o)) {
    objects[Paint::Layer::LINE].push_back(move(o));
  }
  else if (holds_alternative<Paint::Text>(o)) {
    objects[Paint::Layer::TEXT].push_back(move(o));
  }
  else throw 0;
  return *this;
}

Paint::Painter::Settings::Settings()
  : vertex_color(255, 0, 71, 109),
    edge_color(255, 165, 52, 0),
    label_color(200, 255, 255, 255),
    bg_color(255, 0, 10, 17)
{
}