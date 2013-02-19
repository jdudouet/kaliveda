//Created by KVClassFactory on Wed Jul 25 09:43:37 2012
//Author: Guilain ADEMARD

#include "Riostream.h"
#include "KVVAMOSDetector.h"
#include "KVVAMOS.h"
#include "KVNamedParameter.h"
using namespace std;

ClassImp(KVVAMOSDetector)

	////////////////////////////////////////////////////////////////////////////////
	// BEGIN_HTML <!--
	/* -->
	   <h2>KVVAMOSDetector</h2>
	   <h4>Detectors of VAMOS spectrometer</h4>
	   <!-- */
	// --> END_HTML
	////////////////////////////////////////////////////////////////////////////////

KVVAMOSDetector::KVVAMOSDetector()
{
   	// Default constructor
   	init();
}
//________________________________________________________________

KVVAMOSDetector::KVVAMOSDetector(UInt_t number, const Char_t* type) : KVSpectroDetector(type){
	// create a new VAMOS detector of a given material.

   	init();
	SetNumber(number);
}
//________________________________________________________________

KVVAMOSDetector::KVVAMOSDetector (const KVVAMOSDetector& obj)  : KVSpectroDetector()
{
   	// Copy constructor
   	// This ctor is used to make a copy of an existing object (for example
   	// when a method returns an object), and it is always a good idea to
   	// implement it.
   	// If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   	init();
   	obj.Copy(*this);
}
//________________________________________________________________

KVVAMOSDetector::~KVVAMOSDetector()
{
   	// Destructor
   	SafeDelete( fT0list );
}
//________________________________________________________________

void KVVAMOSDetector::Copy (TObject& obj) const
{
   	// This method copies the current state of 'this' object into 'obj'
   	// You should add here any member variables, for example:
   	//    (supposing a member variable KVVAMOSDetector::fToto)
   	//    CastedObj.fToto = fToto;
   	// or
   	//    CastedObj.SetToto( GetToto() );

   	KVSpectroDetector::Copy(obj);
   	//KVVAMOSDetector& CastedObj = (KVVAMOSDetector&)obj;
}
//________________________________________________________________

void KVVAMOSDetector::init(){
	fT0list    = NULL;
}
//________________________________________________________________

void KVVAMOSDetector::Initialize(){
	// Initialize the data members. Called by KVVAMOS::Initialize().

}
//________________________________________________________________

void KVVAMOSDetector::SetCalibrators(){
	// Setup the calibrators for this detector. Call once name
	// has been set.
	// The calibrators are KVFunctionCal.
	// By default the all the calibration functions are first-degree
	// polynomial function and the range [Xmin,Xmax]=[0,4096].
	// Here the calibrator are not ready (KVFunctionCal::GetStatus()).
	// You have to give the parameters and change the status
	// (see KVFunctionCal::SetParameter(...) and KVFunctionCal::SetStatus(...))


	TIter nextpar(GetACQParamList());

	KVACQParam    *par   = NULL;
	Double_t       maxch = 16384.;       // 14 bits

	TString  calibtype("ERROR");

	while((par = (KVACQParam *)nextpar())){

		if( par->IsType("E") ){
 			calibtype = "channel->MeV";
		}
		else if( par->IsType("Q") ){
 			calibtype = "channel->Volt";
			maxch     = 4096.;           // 12 bits
		}
		else if( par->IsType("T") ){
 			calibtype = "channel->ns";
			if( !fT0list ) fT0list = new TList;
			fT0list->Add(new KVNamedParameter( par->GetName() , 0. ));
		}
		else continue;

		calibtype.Append(" ");
		calibtype.Append(par->GetName());

		TF1 *func        = new TF1(par->GetName(),"pol1",0.,maxch);
		KVFunctionCal *c = new KVFunctionCal(this, func);
		c->SetType(calibtype.Data());
		c->SetNumber( par->GetNumber() );
		c->SetACQParam( par );
		c->SetStatus( kFALSE );
		if(!AddCalibrator(c)) delete c;
	}

	// Define and set to zero the T0 values for time of flight measurment
	// from this detector. The time of flight acq parameters are associated
	// to gVamos
	if(gVamos){
		TIter next_vacq( gVamos->GetVACQParamList() );
		while(( par = (KVACQParam *)next_vacq() )){
			if(par->IsType("T") && IsTfromThisDetector( par->GetName()+1 ) ){
				if( !fT0list ) fT0list = new TList;
				fT0list->Add(new KVNamedParameter( par->GetName() , 0. ));
			}
		}
	}

}
//________________________________________________________________

KVFunctionCal *KVVAMOSDetector::GetECalibrator() const{
	// Returns the calibrator for the conversion channel->MeV of the energy
	// signal. 
	// The method assumes that the calibrator is an instance of the class
	// KVFunctionCal.
	
	TString calname;
	calname.Form("channel->MeV %s",GetEBaseName());
 	return (KVFunctionCal *)GetCalibrator( calname.Data() );
}
//________________________________________________________________

Bool_t KVVAMOSDetector::GetPositionInVamos(Double_t &X, Double_t &Y){
	// Return a position X and Y of the particle track in the VAMOS
	// reference frame from the fired acquisition parameters and from the 
	// the location of the TGeoVolumes which defined the detector geometry.
	// The position is randomized in the active volume.
	
	X = Y = -666;
	Warning("GetPositionInVamos","To be implemented or overwritten in daughter classes");
	return kFALSE;
}
//________________________________________________________________

KVFunctionCal *KVVAMOSDetector::GetTCalibrator(const Char_t *type) const{
	// Returns the calibrator for the conversion channel->ns of a time
	// signal of type 'type' (for example SED_HF, SI_HF, SI_SED1, ...) if 
	// the detector was used to defined this time.
	// The method assumes that the calibrator is an instance of the class
	// KVFunctionCal.
	// A time signal is always associated to an object KVVAMOS pointed by 
	// gVamos, then gVamos has to be different to zero;
	
	if(!gVamos) return NULL;
	if(!IsTfromThisDetector( type ) ) return NULL;
	TString calname;
	calname.Form("channel->ns T%s", type);
 	return (KVFunctionCal *)gVamos->GetVCalibrator( calname.Data() );
}
//________________________________________________________________

Double_t KVVAMOSDetector::GetCalibE(){
 	// Calculate energy in MeV from coder values.
   	// Returns 0 if calibration not ready or acquisition parameter not fired
   	// (we require that the acquisition parameter has a value
   	// greater than the current pedestal value)
   	KVFunctionCal *cal = GetECalibrator();
	if( cal && cal->GetStatus() && cal->GetACQParam()->Fired("P"))
		return cal->Compute();
	return 0;
}
//________________________________________________________________

Double_t KVVAMOSDetector::GetCalibT(const Char_t *type){
	// Calculate calibrated time in ns of type 'type' (SED_HF, SI_HF,
	// SI_SED1, ...) for coder values.
	// Returns 0 if calibration not ready or time ACQ parameter not fired.
	// (we require that the acquisition parameter has a value
   	// greater than the current pedestal value)


	KVFunctionCal *cal = GetTCalibrator( type );
	if( cal && cal->GetStatus() && cal->GetACQParam()->Fired("P"))
		return cal->Compute() + GetT0( type );
	return 0;
}
//________________________________________________________________

Double_t KVVAMOSDetector::GetT0(const Char_t *type) const{
	// Returns the value of the constant T0 (in ns) used for calibrating
	// time of flight of type 'type' (SED_HF, SI_HF, SI_SED1, ...).

	if( !fT0list ) return 0.;
	KVNamedParameter *par = (KVNamedParameter *)fT0list->FindObject( Form("T%s",type) );
	return ( par ? par->GetDouble() : 0 );
}
//________________________________________________________________

Bool_t KVVAMOSDetector::IsECalibrated() const{
	// Returns true if the detector has been calibrated in energy.
	
	KVCalibrator *cal = GetECalibrator();
	return ( cal && cal->GetStatus()  );
}
//________________________________________________________________

Bool_t KVVAMOSDetector::IsTCalibrated(const Char_t *type) const{
	// Returns true if the detector has been calibrated in time for type 'type'
	// (SED_HF, SI_HF, SI_SED1, ...) i.e. if
	//  -  the ACQ parameter of type 'type' has associated calibrator
	//  -  this calibrator is ready
	
	KVCalibrator *cal = GetTCalibrator( type );
	return ( cal && cal->GetStatus()  );
}
//________________________________________________________________

Bool_t KVVAMOSDetector::IsTfromThisDetector(const Char_t *type) const{
	// Returns true if the time of flight of type 'type' is measured
	// from this detector i.e. the type begins with the time base-name
	// (GetTBaseName()).
	
	TString tmp(type);
	if( tmp.BeginsWith( GetTBaseName() ) ) return kTRUE;
	return kFALSE;
}
//________________________________________________________________

void KVVAMOSDetector::SetT0(const Char_t *type, Double_t t0){
	// Set the value of the constant T0 (in ns) used for calibrating
	// time of flight of type 'type' (SED_HF, SI_HF, SI_SED1, ...).

	KVNamedParameter *par = (KVNamedParameter *)fT0list->FindObject( Form("T%s",type) );
	if( par ) par->Set( t0 );
	else Error("SetT0","Impossible to set T0 for unknown time ACQ parameter %s",type);
}
//________________________________________________________________

const Char_t *KVVAMOSDetector::GetEBaseName() const{
	// Base name of the energy used to be compatible
	// GANIL acquisition parameters
	//
	// The base name is "E<type><number>".
	
	return Form("E%s%d",GetType(),GetNumber());
}
//________________________________________________________________

const Char_t *KVVAMOSDetector::GetTBaseName() const{
	// Base name of the time of flight parameters used to be compatible
	// GANIL acquisition parameters
	//
	// The root of the name is the detector type.
	
	return GetType();
}
//________________________________________________________________

Double_t KVVAMOSDetector::GetCalibT_HF(){
	// Returns calibrated time of flight in ns,  from coder value of 
	// the time measurment between the beam pulse and timing
	// from this detector.
	// Returns 0 if calibration not ready or ACQ parameter not fired.

	return GetCalibT( Form("%s_HF", GetTBaseName()) );
}
//________________________________________________________________

Float_t KVVAMOSDetector::GetT_HF(){
	// Access acquisition data value associated to the time of flight 
	// recorded between the beam pulse and timing from this detector.
	// Returns value as a floating-point number which is the raw channel number
	// read from the coder plus a random number in the range [-0.5,+0.5].
   	// If the detector has no ACQ parameter for time of flight,
	// or if the raw channel number = 0, the value returned is -1

	return GetT( Form("%s_HF", GetTBaseName()) );
}
//________________________________________________________________

Double_t KVVAMOSDetector::GetT0_HF() const{
	// Retruns value of the constant T0 (in ns) used for calibrating
	// time of flight recorded between the beam pulse and timing
	// from this detector.

	return GetT0( Form("%s_HF", GetTBaseName()) );
}
//________________________________________________________________

Bool_t KVVAMOSDetector::IsTHFCalibrated() const{

return IsTCalibrated( Form("%s_HF", GetTBaseName()) );
}
//________________________________________________________________

void KVVAMOSDetector::SetT0_HF( Double_t t0){
	// Set value of the constant T0 (in ns) used for calibrating
	// time of flight recorded between the beam pulse and timing
	// from this detector.

	SetT0( Form("%s_HF", GetTBaseName()), t0 );
}
