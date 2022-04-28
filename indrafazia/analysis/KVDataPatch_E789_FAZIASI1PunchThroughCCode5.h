#ifndef __KVDATAPATCH_E789_FAZIASI1PUNCHTHROUGHCCODE5_H
#define __KVDATAPATCH_E789_FAZIASI1PUNCHTHROUGHCCODE5_H

#include "KVDataPatch.h"
#include "KVReconstructedNucleus.h"
#include "KVFAZIA.h"

/**
 \class KVDataPatch_E789_FAZIASI1PunchThroughCCode5
 \brief Data patch to correct E789 data generated with 1.12/05

       In 1.12/05 unidentified particles apparently stopped in SI2, with good Si1-PSA identification
       were added to the event, (with CCode=5) but they (mostly) correspond to
       punch-through in SI1 and give bad "accumulations" of data in e.g. Z-V plots.

       So they are best left unidentified! (we still give the CCode value 5)
       They receive idcode KVFAZIA::ID_SI1_PUNCH_THROUGH, are identified, but not in Z (or A)

 \author John Frankland
 \date Tue Apr 26 16:51:28 2022
*/

class KVDataPatch_E789_FAZIASI1PunchThroughCCode5 : public KVDataPatch {
public:
   KVDataPatch_E789_FAZIASI1PunchThroughCCode5() : KVDataPatch()
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
      if (rnuc->InArray("FAZIA") && rnuc->GetParameters()->HasIntParameter("CCode")
            && (rnuc->GetParameters()->GetIntValue("CCode") == 5)) {
         auto idr = rnuc->GetIdentificationResult(rnuc->GetNumberOfIdentificationResults());
         auto sipsa_idtel = (KVIDTelescope*)rnuc->GetReconstructionTrajectory()->GetIDTelescopes()->Last();
         idr->Zident = false;
         idr->Aident = false;
         idr->SetComment("particle partially identified by pulse shape analysis in SI1, although it is punching through (no SI2 signal or SI1-SI2 id)");
         rnuc->SetIdentification(idr, sipsa_idtel);
      }
   }
   void PrintPatchInfo() const;

   ClassDef(KVDataPatch_E789_FAZIASI1PunchThroughCCode5, 1) //Data patch to correct E789 data generated with 1.12/05
};

#endif
