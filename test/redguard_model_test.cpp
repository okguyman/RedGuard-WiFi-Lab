#include <assert.h>
#include <stdio.h>
#include <initializer_list>
#include "RedGuardModel.h"
#include "RedGuardText.h"
using namespace RedGuard;
int main() {
  const char* s = "A中文\xf0\x9f\x98\x80";
  assert(nextCodepoint(s) == 'A');
  assert(nextCodepoint(s) == 0x4e2d);
  assert(nextCodepoint(s) == 0x6587);
  assert(nextCodepoint(s) == 0x1f600);
  assert(nextCodepoint(s) == 0);
  const char* invalid[] = {"\xe4\xb8", "\xf0", "\xc0\xaf", "\xed\xa0\x80", "\xf4\x90\x80\x80", "\x80"};
  for (const char* value : invalid) {
    const char* cursor = value;
    unsigned n = 0;
    while (*cursor) { assert(nextCodepoint(cursor) == '?'); assert(++n <= 4); }
  }
  assert(glyph(0x1f600).codepoint == '?');
  for (unsigned i = 0; i < unsigned(TextId::Count); ++i) {
    const char* p = text(TextId(i));
    while (*p) { auto cp = nextCodepoint(p); assert(glyph(cp).codepoint == cp); }
  }
  assert(rowRect(0).contains(10, 54));
  assert(!rowRect(0).contains(230, 54));
  assert(!rowRect(0).contains(10, 98));
  assert(!rowRect(1).contains(10, 100));
  assert(itemIndex(1, 3) == 7);
  Navigation nav;
  assert(nav.update(0, true, 100) == NavigationEvent::Up);
  assert(nav.update(0, true, 1099) == NavigationEvent::None);
  assert(nav.update(0, true, 1100) == NavigationEvent::Up);
  assert(nav.update(0, true, 1219) == NavigationEvent::None);
  assert(nav.update(0, true, 1220) == NavigationEvent::Up);
  assert(nav.update(-1, false, 1300) == NavigationEvent::None);
  assert(nav.update(1, true, 1400) == NavigationEvent::None);
  assert(nav.update(1, true, 5000) == NavigationEvent::None);
  assert(nav.update(-1, false, 5100) == NavigationEvent::Select);
  assert(nav.update(-1, false, 5200) == NavigationEvent::None);
  assert(nav.update(2, true, 5300) == NavigationEvent::Down);
  assert(nav.update(1, true, 5400) == NavigationEvent::None);
  assert(nav.update(-1, false, 5500) == NavigationEvent::Select);
  nav.reset();
  assert(nav.update(-1, false, 5600) == NavigationEvent::None);
  assert(nav.update(0, true, 0xffffff00) == NavigationEvent::Up);
  assert(nav.update(0, true, 0x300) == NavigationEvent::Up);
  puts("RedGuard UTF-8, font coverage, geometry and touch tests passed");
}
