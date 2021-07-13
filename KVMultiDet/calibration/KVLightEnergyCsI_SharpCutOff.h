#ifndef __KVLIGHTENERGYCSI_SHARPCUTOFF_H
#define __KVLIGHTENERGYCSI_SHARPCUTOFF_H

#include "KVLightEnergyCsI.h"

/**
 \class KVLightEnergyCsI_SharpCutOff
 \brief Light-energy calibration function for CsI detectors using a sharp cut-off for delta-ray production
\ingroup Calibration

Parameterization of light output versus energy for CsI detectors,
the dependence on energy for the delta-ray production is a sharp cut-off at \f$E=a_{3}A\f$.

See Eq.9 of M. Parlog et al., Nuclear Instruments and Methods in Physics Research A 482 (2002) 693–706:

\f[
L = a_{1}E\left\{1 - \frac{a_{2}AZ^{2}}{E}\left[ \ln\left( 1+\frac{E}{a_{2}AZ^{2}} \right) - a_{4}\ln\left(\frac{E+a_{2}AZ^{2}}{E_{\delta}+a_{2}AZ^{2}}\right) \right] \right\}
\f]

with \f$E_{\delta}=a_{3}A\f$ and the parameters:

| Parameter | Meaning |
|-----------|---------|
|\f$a_{1}\f$ | gain factor |
|\f$a_{2}\f$ | nuclear & recombination quenching term |
|\f$a_{3}\f$ | threshold (MeV/u) for delta-ray production |
|\f$a_{4}\f$ | fractional energy loss removed by delta rays |


 \author John Frankland
 \date Mon Jul 12 14:08:20 2021
*/

class KVLightEnergyCsI_SharpCutOff : public KVLightEnergyCsI {
private:
   Double_t CalculLumiere(Double_t*, Double_t*);
public:
   KVLightEnergyCsI_SharpCutOff(): KVLightEnergyCsI(false)
   {
      SetCalibFunction(new TF1("fLight_CsI", this, &KVLightEnergyCsI_SharpCutOff::CalculLumiere, 0., 10000., 4));
   }
   virtual ~KVLightEnergyCsI_SharpCutOff() {}

   ClassDef(KVLightEnergyCsI_SharpCutOff, 1) //Light-energy calibration function for CsI detectors using a sharp cut-off for delta-ray production
};

#endif
