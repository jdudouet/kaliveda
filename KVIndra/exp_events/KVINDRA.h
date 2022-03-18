/***************************************************************************
                          kvindra.h  -  description
                             -------------------
    begin                : Mon May 20 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVINDRA.h,v 1.43 2009/01/21 10:05:51 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVINDRA_H
#define KVINDRA_H

#include "TEnv.h"
#include "KVASMultiDetArray.h"
#include "KVList.h"
#include "KVHashList.h"
#include "KVACQParam.h"
#include "KVDBSystem.h"
#include "KVUpDater.h"
#include "KVDataSetManager.h"
#include "KVINDRATriggerInfo.h"
#include "KVINDRADetector.h"
#include "KVINDRATelescope.h"

class KVLayer;
class KVNucleus;
class KVChIo;
class KVDetectorEvent;
class KVINDRAReconEvent;

//old BaseIndra type definitions
enum EBaseIndra_type {
   ChIo_GG = 1,
   ChIo_PG,                     //=2
   ChIo_T,                      //=3
   Si_GG,                       //=4
   Si_PG,                       //=5
   Si_T,                        //=6
   CsI_R,                       //=7
   CsI_L,                       //=8
   CsI_T,                       //=9
   Si75_GG,                     //=10
   Si75_PG,                     //=11
   Si75_T,                      //=12
   SiLi_GG,                     //=13
   SiLi_PG,                     //=14
   SiLi_T                       //=15
};
enum EBaseIndra_typePhos {
   Phos_R = 1,
   Phos_L,                      //=2
   Phos_T,                      //=3
};

/**
\class KVINDRA
\brief INDRA multidetector array geometry
\ingroup INDRAGeometry
*/

class KVINDRA: public KVASMultiDetArray {

public:
   static Char_t SignalTypes[16][3];    //Use this static array to translate EBaseIndra_type signal type to a string giving the signal type


private:
   UChar_t fTrigger;           //multiplicity trigger used for acquisition
   KVList fOwnedACQParams;     //! to clean up acquisition parameters belonging to the array, not to detectors

   void AddArrayACQParam(KVACQParam* p)
   {
      fOwnedACQParams.Add(p);
      AddACQParam(p);
   }

protected:
   KVHashList* fChIo;              //->List Of ChIo of INDRA
   KVHashList* fSi;                 //->List of Si detectors of INDRA
   KVHashList* fCsI;                //->List of CsI detectors of INDRA
   KVHashList* fPhoswich;           //->List of NE102/NE115 detectors of INDRA

   Bool_t fPHDSet;//set to kTRUE if pulse height defect parameters are set

   KVINDRATriggerInfo* fSelecteur;//infos from DAQ trigger (le Selecteur)

   TEnv fStrucInfos;//! file containing structure of array

   virtual void MakeListOfDetectors();
   virtual void BuildGeometry();
   virtual void SetGroupsAndIDTelescopes();
   void FillListsOfDetectorsByType();
   void SetGGtoPGConversionFactors();
   void LinkToCodeurs();
   void BuildLayer(const Char_t* name);
   KVRing* BuildRing(Int_t number, const Char_t* prefix);
   KVINDRATelescope* BuildTelescope(const Char_t* prefix, Int_t mod);
   void FillTrajectoryIDTelescopeLists();
   Int_t GetIDTelescopes(KVDetector*, KVDetector*, TCollection*);
   void SetNamesOfIDTelescopes() const;

   void PerformClosedROOTGeometryOperations();
#ifdef WITH_MFM
   Bool_t handle_raw_data_event_mfmframe_ebyedat(const MFMEbyedatFrame&);
#endif
   void SetIDCodeForIDTelescope(KVIDTelescope*) const;

   // The following methods are used by the current implementation of the filter.
   // They should be removed in future implementations.
   virtual UShort_t GetBadIDCode()
   {
      // return a general identification code for particles badly identified
      // with this type of ID telescope
      return IDCodes::NO_IDENTIFICATION;
   }
   virtual UShort_t GetCoherencyIDCode()
   {
      // return a general identification code for particles identified
      // with this type of ID telescope after coherency analysis
      return IDCodes::ID_CI_SI_COHERENCY;
   }
   virtual UShort_t GetMultiHitFirstStageIDCode()
   {
      // return a general identification code for particles which cannot
      // be identified correctly due to pile-up in a delta-E detector
      return IDCodes::ID_CI_MULTIHIT;
   }
   virtual UChar_t GetNormalCalibrationCode()
   {
      // return a general calibration code for correctly calibrated particles
      return ECodes::NORMAL_CALIBRATION;
   }

public:
   enum IDCodes {
      /** \enum KVINDRA::IDCodes
          \brief Identification quality codes attributed to particles reconstructed from data
       */
      NO_IDENTIFICATION = 14, ///< no identification either attempted or available for particle
      ID_STOPPED_IN_FIRST_STAGE = 5, ///< particle stopped in first detector of telescope, only minimum Z can be estimated
      ID_GAMMA = 0, ///< 'gamma' particle detected in CsI
      ID_NEUTRON = 1, ///< 'neutron' discriminated by coherency between CsI and Si-CsI identifications
      ID_PHOSWICH = 2, ///< particle identified in phoswich (campaigns 1-3)
      ID_CSI_PSA = 2, ///< particle identified in CsI detector by pulse shape analysis
      ID_SI_CSI = 3, ///< particle identified in Si-CsI telescope
      ID_SI75_SILI = 3, ///< particle identified in Si75-SiLi etalon telescope
      ID_SILI_CSI = 3, ///< particle identified in SiLi-CsI etalon telescope
      ID_CI_SI = 4, ///< particle identified in ChIo-Si telescope
      ID_CI_CSI = 4, ///< particle identified in ChIo-CsI telescope
      ID_CI_SI75 = 4, ///< particle identified in ChIo-Si75 etalon telescope
      ID_CI_SI_COHERENCY = 6, ///< particle identified in ChIo-Si telescope in coincidence with light particle identified in CsI
      ID_CI_COHERENCY = 7, ///< particle stopped in ChIo revealed by coherency tests (Zmin)
      ID_CI_MULTIHIT = 8, ///< particles stopped in multiple Si (ring<10) or CsI (ring>9) behind same ChIo, bad identification
      ID_CSI_FRAGMENT = 9, ///< particle partially identified in CsI detector, with Z greater than identifiable
      ID_CSI_MASS_OUT_OF_RANGE = 10 ///< particle partially identified in CsI detector, mass out of range of apparent Z (pile-up?)
   };
   enum ECodes {
      /** \enum KVINDRA::ECodes
          \brief Calibration quality codes attributed to particles reconstructed from data
       */
      NO_CALIBRATION_ATTEMPTED = 0, ///< particle stopped in detectors with no available calibration
      NORMAL_CALIBRATION = 1,       ///< normal well-calibrated particle with no problems
      SOME_ENERGY_LOSSES_CALCULATED = 2, ///< particle calibration OK, with some detector energies calculated
      WARNING_CSI_MAX_ENERGY = 3,    ///< particle calibration OK, although apparent energy would mean punching through the CsI
      BAD_CALIBRATION = 15          ///< calibration attempted but bad result (negative energies etc.)
   };
   virtual Int_t GetIDCodeForParticlesStoppingInFirstStageOfTelescopes() const
   {
      return IDCodes::ID_STOPPED_IN_FIRST_STAGE;
   }

   KVINDRA();
   virtual ~ KVINDRA();

   virtual void Build(Int_t run = -1);
   virtual Bool_t ArePHDSet() const
   {
      return fPHDSet;
   }
   virtual void PHDSet(Bool_t yes = kTRUE)
   {
      fPHDSet = yes;
   }

   KVLayer* GetChIoLayer();
   inline KVHashList* GetListOfChIo() const
   {
      return fChIo;
   };
   inline KVHashList* GetListOfSi() const
   {
      return fSi;
   };
   inline KVHashList* GetListOfCsI() const
   {
      return fCsI;
   };
   inline KVHashList* GetListOfPhoswich() const
   {
      return fPhoswich;
   };

   virtual KVChIo* GetChIoOf(const Char_t* detname);
   virtual void cd(Option_t* option = "");
   virtual KVINDRADetector* GetDetectorByType(UInt_t cou, UInt_t mod,
         UInt_t type) const;

   void SetTrigger(UChar_t trig);
   UChar_t GetTrigger() const
   {
      return fTrigger;
   }

   void SetPinLasersForCsI();
   virtual TGraph* GetPedestals(const Char_t* det_signal, const Char_t* det_type, Int_t ring_number, Int_t run_number = -1);

   void SetArrayACQParams();
   virtual void GetDetectorEvent(KVDetectorEvent* detev, const TSeqCollection* fired_params = 0);

   KVINDRATriggerInfo* GetTriggerInfo()
   {
      return fSelecteur;
   };

   void CreateROOTGeometry();

   virtual void SetROOTGeometry(Bool_t on = kTRUE);
   void SetMinimumOKMultiplicity(KVEvent*) const;
   KVGroupReconstructor* GetReconstructorForGroup(const KVGroup*) const;
   void SetReconParametersInEvent(KVReconstructedEvent*) const;

   ClassDef(KVINDRA, 6)        //class describing the materials and detectors etc. to build an INDRA multidetector array
};

//................  global variable
R__EXTERN KVINDRA* gIndra;

//................ inline functions
inline void KVINDRA::cd(Option_t*)
{
   gIndra = this;
}

#endif
