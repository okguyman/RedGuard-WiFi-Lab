#include <assert.h>
#include <stdio.h>
#include <initializer_list>
#include "RedGuardRisk.h"
using namespace RedGuard;
int main() {
  Ap ap;
  assert(assess(ap).level == TextId::RiskUnknown);
  ap.auth=3; ap.pairwise=4; ap.group=4;
  assert(assess(ap).level == TextId::RiskNoFinding);
  ap.wps=true;
  assert(assess(ap).level == TextId::RiskReview && assess(ap).advice == TextId::DisableWps);
  ap.wps=false; ap.group=3;
  assert(assess(ap).level == TextId::RiskHigh && assess(ap).cipher == TextId::CipherLegacy);
  ap.group=4;
  for (unsigned auth : {0u,1u,2u,4u,16u}) {
    ap.auth=auth; assert(assess(ap).level == TextId::RiskHigh);
  }
  ap.auth=9;
  assert(assess(ap).level == TextId::RiskNoFinding); // OWE isn't an unencrypted open AP.
  ap.auth=7;
  assert(assess(ap).authentication == TextId::AuthTransition);
  ap.pairwise=255;
  assert(assess(ap).level == TextId::RiskUnknown);
  ap.pairwise=4;
  for (unsigned auth : {8u,11u,12u,13u,17u,255u}) {
    ap.auth=auth; assert(assess(ap).level == TextId::RiskUnknown);
  }
  ap.auth=0; ap.pairwise=0; ap.group=0;
  assert(assess(ap).cipher == TextId::CipherNone && assess(ap).level == TextId::RiskHigh);
  ap.auth=3;
  assert(assess(ap).level == TextId::RiskUnknown); // Inconsistent protected/none isn't declared safe.
  ap.auth=3;ap.pairwise=4;ap.group=4;ap.wps=false;
  assert(assess(ap).knownChecks==2);
  ap.pmf=Pmf::Required;
  assert(assess(ap).knownChecks==3 && assess(ap).level==TextId::RiskNoFinding);
  ap.pmf=Pmf::Capable;
  assert(assess(ap).level==TextId::RiskReview && assess(ap).advice==TextId::EnablePmf);
  ap.pmf=Pmf::NotAdvertised;
  assert(assess(ap).level==TextId::RiskReview);
  ap.pmf=Pmf::Conflicting;
  assert(assess(ap).level==TextId::RiskReview && assess(ap).knownChecks==2);
  ap.pmf=Pmf::Invalid;ap.wps=true;
  assert(assess(ap).knownChecks==3 && assess(ap).advice==TextId::DisableWps);
  puts("Risk evidence, legacy group cipher, WPS and unknown handling passed");
}
