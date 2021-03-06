//$Id: BodyFixedAxes.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  BodyFixedAxes
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
// Created: 2005/01/25
//
/**
 * Implementation of the BodyFixedAxes class.  
 *
 */
//------------------------------------------------------------------------------

#include <iostream>
#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "BodyFixedAxes.hpp"
#include "DynamicAxes.hpp"
#include "SolarSystem.hpp"
#include "CelestialBody.hpp"
#include "DeFile.hpp"
#include "Planet.hpp"
#include "RealUtilities.hpp"
#include "Linear.hpp"
#include "GmatConstants.hpp"
#include "Rvector3.hpp"
#include "TimeSystemConverter.hpp"
#include "CoordinateSystemException.hpp"
#include "MessageInterface.hpp"
#include "Attitude.hpp"

using namespace GmatMathUtil;      // for trig functions, etc.
using namespace GmatTimeConstants;      // for JD offsets, etc.

//#define DEBUG_FIRST_CALL
//#define DEBUG_TIME_CALC
//#define DEBUG_MOON_MATRIX
//#define DEBUG_BF_MATRICES
//#define DEBUG_BF_CHECK_DETERMINANT
//#define DEBUG_BF_ROT_MATRIX
//#define DEBUG_BF_RECOMPUTE
//#define DEBUG_BF_EPOCHS

#ifdef DEBUG_FIRST_CALL
   static bool firstCallFired = false;
#endif


//---------------------------------
// static data
//---------------------------------

//const wxString
//BodyFixedAxes::PARAMETER_TEXT[BodyFixedAxesParamCount - DynamicAxesParamCount] =
//{
//};
//
//const Gmat::ParameterType
//BodyFixedAxes::PARAMETER_TYPE[BodyFixedAxesParamCount - DynamicAxesParamCount] =
//{
//};

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  BodyFixedAxes(const wxString &itsName);
//------------------------------------------------------------------------------
/**
 * Constructs base BodyFixedAxes structures
 * (default constructor).
 *
 * @param itsName Optional name for the object.  Defaults to "".
 */
//------------------------------------------------------------------------------
BodyFixedAxes::BodyFixedAxes(const wxString &itsName) :
DynamicAxes(wxT("BodyFixed"),itsName),
de                       (NULL),
prevEpoch                (0.0),
prevUpdateInterval       (-99.9),
prevOriginUpdateInterval (-99.9),
prevLunaSrc              (Gmat::RotationDataSrcCount)
{
   objectTypeNames.push_back(wxT("BodyFixedAxes"));
   parameterCount = BodyFixedAxesParamCount;
}


//------------------------------------------------------------------------------
//  BodyFixedAxes(const BodyFixedAxes &bfAxes);
//------------------------------------------------------------------------------
/**
 * Constructs base BodyFixedAxes structures used in derived classes, by copying 
 * the input instance (copy constructor).
 *
 * @param bfAxes  BodyFixedAxes instance to copy to create "this" instance.
 */
//------------------------------------------------------------------------------
BodyFixedAxes::BodyFixedAxes(const BodyFixedAxes &bfAxes) :
DynamicAxes(bfAxes),
de                       (NULL),
prevEpoch                (bfAxes.prevEpoch),
prevUpdateInterval       (bfAxes.prevUpdateInterval),
prevOriginUpdateInterval (bfAxes.prevOriginUpdateInterval),
prevLunaSrc              (bfAxes.prevLunaSrc)
{
   #ifdef DEBUG_BF_RECOMPUTE
      MessageInterface::ShowMessage(wxT("Constructing a new BFA (%p) from the old one (%p)\n"),
            this, &bfAxes);
      MessageInterface::ShowMessage(wxT("   and prevEpoch(old) %12.10f copied to prevEpoch(new) %12.10f\n"),
            bfAxes.prevEpoch, prevEpoch);
   #endif
}

//------------------------------------------------------------------------------
//  BodyFixedAxes& operator=(const BodyFixedAxes &bfAxes)
//------------------------------------------------------------------------------
/**
 * Assignment operator for BodyFixedAxes structures.
 *
 * @param bfAxes The original that is being copied.
 *
 * @return Reference to this object
 */
//------------------------------------------------------------------------------
const BodyFixedAxes& BodyFixedAxes::operator=(const BodyFixedAxes &bfAxes)
{
   if (&bfAxes == this)
      return *this;
   DynamicAxes::operator=(bfAxes); 
   de                       = bfAxes.de;
   prevEpoch                = bfAxes.prevEpoch;
   prevUpdateInterval       = bfAxes.prevUpdateInterval;
   prevOriginUpdateInterval = bfAxes.prevOriginUpdateInterval;
   prevLunaSrc              = bfAxes.prevLunaSrc;

   return *this;
}


//------------------------------------------------------------------------------
//  ~BodyFixedAxes(void)
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
BodyFixedAxes::~BodyFixedAxes()
{
}

GmatCoordinate::ParameterUsage BodyFixedAxes::UsesEopFile() const
{
   return GmatCoordinate::REQUIRED;
}

GmatCoordinate::ParameterUsage BodyFixedAxes::UsesItrfFile() const
{
   return GmatCoordinate::REQUIRED;
}

GmatCoordinate::ParameterUsage BodyFixedAxes::UsesNutationUpdateInterval() const
{
   if (originName == SolarSystem::EARTH_NAME) 
      return GmatCoordinate::REQUIRED;
   return DynamicAxes::UsesNutationUpdateInterval();
}


//------------------------------------------------------------------------------
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * Initialization method for this BodyFixedAxes.
 *
 */
//------------------------------------------------------------------------------
bool BodyFixedAxes::Initialize()
{
   DynamicAxes::Initialize();
   if (originName == SolarSystem::EARTH_NAME) InitializeFK5();
   
   #ifdef DEBUG_FIRST_CALL
      firstCallFired = false;
   #endif
   

   return true;
}


//------------------------------------------------------------------------------
// public methods inherited from GmatBase
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the Planet.
 *
 * @return clone of the Planet.
 *
 */
//------------------------------------------------------------------------------
GmatBase* BodyFixedAxes::Clone() const
{
   return (new BodyFixedAxes(*this));
}

//------------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                    const wxString &name)
//------------------------------------------------------------------------------
/**
 * This method sets a reference object for the BodyFixedAxes class.  This is
 * overridden from the CoordinateBase version, in order to make sure the origin
 * is a CelestialBody
 *
 * @param obj   pointer to the reference object
 * @param type  type of the reference object
 * @param name  name of the reference object
 *
 * @return true if successful; otherwise, false.
 *
 */
//------------------------------------------------------------------------------
bool BodyFixedAxes::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                 const wxString &name)
{
   if (obj == NULL)
      return false;

   #ifdef DEBUG_SET_REF
   MessageInterface::ShowMessage
      (wxT("BodyFixedAxes::SetRefObject() <%s>, obj=%p, name=%s\n"), GetName().c_str(),
       obj, name.c_str());
   #endif
   if (name == originName)
   {
      if (!obj->IsOfType(wxT("CelestialBody")))
      {
         CoordinateSystemException cse(wxT(""));
         cse.SetDetails(errorMessageFormatUnnamed.c_str(),
                        (obj->GetName()).c_str(),
                       wxT("Origin"), wxT("Celestial Body"));
         throw cse;
      }
   }
   return DynamicAxes::SetRefObject(obj, type, name);
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
//wxString BodyFixedAxes::GetParameterText(const Integer id) const
//{
//   if (id >= DynamicAxesParamCount && id < BodyFixedAxesParamCount)
//      return PARAMETER_TEXT[id - DynamicAxesParamCount];
//   return DynamicAxes::GetParameterText(id);
//}

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
//Integer BodyFixedAxes::GetParameterID(const wxString &str) const
//{
//   for (Integer i = DynamicAxesParamCount; i < BodyFixedAxesParamCount; i++)
//   {
//      if (str == PARAMETER_TEXT[i - DynamicAxesParamCount])
//         return i;
//   }
//   
//   return DynamicAxes::GetParameterID(str);
//}

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
//Gmat::ParameterType BodyFixedAxes::GetParameterType(const Integer id) const
//{
//   if (id >= DynamicAxesParamCount && id < BodyFixedAxesParamCount)
//      return PARAMETER_TYPE[id - DynamicAxesParamCount];
//   
//   return DynamicAxes::GetParameterType(id);
//}

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
//wxString BodyFixedAxes::GetParameterTypeString(const Integer id) const
//{
//   return DynamicAxes::PARAM_TYPE_STRING[GetParameterType(id)];
//}

//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
* This method returns the string parameter value, given the input
 * parameter ID.
 *
 * @param id  ID for the requested parameter.
 *
 * @return  string value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
//wxString BodyFixedAxes::GetStringParameter(const Integer id) const
//{
//   return DynamicAxes::GetStringParameter(id);
//}

//------------------------------------------------------------------------------
//  wxString  SetStringParameter(const Integer id, const wxString value)
//------------------------------------------------------------------------------
/**
* This method sets the string parameter value, given the input
 * parameter ID.
 *
 * @param id     ID for the requested parameter.
 * @param value  string value for the requested parameter.
 *
 * @return  success flag.
 *
 */
//------------------------------------------------------------------------------
//bool BodyFixedAxes::SetStringParameter(const Integer id, 
//                                       const wxString &value)
//{
//   return DynamicAxes::SetStringParameter(id, value);
//}

//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
* Accessor method used to get a parameter value
 *
 * @param    label  label ID for the parameter
 *
 * @return the value of the parameter
 */
//------------------------------------------------------------------------------
//wxString BodyFixedAxes::GetStringParameter(const wxString &label) const
//{
//   return GetStringParameter(GetParameterID(label));
//}

//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
/**
* Accessor method used to get a parameter value
 *
 * @param    label  Integer ID for the parameter
 * @param    value  The new value for the parameter
 */
//------------------------------------------------------------------------------
//bool BodyFixedAxes::SetStringParameter(const wxString &label,
//                                        const wxString &value)
//{
//   return SetStringParameter(GetParameterID(label), value);
//}


//------------------------------------------------------------------------------
// protected methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  void CalculateRotationMatrix(const A1Mjd &atEpoch, 
//                               bool forceComputation = false)
//------------------------------------------------------------------------------
/**
 * This method will compute the rotMatrix and rotDotMatrix used for rotations
 * from/to this AxisSystem to/from the MJ2000Eq system
 *
 * @param atEpoch  epoch at which to compute the rotation matrix
 *
 * @note Code (for Earth) adapted from C code written by Steve Queen/ GSFC, 
 *       based on Vallado, pgs. 211- 227.  Equation references in parentheses
 *       will refer to those of the Vallado book.
 */
//------------------------------------------------------------------------------
void BodyFixedAxes::CalculateRotationMatrix(const A1Mjd &atEpoch,
                                            bool forceComputation) 
{
   #ifdef DEBUG_FIRST_CALL
      if (!firstCallFired)
         MessageInterface::ShowMessage(
            wxT("Calling CalculateRotationMatrix at epoch %lf; "), atEpoch.Get());
   #endif
   Real theEpoch = atEpoch.Get();
   #ifdef DEBUG_BF_EPOCHS
      MessageInterface::ShowMessage(wxT("CalculateRotationMatrix(%s)   epoch = %12.10f, prevEpoch = %12.10f ...... "),
            (coordName.c_str()), theEpoch, prevEpoch);
   #endif
   #ifdef DEBUG_BF_RECOMPUTE
      MessageInterface::ShowMessage(wxT("Entering CalculateRotationMatrix on object %s (%p) of type %s, origin = %s\n"),
            (coordName.c_str()), this, (GetTypeName()).c_str(), originName.c_str());
      MessageInterface::ShowMessage(wxT("     epoch = %12.10f, prevEpoch = %12.10f\n"), theEpoch, prevEpoch);
//      if (originName == SolarSystem::MOON_NAME)
//         MessageInterface::ShowMessage("   Luna source = %d, prevLunaSrc = %d\n",
//               (Integer) ((CelestialBody*)origin)->GetRotationDataSource(), (Integer) prevLunaSrc);
//      if (originName == SolarSystem::EARTH_NAME)
//      {
//         MessageInterface::ShowMessage("   Earth interval = %12.10f, prevOriginUpdateInterval = %12.10f\n",
//               ((Planet*) origin)->GetNutationUpdateInterval(), prevOriginUpdateInterval);
//         MessageInterface::ShowMessage("    interval = %12.10f, prevUpdateInterval = %12.10f\n",
//               updateInterval, prevUpdateInterval);
//      }
   #endif

   // Code must check to see if we need to recompute.  Recomputation is only necessary
   // if one or more of the following conditions are true:
   // 1. the epoch is different (within machine precision) from the epoch at the last computation
   // 2. if the origin is the Earth, the nutation update interval has changed
   // 3. if the origin is Luna, the rotation data source has changed

   if ((!forceComputation)                    &&
       (originName == SolarSystem::MOON_NAME) &&
       (IsEqual(theEpoch,      prevEpoch))    &&
       (prevLunaSrc == ((CelestialBody*)origin)->GetRotationDataSource()))
      {
         #ifdef DEBUG_BF_RECOMPUTE
            MessageInterface::ShowMessage(wxT("Don't need to recompute for Luna at this time!!\n"));
         #endif
         return;
      }
   

   // compute rotMatrix and rotDotMatrix
   if (originName == SolarSystem::EARTH_NAME)
   {
      Real intervalFromOrigin = ((Planet*) origin)->GetNutationUpdateInterval();
      if ((!forceComputation)                                     &&
          (IsEqual(theEpoch,           prevEpoch)                 &&
          (IsEqual(intervalFromOrigin, prevOriginUpdateInterval)) &&
          (IsEqual(updateInterval,     prevUpdateInterval))))
      {
         #ifdef DEBUG_BF_EPOCHS
            MessageInterface::ShowMessage(wxT("NOT recomputing\n"));
         #endif
         #ifdef DEBUG_BF_RECOMPUTE
            MessageInterface::ShowMessage(wxT("Don't need to recompute for Earth at this time!!\n"));
         #endif
         return;
      }

      #ifdef DEBUG_BF_EPOCHS
         MessageInterface::ShowMessage("RECOMPUTING!!!\n");
      #endif
      #ifdef DEBUG_BF_RECOMPUTE
         MessageInterface::ShowMessage(wxT("   RECOMPUTING!!! - body name is EARTH\n"));
      #endif
      #ifdef DEBUG_FIRST_CALL
         if (!firstCallFired)
            MessageInterface::ShowMessage(wxT("In BFA, Body is the Earth\n"));
      #endif
      Real dPsi             = 0.0;
      Real longAscNodeLunar = 0.0;
      Real cosEpsbar        = 0.0;
      Real cosAst           = 0.0;
      Real sinAst           = 0.0;
      //Real x                = 0.0;
      //Real y                = 0.0;
      // Convert to MJD UTC to use for polar motion  and LOD 
      // interpolations
      
      // 20.02.06 - arg: changed to use enum types instead of strings
//      Real mjdUTC = TimeConverterUtil::Convert(atEpoch.Get(),
//                    "A1Mjd", "UtcMjd", JD_JAN_5_1941);

      Real mjdUTC = TimeConverterUtil::Convert(theEpoch,
                    TimeConverterUtil::A1MJD, TimeConverterUtil::UTCMJD, 
                    JD_JAN_5_1941);
      Real offset = JD_JAN_5_1941 - JD_NOV_17_1858;
      // convert to MJD referenced from time used in EOP file
      mjdUTC = mjdUTC + offset;


      // convert input time to UT1 for later use (for AST calculation)
      // 20.02.06 - arg: changed to use enum types instead of strings
//      Real mjdUT1 = TimeConverterUtil::Convert(atEpoch.Get(),
//                    "A1Mjd", "Ut1Mjd", JD_JAN_5_1941);

      Real mjdUT1 = TimeConverterUtil::Convert(theEpoch,
                    TimeConverterUtil::A1MJD, TimeConverterUtil::UT1, 
                    JD_JAN_5_1941);
      
      // Compute elapsed Julian centuries (UT1)
      //Real tUT1     = (jdUT1 - JD_OF_J2000) / DAYS_PER_JULIAN_CENTURY;
      Real tDiff = JD_JAN_5_1941 - JD_OF_J2000;
      Real tUT1 = (mjdUT1 + tDiff) / DAYS_PER_JULIAN_CENTURY;
       
      // convert input A1 MJD to TT MJD (for most calculations)
      // 20.02.06 - arg: changed to use enum types instead of strings
//      Real mjdTT = TimeConverterUtil::Convert(atEpoch.Get(),
//                   "A1Mjd", "TtMjd", JD_JAN_5_1941);      
      Real mjdTT = TimeConverterUtil::Convert(theEpoch,
                   TimeConverterUtil::A1MJD, TimeConverterUtil::TTMJD, 
                   JD_JAN_5_1941);      
      Real jdTT    = mjdTT + JD_JAN_5_1941; // right? 
      // Compute Julian centuries of TDB from the base epoch (J2000) 
      // NOTE - this is really TT, an approximation of TDB *********
      //Real tTDB    = (jdTT - JD_OF_J2000) / DAYS_PER_JULIAN_CENTURY;
      Real tTDB    = (mjdTT + tDiff) / DAYS_PER_JULIAN_CENTURY;
 
      #ifdef DEBUG_FIRST_CALL
         if (!firstCallFired)
         {
            Real jdUT1    = mjdUT1 + JD_JAN_5_1941; // right?
            MessageInterface::ShowMessage(
               wxT("   Epoch data[mjdUTC, mjdUT1, jdUT1, tUT1, mjdTT1, jdTT, tTDB] ")
               wxT("=\n        [%.10lf %.10lf %.10lf %.10lf %.10lf %.10lf %.10lf ]\n"),
               mjdUTC, mjdUT1, jdUT1, tUT1, mjdTT, jdTT, tTDB);
         }
      #endif

      #ifdef DEBUG_BF_ROT_MATRIX
         MessageInterface::ShowMessage(wxT("   about to figure out update interval ...\n"));
      #endif
      if (overrideOriginInterval) 
      {
         updateIntervalToUse = updateInterval;
         //MessageInterface::ShowMessage("Overriding origin interval .....\n");
         #ifdef DEBUG_FIRST_CALL
            if (!firstCallFired)
               MessageInterface::ShowMessage(
                  wxT("   Overrode origin interval; set to %.12lf\n"), 
                  updateIntervalToUse);
         #endif
      }
      else 
      {
         updateIntervalToUse = intervalFromOrigin;
         //MessageInterface::ShowMessage("Using origin interval .....\n");
         #ifdef DEBUG_FIRST_CALL
            if (!firstCallFired)
               MessageInterface::ShowMessage(
                  wxT("   Using body's origin interval, %.12lf\n"), 
                  updateIntervalToUse);
         #endif
      }
      
      //MessageInterface::ShowMessage("Setting %4.3f as interval \n",
      //                              updateIntervalToUse);
      #ifdef DEBUG_BF_ROT_MATRIX
         MessageInterface::ShowMessage(wxT("About to call ComputePrecessionMatrix\n"));
      #endif
      ComputePrecessionMatrix(tTDB, atEpoch);
      #ifdef DEBUG_BF_ROT_MATRIX
         MessageInterface::ShowMessage(wxT("About to call ComputeNutationMatrix\n"));
      #endif
      ComputeNutationMatrix(tTDB, atEpoch, dPsi, longAscNodeLunar, cosEpsbar,
                            forceComputation);
      #ifdef DEBUG_BF_ROT_MATRIX
         MessageInterface::ShowMessage(wxT("About to call ComputeSiderealTimeRotation\n"));
      #endif
      ComputeSiderealTimeRotation(jdTT, tUT1, dPsi, longAscNodeLunar, cosEpsbar,
                             cosAst, sinAst);
      #ifdef DEBUG_BF_ROT_MATRIX
         MessageInterface::ShowMessage(wxT("About to call ComputeSiderealTimeDotRotation\n"));
      #endif
      ComputeSiderealTimeDotRotation(mjdUTC, atEpoch, cosAst, sinAst,
                                     forceComputation);
      #ifdef DEBUG_BF_ROT_MATRIX
         MessageInterface::ShowMessage(wxT("About to call ComputePolarMotionRotation\n"));
      #endif
      ComputePolarMotionRotation(mjdUTC, atEpoch, forceComputation);
      #ifdef DEBUG_BF_ROT_MATRIX
         MessageInterface::ShowMessage(wxT("DONE calling all computation submethods\n"));
      #endif
      
   #ifdef DEBUG_BF_MATRICES
      MessageInterface::ShowMessage(wxT("atEpoch = %12.10f\n"), atEpoch.Get());
      MessageInterface::ShowMessage(wxT("NUT = %s\n"), NUT.ToString().c_str());
      MessageInterface::ShowMessage(wxT("STderiv = %s\n"), STderiv.ToString().c_str());
      MessageInterface::ShowMessage(wxT("PM = %s\n"), PM.ToString().c_str());
   #endif

      Real np[3][3], rot[3][3], tmp[3][3];
      Integer p3;
      
      // NUT * PREC
      for (Integer p = 0; p < 3; ++p)
      {
         p3 = 3*p;
         for (Integer q = 0; q < 3; ++q)
         {
            np[p][q] = nutData[p3]   * precData[q]   + 
                       nutData[p3+1] * precData[q+3] + 
                       nutData[p3+2] * precData[q+6];
         }
      }     
      
      // ST * (NUT * PREC)
      for (Integer p = 0; p < 3; ++p)
      {
         p3 = 3*p;
         for (Integer q = 0; q < 3; ++q)
         {
            tmp[p][q] = stData[p3]   * np[0][q] + 
                        stData[p3+1] * np[1][q] + 
                        stData[p3+2] * np[2][q];
         }
      }     
      
      // PM * (ST * (NUT * PREC))
      for (Integer p = 0; p < 3; ++p)
      {
         p3 = 3*p;
         for (Integer q = 0; q < 3; ++q)
         {
            rot[p][q] = pmData[p3]   * tmp[0][q] + 
                        pmData[p3+1] * tmp[1][q] + 
                        pmData[p3+2] * tmp[2][q];
         }
      }

      rotMatrix.Set(rot[0][0], rot[1][0], rot[2][0],
                    rot[0][1], rot[1][1], rot[2][1],
                    rot[0][2], rot[1][2], rot[2][2]);

      #ifdef DEBUG_BF_CHECK_DETERMINANT
         Real determinant =
                 rot[0][0] * (rot[1][1] * rot[2][2] - rot[1][2]*rot[2][1]) +
                 rot[0][1] * (rot[1][2] * rot[2][0] - rot[1][0]*rot[2][2]) +
                 rot[0][2] * (rot[1][0] * rot[2][1] - rot[1][1]*rot[2][0]);
         if (Abs(determinant - 1.0) > DETERMINANT_TOLERANCE)
            throw CoordinateSystemException(
                  wxT("Computed rotation matrix has a determinant not equal to 1.0"));
      #endif

      // NUT * PREC calculated above
      // STderiv * (NUT * PREC)
      for (Integer p = 0; p < 3; ++p)
      {
         p3 = 3*p;
         for (Integer q = 0; q < 3; ++q)
         {
            tmp[p][q] = stDerivData[p3]   * np[0][q] + 
                        stDerivData[p3+1] * np[1][q] + 
                        stDerivData[p3+2] * np[2][q];
         }
      }     
      
      // PM * (STderiv * (NUT * PREC))
      for (Integer p = 0; p < 3; ++p)
      {
         p3 = 3*p;
         for (Integer q = 0; q < 3; ++q)
         {
            rot[p][q] = pmData[p3]   * tmp[0][q] + 
                        pmData[p3+1] * tmp[1][q] + 
                        pmData[p3+2] * tmp[2][q];
         }
      }
      rotDotMatrix.Set(rot[0][0], rot[1][0], rot[2][0],
                       rot[0][1], rot[1][1], rot[2][1],
                       rot[0][2], rot[1][2], rot[2][2]);

      // save the data to compare against next time
      prevUpdateInterval       = updateInterval;
      prevOriginUpdateInterval = intervalFromOrigin;

      #ifdef DEBUG_FIRST_CALL
         if (!firstCallFired) 
         {
            MessageInterface::ShowMessage(wxT("FK5 Components\n   PREC = \n"));
            for (Integer m = 0; m < 3; ++m)
               MessageInterface::ShowMessage(
                  wxT("          %20.12le %20.12le %20.12le \n"), PREC(m,0), 
                  PREC(m,1), PREC(m,2));
            MessageInterface::ShowMessage(wxT("\n   NUT = \n"));
            for (Integer m = 0; m < 3; ++m)
               MessageInterface::ShowMessage(
                  wxT("          %20.12le %20.12le %20.12le \n"), NUT(m,0), 
                  NUT(m,1), NUT(m,2));
            MessageInterface::ShowMessage(wxT("\n   ST = \n"));
            for (Integer m = 0; m < 3; ++m)
               MessageInterface::ShowMessage(
                  wxT("          %20.12le %20.12le %20.12le \n"), ST(m,0), 
                  ST(m,1), ST(m,2));
            MessageInterface::ShowMessage(wxT("\n   STderiv = \n"));
            for (Integer m = 0; m < 3; ++m)
               MessageInterface::ShowMessage(
                  wxT("          %20.12le %20.12le %20.12le \n"), STderiv(m,0), 
                  STderiv(m,1), STderiv(m,2));
            MessageInterface::ShowMessage(wxT("\n   PM = \n"));
            for (Integer m = 0; m < 3; ++m)
               MessageInterface::ShowMessage(
                  wxT("          %20.12le %20.12le %20.12le \n"), PM(m,0), 
                  PM(m,1), PM(m,2));
            MessageInterface::ShowMessage(wxT("\n   rotMatrix = \n"));
            for (Integer m = 0; m < 3; ++m)
               MessageInterface::ShowMessage(
                  wxT("          %20.12le %20.12le %20.12le \n"), rotMatrix(m,0), 
                  rotMatrix(m,1), rotMatrix(m,2));
         }
      #endif

   }
   else if ((originName == SolarSystem::MOON_NAME) &&
           (((CelestialBody*)origin)->GetRotationDataSource() == Gmat::DE_405_FILE))
   {
      #ifdef DEBUG_BF_RECOMPUTE
         MessageInterface::ShowMessage(wxT("   RECOMPUTING!!! - body name is LUNA\n"));
      #endif
      if (!de)
      {
         de = (DeFile*) ((CelestialBody*)origin)->GetSourceFile();
         if (!de)
         throw CoordinateSystemException(
               wxT("No DE file specified - cannot get Moon data"));
         // De file is initialized in its constructor
      }
      bool override = ((CelestialBody*)origin)->GetOverrideTimeSystem();
      Real librationAngles[3], andRates[3];
      de->GetAnglesAndRates(atEpoch, librationAngles, andRates, override);
      // convert from rad/day to rad/second
      //andRates[0] /= GmatTimeConstants::SECS_PER_DAY;
      //andRates[1] /= GmatTimeConstants::SECS_PER_DAY;
      //andRates[2] /= GmatTimeConstants::SECS_PER_DAY;
      rotMatrix    = (Attitude::ToCosineMatrix(librationAngles, 3, 1, 3)).Transpose();
      Real ca1    = cos(librationAngles[0]);
      Real ca2    = cos(librationAngles[1]);
      Real ca3    = cos(librationAngles[2]);
      Real sa1    = sin(librationAngles[0]);
      Real sa2    = sin(librationAngles[1]);
      Real sa3    = sin(librationAngles[2]);
      Real s1c2   = sa1*ca2;
      Real s1c3   = sa1*ca3;
      Real s2c3   = sa2*ca3; 
      Real s3c1   = sa3*ca1;  
      Real s3c2   = sa3*ca2;  
      Real s1s2   = sa1*sa2;  
      Real s1s3   = sa1*sa3;
      Real c1c2   = ca1*ca2;  
      Real c1c3   = ca1*ca3;  
      Real c2c3   = ca2*ca3;  

      Real s3c2s1 = s3c2*sa1;
      Real c1c2c3 = ca1*c2c3;
      Real s3c1c2 = s3c1*ca2;
      Real s1c2c3 = c2c3*sa1;
      
      rotDotMatrix.Set(
         -andRates[2]*(s3c1+s1c2c3) + andRates[1]*sa3*s1s2  - andRates[0]*(s1c3+s3c1c2),
         -andRates[2]*(c1c3-s3c2s1) + andRates[1]*ca3*s1s2  + andRates[0]*(s1s3-c1c2c3),
                                      andRates[1]*s1c2      + andRates[0]*sa2*ca1,
         -andRates[2]*(s1s3-c1c2c3) - andRates[1]*s3c1*sa2  + andRates[0]*(c1c3-s3c2s1),
         -andRates[2]*(s1c3+s3c1c2) - andRates[1]*s2c3*ca1  - andRates[0]*(s3c1+s1c2c3),
                                    - andRates[1]*c1c2      + andRates[0]*s1s2,
           andRates[2]*s2c3          + andRates[1]*s3c2,
          -andRates[2]*sa2*sa3       + andRates[1]*c2c3,
                                     - andRates[1]*sa2
         );
      
       #ifdef DEBUG_MOON_MATRIX
          MessageInterface::ShowMessage(wxT("angles are: %.17f  %.17f  %.17f\n"),
           librationAngles[0], librationAngles[1], librationAngles[2]);
          MessageInterface::ShowMessage(wxT("rates are: %.17f  %.17f  %.17f\n"),
           andRates[0], andRates[1], andRates[2]);
          MessageInterface::ShowMessage(
          wxT("rotMatrix = %.17f  %.17f  %.17f  %.17f  %.17f  %.17f %.17f  %.17  %.17ff\n"),
           rotMatrix(0,0), rotMatrix(0,1), rotMatrix(0,2), 
           rotMatrix(1,0), rotMatrix(1,1), rotMatrix(1,2), 
           rotMatrix(2,0), rotMatrix(2,1), rotMatrix(2,2));
          MessageInterface::ShowMessage(
          wxT("rotDotMatrix = %.17f  %.17f  %.17f  %.17f  %.17f  %.17f %.17f  %.17f %.17f\n"),
           rotDotMatrix(0,0), rotDotMatrix(0,1), rotDotMatrix(0,2), 
           rotDotMatrix(1,0), rotDotMatrix(1,1), rotDotMatrix(1,2), 
           rotDotMatrix(2,0), rotDotMatrix(2,1), rotDotMatrix(2,2));
      #endif
   }
   else // compute for other bodies, using IAU data
   {
      #ifdef DEBUG_BF_RECOMPUTE
         MessageInterface::ShowMessage(wxT("   RECOMPUTING!!! - and the body name is %s\n"), originName.c_str());
      #endif
      Real Wderiv[9];
      Real R13[3][3];
      Real rotResult[3][3];
      Real rotDotResult[3][3];
      static Rvector cartCoord(4);  
      const Real *cartC = cartCoord.GetDataVector();
      #ifdef DEBUG_FIRST_CALL
         if (!firstCallFired)
            MessageInterface::ShowMessage(wxT("In BFA, Body is %s\n"), originName.c_str());
      #endif
      // this method will return alpha (deg), delta (deg), 
      // W (deg), and Wdot (deg/day)
      cartCoord           = ((CelestialBody*)origin)->
                            GetBodyCartographicCoordinates(atEpoch);
      Real rot1           = GmatMathConstants::PI_OVER_TWO + Rad(cartC[0]);
      Real rot2           = GmatMathConstants::PI_OVER_TWO - Rad(cartC[1]);
      Real W              = Rad(cartC[2]);
      Real Wdot           = Rad(cartC[3]) / SECS_PER_DAY; 
      // Convert Wdot from deg/day to rad/sec
      Real R3leftT[9]     =  {Cos(rot1),-Sin(rot1),0.0,
                              Sin(rot1), Cos(rot1),0.0,
                                    0.0,       0.0,1.0};
      Real R1middleT[9]   =  {1.0,      0.0,       0.0,
                              0.0,Cos(rot2),-Sin(rot2),
                              0.0,Sin(rot2), Cos(rot2)};
      
      Real R3rightT[9]    =  {Cos(W),-Sin(W),0.0,
                              Sin(W), Cos(W),0.0,
                                 0.0,    0.0,1.0};
      
      Integer p3 = 0;
      for (Integer p = 0; p < 3; ++p)
      {
         p3 = 3*p;
         for (Integer q = 0; q < 3; ++q)
         {
            R13[p][q] = R1middleT[p3]   * R3rightT[q]   + 
                        R1middleT[p3+1] * R3rightT[q+3] + 
                        R1middleT[p3+2] * R3rightT[q+6];
         }
      }     
      for (Integer p = 0; p < 3; ++p)
      {
         p3 = 3*p;
         for (Integer q = 0; q < 3; ++q)
         {
            rotResult[p][q] = R3leftT[p3]   * R13[0][q] + 
                              R3leftT[p3+1] * R13[1][q] + 
                              R3leftT[p3+2] * R13[2][q];
         }
      }   
      rotMatrix.Set(rotResult[0][0],rotResult[0][1],rotResult[0][2],  
                    rotResult[1][0],rotResult[1][1],rotResult[1][2],
                    rotResult[2][0],rotResult[2][1],rotResult[2][2]); 
                    
      Wderiv[0] = -Wdot*Sin(W);
      Wderiv[1] = -Wdot*Cos(W);
      Wderiv[2] =  0.0;
      Wderiv[3] =  Wdot*Cos(W);
      Wderiv[4] = -Wdot*Sin(W);
      Wderiv[5] =  0.0;
      Wderiv[6] =  0.0;
      Wderiv[7] =  0.0;
      Wderiv[8] =  0.0;  
         
      for (Integer p = 0; p < 3; ++p)
      {
         p3 = 3*p;
         for (Integer q = 0; q < 3; ++q)
         {
            R13[p][q] = R1middleT[p3]   * Wderiv[q]   + 
                        R1middleT[p3+1] * Wderiv[q+3] + 
                        R1middleT[p3+2] * Wderiv[q+6];
         }
      }     
      for (Integer p = 0; p < 3; ++p)
      {
         p3 = 3*p;
         for (Integer q = 0; q < 3; ++q)
         {
            rotDotResult[p][q] = R3leftT[p3]   * R13[0][q] + 
                                 R3leftT[p3+1] * R13[1][q] + 
                                 R3leftT[p3+2] * R13[2][q];
         }
      }   
      rotDotMatrix.Set(rotDotResult[0][0],rotDotResult[0][1],rotDotResult[0][2],  
                       rotDotResult[1][0],rotDotResult[1][1],rotDotResult[1][2],
                       rotDotResult[2][0],rotDotResult[2][1],rotDotResult[2][2]); 
   }
      
   // Save the epoch for comparison the next time through
   prevEpoch = theEpoch;
   #ifdef DEBUG_BF_RECOMPUTE
      MessageInterface::ShowMessage(wxT("at the end, just set prevEpoch to %12.10f\n"), prevEpoch);
   #endif
   if (originName == SolarSystem::MOON_NAME)
      prevLunaSrc = ((CelestialBody*)origin)->GetRotationDataSource();

   #ifdef DEBUG_FIRST_CALL
      firstCallFired = true;
      MessageInterface::ShowMessage(wxT("NOW exiting BFA::CalculateRotationMatrix ...\n"));
   #endif

}
