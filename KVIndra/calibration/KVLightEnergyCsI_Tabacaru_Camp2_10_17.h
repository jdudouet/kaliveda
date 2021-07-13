#ifndef __KVLIGHTENERGYCSI_TABACARU_CAMP2_10_17_H
#define __KVLIGHTENERGYCSI_TABACARU_CAMP2_10_17_H

#include "KVLightEnergyCsI.h"

/**
 \class KVLightEnergyCsI_Tabacaru_Camp2_10_17
 \brief Copy of light-energy used for iNDRA 2nd campaign rings 10-17
\ingroup INDRACalibration

 \author John Frankland
 \date Mon Jul 12 15:30:35 2021
*/

class KVLightEnergyCsI_Tabacaru_Camp2_10_17 : public KVLightEnergyCsI {
   Double_t tabacaru(double*, double*);

public:
   KVLightEnergyCsI_Tabacaru_Camp2_10_17()
      : KVLightEnergyCsI(false)
   {
      SetCalibFunction(new TF1("fLight_CsI", this, &KVLightEnergyCsI_Tabacaru_Camp2_10_17::tabacaru, 0., 10000., 4));
      SetUseInverseFunction(kFALSE);
   }
   virtual ~KVLightEnergyCsI_Tabacaru_Camp2_10_17() {}

   ClassDef(KVLightEnergyCsI_Tabacaru_Camp2_10_17, 1) //Copy of light-energy used for iNDRA 2nd campaign rings 10-17
};

#endif
