//Created by KVClassFactory on Sun Jan 20 13:31:10 2013
//Author: John Frankland,,,

#ifndef __KVSIMREADER_ELIE_H
#define __KVSIMREADER_ELIE_H

#include "KVSimReader.h"

/**
   \class KVSimReader_ELIE
\brief Read ascii files containing events generated by Elie
\ingroup Simulation

 This class is for reading primary events before secondary decay,
 i.e. output files with names like: elie_58_197_32_1000_primary.output

The events will be stored in a ROOT file with a name like:
~~~~
ELIE_[PROJ]_[TARG]_[EBEAM]AMeV_PRIM.root
~~~~
 This file will contain a TTree called "ELIE" with a branch "Simulated_evts"

All the informations in the header of the ELIE output file are stored in
 a KVNameValueList in the TTree:
~~~~{.cpp}
   ELIE->GetUserInfo()->FindObject("ELIE Parameters")->ls()

KVNameValueList::ELIE Parameters :  (0x55b2067172f0)
 <ievt=50>
 <projectile=Ni>
 <target=Au>
 <a_proj=58>
 <z_proj=28>
 <a_targ=197>
 <z_targ=79>
 <ebeam=32>
 <ilevel=0>
 <medium_effect=0>
 <g_al=1>
 <fraction_ex=0.5>
 <tempe_limite=5.5>
 <decay_in_flight=1>
 <lab_frame=0>
 <bminimum=0>
 <bmaximum=1>
 <choix_ex=0>
 <fermi=1>
 <version=1000>
 <periph_trick=0>
 <b_limite=0.75>
 <freeze_out=3>
 <proba_free_nucleon=0.001>
 <tole_pourcent=0.01>
 <discrete=1>
 <minimisation=0>
 <geometry=0>
 <free_nucleon=0>
 <lambda_ex=1.3>
~~~~
 If parameter `lab_frame=1`, we transform the events to the CM frame.
 In this way, for further analysis/filtering, ELIE events are always in the CM frame.
*/

class KVSimReader_ELIE : public KVSimReader {
   void init()
   {
      tree_name = "ELIE";
      elie_params = new KVNameValueList;
   }

protected:
   KVNucleus proj, targ;
   Double_t ebeam;
   KVNameValueList* elie_params;
   virtual void define_output_filename();
   void transform_to_cm();

public:
   KVSimReader_ELIE();
   KVSimReader_ELIE(KVString filename);
   virtual ~KVSimReader_ELIE();
   virtual void ConvertEventsInFile(KVString filename);

   void ReadFile();
   Bool_t ReadHeader();
   Bool_t ReadEvent();
   Bool_t ReadNucleus();

   ClassDef(KVSimReader_ELIE, 1) //Read ELIE events before secondary decay
};

#endif
