#include "KVFAZIATrigger.h"
#include <bitset>

ClassImp(KVFAZIATrigger)

std::vector<std::string> KVFAZIATrigger::GetTriggerPatterns() const
{
   // Returns the names of all trigger patterns which are known and set for the trigger
   std::vector<std::string> pats;
   for (auto& p : fTriggerNameToBitPattern) {
      pats.push_back(p.first);
   }
   return pats;
}

void KVFAZIATrigger::Print(Option_t*) const
{
   // Print defined trigger bit patterns with their meanings

   std::cout << "Defined trigger patterns are:\n\n";
   for (auto& p : fTriggerNameToBitPattern) {
      std::cout << "\t" << p.first << " = 0b" << std::bitset<16>(p.second) << std::endl;
   }
}

