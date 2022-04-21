#include "KVDataPatch_E789_FAZIABadCsICalibFragments.h"

ClassImp(KVDataPatch_E789_FAZIABadCsICalibFragments)

void KVDataPatch_E789_FAZIABadCsICalibFragments::PrintPatchInfo() const
{
   std::cout << "Correct mistaken use of Z=2 CsI calib for all fragments in FAZIA\n\n";

   std::cout << "In data written with v.1.12/05, the FAZIA CsI calibration for Z=2 was mistakenly extended to\n";
   std::cout << "use for all fragments stopping in a CsI detector as well. This leads to an underestimation of\n";
   std::cout << "energies of fragment (by up to 50%). This patch corrects the CsI energy, laboratory total energy,\n";
   std::cout << "and centre-of-mass kinematics of these fragments.\n";

   std::cout << "Any particle to which this patch is applied will have a parameter\n\n";
   std::cout << "          DATAPATCH.E789_FAZIABadCsICalibFragments.APPLIED = true\n";
}



