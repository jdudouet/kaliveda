//Created by KVClassFactory on Thu Jan 22 16:02:02 2015
//Author: ,,,

#include "KVISOFAZIA.h"

#include "KVUnits.h"
#include "KVFAZIABlock.h"

ClassImp(KVISOFAZIA)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVISOFAZIA</h2>
<h4>FAZIA set-up for ISOFAZIA experiment June 2015 - 4 blocks</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVISOFAZIA::KVISOFAZIA()
   : KVFAZIA()
{
   // Default constructor
   SetTitle(ClassName());
}

void KVISOFAZIA::GetGeometryParameters()
{

   fNblocks = 4;
   fFDist = 80.0;

}


KVISOFAZIA::~KVISOFAZIA()
{
   // Destructor
}

void KVISOFAZIA::BuildFAZIA()
{
   Info("BuildFAZIA", "Compact geometry, %f cm from target",
        fFDist);

   TGeoVolume* top = gGeoManager->GetTopVolume();

   Double_t distance_block_cible = fFDist * KVUnits::cm;
   Double_t thick_si1 = 300 * KVUnits::um;

   KVFAZIABlock* block = new KVFAZIABlock;

   Double_t theta = 0;
   Double_t phi = 0;

   Double_t arc = block->GetTotalSideWithBlindage() / (distance_block_cible + thick_si1 / 2.);
   arc /= 2;
   arc *= TMath::RadToDeg();

   for (Int_t bb = 0; bb < fNblocks; bb += 1) {
      if (bb == 3) {
         phi = 90;
         theta = 11.8 + arc;
      }
      else if (bb == 2) {
         phi = 90;
         theta = 2.3 + arc;
      }
      else if (bb == 1) {
         phi = -90;
         theta = 2.3 + arc;
      }
      else if (bb == 0) {
         phi = -90;
         theta = 11.8 + arc;
      }
      top->AddNode(block, bb,
                   KVMultiDetArray::GetVolumePositioningMatrix(block->GetNominalDistanceTargetBlockCentre(distance_block_cible),
                         theta, phi));
   }

}
