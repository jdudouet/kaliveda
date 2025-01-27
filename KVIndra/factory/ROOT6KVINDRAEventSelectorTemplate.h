#include "KVINDRAEventSelector.h"

class ROOT6KVINDRAEventSelectorTemplate : public KVINDRAEventSelector {

   double ztot_sys, zvtot_sys;

public:
   ROOT6KVINDRAEventSelectorTemplate() {};
   virtual ~ROOT6KVINDRAEventSelectorTemplate() {};

   virtual void InitRun();
   virtual void EndRun() {}
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis() {}

   ClassDef(ROOT6KVINDRAEventSelectorTemplate, 0);
};
