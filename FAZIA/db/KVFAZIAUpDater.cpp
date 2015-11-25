//Created by KVClassFactory on Mon Sep 14 10:58:04 2015
//Author: ,,,

#include "KVFAZIAUpDater.h"
#include "KVFAZIADB.h"
#include "KVDBParameterList.h"
#include "KVFAZIA.h"
#include "KVSignal.h"
#include "KVFAZIADetector.h"

ClassImp(KVFAZIAUpDater)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIAUpDater</h2>
<h4>handle FAZIA detectors configuration for a given run</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIAUpDater::KVFAZIAUpDater()
{
   // Default constructor
}

KVFAZIAUpDater::~KVFAZIAUpDater()
{
   // Destructor
}

void KVFAZIAUpDater::SetCalibParameters(KVDBRun* dbrun)
{
   SetPSAParameters(dbrun);
}

void KVFAZIAUpDater::SetPSAParameters(KVDBRun* dbrun)
{
   //Put all signals in standard configuration
   //PSA parameters, threshold ...
   TIter ld(gFazia->GetDetectors());
   KVFAZIADetector* det = 0;
   KVSignal* sig = 0;

   while ((det = (KVFAZIADetector*)ld())) {
      TIter ls(det->GetListOfSignals());
      while ((sig = (KVSignal*)ls())) {
         sig->LoadPSAParameters();
      }
   }

   //Loop on exceptions stores in the database
   //and update parameters for each concerned signal
   KVDBParameterList* par = 0;
   TList* list = (TList*)dbrun->GetLinks("Exceptions");
   TIter next(list);
   while ((par = (KVDBParameterList*)next())) {
      TString sdet(par->GetParameters()->GetStringValue("Detector"));
      TString ssig(par->GetParameters()->GetStringValue("Signal"));
      if (!gFazia) {
         Error("SetPSAParameters", "gFazia object not defined");
         return;
      }
      det = (KVFAZIADetector*)gFazia->GetDetector(sdet.Data());
      if (det) {
         sig = det->GetSignal(ssig.Data());
         if (sig) {
            sig->UpdatePSAParameter(par);
         } else {
            Warning("SetPSAParameters", "Unkonwn signal %s", ssig.Data());
         }
      } else {
         Warning("SetPSAParameters", "Unkonwn detector %s", sdet.Data());
      }
   }
}
