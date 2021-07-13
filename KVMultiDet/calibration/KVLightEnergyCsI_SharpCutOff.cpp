#include "KVLightEnergyCsI_SharpCutOff.h"

ClassImp(KVLightEnergyCsI_SharpCutOff)


Double_t KVLightEnergyCsI_SharpCutOff::CalculLumiere(Double_t* x, Double_t* par)
{
   //Calcul de la lumiere totale a partir de Z, A d'une particule et son energie
   //
   //~~~~~~~~~~~~~~~~~~
   // x[0] = energie (MeV)
   // par[0] = a1 : gain factor
   // par[1] = a2 : nuclear & recombination quenching term
   // par[2] = a3 : threshold (MeV/u) for delta-ray production
   // par[3] = a4 : fractional energy loss removed by delta rays
   //~~~~~~~~~~~~~~~~~~
   //

   Double_t energie = x[0];
   Double_t c1 = par[0];
   Double_t c2 = Z * Z * A * par[1];
   Double_t c3 = A * par[2];
   Double_t c4 = energie > c3 ? par[3] : 0.;

   Double_t lumcalc = c1 * energie;
   if (c2 > 0.0) {
      lumcalc = lumcalc - c1 * c2 * TMath::Log(1. + energie / c2)
                + c1 * c2 * c4 * TMath::Log((energie + c2) / (c3 + c2));
   }

   return lumcalc;
}

