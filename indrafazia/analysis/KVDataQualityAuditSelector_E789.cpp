#include "KVDataQualityAuditSelector_E789.h"
#include "KVINDRA.h"
#include "KVFAZIA.h"

void KVDataQualityAuditSelector_E789::InitRun()
{
   // For the moment, we include all identified particles, whether or not they have calibrated energies.

   KVDataQualityAuditSelector::InitRun();
   gIndra->AcceptECodes("");
   gFazia->AcceptECodes("");
   SetParticleConditions({"ok_indra", [](const KVNucleus * N)
   {
      auto rnuc = dynamic_cast<const KVReconstructedNucleus*>(N);
      // make sure KVReconstructedNucleus::IsCalibrated() corresponds to state of calibration
      // (for INDRA, all particles have IsCalibrated()=true, even if they are not/badly calibrated)
      if (rnuc->GetECode() > 0 && rnuc->GetECode() < 4) const_cast<KVReconstructedNucleus*>(rnuc)->SetIsCalibrated();
      else const_cast<KVReconstructedNucleus*>(rnuc)->SetIsUncalibrated();
      return kTRUE;
   }
                         });
}

ClassImp(KVDataQualityAuditSelector_E789)


