//Created by KVClassFactory on Wed Feb 21 13:43:10 2018
//Author: John Frankland,,,

#include "KVINDRAEtalonGroupReconstructor.h"
#include <string>
#include <unordered_map>
#include <iostream>
#include <KVSilicon.h>
#include "KVIDGCsI.h"
using namespace std;

ClassImp(KVINDRAEtalonGroupReconstructor)


KVReconstructedNucleus* KVINDRAEtalonGroupReconstructor::ReconstructTrajectory(const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node)
{
   // \param traj trajectory currently being scanned
   // \param node current detector on trajectory to test
   // \returns pointer to a new reconstructed particle added to this group's event; nullptr if nothing is to be done
   //
   // Specialised particle reconstruction for INDRA groups with etalon telescopes (Rings 10-17).
   //
   // If node is a CsI detector with more than one trajectory passing through it (in front of it),
   // we are behind the etalons Si(75)-Si(Li).
   //
   // As the Si(75)-Si(Li) coders are opened by every firing of the CsI directly behind them,
   // their acquisition parameters are present in the event whether the particle went through them
   // or passed directly from ChIo to CsI.
   //
   // The best way to choose the right reconstruction trajectory is to see if Si(Li)-CsI
   // provides an identification, and if so is it coherent with that of the CsI.

   if (!node->GetDetector()->IsAnalysed() &&
         node->GetDetector()->Fired(GetPartSeedCond()) &&
         node->GetDetector()->IsType("CSI") &&
         node->GetNTrajForwards() > 1) {
      ++nfireddets;
      const KVSeqCollection* idt_list = traj->GetIDTelescopes();
      TIter next_idt(idt_list);
      KVIDTelescope* idt;
      bool with_etalon = (traj->GetN() == 4);

      std::unordered_map<std::string, KVIdentificationResult> IDR;
      while ((idt = (KVIDTelescope*)next_idt())) {
         if (idt->IsReadyForID()) { // is telescope able to identify for this run ?
            idt->Identify(&IDR[idt->GetType()]);
         }
      }
      // cases:
      //  gamma in CsI: * same as general INDRA reconstruction: count the gammmas, do not
      //                  reconstruct a particle (but set CsI analysed state)
      // particle identified in CsI:
      //                * if this is the long trajectory, check the SILI-CSI identification
      //                  if SILI-CSI identification no good, then short trajectory should be used
      //                  so we return nullptr
      //                * if this is the short trajectory, we accept it
      KVIdentificationResult&  idcsi = IDR["CSI_R_L"];
      if (idcsi.IDattempted) {
         if (idcsi.IDOK) {
            if (idcsi.IDcode == KVINDRA::IDCodes::ID_GAMMA || idcsi.IDquality == KVIDGCsI::kICODE10) {
               // gamma
               //Info("ReconstructTrajectory","Gamma in CsI: with_etalon=%d",with_etalon);
               GetEventFragment()->GetParameters()->IncrementValue("INDRA_GAMMA_MULT", 1);
               GetEventFragment()->GetParameters()->IncrementValue("INDRA_GAMMA_DETS", node->GetName());
               node->GetDetector()->SetAnalysed();
               return nullptr;
            }
            else {   // charged particle identified
               //Info("ReconstructTrajectory","Charged particle in CsI: with_etalon=%d",with_etalon);
               if (with_etalon) {
                  if (IDR["SILI_CSI"].IDattempted && IDR["SILI_CSI"].IDOK) {
                     //Info("ReconstructTrajectory","Charged particle in SILI too Zcsi=%d Zsili=%d",idcsi.Z,IDR["SILI_CSI"].Z);
                     return GetEventFragment()->AddParticle();
                  }
                  else return nullptr;
               }
               else
                  return GetEventFragment()->AddParticle();
            }
         }
      }
      return nullptr;
   }
   // standard non-etalon treatment
   return KVINDRABackwardGroupReconstructor::ReconstructTrajectory(traj, node);
}

bool KVINDRAEtalonGroupReconstructor::DoCoherencyAnalysis(KVReconstructedNucleus& PART)
{
   // Coherency analysis for etalon groups on rings 10-17 of INDRA
   // Note that the treatment of all modules in the group except the one with the etalons
   // is standard (KVINDRABackwardGroupReconstructor), but all modules will call here first

   if (!GetSi75(&PART) && !GetSiLi(&PART)) // => no etalons in particle trajectory
      return KVINDRABackwardGroupReconstructor::DoCoherencyAnalysis(PART);

   PART.SetParameter("UseFullChIoEnergyForCalib", !(theChio && theChio->GetNHits() > 1));
   bool ok = CoherencyEtalons(PART);

   return ok;
}

void KVINDRAEtalonGroupReconstructor::DoCalibration(KVReconstructedNucleus* PART)
{
   // Calibration of particle stopping in etalon modules

   fESiLi = fESi75 = 0;

   if (!GetSi75(PART) && !GetSiLi(PART)) // => no etalons in particle trajectory
      return KVINDRABackwardGroupReconstructor::DoCalibration(PART);

   // change fUseFullChioenergyforcalib for "coherency" particles
   // we assume they are calibrated after all other particles in group have
   // been identified, calibrated, and their energy contributions removed
   // from the ChIo
   if (PART->GetIDCode() == KVINDRA::IDCodes::ID_CI_SI_COHERENCY
         || PART->GetIDCode() == KVINDRA::IDCodes::ID_CI_COHERENCY
         || PART->GetIDCode() == KVINDRA::IDCodes::ID_CI_MULTIHIT)
      PART->SetParameter("UseFullChIoEnergyForCalib", kTRUE);

   Bool_t stopped_in_chio = kTRUE;
   auto csi = GetCsI(PART);
   if (csi) {
      stopped_in_chio = kFALSE;
      if (csi->IsCalibrated()) {
         /* CSI ENERGY CALIBRATION */
         if (PART->GetIDCode() == KVINDRA::IDCodes::ID_CSI_PSA && PART->IsIsotope(4, 8)) {
            fECsI = DoBeryllium8Calibration(PART);
         }
         else
            fECsI = csi->GetCorrectedEnergy(PART, -1., kFALSE);
      }
      else {
         SetNoCalibrationStatus(PART);
         return;
      }
      if (fECsI <= 0 && (PART->GetECode() != KVINDRA::ECodes::SOME_ENERGY_LOSSES_CALCULATED)) { // DoBeryllium8Calibration returns fECsI<0 with ECode=SOME_ENERGY_LOSSES_CALCULATED if OK
         SetBadCalibrationStatus(PART);// problem with CsI energy - no calibration
         return;
      }
   }
   if (PART->GetParameters()->GetBoolValue("IncludeEtalonsInCalibration")) {
      auto sili = GetSiLi(PART);
      if (sili) {
         bool stopped_in_sili = PART->GetStoppingDetector()->IsType("SILI");
         Double_t ERES = TMath::Abs(fECsI);
         if (!PART->GetParameters()->GetBoolValue("PileupSiLi") && sili->IsCalibrated()) {
            Bool_t si_transmission = kTRUE;
            if (stopped_in_sili) {
               si_transmission = kFALSE;
            }
            else {
               sili->SetEResAfterDetector(ERES);
            }
            fESiLi = sili->GetCorrectedEnergy(PART, -1., si_transmission);
            if (fESiLi <= 0) {
               if (!stopped_in_sili && ERES > 0.0) {
                  if (!CalculateSiLiDEFromResidualEnergy(ERES, sili, PART)) return;
               }
               else {
                  SetBadCalibrationStatus(PART);
                  return;
               }
            }
         }
         else {
            if (!CalculateSiLiDEFromResidualEnergy(ERES, sili, PART)) return;
         }
      }
      auto si75 = GetSi75(PART);
      if (si75) {
         bool stopped_in_si75 = PART->GetStoppingDetector()->IsType("SI75");
         Double_t ERES = TMath::Abs(fECsI) + TMath::Abs(fESiLi);
         if (!PART->GetParameters()->GetBoolValue("PileupSi75")
               && !PART->GetParameters()->GetBoolValue("PileupSiLi")
               && si75->IsCalibrated()) {
            Bool_t si_transmission = kTRUE;
            if (stopped_in_si75) {
               si_transmission = kFALSE;
            }
            else {
               si75->SetEResAfterDetector(ERES);
            }
            fESi75 = si75->GetCorrectedEnergy(PART, -1., si_transmission);
            if (fESi75 <= 0) {
               if (!stopped_in_si75 && ERES > 0.0) {
                  if (!CalculateSi75DEFromResidualEnergy(ERES, si75, PART)) return;
               }
               else {
                  SetBadCalibrationStatus(PART);
                  return;
               }
            }
         }
         else {
            if (!CalculateSi75DEFromResidualEnergy(ERES, si75, PART)) return;
         }
      }
   }
   if (theChio) {
      /* IONISATION CHAMBER ENERGY CONTRIBUTION */
      // if fUseFullChIoEnergyForCalib = kFALSE, ChIo was hit by other particles in group
      //     therefore we have to estimate the ChIo energy for this particle using the CsI energy
      // if fPileupChIo = kTRUE, there appears to be another particle stopped in the ChIo
      //     therefore we have to estimate the ChIo energy for this particle using the CsI energy
      Double_t ERES = TMath::Abs(fECsI) + TMath::Abs(fESiLi) + TMath::Abs(fESi75);
      if (!PART->GetParameters()->GetBoolValue("PileupChIo")
            && PART->GetParameters()->GetBoolValue("UseFullChIoEnergyForCalib")
            && theChio->IsCalibrated()) {
         // all is apparently well
         Bool_t ci_transmission = kTRUE;
         if (stopped_in_chio) {
            ci_transmission = kFALSE;
         }
         else {
            theChio->SetEResAfterDetector(ERES);
         }
         fEChIo = theChio->GetCorrectedEnergy(PART, -1., ci_transmission);
         if (fEChIo <= 0) {
            if (!stopped_in_chio && ERES > 0) {
               if (!CalculateChIoDEFromResidualEnergy(PART, ERES)) return;
            }
         }
      }
      else {
         if (!stopped_in_chio && ERES > 0) {
            if (!CalculateChIoDEFromResidualEnergy(PART, ERES)) return;
         }
      }
   }

   PART->SetEnergy(TMath::Abs(fECsI) + TMath::Abs(fESiLi) + TMath::Abs(fESi75) + TMath::Abs(fEChIo));
}

Bool_t KVINDRAEtalonGroupReconstructor::CoherencyEtalons(KVReconstructedNucleus& PART)
{
   // Called by Identify() for particles stopping in etalon modules of Rings 10-17.

   KVIdentificationResult* IDcsi, *IDsilicsi, *IDsi75sili, *IDcisi75, *IDcicsi;
   IDcsi = IDsilicsi = IDsi75sili = IDcisi75 = IDcicsi = nullptr;
   if (PART.GetStoppingDetector()->IsType("CSI")) {
      IDcsi = PART.GetIdentificationResult("CSI_R_L");
      IDcicsi = PART.GetIdentificationResult("CI_CSI");
      IDsilicsi = PART.GetIdentificationResult("SILI_CSI");
   }
   IDsi75sili = PART.GetIdentificationResult("SI75_SILI");
   IDcisi75 = PART.GetIdentificationResult("CI_SI75");

   PART.SetParameter("PileupChIo", kFALSE);
   PART.SetParameter("IncludeEtalonsInCalibration", kTRUE);

   Bool_t haveCsI = IDcsi && IDcsi->IDOK;
   Bool_t haveSiLiCsI = IDsilicsi && IDsilicsi->IDOK;
   Bool_t haveSi75SiLi = IDsi75sili && IDsi75sili->IDOK;
   Bool_t haveChIoSi75 = IDcisi75 && IDcisi75->IDOK;

   KVIDTelescope* idt_csi, *idt_silicsi, *idt_si75sili, *idt_cisi75, *idt_cicsi;
   idt_csi = idt_silicsi = idt_si75sili = idt_cisi75 = idt_cicsi = nullptr;
   if (PART.GetStoppingDetector()->IsType("CSI")) {
      idt_csi =
         (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType(IDcsi->GetType());
      idt_silicsi =
         (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType(IDsilicsi->GetType());
      idt_cicsi =
         (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType(IDcicsi->GetType());
   }
   idt_si75sili = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType(IDsi75sili->GetType());
   idt_cisi75 = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType(IDcisi75->GetType());

   // Treat cases where particle hit etalon telescope
   if (idt_csi) {
      if (haveCsI) {

         partID = *IDcsi;
         identifying_telescope = idt_csi;

         if (haveSi75SiLi) {
            // check for heavy fragment in Si75-SiLi
            if (IDsi75sili->Z > partID.Z) {
               if (haveChIoSi75) {
                  // check we don't have a better identification in ChIo-Si75
                  if (IDcisi75->IDquality < IDsi75sili->IDquality && IDcisi75->Z > partID.Z) {
                     PART.SetParameter("PileupSi75", kTRUE);
                     IDcisi75->SetComment("CsI identification with another particle stopped in Si75");
                     PART.SetParameter("UseFullChIoEnergyForCalib", kFALSE); // calculate ChIo energy for this particle
                  }
                  else {
                     PART.SetParameter("PileupSiLi", kTRUE);
                     IDsi75sili->SetComment("CsI identification with another particle stopped in SiLi");
                     PART.SetParameter("UseFullChIoEnergyForCalib", kFALSE); // calculate ChIo energy for this particle
                  }
               }
            }
         }
         else if (haveChIoSi75) {
            // check for heavy fragment in ChIo-Si75
            if (IDcisi75->Z > partID.Z) {
               PART.SetParameter("PileupSi75", kTRUE);
               IDcisi75->SetComment("CsI identification with another particle stopped in Si75");
               PART.SetParameter("UseFullChIoEnergyForCalib", kFALSE); // calculate ChIo energy for this particle
            }
         }
         return kTRUE;
      }
      else if (haveSiLiCsI) {
         partID = *IDsilicsi;
         identifying_telescope = idt_silicsi;
         if (haveChIoSi75) {
            // check for heavy fragment in ChIo-Si75
            if (IDcisi75->Z > partID.Z) {
               PART.SetParameter("PileupSi75", kTRUE);
               IDcisi75->SetComment("CsI identification with another particle stopped in Si75");
               PART.SetParameter("UseFullChIoEnergyForCalib", kFALSE); // calculate ChIo energy for this particle
            }
         }
         return kTRUE;
      }
   }
   else if (PART.GetStoppingDetector()->IsType("SILI")) {
      if (haveSi75SiLi) {
         partID = *IDsi75sili;
         identifying_telescope = idt_si75sili;
         // check ChIo-Si75 id is coherent (either no id or Z<=this one)
         if (haveChIoSi75) {
            if (IDcisi75->Z > partID.Z) PART.SetParameter("PileupChIo", kTRUE);
         }
         return kTRUE;
      }
      else if (haveChIoSi75) {
         partID = *IDcisi75;
         identifying_telescope = idt_cisi75;
         return kTRUE;
      }
   }
   else if (PART.GetStoppingDetector()->IsType("SI75")) {   // MFR march 2014 SiLi is not the real stopping detector
      if (haveChIoSi75) {
         partID = *IDcisi75;
         identifying_telescope = idt_cisi75;
         return kTRUE;
      }                       // end MFR march 2014
   }

   return kFALSE;
}

Bool_t KVINDRAEtalonGroupReconstructor::CalculateSiLiDEFromResidualEnergy(Double_t ERES, KVDetector* sili, KVReconstructedNucleus* n)
{
   // Etalon modules
   // calculate fESiLi from residual CsI energy
   Double_t e0 = sili->GetDeltaEFromERes(n->GetZ(), n->GetA(), ERES);
   sili->SetEResAfterDetector(ERES);
   fESiLi = sili->GetCorrectedEnergy(n, e0);
   if (fESiLi <= 0) {
      fESiLi = 0;
      SetBadCalibrationStatus(n);
      return kFALSE;
   }
   fESiLi = -TMath::Abs(fESiLi);
   SetCalibrationStatus(*n, KVINDRA::ECodes::SOME_ENERGY_LOSSES_CALCULATED);
   return kTRUE;
}

Bool_t KVINDRAEtalonGroupReconstructor::CalculateSi75DEFromResidualEnergy(Double_t ERES, KVDetector* si75, KVReconstructedNucleus* n)
{
   // Etalon modules
   // calculate fESi75 from residual CsI+SiLi energy
   Double_t e0 = si75->GetDeltaEFromERes(n->GetZ(), n->GetA(), ERES);
   si75->SetEResAfterDetector(ERES);
   fESi75 = si75->GetCorrectedEnergy(n, e0);
   if (fESi75 <= 0) {
      fESi75 = 0;
      SetBadCalibrationStatus(n);
      return kFALSE;
   }
   fESi75 = -TMath::Abs(fESi75);
   SetCalibrationStatus(*n, KVINDRA::ECodes::SOME_ENERGY_LOSSES_CALCULATED);
   return kTRUE;
}
