//Created by KVClassFactory on Wed Jun  5 17:00:08 2019
//Author: John Frankland,,,


#ifndef __KVDETECTORSIGNAL_H
#define __KVDETECTORSIGNAL_H

#include "KVBase.h"
#include "KVNameValueList.h"

class KVDetector;
class KVNumberList;

/**
  \class KVDetectorSignal
  \ingroup Calibration
  \brief Output signal data produced by a detector
*/

class KVDetectorSignal : public KVBase {

   const KVDetector* fDetector;//! associated detector
   Double_t    fValue;// signal value

protected:
   void set_value(double x)
   {
      fValue = x;
   }

public:
   KVDetectorSignal()
      : KVBase(), fDetector(nullptr), fValue(0)
   {}
   KVDetectorSignal(const Char_t* type, const KVDetector* det = nullptr);

   virtual ~KVDetectorSignal()
   {}

   virtual Double_t GetValue(const KVNameValueList& = "") const
   {
      return fValue;
   }
   virtual void SetValue(Double_t x)
   {
      // Note that this has no effect on calibrated signals or signal expressions
      if (IsRaw() && !IsExpression()) set_value(x);
   }
   virtual void Reset()
   {
      // "Reset" the value of the signal, i.e. usually means set to zero.
      // Only affects signals whose value can be 'Set' (see SetValue)
      SetValue(0);
   }
   virtual Double_t GetInverseValue(Double_t out_val, const TString& in_sig, const KVNameValueList& = "") const
   {
      // Returns the value of the input signal for a given value of the output,
      // using the inverse calibration function

      if (in_sig == GetName()) return out_val;
      return 0.;
   }

   void SetDetector(const KVDetector* d)
   {
      fDetector = d;
   }

   const KVDetector* GetDetector() const
   {
      return fDetector;
   }

   virtual Bool_t IsValid() const
   {
      return kTRUE;
   }

   virtual Bool_t IsRaw() const
   {
      // Returns kTRUE if signal is available without any calibration
      // being defined i.e. corresponds to raw data
      return kTRUE;
   }

   virtual Bool_t IsExpression() const
   {
      // Returns kTRUE for detector signal expressions
      return kFALSE;
   }

   virtual Bool_t GetValueNeedsExtraParameters() const
   {
      // Returns kTRUE if GetValue() must be called with extra parameters
      // in order to calculate the correct value
      return kFALSE;
   }
   void ls(Option_t* = "") const;

   virtual Int_t GetStatus(const TString&) const;
   virtual Bool_t IsAvailableFor(const KVNameValueList&) const
   {
      // Used by KVCalibratedSignal to determine whether calibrated signals which depend
      // on extra parameters (in the KVNameValueList) are effectively available for
      // a given set of those extra parameters.
      //
      // Default here is kTRUE: if not overridden in child classes, this is a raw data
      // signal and therefore always available.
      return kTRUE;
   }

   ClassDef(KVDetectorSignal, 1) //Data produced by a detector
};

#endif
