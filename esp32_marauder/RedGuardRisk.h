#pragma once
#include "RedGuardScanModel.h"
#include "RedGuardText.h"
namespace RedGuard {
// SDK enum values are checked against the pinned headers in RedGuardScan.cpp.
struct Risk {
  TextId level;
  TextId authentication;
  TextId cipher;
  TextId wps;
  TextId advice;
  uint8_t knownChecks = 0;
};
Risk assess(const Ap& ap);
TextId pmfText(Pmf pmf);
}
