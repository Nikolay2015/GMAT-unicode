//$Id: $
//------------------------------------------------------------------------------
//                                  TopocentricAxes
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
// Created: 2008.09.11
//
/**
 * Implementation of the TopocentricAxes class.  
 *
 */
//------------------------------------------------------------------------------

#include <iostream>
#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "TopocentricAxes.hpp"
#include "DynamicAxes.hpp"
#include "SolarSystem.hpp"
#include "CelestialBody.hpp"
#include "RealUtilities.hpp"
#include "Linear.hpp"
#include "TimeTypes.hpp"
#include "Rvector3.hpp"
#include "GmatDefaults.hpp"
#include "TimeSystemConverter.hpp"
#include "CoordinateSystemException.hpp"
#include "MessageInterface.hpp"
#include "BodyFixedPoint.hpp"

using namespace GmatMathUtil;      // for trig functions, etc.
using namespace GmatTimeConstants;      // for JD offsets, etc.

//#define DEBUG_TOPOCENTRIC_AXES

//---------------------------------
// static data
//---------------------------------
// None ... yet, anyway

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  TopocentricAxes(const wxString &itsName);
//------------------------------------------------------------------------------
/**
 * Constructs base TopocentricAxes structures
 * (default constructor).
 *
 * @param itsName Optional name for the object.  Defaults to "".
 */
//------------------------------------------------------------------------------
TopocentricAxes::TopocentricAxes(const wxString &itsName) :
   DynamicAxes      (wxT("Topocentric"),itsName),
   bfPoint          (NULL),
   bfcs             (NULL),
   centralBody      (NULL),
   itsBodyName      (wxT("")),
   horizonReference (wxT("Sphere")),
   flattening       (-999.99),
   radius           (GmatSolarSystemDefaults::PLANET_EQUATORIAL_RADIUS[GmatSolarSystemDefaults::EARTH])
{
   objectTypeNames.push_back(wxT("TopocentricAxes"));
   parameterCount = TopocentricAxesParamCount;
}


//------------------------------------------------------------------------------
//  TopocentricAxes(const TopocentricAxes &tAxes);
//------------------------------------------------------------------------------
/**
 * Constructs base TopocentricAxes structures used in derived classes, by copying 
 * the input instance (copy constructor).
 *
 * @param tAxes  TopocentricAxes instance to copy to create "this" instance.
 */
//------------------------------------------------------------------------------
TopocentricAxes::TopocentricAxes(const TopocentricAxes &tAxes) :
   DynamicAxes       (tAxes),
   bfPoint           (tAxes.bfPoint),
   bfcs              (tAxes.bfcs),
   centralBody       (tAxes.centralBody),
   itsBodyName       (tAxes.itsBodyName),
   horizonReference  (tAxes.horizonReference),
   flattening        (tAxes.flattening),
   radius            (tAxes.radius)
{
}

//------------------------------------------------------------------------------
//  TopocentricAxes& operator=(const TopocentricAxes &tAxes)
//------------------------------------------------------------------------------
/**
 * Assignment operator for TopocentricAxes structures.
 *
 * @param tAxes The original that is being copied.
 *
 * @return Reference to this object
 */
//------------------------------------------------------------------------------
const TopocentricAxes& TopocentricAxes::operator=(const TopocentricAxes &tAxes)
{
   if (&tAxes == this)
      return *this;
   DynamicAxes::operator=(tAxes); 
   bfPoint          = tAxes.bfPoint;
   bfcs             = tAxes.bfcs;
   centralBody      = tAxes.centralBody;
   itsBodyName      = tAxes.itsBodyName;
   horizonReference = tAxes.horizonReference;
   flattening       = tAxes.flattening;
   radius           = tAxes.radius;
   return *this;
}


//------------------------------------------------------------------------------
//  ~TopocentricAxes(void)
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
TopocentricAxes::~TopocentricAxes()
{
}

//------------------------------------------------------------------------------
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * Initialization method for this TopocentricAxes.
 *
 */
//------------------------------------------------------------------------------
bool TopocentricAxes::Initialize()
{
   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(wxT("Entering TopocentricAxes::Initialize()"));
   #endif
   DynamicAxes::Initialize();
   
   // check to make sure that the Origin is a BodyFixedPoint
   if (!(origin->IsOfType(wxT("BodyFixedPoint"))))
   {
      wxString errMsg = wxT("The origin for a Topocentric Coordinate System ");
      errMsg += wxT("must be a BodyFixedPoint");
      throw CoordinateSystemException(errMsg);
   }
   bfPoint = (BodyFixedPoint*) origin; 
   
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
GmatBase* TopocentricAxes::Clone() const
{
   return (new TopocentricAxes(*this));
}

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
 * @note  The implementaiton of this method for this class 
 * ignores the forceComputation argument 
 */
//------------------------------------------------------------------------------
void TopocentricAxes::CalculateRotationMatrix(const A1Mjd &atEpoch,
                                              bool forceComputation) 
{   
   // Check to make sure that the central body is a celestial body
   itsBodyName       = bfPoint->GetStringParameter(wxT("CentralBody"));
   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(wxT("Origin's central body is %s\n"),
            itsBodyName.c_str());
   #endif
   GmatBase *bodyPtr = bfPoint->GetRefObject(Gmat::CELESTIAL_BODY, itsBodyName);
   if (!bodyPtr)
   {
      wxString errMsg = wxT("Central Body for a BodyFixedPoint used in a ");
      errMsg += wxT(" Topocentric Coordinate System is NULL");
      throw CoordinateSystemException(errMsg);
   }
   if (!(bodyPtr->IsOfType(wxT("CelestialBody"))))
   {
      wxString errMsg = wxT("Central Body for a BodyFixedPoint used in a ");
      errMsg += wxT(" Topocentric Coordinate System must be a Celestial Body");
      throw CoordinateSystemException(errMsg);
   }
   centralBody      = (CelestialBody*) bodyPtr;
   flattening       = centralBody->GetFlattening();
   radius           = centralBody->GetEquatorialRadius();
   bfcs             = bfPoint->GetBodyFixedCoordinateSystem();
   horizonReference = bfPoint->GetStringParameter(wxT("HorizonReference"));
   if ((horizonReference != wxT("Sphere")) && (horizonReference != wxT("Ellipsoid")))
   {
      wxString errMsg = wxT("Unexpected horizon reference \"");
      errMsg += horizonReference + wxT("\" received from BodyFixedPoint \"");
      errMsg += bfPoint->GetName() + wxT("\"");
      throw CoordinateSystemException(errMsg);
   }

   // compute rotMatrix and rotDotMatrix
   // First, calculate the Rft matrix, if the position has changed
   Rvector3 newLoc  = bfPoint->GetBodyFixedLocation(atEpoch);
   
   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(wxT("horizon reference is %s:\n"),
            horizonReference.c_str());
      MessageInterface::ShowMessage(wxT("flattening of %s is %12.17f:\n"),
            itsBodyName.c_str(), flattening);
      MessageInterface::ShowMessage(wxT("equatorial radius of %s is %12.17f:\n\n"),
            itsBodyName.c_str(), radius);
      MessageInterface::ShowMessage(wxT("bfPoint's old location is:\n"));
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bfLocation[0]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bfLocation[1]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bfLocation[2]);
      MessageInterface::ShowMessage(wxT("bfPoint's new location is:\n"));
      MessageInterface::ShowMessage(wxT("%12.17f\n"), newLoc[0]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), newLoc[1]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), newLoc[2]);
   #endif
   if (newLoc != bfLocation)
      CalculateRFT(atEpoch, newLoc);
   // save the location
   bfLocation  = newLoc;
   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(wxT("Now bfLocation is set to:\n"));
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bfLocation[0]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bfLocation[1]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bfLocation[2]);
   #endif
   // Determine rotation matrix from body-fixed to inertial
   Rvector bogusIn(6,7000.0,1000.0,6000.0, 0.0, 0.0, 0.0);
   Rvector bogusOut = bfcs->ToMJ2000Eq(atEpoch, bogusIn);
   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(wxT("bogusIn:\n"));
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bogusIn[0]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bogusIn[1]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bogusIn[2]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bogusIn[3]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bogusIn[4]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bogusIn[5]);
      MessageInterface::ShowMessage(wxT("bogusOut:\n"));
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bogusOut[0]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bogusOut[1]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bogusOut[2]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bogusOut[3]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bogusOut[4]);
      MessageInterface::ShowMessage(wxT("%12.17f\n"), bogusOut[5]);
   #endif
   Rmatrix33 RIF    = bfcs->GetLastRotationMatrix();
   Rmatrix33 RIFDot = bfcs->GetLastRotationDotMatrix();
   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(wxT("last Rotation Matrix (RIF):\n"));
      MessageInterface::ShowMessage(wxT("%12.17f  %12.17f  %12.17f \n"),
            RIF(0,0), RIF(0,1), RIF(0,2));
      MessageInterface::ShowMessage(wxT("%12.17f  %12.17f  %12.17f \n"),
            RIF(1,0), RIF(1,1), RIF(1,2));
      MessageInterface::ShowMessage(wxT("%12.17f  %12.17f  %12.17f \n"),
            RIF(2,0), RIF(2,1), RIF(2,2));
      MessageInterface::ShowMessage(wxT("last Rotation Dot Matrix (RIFDot):\n"));
      MessageInterface::ShowMessage(wxT("%12.17f  %12.17f  %12.17f \n"),
            RIFDot(0,0), RIFDot(0,1), RIFDot(0,2));
      MessageInterface::ShowMessage(wxT("%12.17f  %12.17f  %12.17f \n"),
            RIFDot(1,0), RIFDot(1,1), RIFDot(1,2));
      MessageInterface::ShowMessage(wxT("%12.17f  %12.17f  %12.17f \n"),
            RIFDot(2,0), RIFDot(2,1), RIFDot(2,2));
   #endif
   
   rotMatrix        = RIF    * RFT;
   rotDotMatrix     = RIFDot * RFT;
//   rotMatrix        = RIF * RFT;
//   rotDotMatrix     = (RIFDot * RFT) +
//                      (RIF * RFT);
//   rotMatrix        = RIF * (RFT * RIF.Transpose());
//   rotDotMatrix     = (RIFDot * (RFT * RIF.Transpose())) +
//                      (RIF * (RFT * RIFDot.Transpose()));
   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(wxT("rotMatrix:\n"));
      MessageInterface::ShowMessage(wxT("%12.17f  %12.17f  %12.17f \n"),
            rotMatrix(0,0), rotMatrix(0,1), rotMatrix(0,2));
      MessageInterface::ShowMessage(wxT("%12.17f  %12.17f  %12.17f \n"),
            rotMatrix(1,0), rotMatrix(1,1), rotMatrix(1,2));
      MessageInterface::ShowMessage(wxT("%12.17f  %12.17f  %12.17f \n"),
            rotMatrix(2,0), rotMatrix(2,1), rotMatrix(2,2));
      MessageInterface::ShowMessage(wxT("rotDotMatrix:\n"));
      MessageInterface::ShowMessage(wxT("%12.17f  %12.17f  %12.17f \n"),
            rotDotMatrix(0,0), rotDotMatrix(0,1), rotDotMatrix(0,2));
      MessageInterface::ShowMessage(wxT("%12.17f  %12.17f  %12.17f \n"),
            rotDotMatrix(1,0), rotDotMatrix(1,1), rotDotMatrix(1,2));
      MessageInterface::ShowMessage(wxT("%12.17f  %12.17f  %12.17f \n"),
            rotDotMatrix(2,0), rotDotMatrix(2,1), rotDotMatrix(2,2));
   #endif
}


//------------------------------------------------------------------------------
//  void CalculateRFT(const A1Mjd &atEpoch, const Rvector3 newLocation)
//------------------------------------------------------------------------------
/**
 * This method will compute the rotation matrix from the topocentric frame
 * to the body fixed frame.  This only needs to be recalculated if the 
 * BodyFixedPoint's location changes.
 *
 * @param atEpoch      epoch at which to compute the rotation matrix
 * @param newLocation  location in body-fixed coordinates
 *
 * @note  
 */
//------------------------------------------------------------------------------
void TopocentricAxes::CalculateRFT(const A1Mjd &atEpoch, const Rvector3 newLocation)
{
   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(
            wxT("Now entering TopocentricAxes::CalculateRTF\n"));
   #endif
   Rvector3 kUnit(0.0,0.0,1.0);
   Rvector3 xUnit, yUnit, zUnit;
   Real     x, y, z;
   x = newLocation[0];
   y = newLocation[1];
   z = newLocation[2];
   Real rxy      = GmatMathUtil::Sqrt(x*x + y*y);
   // Check for proximity to a pole
   if (rxy < 1.0e-3)
   {
      wxString errmsg = wxT("Topocentric Coordinate System ");
      errmsg += wxT("is undefined due to numerical singularity at the poles\n");
      throw CoordinateSystemException(errmsg);
   }
   // Calculate the geocentric latitude to use as an initial guess
   // to find the geodetic latitude
   Real phigd    = atan2(z, rxy);
   Real eSquared = 2 * flattening - (flattening * flattening);
   Real phiPrime, C, divided, bfLong;

   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(
            wxT("rxy = %12.17f      phigd = %12.17f    eSquared = %12.17f  \n"),
            rxy, phigd, eSquared);
   #endif
   // Initialize the loop and iterate to find the geodetic latitude
   Real delta  = 1.0;
   while (delta > 1.0e-11)
   {
      phiPrime = phigd;
      C        = radius / GmatMathUtil::Sqrt(1 - eSquared *
                 (GmatMathUtil::Sin(phiPrime) * GmatMathUtil::Sin(phiPrime)));
      divided  = (z + (C * eSquared * GmatMathUtil::Sin(phiPrime))) /rxy;
      phigd    = atan(divided);
      #ifdef DEBUG_TOPOCENTRIC_AXES
         MessageInterface::ShowMessage(
               wxT("In the loop, delta = %12.17f   phiPrime = %12.17f    C = %12.17f\n"),
               delta, phiPrime, C);
         MessageInterface::ShowMessage(
               wxT("In the loop, divided = %12.17f   phigd = %12.17f\n"),
               divided, phigd);
      #endif
      delta    = GmatMathUtil::Abs(phigd - phiPrime);
   }
   // Compute the longitude of the BodyFixedPoint location
   bfLong = atan2(y,x);
   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(
            wxT("At the end of the loop, delta = %12.17f\n"),
            delta);
      MessageInterface::ShowMessage(
            wxT("After the loop, bfLong = %12.17f\n"),
            bfLong);
   #endif
   zUnit[0]    = GmatMathUtil::Cos(phigd) * GmatMathUtil::Cos(bfLong);
   zUnit[1]    = GmatMathUtil::Cos(phigd) * GmatMathUtil::Sin(bfLong);
   zUnit[2]    = GmatMathUtil::Sin(phigd);

   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(
            wxT(" ... computed zUnit = %12.17f  %12.17f  %12.17f  \n"),
            zUnit[0], zUnit[1], zUnit[2]);
   #endif

   // Complete the computation of x, y, and RFT
   yUnit          = Cross(kUnit, zUnit);
   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(
            wxT(" ... computed yUnit = %12.17f  %12.17f  %12.17f  \n"),
            yUnit[0], yUnit[1], yUnit[2]);
   #endif
   yUnit          = yUnit.GetUnitVector();
   xUnit          = Cross(yUnit, zUnit);
   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(
            wxT("After the if/else, zUnit = %12.17f  %12.17f  %12.17f\n"),
            zUnit[0], zUnit[1], zUnit[2]);
      MessageInterface::ShowMessage(
            wxT("After the if/else, yUnit = %12.17f  %12.17f  %12.17f\n"),
            yUnit[0], yUnit[1], yUnit[2]);
      MessageInterface::ShowMessage(
            wxT("After the if/else, xUnit = %12.17f  %12.17f  %12.17f\n"),
            xUnit[0], xUnit[1], xUnit[2]);
   #endif
   // Set RFT
   RFT(0,0)       = xUnit(0);
   RFT(1,0)       = xUnit(1);
   RFT(2,0)       = xUnit(2);
   RFT(0,1)       = yUnit(0);
   RFT(1,1)       = yUnit(1);
   RFT(2,1)       = yUnit(2);
   RFT(0,2)       = zUnit(0);
   RFT(1,2)       = zUnit(1);
   RFT(2,2)       = zUnit(2);
      
   #ifdef DEBUG_TOPOCENTRIC_AXES
      MessageInterface::ShowMessage(
            wxT("After the if/else, RFT = %12.17f  %12.17f  %12.17f\n"),
            RFT(0,0), RFT(0,1), RFT(0,2));
      MessageInterface::ShowMessage(
            wxT("                         %12.17f  %12.17f  %12.17f\n"),
            RFT(1,0), RFT(1,1), RFT(1,2));
      MessageInterface::ShowMessage(
            wxT("                         %12.17f  %12.17f  %12.17f\n"),
            RFT(2,0), RFT(2,1), RFT(2,2));
   #endif
}

