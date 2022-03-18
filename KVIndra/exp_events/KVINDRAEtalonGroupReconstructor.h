//Created by KVClassFactory on Wed Feb 21 13:43:10 2018
//Author: John Frankland,,,

#ifndef __KVINDRAETALONGROUPRECONSTRUCTOR_H
#define __KVINDRAETALONGROUPRECONSTRUCTOR_H

#include "KVINDRABackwardGroupReconstructor.h"

/**
\class KVINDRAEtalonGroupReconstructor
\brief Reconstruct particles in INDRA groups with etalon telescopes
\ingroup INDRAReconstruction
*/

class KVINDRAEtalonGroupReconstructor : public KVINDRABackwardGroupReconstructor {

   Double_t fESi75, fESiLi;

protected:
   Bool_t CoherencyEtalons(KVReconstructedNucleus& PART);
   KVDetector* GetSi75(KVReconstructedNucleus* n)
   {
      return n->GetReconstructionTrajectory()->GetDetector("SI75");
   }
   KVDetector* GetSiLi(KVReconstructedNucleus* n)
   {
      return n->GetReconstructionTrajectory()->GetDetector("SILI");
   }
   Bool_t CalculateSiLiDEFromResidualEnergy(Double_t ERES, KVDetector* sili, KVReconstructedNucleus* n);
   Bool_t CalculateSi75DEFromResidualEnergy(Double_t ERES, KVDetector* si75, KVReconstructedNucleus* n);
public:
   KVINDRAEtalonGroupReconstructor() {}
   virtual ~KVINDRAEtalonGroupReconstructor() {}

   KVReconstructedNucleus* ReconstructTrajectory(const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node);

   bool DoCoherencyAnalysis(KVReconstructedNucleus& PART);
   void DoCalibration(KVReconstructedNucleus* PART);

   ClassDef(KVINDRAEtalonGroupReconstructor, 1) //Reconstruct particles in INDRA groups with etalon telescopes
};

#endif
