#include "KVMultiGaussIsotopeFit.h"



KVMultiGaussIsotopeFit::KVMultiGaussIsotopeFit(int z, int Ngauss, double PID_min, double PID_max, std::vector<int> alist, std::vector<double> pidlist)
   : TF1("MultiGaussIsotopeFit", this, &KVMultiGaussIsotopeFit::FitFunc, PID_min, PID_max, 2 * (Ngauss + 2)),
     Z{z},
     Niso{Ngauss},
     PIDmin{PID_min}, PIDmax{PID_max},
     Alist{alist},
     PIDlist{pidlist}
{
   // Constructor used to initialize and prepare a new fit of isotope PID spectrum
   FixParameter(0, Niso);
   SetParLimits(1, 1e-3, 1e+3);
   SetParameter(1, 4.);
   SetParName(1, "Norm");
   SetParLimits(2, -10, 1e-3);
   SetParameter(2, -0.2);
   SetParName(2, "Bkg. slope");
   SetParName(3, "Sigma");
   SetParLimits(3, 1.e-2, 2.);
   SetParameter(3, 0.1);
   for (int ig = 1; ig <= Niso; ++ig) {
      SetParName(2 * (ig + 1), Form("Norm. A=%d", Alist[ig - 1]));
      SetParLimits(2 * (ig + 1), 1., 1.e+06);
      SetParameter(2 * (ig + 1), 15000.);
      SetParName(2 * (ig + 1) + 1, Form("Centr. A=%d", Alist[ig - 1]));
      FixParameter(2 * (ig + 1) + 1, PIDlist[ig - 1]);
   }
}

void KVMultiGaussIsotopeFit::ReleaseCentroids(double margin)
{
   // Release the constraint on the positions of the centroids, within +/-margin
   for (int ig = 1; ig <= Niso; ++ig) {
      SetParLimits(2 * (ig + 1) + 1, PIDlist[ig - 1] - 0.02, PIDlist[ig - 1] + 0.02);
   }
}

void KVMultiGaussIsotopeFit::UnDraw(TVirtualPad* pad) const
{
   // Remove the graphical representation of this fit from the given pad

   auto old_fit = pad->FindObject(get_name_of_multifit(Z));
   if (old_fit) delete old_fit;
   for (auto a : Alist) {
      UnDrawGaussian(Z, a, pad);
   }
}

void KVMultiGaussIsotopeFit::DrawFitWithGaussians(Option_t* opt) const
{
   // Draw the overall fit plus the individual gaussians for each isotope

   DrawCopy(opt)->SetName(get_name_of_multifit(Z));

   TF1 fgaus("fgaus", "gaus", PIDmin, PIDmax);
   // give a different colour to each gaussian
   auto cstep = TColor::GetPalette().GetSize() / (Niso + 1);
   int ig = 1;
   for (auto& a : Alist) {
      fgaus.SetParameters(GetParameter(2 * (ig + 1)), GetParameter(2 * (ig + 1) + 1), GetParameter(3));
      fgaus.SetNpx(500);
      fgaus.SetLineColor(TColor::GetPalette()[cstep * ig]);
      fgaus.SetLineWidth(2);
      fgaus.SetLineStyle(9);
      fgaus.DrawCopy("same")->SetName(get_name_of_isotope_gaussian(Z, a));
      ++ig;
   }
}

int KVMultiGaussIsotopeFit::GetMostProbableA(double PID, double& P) const
{
   // for a given PID, calculate the most probable value of A, P is its probability.

   auto total = Eval(PID);
   TF1 fgaus("fgaus", "gaus", PIDmin, PIDmax);
   std::map<double, int> probabilities;
   int ig = 1;
   for (auto& a : Alist) {
      fgaus.SetParameters(GetParameter(2 * (ig + 1)), GetParameter(2 * (ig + 1) + 1), GetParameter(3));
      probabilities[fgaus.Eval(PID) / total] = a;
      ++ig;
   }
   // the largest probability is now the last element in the map:
   // get a reverse iterator to the beginning of the reversed map
   auto it = probabilities.rbegin();
   P = it->first;
   return it->second;
}

double KVMultiGaussIsotopeFit::GetMeanA(double PID) const
{
   // for a given PID, calculate the weighted sum of A
   auto total = Eval(PID);
   TF1 fgaus("fgaus", "gaus", PIDmin, PIDmax);
   int ig = 1;
   double amean(0), totprob(0);
   for (auto& a : Alist) {
      fgaus.SetParameters(GetParameter(2 * (ig + 1)), GetParameter(2 * (ig + 1) + 1), GetParameter(3));
      auto weight = fgaus.Eval(PID) / total;
      amean += weight * a;
      totprob += weight;
      ++ig;
   }
   return totprob > 0 ? amean / totprob : -1.;
}

ClassImp(KVMultiGaussIsotopeFit)
