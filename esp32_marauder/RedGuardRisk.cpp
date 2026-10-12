#include "RedGuardRisk.h"
namespace RedGuard {
Risk assess(const Ap& ap) {
  Risk r{TextId::RiskUnknown, TextId::AuthUnknown, TextId::CipherUnknown,
         ap.wps ? TextId::WpsReview : TextId::WpsUnobserved, TextId::ReviewRouter};
  bool high = false, review = ap.wps, knownAuth = false, knownCipher = false;
  switch (ap.auth) {
    case 0: r.authentication = TextId::AuthOpen; high = true; knownAuth = true; break;
    case 1: case 2: case 4: case 16:
      r.authentication = TextId::AuthLegacy; high = true; knownAuth = true; break;
    case 3: case 5: case 6: case 7: case 9: case 10: case 14: case 15:
      r.authentication = ap.auth == 7 || ap.auth == 15 ? TextId::AuthTransition : TextId::AuthModern;
      knownAuth = true; break;
    default: break;
  }
  const auto legacy = [](uint8_t c) { return c == 1 || c == 2 || c == 3 || c == 5; };
  const auto modern = [](uint8_t c) { return c == 4 || c == 8 || c == 9; };
  if (legacy(ap.pairwise) || legacy(ap.group)) {
    r.cipher = TextId::CipherLegacy; high = true; knownCipher = true;
  } else if (ap.pairwise == 0 && ap.group == 0 && ap.auth == 0) {
    r.cipher = TextId::CipherNone; knownCipher = true;
  } else if (modern(ap.pairwise) && modern(ap.group)) {
    r.cipher = TextId::CipherModern; knownCipher = true;
  }
  r.level = high ? TextId::RiskHigh : review ? TextId::RiskReview :
    knownAuth && knownCipher ? TextId::RiskNoFinding : TextId::RiskUnknown;
  r.advice = high ? TextId::UpgradeSecurity : ap.wps ? TextId::DisableWps : TextId::ReviewRouter;
  return r;
}
}
