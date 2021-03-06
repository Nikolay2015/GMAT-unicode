//$Id: MOEEqAxes.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  MOEEqAxes
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
// Author: Wendy C. Shoan
// Created: 2005/04/28
//
/**
 * Implementation of the MOEEqAxes class.  
 *
 */
//------------------------------------------------------------------------------

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "Planet.hpp"
#include "MOEEqAxes.hpp"
#include "InertialAxes.hpp"
#include "GmatConstants.hpp"
#include "TimeSystemConverter.hpp"


//---------------------------------
// static data
//---------------------------------

/* placeholder - may be needed later
const wxString
MOEEqAxes::PARAMETER_TEXT[MOEEqAxesParamCount - InertialAxesParamCount] =
{
   "",
};

const Gmat::ParameterType
MOEEqAxes::PARAMETER_TYPE[MOEEqAxesParamCount - InertialAxesParamCount] =
{
};
*/

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  MOEEqAxes(const wxString &itsType,
//            const wxString &itsName);
//---------------------------------------------------------------------------
/**
 * Constructs base MOEEqAxes structures
 * (default constructor).
 *
 * @param <itsType> GMAT script string associated with this type of object.
 * @param <itsName> Optional name for the object.  Defaults to "".
 *
 */
//---------------------------------------------------------------------------
MOEEqAxes::MOEEqAxes(const wxString &itsName) :
InertialAxes(wxT("MOEEq"),itsName)
{
   objectTypeNames.push_back(wxT("MOEEqAxes"));
   parameterCount = MOEEqAxesParamCount;
}

//---------------------------------------------------------------------------
//  MOEEqAxes(const MOEEqAxes &moe);
//---------------------------------------------------------------------------
/**
 * Constructs base MOEEqAxes structures used in derived classes, by copying 
 * the input instance (copy constructor).
 *
 * @param moe  MOEEqAxes instance to copy to create "this" instance.
 */
//---------------------------------------------------------------------------
MOEEqAxes::MOEEqAxes(const MOEEqAxes &moe) :
InertialAxes(moe)
{
}

//---------------------------------------------------------------------------
//  MOEEqAxes& operator=(const MOEEqAxes &moe)
//---------------------------------------------------------------------------
/**
 * Assignment operator for MOEEqAxes structures.
 *
 * @param moe The original that is being copied.
 *
 * @return Reference to this object
 */
//---------------------------------------------------------------------------
const MOEEqAxes& MOEEqAxes::operator=(const MOEEqAxes &moe)
{
   if (&moe == this)
      return *this;
   InertialAxes::operator=(moe);   
   return *this;
}
//---------------------------------------------------------------------------
//  ~MOEEqAxes(void)
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
//---------------------------------------------------------------------------
MOEEqAxes::~MOEEqAxes()
{
}

//---------------------------------------------------------------------------
//  bool MOEEqAxes::Initialize()
//---------------------------------------------------------------------------
/**
 * Initialization method for this MOEEqAxes.
 *
 */
//---------------------------------------------------------------------------
bool MOEEqAxes::Initialize()
{
   InertialAxes::Initialize();
   //InitializeFK5();  // wcs - not needed just for precession

   // convert epoch (A1 MJD) to TT MJD (for calculations)
   // 20.02.06 - arg: changed to use enum types instead of strings
//   Real mjdTT = TimeConverterUtil::Convert(epoch.Get(),
//                 "A1Mjd", "TtMjd", GmatTimeConstants::JD_JAN_5_1941);      
   Real mjdTT = TimeConverterUtil::Convert(epoch.Get(),
                 TimeConverterUtil::A1MJD, TimeConverterUtil::TTMJD, 
                 GmatTimeConstants::JD_JAN_5_1941);      
   //Real jdTT  = mjdTT + GmatTimeConstants::JD_JAN_5_1941;
   // Compute Julian centuries of TDB from the base epoch (J2000) 
   //Real tTDB  = (jdTT - GmatTimeConstants::JD_OF_J2000) / GmatTimeConstants::DAYS_PER_JULIAN_CENTURY;
   Real offset = GmatTimeConstants::JD_JAN_5_1941 - GmatTimeConstants::JD_OF_J2000;
   Real tTDB  = (mjdTT + offset) / GmatTimeConstants::DAYS_PER_JULIAN_CENTURY;
   
   if (overrideOriginInterval) updateIntervalToUse = 
                               ((Planet*) origin)->GetNutationUpdateInterval();
   else                        updateIntervalToUse = updateInterval;
//   Rmatrix33  PREC      = ComputePrecessionMatrix(tTDB, epoch);
   ComputePrecessionMatrix(tTDB, epoch);
   
   //rotMatrix = PREC.Transpose();
   //rotMatrix = PREC;
   rotMatrix.Set(precData[0], precData[3], precData[6],
                 precData[1], precData[4], precData[7],
                 precData[2], precData[5], precData[8]);

   // rotDotMatrix is still the default zero matrix
   
   return true;
}


GmatCoordinate::ParameterUsage MOEEqAxes::UsesEpoch() const
{
   return GmatCoordinate::REQUIRED;
}

//GmatCoordinate::ParameterUsage MOEEqAxes::UsesEopFile() const
//{
//   return GmatCoordinate::REQUIRED;
//}

//GmatCoordinate::ParameterUsage MOEEqAxes::UsesItrfFile() const
//{
//   return GmatCoordinate::REQUIRED;
//}



//------------------------------------------------------------------------------
// public methods inherited from GmatBase
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the MOEEqAxes.
 *
 * @return clone of the MOEEqAxes.
 *
 */
//------------------------------------------------------------------------------
GmatBase* MOEEqAxes::Clone() const
{
   return (new MOEEqAxes(*this));
}

//------------------------------------------------------------------------------
//  wxString  GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter text, given the input parameter ID.
 *
 * @param id Id for the requested parameter text.
 *
 * @return parameter text for the requested parameter.
 *
 */
//------------------------------------------------------------------------------
/*wxString MOEEqAxes::GetParameterText(const Integer id) const
{
   if (id >= InertialAxesParamCount && id < MOEEqAxesParamCount)
      return PARAMETER_TEXT[id - InertialAxesParamCount];
   return InertialAxes::GetParameterText(id);
}
*/
//------------------------------------------------------------------------------
//  Integer  GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter ID, given the input parameter string.
 *
 * @param str string for the requested parameter.
 *
 * @return ID for the requested parameter.
 *
 */
//------------------------------------------------------------------------------
/*Integer MOEEqAxes::GetParameterID(const wxString &str) const
{
   for (Integer i = InertialAxesParamCount; i < MOEEqAxesParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - InertialAxesParamCount])
         return i;
   }
   
   return InertialAxes::GetParameterID(str);
}
*/
//------------------------------------------------------------------------------
//  Gmat::ParameterType  GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type, given the input parameter ID.
 *
 * @param id ID for the requested parameter.
 *
 * @return parameter type of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
/*Gmat::ParameterType MOEEqAxes::GetParameterType(const Integer id) const
{
   if (id >= InertialAxesParamCount && id < MOEEqAxesParamCount)
      return PARAMETER_TYPE[id - InertialAxesParamCount];
   
   return InertialAxes::GetParameterType(id);
}
*/
//------------------------------------------------------------------------------
//  wxString  GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type string, given the input parameter ID.
 *
 * @param id ID for the requested parameter.
 *
 * @return parameter type string of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
/*wxString MOEEqAxes::GetParameterTypeString(const Integer id) const
{
   return InertialAxes::PARAM_TYPE_STRING[GetParameterType(id)];
}
*/


//------------------------------------------------------------------------------
// protected methods
//------------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  void CalculateRotationMatrix(const A1Mjd &atEpoch,
//                               bool forceComputation = false)
//---------------------------------------------------------------------------
/**
 * This method will compute the rotMatrix and rotDotMatrix used for rotations
 * from/to this AxisSystem to/from the MJ2000EqAxes system.
 *
 * @param atEpoch  epoch at which to compute the rotation matrix
 */
//---------------------------------------------------------------------------
void MOEEqAxes::CalculateRotationMatrix(const A1Mjd &atEpoch,
                                        bool forceComputation)
{
   // already computed in Initialize
}
