//$Id: MODEcAxes.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  MODEcAxes
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
// Created: 2005/05/11
//
/**
 * Implementation of the MODEcAxes class.  
 *
 */
//------------------------------------------------------------------------------

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "Planet.hpp"
#include "MODEcAxes.hpp"
#include "MeanOfDateAxes.hpp"
#include "GmatConstants.hpp"
#include "RealUtilities.hpp"
#include "TimeSystemConverter.hpp"

//---------------------------------
// static data
//---------------------------------

/* placeholder - may be needed later
const wxString
MODEcAxes::PARAMETER_TEXT[MODEcAxesParamCount - MeanOfDateAxesParamCount] =
{
   "",
};

const Gmat::ParameterType
MODEcAxes::PARAMETER_TYPE[MODEcAxesParamCount - MeanOfDateAxesParamCount] =
{
};
*/

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  MODEcAxes(const wxString &itsType,
//            const wxString &itsName);
//---------------------------------------------------------------------------
/**
 * Constructs base MODEcAxes structures
 * (default constructor).
 *
 * @param <itsType> GMAT script string associated with this type of object.
 * @param <itsName> Optional name for the object.  Defaults to "".
 *
 */
//---------------------------------------------------------------------------
MODEcAxes::MODEcAxes(const wxString &itsName) :
MeanOfDateAxes(wxT("MODEc"),itsName)
{
   objectTypeNames.push_back(wxT("MODEcAxes"));
   parameterCount = MODEcAxesParamCount;
}

//---------------------------------------------------------------------------
//  MODEcAxes(const MODEcAxes &mod);
//---------------------------------------------------------------------------
/**
 * Constructs base MODEcAxes structures used in derived classes, by copying 
 * the input instance (copy constructor).
 *
 * @param mod  MODEcAxes instance to copy to create "this" instance.
 */
//---------------------------------------------------------------------------
MODEcAxes::MODEcAxes(const MODEcAxes &mod) :
MeanOfDateAxes(mod)
{
}

//---------------------------------------------------------------------------
//  MODEcAxes& operator=(const MODEcAxes &mod)
//---------------------------------------------------------------------------
/**
 * Assignment operator for MODEcAxes structures.
 *
 * @param mod The original that is being copied.
 *
 * @return Reference to this object
 */
//---------------------------------------------------------------------------
const MODEcAxes& MODEcAxes::operator=(const MODEcAxes &mod)
{
   if (&mod == this)
      return *this;
   MeanOfDateAxes::operator=(mod);   
   return *this;
}
//---------------------------------------------------------------------------
//  ~MODEcAxes(void)
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
//---------------------------------------------------------------------------
MODEcAxes::~MODEcAxes()
{
}

//---------------------------------------------------------------------------
//  bool MODEcAxes::Initialize()
//---------------------------------------------------------------------------
/**
 * Initialization method for this MODEcAxes.
 *
 */
//---------------------------------------------------------------------------
bool MODEcAxes::Initialize()
{
   MeanOfDateAxes::Initialize();
   // InitializeFK5(); // wcs - not needed just for precession
   return true;
}


//------------------------------------------------------------------------------
// public methods inherited from GmatBase
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the MODEcAxes.
 *
 * @return clone of the MODEcAxes.
 *
 */
//------------------------------------------------------------------------------
GmatBase* MODEcAxes::Clone() const
{
   return (new MODEcAxes(*this));
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
/*wxString MODEcAxes::GetParameterText(const Integer id) const
{
   if (id >= MeanOfDateAxesParamCount && id < MODEcAxesParamCount)
      return PARAMETER_TEXT[id - MeanOfDateAxesParamCount];
   return MeanOfDateAxes::GetParameterText(id);
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
/*Integer MODEcAxes::GetParameterID(const wxString &str) const
{
   for (Integer i = MeanOfDateAxesParamCount; i < MODEcAxesParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - MeanOfDateAxesParamCount])
         return i;
   }
   
   return MeanOfDateAxes::GetParameterID(str);
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
/*Gmat::ParameterType MODEcAxes::GetParameterType(const Integer id) const
{
   if (id >= MeanOfDateAxesParamCount && id < MODEcAxesParamCount)
      return PARAMETER_TYPE[id - MeanOfDateAxesParamCount];
   
   return MeanOfDateAxes::GetParameterType(id);
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
/*wxString MODEcAxes::GetParameterTypeString(const Integer id) const
{
   return MeanOfDateAxes::PARAM_TYPE_STRING[GetParameterType(id)];
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
void MODEcAxes::CalculateRotationMatrix(const A1Mjd &atEpoch,
                                        bool forceComputation)
{
   
   // convert epoch (A1 MJD) to TT MJD (for calculations)
   // 20.02.06 - arg: changed to use enum types instead of strings
//   Real mjdTT = TimeConverterUtil::Convert(atEpoch.Get(),
//                "A1Mjd", "TtMjd", GmatTimeConstants::JD_JAN_5_1941);      
   Real mjdTT = TimeConverterUtil::Convert(atEpoch.Get(),
                TimeConverterUtil::A1MJD, TimeConverterUtil::TTMJD, 
                GmatTimeConstants::JD_JAN_5_1941);      
   //Real jdTT  = mjdTT + GmatTimeConstants::JD_JAN_5_1941;
   //Real tTDB  = (jdTT - GmatTimeConstants::JD_OF_J2000) / GmatTimeConstants::DAYS_PER_JULIAN_CENTURY;
   // Compute Julian centuries of TDB from the base epoch (J2000) 
   Real offset = GmatTimeConstants::JD_JAN_5_1941 - GmatTimeConstants::JD_OF_J2000;
   Real tTDB  = (mjdTT + offset) / GmatTimeConstants::DAYS_PER_JULIAN_CENTURY;
   Real tTDB2 = tTDB * tTDB;
   Real tTDB3 = tTDB * tTDB2;
   
   // Vallado Eq. 3-52
   Real Epsbar  = (84381.448 - 46.8150*tTDB - 0.00059*tTDB2 + 0.001813*tTDB3)
      * GmatMathConstants::RAD_PER_ARCSEC;

   Real R1EpsT[3][3] = { {1.0,                      0.0,                       0.0},
                         {0.0,GmatMathUtil::Cos(Epsbar),-GmatMathUtil::Sin(Epsbar)},
                         {0.0,GmatMathUtil::Sin(Epsbar), GmatMathUtil::Cos(Epsbar)}};
   //Rmatrix33 R1Eps( 1.0,                        0.0,                       0.0,
   //                 0.0,  GmatMathUtil::Cos(Epsbar), GmatMathUtil::Sin(Epsbar),
   //                 0.0, -GmatMathUtil::Sin(Epsbar), GmatMathUtil::Cos(Epsbar));
   
   if (overrideOriginInterval) updateIntervalToUse = 
                               ((Planet*) origin)->GetNutationUpdateInterval();
   else                        updateIntervalToUse = updateInterval;
//   Rmatrix33  PREC      = ComputePrecessionMatrix(tTDB, atEpoch);
   ComputePrecessionMatrix(tTDB, atEpoch);
   
   Real PrecT[9] = {precData[0], precData[3], precData[6],
                    precData[1], precData[4], precData[7],
                    precData[2], precData[5], precData[8]};
   
   //rotMatrix = PREC.Transpose() * R1Eps.Transpose();
   Real res[3][3];
   Integer p3 = 0;
   for (Integer p = 0; p < 3; ++p)
   {
      p3 = 3*p;
      for (Integer q = 0; q < 3; ++q)
      {
         res[p][q] = PrecT[p3]   * R1EpsT[0][q] + 
                     PrecT[p3+1] * R1EpsT[1][q] + 
                     PrecT[p3+2] * R1EpsT[2][q];
      }
   }     
   
   rotMatrix.Set(res[0][0],res[0][1],res[0][2],
                 res[1][0],res[1][1],res[1][2],
                 res[2][0],res[2][1],res[2][2]); 
   // rotDotMatrix is still the default zero matrix
   // (assume it is negligibly small)
   
}
