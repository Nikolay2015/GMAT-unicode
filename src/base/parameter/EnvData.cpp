//$Id: EnvData.cpp 9811 2011-08-29 21:15:18Z wendys-dev $
//------------------------------------------------------------------------------
//                                  EnvData
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
// Author: Linda Jun
// Created: 2004/12/07
//
/**
 * Implements class which provides environmental data, such as atmospheric density,
 * solar flux, etc.
 */
//------------------------------------------------------------------------------
#include "gmatdefs.hpp"
#include "EnvData.hpp"
#include "ParameterException.hpp"
#include "CelestialBody.hpp"
#include "GmatConstants.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_ENVDATA_INIT 1
//#define DEBUG_ENVDATA_RUN 1

using namespace GmatMathUtil;

//---------------------------------
// static data
//---------------------------------

const Real EnvData::ENV_REAL_UNDEFINED = GmatRealConstants::REAL_UNDEFINED_LARGE;

const wxString
EnvData::VALID_OBJECT_TYPE_LIST[EnvDataObjectCount] =
{
   wxT("Spacecraft"),
   wxT("SolarSystem"),
   wxT("SpacePoint")
}; 

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// EnvData()
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
EnvData::EnvData()
   : RefData()
{
   mSpacecraft = NULL;
   mSolarSystem = NULL;
   mOrigin = NULL;
}


//------------------------------------------------------------------------------
// EnvData(const EnvData &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the EnvData object being copied.
 */
//------------------------------------------------------------------------------
EnvData::EnvData(const EnvData &copy)
   : RefData(copy)
{
   mSpacecraft = copy.mSpacecraft;
   mSolarSystem = copy.mSolarSystem;
   mOrigin = copy.mOrigin;
}


//------------------------------------------------------------------------------
// EnvData& operator= (const EnvData &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the object being copied.
 *
 * @return reference to this object
 */
//------------------------------------------------------------------------------
EnvData& EnvData::operator= (const EnvData &right)
{
   if (this == &right)
      return *this;
   
   RefData::operator=(right);
   
   mSpacecraft = right.mSpacecraft;
   mSolarSystem = right.mSolarSystem;
   mOrigin = right.mOrigin;
   
   return *this;
}


//------------------------------------------------------------------------------
// ~EnvData()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
EnvData::~EnvData()
{
   //MessageInterface::ShowMessage(wxT("==> EnvData::~EnvData()\n"));
}


//------------------------------------------------------------------------------
// Real GetEnvReal(const wxString &str)
//------------------------------------------------------------------------------
/**
 * Retrives atmospheric density where spacecraft is at.
 */
//------------------------------------------------------------------------------
Real EnvData::GetEnvReal(const wxString &str)
{
   //-------------------------------------------------------
   // 1. Get Spacecraft's central body (It's done in InitializeRefObjects())
   // 2. Get current time (use parameter or sc pointer?)
   // 3. Get spacecraft's position (use parameter?)
   // 4. Call GetDensity() on central body
   //-------------------------------------------------------
   
   if (mSpacecraft == NULL || mSolarSystem == NULL || mOrigin == NULL)
      InitializeRefObjects();

   if (str == wxT("AtmosDensity"))
   {
      Real a1mjd = mSpacecraft->GetRealParameter(wxT("A1Epoch"));
      Rvector6 cartState = mSpacecraft->GetState().GetState();
      
//       // Rvector6 cartState = mSpacecraft->GetStateVector(wxT("Cartesian"));
//       Rvector6 cartState = mSpacecraft->GetState(0);
//       Real state[6];
//       for (int i=0; i<6; i++)
//          state[i] = cartState[i];
      Real *state = (Real*)cartState.GetDataVector();
      Real density = 0.0;
      
      // Call GetDensity() on if origin is CelestialBody
      if (mOrigin->IsOfType(Gmat::CELESTIAL_BODY))
      {
         if (((CelestialBody*)mOrigin)->GetDensity(state, &density, a1mjd, 1))
         {
            #ifdef DEBUG_ENVDATA_RUN
            MessageInterface::ShowMessage
               (wxT("EnvData::GetEnvReal() mOrigin=%s, a1mjd=%f, state=%s, density=%g\n"),
                mOrigin->GetName().c_str(), a1mjd, cartState.ToString().c_str(), density);
            #endif
         }
         else
         {
            #ifdef DEBUG_ENVDATA_RUN
            MessageInterface::ShowMessage
               (wxT("EnvData::GetEnvReal() AtmosphereModel used for %s is NULL\n"),
                mOrigin->GetName().c_str());
            #endif
         }
      }
      
      return density;
   }
   else
   {
      throw ParameterException(wxT("EnvData::GetEnvReal() Unknown parameter name: ") +
                               str);
   }
}


//-------------------------------------
// Inherited methods from RefData
//-------------------------------------

//------------------------------------------------------------------------------
// virtual const wxString* GetValidObjectList() const
//------------------------------------------------------------------------------
const wxString* EnvData::GetValidObjectList() const
{
   return VALID_OBJECT_TYPE_LIST;
}


//------------------------------------------------------------------------------
// bool ValidateRefObjects(GmatBase *param)
//------------------------------------------------------------------------------
/**
 * Validates reference objects for given parameter
 */
//------------------------------------------------------------------------------
bool EnvData::ValidateRefObjects(GmatBase *param)
{
   int objCount = 0;
   for (int i=0; i<EnvDataObjectCount; i++)
   {
      if (HasObjectType(VALID_OBJECT_TYPE_LIST[i]))
         objCount++;
   }

   if (objCount == EnvDataObjectCount)
      return true;
   else
      return false;
}


//------------------------------------------------------------------------------
// virtual void InitializeRefObjects()
//------------------------------------------------------------------------------
void EnvData::InitializeRefObjects()
{
   #if DEBUG_ENVDATA_INIT
   MessageInterface::ShowMessage(wxT("EnvData::InitializeRefObjects() entered.\n"));
   #endif
   
   mSpacecraft = (Spacecraft*)FindFirstObject(VALID_OBJECT_TYPE_LIST[SPACECRAFT]);

   if (mSpacecraft == NULL)
      throw ParameterException
         (wxT("EnvData::InitializeRefObjects() Cannot find Spacecraft object.\n")
          wxT("Make sure Spacecraft is set to any unnamed parameters\n"));
   
   mSolarSystem = (SolarSystem*)FindFirstObject(VALID_OBJECT_TYPE_LIST[SOLAR_SYSTEM]);
   if (mSolarSystem == NULL)
      throw ParameterException
         (wxT("EnvData::InitializeRefObjects() Cannot find SolarSystem object\n"));

   // set origin
   wxString originName =
      FindFirstObjectName(GmatBase::GetObjectType(VALID_OBJECT_TYPE_LIST[SPACE_POINT]));

   if (originName != wxT(""))
   {
      #if DEBUG_ENVDATA_INIT
         MessageInterface::ShowMessage
            (wxT("EnvData::InitializeRefObjects() getting originName:%s pointer.\n"),
             originName.c_str());
      #endif
         
      mOrigin =
         (CelestialBody*)FindFirstObject(VALID_OBJECT_TYPE_LIST[SPACE_POINT]);
      
      if (!mOrigin)
         throw ParameterException
            (wxT("EnvData::InitializeRefObjects() parameter dependent body not ")
             wxT("found in the SolarSystem: ") + originName + wxT("\n"));

   }
}


//------------------------------------------------------------------------------
// virtual bool IsValidObjectType(Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Checks reference object type.
 *
 * @return return true if object is valid object, false otherwise
 */
//------------------------------------------------------------------------------
bool EnvData::IsValidObjectType(Gmat::ObjectType type)
{
   for (int i=0; i<EnvDataObjectCount; i++)
   {
      if (GmatBase::GetObjectTypeString(type) == VALID_OBJECT_TYPE_LIST[i])
         return true;
   }
   
   return false;

}

