#include "KVFAZIAGroupReconstructor.h"

#include <KVFAZIA.h>
#include <KVFAZIADetector.h>
#include <KVSignal.h>
#include <KVLightEnergyCsIFull.h>
#include <KVLightEnergyCsI.h>
#include <KVCalibrator.h>
#include <KVIDGCsI.h>
#include <KVCalibratedSignal.h>

ClassImp(KVFAZIAGroupReconstructor)

void KVFAZIAGroupReconstructor::CalibrateParticle(KVReconstructedNucleus* PART)
{
   // Perform energy calibration of (previously identified) charged particle
   //
   // "Gamma" particles identified in CsI are given the equivalent energy of proton,
   // if the CsI detector is calibrated

   KVFAZIADetector* det = (KVFAZIADetector*)PART->GetStoppingDetector();

   if (det->IsType("CsI") && PART->GetIDCode() == 0) { // gammas
      if (det->IsCalibrated()) {
         double Ep = det->GetDetectorSignalValue("Energy", "Z=1,A=1");
         if (Ep > 0) {
            PART->SetEnergy(Ep);
            PART->SetIsCalibrated();
            PART->SetECode(1);
            PART->SetParameter("FAZIA.ECSI", Ep);
         }
      }
   }

   if (PART->GetIDCode() == 5) { // stopped in SI1, no PSA identification
      if (det->IsCalibrated()) {
         double Ep = det->GetEnergy();
         PART->SetEnergy(Ep);
         PART->SetIsCalibrated();
         PART->SetECode(1);
         PART->SetParameter("FAZIA.ESI1", Ep);
      }
   }

   // particle identified in Si1 PSA, detector is calibrated
   if (PART->GetIDCode() == 11) {
      KVFAZIADetector* si1 = (KVFAZIADetector*)PART->GetIdentifyingTelescope()->GetDetector(1);
      if (si1->IsCalibrated()) {
         double e1 = si1->GetCorrectedEnergy(PART, -1., kFALSE);
         if (e1 <= 0) {
            Warning("CalibrateParticle",
                    "IDCODE=11 Z=%d A=%d calibrated SI1 E=%f",
                    PART->GetZ(), PART->GetA(), e1);
            return;
         }
         PART->SetParameter("FAZIA.ESI1", e1);
         PART->SetEnergy(e1);
         PART->SetIsCalibrated();
         PART->SetECode(1);
      }
   }

   // particle identified in Si1-Si2
   if (PART->GetIDCode() == 12) {
      KVFAZIADetector* si1 = (KVFAZIADetector*)PART->GetIdentifyingTelescope()->GetDetector(1);
      KVFAZIADetector* si2 = (KVFAZIADetector*)PART->GetIdentifyingTelescope()->GetDetector(2);
      if (si1->IsCalibrated() && si2->IsCalibrated()) { //  with both detectors calibrated
         double e1(-1), e2(-1);
         bool calc_e1(false), calc_e2(false);
         if ((e2 = si2->GetEnergy()) > 0) {
            if (!((e1 = si1->GetEnergy()) > 0)) { // if SI1 not fired, we calculate from SI2 energy
               e1 = si1->GetDeltaEFromERes(PART->GetZ(), PART->GetA(), e2);
               calc_e1 = true;
            }
         }
         else if ((e1 = si1->GetEnergy()) > 0) {
            // if SI2 not fired, we calculate from SI1 energy
            e2 = si1->GetEResFromDeltaE(PART->GetZ(), PART->GetA());
            calc_e2 = true;
         }

         PART->SetParameter("FAZIA.ESI1", (calc_e1 ? -e1 : e1));
         PART->SetParameter("FAZIA.ESI2", (calc_e2 ? -e2 : e2));
         if (e1 > 0 && e2 > 0) {
            PART->SetEnergy(e1 + e2);
            PART->SetIsCalibrated();
            PART->SetECode((calc_e1 || calc_e2) ? 2 : 1);
         }
      }
   }

   // particle identified in Si2-CsI
   if (PART->GetIDCode() == 23) {
      KVFAZIADetector* si2 = (KVFAZIADetector*)PART->GetReconstructionTrajectory()->GetDetector("SI2");
      KVFAZIADetector* csi = (KVFAZIADetector*)PART->GetStoppingDetector();
      KVFAZIADetector* si1 = (KVFAZIADetector*)PART->GetReconstructionTrajectory()->GetDetector("SI1");

      KVNameValueList part_id(Form("Z=%d,A=%d", PART->GetZ(), PART->GetA()));

      if (!(si1 && si2 && csi)) {
         Error("CalibrateParticle",
               "IDCODE=23 si1=%s si2=%s csi=%s",
               (si1 ? si1->GetName() : "?"), (si2 ? si2->GetName() : "?"), (csi ? csi->GetName() : "?"));
      }
      if (csi->IsCalibrated(part_id) && si1->IsCalibrated() && si2->IsCalibrated()) {
         // treat case of all detectors calibrated
         double esi1 = si1->GetEnergy();
         double esi2 = si2->GetEnergy();
         double ecsi = csi->GetDetectorSignalValue("Energy", part_id);
         PART->SetParameter("FAZIA.ESI1", esi1);
         PART->SetParameter("FAZIA.ESI2", esi2);
         PART->SetParameter("FAZIA.ECSI", ecsi);
         PART->SetEnergy(esi1 + esi2 + ecsi);
         PART->SetIsCalibrated();
         PART->SetECode(1); // all energies calibrated
      }
      else {
         // treat case of uncalibrated CsI detector
         // case where SI1 && SI2 are calibrated & present in event
         // (STRICTLY SPEAKING, FIRST NEED TO CHECK THAT NOTHING ELSE STOPPED IN SI1 (for Si2-CsI id) or SI2 (for CsI id)):
         // THIS SHOULD BE DONE IN IDENTIFICATION COHERENCY CHECKS
         if (si1->IsCalibrated() && si1->GetEnergy() && si2->IsCalibrated() && si2->GetEnergy()) {
            // calculate total delta-E in (SI1+SI2) then use to calculate CsI energy
            double deltaE = si1->GetEnergy() + si2->GetEnergy();
            KVDetector si1si2("Si", si1->GetThickness() + si2->GetThickness());
            double ecsi = si1si2.GetEResFromDeltaE(PART->GetZ(), PART->GetA(), deltaE);
            PART->SetParameter("FAZIA.ESI1", si1->GetEnergy());
            PART->SetParameter("FAZIA.ESI2", si2->GetEnergy());
            PART->SetParameter("FAZIA.ECSI", -ecsi);
            PART->SetEnergy(deltaE + ecsi);
            PART->SetIsCalibrated();
            PART->SetECode(2); // CsI energy calculated
            //PART->ls();
         }
      }
   }
   // particle identified in CsI
   if (PART->GetIDCode() == 33) {
      KVFAZIADetector* si2 = (KVFAZIADetector*)PART->GetReconstructionTrajectory()->GetDetector("SI2");
      KVFAZIADetector* csi = (KVFAZIADetector*)PART->GetStoppingDetector();
      KVFAZIADetector* si1 = (KVFAZIADetector*)PART->GetReconstructionTrajectory()->GetDetector("SI1");

      bool si1_pileup = PART->GetParameters()->GetBoolValue("si1_pileup");
      bool si2_pileup = PART->GetParameters()->GetBoolValue("si2_pileup");

      KVNameValueList part_id(Form("Z=%d,A=%d", PART->GetZ(), PART->GetA()));

      if (!(si1 && si2 && csi)) {
         Error("CalibrateParticle",
               "IDCODE=23 si1=%s si2=%s csi=%s",
               (si1 ? si1->GetName() : "?"), (si2 ? si2->GetName() : "?"), (csi ? csi->GetName() : "?"));
      }
      double esi1 = -1;
      double esi2 = -1;
      double ecsi = -1;
      std::map<std::string, bool> is_calculated;
      is_calculated["csi"] = is_calculated["si2"] = is_calculated["si1"] = false;
      if (csi->IsCalibrated(part_id)) ecsi = csi->GetDetectorSignalValue("Energy", part_id);
      if (!si2_pileup && si2->IsCalibrated()) esi2 = si2->GetEnergy();
      if (!si1_pileup && si1->IsCalibrated()) esi1 = si1->GetEnergy();
      // pile-up in Si2 with calibrated CsI: calculate esi1 & esi2
      if (si2_pileup && csi->IsCalibrated(part_id) && ecsi > 0) {
         esi2 = si2->GetDeltaEFromERes(PART->GetZ(), PART->GetA(), ecsi);
         is_calculated["si2"] = true;
         esi1 = si1->GetDeltaEFromERes(PART->GetZ(), PART->GetA(), esi2 + ecsi);
         is_calculated["si1"] = true;
      }
      // pile-up in Si1 with calibrated CsI & calibrated Si2: calculate esi1
      else if (si1_pileup && csi->IsCalibrated(part_id) && si2->IsCalibrated() && ecsi > 0 && esi2 > 0) {
         esi1 = si1->GetDeltaEFromERes(PART->GetZ(), PART->GetA(), esi2 + ecsi);
         is_calculated["si1"] = true;
      }
      // treat case of uncalibrated CsI detector
      // case where SI1 && SI2 are calibrated & present in event & no pileup detected in Si1/Si2
      if (!csi->IsCalibrated(part_id) && (si1->IsCalibrated() && si1->GetEnergy() && si2->IsCalibrated() && si2->GetEnergy())
            && !si1_pileup && !si2_pileup) {
         // calculate total delta-E in (SI1+SI2) then use to calculate CsI energy
         double deltaE = esi1 + esi2;
         KVDetector si1si2("Si", si1->GetThickness() + si2->GetThickness());
         ecsi = si1si2.GetEResFromDeltaE(PART->GetZ(), PART->GetA(), deltaE);
         is_calculated["csi"] = true;
      }

      if (ecsi > 0 && esi1 > 0 && esi2 > 0) {
         PART->SetIsCalibrated();
         if (is_calculated["si1"] || is_calculated["si2"] || is_calculated["csi"]) PART->SetECode(2);
         else PART->SetECode(1);
         PART->SetEnergy(esi1 + esi2 + ecsi);
         PART->SetParameter("FAZIA.ESI1", is_calculated["si1"] ? -esi1 : esi1);
         PART->SetParameter("FAZIA.ESI2", is_calculated["si2"] ? -esi2 : esi2);
         PART->SetParameter("FAZIA.ECSI", is_calculated["csi"] ? -ecsi : ecsi);
      }
      else
         PART->SetECode(0); // aucune calibration effectuee
   }

   if (PART->IsCalibrated()) {
      // check for energy loss coherency
      KVNucleus avatar;
      avatar.SetZAandE(PART->GetZ(), PART->GetA(), PART->GetKE());

      int ndet = 0;
      const char* detnames[] = {"SI1", "SI2", "CSI"};
      KVGeoDetectorNode* node = 0;
      // iterating over detectors starting from the target
      // compute the theoretical energy loss of the avatar
      // compare to the calibrated/calculated energy
      // remove this energy from the avatar energy
      PART->GetReconstructionTrajectory()->IterateBackFrom();
      while ((node = PART->GetReconstructionTrajectory()->GetNextNode())) {
         det = (KVFAZIADetector*)node->GetDetector();
         Double_t temp = det->GetELostByParticle(&avatar);
         PART->SetParameter(Form("FAZIA.avatar.E%s", detnames[det->GetIdentifier()]), temp);
         avatar.SetKE(avatar.GetKE() - temp);
         ndet++;
      }

      //add correction for target energy loss - moving charged particles only!
      Double_t E_targ = 0.;
      if (PART->GetZ() && PART->GetEnergy() > 0) {
         E_targ = GetTargetEnergyLossCorrection(PART);
         PART->SetTargetEnergyLoss(E_targ);
      }
      Double_t E_tot = PART->GetEnergy() + E_targ;
      PART->SetEnergy(E_tot);
      // set particle momentum from telescope dimensions (random)
      PART->GetAnglesFromReconstructionTrajectory();
      //PART->ls();
   }
}

void KVFAZIAGroupReconstructor::PostReconstructionProcessing()
{
   // Copy FPGA energy values to reconstructed particle parameter lists
   // Set values in detectors for identification/calibration procedures

   for (KVEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
      KVReconstructedNucleus* rnuc = it.get_pointer<KVReconstructedNucleus>();

      rnuc->GetReconstructionTrajectory()->IterateFrom();

      KVGeoDetectorNode* node;
      while ((node = rnuc->GetReconstructionTrajectory()->GetNextNode())) {

         KVFAZIADetector* det = (KVFAZIADetector*)node->GetDetector();

         // now copy all detector signals to reconstructed particle parameter list...
         // they are stored with format "[detname].[signal_name]" except for
         // DetTag and GTTag which are the same for all detectors of the same telescope
         // and so are only stored once with name "DetTag" or "GTTag".
         TIter it(&det->GetListOfDetectorSignals());
         KVDetectorSignal* ds;
         while ((ds = (KVDetectorSignal*)it())) {
            if (ds->IsRaw() && !ds->IsExpression())
               // only store raw data, excluding any expressions based only on raw data
            {
               TString pname;
               // Only store non-zero parameters
               if (ds->GetValue() != 0) {
                  if (TString(ds->GetName()) != "DetTag" && TString(ds->GetName()) != "GTTag")
                     pname = Form("%s.%s", det->GetName(), ds->GetName());
                  else
                     pname = ds->GetName();
                  rnuc->GetParameters()->SetValue(pname, ds->GetValue());
               }
            }
         }
      }
   }
}

void KVFAZIAGroupReconstructor::IdentifyParticle(KVReconstructedNucleus& PART)
{
   KVGroupReconstructor::IdentifyParticle(PART);

   // Coherency codes (CCode):
   // -1 no modification du to coherency checks
   // 1 CsI id = gamma + good in Si-Si -> Si-Si
   // 2 CsI id -> Si-CsI id
   // 3 Si-CsI id -> Si-Si id because Z(sicsi)<Z(sisi)
   // 4 stopped in CsI (no id) + good id Si-Si -> Si-Si

   bool si1_pileup(false), si2_pileup(false);

   // check for failed PSA identification for particles stopped in Si1
   // change status => KVReconstructedNucleus::kStatusStopFirstStage
   // estimation of minimum Z from energy loss (if Si1 calibrated)
   // IDCODE = 5
   if (PART.GetStoppingDetector()->IsLabelled("SI1") && !PART.IsIdentified()) {
      PART.SetStatus(KVReconstructedNucleus::kStatusStopFirstStage);
      TreatStatusStopFirstStage(PART);
      PART.SetIDCode(5);
      PART.SetParameter("si1_pileup", si1_pileup);
      PART.SetParameter("si2_pileup", si2_pileup);
      return;
   }


   // Coherency checks for identifications
   if (PART.IsIdentified()) {
      if (partID.IsType("CsI")) {
         if (partID.IDcode == 0) { // gammas
            // look at Si1-Si2 identification.
            // if a correct identification was obtained, we ignore the gamma in CsI and change to Si1-Si2
#ifndef WITH_CPP11
            std::map<std::string, KVIdentificationResult*>::iterator si1si2 = id_by_type.find("Si-Si");
#else
            auto si1si2 = id_by_type.find("Si-Si");
#endif
            if (si1si2 != id_by_type.end()) {
               if (si1si2->second->IDOK && si1si2->second->IDquality < KVIDZAGrid::kICODE4) {
                  //               Info("IdentifyParticle","Got GAMMA in CsI, changing to Si1Si2 identification [Z=%d A=%d]",
                  //                    si1si2->second->Z, si1si2->second->A);
                  ChangeReconstructedTrajectory(PART);
                  partID = *(si1si2->second);
                  identifying_telescope = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType("Si-Si");
                  PART.SetIdentifyingTelescope(identifying_telescope);
                  PART.SetIdentification(&partID, identifying_telescope);
                  PART.GetParameters()->SetValue("CCode", 1);
                  //PART.Print();
               }
            }
         }
         else if (partID.IDOK) {
            // good identification in CsI (light charged particle)
            // check for pile-up in Si2 (and beyond) - in this case, measured energy loss in Si2+Si1
            // should not be attributed to this particle
#ifndef WITH_CPP11
            std::map<std::string, KVIdentificationResult*>::iterator si2csi = id_by_type.find("Si-CsI");
#else
            auto si2csi = id_by_type.find("Si-CsI");
#endif
            if (si2csi != id_by_type.end()) {
               // detect a pile-up in Si2 in coincidence with particule detected in CsI
               // the following covers the following cases:
               //   - good ID Si-CsI (IDquality < kICODE4) with Z>Zcsi
               //   - ID "between the lines" in Si-CsI (IDquality=kICODE4,kICODE5) with Z>Zcsi
               //   - point above last Z line in Si-CsI grid (kICODE7) with Z>Zcsi
               // In these cases, there is a second particle stopped in Si2: and
               // hence the measured Si1 energy loss is not to be used either
               if ((si2csi->second->Z > partID.Z)) {
                  si2_pileup = true;
                  si1_pileup = true;
               }
            }
#ifndef WITH_CPP11
            std::map<std::string, KVIdentificationResult*>::iterator si1si2 = id_by_type.find("Si-Si");
#else
            auto si1si2 = id_by_type.find("Si-Si");
#endif
            if (si1si2 != id_by_type.end()) {
               // detect a pile-up in Si2 in coincidence with particule detected in CsI
               // the following covers the following cases:
               //   - good ID Si1-Si2 (IDquality < kICODE4) with Z>Zcsi
               //   - ID "between the lines" in Si1-Si2 (IDquality=kICODE4,kICODE5) with Z>Zcsi
               //   - point above last Z line in Si1-Si2 grid (kICODE7) with Z>Zcsi
               // In these cases, there is a second particle stopped in Si2: and
               // hence the measured Si1 energy loss is not to be used either
               if ((si1si2->second->Z > partID.Z)) {
                  si2_pileup = true;
                  si1_pileup = true;
               }
            }
#ifndef WITH_CPP11
            std::map<std::string, KVIdentificationResult*>::iterator sipsa = id_by_type.find("SiPSA");
#else
            auto sipsa = id_by_type.find("SiPSA");
#endif
            if (sipsa != id_by_type.end()) {
               // detect a pile-up in Si1 in coincidence with particule detected in CsI
               if ((sipsa->second->Z > partID.Z)) {
                  si1_pileup = true;
               }
            }

         }
      }
      else if (partID.IsType("Si-CsI")) {
         // ions correctly identified in Si2-CsI should have coherent identification in Si1-Si2: as the particle punches
         // through Si2 the Si1-Si2 identification should underestimate the A and/or Z of the ion, i.e. either the A
         // or the Z from Si1-Si2 identification should be smaller than from Si2-CsI identification.
         // Unless of course some other particle stops in Si1 at the same time... (check PSA?)
         int zz = partID.Z;
#ifndef WITH_CPP11
         std::map<std::string, KVIdentificationResult*>::iterator si1si2 = id_by_type.find("Si-Si");
#else
         auto si1si2 = id_by_type.find("Si-Si");
#endif
         if (si1si2 != id_by_type.end()) {
            KVIdentificationResult* idr_si1si2 = si1si2->second;
            if (idr_si1si2->IDOK && idr_si1si2->IDquality < KVIDZAGrid::kICODE4) {
               if (zz < idr_si1si2->Z) {
                  //               Info("IdentifyParticle","SiCsI identification [Z=%d A=%d] changed to SiSi identification [Z=%d A=%d]",
                  //                    PART.GetZ(),PART.GetA(),si1si2->second->Z,si1si2->second->A);
                  ChangeReconstructedTrajectory(PART);
                  partID = *(si1si2->second);
                  identifying_telescope = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType("Si-Si");
                  PART.SetIdentifyingTelescope(identifying_telescope);
                  PART.SetIdentification(&partID, identifying_telescope);
                  PART.GetParameters()->SetValue("CCode", 3);

                  //PART.Print();
               }
            }
         }
      }
   }
   else {
      // particle not identified, stopped in CsI, with good Si1-Si2 identification?
      if (PART.GetStoppingDetector()->IsLabelled("CSI")) {
#ifndef WITH_CPP11
         std::map<std::string, KVIdentificationResult*>::iterator si1si2 = id_by_type.find("Si-Si");
#else
         auto si1si2 = id_by_type.find("Si-Si");
#endif
         if (si1si2 != id_by_type.end()) {

            if (si1si2->second->IDOK && si1si2->second->IDquality < KVIDZAGrid::kICODE4) {
               // stopping detector becomes Si2, identification Si1-Si2 accepted
               //               Info("IdentifyParticle","Unidentified, stopped in CsI, good Si1Si2 identification [Z=%d A=%d]",
               //                   si1si2->second->Z,si1si2->second->A);
               ChangeReconstructedTrajectory(PART);
               partID = *(si1si2->second);
               identifying_telescope = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType("Si-Si");
               PART.SetIdentifyingTelescope(identifying_telescope);
               PART.SetIdentification(&partID, identifying_telescope);
               PART.GetParameters()->SetValue("CCode", 4);
            }
         }
      }
   }
   PART.SetParameter("si1_pileup", si1_pileup);
   PART.SetParameter("si2_pileup", si2_pileup);
}

void KVFAZIAGroupReconstructor::ChangeReconstructedTrajectory(KVReconstructedNucleus& PART)
{
   // change recon trajectory (modifies stopping detector)

   PART.GetReconstructionTrajectory()->IterateFrom();
   PART.GetReconstructionTrajectory()->GetNextNode();
   KVGeoDetectorNode* node = PART.GetReconstructionTrajectory()->GetNextNode();

   KVGroup* group = PART.GetGroup();
   KVGeoDNTrajectory* traj = (KVGeoDNTrajectory*)node->GetTrajectories()->First();

   const KVReconNucTrajectory* newtraj = (const KVReconNucTrajectory*)group->GetTrajectoryForReconstruction(traj, node);
   PART.ModifyReconstructionTrajectory(newtraj);
}
