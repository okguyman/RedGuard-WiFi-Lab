#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "RedGuardScanModel.h"
using namespace RedGuard;
Ap ap(unsigned id, int rssi, unsigned channel = 6) {
  Ap value;
  value.bssid[4] = id >> 8; value.bssid[5] = id & 255;
  value.rssi = rssi; value.channel = channel;
  snprintf(value.ssid,sizeof(value.ssid),"AP-%u",id);
  return value;
}
int main() {
  Snapshot s;
  assert(!s.at(0));
  s.add(ap(1,-80)); s.add(ap(2,-40)); s.add(ap(3,-60,11));
  assert(s.size()==3 && s.at(0)->bssid[5]==2 && s.at(2)->bssid[5]==1);
  s.add(ap(1,-30,1));
  assert(s.size()==3 && s.at(0)->bssid[5]==1);
  assert(s.channelCount(6)==1 && s.channelCount(1)==1 && s.channelCount(11)==1);
  s.add(ap(99,-10,0)); s.add(ap(99,-10,15)); assert(s.size()==3);
  s.clear();assert(s.size()==0 && s.channelCount(6)==0 && !s.at(0));
  for (unsigned i=0;i<100;++i) s.add(ap(i,-100+int(i)));
  assert(s.size()==64 && s.at(0)->rssi==-1 && s.at(63)->rssi==-64);
  s.add(ap(999,-127)); assert(s.at(63)->rssi==-64);
  s.add(ap(999,0)); assert(s.at(0)->rssi==0 && !s.at(64));
  s.clear();s.add(ap(9,-50));s.add(ap(3,-50));assert(s.at(0)->bssid[5]==3);
  Ap nonterminated=ap(1,-20);memset(nonterminated.ssid,'A',sizeof(nonterminated.ssid));
  s.add(nonterminated);assert(strlen(s.at(0)->ssid)==32);
  assert(signalPercent(-127)==0 && signalPercent(-100)==0 && signalPercent(-75)==50);
  assert(signalPercent(-50)==100 && signalPercent(3)==100);
  puts("RedGuard scan deduplication, bounded top-64, sorting, channels and signal tests passed");
}
