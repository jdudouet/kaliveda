/***************************************************************************
                          KVIDChIoCsI.cpp  -  description
                             -------------------
    begin                : Fri Feb 20 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVIDChIoCsI.h"
#include "KVReconstructedNucleus.h"
#include "KVINDRACodes.h"

ClassImp(KVIDChIoCsI)
/////////////////////////////////////////////////////////////////////
//KVIDChIoCsI
//
//Identification in ChIo-CsI matrices of INDRA
//
//ID subcodes are written in bits 4-7 of KVIDSubCodeManager
//(see KVINDRACodes)
KVIDChIoCsI::KVIDChIoCsI()
{
   //set ID code for telescope
   fIDCode = kIDCode_ChIoCsI;
   fZminCode = kIDCode_ArretChIo;
   fECode = kECode1;
   SetSubCodeManager(4, 7);
}
