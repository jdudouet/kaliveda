#ifndef KVTELESCOPE_H
#define KVTELESCOPE_H
#include "KVGroup.h"
#include "KVPosition.h"
#include "KVDetector.h"
#include "KVList.h"
#include "KVNameValueList.h"
#include "KVGeoStrucElement.h"

/**
 \class KVTelescope
 \ingroup Geometry
 \brief Associates two detectors placed one behind the other

 An assembly of one or more layers of detectors, usually any particle
 traversing the layer closest to the target will then traverse all
 subsequent layers of the telescope (if it has sufficient energy).

 Telescope structures own their detectors, and will delete them when
 the telescope is deleted.

 As detectors are added, we automatically set the KVDetectorNode
 information as they are supposed to be placed one behind the other.

 In imported ROOT geometries (see KVGeoImport), you can trigger the creation
 of a KVTelescope structure by using a node name of the type
~~~~
     STRUCT_TELESCOPE_[number]
~~~~
*/

class KVNucleus;
class TGraph;

class KVTelescope: public KVGeoStrucElement, public KVPosition {

protected:
   Int_t fNdets;                //number of detectors in telescope
   Float_t* fDepth;             //[fNdets] depth of each element starting from nearest target
   void init();

public:

   KVTelescope();
   virtual ~ KVTelescope();

   void Add(KVBase* element);

   KVDetector* GetDetector(Int_t n) const
   {
      //returns the nth detector in the telescope structure
      if (n > GetSize() || n < 1) {
         Error("GetDetector(UInt_t n)", "%s n must be between 1 and %u",
               GetName(), GetSize());
         return 0;
      }
      KVDetector* d = (KVDetector*) GetDetectors()->At((n - 1));
      return d;
   };
   Int_t GetDetectorRank(KVDetector* kvd);
   Int_t GetSize() const
   {
      //returns number of detectors in telescope
      return GetDetectors()->GetSize();
   };
   virtual void DetectParticle(KVNucleus* kvp, KVNameValueList* nvl = 0);
   void ResetDetectors();


   void SetDepth(UInt_t ndet, Float_t depth);
   Float_t GetDepth(Int_t ndet) const;
   Double_t GetTotalLengthInCM() const;
   Double_t GetDepthInCM(Int_t ndet) const
   {
      // return depth inside telescope of detector number ndet in centimetres
      return ((Double_t)GetDepth(ndet)) / 10.;
   }
   Float_t GetDepth() const;

   virtual TGeoVolume* GetGeoVolume();
   virtual void AddToGeometry();

   // override KVPosition methods to apply to all detectors
   virtual void SetAzimuthalAngle(Double_t ph)
   {
      KVPosition::SetAzimuthalAngle(ph);
      const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, SetAzimuthalAngle)(ph);
   }
   virtual void SetPolarAngle(Double_t th)
   {
      KVPosition::SetPolarAngle(th);
      const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, SetPolarAngle)(th);
   }
   virtual void SetPolarWidth(Double_t pw)
   {
      KVPosition::SetPolarWidth(pw);
      const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, SetPolarWidth)(pw);
   }
   virtual void SetPolarMinMax(Double_t min, Double_t max)
   {
      KVPosition::SetPolarMinMax(min, max);
      const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, SetPolarMinMax)(min, max);
   }
   virtual void SetAzimuthalWidth(Double_t aw)
   {
      KVPosition::SetAzimuthalWidth(aw);
      const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, SetAzimuthalWidth)(aw);
   }
   virtual void SetAzimuthalMinMax(Double_t min, Double_t max)
   {
      KVPosition::SetAzimuthalMinMax(min, max);
      const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, SetAzimuthalMinMax)(min, max);
   }

   ClassDef(KVTelescope, 3)     //Multi-layered telescopes composed of different absorbers
};

#endif
