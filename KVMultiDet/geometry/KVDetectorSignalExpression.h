//Created by KVClassFactory on Thu Jun 20 15:36:03 2019
//Author: John Frankland,,,

#ifndef __KVDETECTORSIGNALEXPRESSION_H
#define __KVDETECTORSIGNALEXPRESSION_H

#include "KVDetectorSignal.h"
#include "TFormula.h"

/**
 \class KVDetectorSignalExpression
 \ingroup Calibration
 \brief Detector output from a mathematical combination of other signals
 */

class KVDetectorSignalExpression : public KVDetectorSignal {
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   TFormula* fFormula;//!
#else
   std::unique_ptr<TFormula> fFormula;
#endif
   std::vector<KVDetectorSignal*> fSignals;
   Bool_t fValid;
   Bool_t fRaw;

public:
   KVDetectorSignalExpression(const Char_t* type, const KVString& _expr, KVDetector* det);
   virtual ~KVDetectorSignalExpression()
   {
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
      SafeDelete(fFormula);
#endif
   }

   Double_t GetValue(const KVNameValueList& params = "") const;
   Bool_t IsValid() const
   {
      return fValid;
   }
   Bool_t IsRaw() const
   {
      // This is not a raw data parameter, in the sense it cannot be used to decide whether
      // the detector fired in the last read raw data
      return kFALSE;
   }
   Bool_t IsExpression() const
   {
      // Returns kTRUE, obvs.
      return kTRUE;
   }
   void SetValue(Double_t)
   {
      Warning("SetValue", "[%s] : Calling SetValue for a signal expression has no effect", GetName());
   }
   KVString GetExpression() const
   {
      // \returns the mathematical expression used to define this signal
      return GetLabel();
   }

   ClassDef(KVDetectorSignalExpression, 1) //Mathematical expression involving one or more detector signals
};

#endif
