#ifndef KVMultIMFAv_h
#define KVMultIMFAv_h
#include "KVMultAv.h"

/**
  \class KVMultIMFAv
  \ingroup GlobalVariables

  \brief Multiplicity of IMF \f$Z\geq 3\f$ in forward hemisphere of c.m. frame

  This is a KVMultAv with the added condition

  ~~~~~~~{.cpp}
  AddSelection("_NUC_->GetZ()>=3");
  ~~~~~~~
 */

class KVMultIMFAv: public KVMultAv {
   void init()
   {
      AddSelection("_NUC_->GetZ()>=3");
   }

public:
   ROOT_FULL_SET_WITH_INIT(KVMultIMFAv, KVMultAv)

   ClassDef(KVMultIMFAv, 1)
};
#endif
