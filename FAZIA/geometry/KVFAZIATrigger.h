#ifndef __KVFAZIATRIGGER_H
#define __KVFAZIATRIGGER_H

#include "KVBase.h"
#include "KVNameValueList.h"
#include <unordered_map>

/**
 \class KVFAZIATrigger
 \brief The trigger pattern for each FAZIA event
 \ingroup DAQ

 Can be used to inquire about, check and otherwise verify the FAZIA DAQ trigger for each event
 (for compatible datasets).

 \author John Frankland
 \date Wed Feb 16 20:39:46 2022
*/

class KVFAZIATrigger : public KVBase {
public:
   enum class TriggerPattern : uint8_t {
      Mult1,                ///< Minimum bias M>=1 multiplicity trigger
      Mult1Downscale100,    ///< Downscaled (1/100) minimum bias M>=1 multiplicity trigger
      Mult2                 ///< M>=2 multiplicity trigger
   };
private:
   uint16_t fTriggerPattern;
   mutable std::unordered_map<std::string, TriggerPattern> fTriggerPatterns = {
      {"Mult1", TriggerPattern::Mult1},
      {"Mult1/100", TriggerPattern::Mult1Downscale100},
      {"Mult2", TriggerPattern::Mult2},
   };
   std::unordered_map<std::string, uint16_t> fTriggerNameToBitPattern;
   mutable std::map<TriggerPattern, uint16_t> fTriggerEnumToBitPattern;
public:
   void SetTriggerPattern(uint16_t tp)
   {
      // Set the actual value of the trigger pattern read from data
      fTriggerPattern = tp;
   }
   void AddTriggerPattern(const TString& name, uint16_t value)
   {
      // Add a trigger pattern to the list of all known trigger patterns for the experiment
      fTriggerNameToBitPattern[name.Data()] = value;
      fTriggerEnumToBitPattern[fTriggerPatterns[name.Data()]] = value;
   }
   std::vector<std::string> GetTriggerPatterns() const;
   bool IsTrigger(TriggerPattern tp) const
   {
      // \returns true if the given trigger pattern is compatible with the current value
      return fTriggerPattern & fTriggerEnumToBitPattern[tp];
   }
   bool IsTrigger(const std::string& tp) const
   {
      // \returns true if the given trigger pattern is compatible with the current value
      return IsTrigger(fTriggerPatterns[tp]);
   }
   void Print(Option_t* = "") const;

   ClassDef(KVFAZIATrigger, 1) //The trigger pattern for each FAZIA event
};

#endif
