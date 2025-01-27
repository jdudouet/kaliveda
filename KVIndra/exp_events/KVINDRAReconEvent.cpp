/***************************************************************************
                          kvindrareconevent.cpp  -  description
                             -------------------
    begin                : Thu Oct 10 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Riostream.h"
#include "TROOT.h"
#include "KVINDRAReconEvent.h"
#include "KVList.h"
#include "KVGroup.h"
#include "KVTelescope.h"
#include "KVDetector.h"
#include "KVCsI.h"
#include "KVSilicon.h"
#include "KVDetectorEvent.h"
#include "KVINDRAReconNuc.h"
#include "KVINDRA.h"
#include "TStreamerInfo.h"
#include "KVIDCsI.h"
#include "KVIDGCsI.h"
#include "KVDataSet.h"
#include "KVChIo.h"

using namespace std;

ClassImp(KVINDRAReconEvent);

void KVINDRAReconEvent::init()
{
   //default initialisations
   fCodeMask = 0;
   fHitGroups = 0;
}


KVINDRAReconEvent::KVINDRAReconEvent(Int_t mult, const char* classname)
   : KVReconstructedEvent(mult, classname)
{
   init();
}

KVINDRAReconEvent::~KVINDRAReconEvent()
{
   if (fCodeMask) {
      delete fCodeMask;
      fCodeMask = 0;
   }
   SafeDelete(fHitGroups);
};

/////////////////////////////////////////////////////////////////////////////

KVINDRAReconNuc* KVINDRAReconEvent::GetParticle(Int_t npart) const
{
   //
   //Access to event member with index npart (1<=npart<=fMult)
   //

   return (KVINDRAReconNuc*)(KVReconstructedEvent::GetParticle(npart));
}

/////////////////////////////////////////////////////////////////////////////

KVINDRAReconNuc* KVINDRAReconEvent::AddParticle()
{
   //Wrapper for KVEvent::GetNextParticle casting result to KVINDRAReconNuc*

   KVINDRAReconNuc* tmp = (KVINDRAReconNuc*)(KVReconstructedEvent::AddParticle());
   return tmp;
}

//______________________________________________________________________________
void KVINDRAReconEvent::Print(Option_t* option) const
{
   //Print out list of particles in the event.
   //If option="ok" only particles with IsOK=kTRUE are included.

   cout << GetTitle() << endl;  //system
   cout << GetName() << endl;   //run
   cout << "Event number: " << GetNumber() << endl << endl;
   cout << "MULTIPLICITY = " << ((KVINDRAReconEvent*) this)->
        GetMult(option) << endl << endl;

   KVINDRAReconNuc* frag = 0;
   while ((frag = ((KVINDRAReconEvent*) this)->GetNextParticle(option))) {
      frag->Print();
   }
}

KVINDRAReconNuc* KVINDRAReconEvent::GetNextParticle(Option_t* opt)
{
   //Use this method to iterate over the list of particles in the event.
   //
   //If opt="ok" only the particles whose ID codes and E codes correspond to those set using AcceptIDCodes and
   //AcceptECodes will be returned, in the order in which they appear in the event.
   //
   //After the last particle in the event GetNextParticle() returns a null pointer and
   //resets itself ready for a new iteration over the particle list.
   //Therefore, to loop over all particles in an event, use a structure like:
   //
   //      KVINDRAReconNuc* p;
   //      while( (p = GetNextParticle() ){
   //              ... your code here ...
   //      }

   return (KVINDRAReconNuc*) KVEvent::GetNextParticle(opt);
}

//______________________________________________________________________________________________

void KVINDRAReconEvent::ChangeFragmentMasses(UChar_t mass_formula)
{
   //Changes the mass formula used to calculate A from Z for all nuclei in event
   //For the values of mass_formula, see KVNucleus::GetAFromZ
   //
   //The fragment energy is modified in proportion to its mass, this is due to the
   //contribution from the CsI light-energy calibration:
   //
   //    E -> E + E_CsI*( newA/oldA - 1 )
   //
   //From an original lunch by Remi Bougault.
   //
   //Only particles with 'acceptable' ID & E codes stopping in (or passing through)
   //a CsI detector are affected; particles whose mass was measured
   //(i.e. having KVReconstructedNucleus::IsAMeasured()==kTRUE)
   //are not affected by the change of mass formula.

   ResetGetNextParticle();
   KVINDRAReconNuc* frag;
   while ((frag = (KVINDRAReconNuc*)GetNextParticle("ok"))) {

      if (!frag->IsAMeasured()) {

         Float_t oldA = (Float_t)frag->GetA();
         frag->SetMassFormula(mass_formula);

         if (frag->GetCsI()) {
            Float_t oldECsI = frag->GetEnergyCsI();
            Float_t oldE = frag->GetEnergy();
            Float_t newECsI = oldECsI * ((Float_t)frag->GetA() / oldA);
            frag->GetCsI()->SetEnergy(newECsI);
            frag->SetEnergy(oldE - oldECsI + newECsI);
         }
      }
   }

}

//____________________________________________________________________________

void KVINDRAReconEvent::IdentifyEvent()
{
   // Performs event identification (see KVReconstructedEvent::IdentifyEvent), and then
   // particles stopping in first member of a telescope (GetStatus() == KVReconstructedNucleus::kStatusStopFirstStage) are
   // labelled with VEDA ID code kIDCode5 (Zmin)
   //
   //   When CsI identification gives a gamma, we unset the 'analysed' state of all detectors
   // in front of the CsI and reanalyse the group in order to reconstruct and identify charged particles
   // stopping in them.
   //
   // Unidentified particles receive the general ID code for non-identified particles (kIDCode14)

   KVReconstructedEvent::IdentifyEvent();
   KVINDRAReconNuc* d = 0;
   int mult = GetMult();
   KVUniqueNameList gammaGroups;//list of groups with gammas identified in CsI
   ResetGetNextParticle();

   while ((d = GetNextParticle())) {
      if (d->IsIdentified() && d->GetStatus() == KVReconstructedNucleus::kStatusStopFirstStage) {
         d->SetIDCode(kIDCode5);   // Zmin
      }
      else if (d->IsIdentified() && d->GetCodes().TestIDCode(kIDCode0)) {
         // gamma identified in CsI
         // reset analysed state of all detectors in front of CsI
         if (d->GetCsI()) {
            if (d->GetCsI()->GetAlignedDetectors()) {
               TIter next(d->GetCsI()->GetAlignedDetectors());
               KVDetector* det = (KVDetector*)next(); //first detector = CsI
               while ((det = (KVDetector*)next())) det->SetAnalysed(kFALSE);
               gammaGroups.Add(d->GetGroup());
            }
            else {
               Error("IdentifyEvent", "particule id gamma, no aligned detectors???");
               d->Print();
            }
         }
         else {
            Error("IdentifyEvent", "particule identified as gamma, has no CsI!!");
            d->Print();
         }
      }
   }

   // perform secondary reconstruction in groups with detected gammas
   int ngamG = gammaGroups.GetEntries();
   if (ngamG) {
      for (int i = 0; i < ngamG; i++) {
         gIndra->AnalyseGroupAndReconstructEvent(this, (KVGroup*)gammaGroups.At(i));
      }
   }
   if (GetMult() > mult) {
      /*Info("IdentifyEvent", "Event#%d: Secondary reconstruction (gammas) -> %d new particles",
         GetNumber(), GetMult()-mult);*/

      // identify new particles generated in secondary reconstruction
      KVReconstructedEvent::IdentifyEvent();
      ResetGetNextParticle();

      while ((d = GetNextParticle())) {
         if (d->IsIdentified() && d->GetStatus() == KVReconstructedNucleus::kStatusStopFirstStage) {
            d->SetIDCode(kIDCode5);   // Zmin
         }
         else if (!d->IsIdentified()) {
            d->SetIDCode(kIDCode14);
         }
      }
      /*
      for(int i=mult+1; i<=GetMult(); i++){
         d = GetParticle(i);
         if(d->IsIdentified())
            printf("\t%2d: Ring %2d Module %2d Z=%2d  A=%3d  code=%d\n",i,d->GetRingNumber(),
                  d->GetModuleNumber(),d->GetZ(),d->GetA(),d->GetCodes().GetVedaIDCode());
         else
            printf("\t%2d: Ring %2d Module %2d UNIDENTIFIED status=%d\n", i,d->GetRingNumber(),
                  d->GetModuleNumber(), d->GetStatus());
      }
      */
   }
}

void KVINDRAReconEvent::SecondaryIdentCalib()
{
   // Perform identifications and calibrations of particles not included
   // in first round (methods IdentifyEvent() and CalibrateEvent()).
   //
   // Here we treat particles with GetStatus()==KVReconstructedNucleus::kStatusOKafterSub
   // after subtracting the energy losses of all previously calibrated particles in group from the
   // measured energy losses in the detectors they crossed.

   if (!fHitGroups)
      fHitGroups = new KVUniqueNameList;
   else
      fHitGroups->Clear();
   // build list of hit groups
   KVINDRAReconNuc* d;
   while ((d = GetNextParticle())) fHitGroups->Add(d->GetGroup());

   //loop over hit groups
   TIter next_grp(fHitGroups);
   KVGroup* grp;
   while ((grp = (KVGroup*)next_grp())) {
      SecondaryAnalyseGroup(grp);
   }

   // set "unidentified" code for any remaining unidentified particle
   ResetGetNextParticle();
   while ((d = GetNextParticle())) {
      if (!d->IsIdentified()) {
         d->SetIDCode(kIDCode14);
      }
   }
}

void KVINDRAReconEvent::SecondaryAnalyseGroup(KVGroup*)
{
   Obsolete("SecondaryAnalyseGroup", "1.11", "2.0");
}

