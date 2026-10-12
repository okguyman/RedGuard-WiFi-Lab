#pragma once
#include <stdint.h>
#include <stddef.h>
#include "MenuInputRepeat.h"

namespace RedGuard {
// 纯逻辑与硬件解耦：UTF-8 边界、分页、触摸释放均可在 Mac 测试。
uint32_t nextCodepoint(const char*& text);
struct Rect {
  int x, y, w, h;
  bool contains(int px, int py) const {
    return px >= x && py >= y && px < x + w && py < y + h;
  }
};
constexpr Rect rowRect(unsigned row) { return {10, 54 + int(row) * 48, 220, 44}; }
constexpr Rect bottomRect{10, 290, 220, 26};
constexpr unsigned itemIndex(unsigned page, unsigned row) { return page * 4 + row; }

enum class NavigationEvent : int8_t { None = -1, Up = 0, Select = 1, Down = 2 };
class Navigation {
 public:
  // held 使用原版 menuButton() 的区域编号，完全复用其重复和释放规则。
  NavigationEvent update(int8_t held, bool pressed, uint32_t now) {
    int8_t released = menuTouchReleasedButton(previous_, held, pressed);
    previous_ = held;
    bool up = up_.update(held == 0, now);
    bool down = down_.update(held == 2, now);
    return up ? NavigationEvent::Up : down ? NavigationEvent::Down :
      released == 1 ? NavigationEvent::Select : NavigationEvent::None;
  }
  void reset() { previous_ = -1; up_.reset(); down_.reset(); }
 private:
  int8_t previous_ = -1;
  MenuInputRepeat up_, down_;
};
}
