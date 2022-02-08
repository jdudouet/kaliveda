/*
$Id: KVIDZoneLine.cpp,v 1.2 2009/03/03 13:36:00 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 13:36:00 $
*/

//Created by KVClassFactory on Fri Feb 15 15:00:17 2008
//Author: franklan

#include "KVIDZoneLine.h"
#include "Riostream.h"
#include "KVIDGraph.h"

using namespace std;

ClassImp(KVIDZoneLine)

KVIDZoneLine::KVIDZoneLine() : fAcceptedDirection("above")
{
   // Default constructor
}

KVIDZoneLine::~KVIDZoneLine()
{
   // Destructor
}

//__________________________________________________________

void KVIDZoneLine::WriteAsciiFile_extras(ofstream& file, const Char_t*)
{
   // Write accepted direction for Zone

   if (fAcceptedDirection == "") file << "[undefined accepted direction]" << endl;
   else file << fAcceptedDirection.Data() << endl;
}

//__________________________________________________________

void KVIDZoneLine::ReadAsciiFile_extras(ifstream& file)
{
   // Read accepted direction for Zone

   fAcceptedDirection.ReadLine(file);
   if (fAcceptedDirection == "[undefined accepted direction]") fAcceptedDirection = "";
}

void KVIDZoneLine::SetAcceptedDirection(const Char_t* dir)
{
   // Set the direction of the acceptable region relative to the Zone line
   // E.g. if points to identify must be above this Zone, use
   //    Zone->SetAcceptedDirection("above")
   // Possible values are: "above", "below", "left" or "right"
   // (see KVIDLine::WhereAmI).
   fAcceptedDirection = dir;
   if (GetParent()) GetParent()->Modified();
}

