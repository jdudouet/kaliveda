#include "KVLightEnergyCsI_Tabacaru_Camp2_10_17.h"

ClassImp(KVLightEnergyCsI_Tabacaru_Camp2_10_17)


Double_t KVLightEnergyCsI_Tabacaru_Camp2_10_17::tabacaru(double* L, double* par)
{
   // The original inverted function (i.e. gives energy for a given value of light) from the INDRA
   // 2nd campaign FORTRAN (csi10_17_2_E.f), because the inversion of CalculLumiere by TF1
   // does not give the same results - I suspect that in fact the formula used was not the same.
   // L=Lumiere
   // par = a1, a2, a3, a4

   double hl = L[0];
   double a1t = par[0];
   double a2t = par[1];
   double a3t = par[2];
   double a4t = par[3];

   double x;
   if (a2t < 1.e-7)
      x = (hl) / a1t;
   else
      x = 2000.;

   double az2 = A * Z * Z;
   double argx = 1. + x / (a2t * az2);
   double cond = true;

   double niter = 0;
   double eps = 1.e-4;
   double fbe = 1.0;

   double   E0 = a3t * A;

   while (cond) {
      niter = niter + 1;

      double a4e = a4t;
      if (x < E0) a4e = 0;

      double fx = -hl + fbe * a1t * (x - a2t * az2 * log(argx) + a2t * a4e * az2 * log((x + a2t * az2) / (E0 + a2t * az2)));
      double fpx = fbe * a1t * (x + a2t * az2 * a4e) / (x + a2t * az2);

      double dx = - fx / fpx;
      x += dx;
      argx = 1. + x / (a2t * az2);
      cond = (abs(dx / x) >= eps) && (argx > 0.) && (niter <= 50);
   }

   if (argx <= 0 || niter > 50) {
      return 0;
   }
   return fbe * x;
}
