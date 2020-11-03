//Created by KVClassFactory on Tue Sep  8 16:14:25 2015
//Author: ,,,

#ifndef __KVFAZIAIDSICSI_H
#define __KVFAZIAIDSICSI_H

#include "KVFAZIAIDTelescope.h"
#include "KVIDZAGrid.h"
#include "KVFAZIADetector.h"
#include "KVIDCutLine.h"



class KVFAZIAIDSiCsI : public KVFAZIAIDTelescope {

protected:

   KVIDZAGrid* TheGrid;    //! telescope's grid
   KVFAZIADetector* fCsI;//!
   KVFAZIADetector* fSi2;//!
   KVIDCutLine* fBelowProton; //;
   KVIDCutLine* fSiThreshold; //;


public:

   KVFAZIAIDSiCsI();
   virtual ~KVFAZIAIDSiCsI();

   virtual UShort_t GetIDCode()
   {
      return kSi2CsI;
   };
   virtual Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   Double_t GetIDMapX(Option_t* opt = "");
   Double_t GetIDMapY(Option_t* opt = "");
   virtual void Initialize();

   KVIDCutLine* GetBelowProtonLine() const
   {
      return fBelowProton;
   }
   KVIDCutLine* GetSiThresholdLine() const
   {
      return fSiThreshold;
   }
//   void SetIdentificationStatus(KVReconstructedNucleus* n);

   ClassDef(KVFAZIAIDSiCsI, 1) //id telescope to manage FAZIA Si-CsI identification
};

#endif
