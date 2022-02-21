#ifndef __KVFAZIATRIGGER_H
#define __KVFAZIATRIGGER_H

#include "KVBase.h"
#include "KVNameValueList.h"

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
      Mult1,
      Mult1Downscale100,
      Mult2
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
      fTriggerPattern = tp;
   }
   void SetTriggerPattern(const TString& name, uint16_t value)
   {
      fTriggerNameToBitPattern[name.Data()] = value;
      fTriggerEnumToBitPattern[fTriggerPatterns[name.Data()]] = value;
   }
   std::vector<std::string> GetTriggerPatterns() const;
   bool IsTrigger(TriggerPattern tp) const
   {
      return fTriggerPattern & fTriggerEnumToBitPattern[tp];
   }
   bool IsTrigger(const std::string& tp) const
   {
      return IsTrigger(fTriggerPatterns[tp]);
   }
   void Print(Option_t* = "") const;

   ClassDef(KVFAZIATrigger, 1) //The trigger pattern for each FAZIA event
};

#endif
