#ifndef __KVMULTIGAUSSISOTOPEFIT_H
#define __KVMULTIGAUSSISOTOPEFIT_H

#include "TF1.h"
#include "TVirtualPad.h"
#include "TColor.h"
#include "TArrayI.h"

/**
 \class KVMultiGaussIsotopeFit
 \brief Function for fitting PID mass spectra

 Write a detailed documentation for your class here, see doxygen manual for help.

 \author eindra
 \date Wed Jun 15 10:56:41 2022
*/

class KVMultiGaussIsotopeFit : public TF1 {
   double FitFunc(double* x, double* p)
   {
      /*
         p[0] = number of gaussians = Ng
         p[1],p[2] = background parameters: exp(p[1]+p[2]*x)
         p[3]      = sigma for all gaussians
         p[4],p[5] = norm, mean of first gaussian
         p[6],p[7] = norm, mean of second gaussian
         ...
         p[2*(N+1)],p[2*(N+1)+1] = norm, mean of Nth gaussian

         Total number of parameters is 2*(Ng+2).
      */
      double background = TMath::Exp(p[1] + p[2] * x[0]);
      for (int i = 1; i <= p[0]; ++i) {
         background += p[2 * (i + 1)] * TMath::Gaus(x[0], p[2 * (i + 1) + 1], p[3]);
      }
      return background;
   }
   int Z; // atomic number of the isotopes
   int Niso; // number of isotopes to fit = number of gaussians
   double PIDmin, PIDmax; // PID limits for current set of isotopes
   std::vector<int> Alist; // list of masses of isotopes (in increasing order)
   std::vector<double> PIDlist; // list of initial centroid (PID) of each isotope (in increasing order)

public:
   KVMultiGaussIsotopeFit() : TF1() {}
   KVMultiGaussIsotopeFit(int z, std::vector<int> alist)
      : TF1(),
        Z{z},
        Alist{alist}
   {
      // This constructor cannot be used to perform fits, but can be used to UnDraw() an existing fit
   }
   KVMultiGaussIsotopeFit(int z, int Ngauss, double PID_min, double PID_max, std::vector<int> alist, std::vector<double> pidlist);

   void ReleaseCentroids(double margin = 0.02);

   void UnDraw(TVirtualPad* pad = gPad) const;

   static void UnDrawGaussian(int z, int a, TVirtualPad* pad = gPad)
   {
      // Remove the graphical representation of the given gaussian from the given pad

      auto old_fit = pad->FindObject(get_name_of_isotope_gaussian(z, a));
      if (old_fit) delete old_fit;
   }

   void DrawFitWithGaussians(Option_t* opt = "") const;

   int GetMostProbableA(double PID, double& P) const;
   double GetMeanA(double PID) const;

   static TString get_name_of_multifit(int z)
   {
      return Form("multigauss_fit_Z=%d", z);
   }
   static TString get_name_of_isotope_gaussian(int z, int a)
   {
      return Form("gauss_fit_Z=%d_A=%d", z, a);
   }

   double GetCentroid(int i) const
   {
      // \returns the fitted centroid position of the ith gaussian (i=1,2,...,Niso)
      assert(i > 0 && i <= Niso);
      return GetParameter(2 * (i + 1) + 1);
   }
   double GetGaussianWidth() const
   {
      // \returns the fitted width (sigma) used for all gaussians
      return GetParameter(3);
   }

   ClassDef(KVMultiGaussIsotopeFit, 1) //Function for fitting PID mass spectrum
};

#endif
