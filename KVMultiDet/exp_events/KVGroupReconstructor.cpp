//Created by KVClassFactory on Mon Oct 19 14:11:26 2015
//Author: John Frankland,,,

#include "KVGroupReconstructor.h"
#include "KVMultiDetArray.h"
#include "KVTarget.h"

#include <TPluginManager.h>
using std::cout;
using std::endl;

ClassImp(KVGroupReconstructor)

bool KVGroupReconstructor::fDoIdentification = kTRUE;
bool KVGroupReconstructor::fDoCalibration = kTRUE;

KVGroupReconstructor::KVGroupReconstructor()
   : KVBase("KVGroupReconstructor", "Reconstruction of particles in detector groups"),
     fGroup(nullptr), fGrpEvent(nullptr)
{
   // Default constructor

}

KVGroupReconstructor::~KVGroupReconstructor()
{
   // Destructor

   SafeDelete(fGrpEvent);
}

//________________________________________________________________

void KVGroupReconstructor::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVGroupReconstructor::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   //KVGroupReconstructor& CastedObj = (KVGroupReconstructor&)obj;
}

void KVGroupReconstructor::SetGroup(KVGroup* g)
{
   // set the group to be reconstructed
   fGroup = g;
   fPartSeedCond = dynamic_cast<KVMultiDetArray*>(fGroup->GetArray())->GetPartSeedCond();
}

void KVGroupReconstructor::SetReconEventClass(TClass* c)
{
   // Instantiate event fragment object
   // Set condition for seeding reconstructed particles

   if (!fGrpEvent) fGrpEvent = (KVReconstructedEvent*)c->New();
}

KVGroupReconstructor* KVGroupReconstructor::Factory(const TString& plugin)
{
   // Create a new object of a class derived from KVGroupReconstructor defined by a plugin.
   // If plugin="" this is just a new KVGroupReconstructor instance

   if (plugin == "") return new KVGroupReconstructor;
   TPluginHandler* ph = LoadPlugin("KVGroupReconstructor", plugin);
   if (ph) {
      return (KVGroupReconstructor*)ph->ExecPlugin(0);
   }
   return nullptr;
}

void KVGroupReconstructor::Process()
{
   // Perform full reconstruction for group: reconstruct, identify, calibrate
   //   - identification can be inhibited (for all groups) by calling KVGroupReconstructor::SetDoIdentification(false);
   //   - calibration can be inhibited (for all groups) by calling KVGroupReconstructor::SetDoCalibration(false);

   nfireddets = 0;
   coherency_particles.clear();
   Reconstruct();
   if (GetEventFragment()->GetMult() == 0) {
      return;
   }
   if (fDoIdentification) Identify();
   if (fDoCalibration) {
      Calibrate();
      if (!coherency_particles.empty()) AddCoherencyParticles();
   }
}

void KVGroupReconstructor::Reconstruct()
{
   // Reconstruct the particles in the group from hit trajectories
   // The condition for seeding particles is determined by the
   // multidetector to which the group belongs and the current dataset

   TIter nxt_traj(GetGroup()->GetTrajectories());
   KVGeoDNTrajectory* traj;

   // loop over trajectories
   while ((traj = (KVGeoDNTrajectory*)nxt_traj())) {

      // Work our way along the trajectory, starting from furthest detector from target,
      // start reconstruction of new detected particle from first fired detector.
      traj->IterateFrom();
      KVGeoDetectorNode* node;
      while ((node = traj->GetNextNode())) {

         KVReconstructedNucleus* kvdp;
         if ((kvdp = ReconstructTrajectory(traj, node))) {
            ReconstructParticle(kvdp, traj, node);
            break; //start next trajectory
         }

      }

   }

   if (GetEventFragment()->GetMult()) {
      AnalyseParticles();
      PostReconstructionProcessing();
   }
}

void KVGroupReconstructor::PostReconstructionProcessing()
{
   // This method will be called after reconstruction and first-order coherency analysis
   // of all particles in the group (if there are any reconstructed particles).
   // By default it does nothing.
}

KVReconstructedNucleus* KVGroupReconstructor::ReconstructTrajectory(const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node)
{
   KVDetector* d = node->GetDetector();
   nfireddets += d->Fired();
   // if d has fired and is either independent (only one trajectory passes through it)
   // or, if several trajectories pass through it,
   // only if the detector directly in front of it on this trajectory fired also
   if (!d->IsAnalysed() && d->Fired(fPartSeedCond)
         && (node->GetNTraj() == 1 ||
             (traj->GetNodeInFront(node) &&
              traj->GetNodeInFront(node)->GetDetector()->Fired()))) {
      return GetEventFragment()->AddParticle();
   }
   return nullptr;
}

void KVGroupReconstructor::ReconstructParticle(KVReconstructedNucleus* part, const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node)
{
   // Reconstruction of a detected nucleus from the successive energy losses
   // measured in a series of detectors/telescopes along a given trajectory

   const KVReconNucTrajectory* Rtraj = (const KVReconNucTrajectory*)GetGroup()->GetTrajectoryForReconstruction(traj, node);
   part->SetReconstructionTrajectory(Rtraj);
   part->SetParameter("ARRAY", GetGroup()->GetArray()->GetName());

   Rtraj->IterateFrom();// iterate over trajectory
   KVGeoDetectorNode* n;
   while ((n = Rtraj->GetNextNode())) {

      KVDetector* d = n->GetDetector();
      d->AddHit(part);  // add particle to list of particles hitting detector

   }

}

void KVGroupReconstructor::AnalyseParticles()
{
   if (GetNUnidentifiedInGroup() > 1) { //if there is more than one unidentified particle in the group

      Int_t n_nseg_1 = 0;
      //loop over particles counting up different cases
      for (KVEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
         KVReconstructedNucleus* nuc = it.get_pointer<KVReconstructedNucleus>();
         //ignore identified particles
         if (nuc->IsIdentified())
            continue;
         // The condition for a particle to be identifiable straight away is that the first
         // identification method that will be used must be independent
         //if (nuc->GetNSegDet() >= 1) {
         if (nuc->GetReconstructionTrajectory()->GetIDTelescopes()
               && ((KVIDTelescope*)nuc->GetReconstructionTrajectory()->GetIDTelescopes()->First())->IsIndependent()) {
            //all particles whose first identification telescope is independent are fine
            nuc->SetStatus(KVReconstructedNucleus::kStatusOK);
         }
         else if (nuc->GetReconstructionTrajectory()->GetNumberOfIdentifications()) {
            //no independent identification telescope => depends on what's in the rest of the group
            ++n_nseg_1;
         }
         else {
            //no identification available
            nuc->SetStatus(KVReconstructedNucleus::kStatusStopFirstStage);
         }
      }
      //loop again, setting status
      for (KVEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
         KVReconstructedNucleus* nuc = it.get_pointer<KVReconstructedNucleus>();
         if (nuc->IsIdentified())
            continue;           //ignore identified particles

         if (!(nuc->GetReconstructionTrajectory()->GetIDTelescopes()
               && ((KVIDTelescope*)nuc->GetReconstructionTrajectory()->GetIDTelescopes()->First())->IsIndependent())
               && nuc->GetReconstructionTrajectory()->GetNumberOfIdentifications()) {
            //particles with no independent identification possibility
            if (n_nseg_1 == 1) {
               //just the one ? then we can get it no problem
               //after identifying the others and subtracting their calculated
               //energy losses from the other detectors
               nuc->SetStatus(KVReconstructedNucleus::kStatusOKafterSub);
            }
            else {
               //more than one ? then we can make some wild guess by sharing the
               //contribution between them, but I wouldn't trust it as far as I can spit
               nuc->SetStatus(KVReconstructedNucleus::kStatusOKafterShare);
            }
            //one possibility remains: the particle may actually have stopped e.g.
            //in the DE detector of a DE-E telescope
            if (!nuc->GetReconstructionTrajectory()->GetNumberOfIdentifications()) {
               //no ID telescopes with which to identify particle
               nuc->SetStatus(KVReconstructedNucleus::kStatusStopFirstStage);
            }
         }
      }
   }
   else if (GetNUnidentifiedInGroup() == 1) {
      //only one unidentified particle in group: just need an idtelescope which works

      //loop over particles looking for the unidentified one
      KVReconstructedNucleus* nuc(nullptr);
      for (KVEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
         nuc = it.get_pointer<KVReconstructedNucleus>();
         if (!nuc->IsIdentified()) break;
      }
      //cout << "nuc->GetNSegDet()=" << nuc->GetNSegDet() << endl;
      if (nuc->GetReconstructionTrajectory()->GetNumberOfIdentifications()) {
         //OK no problem
         nuc->SetStatus(KVReconstructedNucleus::kStatusOK);
      }
      else {
         //dead in the water
         nuc->SetStatus(KVReconstructedNucleus::kStatusStopFirstStage);
      }
   }
}

void KVGroupReconstructor::IdentifyParticle(KVReconstructedNucleus& PART)
{
   // Try to identify this nucleus by calling the Identify() function of each
   // ID telescope crossed by it, starting with the telescope where the particle stopped, in order
   //   -  only telescopes which have been correctly initialised for the current run are used,
   //      i.e. those for which KVIDTelescope::IsReadyForID() returns kTRUE.
   //
   // This continues until a successful identification is achieved or there are no more ID telescopes to try.
   //
   // The identification code corresponding to the identifying telescope is set as the identification code of the particle.

   const KVSeqCollection* idt_list = PART.GetReconstructionTrajectory()->GetIDTelescopes();
   identifying_telescope = nullptr;
   id_by_type.clear();

   if (idt_list->GetEntries() > 0) {

      KVIDTelescope* idt;
      TIter next(idt_list);
      Int_t idnumber = 1;
      Int_t n_success_id = 0;//number of successful identifications
      while ((idt = (KVIDTelescope*) next())) {
         KVIdentificationResult* IDR = PART.GetIdentificationResult(idnumber++);
         IDR->SetIDType(idt->GetType());// without this, the identification type is never set!
         if (idt->IsReadyForID()) { // is telescope able to identify for this run ?
            id_by_type[IDR->GetIDType()] = IDR;// map contains only attempted identifications
            IDR->IDattempted = kTRUE;
            idt->Identify(IDR);

            if (IDR->IDOK) n_success_id++;
         }
         else
            IDR->IDattempted = kFALSE;

         if (n_success_id < 1 &&
               ((!IDR->IDattempted) || (IDR->IDattempted && !IDR->IDOK))) {
            // the particle is less identifiable than initially thought
            // we may have to wait for secondary identification
            Int_t nseg = PART.GetNSegDet();
            PART.SetNSegDet(TMath::Max(nseg - 1, 0));
            //if there are other unidentified particles in the group and NSegDet is < 1
            //then exact status depends on segmentation of the other particles : reanalyse
            if (PART.GetNSegDet() < 1 && GetNUnidentifiedInGroup() > 1) {
               AnalyseParticles();
               return;
            }
            //if NSegDet = 0 it's hopeless
            if (!PART.GetNSegDet()) {
               AnalyseParticles();
               return;
            }
         }

      }
      // set first successful identification as particle identification
      // as long as the id telescope concerned contains the stopping detector!
      Int_t id_no = 1;
      Bool_t ok = kFALSE;
      KVIdentificationResult* pid = PART.GetIdentificationResult(id_no);
      next.Reset();
      idt = (KVIDTelescope*)next();
      while (idt->HasDetector(PART.GetStoppingDetector())) {
         if (pid->IDattempted && pid->IDOK) {
            ok = kTRUE;
            partID = *pid;
            identifying_telescope = idt;
            break;
         }
         ++id_no;
         pid = PART.GetIdentificationResult(id_no);
         idt = (KVIDTelescope*)next();
      }
      if (ok) {
         PART.SetIsIdentified();
         PART.SetIdentifyingTelescope(identifying_telescope);
         PART.SetIdentification(&partID, identifying_telescope);
      }

   }

}

//_________________________________________________________________________________

void KVGroupReconstructor::TreatStatusStopFirstStage(KVReconstructedNucleus& d)
{
   // particles stopped in first member of a telescope
   // estimation of Z (minimum) from energy loss (if detector is calibrated)
   Int_t zmin = d.GetStoppingDetector()->FindZmin(-1., d.GetMassFormula());
   if (zmin) {
      d.SetZ(zmin);
      d.SetIsIdentified();
      KVGeoDNTrajectory* t = (KVGeoDNTrajectory*)d.GetStoppingDetector()->GetNode()->GetTrajectories()->First();
      d.SetIdentifyingTelescope((KVIDTelescope*) t->GetIDTelescopes()->Last());
   }
}

void KVGroupReconstructor::Identify()
{
   // All particles which have not been previously identified (IsIdentified=kFALSE), and which
   // may be identified independently of all other particles in their group according to the 1st
   // order coherency analysis (KVReconstructedNucleus::GetStatus=0), will be identified.
   // Particles stopping in first member of a telescope (KVReconstructedNucleus::GetStatus=3) will
   // have their Z estimated from the energy loss in the detector (if calibrated).

   for (KVEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
      KVReconstructedNucleus& d = it.get_reference<KVReconstructedNucleus>();
      if (!d.IsIdentified()) {
         if (d.GetStatus() == KVReconstructedNucleus::kStatusOK) {
            // identifiable particles
            IdentifyParticle(d);
         }
         else if (d.GetStatus() == KVReconstructedNucleus::kStatusStopFirstStage) {
            // particles stopped in first member of a telescope
            // estimation of Z (minimum) from energy loss (if detector is calibrated)
            TreatStatusStopFirstStage(d);
         }
      }
   }
}

//_____________________________________________________________________________

void KVGroupReconstructor::Calibrate()
{
   // Calculate and set energies of all identified but uncalibrated particles in event.

   KVReconstructedNucleus* d;

   while ((d = GetEventFragment()->GetNextParticle())) {

      if (d->IsIdentified() && !d->IsCalibrated()) {
         CalibrateParticle(d);
      }

   }

}

void KVGroupReconstructor::AddCoherencyParticles()
{
   // Called to add any nuclei not included in the initial reconstruction, but "revealed"
   // by consistency checks between identifications and calibrations of other nuclei

   std::cout << "===========================================================================================\n" << std::endl;
   Info("AddCoherencyParticles", "There are %d particles to add to this event\n", (int)coherency_particles.size());
   for (auto& part : coherency_particles) {
      auto rnuc = GetEventFragment()->AddParticle();
      Info("AddCoherencyParticle", "Adding particle in pile-up with Z=%d A=%d E=%f in %s",
           part.original_particle->GetZ(), part.original_particle->GetA(), part.original_particle->GetE(), part.original_particle->GetStoppingDetector()->GetName());
      auto ESI1_parent = part.original_particle->GetParameters()->HasDoubleParameter("FAZIA.ESI1") ?
                         TMath::Abs(part.original_particle->GetParameters()->GetDoubleValue("FAZIA.ESI1"))
                         : 0;
      auto ESI2_parent = part.original_particle->GetParameters()->HasDoubleParameter("FAZIA.ESI2") ?
                         TMath::Abs(part.original_particle->GetParameters()->GetDoubleValue("FAZIA.ESI2"))
                         : 0;
      std::cout << "ESI1 = " << ESI1_parent << " ESI2 = " << ESI2_parent << std::endl;

      // reconstruction
      auto Rtraj = (const KVReconNucTrajectory*)GetGroup()->GetTrajectoryForReconstruction(part.stopping_trajectory, part.stopping_detector_node);

      std::cout << "SI1: " << (Rtraj->GetDetector("SI1")->IsCalibrated() ? "CALIB." : "NOT CALIB.")
                << "   SI2: " << (Rtraj->GetDetector("SI2")->IsCalibrated() ? "CALIB." : "NOT CALIB.")
                << "   CSI: " << (part.original_particle->GetStoppingDetector()->IsCalibrated(
                                     Form("Z=%d,A=%d", part.original_particle->GetZ(), part.original_particle->GetA())
                                  ) ? "CALIB." : "NOT CALIB.")  << std::endl;

      rnuc->SetReconstructionTrajectory(Rtraj);
      rnuc->SetParameter("ARRAY", GetGroup()->GetArray()->GetName());
      rnuc->SetParameter("CoherencyParticle",
                         "Particle added to event after consistency checks between identifications and calibrations of other nuclei");
      // identification
      Int_t idnumber = 1;
      for (int i = part.first_id_result_to_copy; i <= part.max_id_result_index; ++i) {
         auto IDR = rnuc->GetIdentificationResult(idnumber++);
         // copy existing identification results from "parent" particle
         part.original_particle->GetIdentificationResult(i)->Copy(*IDR);
      }
      rnuc->SetIsIdentified();
      rnuc->SetIdentifyingTelescope(part.identifying_telescope);
      rnuc->SetIdentification(rnuc->GetIdentificationResult(1), part.identifying_telescope);
      Info("AddCoherencyParticle", "Initial ident of particle to add: Z=%d A=%d identified in %s",
           rnuc->GetZ(), rnuc->GetA(), rnuc->GetIdentifyingTelescope()->GetType());
      rnuc->GetIdentificationResult(1)->Print();
      // with both silicons calibrated, we can try to subtract the contributions of the parent
      // particle (using inverse calibrations and calculated energy losses of parent),
      // then try a new identification.
      // as E789 Si1-Si2 identifications are implemented with 2 grids, first a low range
      // QL1.Amplitude vs. Q2.FPGAEnergy grid (upto Z=8), then full range QH1.FPGAEnergy vs.
      // Q2.FPGAEnergy, and calibrations for SI1 & SI2 use QH1.FPGAEnergy and Q2.FPGAEnergy
      // respectively, we set QL1.Amplitude=0 in order to force the use of the recalculated
      // QH1.FPGAEnergy and Q2.FPGAEnergy in the full range grid.
      // *actually, SI1 may have also an "Energy-QL1" calibration from QL1.Amplitude,
      // in which case we can modify this as well
      auto SI1 = rnuc->GetReconstructionTrajectory()->GetDetector("SI1");
      auto SI2 = rnuc->GetReconstructionTrajectory()->GetDetector("SI2");
      if (SI1->IsCalibrated() && SI2->IsCalibrated()) {
         // calculate new values of raw parameters
         double new_q2{0}, new_qh1{0}, new_ql1{0};
         auto ESI1_qh1 = SI1->GetDetectorSignalValue("Energy");
         auto ESI1_ql1 = SI1->GetDetectorSignalValue("Energy-QL1");
         auto ESI2 = SI2->GetEnergy();
         new_qh1 = SI1->GetInverseDetectorSignalValue("Energy", TMath::Max(0., ESI1_qh1 - ESI1_parent), "QH1.FPGAEnergy");
         new_q2 = SI2->GetInverseDetectorSignalValue("Energy", TMath::Max(0., ESI2 - ESI2_parent), "Q2.FPGAEnergy");
         if (SI1->HasDetectorSignalValue("Energy-QL1")) {
            new_ql1 = SI1->GetInverseDetectorSignalValue("Energy-QL1", TMath::Max(0., ESI1_ql1 - ESI1_parent), "QL1.Amplitude");
         }
         Info("AddCoherencyParticle", "Changing raw data parameters:");
         std::cout << "  QH1:  " << SI1->GetDetectorSignalValue("QH1.FPGAEnergy") << "  ==>  " << new_qh1 << std::endl;
         std::cout << "  QL1:  " << SI1->GetDetectorSignalValue("QL1.Amplitude") << "  ==>  " << new_ql1 << std::endl;
         std::cout << "   Q2:  " << SI2->GetDetectorSignalValue("Q2.FPGAEnergy") << "  ==>  " << new_q2 << std::endl;
         SI1->SetDetectorSignalValue("QH1.FPGAEnergy", new_qh1);
         SI1->SetDetectorSignalValue("QL1.Amplitude", new_ql1);
         SI2->SetDetectorSignalValue("Q2.FPGAEnergy", new_q2);
         // now retry the identification
         rnuc->GetIdentificationResult(1)->IDOK = kFALSE;
         part.identifying_telescope->Identify(rnuc->GetIdentificationResult(1));
         rnuc->GetIdentificationResult(1)->Print();
         if (rnuc->GetIdentificationResult(1)->IDOK) {
            Info("AddCoherencyParticle", "Achieved new identification for particle:");
            rnuc->SetIdentification(rnuc->GetIdentificationResult(1), part.identifying_telescope);
         }
         else {
            Info("AddCoherencyParticle", "Identification has failed for particle");
            if (new_q2 < 1) {
               Info("AddCoherencyParticle", "Try SI1-PSA identification?");
               part.original_particle->GetIdentificationResult(5)->Print();
               // subtraction of original particle leaves nothing in SI2: try SI1-PSA ?
               if (part.original_particle->GetIdentificationResult(5)->IDOK) {
                  // modify reconstruction trajectory: now starts on SI1 not SI2
                  Rtraj = (const KVReconNucTrajectory*)GetGroup()->GetTrajectoryForReconstruction(part.stopping_trajectory,
                          SI1->GetNode());
                  rnuc->ModifyReconstructionTrajectory(Rtraj);
                  part.original_particle->GetIdentificationResult(5)->Copy(*rnuc->GetIdentificationResult(1));
                  auto idt = (KVIDTelescope*)Rtraj->GetIDTelescopes()->First();
                  rnuc->SetIdentification(rnuc->GetIdentificationResult(1), idt);
               }
               else {
                  // Q2 = 0 & Si1-PSA has failed: particle remains unidentified
                  //rnuc->SetU
               }
            }
         }
         // calibration
         if (rnuc->IsIdentified()) {
            CalibrateCoherencyParticle(rnuc);
            Info("AddCoherencyParticle", "Added particle with Z=%d A=%d E=%f identified in %s",
                 rnuc->GetZ(), rnuc->GetA(), rnuc->GetE(), rnuc->GetIdentifyingTelescope()->GetType());
            std::cout << "ESI1 = " << rnuc->GetParameters()->GetDoubleValue("FAZIA.ESI1")
                      << " [" << rnuc->GetParameters()->GetDoubleValue("FAZIA.avatar.ESI1") << "]"
                      << " ESI2 = " << rnuc->GetParameters()->GetDoubleValue("FAZIA.ESI2")
                      << " [" << rnuc->GetParameters()->GetDoubleValue("FAZIA.avatar.ESI2") << "]"
                      << std::endl << std::endl;
         }
      }
   }
   std::cout << "===========================================================================================\n\n" << std::endl;
}

Double_t KVGroupReconstructor::GetTargetEnergyLossCorrection(KVReconstructedNucleus* ion)
{
   // Calculate the energy loss in the current target of the multidetector
   // for the reconstructed charged particle 'ion', assuming that the current
   // energy and momentum of this particle correspond to its state on
   // leaving the target.
   //
   // WARNING: for this correction to work, the target must be in the right 'state':
   //
   //      gMultiDetArray->GetTarget()->SetIncoming(kFALSE);
   //      gMultiDetArray->GetTarget()->SetOutgoing(kTRUE);
   //
   // (see KVTarget::GetParticleEIncFromERes).
   //
   // The returned value is the energy lost in the target in MeV.
   // The energy/momentum of 'ion' are not affected.

   KVMultiDetArray* array = (KVMultiDetArray*)GetGroup()->GetArray();
   if (!array->GetTarget() || !ion) return 0.0;
   return (array->GetTarget()->GetParticleEIncFromERes(ion) - ion->GetEnergy());
}

