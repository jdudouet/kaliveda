//Created by KVClassFactory on Fri Feb 17 09:32:40 2017
//Author: John Frankland,,,

#ifndef __KVNAMEVALUELISTGUI_H
#define __KVNAMEVALUELISTGUI_H

#include "TGFrame.h"
#include "RQ_OBJECT.h"
#include "KVNameValueList.h"

#include <TGButton.h>
#include <TObjArray.h>

/**
\class KVNameValueListGUI
\brief GUI for setting KVNameValueList parameters
\ingroup GUI
*/

class KVNameValueListGUI {
   RQ_OBJECT("KVNameValueListGUI")
private:
   KVNameValueList* theList;
   KVNameValueList fOriginal;

   TGTransientFrame* fMain;
   TGTextButton* fOKBut;        //OK button
   TGTextButton* fCancelBut;    //Cancel button
   Bool_t* fOK;                 //set to kTRUE if OK button is pressed
   UInt_t max_width;
   UInt_t fWidth, fHeight;
   TObjArray fData;

   Bool_t* fCancel;
   Bool_t fWaitForMain;

protected:
   virtual TObject* AddAString(Int_t i, TGHorizontalFrame* hf);
   virtual TObject* AddABool(Int_t i, TGHorizontalFrame* hf);
   virtual TObject* AddADouble(Int_t i, TGHorizontalFrame* hf);
   virtual TObject* AddAInt(Int_t i, TGHorizontalFrame* hf);

   TObject* GetDataWidget(int i) const
   {
      return fData[i];
   }
   KVNameValueList* GetList() const
   {
      return theList;
   }
   TGTransientFrame* GetMain() const
   {
      return fMain;
   }
   TObject* GetDataWidget(const TString& parname) const
   {
      // Return pointer to widget corresponding to named parameter in list

      auto index = theList->GetNameIndex(parname);
      if (index < 0) return nullptr;
      return GetDataWidget(index);
   }

public:
   KVNameValueListGUI(const TGWindow* main, KVNameValueList* params, Bool_t* cancel_pressed, Bool_t wait_for_main = kTRUE);

   virtual ~KVNameValueListGUI();

   void ReadData();
   void RestoreData();
   void DoClose();
   void CloseWindow();

   bool EnableDependingOnBool(const TString& value_to_enable, const TString& bool_parameter);

   ClassDef(KVNameValueListGUI, 1) //GUI for setting KVNameValueList parameters
   void DisplayDialog();
};

#endif
