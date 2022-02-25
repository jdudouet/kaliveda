//Created by KVClassFactory on Wed Jun  9 11:30:26 2010
//Author: John Frankland,,,

#include "KVIdentificationResult.h"

ClassImp(KVIdentificationResult)

void KVIdentificationResult::Copy(TObject& obj) const
{
   // Copy this to obj
   KVBase::Copy(obj);
   KVIdentificationResult& id = (KVIdentificationResult&)obj;
   id.IDattempted = IDattempted;
   id.IDOK = IDOK;
   id.IDcode = IDcode;
   id.Zident = Zident;
   id.Aident = Aident;
   id.IDquality = IDquality;
   id.Z = Z;
   id.A = A;
   id.PID = PID;
   id.deltaEpedestal = deltaEpedestal;
   id.Rejecting_Cut = Rejecting_Cut;
   id.flags = flags;
}

void KVIdentificationResult::Clear(Option_t*)
{
   // Reset to initial values
   SetIDType("");
   SetComment("");
   SetGridName("");
   IDattempted = kFALSE;
   IDOK = kFALSE;
   IDcode = -1;
   Zident = kFALSE;
   Aident = kFALSE;
   IDquality = -1;
   Z = -1;
   A = -1;
   PID = -1.0;
   deltaEpedestal = deltaEpedestal_UNKNOWN;
   Rejecting_Cut = "";
   flags.clear();
}

void KVIdentificationResult::Print(Option_t*) const
{
   printf("Identification #%d  -  Type:%s ", GetNumber(), GetIDType());
   if (!IDattempted) {
      printf("   => not attempted\n\n");
      return;
   }
   if (IDOK) printf("   => SUCCESS\n");
   else printf("   => FAILURE\n");
   printf("  Grid used: %s\n", GetGridName());
   if (Rejecting_Cut != "") printf("   rejected by cut : %s", Rejecting_Cut.Data());
   printf("  ID code = %d  Quality code = %d (%s)\n", IDcode, IDquality, GetComment());
   if (Zident) printf("  Z identified = %d", Z);
   else printf("  Z returned = %d", Z);
   if (Aident) printf("    A identified = %d", A);
   else printf("  A returned = %d", A);
   if (Zident || Aident) printf("    PID = %f", PID);
   printf("\n");
   switch (deltaEpedestal) {

      case deltaEpedestal_NO:
         printf("  delta-E pedestal : NO\n");
         break;

      case deltaEpedestal_YES:
         printf("  delta-E pedestal : YES\n");
         break;

      default:
      case deltaEpedestal_UNKNOWN:
         break;
   }
   if (auto nflags = flags.size()) {
      printf("  Info flags:\n");
      for (auto& fv : flags) {
         printf("  %s : ", fv.first.c_str());
         TString fmt_flags;
         auto nflags = fv.second.size();
         for (uint i = 0; i < nflags; ++i) {
            if (i) fmt_flags += "|";
            fmt_flags += fv.second[i];
         }
         printf("%s\n", fmt_flags.Data());
      }
   }
}
