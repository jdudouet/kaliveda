#ifndef __KVDATAPATCH_E789_FAZIABADCSICALIBFRAGMENTS_H
#define __KVDATAPATCH_E789_FAZIABADCSICALIBFRAGMENTS_H

#include "KVDataPatch.h"
#include "KVReconstructedNucleus.h"
#include "KVFAZIA.h"

/**
 \class KVDataPatch_E789_FAZIABadCsICalibFragments
 \brief Correct mistaken use of Z=2 CsI calib for all fragments in FAZIA

 In data written with v.1.12/05, the FAZIA CsI calibration for Z=2 was mistakenly extended to
 use for all fragments stopping in a CsI detector as well. This leads to an underestimation of
 energies of fragment (by up to 50%). This patch corrects the CsI energy, laboratory total energy,
 and centre-of-mass kinematics of these fragments.

 Any particle to which this patch is applied will have a parameter
~~~~
DATAPATCH.E789_FAZIABadCsICalibFragments.APPLIED = true
~~~~

 \author eindra
 \date Thu Apr 21 09:41:40 2022
*/

class KVDataPatch_E789_FAZIABadCsICalibFragments : public KVDataPatch {
public:
   KVDataPatch_E789_FAZIABadCsICalibFragments(): KVDataPatch()
   {
      // Default constructor
      SetName(ClassName());
      SetTitle(Class()->GetTitle());
   }

   Bool_t IsRequired(TString dataset, TString datatype, Int_t,
                     TString dataseries, Int_t datareleasenumber, const TList*)
   {
      return (dataset == "INDRAFAZIA.E789") && (datatype == "recon") && (dataseries == "1.12") && (datareleasenumber == 5);
   }

   Bool_t IsEventPatch()
   {
      return false;
   }
   Bool_t IsParticlePatch()
   {
      return true;
   }
   void ApplyToEvent(KVReconstructedEvent*) {}
   void ApplyToParticle(KVReconstructedNucleus* rnuc)
   {
      if (rnuc->InArray("FAZIA") && rnuc->IsZMeasured() && rnuc->IsCalibrated()
            && rnuc->GetStoppingDetector()->IsType("CsI") && rnuc->GetZ() > 2)
         correct_ecsi(rnuc);
   }

   void PrintPatchInfo() const;

   bool correct_ecsi(KVReconstructedNucleus* rnuc)
   {
      // if nucleus stopped in FAZIA csi & has Z>2 with a CsI energy taken from calibration (FAZIA.ECSI>0),
      // replace CsI energy with energy deduced from measured energy in SI1 & SI2
      // (if & only if both detectors are working & calibrated, and if there is no suspicion of
      // pile-up in SI1 (for SI2-CSI identification) or in SI1/SI2 (for CSI identification)):
      // in this case we must have FAZIA.ESI1>0 and FAZIA.ESI2>0.
      //
      // return true if correction is performed

      double ecsi, esi1, esi2;
      if ((ecsi = rnuc->GetParameters()->GetDoubleValue("FAZIA.ECSI")) > 0
            && (esi1 = rnuc->GetParameters()->GetDoubleValue("FAZIA.ESI1")) > 0
            && (esi2 = rnuc->GetParameters()->GetDoubleValue("FAZIA.ESI2")) > 0) {
         // calculate CsI energy from total energy loss ESI1+ESI2 in a silicon detector
         // with effective thickness equal to the sum of SI1 & SI2 thicknesses
         auto si1 = rnuc->GetReconstructionTrajectory()->GetDetector("SI1");
         auto si2 = rnuc->GetReconstructionTrajectory()->GetDetector("SI2");
         KVDetector si1si2("Si", si1->GetThickness() + si2->GetThickness());
         auto new_ecsi = si1si2.GetEResFromDeltaE(rnuc->GetZ(), rnuc->GetA(), esi1 + esi2);
         if (new_ecsi > 0) {
            rnuc->SetParameter("DATAPATCH.E789_FAZIABadCsICalibFragments.APPLIED", true);
            rnuc->SetParameter("FAZIA.ECSI_BAD", ecsi);
            rnuc->SetParameter("FAZIA.ECSI", -new_ecsi); // calculated CsI energy
            rnuc->SetECode(KVFAZIA::ECodes::SOME_ENERGY_LOSSES_CALCULATED);
            // set new energy in lab frame
            rnuc->SetEnergy(esi1 + esi2 + new_ecsi);
            // update kinematics in other frames (e.g. "CM" frame automatically defined for analysis)
            rnuc->UpdateAllFrames();
            return true;
         }
      }
      return false;
   }

   ClassDef(KVDataPatch_E789_FAZIABadCsICalibFragments, 1) //Correct mistaken use of Z=2 CsI calib for all fragments in FAZIA
};

#endif
