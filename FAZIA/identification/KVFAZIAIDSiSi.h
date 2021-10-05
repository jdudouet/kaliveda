//Created by KVClassFactory on Mon Mar 16 11:45:14 2015
//Author: ,,,

#ifndef __KVFAZIAIDSISI_H
#define __KVFAZIAIDSISI_H

#include "KVFAZIAIDTelescope.h"

class KVFAZIAIDSiSi : public KVFAZIAIDTelescope {

public:
   KVFAZIAIDSiSi();
   virtual ~KVFAZIAIDSiSi();
//   virtual void SetIdentificationStatus(KVReconstructedNucleus* n);


   ClassDef(KVFAZIAIDSiSi, 1) //identification telescope for FAZIA Si-Si idcards
};

#endif
