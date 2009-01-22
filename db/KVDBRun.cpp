/***************************************************************************
$Id: KVDBRun.cpp,v 1.12 2008/12/08 14:07:37 franklan Exp $
                          KVDBRun.cpp  -  description
                             -------------------
    begin                : jeu f�v 13 2003
    copyright            : (C) 2003 by Alexis Mignon
    email                : mignon@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "KVDBRun.h"
#include "Riostream.h"
#include "TEnv.h"
#include "TObjString.h"
#include "TObjArray.h"

ClassImp(KVDBRun);
///////////////////////////////////////////////////
//KVDBRun
//
//Base class database entry for an experimental run
//
//____________________________________________________________________________

KVDBRun::KVDBRun():fDatime()
{
   //default ctor
}

//____________________________________________________________________________
KVDBRun::KVDBRun(Int_t number, const Char_t * title):fDatime()
{
   //ctor for a given run number

   SetNumber(number);
   SetTitle(title);
}

//____________________________________________________________________________

void KVDBRun::SetNumber(Int_t number)
{
   //Set run number. Also changes name of object to "Run xxx".
   KVDBRecord::SetNumber(number);
   SetName(Form("Run %d", number));
}
   
//____________________________________________________________________________

KVDBRun::~KVDBRun()
{
   //dtor
}

//___________________________________________________________________________

void KVDBRun::Print(Option_t * option) const
{
   cout << "___________________________________________________" << endl
       << GetName() << "  (" << GetTitle() << ")" << endl;
   if (GetSystem()) {
      cout << "System : " << GetSystem()->GetName() << endl;
      if (GetSystem()->GetTarget())
         cout << "Target : " << GetSystem()->GetTarget()->
             GetName() << endl;
   }
   cout << "___________________________________________________" << endl;
   //print values of all parameters
   fStringPar.Print();
   cout << "___________________________________________________" << endl;
   fFloatPar.Print();
   cout << "___________________________________________________" << endl;
   fIntPar.Print();
   cout << "___________________________________________________" << endl;
}

//___________________________________________________________________________

void KVDBRun::WriteRunListLine(ostream & outstr, Char_t delim) const
{
   //Write informations on run in format used for runlists, i.e. a line of fields separated by the
   //separator character '|' (the 'delim' argument is obsolete and is not used) 

   TString _delim = " | ";
   KVString s;
   //write run number
   outstr << GetNumber() << _delim.Data();
   //write all scalers
   for(int i=0; i<fIntPar.GetNPar(); i++){
      s.Form("%s=%d", fIntPar.GetParameter(i)->GetName(), fIntPar.GetParameter(i)->GetVal());
      outstr << s.Data() << _delim.Data();
   }
   //write all floating point values
   for(int i=0; i<fFloatPar.GetNPar(); i++){
      s.Form("%s=%f", fFloatPar.GetParameter(i)->GetName(), fFloatPar.GetParameter(i)->GetVal());
      outstr << s.Data() << _delim.Data();
   }
   //write all string values
   for(int i=0; i<fStringPar.GetNPar(); i++){
      // as we write all strings as 'parname=value', there is a problem if the string 'value'
      // itself contains the '=' symbol !!
      // therefore we replace any '=' in the string by '\equal' before writing in the file.
      // when reading back (see ReadRunListLine), we replace '\equal' by '='
      TString tmp( fStringPar.GetParameter(i)->GetVal().Data() );
      tmp.ReplaceAll("=","\\equal");
      s.Form("%s=%s", fStringPar.GetParameter(i)->GetName(), tmp.Data());
      outstr << s.Data() << _delim.Data();
   }
   outstr << endl;
}

//___________________________________________________________________________

void KVDBRun::ReadRunListLine(const KVString& line)
{
   //Set run characteristics by reading informations in 'line' (assumed to have been written
   //by WriteRunListLine).
   
   //Break line into fields using delimiter '|'
   TObjArray* fields = line.Tokenize('|');
   if(fields->GetEntries()<1){
      //not a valid line for run
      delete fields;
      return;
   }
   
   //first field is run number
   KVString kvs = ((TObjString*)fields->At(0))->String().Remove(TString::kBoth,' ');
   if(kvs.IsDigit()){
      SetNumber(kvs.Atoi());
   } else {
      //not a valid line for run
      delete fields;
      return;
   }
   
 //  cout << GetName() << endl;
   
   //loop over other fields
   for(int i = 1; i < fields->GetEntries(); i++){
      
      //each field is of the form "parameter=value"
      KVString kvs = ((TObjString*)fields->At(i))->String().Remove(TString::kBoth,' ');
      TObjArray* toks = kvs.Tokenize('=');
      if( toks->GetEntries() == 2){
         KVString parameter = ((TObjString*)toks->At(0))->String().Remove(TString::kBoth,' ');
         KVString value = ((TObjString*)toks->At(1))->String().Remove(TString::kBoth,' ');
         //set parameter based on value
         if( value.IsDigit() ){
            //only true for non-floating point i.e. scaler values
            SetScaler(parameter.Data(), value.Atoi());
//            cout << " -- SCA " << parameter.Data() << " = " << GetScaler(parameter.Data()) << endl;
         }
         else if( value.IsFloat() ){
            Set(parameter.Data(), value.Atof());
  //          cout << " -- FLO " << parameter.Data() << " = " << Get(parameter.Data()) << endl;
         }
         else { // string value
            // as we write all strings as 'parname=value', there is a problem if the string 'value'
            // itself contains the '=' symbol !!
            // therefore we replace any '=' in the string by '\equal' before writing in the file
            // (see WriteRunListLine).
            // when reading back, we replace '\equal' by '='
            value.ReplaceAll("\\equal","=");
            Set(parameter.Data(), value.Data());
    //        cout << " -- STR " << parameter.Data() << " = " << GetString(parameter.Data()) << endl;
         }
      }
      delete toks;
   }
   
   delete fields;
}

//___________________________________________________________________________

void KVDBRun::WriteRunListHeader(ostream & outstr, Char_t delim) const
{
   //Write the version flag

   outstr << "Version=10" << endl;
}

void KVDBRun::UnsetSystem()
{
   //If this run has previously been associated with a system in the database,
   //this will remove the association. The run will also be removed from the system's
   //list of associated runs.
   
   if (GetSystem()) {
      GetSystem()->RemoveRun(this);
   }
   SetTitle("Experimental run");
}

KVDBSystem *KVDBRun::GetSystem() const
{
   if (GetKey("Systems")) {
      if (GetKey("Systems")->GetLinks()->GetSize())
         return (KVDBSystem *) GetKey("Systems")->GetLinks()->First();
   }
   return 0;
}

void KVDBRun::SetSystem(KVDBSystem * system)
{
   //Set system for run. Any previous system is unassociated (run will be removed from system's list)
   if (!GetKey("Systems")) {
      KVDBKey *key = AddKey("Systems", "Physical system used");
      key->SetUniqueStatus(kTRUE);
      key->SetSingleStatus(kTRUE);
   } else {
      UnsetSystem();
   }
   if (!AddLink("Systems", system)) {
      Warning("SetSystem(KVDBSystem*)",
              "System %s couldn't be set for Run %d. This bizarre...",
              system->GetName(), GetNumber());
   } else {
      //set title of run = name of system
      SetTitle(system->GetName());
   }
}

