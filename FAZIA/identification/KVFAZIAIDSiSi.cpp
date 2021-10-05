//Created by KVClassFactory on Mon Mar 16 11:45:14 2015
//Author: ,,,

#include "KVFAZIAIDSiSi.h"
#include "KVIDZAGrid.h"
#include "KVFAZIADetector.h"
#include "KVDataSet.h"

ClassImp(KVFAZIAIDSiSi)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIAIDSiSi</h2>
<h4>identification telescope for FAZIA Si-Si idcards</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIAIDSiSi::KVFAZIAIDSiSi()
{
   // Default constructor
   SetType("Si-Si");
   set_id_code(kSi1Si2);
   SetHasMassID(kTRUE);
   fMaxZ = 22.5;
   fSigmaZ = .5;
}

KVFAZIAIDSiSi::~KVFAZIAIDSiSi()
{
   // Destructor
}

//void KVFAZIAIDSiSi::SetIdentificationStatus(KVReconstructedNucleus *n)
//{
//    fMassIDProb->SetParameters(22.5, .5);
//    KVFAZIAIDTelescope::SetIdentificationStatus(n);
//}

