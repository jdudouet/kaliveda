#include "KVLightEnergyCsI_Tabacaru_Camp2_10_17.h"

ClassImp(KVLightEnergyCsI_Tabacaru_Camp2_10_17)


Double_t KVLightEnergyCsI_Tabacaru_Camp2_10_17::tabacaru(double* L, double* par)
{
   // The original inverted function (i.e. gives energy for a given value of light) from the INDRA
   // 2nd campaign FORTRAN (csi10_17_2_E.f), because the inversion of CalculLumiere by TF1
   // does not give the same results - I suspect that in fact the formula used was not the same.
   // L=Lumiere
   // par = a1, a2, a3, a4

   auto hl = L[0];
   auto a1t = par[0];
   auto a2t = par[1];
   auto a3t = par[2];
   auto a4t = par[3];

   double x;
   if (a2t < 1.e-7)
      x = (hl) / a1t;
   else
      x = 2000.;

   auto az2 = A * Z * Z;
   auto argx = 1. + x / (a2t * az2);
   auto cond = true;

   auto niter = 0;
   auto eps = 1.e-4;
   auto fbe = 1.0;

   auto   E0 = a3t * A;

   while (cond) {
      niter = niter + 1;

      auto a4e = a4t;
      if (x < E0) a4e = 0;

      auto fx = -hl + fbe * a1t * (x - a2t * az2 * log(argx) + a2t * a4e * az2 * log((x + a2t * az2) / (E0 + a2t * az2)));
      auto fpx = fbe * a1t * (x + a2t * az2 * a4e) / (x + a2t * az2);

      auto dx = - fx / fpx;
      x += dx;
      argx = 1. + x / (a2t * az2);
      cond = (abs(dx / x) >= eps) && (argx > 0.) && (niter <= 50);
   }

   if (argx <= 0 || niter > 50) {
      return 0;
   }
   return fbe * x;
}
