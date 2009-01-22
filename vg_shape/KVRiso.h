//
// D.Cussol 
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVRiso_h
#define KVRiso_h
#include "KVVarGlob.h"

//#define DEBUG_KVRiso

class KVRiso:public KVVarGlob {
 public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
// Champs intermediaires 
 protected:
    Double_t Riso;
   Double_t Epar;
   Double_t Etrans;

// Methodes
 protected:
   void init_KVRiso(void);

 public:
    KVRiso(void);               // constructeur par defaut
    KVRiso(Char_t * nom);
    KVRiso(const KVRiso & a);   // constructeur par Copy

    virtual ~ KVRiso(void);     // destructeur

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject & obj) const;
#else
   virtual void Copy(TObject & obj);
#endif

    KVRiso & operator =(const KVRiso & a);      // operateur =

   virtual void Init(void);     // methode d'initialisation des
   // variables Internes
   virtual void Reset(void);    // Remise a zero avant le
   // traitement d'un evenement
   virtual void Fill(KVNucleus * c);    // Remplissage de la variable.

   virtual Double_t GetValue(Int_t i);  // on retourne la ieme valeur du
   // tableau
   virtual Double_t GetValue(Char_t * name);    // on retourne la valeur de
   // la variable "name"
   virtual Double_t GetValue(void) const;       // On retourne la valeur de la

   virtual Double_t *GetValuePtr(void); // On retourne la valeur de la

   virtual void CalculateRatio(void);   //on calculate the value of Riso

    ClassDef(KVRiso, 1)         // Global variable Riso=Sum(epar)/2*Sum(eper)
};
#endif
