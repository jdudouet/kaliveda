//Created by KVClassFactory on Fri Sep 11 10:44:41 2015
//Author: fable

#ifndef __NEWROOMINUIT_H
#define __NEWROOMINUIT_H

#include "KVConfig.h"
#include "TObject.h"
#include "TStopwatch.h"
#include <fstream>
#include "TMatrixDSymfwd.h"
#include <vector>
#include <string>
#include <map>

class RooAbsReal ;
class NewRooFitResult ;
class RooArgList ;
class RooRealVar ;
class RooArgSet ;
class RooAbsArg ;
class TVirtualFitter ;
class TH2F ;
class RooPlot ;

void RooMinuitGlue(Int_t& /*np*/, Double_t* /*gin*/,  Double_t& f, Double_t* par, Int_t /*flag*/) ;


class NewRooMinuit : public TObject {

public:

   NewRooMinuit(RooAbsReal& function) ;
   virtual ~NewRooMinuit() ;

   enum Strategy { Speed = 0, Balance = 1, Robustness = 2 } ;
   enum PrintLevel { None = -1, Reduced = 0, Normal = 1, ExtraForProblem = 2, Maximum = 3 } ;
   void setStrategy(Int_t strat) ;
   void setErrorLevel(Double_t level) ;
   void setEps(Double_t eps) ;
   void setMaxIter(Int_t niter = 5000) ;                   //Modified
   inline Double_t getEps() const
   {
      return _eps ;   //Modified
   }
   inline Int_t getNMaxIter() const
   {
      return _niter ;   //Modified
   }
   void optimizeConst(Int_t flag) ;
   void setEvalErrorWall(Bool_t flag)
   {
      _doEvalErrorWall = flag ;
   }
   void setOffsetting(Bool_t flag) ;

   NewRooFitResult* fit(const char* options) ;

   Int_t migrad() ;
   Int_t hesse() ;
   Int_t minos() ;
   Int_t minos(const RooArgSet& minosParamList) ;  // added FMV, 08/18/03
   Int_t seek() ;
   Int_t simplex() ;
   Int_t improve() ;

   NewRooFitResult* save(const char* name = 0, const char* title = 0) ;
   RooPlot* contour(RooRealVar& var1, RooRealVar& var2,
                    Double_t n1 = 1, Double_t n2 = 2, Double_t n3 = 0,
                    Double_t n4 = 0, Double_t n5 = 0, Double_t n6 = 0) ;

   Int_t setPrintLevel(Int_t newLevel) ;
   void setNoWarn() ;
   Int_t setWarnLevel(Int_t newLevel) ;
   void setPrintEvalErrors(Int_t numEvalErrors)
   {
      _printEvalErrors = numEvalErrors ;
   }
   void setVerbose(Bool_t flag = kTRUE)
   {
      _verbose = flag ;
   }
   void setProfile(Bool_t flag = kTRUE)
   {
      _profile = flag ;
   }
   void setMaxEvalMultiplier(Int_t n)
   {
      _maxEvalMult = n ;
   }
   Bool_t setLogFile(const char* logfile = 0) ;

   static void cleanup() ;

   Int_t evalCounter() const
   {
      return _evalCounter ;
   }
   void zeroEvalCount()
   {
      _evalCounter = 0 ;
   }

protected:

   friend class RooAbsPdf ;
   friend class NewRooAddPdf ;   //Modified

   void applyCovarianceMatrix(TMatrixDSym& V) ;

   friend void RooMinuitGlue(Int_t& np, Double_t* gin, Double_t& f, Double_t* par, Int_t flag) ;

   void profileStart() ;
   void profileStop() ;

   Bool_t synchronize(Bool_t verbose) ;
   void backProp() ;

   inline Int_t getNPar() const
   {
      return _nPar ;
   }
   inline std::ofstream* logfile() const
   {
      return _logfile ;
   }
   inline Double_t& maxFCN()
   {
      return _maxFCN ;
   }

   Double_t _eps;                                          //Modified
   Int_t _niter;                                           //Modified
   Double_t getPdfParamVal(Int_t index) ;
   Double_t getPdfParamErr(Int_t index) ;
   virtual Bool_t setPdfParamVal(Int_t index, Double_t value, Bool_t verbose = kFALSE) ;
   void setPdfParamErr(Int_t index, Double_t value) ;
   void setPdfParamErr(Int_t index, Double_t loVal, Double_t hiVal) ;
   void clearPdfParamAsymErr(Int_t index) ;

   void saveStatus(const char* label, Int_t status)
   {
      _statusHistory.push_back(std::pair<std::string, int>(label, status)) ;
   }

   void updateFloatVec() ;

private:

   Int_t       _evalCounter ;
   Int_t       _printLevel ;
   Int_t       _warnLevel ;
   Int_t       _status ;
   Int_t       _optConst ;
   Bool_t      _profile ;
   Bool_t      _handleLocalErrors ;
   Int_t       _numBadNLL ;
   Int_t       _nPar ;
   Int_t       _printEvalErrors ;
   Bool_t      _doEvalErrorWall ;
   Int_t       _maxEvalMult ;
   RooArgList* _floatParamList ;
   std::vector<RooAbsArg*> _floatParamVec ;
   RooArgList* _initFloatParamList ;
   RooArgList* _constParamList ;
   RooArgList* _initConstParamList ;
   RooAbsReal* _func ;

   Double_t    _maxFCN ;
   std::ofstream*   _logfile ;
   Bool_t      _verbose ;
   TStopwatch  _timer ;
   TStopwatch  _cumulTimer ;

   TMatrixDSym* _extV ;

   static TVirtualFitter* _theFitter ;

   std::vector<std::pair<std::string, int> > _statusHistory ;

   NewRooMinuit(const NewRooMinuit&): TObject() {}
   ROOT_COPY_ASSIGN_OP(NewRooMinuit)

   ClassDef(NewRooMinuit, 1) //Modified Roofit class NewRooMinuit in order to use it with NewRooAddPdf
};

#endif
