#ifndef KVUNITS__H
#define KVUNITS__H

#include <Rtypes.h>

/**
\namespace KVUnits
\brief Standard units of length, mass, volume, and pressure, and their conversion factors
\ingroup Core

This is a set of numerical constants used to define and convert units
of length, mass, and pressure. The standard unit defined for each
quantity is as follows:
- standard mass unit : grammes(KVUnits::g)
- standard length unit : centimetres (KVUnits::cm)
- standard pressure unit : torr (KVUnits::torr)
- standard mass unit : grammes (KVUnits::gr)
- standard volume unit : cc/ml (KVUnits::cc)
- standard energy unit : MeV (KVUnits::MeV)
- standard time unit : fm/c (KVUnits::fmpc)
- standard density unit : nuc/fm3 (KVUnits::nucpfm3)
- standard temperature unit : MeV/kb (KVUnits::MeVpkb)

The value of the corresponding numerical constant for each of the
standard units is 1. The other available units are:
 - mass units: microgramme (KVUnits::ug), milligramme (KVUnits::mg), kilogramme (KVUnits::kg)
 - length units: micron/micrometre (KVUnits::um), millimetre (KVUnits::mm), metre (KVUnits::m)
 - pressure units: millibar (KVUnits::mbar), pascal (KVUnits::Pa), atmosphere (KVUnits::atm)

#### Unit conversion
If x is a quantity expressed in terms of one of the standard units, the
corresponding quantity in terms of a different unit is obtained by
dividing x by the appropriate numerical constant:
 - x = pressure in torr: (x/KVUnits::mbar) is pressure in millibar;
 - x = mass in grammes: (x/KVUnits::mg) is mass in milligrammes;
 - x = length in centimetres: (x/KVUnits::um) is length in microns/micrometres.

On the other hand, if x is a quantity expressed in arbitrary units,
then in order to express it in terms of standard units multiply x by
the appropriate numerical constant:
 - x = pressure in atmospheres: (x*KVUnits::atm) is pressure in torr;
 - x = mass in microgrammes: (x*KVUnits::ug) is mass in grammes;
 - x = length in millimetres: (x*KVUnits::mm) is length in centimetres.

#### Composite units
Similar conversions can be achieved by combinations of numerical
constants. For example, if x is a density expressed in kilogrammes per
cubic metre, the corresponding density in standard units (\f$g/cm^3\f$) is
~~~~{.cpp}
x*KVUnits::kg/pow(KVUnits::m, 3);
~~~~
*/

namespace KVUnits {
   // UNITS
   // Standard units are:
   //   [L]   cm
   //   [M]   g
   //   [P]   Torr
   //   [V]   cc/ml
   //   [E]   MeV
   //   [t]   fm/c
   //   [D]   nuc/fm3
   //   [T]   MeV/kb
   // lengths
   const long double cm = 1.0l;
   const long double fm = 1.e-13l;
   const long double um = 1.e-4l;
   const long double mm = 1.e-1l;
   const long double m = 1.e+2l;
   //  masses
   const long double g = 1.0l;
   const long double kg = 1.e+3l;
   const long double mg = 1.e-3l;
   const long double ug = 1.e-6l;
   const long double MeVpc2 = 1.782662e-27l;
   // pressures
   const long double torr = 1.0l;
   const long double atm = 760.l;
   const long double Pa = atm / 101325.l;
   const long double mbar = 100.l * Pa;
   // volumes
   const long double cc = 1.0l;
   const long double litre = 1.e+3l;
   const long double cl = 10.l * cc;
   const long double ml = cc;
   // energies
   const long double MeV = 1.0l;
   const long double keV = 1.e-3l;
   const long double eV = 1.e-6l;
   const long double Joule = 6.241509e+12l;
   // times
   const long double fmpc = 1.0l;
   const long double s = 2.997925e+23l;
   // densities
   const long double nucpfm3 = 1.0l;
   const long double gpcm3 = 6.022141e-16;   // 1./(KVNucleus::kAMU*KVUnits::MeVpc2/TMath::Power(KVUnits::fm,3.))
   // temperatures
   const long double MeVpkb = 1.0l;
   const long double GK = 8.617331e-02;
};

#endif
