//Created by KVClassFactory on Tue Feb 27 11:35:25 2018
//Author: John Frankland,,,

#include "KVINDRABackwardGroupReconstructor.h"

ClassImp(KVINDRABackwardGroupReconstructor)

bool KVINDRABackwardGroupReconstructor::DoCoherencyAnalysis(KVReconstructedNucleus& PART)
{
   // Coherency analysis for backward rings 10-17 of INDRA

   PART.SetParameter("UseFullChIoEnergyForCalib", !(theChio && theChio->GetNHits() > 1));
   bool ok = false;
   if (PART.GetStoppingDetector()->IsType("CSI")) {
      // particles stopping in CsI detectors
      // check coherency of CsI-R/L and Si-CsI identifications
      ok = CoherencyChIoCsI(PART);
   }
   else {
      // particle stopped in ChIo (=> Zmin)
      ok = PART.IsIdentified() && identifying_telescope;
   }

   return ok;
}

void KVINDRABackwardGroupReconstructor::DoCalibration(KVReconstructedNucleus* PART)
{
   // Special calibration for particles in rings 10 to 17
   // We set the energy calibration code for the particle here
   //    kECode0 = no calibration (e.g. gammas)
   //    kECode1 = everything OK
   //    kECode2 = small warning, for example if energy loss in a detector is calculated
   //    kECode15 = bad, calibration is no good
   // The contributions from ChIo & CsI are stored in particle parameters
   // INDRA.ECHIO and INDRA.ECSI
   // If the contribution is calculated rather than measured, it is stored as a negative value

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
      if (csi->IsCalibrated() && csi->GetDetectorSignalValue("TotLight") > 0) {
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

   if (theChio) {
      /* IONISATION CHAMBER ENERGY CONTRIBUTION */
      // if fUseFullChIoEnergyForCalib = kFALSE, ChIo was hit by other particles in group
      //     therefore we have to estimate the ChIo energy for this particle using the CsI energy
      // if fPileupChIo = kTRUE, there appears to be another particle stopped in the ChIo
      //     therefore we have to estimate the ChIo energy for this particle using the CsI energy
      Double_t ERES = TMath::Abs(fECsI);
      if (!PART->GetParameters()->GetBoolValue("PileupChIo") &&
            PART->GetParameters()->GetBoolValue("UseFullChIoEnergyForCalib")
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
         else {
            // particle stopped in ChIo, no calibration available
            SetNoCalibrationStatus(PART);
            return;
         }
      }
   }

   PART->SetEnergy(TMath::Abs(fECsI) + TMath::Abs(fEChIo));
}

Bool_t KVINDRABackwardGroupReconstructor::CoherencyChIoCsI(KVReconstructedNucleus& PART)
{
   // Called by Identify() for particles stopping in CsI detectors on rings 10-17,
   // which have a ChIo detector just in front of them.
   //
   // fPileupChIo = kTRUE if ChIo-CsI identification gives Z >> CsI-R/L identification
   //              this means that the particle identified in CsI-R/L is correct,
   //              and there is probably a second particle which stopped in the ChIo
   //              detector at the same time (will be added as a Zmin/code5)

   KVIdentificationResult* IDcsi = PART.GetIdentificationResult(1);
   KVIdentificationResult* IDcicsi = PART.GetIdentificationResult(2);
   KVIDTelescope* idt_cicsi = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType(IDcicsi->GetType());
   KVIDTelescope* idt_csi = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType(IDcsi->GetType());

   PART.SetParameter("PileupChIo", kFALSE);

   // Unsuccessful/no CsI id attempt with successful ChIo-CsI id ?
   // Then use ChIo-CsI identification result
   if (IDcsi && !IDcsi->IDOK) {
      if (IDcicsi && IDcicsi->IDOK) {
         partID = *IDcicsi;
         identifying_telescope = idt_cicsi;
         return kTRUE;
      }
   }

   // check coherency of CsI-R/L and ChIo-CsI identifications
   if (IDcsi && IDcsi->IDOK) {

      // We check the coherency of the identifications
      // Because ChIo-Csi identification is not of very high quality (compared to CsI R-L),
      // we only check that the Z given by ChIo-CsI is < Zref+1
      // If not, we can suspect the presence of another particle in the ChIo

      if (IDcicsi && IDcicsi->IDOK) {
         Int_t Zref = IDcsi->Z;
         if (IDcicsi->Z > (Zref + 1) && PART.GetParameters()->GetBoolValue("UseFullChIoEnergyForCalib")) {
            PART.SetParameter("PileupChIo", kTRUE);
            IDcicsi->SetComment("Possible pile-up in ChIo");
         }
      }

      // in all other cases accept CsI identification
      partID = *IDcsi;
      identifying_telescope = idt_csi;
      return kTRUE;
   }
   return kFALSE;
}
