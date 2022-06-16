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
   double centroid_fit(double* x, double* p)
   {
      /*
         centroids of gaussians are expected to increase linearly with mass A
         x[0]=A
         p[0]=offset
         p[1]=slope
         */
      return p[0] + p[1] * x[0] + p[2] * x[0] * x[0];
   }

   double FitFunc(double* x, double* p)
   {
      /*
         x[0] = PID
         p[0] = number of gaussians = Ng (fixed)
         p[1],p[2] = background parameters: exp(p[1]+p[2]*x)
         p[3]      = sigma for all gaussians
         p[4],p[5],p[6] = offset & slope for centroid PID vs. A correlation
         p[7]...p[6+Ng] = norm of each gaussian
         p[6+Ng+1]...p[6+2*Ng] = A of each gaussian

         Total number of parameters is 7+2*Ng
      */
      int Ng = p[0];
      double background = TMath::Exp(p[1] + p[2] * x[0]);
      for (int i = 1; i <= Ng; ++i) {
         background += p[get_gauss_norm_index(i)] * TMath::Gaus(x[0], centroid_fit(&p[get_mass_index(i, Ng)], &p[4]), p[3]);
      }
      return background;
   }
   int get_gauss_norm_index(int ig) const
   {
      return 6 + ig;
   }
   int get_mass_index(int ig, int ng) const
   {
      return 6 + ng + ig;
   }
   int total_number_parameters(int ng) const
   {
      return 7 + 2 * ng;
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

   void ReleaseCentroids()
   {
      // Release the constraint on the positions of the centroids
      SetParLimits(4, -50, 50);
      SetParLimits(5, 1.e-2, 5.);
      SetParLimits(6, -5, 5.);
   }

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
      return GetParameter(4) + (GetParameter(5) + GetParameter(6) * Alist[i - 1]) * Alist[i - 1];
   }
   double GetGaussianWidth(int) const
   {
      // \returns the fitted width (sigma) used for all gaussians
      return GetParameter(3);
   }
   double GetGaussianNorm(int i) const
   {
      // \returns the fitted normalisation constant of the ith gaussian (i=1,2,...,Niso)
      assert(i > 0 && i <= Niso);
      return GetParameter(get_gauss_norm_index(i));
   }

   ClassDef(KVMultiGaussIsotopeFit, 1) //Function for fitting PID mass spectrum
};

#endif
