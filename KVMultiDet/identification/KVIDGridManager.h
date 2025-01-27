/***************************************************************************
                          KVIDGridManager.h  -  description
                             -------------------
    begin                : Jan 27 2005
    copyright            : (C) 2005 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVIDGridManager.h,v 1.11 2009/03/03 14:27:15 franklan Exp $
***************************************************************************/

#ifndef KVIDGridManager_H
#define KVIDGridManager_H

#include "KVList.h"
#include "KVIDGraph.h"
#include "RQ_OBJECT.h"

/**
\class KVIDGridManager
\brief Handles a stock of identification grids to be used by one or more identification telescopes
\ingroup Identification

Each KVIDTelescope asks the grid manager for a grid (FindGrid), if one is available its
address is given to the telescope to use. In this way many IDTelescopes can use the same
grid without making multiple copies.
*/

class KVIDGridManager: public KVBase {

   RQ_OBJECT("KVIDGridManager")

   friend class KVIDGraph;

private:

   KVList* fGrids;              //collection of all ID graphs handled by manager
   TList fLastReadGrids;        //! list of grids created by last call to ReadAsciiFile

protected:

   void AddGrid(KVIDGraph*);

public:

   KVIDGridManager();
   virtual ~ KVIDGridManager();

   void DeleteGrid(KVIDGraph*, Bool_t update = kTRUE);
   KVIDGraph* GetGrid(const Char_t* name);

   KVList* GetGrids()
   {
      return fGrids;
   };

   void Clear(Option_t* opt = "");
   Bool_t ReadAsciiFile(const Char_t* filename);
   const TList* GetLastReadGrids() const
   {
      // List containing grids created by most recent call to ReadAsciiFile
      return &fLastReadGrids;
   }
   Int_t WriteAsciiFile(const Char_t* filename, const TCollection* selection = 0);

   void Print(Option_t* /*opt*/ = "") const
   {
      ls();
   };
   void ls(Option_t* /*opt*/ = "") const
   {
      fGrids->ls();
   };

   void StartViewer() const;

   void Modified()
   {
      Emit("Modified()");
   };                           // *SIGNAL*

   KVList* GetGridsForIDTelescope(const Char_t* label)
   {
      // Returns list of ID grids associated to ID telescopes of type 'label'
      // (note: 'type' is actually value of KVIDTelescope::GetLabel)
      // DELETE LIST AFTER USE !!!

      return (KVList*)fGrids->GetSubListWithMethod(label, "GetIDTelescopeLabel");
      //N.B. cast to (KVList*) is valid as long as fGrids is a KVList
   };
   void GetListOfIDTelescopeLabels(KVString&);
   void Initialize(Option_t* /*opt*/ = "");

   ClassDef(KVIDGridManager, 0) //Handles a collection of identification grids
};

//................  global variable
R__EXTERN KVIDGridManager* gIDGridManager;

#endif
