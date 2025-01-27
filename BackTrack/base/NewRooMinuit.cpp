//Created by KVClassFactory on Fri Sep 11 10:44:41 2015
//Author: fable

#include "RooFit.h"
#include "Riostream.h"

#include "TClass.h"

#include <fstream>
#include <iomanip>
#include "TH1.h"
#include "TH2.h"
#include "TMarker.h"
#include "TGraph.h"
#include "TStopwatch.h"
#include "TFitter.h"
#include "TMinuit.h"
#include "TDirectory.h"
#include "TMatrixDSym.h"
#include "NewRooMinuit.h"
#include "RooArgSet.h"
#include "RooArgList.h"
#include "RooAbsReal.h"
#include "RooAbsRealLValue.h"
#include "RooRealVar.h"
#include "NewRooFitResult.h"
#include "RooAbsPdf.h"
#include "RooSentinel.h"
#include "RooMsgService.h"
#include "RooPlot.h"
#include "TVirtualFitter.h"



#if (__GNUC__==3&&__GNUC_MINOR__==2&&__GNUC_PATCHLEVEL__==3)
char* operator+(streampos&, char*);
#endif

using namespace std;


ClassImp(NewRooMinuit)

TVirtualFitter* NewRooMinuit::_theFitter = 0 ;


////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>NewRooMinuit</h2>
<h4>Modified Roofit class NewRooMinuit in order to use it with NewRooAddPdf</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

//_____________________________________________________________________________
void NewRooMinuit::cleanup()
{
   // Cleanup method called by atexit handler installed by RooSentinel
   // to delete all global heap objects when the program is terminated
   if (_theFitter) {
      delete _theFitter ;
      _theFitter = 0 ;
   }
}



//_____________________________________________________________________________
NewRooMinuit::NewRooMinuit(RooAbsReal& function)
{
   // Construct MINUIT interface to given function. Function can be anything,
   // but is typically a -log(likelihood) implemented by RooNLLVar or a chi^2
   // (implemented by RooChi2Var). Other frequent use cases are a RooAddition
   // of a RooNLLVar plus a penalty or constraint term. This class propagates
   // all RooFit information (floating parameters, their values and errors)
   // to MINUIT before each MINUIT call and propagates all MINUIT information
   // back to the RooFit object at the end of each call (updated parameter
   // values, their (asymmetric errors) etc. The default MINUIT error level
   // for HESSE and MINOS error analysis is taken from the defaultErrorLevel()
   // value of the input function.

   RooSentinel::activate() ;

   // Store function reference
   _evalCounter = 0 ;
   _extV = 0 ;
   _func = &function ;
   _logfile = 0 ;
   _optConst = kFALSE ;
   _verbose = kFALSE ;
   _profile = kFALSE ;
   _handleLocalErrors = kTRUE ;
   _printLevel = 1 ;
   _printEvalErrors = 10 ;
   _warnLevel = -999 ;
   _maxEvalMult = 500 ;
   _doEvalErrorWall = kTRUE ;

   // Examine parameter list
   RooArgSet* paramSet = function.getParameters(RooArgSet()) ;
   RooArgList paramList(*paramSet) ;
   delete paramSet ;

   _floatParamList = (RooArgList*) paramList.selectByAttrib("Constant", kFALSE) ;
   if (_floatParamList->getSize() > 1) {
      _floatParamList->sort() ;
   }
   _floatParamList->setName("floatParamList") ;

   _constParamList = (RooArgList*) paramList.selectByAttrib("Constant", kTRUE) ;
   if (_constParamList->getSize() > 1) {
      _constParamList->sort() ;
   }
   _constParamList->setName("constParamList") ;

   // Remove all non-RooRealVar parameters from list (MINUIT cannot handle them)
   TIterator* pIter = _floatParamList->createIterator() ;
   RooAbsArg* arg ;
   while ((arg = (RooAbsArg*)pIter->Next())) {
      if (!arg->IsA()->InheritsFrom(RooAbsRealLValue::Class())) {
         coutW(Minimization) << "NewRooMinuit::NewRooMinuit: removing parameter " << arg->GetName()
                             << " from list because it is not of type RooRealVar" << endl ;
         _floatParamList->remove(*arg) ;
      }
   }
   _nPar      = _floatParamList->getSize() ;
   delete pIter ;

   updateFloatVec() ;

   // Save snapshot of initial lists
   _initFloatParamList = (RooArgList*) _floatParamList->snapshot(kFALSE) ;
   _initConstParamList = (RooArgList*) _constParamList->snapshot(kFALSE) ;

   // Initialize MINUIT
   Int_t nPar = _floatParamList->getSize() + _constParamList->getSize() ;
   if (_theFitter) delete _theFitter ;
   _theFitter = TVirtualFitter::Fitter(this, nPar * 2 + 1);

   //_theFitter = new TFitter(nPar*2+1) ; //WVE Kludge, nPar*2 works around TMinuit memory allocation bug
   //_theFitter->SetObjectFit(this) ;

   // Shut up for now
   setPrintLevel(-1) ;
   _theFitter->Clear();

   // Tell MINUIT to use our global glue function
   _theFitter->SetFCN(RooMinuitGlue);

   // Use +0.5 for 1-sigma errors
   setErrorLevel(function.defaultErrorLevel()) ;

   // Declare our parameters to MINUIT
   synchronize(kFALSE) ;

   // Reset the *largest* negative log-likelihood value we have seen so far
   _maxFCN = -1e30 ;
   _numBadNLL = 0 ;

   // Now set default verbosity
   if (RooMsgService::instance().silentMode()) {
      setWarnLevel(-1) ;
      setPrintLevel(-1) ;
   }
   else {
      setWarnLevel(1) ;
      setPrintLevel(1) ;
   }
}



//_____________________________________________________________________________
NewRooMinuit::~NewRooMinuit()
{
   // Destructor

   delete _floatParamList ;
   delete _initFloatParamList ;
   delete _constParamList ;
   delete _initConstParamList ;
   if (_extV) {
      delete _extV ;
   }
}



//_____________________________________________________________________________
void NewRooMinuit::setStrategy(Int_t istrat)
{
   // Change MINUIT strategy to istrat. Accepted codes
   // are 0,1,2 and represent MINUIT strategies for dealing
   // most efficiently with fast FCNs (0), expensive FCNs (2)
   // and 'intermediate' FCNs (1)

   Double_t stratArg(istrat) ;
   _theFitter->ExecuteCommand("SET STR", &stratArg, 1) ;
}



//_____________________________________________________________________________
void NewRooMinuit::setErrorLevel(Double_t level)
{
   // Set the level for MINUIT error analysis to the given
   // value. This function overrides the default value
   // that is taken in the NewRooMinuit constructor from
   // the defaultErrorLevel() method of the input function
   _theFitter->ExecuteCommand("SET ERR", &level, 1);
}



//_____________________________________________________________________________
void NewRooMinuit::setEps(Double_t eps)
{
   // Change MINUIT epsilon
   _eps = eps;
   _theFitter->ExecuteCommand("SET EPS", &_eps, 1) ;
}


//_____________________________________________________________________________
void NewRooMinuit::setMaxIter(Int_t niter)
{
   // Change MINUIT number of maximum iterations
   _niter = niter;
   _theFitter->SetMaxIterations(_niter);
}

//_____________________________________________________________________________
void NewRooMinuit::setOffsetting(Bool_t flag)
{
   // Enable internal likelihood offsetting for enhanced numeric precision
   _func->enableOffsetting(flag) ;
}


//_____________________________________________________________________________
NewRooFitResult* NewRooMinuit::fit(const char* options)
{
   // Parse traditional RooAbsPdf::fitTo driver options
   //
   //  s - Run Hesse first to estimate initial step size
   //  m - Run Migrad only
   //  h - Run Hesse to estimate errors
   //  v - Verbose mode
   //  l - Log parameters after each Minuit steps to file
   //  t - Activate profile timer
   //  r - Save fit result
   //  0 - Run Migrad with strategy 0

   if (_floatParamList->getSize() == 0) {
      return 0 ;
   }

   _theFitter->SetObjectFit(this) ;

   TString opts(options) ;
   opts.ToLower() ;

   // Initial configuration
   if (opts.Contains("v")) setVerbose(1) ;
   if (opts.Contains("t")) setProfile(1) ;
   if (opts.Contains("l")) setLogFile(Form("%s.log", _func->GetName())) ;
   if (opts.Contains("c")) optimizeConst(1) ;

   // Fitting steps
   if (opts.Contains("s")) hesse() ;
   if (opts.Contains("0")) setStrategy(0) ;
   migrad() ;
   if (opts.Contains("0")) setStrategy(1) ;
   if (opts.Contains("h") || !opts.Contains("m")) hesse() ;
   if (!opts.Contains("m")) minos() ;

   return (opts.Contains("r")) ? save() : 0 ;
}



//_____________________________________________________________________________
Int_t NewRooMinuit::migrad()
{
   // Execute MIGRAD. Changes in parameter values
   // and calculated errors are automatically
   // propagated back the RooRealVars representing
   // the floating parameters in the MINUIT operation

   if (_floatParamList->getSize() == 0) {
      return -1 ;
   }

   _theFitter->SetObjectFit(this) ;

   Double_t arglist[2];
   arglist[0] = _maxEvalMult * _nPar; // maximum iterations
   arglist[1] = 1.0;      // tolerance

   synchronize(_verbose) ;
   profileStart() ;
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::CollectErrors) ;
   RooAbsReal::clearEvalErrorLog() ;
   _status = _theFitter->ExecuteCommand("MIGRAD", arglist, 2);
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::PrintErrors) ;
   profileStop() ;
   backProp() ;

   saveStatus("MIGRAD", _status) ;

   return _status ;
}



//_____________________________________________________________________________
Int_t NewRooMinuit::hesse()
{
   // Execute HESSE. Changes in parameter values
   // and calculated errors are automatically
   // propagated back the RooRealVars representing
   // the floating parameters in the MINUIT operation

   if (_floatParamList->getSize() == 0) {
      return -1 ;
   }

   _theFitter->SetObjectFit(this) ;

   Double_t arglist[2];
   arglist[0] = _maxEvalMult * _nPar; // maximum iterations

   synchronize(_verbose) ;
   profileStart() ;
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::CollectErrors) ;
   RooAbsReal::clearEvalErrorLog() ;
   _status = _theFitter->ExecuteCommand("HESSE", arglist, 1);
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::PrintErrors) ;
   profileStop() ;
   backProp() ;

   saveStatus("HESSE", _status) ;

   return _status ;
}



//_____________________________________________________________________________
Int_t NewRooMinuit::minos()
{
   // Execute MINOS. Changes in parameter values
   // and calculated errors are automatically
   // propagated back the RooRealVars representing
   // the floating parameters in the MINUIT operation

   if (_floatParamList->getSize() == 0) {
      return -1 ;
   }

   _theFitter->SetObjectFit(this) ;

   Double_t arglist[2];
   arglist[0] = _maxEvalMult * _nPar; // maximum iterations

   synchronize(_verbose) ;
   profileStart() ;
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::CollectErrors) ;
   RooAbsReal::clearEvalErrorLog() ;
   _status = _theFitter->ExecuteCommand("MINOS", arglist, 1);
   // check also the status of Minos looking at fCstatu
   if (_status == 0 && gMinuit->fCstatu != "SUCCESSFUL") {
      if (gMinuit->fCstatu == "FAILURE" ||
            gMinuit->fCstatu == "PROBLEMS") _status = 5;
      _status = 6;
   }

   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::PrintErrors) ;
   profileStop() ;
   backProp() ;

   saveStatus("MINOS", _status) ;
   return _status ;
}


// added FMV, 08/18/03

//_____________________________________________________________________________
Int_t NewRooMinuit::minos(const RooArgSet& minosParamList)
{
   // Execute MINOS for given list of parameters. Changes in parameter values
   // and calculated errors are automatically
   // propagated back the RooRealVars representing
   // the floating parameters in the MINUIT operation

   if (_floatParamList->getSize() == 0) {
      return -1 ;
   }

   _theFitter->SetObjectFit(this) ;

   Int_t nMinosPar(0) ;
   Double_t* arglist = new Double_t[_nPar + 1];

   if (minosParamList.getSize() > 0) {
      TIterator* aIter = minosParamList.createIterator() ;
      RooAbsArg* arg ;
      while ((arg = (RooAbsArg*)aIter->Next())) {
         RooAbsArg* par = _floatParamList->find(arg->GetName());
         if (par && !par->isConstant()) {
            Int_t index = _floatParamList->index(par);
            nMinosPar++;
            arglist[nMinosPar] = index + 1;
         }
      }
      delete aIter ;
   }
   arglist[0] = _maxEvalMult * _nPar; // maximum iterations

   synchronize(_verbose) ;
   profileStart() ;
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::CollectErrors) ;
   RooAbsReal::clearEvalErrorLog() ;
   _status = _theFitter->ExecuteCommand("MINOS", arglist, 1 + nMinosPar);
   // check also the status of Minos looking at fCstatu
   if (_status == 0 && gMinuit->fCstatu != "SUCCESSFUL") {
      if (gMinuit->fCstatu == "FAILURE" ||
            gMinuit->fCstatu == "PROBLEMS") _status = 5;
      _status = 6;
   }
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::PrintErrors) ;
   profileStop() ;
   backProp() ;

   delete[] arglist ;

   saveStatus("MINOS", _status) ;

   return _status ;
}



//_____________________________________________________________________________
Int_t NewRooMinuit::seek()
{
   // Execute SEEK. Changes in parameter values
   // and calculated errors are automatically
   // propagated back the RooRealVars representing
   // the floating parameters in the MINUIT operation

   if (_floatParamList->getSize() == 0) {
      return -1 ;
   }

   _theFitter->SetObjectFit(this) ;

   Double_t arglist[2];
   arglist[0] = _maxEvalMult * _nPar; // maximum iterations

   synchronize(_verbose) ;
   profileStart() ;
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::CollectErrors) ;
   RooAbsReal::clearEvalErrorLog() ;
   _status = _theFitter->ExecuteCommand("SEEK", arglist, 1);
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::PrintErrors) ;
   profileStop() ;
   backProp() ;

   saveStatus("SEEK", _status) ;

   return _status ;
}



//_____________________________________________________________________________
Int_t NewRooMinuit::simplex()
{
   // Execute SIMPLEX. Changes in parameter values
   // and calculated errors are automatically
   // propagated back the RooRealVars representing
   // the floating parameters in the MINUIT operation

   if (_floatParamList->getSize() == 0) {
      return -1 ;
   }

   _theFitter->SetObjectFit(this) ;

   Double_t arglist[2];
   arglist[0] = _maxEvalMult * _nPar; // maximum iterations
   arglist[1] = 1.0;      // tolerance

   synchronize(_verbose) ;
   profileStart() ;
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::CollectErrors) ;
   RooAbsReal::clearEvalErrorLog() ;
   _status = _theFitter->ExecuteCommand("SIMPLEX", arglist, 2);
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::PrintErrors) ;
   profileStop() ;
   backProp() ;

   saveStatus("SIMPLEX", _status) ;

   return _status ;
}



//_____________________________________________________________________________
Int_t NewRooMinuit::improve()
{
   // Execute IMPROVE. Changes in parameter values
   // and calculated errors are automatically
   // propagated back the RooRealVars representing
   // the floating parameters in the MINUIT operation

   if (_floatParamList->getSize() == 0) {
      return -1 ;
   }

   _theFitter->SetObjectFit(this) ;

   Double_t arglist[2];
   arglist[0] = _maxEvalMult * _nPar; // maximum iterations

   synchronize(_verbose) ;
   profileStart() ;
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::CollectErrors) ;
   RooAbsReal::clearEvalErrorLog() ;
   _status = _theFitter->ExecuteCommand("IMPROVE", arglist, 1);
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::PrintErrors) ;
   profileStop() ;
   backProp() ;

   saveStatus("IMPROVE", _status) ;

   return _status ;
}



//_____________________________________________________________________________
Int_t NewRooMinuit::setPrintLevel(Int_t newLevel)
{
   // Change the MINUIT internal printing level
   Int_t ret = _printLevel ;
   Double_t arg(newLevel) ;
   _theFitter->ExecuteCommand("SET PRINT", &arg, 1);
   _printLevel = newLevel ;
   return ret ;
}



//_____________________________________________________________________________
void NewRooMinuit::setNoWarn()
{
   // Instruct MINUIT to suppress warnings

   Double_t arg(0) ;
   _theFitter->ExecuteCommand("SET NOWARNINGS", &arg, 1);
   _warnLevel = -1 ;
}



//_____________________________________________________________________________
Int_t NewRooMinuit::setWarnLevel(Int_t newLevel)
{
   // Set MINUIT warning level to given level

   if (newLevel == _warnLevel) {
      return _warnLevel ;
   }

   Int_t ret = _warnLevel ;
   Double_t arg(newLevel) ;

   if (newLevel >= 0) {
      _theFitter->ExecuteCommand("SET WARNINGS", &arg, 1);
   }
   else {
      Double_t arg2(0) ;
      _theFitter->ExecuteCommand("SET NOWARNINGS", &arg2, 1);
   }
   _warnLevel = newLevel ;

   return ret ;
}



//_____________________________________________________________________________
Bool_t NewRooMinuit::synchronize(Bool_t verbose)
{
   // Internal function to synchronize TMinuit with current
   // information in RooAbsReal function parameters

   Int_t oldPrint = setPrintLevel(-1) ;
   gMinuit->fNwrmes[0] = 0;  // to clear buffer
   Int_t oldWarn = setWarnLevel(-1) ;

   Bool_t constValChange(kFALSE) ;
   Bool_t constStatChange(kFALSE) ;

   Int_t index(0) ;

   // Handle eventual migrations from constParamList -> floatParamList
   for (index = 0; index < _constParamList->getSize() ; index++) {
      RooRealVar* par = dynamic_cast<RooRealVar*>(_constParamList->at(index)) ;
      if (!par) continue ;

      RooRealVar* oldpar = dynamic_cast<RooRealVar*>(_initConstParamList->at(index)) ;
      if (!oldpar) continue ;

      // Test if constness changed
      if (!par->isConstant()) {

         // Remove from constList, add to floatList
         _constParamList->remove(*par) ;
         _floatParamList->add(*par) ;
         _initFloatParamList->addClone(*oldpar) ;
         _initConstParamList->remove(*oldpar) ;
         constStatChange = kTRUE ;
         _nPar++ ;

         if (verbose) {
            coutI(Minimization) << "NewRooMinuit::synchronize: parameter " << par->GetName() << " is now floating." << endl ;
         }
      }

      // Test if value changed
      if (par->getVal() != oldpar->getVal()) {
         constValChange = kTRUE ;
         if (verbose) {
            coutI(Minimization) << "NewRooMinuit::synchronize: value of constant parameter " << par->GetName()
                                << " changed from " << oldpar->getVal() << " to " << par->getVal() << endl ;
         }
      }

   }

   // Update reference list
   *_initConstParamList = *_constParamList ;


   // Synchronize MINUIT with function state
   for (index = 0; index < _nPar; index++) {
      RooRealVar* par = dynamic_cast<RooRealVar*>(_floatParamList->at(index)) ;
      if (!par) continue ;

      Double_t pstep(0) ;
      Double_t pmin(0) ;
      Double_t pmax(0) ;

      if (!par->isConstant()) {

         // Verify that floating parameter is indeed of type RooRealVar
         if (!par->IsA()->InheritsFrom(RooRealVar::Class())) {
            coutW(Minimization) << "NewRooMinuit::fit: Error, non-constant parameter " << par->GetName()
                                << " is not of type RooRealVar, skipping" << endl ;
            continue ;
         }

         // Set the limits, if not infinite
         if (par->hasMin() && par->hasMax()) {
            pmin = par->getMin();
            pmax = par->getMax();
         }

         // Calculate step size
         pstep = par->getError();
         if (pstep <= 0) {
            // Floating parameter without error estitimate
            if (par->hasMin() && par->hasMax()) {
               pstep = 0.1 * (pmax - pmin);

               // Trim default choice of error if within 2 sigma of limit
               if (pmax - par->getVal() < 2 * pstep) {
                  pstep = (pmax - par->getVal()) / 2 ;
               }
               else if (par->getVal() - pmin < 2 * pstep) {
                  pstep = (par->getVal() - pmin) / 2 ;
               }

               // If trimming results in zero error, restore default
               if (pstep == 0) {
                  pstep = 0.1 * (pmax - pmin);
               }

            }
            else {
               pstep = 1 ;
            }
            if (_verbose) {
               coutW(Minimization) << "NewRooMinuit::synchronize: WARNING: no initial error estimate available for "
                                   << par->GetName() << ": using " << pstep << endl;
            }
         }
      }
      else {
         pmin = par->getVal() ;
         pmax = par->getVal() ;
      }

      // Extract previous information
      Double_t oldVar, oldVerr, oldVlo, oldVhi ;
      char oldParname[100] ;
      Int_t ierr = _theFitter->GetParameter(index, oldParname, oldVar, oldVerr, oldVlo, oldVhi)  ;

      // Determine if parameters is currently fixed in MINUIT

      Int_t ix ;
      Bool_t oldFixed(kFALSE) ;
      if (ierr >= 0) {
         for (ix = 1; ix <= gMinuit->fNpfix; ++ix) {
            if (gMinuit->fIpfix[ix - 1] == index + 1) oldFixed = kTRUE ;
         }
      }

      if (par->isConstant() && !oldFixed) {

         // Parameter changes floating -> constant : update only value if necessary
         if (oldVar != par->getVal()) {
            Double_t arglist[2] ;
            arglist[0] = index + 1 ;
            arglist[1] = par->getVal() ;
            _theFitter->ExecuteCommand("SET PAR", arglist, 2) ;
            if (verbose) {
               coutI(Minimization) << "NewRooMinuit::synchronize: value of parameter " << par->GetName() << " changed from " << oldVar << " to " << par->getVal() << endl ;
            }
         }

         _theFitter->FixParameter(index) ;
         constStatChange = kTRUE ;
         if (verbose) {
            coutI(Minimization) << "NewRooMinuit::synchronize: parameter " << par->GetName() << " is now fixed." << endl ;
         }

      }
      else if (par->isConstant() && oldFixed) {

         // Parameter changes constant -> constant : update only value if necessary
         if (oldVar != par->getVal()) {
            Double_t arglist[2] ;
            arglist[0] = index + 1 ;
            arglist[1] = par->getVal() ;
            _theFitter->ExecuteCommand("SET PAR", arglist, 2) ;
            constValChange = kTRUE ;

            if (verbose) {
               coutI(Minimization) << "NewRooMinuit::synchronize: value of fixed parameter " << par->GetName() << " changed from " << oldVar << " to " << par->getVal() << endl ;
            }
         }

      }
      else {

         if (!par->isConstant() && oldFixed) {
            _theFitter->ReleaseParameter(index) ;
            constStatChange = kTRUE ;

            if (verbose) {
               coutI(Minimization) << "NewRooMinuit::synchronize: parameter " << par->GetName() << " is now floating." << endl ;
            }
         }

         // Parameter changes constant -> floating : update all if necessary
         if (oldVar != par->getVal() || oldVlo != pmin || oldVhi != pmax || oldVerr != pstep) {
            _theFitter->SetParameter(index, par->GetName(), par->getVal(), pstep, pmin, pmax);
         }

         // Inform user about changes in verbose mode
         if (verbose && ierr >= 0) {
            // if ierr<0, par was moved from the const list and a message was already printed

            if (oldVar != par->getVal()) {
               coutI(Minimization) << "NewRooMinuit::synchronize: value of parameter " << par->GetName() << " changed from " << oldVar << " to " << par->getVal() << endl ;
            }
            if (oldVlo != pmin || oldVhi != pmax) {
               coutI(Minimization) << "NewRooMinuit::synchronize: limits of parameter " << par->GetName() << " changed from [" << oldVlo << "," << oldVhi
                                   << "] to [" << pmin << "," << pmax << "]" << endl ;
            }

            // If oldVerr=0, then parameter was previously fixed
            if (oldVerr != pstep && oldVerr != 0) {
               coutI(Minimization) << "NewRooMinuit::synchronize: error/step size of parameter " << par->GetName() << " changed from " << oldVerr << " to " << pstep << endl ;
            }
         }
      }
   }


   gMinuit->fNwrmes[0] = 0;  // to clear buffer
   oldWarn = setWarnLevel(oldWarn) ;
   oldPrint = setPrintLevel(oldPrint) ;

   if (_optConst) {
      if (constStatChange) {

         RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::CollectErrors) ;

         coutI(Minimization) << "NewRooMinuit::synchronize: set of constant parameters changed, rerunning const optimizer" << endl ;
         _func->constOptimizeTestStatistic(RooAbsArg::ConfigChange) ;
      }
      else if (constValChange) {
         coutI(Minimization) << "NewRooMinuit::synchronize: constant parameter values changed, rerunning const optimizer" << endl ;
         _func->constOptimizeTestStatistic(RooAbsArg::ValueChange) ;
      }

      RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::PrintErrors) ;

   }

   updateFloatVec() ;

   return 0 ;
}




//_____________________________________________________________________________
void NewRooMinuit::optimizeConst(Int_t flag)
{
   // If flag is true, perform constant term optimization on
   // function being minimized.

   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::CollectErrors) ;

   if (_optConst && !flag) {
      if (_printLevel > -1) coutI(Minimization) << "NewRooMinuit::optimizeConst: deactivating const optimization" << endl ;
      _func->constOptimizeTestStatistic(RooAbsArg::DeActivate, flag > 1) ;
      _optConst = flag ;
   }
   else if (!_optConst && flag) {
      if (_printLevel > -1) coutI(Minimization) << "NewRooMinuit::optimizeConst: activating const optimization" << endl ;
      _func->constOptimizeTestStatistic(RooAbsArg::Activate, flag > 1) ;
      _optConst = flag ;
   }
   else if (_optConst && flag) {
      if (_printLevel > -1) coutI(Minimization) << "NewRooMinuit::optimizeConst: const optimization already active" << endl ;
   }
   else {
      if (_printLevel > -1) coutI(Minimization) << "NewRooMinuit::optimizeConst: const optimization wasn't active" << endl ;
   }

   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::PrintErrors) ;

}



//_____________________________________________________________________________
NewRooFitResult* NewRooMinuit::save(const char* userName, const char* userTitle)
{
   // Save and return a NewRooFitResult snaphot of current minimizer status.
   // This snapshot contains the values of all constant parameters,
   // the value of all floating parameters at NewRooMinuit construction and
   // after the last MINUIT operation, the MINUIT status, variance quality,
   // EDM setting, number of calls with evaluation problems, the minimized
   // function value and the full correlation matrix

   TString name, title ;
   name = userName ? userName : Form("%s", _func->GetName()) ;
   title = userTitle ? userTitle : Form("%s", _func->GetTitle()) ;

   if (_floatParamList->getSize() == 0) {
      NewRooFitResult* fitRes = new NewRooFitResult(name, title) ;
      fitRes->setConstParList(*_constParamList) ;
      fitRes->setInitParList(RooArgList()) ;
      fitRes->setFinalParList(RooArgList()) ;
      fitRes->setStatus(-999) ;
      fitRes->setCovQual(-999) ;
      fitRes->setMinNLL(_func->getVal()) ;
      fitRes->setNumInvalidNLL(0) ;
      fitRes->setEDM(-999) ;
      return fitRes ;
   }

   NewRooFitResult* fitRes = new NewRooFitResult(name, title) ;

   // Move eventual fixed parameters in floatList to constList
   Int_t i ;
   RooArgList saveConstList(*_constParamList) ;
   RooArgList saveFloatInitList(*_initFloatParamList) ;
   RooArgList saveFloatFinalList(*_floatParamList) ;
   for (i = 0 ; i < _floatParamList->getSize() ; i++) {
      RooAbsArg* par = _floatParamList->at(i) ;
      if (par->isConstant()) {
         saveFloatInitList.remove(*saveFloatInitList.find(par->GetName()), kTRUE) ;
         saveFloatFinalList.remove(*par) ;
         saveConstList.add(*par) ;
      }
   }
   saveConstList.sort() ;

   fitRes->setConstParList(saveConstList) ;
   fitRes->setInitParList(saveFloatInitList) ;

   Double_t edm, errdef, minVal;
   Int_t nvpar, nparx;
   Int_t icode = _theFitter->GetStats(minVal, edm, errdef, nvpar, nparx);
   fitRes->setStatus(_status) ;
   fitRes->setCovQual(icode) ;
   fitRes->setMinNLL(minVal) ;
   fitRes->setNumInvalidNLL(_numBadNLL) ;
   fitRes->setEDM(edm) ;
   fitRes->setFinalParList(saveFloatFinalList) ;
   if (!_extV) {
      fitRes->fillCorrMatrix() ;
   }
   else {
      fitRes->setCovarianceMatrix(*_extV) ;
   }

   fitRes->setStatusHistory(_statusHistory) ;

   return fitRes ;
}




//_____________________________________________________________________________
RooPlot* NewRooMinuit::contour(RooRealVar& var1, RooRealVar& var2, Double_t n1, Double_t n2, Double_t n3, Double_t n4, Double_t n5, Double_t n6)
{
   // Create and draw a TH2 with the error contours in parameters var1 and v2 at up to 6 'sigma' settings
   // where 'sigma' is calculated as n*n*errorLevel


   _theFitter->SetObjectFit(this) ;

   RooArgList* paramSave = (RooArgList*) _floatParamList->snapshot() ;

   // Verify that both variables are floating parameters of PDF
   Int_t index1 = _floatParamList->index(&var1);
   if (index1 < 0) {
      coutE(Minimization) << "NewRooMinuit::contour(" << GetName()
                          << ") ERROR: " << var1.GetName() << " is not a floating parameter of " << _func->GetName() << endl ;
      return 0;
   }

   Int_t index2 = _floatParamList->index(&var2);
   if (index2 < 0) {
      coutE(Minimization) << "NewRooMinuit::contour(" << GetName()
                          << ") ERROR: " << var2.GetName() << " is not a floating parameter of PDF " << _func->GetName() << endl ;
      return 0;
   }

   // create and draw a frame
   RooPlot* frame = new RooPlot(var1, var2) ;

   // draw a point at the current parameter values
   TMarker* point = new TMarker(var1.getVal(), var2.getVal(), 8);
   frame->addObject(point) ;

   // remember our original value of ERRDEF
   Double_t errdef = gMinuit->fUp;

   Double_t n[6] ;
   n[0] = n1 ;
   n[1] = n2 ;
   n[2] = n3 ;
   n[3] = n4 ;
   n[4] = n5 ;
   n[5] = n6 ;


   for (Int_t ic = 0 ; ic < 6 ; ic++) {
      if (n[ic] > 0) {
         // set the value corresponding to an n1-sigma contour
         gMinuit->SetErrorDef(n[ic]*n[ic]*errdef);
         // calculate and draw the contour
         TGraph* graph = (TGraph*)gMinuit->Contour(50, index1, index2);
         if (!graph) {
            coutE(Minimization) << "NewRooMinuit::contour(" << GetName() << ") ERROR: MINUIT did not return a contour graph for n=" << n[ic] << endl ;
         }
         else {
            graph->SetName(Form("contour_%s_n%f", _func->GetName(), n[ic])) ;
            graph->SetLineStyle(ic + 1) ;
            graph->SetLineWidth(2) ;
            graph->SetLineColor(kBlue) ;
            frame->addObject(graph, "L") ;
         }
      }
   }

   // restore the original ERRDEF
   gMinuit->SetErrorDef(errdef);

   // restore parameter values
   *_floatParamList = *paramSave ;
   delete paramSave ;


   return frame ;
}



//_____________________________________________________________________________
Bool_t NewRooMinuit::setLogFile(const char* inLogfile)
{
   // Change the file name for logging of a NewRooMinuit of all MINUIT steppings
   // through the parameter space. If inLogfile is null, the current log file
   // is closed and logging is stopped.

   if (_logfile) {
      coutI(Minimization) << "NewRooMinuit::setLogFile: closing previous log file" << endl ;
      _logfile->close() ;
      delete _logfile ;
      _logfile = 0 ;
   }
   _logfile = new ofstream(inLogfile) ;
   if (!_logfile->good()) {
      coutI(Minimization) << "NewRooMinuit::setLogFile: cannot open file " << inLogfile << endl ;
      _logfile->close() ;
      delete _logfile ;
      _logfile = 0;
   }
   return kFALSE ;
}



//_____________________________________________________________________________
Double_t NewRooMinuit::getPdfParamVal(Int_t index)
{
   // Access PDF parameter value by ordinal index (needed by MINUIT)

   return ((RooRealVar*)_floatParamList->at(index))->getVal() ;
}



//_____________________________________________________________________________
Double_t NewRooMinuit::getPdfParamErr(Int_t index)
{
   // Access PDF parameter error by ordinal index (needed by MINUIT)

   return ((RooRealVar*)_floatParamList->at(index))->getError() ;
}



//_____________________________________________________________________________
Bool_t NewRooMinuit::setPdfParamVal(Int_t index, Double_t value, Bool_t verbose)
{
   // Modify PDF parameter value by ordinal index (needed by MINUIT)

   //RooRealVar* par = (RooRealVar*)_floatParamList->at(index) ;
   RooRealVar* par = (RooRealVar*)_floatParamVec[index] ;

   if (par->getVal() != value) {
      if (verbose) cout << par->GetName() << "=" << value << ", " ;
      par->setVal(value) ;
      return kTRUE ;
   }

   return kFALSE ;
}



//_____________________________________________________________________________
void NewRooMinuit::setPdfParamErr(Int_t index, Double_t value)
{
   // Modify PDF parameter error by ordinal index (needed by MINUIT)

   ((RooRealVar*)_floatParamList->at(index))->setError(value) ;
}



//_____________________________________________________________________________
void NewRooMinuit::clearPdfParamAsymErr(Int_t index)
{
   // Modify PDF parameter error by ordinal index (needed by MINUIT)

   ((RooRealVar*)_floatParamList->at(index))->removeAsymError() ;
}


//_____________________________________________________________________________
void NewRooMinuit::setPdfParamErr(Int_t index, Double_t loVal, Double_t hiVal)
{
   // Modify PDF parameter error by ordinal index (needed by MINUIT)

   ((RooRealVar*)_floatParamList->at(index))->setAsymError(loVal, hiVal) ;
}



//_____________________________________________________________________________
void NewRooMinuit::profileStart()
{
   // Start profiling timer
   if (_profile) {
      _timer.Start() ;
      _cumulTimer.Start(kFALSE) ;
   }
}




//_____________________________________________________________________________
void NewRooMinuit::profileStop()
{
   // Stop profiling timer and report results of last session
   if (_profile) {
      _timer.Stop() ;
      _cumulTimer.Stop() ;
      coutI(Minimization) << "Command timer: " ;
      _timer.Print() ;
      coutI(Minimization) << "Session timer: " ;
      _cumulTimer.Print() ;
   }
}





//_____________________________________________________________________________
void NewRooMinuit::backProp()
{
   // Transfer MINUIT fit results back into RooFit objects

   Double_t val, err, vlo, vhi, eplus, eminus, eparab, globcc;
   char buffer[10240];
   Int_t index ;
   for (index = 0; index < _nPar; index++) {
      _theFitter->GetParameter(index, buffer, val, err, vlo, vhi);
      setPdfParamVal(index, val);
      _theFitter->GetErrors(index, eplus, eminus, eparab, globcc);

      // Set the parabolic error
      setPdfParamErr(index, err);

      if (eplus > 0 || eminus < 0) {
         // Store the asymmetric error, if it is available
         setPdfParamErr(index, eminus, eplus);
      }
      else {
         // Clear the asymmetric error
         clearPdfParamAsymErr(index) ;
      }
   }
}


//_____________________________________________________________________________
void NewRooMinuit::updateFloatVec()
{
   _floatParamVec.clear() ;
   RooFIter iter = _floatParamList->fwdIterator() ;
   RooAbsArg* arg ;
   _floatParamVec.resize(_floatParamList->getSize()) ;
   Int_t i(0) ;
   while ((arg = iter.next())) {
      _floatParamVec[i++] = arg ;
   }
}



//_____________________________________________________________________________
void NewRooMinuit::applyCovarianceMatrix(TMatrixDSym& V)
{
   // Apply results of given external covariance matrix. i.e. propagate its errors
   // to all RRV parameter representations and give this matrix instead of the
   // HESSE matrix at the next save() call

   _extV = (TMatrixDSym*) V.Clone() ;

   for (Int_t i = 0 ; i < getNPar() ; i++) {
      // Skip fixed parameters
      if (_floatParamList->at(i)->isConstant()) {
         continue ;
      }
      NewRooMinuit* context = (NewRooMinuit*) NewRooMinuit::_theFitter->GetObjectFit() ;
      if (context && context->_verbose)
         cout << "setting parameter " << i << " error to " << sqrt((*_extV)(i, i)) << endl ;
      setPdfParamErr(i, sqrt((*_extV)(i, i))) ;
   }

}




void RooMinuitGlue(Int_t& /*np*/, Double_t* /*gin*/,
                   Double_t& f, Double_t* par, Int_t /*flag*/)
{
   // Static function that interfaces minuit with NewRooMinuit

   // Retrieve fit context and its components
   NewRooMinuit* context = (NewRooMinuit*) NewRooMinuit::_theFitter->GetObjectFit() ;
   ofstream* logf   = context->logfile() ;
   Double_t& maxFCN = context->maxFCN() ;
   Bool_t verbose   = context->_verbose ;

   // Set the parameter values for this iteration
   Int_t nPar = context->getNPar();
   for (Int_t index = 0; index < nPar; index++) {
      if (logf)(*logf) << par[index] << " " ;
      context->setPdfParamVal(index, par[index], verbose);
   }

   // Calculate the function for these parameters
   RooAbsReal::setHideOffset(kFALSE) ;
   f = context->_func->getVal() ;
   RooAbsReal::setHideOffset(kTRUE) ;
   context->_evalCounter++ ;
   if (RooAbsPdf::evalError() || RooAbsReal::numEvalErrors() > 0 || f > 1e30) {

      if (context->_printEvalErrors >= 0) {

         if (context->_doEvalErrorWall) {
            oocoutW(context, Minimization) << "RooMinuitGlue: Minimized function has error status." << endl
                                           << "Returning maximum FCN so far (" << maxFCN
                                           << ") to force MIGRAD to back out of this region. Error log follows" << endl ;
         }
         else {
            oocoutW(context, Minimization) << "RooMinuitGlue: Minimized function has error status but is ignored" << endl ;
         }

         TIterator* iter = context->_floatParamList->createIterator() ;
         RooRealVar* var ;
         Bool_t first(kTRUE) ;
         ooccoutW(context, Minimization) << "Parameter values: " ;
         while ((var = (RooRealVar*)iter->Next())) {
            if (first) {
               first = kFALSE ;
            }
            else ooccoutW(context, Minimization) << ", " ;
            ooccoutW(context, Minimization) << var->GetName() << "=" << var->getVal() ;
         }
         delete iter ;
         ooccoutW(context, Minimization) << endl ;

         RooAbsReal::printEvalErrors(ooccoutW(context, Minimization), context->_printEvalErrors) ;
         ooccoutW(context, Minimization) << endl ;
      }

      if (context->_doEvalErrorWall) {
         f = maxFCN + 1 ;
      }

      RooAbsPdf::clearEvalError() ;
      RooAbsReal::clearEvalErrorLog() ;
      context->_numBadNLL++ ;
   }
   else if (f > maxFCN) {
      maxFCN = f ;
   }

   // Optional logging
   if (logf)(*logf) << setprecision(15) << f << setprecision(4) << endl;
   if (verbose) {
      cout << "\nprevFCN" << (context->_func->isOffsetting() ? "-offset" : "") << " = " << setprecision(10) << f << setprecision(4) << "  " ;
      cout.flush() ;
   }
}
