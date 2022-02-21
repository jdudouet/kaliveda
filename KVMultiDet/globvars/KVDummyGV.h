#ifndef __KVDUMMYGV_H
#define __KVDUMMYGV_H

#include "KVVarGlob.h"

/**
 \class KVDummyGV
 \brief Dummy global variable for event selection
 \ingroup GlobalVariables

 A KVDummyGV can be added to a KVGVList of global variables, not to calculate anything,
 but just to perform a selection of events with the KVVarGlob::TestEventSelection() mechanism.

 To use, simply define the required event selection with method SetEventSelection().

 \author John Frankland
 \date Mon Feb 21 10:10:57 2022
*/

class KVDummyGV : public KVVarGlob {
   virtual Double_t getvalue_int(Int_t) const
   {
      return 0;
   };

public:
   KVDummyGV() : KVVarGlob()
   {
      SetMaxNumBranches(0);
   }
   KVDummyGV(const Char_t* nom) : KVVarGlob(nom)
   {
      SetMaxNumBranches(0);
   }

   /// Skip the calculation loop in KVGVList::CalculateGlobalVariables
   Bool_t IsGlobalVariable() const
   {
      return kFALSE;
   }

   void Init() {}
   void Reset() {}
   void Calculate() {}

   ClassDef(KVDummyGV, 1) //Dummy global variable for event selection
};

#endif
