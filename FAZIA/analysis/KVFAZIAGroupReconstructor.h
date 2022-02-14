#ifndef KVFAZIAGROUPRECONSTRUCTOR_H
#define KVFAZIAGROUPRECONSTRUCTOR_H

#include "KVGroupReconstructor.h"

/**
\class KVFAZIAGroupReconstructor
\brief Reconstruction of particles detected in FAZIA telescopes
\ingroup FAZIARecon
*/

class KVFAZIAGroupReconstructor : public KVGroupReconstructor {

   KVGeoDNTrajectory* theTrajectory;//! 1 FAZIA group = 1 telescope with 1 unique trajectory SI1 - SI2 - CSI

protected:

   void CalibrateParticle(KVReconstructedNucleus* PART);
   void CalibrateCoherencyParticle(KVReconstructedNucleus* PART);
   void PostReconstructionProcessing();
   void IdentifyParticle(KVReconstructedNucleus& PART);

   void  ChangeReconstructedTrajectory(KVReconstructedNucleus& PART);

public:
   KVFAZIAGroupReconstructor() {}
   virtual ~KVFAZIAGroupReconstructor() {}

   void SetGroup(KVGroup* g)
   {
      KVGroupReconstructor::SetGroup(g);
      // set unique trajectory for group
      theTrajectory = (KVGeoDNTrajectory*)g->GetDetectorByType("CsI")->GetNode()->GetForwardTrajectories()->First();
   }

   ClassDef(KVFAZIAGroupReconstructor, 1) // Reconstruct particles in FAZIA telescopes
};

#endif // KVFAZIAGROUPRECONSTRUCTOR_H
