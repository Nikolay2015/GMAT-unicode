//$Id: Cartesian.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                             Cartesian
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
// Author: Rodger Abel
// Created: 1995/07/18 for GSS project (originally CartesianElements)
// Modified: 2003/09/16 Linda Jun - Added new methods and member data to return
//           data descriptions and values.
/**
 * Declares Cartesian class; performs conversions from Keplerian elements to
 * Cartesian elements.
 */
//------------------------------------------------------------------------------
#ifndef Cartesian_hpp
#define Cartesian_hpp

#include <iostream>
#include "gmatdefs.hpp"
#include "Linear.hpp"
#include "Rvector3.hpp"
#include "Rvector6.hpp"

// Forward declaration
class Keplerian;       

class GMAT_API Cartesian
{
/**
 *  The position and velocity will be stored in whatever units they are 
 *  initially entered into the second constructor.  The argument, mu, in the
 *  ToCartesian method has a default value in km^3 / s^2, AND assumes that the 
 *  central body is the Earth.  If these do not correspond to the units you 
 *  are using, or if the Earth is not the central body, you must enter mu in 
 *  the correct units when calling these methods.
 *
 *  Two routines are provided to convert from Keplerian elements
 *  to Cartesian elements.  The first will use the input inclination, 
 *  right ascension to the ascending node, and argument
 *  of periapsis to recompute the P and Q vectors each time.  In
 *  the pure 2-body case, the P and Q functions can be called 
 *  initially to compute the two vectors.  These can then be
 *  assumed constant and can be passed into the second conversion function - 
 *  this can be done to improve efficiency.
 */
public:
   Cartesian();
   Cartesian(Real posX, Real posY, Real posZ,
             Real velX, Real velY, Real velZ);
   Cartesian(const Rvector3 &p, const Rvector3 &v);
   Cartesian(const Rvector6 &state); // loj: 4/19/04 added
   Cartesian(const Cartesian &Cartesian);
   Cartesian& operator=(const Cartesian &Cartesian);
   virtual ~Cartesian();

   Rvector3 GetPosition() const;
   Rvector3 GetVelocity() const;
   Real GetPosition(Integer id) const;
   Real GetVelocity(Integer id) const;

   void SetPosition(const Rvector3 &pos);
   void SetVelocity(const Rvector3 &vel);
   void SetPosition(Integer id, const Real &val);
   void SetVelocity(Integer id, const Real &val);
   void SetAll(const Real &posX, const Real &posY, const Real &posZ,
               const Real &velX, const Real &velY, const Real &velZ);

   //  Friend functions
//    friend Cartesian ToCartesian(const Keplerian &k,  const Rvector3 &pVector,
//                                 const Rvector3 &qVector, Real mu);
//    friend Cartesian ToCartesian(const Keplerian &k, Real mu);
   friend std::ostream& operator<<(std::ostream& output, Cartesian& c);
   friend std::istream& operator>>(std::istream& input, Cartesian& c);

   Integer GetNumData() const;
   const wxString* GetDataDescriptions() const;
   wxString* ToValueStrings();

   static const Cartesian ZeroCartesian;
    
   enum
   {
      X, Y, Z
   };
    
protected:

private:
   Rvector3 positionD;               //  Position vector (3 elements)
   Rvector3 velocityD;               //  Velocity vector (3 elements)

   static const Integer NUM_DATA = 6;
   static const wxString DATA_DESCRIPTIONS[NUM_DATA];
   wxString stringValues[NUM_DATA];

};
#endif // Cartesian_hpp
