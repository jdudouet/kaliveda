#include "KVDataQualityAudit.h"
#include <algorithm>

void KVDataQualityAudit::Add(const KVReconstructedNucleus& N)
{
   // Add this reconstructed nucleus to the audit.

   auto tel = telescopes.get_object<idtelescope>(N.GetIdentifyingTelescope()->GetName());
   if (!tel) {
      tel = new idtelescope(N.GetIdentifyingTelescope()->GetName());
      telescopes.Add(tel);
   }
   tel->add(N);
}

void KVDataQualityAudit::Print(Option_t* opt) const
{
   for (auto p : telescopes) {
      p->Print(opt);
   }
}

Long64_t KVDataQualityAudit::Merge(TCollection* l)
{
   // Used to merge audit objects in different ROOT files (using hadd).
   //
   // This method is called for the object in the first file, the TCollection
   // contains the objects with the same name in all the other files.

   TIter next(l);
   TObject* o;
   Long64_t nmerge = 0;
   while ((o = next())) {
      nmerge += merge(dynamic_cast<KVDataQualityAudit*>(o));
   }
   return nmerge;
}

void KVDataQualityAudit::add(const KVDataQualityAudit::idtelescope* idt)
{
   // Add copy of idtelescope data to this audit
   telescopes.Add(new idtelescope(*idt));
}

Long64_t KVDataQualityAudit::merge(KVDataQualityAudit* other)
{
   // Merge other audit object with this one. Return 1 if merge is successful,
   // 0 in case of problems.
   if (!other) return 0; // possibly called for wrong object type

   TIter next_tel(other->GetTelescopeList());
   idtelescope* idt;
   Long64_t nmerge = 0;
   while ((idt = (idtelescope*)next_tel())) {
      if (HasTelescope(idt))
         GetTelescope(idt)->merge(idt);
      else
         add(idt);
      ++nmerge;
   }
   return nmerge;
}

void KVDataQualityAudit::isotope::add(const KVReconstructedNucleus& N)
{
   // add an isotopically identified nucleus
   //
   // for calibrated particles, we store the minimum energy (=> threshold)
   A = N.GetA();
   ++counts;
   if (N.IsCalibrated() && ((emin < 0) || (N.GetEnergy() < emin))) emin = N.GetEnergy();
}

void KVDataQualityAudit::isotope::print() const
{
   std::cout << "\t\tA=" << A << "\t\t[E>" << emin << "]" << std::endl;
}

void KVDataQualityAudit::element::add(const KVReconstructedNucleus& N)
{
   // add an element (Z-identified nucleus)
   //
   // if isotopically identified, add to list of isotopes
   //
   // for calibrated particles, we store the minimum energy (=> threshold)
   Z = N.GetZ();
   ++counts;
   if (N.IsCalibrated() && ((emin < 0) || (N.GetEnergy() < emin))) emin = N.GetEnergy();
   if (N.IsAMeasured()) {
      isotopes[N.GetA()].add(N);
   }
   else
      A = N.GetA(); // default mass for Z-only identification
}

void KVDataQualityAudit::element::print() const
{
   std::cout << "\tZ=" << (int)Z << "\t\t[E>" << emin << "]";
   if (A > 0) std::cout << " default A=" << A;
   if (!HasIsotopes()) {
      std::cout << std::endl;
      return;
   }
   std::cout << "\t\tIsotopes: " << GetIsotopeList().AsString() << std::endl;
   for (auto i : isotopes) i.second.print();
}

void KVDataQualityAudit::element::merge(const KVDataQualityAudit::element& elem)
{
   // Merge the informations in element elem with those in this one:
   //
   //  - we keep the lower of the two threshold values
   //  - we sum the counts for the element
   //  - we merge the lists of isotopes of the two elements
   //  - we keep the default mass, if it has been set

   if (emin > 0) {
      if (elem.emin > 0) emin = std::min(emin, elem.emin);
   }
   else if (elem.emin > 0) emin = elem.emin;
   A = std::max(A, elem.A);
   counts += elem.counts;
   // look at isotopes in other's list. if any are not in our list they are added.
   // if any are in both, they are merged
   std::for_each(std::begin(elem.isotopes), std::end(elem.isotopes),
   [&](const std::pair<int, isotope>& isotop) {
      if (HasIsotope(isotop.first)) {
         isotopes[isotop.first].merge(isotop.second);
      }
      else {
         isotopes[isotop.first] = isotop.second;
      }
   }
                );
}

double KVDataQualityAudit::element::get_mean_isotopic_mass() const
{
   // calculate and return mean mass of isotopes measured for this element

   if (isotopes.empty()) return 0;
   double sum{0}, weight{0};
   std::for_each(std::begin(isotopes), std::end(isotopes),
   [&](const std::pair<int, isotope>& count) {
      sum += count.second.counts;
      weight += count.first * count.second.counts;
   }
                );
   if (sum > 0) return weight / sum;
   return 0;
}

int KVDataQualityAudit::element::get_max_isotopic_mass() const
{
   // return max A of isotopes measured for this element

   if (isotopes.empty()) return 0;
   auto last = isotopes.rbegin();
   return (*last).first;
}

int KVDataQualityAudit::element::get_min_isotopic_mass() const
{
   // return min A of isotopes measured for this element

   if (isotopes.empty()) return 0;
   auto first = isotopes.begin();
   return (*first).first;
}

double KVDataQualityAudit::element::get_minimum_isotopic_threshold_mev_per_nuc() const
{
   // \returns the smallest recorded threshold (in MeV/u) for isotopic identification for the element
   if (isotopes.empty()) return -1;
   double minE = 9999;
   for (auto& p : isotopes) {
      double myE = p.second.emin / p.first;
      if ((myE > 0) && (myE < minE)) minE = myE;
   }
   if (minE < 9999) return minE;
   // uncalibrated particles: all thresholds are -1 MeV
   return -1;
}

std::map<int, double> KVDataQualityAudit::element::get_isotopic_distribution() const
{
   // \returns probability distribution for each isotope as a map between isotope A and P(A)

   double sum{0};
   std::for_each(std::begin(isotopes), std::end(isotopes),
   [&](const std::pair<int, isotope>& count) {
      sum += count.second.counts;
   }
                );
   std::map<int, double> proba;
   for (auto& p : isotopes) {
      proba[p.first] = p.second.counts / sum;
   }
   return proba;
}

void KVDataQualityAudit::idtelescope::add(const KVReconstructedNucleus& N)
{
   elements[N.GetZ()].add(N);
}

void KVDataQualityAudit::idtelescope::Print(Option_t*) const
{
   std::cout << GetName() << " :" << std::endl;
   std::cout << "Elements: " << GetElementList().AsString() << std::endl;
   for (auto e : elements) e.second.print();
}

void KVDataQualityAudit::idtelescope::merge(const KVDataQualityAudit::idtelescope* idt)
{
   // Merge data in idtelescope idt with this one

   // look at elements in other's list. if any are not in our list they are added.
   // if any are in both, they are merged
   std::for_each(std::begin(idt->elements), std::end(idt->elements),
   [&](const std::pair<int, element>& elem) {
      if (HasElement(elem.first)) {
         elements[elem.first].merge(elem.second);
      }
      else {
         elements[elem.first] = elem.second;
      }
   }
                );
}

std::map<int, double> KVDataQualityAudit::idtelescope::get_element_distribution() const
{
   // \returns probability distribution for all elements identified by telescope as a map between Z and P(Z)

   double sum{0};
   std::for_each(std::begin(elements), std::end(elements),
   [&](const std::pair<int, element>& count) {
      sum += count.second.counts;
   }
                );
   std::map<int, double> proba;
   for (auto& p : elements) {
      proba[p.first] = p.second.counts / sum;
   }
   return proba;
}

double KVDataQualityAudit::idtelescope::get_mean_Z() const
{
   // \returns mean Z measured in telescope
   double sum{0}, weight{0};
   std::for_each(std::begin(elements), std::end(elements),
   [&](const std::pair<int, element>& count) {
      sum += count.second.counts;
      weight += count.second.counts * count.first;
   }
                );
   return weight / sum;
}

int KVDataQualityAudit::idtelescope::get_max_Z() const
{
   // \returns maximum Z measured in telescope
   auto last = elements.rbegin();
   return (*last).first;
}
int KVDataQualityAudit::idtelescope::get_max_Z_with_isotopes() const
{
   // \returns maximum Z for which isotopic masses were measured
   auto it = elements.rbegin();
   while (it != elements.rend()) {
      if ((*it).second.HasIsotopes()) return (*it).first;
      ++it;
   }
   return 0;
}
int KVDataQualityAudit::idtelescope::get_min_Z() const
{
   // \returns minimum Z measured in telescope
   auto first = elements.begin();
   return (*first).first;
}

double KVDataQualityAudit::idtelescope::get_mean_A() const
{
   // \returns mean A measured in telescope
   double sum{0}, weight{0};
   std::for_each(std::begin(elements), std::end(elements),
   [&](const std::pair<int, element>& count) {
      sum += count.second.counts;
      weight += count.second.counts * count.second.get_mean_isotopic_mass();
   }
                );
   return weight / sum;
}

int KVDataQualityAudit::idtelescope::get_max_A() const
{
   // \returns maximum A measured in telescope
   int max = 0;
   for (auto& e : elements) {
      max = std::max(max, e.second.get_max_isotopic_mass());
   }
   return max;
}

int KVDataQualityAudit::idtelescope::get_min_A() const
{
   // \returns smallest A measured in telescope
   int min = 999;
   for (auto& e : elements) {
      if (auto other_A = e.second.get_min_isotopic_mass())
         min = std::min(min, other_A);
   }
   return min;
}



ClassImp(KVDataQualityAudit)


