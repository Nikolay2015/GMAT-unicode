//$Id: ModKeplerian.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                           ModKeplerian
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Arthor:  Joey Gurganus 
// Created: 2004/01/08 
//
/**
 * Definition for the Modifed Keplerian class including raduis of periapsis, 
 * raduis of apoapsis, inclination, right ascension of the ascending node, 
 * argument of periapsis, and anomaly.
 * 
 */
//------------------------------------------------------------------------------
#ifndef ModKeplerian_hpp
#define ModKeplerian_hpp

#include <iostream>
#include <sstream>
#include "gmatdefs.hpp"
#include "Rvector6.hpp"
#include "RealUtilities.hpp"
#include "CoordUtil.hpp"
#include "UtilityException.hpp"

class GMAT_API ModKeplerian
{
public:
   ModKeplerian();
   ModKeplerian(const Rvector6& state);
   ModKeplerian(const Real rp, const Real ra, const Real inc, 
                     const Real mRAAN, const Real mAOP, const Real anom);
   ModKeplerian(const ModKeplerian &modKeplerian);
   ModKeplerian& operator=(const ModKeplerian &m);
   virtual ~ModKeplerian();

   //  Friend functions
   friend std::ostream& operator<<(std::ostream& output, ModKeplerian& m);
   friend std::istream& operator>>(std::istream& input, ModKeplerian& m);

   friend Rvector6 KeplerianToModKeplerian(const Rvector6& keplerian);
   friend Rvector6 ModKeplerianToKeplerian(const Rvector6& modKeplerian);

   // public methods
   Rvector6 GetState();
   void SetState(const Rvector6& state);

   Integer GetNumData() const;
   const wxString* GetDataDescriptions() const;
   wxString* ToValueStrings();

protected:

private:
   Real     radiusOfPeriapsis;      
   Real     radiusOfApoapsis;      
   Real     inclination;      
   Real     raan;              // Right ascension of the ascending node
   Real     aop;               // Argument of Periapsis 
   Real     anomaly;           // True Anomaly as default; otherwise 
    
   static const Integer NUM_DATA = 6;
   static const wxString DATA_DESCRIPTIONS[NUM_DATA];
   wxString stringValues[NUM_DATA];

};
#endif // ModKeplerian_hpp
