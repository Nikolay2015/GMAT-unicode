//$Id: Asteroid.cpp 5553 2008-06-03 16:46:30Z djcinsb $
//------------------------------------------------------------------------------
//                                  Asteroid
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Wendy C. Shoan
// Created: 2009.01.12
//
/**
 * Implementation of the Asteroid class.
 *
 */
//------------------------------------------------------------------------------

#include "gmatdefs.hpp"
#include "SolarSystem.hpp"
#include "SolarSystemException.hpp"
#include "CelestialBody.hpp"
#include "Asteroid.hpp"
#include "GmatConstants.hpp"
#include "MessageInterface.hpp"
#include "RealUtilities.hpp"
#include "AngleUtil.hpp"
#include "StringUtil.hpp"

//#define DEBUG_ASTEROID 1


//---------------------------------
// static data
//---------------------------------
//const wxString
//Asteroid::PARAMETER_TEXT[AsteroidParamCount - CelestialBodyParamCount] =
//{
//  
//};
//
//const Gmat::ParameterType
//Asteroid::PARAMETER_TYPE[AsteroidParamCount - CelestialBodyParamCount] =
//{
//   
//};


//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  Asteroid(wxString name)
//------------------------------------------------------------------------------
/**
* This method creates an object of the Asteroid class
 * (default constructor).
 *
 * @param <name> optional parameter indicating the name of the celestial
 *               body (default is wxT("")).
 */
//------------------------------------------------------------------------------
Asteroid::Asteroid(wxString name) :
CelestialBody     (wxT("Asteroid"),name)
{
//   CelestialBody::InitializeBody(wxT("Asteroid"));
   
   objectTypeNames.push_back(wxT("Asteroid")); 
   parameterCount = AsteroidParamCount;
   
   theCentralBodyName  = SolarSystem::SUN_NAME; 
   bodyType            = Gmat::ASTEROID;
   bodyNumber          = -1;
   referenceBodyNumber = -1;
   
//   // defaults for now ...
//   Rmatrix s(5,5,
//         0.0, 0.0,             0.0,             0.0,             0.0,
//         0.0, 0.0,             0.0,             0.0,             0.0,
//         0.0, 0.0,             0.0,             0.0,             0.0,
//         0.0, 0.0,             0.0,             0.0,             0.0,
//         0.0, 0.0,             0.0,             0.0,             0.0);
//   Rmatrix c(5,5,
//         0.0, 0.0,             0.0,             0.0,             0.0,
//         0.0, 0.0,             0.0,             0.0,             0.0,
//         0.0, 0.0,             0.0,             0.0,             0.0,
//         0.0, 0.0,             0.0,             0.0,             0.0,
//         0.0, 0.0,             0.0,             0.0,             0.0);
//   sij = s;
//   cij = c;

   
   // @todo - add other default values here

   DeterminePotentialFileNameFromStartup();
   SaveAllAsDefault();

}

//------------------------------------------------------------------------------
//  Asteroid(wxString name, const wxString &cBody)
//------------------------------------------------------------------------------
/**
* This method creates an object of the Asteroid class
 * (constructor).
 *
 * @param <name> optional parameter indicating the name of the celestial
 *               body.
 * @param <cBody> pointer to a central body.
 */
//------------------------------------------------------------------------------
Asteroid::Asteroid(wxString name, const wxString &cBody) :
CelestialBody     (wxT("Asteroid"),name)
{
//   CelestialBody::InitializeBody(wxT("Asteroid"));
   
   objectTypeNames.push_back(wxT("Asteroid"));
   parameterCount = AsteroidParamCount;

   theCentralBodyName  = cBody; 
   bodyType            = Gmat::ASTEROID;
   bodyNumber          = -1;
   referenceBodyNumber = -1;

   // @todo - add other default values here

   DeterminePotentialFileNameFromStartup();
   SaveAllAsDefault();

}

//------------------------------------------------------------------------------
//  Asteroid(const Asteroid &copy)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the Asteroid class as a copy of the
 * specified Asteroid class (copy constructor).
 *
 * @param <copy> Asteroid object to copy.
 */
//------------------------------------------------------------------------------
Asteroid::Asteroid(const Asteroid &copy) :
CelestialBody (copy)
{
}

//------------------------------------------------------------------------------
//  Asteroid& operator= (const Asteroid& copy)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the Asteroid class.
 *
 * @param <copy> the Asteroid object whose data to assign to wxT("this")
 *            solar system.
 *
 * @return wxT("this") Asteroid with data of input Asteroid copy.
 */
//------------------------------------------------------------------------------
Asteroid& Asteroid::operator=(const Asteroid &copy)
{
   if (&copy == this)
      return *this;

   CelestialBody::operator=(copy);
   return *this;
}

//------------------------------------------------------------------------------
//  ~Asteroid()
//------------------------------------------------------------------------------
/**
 * Destructor for the Asteroid class.
 */
//------------------------------------------------------------------------------
Asteroid::~Asteroid()
{
}


//------------------------------------------------------------------------------
//  GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the Asteroid.
 *
 * @return clone of the Asteroid.
 *
 */
//------------------------------------------------------------------------------
GmatBase* Asteroid::Clone() const
{
   return (new Asteroid(*this));
}


//---------------------------------------------------------------------------
//  void Copy(const GmatBase* orig)
//---------------------------------------------------------------------------
/**
 * Set this instance to match the one passed in.
 *
 * @param <orig> The object that is being copied.
 */
//---------------------------------------------------------------------------
void Asteroid::Copy(const GmatBase* orig)
{
   operator=(*((Asteroid *)(orig)));
}



//------------------------------------------------------------------------------
// protected methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// private methods
//------------------------------------------------------------------------------
// none at this time

