#include "RedGuardModel.h"
namespace RedGuard {
uint32_t nextCodepoint(const char*& text) {
  const auto* p = reinterpret_cast<const unsigned char*>(text);
  if (!*p) return 0;
  const unsigned char lead = *p++;
  if (lead < 0x80) { ++text; return lead; }
  unsigned count;
  uint32_t cp, minimum;
  if (lead >= 0xc2 && lead <= 0xdf) { count = 1; cp = lead & 31; minimum = 0x80; }
  else if (lead >= 0xe0 && lead <= 0xef) { count = 2; cp = lead & 15; minimum = 0x800; }
  else if (lead >= 0xf0 && lead <= 0xf4) { count = 3; cp = lead & 7; minimum = 0x10000; }
  else { ++text; return '?'; }
  for (unsigned i = 0; i < count; ++i) {
    if ((*p & 0xc0) != 0x80) { ++text; return '?'; }
    cp = (cp << 6) | (*p++ & 63);
  }
  if (cp < minimum || cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff)) {
    ++text; return '?';
  }
  text = reinterpret_cast<const char*>(p);
  return cp;
}
}
