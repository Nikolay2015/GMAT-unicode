//$Id: Thruster.cpp 9805 2011-08-26 19:28:13Z djcinsb $
//------------------------------------------------------------------------------
//                               Thruster
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Darrel J. Conway
// Created: 2004/11/08
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under MOMS Task
// Order 124.
//
/**
 * Class implementation for the Spacecraft engines (aka Thrusters).
 */
//------------------------------------------------------------------------------


#include "Thruster.hpp"
#include "ObjectReferencedAxes.hpp"
#include "Spacecraft.hpp"
#include "StringUtil.hpp"
#include "HardwareException.hpp"
#include "MessageInterface.hpp"
#include <sstream>
#include <math.h>          // for pow(real, real)

//#define DEBUG_THRUSTER
//#define DEBUG_THRUSTER_CONSTRUCTOR
//#define DEBUG_THRUSTER_SET
//#define DEBUG_THRUSTER_REF_OBJ
//#define DEBUG_THRUSTER_INIT
//#define DEBUG_THRUSTER_CONVERT
//#define DEBUG_THRUSTER_CONVERT_ROTMAT
//#define DEBUG_BURN_CONVERT_ROTMAT

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//---------------------------------
// static data
//---------------------------------

/// Available local axes labels
StringArray Thruster::localAxesLabels;
/// C-coefficient units
StringArray Thruster::cCoefUnits;
/// K-coefficient units
StringArray Thruster::kCoefUnits;

/// Labels used for the thruster element parameters.
const wxString
Thruster::PARAMETER_TEXT[ThrusterParamCount - HardwareParamCount] =
{
   wxT("IsFiring"),
   wxT("CoordinateSystem"),
   wxT("Origin"),
   wxT("Axes"),
   wxT("DutyCycle"),
   wxT("ThrustScaleFactor"),
   wxT("DecrementMass"),
   wxT("Tank"),
   wxT("GravitationalAccel"),
   wxT("C1"),  wxT("C2"),  wxT("C3"),  wxT("C4"),  wxT("C5"),  wxT("C6"),  wxT("C7"),  wxT("C8"),
   wxT("C9"), wxT("C10"), wxT("C11"), wxT("C12"), wxT("C13"), wxT("C14"), wxT("C15"), wxT("C16"),
   
   wxT("K1"),  wxT("K2"),  wxT("K3"),  wxT("K4"),  wxT("K5"),  wxT("K6"),  wxT("K7"),  wxT("K8"),
   wxT("K9"), wxT("K10"), wxT("K11"), wxT("K12"), wxT("K13"), wxT("K14"), wxT("K15"), wxT("K16"),
   wxT("C_UNITS"),
   wxT("K_UNITS"),
};

/// Types of the parameters used by thrusters.
const Gmat::ParameterType
Thruster::PARAMETER_TYPE[ThrusterParamCount - HardwareParamCount] =
{
   Gmat::BOOLEAN_TYPE,     // wxT("IsFiring")
   Gmat::OBJECT_TYPE,      // wxT("CoordinateSystem")
   Gmat::OBJECT_TYPE,      // wxT("Origin")
   Gmat::ENUMERATION_TYPE, // wxT("Axes")
   Gmat::REAL_TYPE,        // wxT("DutyCycle")
   Gmat::REAL_TYPE,        // wxT("ThrustScaleFactor")
   Gmat::BOOLEAN_TYPE,     // wxT("DecrementMass")
   Gmat::OBJECTARRAY_TYPE, // wxT("Tank")
   Gmat::REAL_TYPE,        // wxT("GravitationalAccel")
   // C's
   Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE,
   Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE,
   Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE,
   Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE,
   // K's
   Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE,
   Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE,
   Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE,
   Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE, Gmat::REAL_TYPE,
   Gmat::STRINGARRAY_TYPE,
   Gmat::STRINGARRAY_TYPE,
};


//------------------------------------------------------------------------------
//  Thruster(wxString nomme)
//------------------------------------------------------------------------------
/**
 * Thruster constructor with default VNB Local CoordinateSystem.
 *
 * @param nomme Name of the thruster.
 *
 * @note coordSystem and spacecraft are set through SetRefObject() during
 *       Sandbox initialization. localOrigin and j2000Body are reset when
 *       solarSystem is set. localCoordSystem is created during initialization
 *       or when new spacecraft is set
 */
//------------------------------------------------------------------------------
Thruster::Thruster(wxString nomme) :
   Hardware             (Gmat::THRUSTER, wxT("Thruster"), nomme),
   solarSystem          (NULL),
   localCoordSystem     (NULL),
   coordSystem          (NULL),
   localOrigin          (NULL),
   j2000Body            (NULL),
   spacecraft           (NULL),
   coordSystemName      (wxT("Local")),
   localOriginName      (wxT("Earth")),
   localAxesName        (wxT("VNB")),
   j2000BodyName        (wxT("Earth")),
   satName              (wxT("")),
   gravityAccel         (9.81),
   dutyCycle            (1.0),
   thrustScaleFactor    (1.0),
   pressure             (1500.0),
   temperatureRatio     (1.0),
   thrust               (500.0),
   impulse              (2150.0),
   mDot                 (0.0),
   decrementMass        (false),
   thrusterFiring       (false),
   constantExpressions  (true),
   simpleExpressions    (true),
   usingLocalCoordSys   (true),
   isMJ2000EqAxes       (false),
   isSpacecraftBodyAxes (false),
   initialized          (false)
{
   objectTypes.push_back(Gmat::THRUSTER);
   objectTypeNames.push_back(wxT("Thruster"));
   parameterCount = ThrusterParamCount;
   
   inertialDirection[0] = 1.0;
   inertialDirection[1] = 0.0;
   inertialDirection[2] = 0.0;
   
   cCoefficients[0]  = 10.0;
   for (int i=1; i<COEFFICIENT_COUNT; i++)
      cCoefficients[i] = 0.0;
   
   kCoefficients[0]  = 300.0;
   for (int i=1; i<COEFFICIENT_COUNT; i++)
      kCoefficients[i] = 0.0;
   
   // Available local axes labels
   // Since it is static data, clear it first
   localAxesLabels.clear();
   localAxesLabels.push_back(wxT("VNB"));
   localAxesLabels.push_back(wxT("LVLH"));
   localAxesLabels.push_back(wxT("MJ2000Eq"));
   localAxesLabels.push_back(wxT("SpacecraftBody"));
   
   // C coefficient units
   cCoefUnits.clear();
   cCoefUnits.push_back(wxT("N"));
   cCoefUnits.push_back(wxT("N/Pa"));
   cCoefUnits.push_back(wxT("N"));
   cCoefUnits.push_back(wxT("N/Pa"));
   cCoefUnits.push_back(wxT("N/Pa^2"));
   cCoefUnits.push_back(wxT("N/Pa^C7"));      
   cCoefUnits.push_back(wxT("None"));   
   cCoefUnits.push_back(wxT("N/Pa^C9"));
   cCoefUnits.push_back(wxT("None"));
   cCoefUnits.push_back(wxT("N/Pa^C11"));
   cCoefUnits.push_back(wxT("None"));
   cCoefUnits.push_back(wxT("N"));
   cCoefUnits.push_back(wxT("None"));
   cCoefUnits.push_back(wxT("1/Pa"));
   cCoefUnits.push_back(wxT("None"));
   cCoefUnits.push_back(wxT("1/Pa"));
   
   // K coefficient units
   kCoefUnits.clear();
   kCoefUnits.push_back(wxT("s"));
   kCoefUnits.push_back(wxT("s/Pa"));
   kCoefUnits.push_back(wxT("s"));
   kCoefUnits.push_back(wxT("s/Pa"));
   kCoefUnits.push_back(wxT("s/Pa^2"));      
   kCoefUnits.push_back(wxT("s/Pa^K7"));   
   kCoefUnits.push_back(wxT("None"));
   kCoefUnits.push_back(wxT("s/Pa^K9"));
   kCoefUnits.push_back(wxT("None"));
   kCoefUnits.push_back(wxT("s/Pa^K11"));
   kCoefUnits.push_back(wxT("None"));
   kCoefUnits.push_back(wxT("s"));
   kCoefUnits.push_back(wxT("None"));
   kCoefUnits.push_back(wxT("1/Pa"));
   kCoefUnits.push_back(wxT("None"));
   kCoefUnits.push_back(wxT("1/Pa"));
   
   // set parameter write order
   for (Integer i=HardwareParamCount; i <= AXES; i++)
      parameterWriteOrder.push_back(i);
   
   parameterWriteOrder.push_back(DIRECTION_X);
   parameterWriteOrder.push_back(DIRECTION_Y);
   parameterWriteOrder.push_back(DIRECTION_Z);
   
   for (Integer i=DUTY_CYCLE; i < ThrusterParamCount; i++)
      parameterWriteOrder.push_back(i);
   
}


//------------------------------------------------------------------------------
//  ~Thruster()
//------------------------------------------------------------------------------
/**
 * Thruster destructor.
 */
//------------------------------------------------------------------------------
Thruster::~Thruster()
{
   // delete local coordiante system
   if (usingLocalCoordSys && localCoordSystem)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (localCoordSystem, wxT("localCS"), wxT("Thruster::~Thruster()"),
          wxT("deleting localCoordSystem"));
      #endif
      delete localCoordSystem;
      localCoordSystem = NULL;
   }
}


//------------------------------------------------------------------------------
//  Thruster(const Thruster& th)
//------------------------------------------------------------------------------
/**
 * Thruster copy constructor.
 *
 * @param th The object being copied.
 *
 * @note coordSystem and spacecraft are set through SetRefObject() during
 *       Sandbox initialization. localOrigin and j2000Body are reset when
 *       solarSystem is set. localCoordSystem is created during initialization
 *       or when new spacecraft is set
 */
//------------------------------------------------------------------------------
Thruster::Thruster(const Thruster& th) :
   Hardware             (th),   
   solarSystem          (th.solarSystem), // copy
   localCoordSystem     (NULL),
   coordSystem          (th.coordSystem), // copy
   localOrigin          (th.localOrigin), // copy
   j2000Body            (th.j2000Body),   // copy
   spacecraft           (NULL),
   coordSystemName      (th.coordSystemName),
   localOriginName      (th.localOriginName),
   localAxesName        (th.localAxesName),
   j2000BodyName        (th.j2000BodyName),
   satName              (th.satName),
   gravityAccel         (th.gravityAccel),
   dutyCycle            (th.dutyCycle),
   thrustScaleFactor    (th.thrustScaleFactor),
   pressure             (th.pressure),
   temperatureRatio     (th.temperatureRatio),
   thrust               (th.thrust),
   impulse              (th.impulse),
   mDot                 (th.mDot),
   decrementMass        (th.decrementMass),
   thrusterFiring       (th.thrusterFiring),
   constantExpressions  (th.constantExpressions),
   simpleExpressions    (th.simpleExpressions),
   usingLocalCoordSys   (th.usingLocalCoordSys),
   isMJ2000EqAxes       (th.isMJ2000EqAxes),
   isSpacecraftBodyAxes (th.isSpacecraftBodyAxes),
   initialized          (false),
   tankNames            (th.tankNames)
{
   #ifdef DEBUG_THRUSTER_CONSTRUCTOR
   MessageInterface::ShowMessage
      (wxT("Thruster::Thruster(copy) entered, this = <%p>'%s', copying from <%p>'%s'\n"),
       this, GetName().c_str(), &th, th.GetName().c_str());
   #endif
   
   parameterCount = th.parameterCount;
   localAxesLabels = th.localAxesLabels;
   
   inertialDirection[0] = th.inertialDirection[0];
   inertialDirection[1] = th.inertialDirection[1];
   inertialDirection[2] = th.inertialDirection[2];
   
   memcpy(cCoefficients, th.cCoefficients, COEFFICIENT_COUNT * sizeof(Real));
   memcpy(kCoefficients, th.kCoefficients, COEFFICIENT_COUNT * sizeof(Real));
   
   #ifdef DEBUG_THRUSTER_CONSTRUCTOR
   MessageInterface::ShowMessage
      (wxT("   copying tanks. There are %d tank(s) to copy from <%p>\n"), th.tanks.size(), &th);
   MessageInterface::ShowMessage
      (wxT("   solarSystem=<%p>, usingLocalCoordSys=%d, coordSystem=<%p>\n"),
       solarSystem, usingLocalCoordSys, coordSystem);
   #endif
   
   // copy tanks
   tanks = th.tanks;
   
   #ifdef DEBUG_THRUSTER_CONSTRUCTOR
   MessageInterface::ShowMessage(wxT("Thruster::Thruster(copy) exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
//  Thruster& operator=(const Thruster& th)
//------------------------------------------------------------------------------
/**
 * Thruster assignment operator.
 *
 * @param th The object being copied.
 *
 * @return this object, with parameters set to the input object's parameters.
 *
 * @note coordSystem and spacecraft are set through SetRefObject() during
 *       Sandbox initialization. localOrigin and j2000Body are reset when
 *       solarSystem is set. localCoordSystem is created during initialization
 *       or when new spacecraft is set
 */
//------------------------------------------------------------------------------
Thruster& Thruster::operator=(const Thruster& th)
{
   if (&th == this)
      return *this;
   
   #ifdef DEBUG_THRUSTER_CONSTRUCTOR
   MessageInterface::ShowMessage(wxT("Thruster::operator= entered\n"));
   #endif
   
   Hardware::operator=(th);
   
   solarSystem         = th.solarSystem; // copy
   localCoordSystem    = NULL;
   coordSystem         = th.coordSystem; // copy
   localOrigin         = th.localOrigin; // copy
   j2000Body           = th.j2000Body;   // copy
   spacecraft          = NULL;
   coordSystemName     = th.coordSystemName;
   localOriginName     = th.localOriginName;
   localAxesName       = th.localAxesName;
   j2000BodyName       = th.j2000BodyName;
   satName             = th.satName;
   
   gravityAccel        = th.gravityAccel;
   dutyCycle           = th.dutyCycle;
   thrustScaleFactor   = th.thrustScaleFactor;
   pressure            = th.pressure;
   temperatureRatio    = th.temperatureRatio;
   thrust              = th.thrust;
   impulse             = th.impulse;
   mDot                = th.mDot;
   
   inertialDirection[0]  = th.inertialDirection[0];
   inertialDirection[1]  = th.inertialDirection[1];
   inertialDirection[2]  = th.inertialDirection[2];
   
   memcpy(cCoefficients, th.cCoefficients, COEFFICIENT_COUNT * sizeof(Real));
   memcpy(kCoefficients, th.kCoefficients, COEFFICIENT_COUNT * sizeof(Real));
   
   thrusterFiring      = th.thrusterFiring;
   decrementMass       = th.decrementMass;
   constantExpressions = th.constantExpressions;
   simpleExpressions   = th.simpleExpressions;
   usingLocalCoordSys  = th.usingLocalCoordSys;
   usingLocalCoordSys  = th.usingLocalCoordSys;
   localAxesLabels     = th.localAxesLabels;
   initialized         = false;
   
   localAxesLabels     = th.localAxesLabels;
   tankNames           = th.tankNames;
   
   // copy tanks
   tanks               = th.tanks;
   
   #ifdef DEBUG_THRUSTER_CONSTRUCTOR
   MessageInterface::ShowMessage(wxT("Thruster::operator= exiting\n"));
   #endif
   
   return *this;
}


//---------------------------------------------------------------------------
//  GmatBase* Clone() const
//---------------------------------------------------------------------------
/**
 * Provides a clone of this object by calling the copy constructor.
 *
 * @return A GmatBase pointer to the cloned thruster.
 */
//---------------------------------------------------------------------------
GmatBase* Thruster::Clone() const
{
   return new Thruster(*this);
}


//---------------------------------------------------------------------------
//  void Copy(GmatBase* orig)
//---------------------------------------------------------------------------
/**
 * Sets this object to match another one.
 *
 * @param orig The original that is being copied.
 *
 * @return A GmatBase pointer to the cloned thruster.
 */
//---------------------------------------------------------------------------
void Thruster::Copy(const GmatBase* orig)
{
   operator=(*((Thruster *)(orig)));
}


//------------------------------------------------------------------------------
//  wxString  GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter text, given the input parameter ID.
 *
 * @param <id> Id for the requested parameter text.
 *
 * @return parameter text for the requested parameter.
 */
//------------------------------------------------------------------------------
wxString Thruster::GetParameterText(const Integer id) const
{
   if (id >= HardwareParamCount && id < ThrusterParamCount)
      return PARAMETER_TEXT[id - HardwareParamCount];
   
   if (id == DIRECTION_X)
      return wxT("ThrustDirection1");
   if (id == DIRECTION_Y)
      return wxT("ThrustDirection2");
   if (id == DIRECTION_Z)
      return wxT("ThrustDirection3");
   
   return Hardware::GetParameterText(id);
}

//------------------------------------------------------------------------------
//  Integer  GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter ID, given the input parameter string.
 *
 * @param <str> string for the requested parameter.
 *
 * @return ID for the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Integer Thruster::GetParameterID(const wxString &str) const
{
   for (Integer i = HardwareParamCount; i < ThrusterParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - HardwareParamCount])
         return i;
   }
   
   if (str == wxT("ThrustDirection1"))
      return DIRECTION_X;
   if (str == wxT("ThrustDirection2"))
      return DIRECTION_Y;
   if (str == wxT("ThrustDirection3"))
      return DIRECTION_Z;
   
   if (str == wxT("X_Direction") || str == wxT("Element1"))
   {
      WriteDeprecatedMessage(str, wxT("ThrustDirection1"));
      return DIRECTION_X;
   }
   if (str == wxT("Y_Direction") || str == wxT("Element2"))
   {
      WriteDeprecatedMessage(str, wxT("ThrustDirection2"));
      return DIRECTION_Y;
   }
   if (str == wxT("Z_Direction") || str == wxT("Element3"))
   {
      WriteDeprecatedMessage(str, wxT("ThrustDirection3"));
      return DIRECTION_Z;
   }
   
   return Hardware::GetParameterID(str);
}


//------------------------------------------------------------------------------
//  Gmat::ParameterType  GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type, given the input parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return parameter type of the requested parameter.
 */
//------------------------------------------------------------------------------
Gmat::ParameterType Thruster::GetParameterType(const Integer id) const
{
   if (id >= HardwareParamCount && id < ThrusterParamCount)
      return PARAMETER_TYPE[id - HardwareParamCount];

   return Hardware::GetParameterType(id);
}


//------------------------------------------------------------------------------
//  wxString  GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type string, given the input parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return parameter type string of the requested parameter.
 */
//------------------------------------------------------------------------------
wxString Thruster::GetParameterTypeString(const Integer id) const
{
   return GmatBase::PARAM_TYPE_STRING[GetParameterType(id)];
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <id> Description for the parameter.
 *
 * @return true if the parameter is read only, false (the default) if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool Thruster::IsParameterReadOnly(const Integer id) const
{
   if ((id == THRUSTER_FIRING) || id == C_UNITS || id == K_UNITS)
      return true;
   
   if ((id == ORIGIN || id == AXES))
      if (coordSystemName != wxT("Local"))
         return true;
   
   return Hardware::IsParameterReadOnly(id);
}


//------------------------------------------------------------------------------
//  Real GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieve the value for a Real parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
Real Thruster::GetRealParameter(const Integer id) const
{
   switch (id)
   {
      case C1:
         return cCoefficients[0];
      case C2:
         return cCoefficients[1];
      case C3:
         return cCoefficients[2];
      case C4:
         return cCoefficients[3];
      case C5:
         return cCoefficients[4];
      case C6:
         return cCoefficients[5];
      case C7:
         return cCoefficients[6];
      case C8:
         return cCoefficients[7];
      case C9:
         return cCoefficients[8];
      case C10:
         return cCoefficients[9];
      case C11:
         return cCoefficients[10];
      case C12:
         return cCoefficients[11];
      case C13:
         return cCoefficients[12];
      case C14:
         return cCoefficients[13];
      case C15:
         return cCoefficients[14];
      case C16:
         return cCoefficients[15];

      case K1:
         return kCoefficients[0];
      case K2:
         return kCoefficients[1];
      case K3:
         return kCoefficients[2];
      case K4:
         return kCoefficients[3];
      case K5:
         return kCoefficients[4];
      case K6:
         return kCoefficients[5];
      case K7:
         return kCoefficients[6];
      case K8:
         return kCoefficients[7];
      case K9:
         return kCoefficients[8];
      case K10:
         return kCoefficients[9];
      case K11:
         return kCoefficients[10];
      case K12:
         return kCoefficients[11];
      case K13:
         return kCoefficients[12];
      case K14:
         return kCoefficients[13];
      case K15:
         return kCoefficients[14];
      case K16:
         return kCoefficients[15];
         
      case DUTY_CYCLE:
         return dutyCycle;
      case THRUST_SCALE_FACTOR:
         return thrustScaleFactor;
      case GRAVITATIONAL_ACCELERATION:
         return gravityAccel;

      default:
         break;   // Default just drops through
   }

   return Hardware::GetRealParameter(id);
}


//------------------------------------------------------------------------------
//  Real SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * Set the value for a Real parameter.
 *
 * @param id The integer ID for the parameter.
 * @param value The new parameter value.
 *
 * @return the parameter value at the end of this call, or throw an exception
 *         if the parameter id is invalid or the parameter type is not Real.
 */
//------------------------------------------------------------------------------
Real Thruster::SetRealParameter(const Integer id, const Real value)
{
   #ifdef DEBUG_THRUSTER_SET
   MessageInterface::ShowMessage
      (wxT("Thruster::SetRealParameter() '%s' entered, id=%d, value=%f\n"),
       GetName().c_str(), id, value);
   #endif
   
   switch (id) 
   {
      // Thrust coefficients
      case C1:
         return cCoefficients[0] = value;
      case C2:
         if (value != 0.0)
            constantExpressions = false;
         return cCoefficients[1] = value;
      case C3:
         return cCoefficients[2] = value;
      case C4:
         if (value != 0.0) 
            constantExpressions = false;
         return cCoefficients[3] = value;
      case C5:
         if (value != 0.0) 
            constantExpressions = false;
         return cCoefficients[4] = value;
      case C6:
         if (value != 0.0) 
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return cCoefficients[5] = value;
      case C7:
         if (value != 0.0) 
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return cCoefficients[6] = value;
      case C8:
         if (value != 0.0) 
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return cCoefficients[7] = value;
      case C9:
         if (value != 0.0) 
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return cCoefficients[8] = value;
      case C10:
         if (value != 0.0) 
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return cCoefficients[9] = value;
      case C11:
         if (value != 0.0)
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return cCoefficients[10] = value;
      case C12:
         if (value != 0.0)
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return cCoefficients[11] = value;
      case C13:
         if ((value != 0.0) && (value != 1.0)) 
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return cCoefficients[12] = value;
      case C14:
         if (value != 0.0) 
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return cCoefficients[13] = value;
      case C15:
         return cCoefficients[14] = value;
      case C16:
         return cCoefficients[15] = value;
         
      // Isp Coefficients
      case K1:
         return kCoefficients[0] = value;
      case K2:
         if (value != 0.0)
            constantExpressions = false;
         return kCoefficients[1] = value;
      case K3:
         return kCoefficients[2] = value;
      case K4:
         if (value != 0.0) 
            constantExpressions = false;
         return kCoefficients[3] = value;
      case K5:
         if (value != 0.0) 
            constantExpressions = false;
         return kCoefficients[4] = value;
      case K6:
         if (value != 0.0) 
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return kCoefficients[5] = value;
      case K7:
         if (value != 0.0) 
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return kCoefficients[6] = value;
      case K8:
         if (value != 0.0)
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return kCoefficients[7] = value;
      case K9:
         if (value != 0.0)
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return kCoefficients[8] = value;
      case K10:
         if (value != 0.0) 
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return kCoefficients[9] = value;
      case K11:
         if (value != 0.0) 
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return kCoefficients[10] = value;
      case K12:
         if (value != 0.0) 
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return kCoefficients[11] = value;
      case K13:
         if ((value != 0.0) && (value != 1.0))  
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return kCoefficients[12] = value;
      case K14:
         if (value != 0.0) 
         {
            constantExpressions = false;
            simpleExpressions = false;
         }
         return kCoefficients[13] = value;
      case K15:
         return kCoefficients[14] = value;
      case K16:
         return kCoefficients[15] = value;
         
      // Other parameters
      case DUTY_CYCLE:
         if ((value >= 0.0) && (value <= 1.0))
            dutyCycle = value;
         else
         {
            HardwareException he;
            he.SetDetails(errorMessageFormat.c_str(),
                          GmatStringUtil::ToString(value, GetDataPrecision()).c_str(),
                          wxT("DutyCycle"), wxT("0.0 <= Real Number <= 1.0"));
            throw he;
         }
         return dutyCycle;
      case THRUST_SCALE_FACTOR:
         if (value >= 0.0)
            thrustScaleFactor = value;
         else
         {
            HardwareException he;
            he.SetDetails(errorMessageFormat.c_str(),
                          GmatStringUtil::ToString(value, GetDataPrecision()).c_str(),
                          wxT("ThrustScaleFactor"), wxT("Real Number >= 0.0"));
            throw he;
         }
         return thrustScaleFactor;
      case GRAVITATIONAL_ACCELERATION:
         if (value > 0.0)
            gravityAccel = value;
         return gravityAccel;


      default:
         break;   // Default just drops through
   }
   
   return Hardware::SetRealParameter(id, value);
}


//---------------------------------------------------------------------------
//  wxString GetStringParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve a string parameter.
 *
 * @param id The integer ID for the parameter.
 *
 * @return The string stored for this parameter, or throw ab=n exception if
 *         there is no string association.
 */
//---------------------------------------------------------------------------
wxString Thruster::GetStringParameter(const Integer id) const
{
   if (id == COORDINATE_SYSTEM)
      return coordSystemName;
   if (id == ORIGIN)
      return localOriginName;
   if (id == AXES)
      return localAxesName;
   
   return Hardware::GetStringParameter(id);
}


//---------------------------------------------------------------------------
//  bool SetStringParameter(const Integer id, const wxString &value)
//---------------------------------------------------------------------------
/**
 * Change the value of a string parameter.
 *
 * @param id The integer ID for the parameter.
 * @param value The new string for this parameter.
 *
 * @return true if the string is stored, throw if the string is not stored.
 */
//---------------------------------------------------------------------------
bool Thruster::SetStringParameter(const Integer id, const wxString &value)
{
   #ifdef DEBUG_THRUSTER_SET
   MessageInterface::ShowMessage
      (wxT("Thruster::SetStringParameter() '%s' entered, id=%d, value='%s'\n"),
       GetName().c_str(), id, value.c_str());
   #endif
   
   switch (id)
   {
   case COORDINATE_SYSTEM:
      coordSystemName = value;
      if (value == wxT("Local"))
         usingLocalCoordSys = true;
      else
         usingLocalCoordSys = false;
      return true;
   case ORIGIN:
      localOriginName = value;
      return true;
   case AXES:
      {
         localAxesName = value;
         if (find(localAxesLabels.begin(), localAxesLabels.end(), localAxesName)
             == localAxesLabels.end())
         {
            wxString framelist = localAxesLabels[0];
            for (UnsignedInt n = 1; n < localAxesLabels.size(); ++n)
               framelist += wxT(", ") + localAxesLabels[n];
            
            wxString msg =
               wxT("The value of \"") + value + wxT("\" for field \"Axes\"")
               wxT(" on object \"") + instanceName + wxT("\" is not an allowed value.\n")
               wxT("The allowed values are: [ ") + framelist + wxT(" ]. ");
            
            throw HardwareException(msg);
         }
         return true;
      }
   case TANK:
      if ((tankNames.size() > 0) && (value != tankNames[0]))
      {
         wxString errmsg;
         errmsg =  wxT("The value of \"");
         errmsg += value;
         errmsg += wxT("\" on Thruster \"");
         errmsg += instanceName;
         errmsg += wxT("\" is not an allowed value.  GMAT does not currently ")
               wxT("support more than one tank per thruster; the thruster is ")
               wxT("already associated with tank \"");
         errmsg += tankNames[0];
         errmsg += wxT("\".");
         throw HardwareException(errmsg);
      }
      // if not the same name push back
      if (find(tankNames.begin(), tankNames.end(), value) == tankNames.end())
         tankNames.push_back(value);
      return true;
   default:
      return Hardware::SetStringParameter(id, value);
   }
}


//---------------------------------------------------------------------------
//  wxString GetStringParameter(const Integer id, const Integer index) const
//---------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
wxString Thruster::GetStringParameter(const Integer id,
                                         const Integer index) const
{
   switch (id)
   {
   case TANK:
      {
         if (index >= 0 && index < (Integer)tankNames.size())
            return tankNames[index];
         else
            throw HardwareException
               (wxT("Thruster::GetStringParameter() \"") + instanceName + wxT(", ") + 
                GmatStringUtil::ToString(index) +  wxT(" is invalid Tank index"));
      }
   default:
      return Hardware::GetStringParameter(id, index);
   }
}


//---------------------------------------------------------------------------
//  bool SetStringParameter(const Integer id, const wxString &value,
//                          const Integer index)
//---------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
bool Thruster::SetStringParameter(const Integer id, const wxString &value,
                                  const Integer index)
{
   switch (id)
   {
   case TANK:
      {
         if (index > 0)
         {
            wxString errmsg;
            errmsg =  wxT("The value of \"");
            errmsg += value;
            errmsg += wxT("\" on Thruster \"");
            errmsg += instanceName;
            errmsg += wxT("\" is not an allowed value.  GMAT does not currently ")
                  wxT("support more than one tank per thruster");
            throw HardwareException(errmsg);
         }

         if (index < (Integer)tankNames.size())
            tankNames[index] = value;
         else
            // Only add the tank if it is not in the list already
            if (find(tankNames.begin(), tankNames.end(), value) == tankNames.end()) 
               tankNames.push_back(value);
         
         return true;
      }
   default:
      return Hardware::SetStringParameter(id, value, index);
   }
}


//---------------------------------------------------------------------------
//  const StringArray& GetStringArrayParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Access an array of string data.
 *
 * @param id The integer ID for the parameter.
 *
 * @return The requested StringArray; throws if the parameter is not a
 *         StringArray.
 */
//---------------------------------------------------------------------------
const StringArray& Thruster::GetStringArrayParameter(const Integer id) const
{
   if (id == TANK)
      return tankNames;
   
   if (id == C_UNITS)
      return cCoefUnits;
   
   if (id == K_UNITS)
      return kCoefUnits;
   
   return Hardware::GetStringArrayParameter(id);
}


//---------------------------------------------------------------------------
//  bool GetBooleanParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve a boolean parameter.
 *
 * @param id The integer ID for the parameter.
 *
 * @return the boolean value for this parameter, or throw an exception if the
 *         parameter access in invalid.
 */
//---------------------------------------------------------------------------
bool Thruster::GetBooleanParameter(const Integer id) const
{
   if (id == THRUSTER_FIRING)
      return thrusterFiring;
   
   if (id == DECREMENT_MASS)
      return decrementMass;
   
   return Hardware::GetBooleanParameter(id);
}


//---------------------------------------------------------------------------
//  bool SetBooleanParameter(const Integer id, const bool value)
//---------------------------------------------------------------------------
/**
 * Sets the value for a boolean parameter.
 *
 * @param id The integer ID for the parameter.
 * @param value The new value.
 *
 * @return the boolean value for this parameter, or throw an exception if the
 *         parameter is invalid or not boolean.
 */
//---------------------------------------------------------------------------
bool Thruster::SetBooleanParameter(const Integer id, const bool value)
{
   if (id == THRUSTER_FIRING) {
      #ifdef DEBUG_THRUSTER
         MessageInterface::ShowMessage(
            wxT("Setting thruster %s firing mode to %s\n"), instanceName.c_str(),
            (value == true ? wxT("true") : wxT("false")));
      #endif
      thrusterFiring = value;
      return thrusterFiring;
   }
   
   if (id == DECREMENT_MASS)
   {
      decrementMass = value;
      return decrementMass;
   }
   
   return Hardware::SetBooleanParameter(id, value);
}


//------------------------------------------------------------------------------
//  const StringArray& GetPropertyEnumStrings(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Access an array of enumerated string data.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The requested StringArray; throws if the parameter is not a 
 *         StringArray.
 */
//------------------------------------------------------------------------------
const StringArray& Thruster::GetPropertyEnumStrings(const Integer id) const
{
   if (id == AXES)
      return localAxesLabels;
   
   return GmatBase::GetPropertyEnumStrings(id);
}


//------------------------------------------------------------------------------
//  const StringArray& GetPropertyEnumStrings(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Access an array of enumerated string data.
 *
 * @param <label> The parameter name.
 *
 * @return The requested StringArray
 */
//------------------------------------------------------------------------------
const StringArray& Thruster::GetPropertyEnumStrings(const wxString &label) const
{
   return GetPropertyEnumStrings(GetParameterID(label));
}


//------------------------------------------------------------------------------
// wxString GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
wxString Thruster::GetRefObjectName(const Gmat::ObjectType type) const
{
   #ifdef DEBUG_THRUSTER_REF_OBJ
   MessageInterface::ShowMessage
      (wxT("Thruster::GetRefObjectName() <%p>'%s' entered, type=%d\n"), this,
       GetName().c_str(), type);
   #endif
   
   wxString refObjName;
   if (type == Gmat::COORDINATE_SYSTEM)
   {
      wxString refObjName;
      if (!usingLocalCoordSys)
         refObjName = coordSystemName;
      
      #ifdef DEBUG_THRUSTER_REF_OBJ
      MessageInterface::ShowMessage
         (wxT("Thruster::GetRefObjectName() <%p>'%s' returning '%s'\n"), this,
          GetName().c_str(), refObjName.c_str());
      #endif
      
      return refObjName;
   }
   
   return Hardware::GetRefObjectName(type);
}


//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by this class.
 *
 * @return the list of object types.
 * 
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& Thruster::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::COORDINATE_SYSTEM);
   refObjectTypes.push_back(Gmat::CELESTIAL_BODY);
   refObjectTypes.push_back(Gmat::SPACECRAFT);
   return refObjectTypes;
}


//------------------------------------------------------------------------------
// virtual const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
const StringArray& Thruster::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   #ifdef DEBUG_THRUSTER_REF_OBJ
   MessageInterface::ShowMessage
      (wxT("Thruster::GetRefObjectNameArray() <%p>'%s' entered, type=%d\n"),
       this, GetName().c_str(), type);
   #endif
   
   refObjectNames.clear();
   if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::COORDINATE_SYSTEM)
   {
      if (!usingLocalCoordSys)
         refObjectNames.push_back(coordSystemName);
   }
   
   if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::CELESTIAL_BODY)
   {
      if (usingLocalCoordSys)
      {
         refObjectNames.push_back(j2000BodyName);
         if (localOriginName != j2000BodyName)
            refObjectNames.push_back(localOriginName);
      }
   }
   
   if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::SPACECRAFT)
   {
      if (satName != wxT(""))
         refObjectNames.push_back(satName);
   }
   
   #ifdef DEBUG_THRUSTER_REF_OBJ
   MessageInterface::ShowMessage
      (wxT("Thruster::GetRefObjectNameArray(), refObjectNames.size()=%d\n"),
       refObjectNames.size());
   for (UnsignedInt i=0; i<refObjectNames.size(); i++)
      MessageInterface::ShowMessage(wxT("   '%s'\n"), refObjectNames[i].c_str());
   #endif
   
   return refObjectNames;
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
bool Thruster::RenameRefObject(const Gmat::ObjectType type,
                               const wxString &oldName,
                               const wxString &newName)
{
   #if DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("Thruster::RenameRefObject() type=%s, oldName=%s, newName=%s\n"),
       GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif
   
   if (type != Gmat::HARDWARE && type != Gmat::COORDINATE_SYSTEM)
      return true;
   
   if (type == Gmat::COORDINATE_SYSTEM)
   {
      if (coordSystemName == oldName)
         coordSystemName = newName;
   }

   if (type == Gmat::HARDWARE)
   {
      for (UnsignedInt i=0; i<tankNames.size(); i++)
      {
         if (tankNames[i] == oldName)
         {
            tankNames[i] = newName;
            break;
         }
      }
   }
   
   return true;
}


//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name)
//------------------------------------------------------------------------------
/**
 * Sets referenced objects.
 *
 * @param obj The object.
 * @param type Type of the object.
 * @param name Name of the object.
 * 
 * @return true if the ref object was set, false if not.
 */
//------------------------------------------------------------------------------
bool Thruster::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                            const wxString &name)
{
   #ifdef DEBUG_THRUSTER_REF_OBJ
   MessageInterface::ShowMessage
      (wxT("Thruster::SetRefObject() <%p>'%s' entered, obj=<%p>, type=%d, name='%s'\n"),
       this, GetName().c_str(), obj, type, name.c_str());
   #endif
   
   if (obj == NULL)
      return false;
   
   if (type == Gmat::COORDINATE_SYSTEM && obj->GetType() == Gmat::COORDINATE_SYSTEM)
   {
      if (coordSystemName == name)
      {
         coordSystem = (CoordinateSystem*)obj;
         #ifdef DEBUG_THRUSTER_REF_OBJ
         MessageInterface::ShowMessage
            (wxT("Thruster::SetRefObject() leaving, the CoordinateSystem set to <%p>\n"),
             coordSystem);
         #endif
      }
      return true;
   }
   
   if (type == Gmat::CELESTIAL_BODY && obj->GetType() == Gmat::CELESTIAL_BODY &&
       localOriginName == name)
   {
      localOrigin = (CelestialBody*)obj;
      return true;
   }
   
   if (type == Gmat::CELESTIAL_BODY && obj->GetType() == Gmat::CELESTIAL_BODY &&
       j2000BodyName == name)
   {
      j2000Body = (CelestialBody*)obj;
      return true;
   }
   
   if (obj->GetType() == Gmat::SPACECRAFT)
   {
      return SetSpacecraft((Spacecraft*)obj);
   }
   
   if (obj->GetTypeName() == wxT("FuelTank"))
   {
      #ifdef DEBUG_THRUSTER_REF_OBJ
         MessageInterface::ShowMessage(wxT("   Setting tank \"%s\" on thruster \"%s\"\n"),
                                       name.c_str(), instanceName.c_str());
      #endif
      
      if (tanks.empty())
      {
         tanks.push_back((FuelTank*)obj);
         #ifdef DEBUG_THRUSTER_REF_OBJ
         MessageInterface::ShowMessage
            (wxT("   <%p>'%s' added to tank list, now size is %d\n"),
             obj, obj->GetName().c_str(), tanks.size());
         #endif
      }
      else if (find(tanks.begin(), tanks.end(), obj) == tanks.end())
      {
         // Replace old tank with new one. We don't want to delete the
         // old tank here since Spacecraft owns it (tank is not cloned in the
         // copy constructor)
         for (UnsignedInt i=0; i<tanks.size(); i++)
         {
            if (tanks[i]->GetName() == name)
            {
               #ifdef DEBUG_THRUSTER_REF_OBJ
               MessageInterface::ShowMessage
                  (wxT("   <%p>'%s' will replace the old tank <%p>'%s'\n"), obj,
                   obj->GetName().c_str(), tanks[i], tanks[i]->GetName().c_str());
               #endif
               tanks[i] = (FuelTank*)obj;
            }
         }
      }
      
      #ifdef DEBUG_THRUSTER
         // Peek at the mass flow data
         bool temp = thrusterFiring;
         thrusterFiring = true;
         CalculateMassFlow();
         thrusterFiring = temp;
      #endif
      
      #ifdef DEBUG_THRUSTER_REF_OBJ
      MessageInterface::ShowMessage
         (wxT("Thruster::SetRefObject() <%p>'%s' returning true\n"), this, GetName().c_str());
      #endif
      return true;
   }
   
   return Hardware::SetRefObject(obj, type, name);
}


//---------------------------------------------------------------------------
//  ObjectArray& GetRefObjectArray(const Gmat::ObjectType type)
//---------------------------------------------------------------------------
/**
 * Obtains an array of GmatBase pointers by type.
 *
 * @param type The type of objects requested
 *
 * @return Reference to the array.
 */
//---------------------------------------------------------------------------
ObjectArray& Thruster::GetRefObjectArray(const Gmat::ObjectType type)
{
   if (type == Gmat::HARDWARE)
   {
      tempArray.clear();
      for (UnsignedInt i = 0; i < tanks.size(); ++i)
         tempArray.push_back(tanks[i]);
      return tempArray;
   }

   return Hardware::GetRefObjectArray(type);
}


//---------------------------------------------------------------------------
//  ObjectArray& GetRefObjectArray(const wxString& typeString)
//---------------------------------------------------------------------------
/**
 * Obtains an array of GmatBase pointers based on a string (e.g. the typename).
 *
 * @param typeString The string used to find the objects requested.
 *
 * @return Reference to the array.  This default method returns an empty vector.
 */
//---------------------------------------------------------------------------
ObjectArray& Thruster::GetRefObjectArray(const wxString& typeString)
{
//   if ((typeString == wxT("FuelTank")) || (typeString == wxT("Tanks")))
//      return tanks;

   return Hardware::GetRefObjectArray(typeString);
}


//---------------------------------------------------------------------------
//  bool TakeAction(const wxString &action, const wxString &actionData)
//---------------------------------------------------------------------------
/**
 * Interface used to support user actions.
 *
 * Thrusters use this method to clear the pointers and names of tanks used for
 * a burn, prior to reassigning the tanks.
 *
 * @param <action> The string descriptor for the requested action.
 * @param <actionData> Optional data used for the action.
 *
 * @return true if the action was performed, false if not.
 */
//---------------------------------------------------------------------------
bool Thruster::TakeAction(const wxString &action,
                          const wxString &actionData)
{
   if (action == wxT("ClearTanks")) {
//      if (thrusterFiring)
//         throw HardwareException(wxT("Thruster ") + instanceName +
//            wxT(" is attempting to remove fuel tank access during a finite burn"));

      tankNames.clear();
      tanks.clear();

      return true;
   }

   return Hardware::TakeAction(action, actionData);
}


//------------------------------------------------------------------------------
//  void SetSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 * Sets the internal solar system pointer for objects that have one.
 *
 * @param ss   The solar system.
 */
//------------------------------------------------------------------------------
void Thruster::SetSolarSystem(SolarSystem *ss)
{
   #ifdef DEBUG_THRUSTER_REF_OBJ
   MessageInterface::ShowMessage
      (wxT("Thruster::SetSolarSystem() ss=<%p> '%s'\n"), ss, ss->GetName().c_str());
   #endif
   
   if (solarSystem != ss)
   {
      solarSystem = ss;
      
      if (usingLocalCoordSys)
      {
         localOrigin = solarSystem->GetBody(localOriginName);
         j2000Body = solarSystem->GetBody(j2000BodyName);
      }
   }
}


//------------------------------------------------------------------------------
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * Sets up the bodies used in the thrust calculations.
 */
//------------------------------------------------------------------------------
bool Thruster::Initialize()
{
   #ifdef DEBUG_THRUSTER_INIT
   MessageInterface::ShowMessage
      (wxT("Thruster::Initialize() <%p>'%s' entered, spacecraft=<%p>, ")
       wxT("usingLocalCoordSys=%d, coordSystem=<%p>, localCoordSystem=<%p>\n"),
       this, GetName().c_str(), spacecraft, usingLocalCoordSys,
       coordSystem, localCoordSystem);
   #endif
   
   bool retval = Hardware::Initialize();
   if (!retval)
      return false;
   
   if (!usingLocalCoordSys)
   {
      if (coordSystem == NULL)
         throw HardwareException
            (wxT("Thruster::Initialize() the Coordinate System \"") + coordSystemName +
             wxT("\" has not been set."));
      
      // If spacecraft is not set this time, just return true
      // This instance of Thruster may be just cloned one from the Sandbox
      if (spacecraft == NULL)
         return true;
   }
   else
   {
      if (solarSystem == NULL)
         throw HardwareException
            (wxT("Unable to initialize the Thruster object \"") + 
             instanceName + wxT("\"; ") + wxT("\"SolarSystem\" was not set."));
      
      if (!j2000Body)
         throw HardwareException
            (wxT("Unable to initialize the thruster object \"") + 
             instanceName + wxT("\"; \"") + j2000BodyName + wxT("\" was not set."));
      
      if (!localOrigin)
         throw HardwareException
            (wxT("Unable to initialize the thruster object \"") + 
             instanceName + wxT("\"; \"") + localOriginName + wxT("\" \" was not set."));
      
      // delete old local coordinate system
      if (localCoordSystem != NULL)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (localCoordSystem, wxT("localCS"), wxT("Thruster::Initialize()"),
             wxT("deleting localCoordSystem"));
         #endif
         delete localCoordSystem;
         localCoordSystem = NULL;
      }
      
      // If spacecraft is available, create new local coordinate system
      if (spacecraft != NULL)
         localCoordSystem = CreateLocalCoordinateSystem();
      
      // localCoordSystem is still NULL, just return
      if (localCoordSystem == NULL)
      {
         #ifdef DEBUG_THRUSTER_INIT
         MessageInterface::ShowMessage
            (wxT("Thruster::Initialize() <%p>'%s' returning false, localCoordSystem is NULL\n"),
             this, GetName().c_str());
         #endif
         return false;
      }

      #ifdef DEBUG_THRUSTER_CONVERT
         MessageInterface::ShowMessage
            (wxT("Local thruster coord system is\n%s\n"),localCoordSystem->
                  GetGeneratingString(Gmat::NO_COMMENTS).c_str());
      #endif

   }
   
   // Convert direction to inertial coord system
   Real epoch = spacecraft->GetRealParameter(wxT("A1Epoch"));
   ConvertDirectionToInertial(direction, inertialDirection, epoch);
   
   #ifdef DEBUG_THRUSTER_INIT
   MessageInterface::ShowMessage
      (wxT("   Inertial thrust direction:  %18le  %18le  %18le\n"),
       inertialDirection[0], inertialDirection[1], inertialDirection[2]);
   MessageInterface::ShowMessage
      (wxT("   %s tank mass computation\n"), decrementMass ? wxT("Continue with ") : wxT("Skipping"));
   #endif
   
   #ifdef DEBUG_THRUSTER_INIT
   MessageInterface::ShowMessage
      (wxT("Thruster::Initialize() <%p>'%s' returning %s\n"), this, GetName().c_str(),
       retval ? wxT("true") : wxT("false"));
   #endif
   
   return retval;
}


//---------------------------------------------------------------------------
//  bool CalculateThrustAndIsp()
//---------------------------------------------------------------------------
/**
 * Evaluates the thrust and specific impulse polynomials.
 *
 * GMAT uses polynomial expressions for the thrust and specific impulse
 * imparted to the spacecraft by thrusters attached to the spacecraft.
 * Both thrust and specific impulse are expressed as functions of pressure
 * and temperature. The pressure and temperature are values obtained
 * from fuel tanks containing the fuel. All measurements in GMAT are
 * expressed in metric units. The thrust, in Newtons, applied by a spacecraft
 * engine is given by
 *
   \f[
    F_{T}(P,T) = C_{1}+C_{2}P + \left\{ C_{3}+C_{4}P+C_{5}P^{2}+C_{6}P^{C_{7}}+
                   C_{8}P^{C_{9}}+C_{10}P^{C_{11}}+
                   C_{12}C_{13}^{C_{14}P}\right\} \left(\frac{T}{T_{ref}}
                   \right)^{1+C_{15}+C_{16}P}\f]
 *
 * Pressures are expressed in kilopascals, and temperatures in degrees
 * centigrade. The coefficients C1 - C16 are set by the user. Each coefficient
 * is expressed in units commiserate with the final expression in Newtons;
 * for example, C1 is expressed in Newtons, C2 in Newtons per kilopascal,
 * and so forth.
 *
 * Specific Impulse, measured in sec is expressed using a similar equation:
 *
   \f[
    I_{sp}(P,T) = K_{1}+K_{2}P + \left\{ K_{3}+K_{4}P+K_{5}P^{2}+K_{6}P^{K_{7}}+
                    K_{8}P^{K_{9}}+K_{10}P^{K_{11}}+K_{12}K_{13}^{K_{14}P}\right\}
         \left(\frac{T}{T_{ref}}\right)^{1+K_{15}+K_{16}P}\f]
 *
 * @return true on successful evaluation.
 */
//---------------------------------------------------------------------------
bool Thruster::CalculateThrustAndIsp()
{
   if (!thrusterFiring) 
   {
      thrust  = 0.0;
      impulse = 0.0;
   }
   else 
   {
      if (tanks.empty())
         throw HardwareException(wxT("Thruster \"") + instanceName +
                                 wxT("\" does not have a fuel tank"));

      // Require that the tanks all be at the same pressure and temperature
      Integer pressID = tanks[0]->GetParameterID(wxT("Pressure"));
      Integer tempID = tanks[0]->GetParameterID(wxT("Temperature"));
      Integer refTempID = tanks[0]->GetParameterID(wxT("RefTemperature"));

      pressure = tanks[0]->GetRealParameter(pressID);
      temperatureRatio = tanks[0]->GetRealParameter(tempID) /
                         tanks[0]->GetRealParameter(refTempID);

      thrust = cCoefficients[2];
      impulse = kCoefficients[2];

      if (!constantExpressions) 
      {

         thrust  += pressure*(cCoefficients[3] + pressure*cCoefficients[4]);
         impulse += pressure*(kCoefficients[3] + pressure*kCoefficients[4]);

         // For efficiency, if thrust and Isp are simple, don't bother evaluating
         // higher order terms
         if (!simpleExpressions) {
            thrust  += cCoefficients[5] * pow(pressure, cCoefficients[6]) +
                       cCoefficients[7] * pow(pressure, cCoefficients[8]) +
                       cCoefficients[9] * pow(pressure, cCoefficients[10]) +
                       cCoefficients[11] * pow(cCoefficients[12],
                                              pressure * cCoefficients[13]);

            impulse += kCoefficients[5] * pow(pressure, kCoefficients[6]) +
                       kCoefficients[7] * pow(pressure, kCoefficients[8]) +
                       kCoefficients[9] * pow(pressure, kCoefficients[10]) +
                       kCoefficients[11] * pow(kCoefficients[12],
                                              pressure * kCoefficients[13]);
         }
      }
      
//       thrust  *= pow(temperatureRatio, (1.0 + cCoefficients[14] +
//                      pressure*cCoefficients[15])) * thrustScaleFactor * dutyCycle;
      thrust  *= pow(temperatureRatio, (1.0 + cCoefficients[14] +
                     pressure*cCoefficients[15]));
      impulse *= pow(temperatureRatio, (1.0 + kCoefficients[14] +
                     pressure*kCoefficients[15]));
      
      // Now add the temperature independent pieces
      thrust  += cCoefficients[0] + cCoefficients[1] * pressure;
      impulse += kCoefficients[0] + kCoefficients[1] * pressure;
   }
   
   return true;
}


//---------------------------------------------------------------------------
//  Real CalculateMassFlow()
//---------------------------------------------------------------------------
/**
 * Evaluates the time rate of change of mass due to a thruster firing.
 *
 *  \f[\frac{dm}{dt} = \frac{F_T}{I_{sp}} \f]
 *
 * @return the mass flow rate from this thruster, used in integration.
 */
//---------------------------------------------------------------------------
Real Thruster::CalculateMassFlow()
{
   if (!thrusterFiring)
      return 0.0;
   else 
   {
      if (tanks.empty())
         throw HardwareException(wxT("Thruster \"") + instanceName +
                                 wxT("\" does not have a fuel tank"));

      // For now, always calculate T and I_sp
      //if (!constantExpressions) {
         if (!CalculateThrustAndIsp())
            throw HardwareException(wxT("Thruster \"") + instanceName +
                                    wxT("\" could not calculate dm/dt"));
         if (impulse == 0.0)
            throw HardwareException(wxT("Thruster \"") + instanceName +
                                    wxT("\" has specific impulse == 0.0"));

         // Mass flows out, so need a negative value here
         mDot = -thrust / (gravityAccel * impulse);

         // Old code:
         // mDot = -(thrust/thrustScaleFactor) / (gravityAccel * impulse);
      //}
   }

   #ifdef DEBUG_THRUSTER
      MessageInterface::ShowMessage(
            wxT("   Thrust = %15lf, Isp = %15lf, gravity accel = %lf, TSF = %lf, ")
            wxT("dutyCycle = %15lf, MassFlow = %15lf T/Isp =  %lf\n"),
            thrust, impulse, gravityAccel, thrustScaleFactor, dutyCycle, mDot, 
            thrust/impulse);
   #endif

   
   return mDot * dutyCycle;
}


//------------------------------------------------------------------------------
//  bool SetSpacecraft(Spacecraft *sat)
//------------------------------------------------------------------------------
/**
 * Accessor method to pass in the spacecraft pointer. This method is usually
 * called during the Sandbox initialization when building Spacecraft owned
 * objects such as tanks and thrusters. The spacecraft passes itself to this
 * class instance using Thruster::SetRefObject() in Spacecraft::SetRefObject().
 *
 * This method creates new local coordinate system if current spacecraft is not
 * the same as old spacecraft.
 * 
 * @param <sat> the Spacecraft
 */
//------------------------------------------------------------------------------
bool Thruster::SetSpacecraft(Spacecraft *sat)
{
   if (sat == NULL)
      return false;
   
   #ifdef DEBUG_THRUSTER_REF_OBJ
   MessageInterface::ShowMessage
      (wxT("Thruster::SetSpacecraft() sat=<%p>'%s', usingLocalCoordSys=%d, ")
       wxT("localCoordSystem=<%p>\n"), sat, sat->GetName().c_str(), usingLocalCoordSys,
       localCoordSystem);
   #endif
   
   // If spcecraft is different
   if (spacecraft != sat)
   {
      spacecraft = sat;
      satName = spacecraft->GetName();
      
      // create new local coordinate system
      if (usingLocalCoordSys)
      {
         if (localCoordSystem)
         {
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Remove
               (localCoordSystem, wxT("localCS"), wxT("Thruster::SetSpacecraft()"),
                wxT("deleting localCoordSystem"));
            #endif
            delete localCoordSystem;
            localCoordSystem = NULL;
         }
         localCoordSystem = CreateLocalCoordinateSystem();
         
         #ifdef DEBUG_THRUSTER_REF_OBJ
         MessageInterface::ShowMessage
            (wxT("   new localCoordSystem <%p> created\n"), localCoordSystem);
         #endif
      }
   }
   
   #ifdef DEBUG_THRUSTER_REF_OBJ
   MessageInterface::ShowMessage(wxT("Thruster::SetSpacecraft() returning true\n"));
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// CoordinateSystem* CreateLocalCoordinateSystem()
//------------------------------------------------------------------------------
CoordinateSystem* Thruster::CreateLocalCoordinateSystem()
{
   #ifdef DEBUG_THRUSTER_INIT
   MessageInterface::ShowMessage
      (wxT("Thruster::CreateLocalCoordinateSystem() <%p>'%s' entered, usingLocalCoordSys=%d, ")
       wxT("spacecraft=<%p>\n"), this, GetName().c_str(), usingLocalCoordSys, spacecraft);
   #endif
   
   CoordinateSystem *localCS = NULL;
   
   // If coordinate system being used is local, then create
   if (usingLocalCoordSys)
   {
      if (solarSystem == NULL)
      {
         // Since SolarSystem may be set later, just return NULL for now
         #ifdef DEBUG_THRUSTER_INIT
         MessageInterface::ShowMessage
            (wxT("Thruster::CreateLocalCoordinateSystem() SolarSystem is not set so, ")
             wxT("returning NULL\n"));
         #endif
         return NULL;
         
         //throw HardwareException
         //   (wxT("Unable to initialize the Thruster object ") + 
         //    instanceName + wxT(" ") + wxT("\"SolarSystem\" was not set for the thruster."));
      }
      
      if (spacecraft == NULL)
      {
         // Since spacecraft may be set later, just return NULL for now
         #ifdef DEBUG_THRUSTER_INIT
         MessageInterface::ShowMessage
            (wxT("Thruster::CreateLocalCoordinateSystem() spacecraft is not set so, ")
             wxT("returning NULL\n"));
         #endif
         return NULL;
         
         //throw HardwareException(wxT("Unable to initialize the Thruster object ") + 
         //   instanceName + wxT(" ") + satName + wxT(" was not set for the thruster."));
      }
            
      // Call CoordinateSystem static method to create a local coordinate system
      localOrigin = solarSystem->GetBody(localOriginName);
      localCS = CoordinateSystem::CreateLocalCoordinateSystem
         (wxT("Local"), localAxesName, spacecraft, localOrigin, spacecraft,
          j2000Body, solarSystem);
      
      if (localCS == NULL)
         return NULL;
      
      if (localAxesName == wxT("MJ2000Eq"))
         isMJ2000EqAxes = true;
      else if (localAxesName == wxT("SpacecraftBody"))
         isSpacecraftBodyAxes = true;
      
   }
   else
   {
      // If not using local cooordinate system, then it is using configured CS and
      // it should have been set by this time
      if (coordSystem == NULL)
      {
         throw HardwareException
            (wxT("Unable to initialize the Thruster object ") + 
             instanceName + wxT(" ") + coordSystemName + wxT(" was not set for the thruster."));
      }
      localCS = coordSystem;
   }
   
   #ifdef DEBUG_THRUSTER_INIT
   MessageInterface::ShowMessage
      (wxT("Thruster::CreateLocalCoordinateSystem() <%p>'%s' returning <%p>\n"),
       this, GetName().c_str(), localCS);
   #endif
   
   return localCS;
}


//------------------------------------------------------------------------------
// void ConvertDirectionToInertial(Real *dir, Real *dirInertial, Real epoch)
//------------------------------------------------------------------------------
/*
 * Converts thruster direction to inertial frame
 *
 * @param dir  Thruster direction in thruster frame
 * @param dirInertial  Thruster direction in inertial frame
 * @param epoch  Epoch to be used for conversion
 */
//------------------------------------------------------------------------------
void Thruster::ConvertDirectionToInertial(Real *dir, Real *dirInertial, Real epoch)
{
   #ifdef DEBUG_THRUSTER_CONVERT
   MessageInterface::ShowMessage
      (wxT("Thruster::ConvertDirectionToInertial() <%p>'%s' entered, epoch=%.15f\n   ")
       wxT("dir=%.15f %.15f %.15f\n"), this, GetName().c_str(), epoch, dir[0], dir[1], dir[1]);
   MessageInterface::ShowMessage
      (wxT("   usingLocalCoordSys=%s, coordSystemName='%s', coordSystem=<%p>, ")
       wxT("localCoordSystem=<%p>\n"), (usingLocalCoordSys?wxT("true"):wxT("false")),
       coordSystemName.c_str(), coordSystem, localCoordSystem);
   #endif
   
   if (usingLocalCoordSys && localCoordSystem == NULL)
   {      
      // try creating localCoordSystem
      //localCoordSystem = CreateLocalCoordinateSystem();
      // try Initialize again (LOJ: 2009.07.28)
      Initialize();
      if (!localCoordSystem)
         throw HardwareException
            (wxT("Unable to convert thrust direction to Inertial, the local Coordinate ")
             wxT("System has not been created."));
   }
   else if (!usingLocalCoordSys && coordSystem == NULL)
   {
      throw HardwareException
         (wxT("Unable to convert thrust direction to Inertial, the Coordinate ")
          wxT("System \"") + coordSystemName + wxT("\" has not been set."));      
   }
   
   Real inDir[6], outDir[6];
   for (Integer i=0; i<3; i++)
      inDir[i] = dir[i];
   for (Integer i=3; i<6; i++)
      inDir[i] = 0.0;
   
   // if not using local CS, use ref CoordinateSystem
   if (!usingLocalCoordSys)
   {     
      // Now rotate to MJ2000Eq axes, we don't want to translate so
      // set coincident to true
      coordSystem->ToMJ2000Eq(epoch, inDir, outDir, true);
      
      #ifdef DEBUG_BURN_CONVERT_ROTMAT
      Rmatrix33 rotMat = coordSystem->GetLastRotationMatrix();
      MessageInterface::ShowMessage
         (wxT("rotMat=\n%s\n"), rotMat.ToString(16, 20).c_str());
      #endif
      
      dirInertial[0] = outDir[0];
      dirInertial[1] = outDir[1];
      dirInertial[2] = outDir[2];
   }
   else
   {
      // if MJ2000Eq axes rotation matrix is always identity matrix
      if (isMJ2000EqAxes)
      {

         dirInertial[0] = dir[0];
         dirInertial[1] = dir[1];
         dirInertial[2] = dir[2];
      }
      else if (isSpacecraftBodyAxes)
      {
         Rvector3 inDir(dir[0], dir[1], dir[2]);
         Rvector3 outDir;
         // Get attitude matrix from Spacecraft and transpose since
         // attitude matrix from spacecraft gives rotation matrix from
         // inertial to body
         Rmatrix33 inertialToBody = spacecraft->GetAttitude(epoch);
         Rmatrix33 rotMat = inertialToBody.Transpose();
         outDir = inDir * rotMat;
         for (Integer i=0; i<3; i++)
            dirInertial[i] = outDir[i];
      }
      else
      {         
         // Now rotate to MJ2000Eq axes
         localCoordSystem->ToMJ2000Eq(epoch, inDir, outDir, true);
         
         dirInertial[0] = outDir[0];
         dirInertial[1] = outDir[1];
         dirInertial[2] = outDir[2];
      }
   }
   
   #ifdef DEBUG_THRUSTER_CONVERT
   MessageInterface::ShowMessage
      (wxT("Thruster::ConvertDirectionToInertial() returning\n   ")
       wxT("  direction = %.15f %.15f %.15f\n   ")
       wxT("dirInertial = %.15f %.15f %.15f\n"),
       direction[0], direction[1], direction[2],
       dirInertial[0], dirInertial[1], dirInertial[2]);
   #endif
}


//---------------------------------------------------------------------------
// void ComputeInertialDirection(Real epoch)
//---------------------------------------------------------------------------
void Thruster::ComputeInertialDirection(Real epoch)
{
   ConvertDirectionToInertial(direction, inertialDirection, epoch);
}


//------------------------------------------------------------------------------
// void WriteDeprecatedMessage(const wxString &oldProp,
//                             const wxString &newProp) const
//------------------------------------------------------------------------------
void Thruster::WriteDeprecatedMessage(const wxString &oldProp,
                                      const wxString &newProp) const
{
   // Write only one message per session
   static bool writeXDirection = true;
   static bool writeYDirection = true;
   static bool writeZDirection = true;
   static bool writeElement1 = true;
   static bool writeElement2 = true;
   static bool writeElement3 = true;
   
   if ((oldProp == wxT("X_Direction") && writeXDirection) ||
       (oldProp == wxT("Y_Direction") && writeYDirection) ||
       (oldProp == wxT("Z_Direction") && writeZDirection) ||
       (oldProp == wxT("Element1") && writeElement1) ||
       (oldProp == wxT("Element2") && writeElement2) ||
       (oldProp == wxT("Element3") && writeElement3))
   {
      MessageInterface::ShowMessage
         (wxT("*** WARNING *** \"") + oldProp + wxT("\" of Thruster orientation is ")
          wxT("deprecated and will be removed from a future build; please use \"") +
          newProp + wxT("\" instead.\n"));
   }
   
   if (oldProp == wxT("X_Direction"))
      writeXDirection = false;
   else if (oldProp == wxT("Y_Direction"))
      writeYDirection = false;
   else if (oldProp == wxT("Z_Direction"))
      writeYDirection = false;
   else if (oldProp == wxT("Element1"))
      writeElement1 = false;
   else if (oldProp == wxT("Element2"))
      writeElement2 = false;
   else if (oldProp == wxT("Element3"))
      writeElement3 = false;
   
}

