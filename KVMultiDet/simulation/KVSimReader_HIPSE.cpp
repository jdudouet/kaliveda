//Created by KVClassFactory on Wed Jul  7 12:15:56 2010
//Author: bonnet

#include "KVSimReader_HIPSE.h"

ClassImp(KVSimReader_HIPSE)


KVSimReader_HIPSE::KVSimReader_HIPSE()
{
   // Default constructor
   init();
}

KVSimReader_HIPSE::KVSimReader_HIPSE(KVString filename)
{
   init();
   ConvertEventsInFile(filename);
}

KVSimReader_HIPSE::~KVSimReader_HIPSE()
{
}


void KVSimReader_HIPSE::ReadFile()
{
   h1 = new TH1F("impact_parameter", "distri", 200, 0, 20);
   AddObject(h1);
   h1->SetDirectory(nullptr);//avoid double deletion on ROOT file close

   while (IsOK()) {
      while (ReadEvent()) {
         if (nevt % 1000 == 0) Info("ReadFile", "%d evts lus", nevt);
         h1->Fill(evt->GetParameters()->GetDoubleValue("Bparstore"));
         if (HasToFill()) FillTree();
      }
   }

}

Bool_t KVSimReader_HIPSE::ReadHeader()
{

   Int_t res = ReadLineAndCheck(2, " ");
   switch (res) {
      case 0:
         return kFALSE;
      case 1:
         AddInfo("Aproj", GetReadPar(0).Data());
         AddInfo("Zproj", GetReadPar(1).Data());
         proj.SetZandA(GetReadPar(1).Atoi(), GetReadPar(0).Atoi());
         break;
      default:
         return kFALSE;
   }

   res = ReadLineAndCheck(2, " ");
   switch (res) {
      case 0:
         return kFALSE;
      case 1:
         AddInfo("Atarg", GetReadPar(0).Data());
         AddInfo("Ztarg", GetReadPar(1).Data());
         targ.SetZandA(GetReadPar(1).Atoi(), GetReadPar(0).Atoi());

         break;
      default:

         return kFALSE;
   }

   res = ReadLineAndCheck(1, " ");
   switch (res) {
      case 0:
         return kFALSE;
      case 1:
         AddInfo("Ebeam", GetReadPar(0).Data());
         ebeam = GetReadPar(0).Atof();
         return kTRUE;


      default:

         return kFALSE;
   }

}

Bool_t KVSimReader_HIPSE::ReadEvent()
{

   evt->Clear();
   Int_t mult = 0, mtotal = 0;

   /*---------------------------------------------
     mul_vrai = multiplicity of charged particles
     mult     = total multiplicity (i.e. including neutrons)
   //---------------------------------------------
   */
   Int_t res = ReadLineAndCheck(2, " ");
   switch (res) {
      case 0:
         Info("ReadEvent", "case 0 line est vide");
         return kFALSE;
      case 1:
         evt->SetNumber(nevt);
         mult = GetIntReadPar(0);   //mul_vrai
         mtotal = GetIntReadPar(1);
         evt->GetParameters()->SetValue("mult", mtotal);

         break;
      default:

         return kFALSE;
   }

   /*---------------------------------------------
     Esa       = excitation per nucleon
     vcm       = center of mass energy
     Bparstore = impact parameter
   //---------------------------------------------
   */
   res = ReadLineAndCheck(3, " ");
   switch (res) {
      case 0:
         return kFALSE;
      case 1:
         evt->GetParameters()->SetValue("Esa", GetDoubleReadPar(0));
         evt->GetParameters()->SetValue("vcm", GetDoubleReadPar(1));
         evt->GetParameters()->SetValue("Bparstore", GetDoubleReadPar(2));

         break;
      default:

         return kFALSE;
   }

   /*---------------------------------------------
     energetic information
     excitat        : total excitation energy
     xmassav        : Q-value
     ekinav         : total kinetic energy at freeze-out
     epotav         : total potential energy at freeze-out
     erotav         : total rotational energy at freeze-out
   //---------------------------------------------
   */
   res = ReadLineAndCheck(2, " ");
   switch (res) {
      case 0:
         return kFALSE;
      case 1:
         evt->GetParameters()->SetValue("excitat", GetDoubleReadPar(0));
         evt->GetParameters()->SetValue("xmassav", GetDoubleReadPar(1));

         break;
      default:

         return kFALSE;
   }

   res = ReadLineAndCheck(3, " ");
   switch (res) {
      case 0:
         return kFALSE;
      case 1:

         evt->GetParameters()->SetValue("ekinav", GetDoubleReadPar(0));
         evt->GetParameters()->SetValue("epotav", GetDoubleReadPar(1));
         evt->GetParameters()->SetValue("erotav", GetDoubleReadPar(2));
         break;

      default:
         return kFALSE;
   }

   evt->SetNumber(nevt);
   for (Int_t mm = 0; mm < mult; mm += 1) {
      nuc = (KVSimNucleus*)evt->AddParticle();
      if (!ReadNucleus()) return kFALSE;
   }

   nevt += 1;
   return kTRUE;

}

Bool_t KVSimReader_HIPSE::ReadNucleus()
{

   Int_t res = ReadLineAndCheck(3, " ");
   switch (res) {
      case 0:
         Info("ReadNucleus", "case 0 line est vide");
         return kFALSE;

      case 1:
         /*
          proven = 0 -> fusion of the QP and QT
          proven = 1 -> QP
          proven = 2 -> QT
          proven > 2 -> other
          */
         nuc->SetZ(GetIntReadPar(1));
         nuc->SetA(GetIntReadPar(0));
         nuc->GetParameters()->SetValue("proven", GetDoubleReadPar(2));
         break;

      default:
         return kFALSE;
   }

   res = ReadLineAndCheck(3, " ");
   switch (res) {
      case 0:
         Info("ReadNucleus", "case 0 line est vide");
         return kFALSE;

      case 1:
         //Axe "faisceau dans HIPSE x -> on effectue une rotation X,Y,Z -> Y,Z,X"
         nuc->SetMomentum(GetDoubleReadPar(1), GetDoubleReadPar(2), GetDoubleReadPar(0));
         break;

      default:
         return kFALSE;
   }

   int AA = nuc->GetA();

   /*---------------------------------------------------------
     exci(I)  : excitation energy
     ether(I) : not used
     spinx(i),spiny(i),spinz(i) : angular momentum (hbar units)
   //---------------------------------------------
   */
   res = ReadLineAndCheck(2, " ");
   switch (res) {
      case 0:
         Info("ReadNucleus", "case 0 line est vide");
         return kFALSE;

      case 1:
         nuc->SetExcitEnergy(GetDoubleReadPar(0)*AA);
         nuc->GetParameters()->SetValue("exci", GetDoubleReadPar(0));
         nuc->GetParameters()->SetValue("ether", GetDoubleReadPar(1));
         break;

      default:
         return kFALSE;
   }

   res = ReadLineAndCheck(3, " ");
   switch (res) {
      case 0:
         Info("ReadNucleus", "case 0 line est vide");
         return kFALSE;

      case 1:
         //On effectue la meme rotation que les impulsions ... à vérifier
         nuc->SetAngMom(GetDoubleReadPar(1), GetDoubleReadPar(2), GetDoubleReadPar(0));
         return kTRUE;

      default:
         return kFALSE;
   }



}
void KVSimReader_HIPSE::define_output_filename()
{
   // ROOT file called: HIPSE_[PROJ]_[TARG]_[EBEAM]AMeV.root
   // Call after reading file header

   SetROOTFileName(Form("HIPSE_%s_%s_%.1fAMeV.root",
                        proj.GetSymbol(), targ.GetSymbol(), ebeam));
}


void KVSimReader_HIPSE::ConvertEventsInFile(KVString filename)
{
   if (!OpenFileToRead(filename)) return;
   if (!ReadHeader()) return;
   define_output_filename();
   tree_title.Form("HIPSE primary events %s + %s %.1f MeV/nuc.",
                   proj.GetSymbol(), targ.GetSymbol(), ebeam);
   Run();
   CloseFile();
}
