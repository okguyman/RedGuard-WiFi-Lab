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
  const bool pmfKnown = ap.pmf == Pmf::NotAdvertised || ap.pmf == Pmf::Capable || ap.pmf == Pmf::Required;
  const bool pmfOptional = ap.pmf == Pmf::NotAdvertised || ap.pmf == Pmf::Capable;
  review = review || pmfOptional || ap.pmf == Pmf::Invalid || ap.pmf == Pmf::Conflicting;
  r.knownChecks = uint8_t(knownAuth) + uint8_t(knownCipher) + uint8_t(ap.wps) + uint8_t(pmfKnown);
  r.level = high ? TextId::RiskHigh : review ? TextId::RiskReview :
    knownAuth && knownCipher ? TextId::RiskNoFinding : TextId::RiskUnknown;
  r.advice = high ? TextId::UpgradeSecurity : ap.wps ? TextId::DisableWps : pmfOptional ? TextId::EnablePmf : TextId::ReviewRouter;
  return r;
}
TextId pmfText(Pmf pmf) {
  switch (pmf) {
    case Pmf::CapabilitiesMissing: return TextId::PmfMissing;
    case Pmf::NotAdvertised: return TextId::PmfNotAdvertised;
    case Pmf::Capable: return TextId::PmfCapable;
    case Pmf::Required: return TextId::PmfRequired;
    case Pmf::NoRsn: return TextId::PmfNoRsn;
    case Pmf::Invalid: return TextId::PmfInvalid;
    case Pmf::Conflicting: return TextId::PmfConflicting;
    default: return TextId::UnknownPmf;
  }
}
}
