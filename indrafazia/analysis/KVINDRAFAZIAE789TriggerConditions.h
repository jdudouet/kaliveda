#ifndef __KVINDRAFAZIAE789TRIGGERCONDITIONS_H
#define __KVINDRAFAZIAE789TRIGGERCONDITIONS_H

#include "KVTriggerConditions.h"

/**
 \class KVINDRAFAZIAE789TriggerConditions
 \brief Trigger conditions for E789 INDRAFAZIA experiment
 \ingroup INDRAFAZIAnalysis

 Implements rejection of events incompatible with the (principal) on-line DAQ trigger for each run of E789.

 Used for analysis events when KVEventSelector::SetTriggerConditionsForRun() is called in KVEventSelector::InitRun().

 This will do 2 things: for physics runs (where the principal trigger for FAZIA was "Mult2"), we retain only
 events for which the trigger bit pattern is compatible with "Mult2", which can be either "Mult2" alone or
 "Mult2"+"Mult1/100" (see KVFAZIA::SetTriggerPatternsForDataSet() and KVFAZIA::GetTriggerForCurrentRun()).
 Events for which only "Mult1/100" (multiplicity 1 trigger downscaled by 100) fired are rejected.

 The 2nd thing this will do is reject the (very small) number of events where FAZIA is not present, only INDRA
 (in this case there is no FAZIA trigger bit pattern, which is not consistent with a trigger pattern "Mult2").

 \sa KVFAZIATrigger
 \author John Frankland
 \date Thu Feb 17 15:03:30 2022
*/

class KVINDRAFAZIAE789TriggerConditions : public KVTriggerConditions {
public:
   void SetTriggerConditionsForRun(KVEventSelector*, int);

   ClassDef(KVINDRAFAZIAE789TriggerConditions, 1) //Trigger conditions for E789 INDRAFAZIA experiment
};

#endif
