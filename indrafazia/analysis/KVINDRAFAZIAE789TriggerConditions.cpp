#include "KVINDRAFAZIAE789TriggerConditions.h"
#include "KVFAZIA.h"
#include "KVEventSelector.h"

ClassImp(KVINDRAFAZIAE789TriggerConditions)

void KVINDRAFAZIAE789TriggerConditions::SetTriggerConditionsForRun(KVEventSelector* Selector, int)
{
   auto gv = Selector->GetGVList()->AddGVFirst("KVDummyGV", "FaziaTrigSelect");
   auto TrigPat = gFazia->GetTriggerForCurrentRun();
   Info("SetTriggerConditionsForRun", "Rejecting all events for which FAZIA trigger pattern is NOT: %s", TrigPat.c_str());
   gv->SetEventSelection([ = ](const KVVarGlob*) {
      return gFazia->GetTrigger().IsTrigger(TrigPat);
   });
}
