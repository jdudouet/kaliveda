//Created by KVClassFactory on Fri Jul  2 15:16:15 2010
//Author: bonnet

#ifndef __KVSIMREADER_HIPSE_ASYM_H
#define __KVSIMREADER_HIPSE_ASYM_H

#include "KVSimReader_HIPSE.h"
#include "KVNameValueList.h"
#include "TRotation.h"

class KVNucleus;
class TH1F;

/**
\class KVSimReader_HIPSE_asym
\brief Read ascii file for asymptotic events of the HIPSE code after SIMON deexcitation
\ingroup Simulation
*/

class KVSimReader_HIPSE_asym : public KVSimReader_HIPSE {
   void init()
   {
      Info("init", "passe");
      tree_name = "HIPSE_asym";
      Info("init", "%s", branch_name.Data());
      fPhiPlan = 0.;
   }

protected:
   Double_t fPhiPlan;
   TRotation rr;
   virtual void define_output_filename();

public:

   KVSimReader_HIPSE_asym();
   KVSimReader_HIPSE_asym(KVString filename);

   virtual ~KVSimReader_HIPSE_asym();

   virtual Bool_t ReadEvent();
   virtual Bool_t ReadNucleus();

   void ConvertEventsInFile(KVString filename);

   ClassDef(KVSimReader_HIPSE_asym, 1) //Read ascii file for asymptotic events of the HIPSE code after SIMON deexcitation
};

#endif
