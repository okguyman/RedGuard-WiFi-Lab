#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <utility>
#include <sys/mman.h>
#include <unistd.h>
#include "RedGuardPmf.h"
using namespace RedGuard;
using Bytes = std::vector<uint8_t>;
Bytes beacon(int capabilities = 0xc0) {
  Bytes b(36,0);
  b[0]=0x80;
  for (unsigned i=0;i<6;++i) {b[10+i]=uint8_t(2+i*2);b[16+i]=b[10+i];}
  const Bytes rsn{48,20,1,0,0,15,172,4,1,0,0,15,172,4,1,0,0,15,172,2,uint8_t(capabilities),0};
  b.insert(b.end(),rsn.begin(),rsn.end());
  return b;
}
int main() {
  PmfObservation result;
  auto b=beacon();
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Observed && result.pmf==Pmf::Required);
  assert(!memcmp(result.bssid,b.data()+16,6));
  for (auto pair : {std::pair<int,Pmf>{0,Pmf::NotAdvertised},{0x80,Pmf::Capable},{0x40,Pmf::Invalid}}) {
    b=beacon(pair.first);
    assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Observed && result.pmf==pair.second);
  }
  b[0]=0x50;
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Observed);
  b=beacon();
  for (size_t length=0;length<b.size();++length) {
    const auto status=parsePmfAdvertisement(b.data(),length,result);
    if (status==AdvertisementStatus::Observed) assert(result.pmf!=Pmf::Required);
    else assert(result.pmf==Pmf::Unknown);
  }
  // Valid explicit prefix without optional capabilities stays distinct from no RSNE.
  b.resize(b.size()-2);b[37]-=2;
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Observed && result.pmf==Pmf::CapabilitiesMissing);
  b.resize(36);
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Observed && result.pmf==Pmf::NoRsn);
  // Unexpected trailing IE corruption invalidates the entire observation.
  b=beacon();b.push_back(1);
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Malformed);
  b=beacon();{const Bytes duplicate(b.begin()+36,b.end());b.insert(b.end(),duplicate.begin(),duplicate.end());}
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Malformed);
  b=beacon();b[44]=255;b[45]=255; // Pairwise count cannot fit the enclosing IE.
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Malformed);
  b=beacon();b[38]=2; // Unsupported RSN version.
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Malformed);
  b=beacon();b.push_back(0);b.push_back(0);b[37]+=2; // Empty PMKID list.
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Observed && result.pmf==Pmf::Required);
  const Bytes management{0,15,172,6};b.insert(b.end(),management.begin(),management.end());b[37]+=4;
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Observed);
  b=beacon();b.push_back(1);b.push_back(0);b[37]+=2; // Declares a PMKID but no room for it.
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Malformed);
  for (uint8_t flags : {1,2,4,64,128}) {
    b=beacon();b[1]=flags;
    assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Malformed);
  }
  b=beacon();b[22]=1;
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Malformed);
  b=beacon();b[16]|=1;
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Malformed);
  b=beacon();b[0]=0x08;
  assert(parsePmfAdvertisement(b.data(),b.size(),result)==AdvertisementStatus::Ignored);
  b=beacon();b.insert(b.end(),{0xde,0xad,0xbe,0xef});
  assert(parsePmfEsp32Packet(b.data(),b.size(),true,result)==AdvertisementStatus::Observed && result.pmf==Pmf::Required);
  assert(parsePmfEsp32Packet(b.data(),b.size(),false,result)==AdvertisementStatus::Ignored);
  assert(parsePmfEsp32Packet(nullptr,3,true,result)==AdvertisementStatus::Ignored);
  assert(parsePmfEsp32Packet(nullptr,4096,true,result)==AdvertisementStatus::Ignored);
  // Put every truncated prefix directly against an unreadable page.
  const size_t pageSize = size_t(sysconf(_SC_PAGESIZE));
  void* pages = mmap(nullptr,pageSize*2,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);
  assert(pages!=MAP_FAILED);
  auto* boundary = static_cast<uint8_t*>(pages)+pageSize;
  assert(mprotect(boundary,pageSize,PROT_NONE)==0);
  b=beacon();
  for (size_t length=0;length<=b.size();++length) {
    auto* input=boundary-length;
    if (length) memcpy(input,b.data(),length);
    parsePmfAdvertisement(input,length,result);
  }
  assert(munmap(pages,pageSize*2)==0);
  PmfEvidence table;
  PmfObservation observation;
  observation.bssid[0]=2;observation.pmf=Pmf::Required;
  table.add(observation,-80);assert(table.size()==1 && table.lookup(observation.bssid)==Pmf::Required);
  observation.pmf=Pmf::Capable;table.add(observation,-90);
  assert(table.lookup(observation.bssid)==Pmf::Conflicting);
  observation.pmf=Pmf::Required;table.add(observation,-20);
  assert(table.lookup(observation.bssid)==Pmf::Conflicting);
  for (unsigned i=1;i<100;++i) {observation.bssid[5]=i;table.add(observation,-70);}
  assert(table.size()==64);
  observation.bssid[5]=255;table.add(observation,-127);
  assert(table.lookup(observation.bssid)==Pmf::Unknown);
  table.add(observation,-10);assert(table.lookup(observation.bssid)==Pmf::Required);
  table.clear();assert(table.size()==0 && table.lookup(observation.bssid)==Pmf::Unknown);
  observation.pmf=Pmf::NoRsn;table.add(observation,-40);
  observation.pmf=Pmf::CapabilitiesMissing;table.add(observation,-40);
  assert(table.lookup(observation.bssid)==Pmf::Conflicting);
  table.clear();observation.pmf=Pmf::CapabilitiesMissing;table.add(observation,-40);
  observation.pmf=Pmf::Required;table.add(observation,-40);
  assert(table.lookup(observation.bssid)==Pmf::Required);
  observation.pmf=Pmf::CapabilitiesMissing;table.add(observation,-40);
  assert(table.lookup(observation.bssid)==Pmf::Required);
  // Deterministic malformed-input corpus under UBSan, with bounded allocations.
  uint32_t random=0x6712abcd;
  for (unsigned i=0;i<20000;++i) {
    random=random*1664525u+1013904223u;
    Bytes noise(random%512);
    for (auto& byte : noise) {random=random*1664525u+1013904223u;byte=random>>24;}
    if (i%2==0 && noise.size()>=36) {
      const auto valid=beacon();memcpy(noise.data(),valid.data(),36);
    }
    parsePmfAdvertisement(noise.data(),noise.size(),result);
    auto mutated=beacon();
    for (unsigned edit=0;edit<3;++edit) {
      random=random*1664525u+1013904223u;
      mutated[random%mutated.size()]^=uint8_t(random>>24);
    }
    parsePmfAdvertisement(mutated.data(),mutated.size(),result);
  }
  puts("PMF explicit flags, malformed/truncated frames, FCS adapter, bounded evidence and conflicts passed");
}
