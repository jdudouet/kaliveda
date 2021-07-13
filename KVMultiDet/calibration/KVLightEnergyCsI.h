#ifndef KV_LIGHT_ENERGY_CSI_H
#define KV_LIGHT_ENERGY_CSI_H

#include "KVCalibrator.h"

/**
  \class KVLightEnergyCsI
  \brief Light-energy calibration function for CsI detectors using a Fermi-function dependence on energy for delta-ray production
\ingroup Calibration

Parameterization of the total light output, \f$L\f$, as a function of deposited energy, \f$E\f$, (in MeV) for CsI detectors,
using a Fermi-function dependence on energy for the delta-ray production, instead of a sharp cut-off at \f$E=a_{3}A\f$:

\f[
L = a_{1}E\left\{1 - \frac{a_{2}AZ^{2}}{E}\left[ \ln\left( 1+\frac{E}{a_{2}AZ^{2}} \right) - \frac{a_{4}}{1+\exp\left( \frac{E_{\delta}-E}{8A} \right)}\ln\left(\frac{E+a_{2}AZ^{2}}{E_{\delta}+a_{2}AZ^{2}}\right) + \frac{a_{4}}{1+\exp (E_{\delta}/8A)\ln\left(\frac{a_{2}AZ^{2}}{E_{\delta}+a_{2}AZ^{2}}\right)} \right] \right\}
\f]

with \f$E_{\delta}=a_{3}A\f$ and the parameters:

| Parameter | Meaning |
|-----------|---------|
|\f$a_{1}\f$ | gain factor |
|\f$a_{2}\f$ | nuclear & recombination quenching term |
|\f$a_{3}\f$ | threshold (MeV/u) for delta-ray production |
|\f$a_{4}\f$ | fractional energy loss removed by delta rays |

See M. Parlog et al., Nuclear Instruments and Methods in Physics Research A 482 (2002) 693–706

In order to correctly reproduce the light-energy
relationship for all ions, two parameterizations should be used: one for Z=1 and another for Z>1.
The main difference is the gain parameter, a1, which compensates the understimation of total
light output for high energy protons.

The parameter a3 normally has a fixed value (a3=6), but this is not "hard-coded" : it should be fixed
when fitting data.
*/

class KVLightEnergyCsI: public KVCalibrator {

   Double_t CalculLumiere(Double_t*, Double_t*);

protected:
   mutable Double_t Z;
   mutable Double_t A;

public:
   KVLightEnergyCsI(Bool_t make_func = kTRUE);
   virtual ~ KVLightEnergyCsI() {}

   virtual Double_t Compute(Double_t chan, const KVNameValueList& z_and_a = "") const;
   virtual Double_t Invert(Double_t, const KVNameValueList& z_and_a = "") const;

   Bool_t IsAvailableFor(const KVNameValueList& z_and_a) const;

   ClassDef(KVLightEnergyCsI, 1)        //Light-energy calibration for CsI detectors
};

#endif
