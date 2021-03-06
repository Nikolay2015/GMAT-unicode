//$Id: ItrfCoefficientsFile.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  ItrfCoefficientsFile
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under 
// MOMS Task order 124.
//
// Author: Wendy C. Shoan/GSFC/MAB
// Created: 2005/01/31
//
/**
 * Definition of the ItrfCoefficientsFile class.  This is the code that reads 
 * the nutation and planetary coefficients from the file.
 *
 * The source of the nutation and planetary coefficient data is 
 * http://www.celestrak.com/software/vallado-sw.asp 
 *
 */
//------------------------------------------------------------------------------

#ifndef ItrfCoefficientsFile_hpp
#define ItrfCoefficientsFile_hpp

#include "gmatdefs.hpp"
#include "Rvector.hpp"

namespace GmatItrf
{
   enum NutationTerms
   {
      NUTATION_1980,
      NUTATION_1996,
      NUTATION_2000
   };
   enum PlanetaryTerms
   {
      PLANETARY_1980,
      PLANETARY_1996
      // not available for 2000?
   };
};

class GMAT_API ItrfCoefficientsFile
{
public:

   // default constructor
   ItrfCoefficientsFile(const wxString &nutFileName = wxT("NUTATION.DAT"), 
               const wxString planFileName = wxT("NUT85.DAT"),
               GmatItrf::NutationTerms  nutTerms  = GmatItrf::NUTATION_1980,
               GmatItrf::PlanetaryTerms planTerms = GmatItrf::PLANETARY_1980);
   // copy constructor
   ItrfCoefficientsFile(const ItrfCoefficientsFile &itrfF);
   // operator = 
   const ItrfCoefficientsFile& operator=(const ItrfCoefficientsFile &itrfF);
   // destructor
   virtual ~ItrfCoefficientsFile();
   
   
   // initializes the ItrfCoefficientsFile
   virtual void Initialize();

   // method to return the name of the coefficients file
   virtual GmatItrf::NutationTerms  GetNutationTermsSource() const;
   virtual GmatItrf::PlanetaryTerms GetPlanetaryTermsSource() const;
   virtual wxString GetNutationFileName() const;
   virtual wxString GetPlanetaryFileName() const;
      
   virtual Integer GetNumberOfNutationTerms();
   virtual Integer GetNumberOfPlanetaryTerms();
   virtual bool GetNutationTerms(std::vector<IntegerArray> &a5, Rvector &Aval, 
                                 Rvector &Bval, Rvector &Cval, Rvector &Dval, 
                                 Rvector &Eval, Rvector &Fval);
   virtual bool GetPlanetaryTerms(std::vector<IntegerArray> &ap10, 
                                  Rvector &Apval, Rvector &Bpval, 
                                  Rvector &Cpval, Rvector &Dpval);
   
  
protected:

   // additional protected data
   // (NOTE - static const strings are initialized in source file)
   static const Integer MAX_1980_NUT_TERMS;//         = 106;
   static const Real    MULT_1980_NUT;//              = 1.0e-04;
   static const wxString FIRST_NUT_PHRASE_1980;
   
   static const Integer MAX_1996_NUT_TERMS;//         = 263;
   static const Real    MULT_1996_NUT;//              = 1.0e-06;
   static const wxString FIRST_NUT_PHRASE_1996;
   
   static const Integer MAX_2000_NUT_TERMS;//         = 106;
   static const Real    MULT_2000_NUT;//              = 1.0e-04;
   static const wxString FIRST_NUT_PHRASE_2000;
   
   static const Integer MAX_1980_PLANET_TERMS;//      = 85;
   static const Real    MULT_1980_PLANET;//           = 1.0e-04;
   static const wxString FIRST_PLAN_PHRASE_1980;
   
   static const Integer MAX_1996_PLANET_TERMS;//      = 112;
   static const Real    MULT_1996_PLANET;//           = 1.0e-04;
   static const wxString FIRST_PLAN_PHRASE_1996;
   
   static const Integer MAX_2000_PLANET_TERMS;// = 112;            // ????
   static const Real    MULT_2000_PLANET;//      = 1.0e-04;        // ????
   static const wxString FIRST_PLAN_PHRASE_2000; // ????
   
   
   /// number of terms in nutation logitude series <=MAX_NUT_TERMS
   Integer    nut;  
   /// number of terms in nutation planetary series <=MAX_PLANET_TERMS
   Integer    nutpl;
   
   /// multipliers for the nutation and planetary coefficients
   Real       nutMult;
   Real       planMult;
   
   wxString firstNutPhrase;
   wxString firstPlanPhrase;
   
   GmatItrf::NutationTerms  nutation;
   GmatItrf::PlanetaryTerms planetary;

   wxString nutationFileName;
   wxString planetaryFileName;
   bool filesAreInitialized;

   std::vector<IntegerArray> a;
   /// reduction terms
   Rvector*    A;
   Rvector*    B;
   Rvector*    C;
   Rvector*    D;
   Rvector*    E;
   Rvector*    F;
   /// planetary terms (F0-F9) multipliers
   std::vector<IntegerArray> ap;
   Rvector*    Ap;
   Rvector*    Bp;
   Rvector*    Cp;
   Rvector*    Dp;

   bool InitializeArrays(GmatItrf::NutationTerms nutT,
                         GmatItrf::PlanetaryTerms planT);
   
   bool IsBlank(const wxString& aLine);
   
};
#endif // ItrfCoefficientsFile_hpp
