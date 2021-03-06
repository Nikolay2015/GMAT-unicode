//$Id: HarmonicField.cpp 9700 2011-07-14 17:30:41Z wendys-dev $
//------------------------------------------------------------------------------
//                              HarmonicField
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: D. Conway
// Created: 2003/03/14
// Modified: 2004/04/15 W. Shoan GSFC Code 583
//           Updated for GMAT style, standards; changed quad-t to Real, int
//           to Integer, add use of GMAT util classes, etc.
///
/**
 * This is the HarmonicField class.
 *
 * @note original prolog information included at end of file prolog.
 */
//
// *** File Name : shfieldmodel.cpp  -> HarmonicField.cpp (wcs 2004.04.14)
// *** Created   : March 14, 2003
// **************************************************************************
// ***  Developed By  :  Thinking Systems, Inc. (www.thinksysinc.com)     ***
// ***  For:  Flight Dynamics Analysis Branch (Code 572)                  ***
// ***  Under Contract:  P.O.  GSFC S-67521-G                             ***
// ***                                                                    ***
// ***  Copyright U.S. Government 2002                                    ***
// ***  Copyright United States Government as represented by the          ***
// ***  Administrator of the National Aeronautics and Space               ***
// ***  Administration                                                    ***
// ***                                                                    ***
// ***  This software is subject to the Sofware Usage Agreement described ***
// ***  by NASA Case Number GSC-14735-1.  The Softare Usage Agreement     ***
// ***  must be included in any distribution.  Removal of this header is  ***
// ***  strictly prohibited.                                              ***
// ***                                                                    ***
// ***                                                                    ***
// ***  Header Version: July 12, 2002                                     ***
// **************************************************************************
// Module Type               : ANSI C/C++ Source
// Development Environment   : Borland C++ 5.02
// Modification History      : 3/14/2003 - D. Conway, Thinking Systems, Inc.
//                             Original delivery
// **************************************************************************

#include "HarmonicField.hpp"

#include "PhysicalModel.hpp"
#include "ODEModelException.hpp"
#include "StringUtil.hpp"     // for ToString()
#include "RealUtilities.hpp"
#include "TimeTypes.hpp"
#include "Rvector.hpp"
#include "Rmatrix.hpp"
#include "FileManager.hpp"
#include "MessageInterface.hpp"

/*
#include <iostream>
 
#define DEBUG_REFERENCE_SETTING
*/

using namespace GmatMathUtil;

//#define DEBUG_HARMONIC_FIELD
//#define DEBUG_HARMONIC_FIELD_FILENAME

//---------------------------------
// static data
//---------------------------------
const wxString
HarmonicField::PARAMETER_TEXT[HarmonicFieldParamCount - PhysicalModelParamCount] =
{
   wxT("MaxDegree"),
   wxT("MaxOrder"),
   wxT("Degree"),
   wxT("Order"),
   wxT("PotentialFile"),
   wxT("InputCoordinateSystem"),
   wxT("FixedCoordinateSystem"),
   wxT("TargetCoordinateSystem"),
};

const Gmat::ParameterType
HarmonicField::PARAMETER_TYPE[HarmonicFieldParamCount - PhysicalModelParamCount] =
{
   Gmat::INTEGER_TYPE,   // wxT("MaxDegree"),
   Gmat::INTEGER_TYPE,   // wxT("MaxOrder"),
   Gmat::INTEGER_TYPE,   // wxT("Degree"),
   Gmat::INTEGER_TYPE,   // wxT("Order"),
   Gmat::STRING_TYPE,    // wxT("PotentialFile"),
   Gmat::STRING_TYPE,    // wxT("InputCoordinateSystem"),
   Gmat::STRING_TYPE,    // wxT("FixedCoordinateSystem"),
   Gmat::STRING_TYPE,    // wxT("TargetCoordinateSystem"),
};


//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  HarmonicField(const wxString &name, Integer deg, Integer ord)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the HarmonicField class
 * (default constructor).
 *
 * @param <name> parameter indicating the name of the object.
 * @param <deg>  maximum degree of he polynomials.
 * @param <ord>  maximum order of he polynomials.
 */
//------------------------------------------------------------------------------
HarmonicField::HarmonicField(const wxString &name, const wxString &typeName,
                             Integer maxDeg, Integer maxOrd) :
PhysicalModel           (Gmat::PHYSICAL_MODEL, typeName, name),
hMinitialized           (false),
maxDegree               (maxDeg),
maxOrder                (maxOrd),
degree                  (4),
order                   (4),
filename                (wxT("")),
fileRead                (false),
usingDefaultFile        (false),
isFirstTimeDefault      (true),
inputCSName             (wxT("EarthMJ2000Eq")),
fixedCSName             (wxT("EarthFixed")),
targetCSName            (wxT("EarthMJ2000Eq")),
potPath                 (wxT("")),
inputCS                 (NULL),
fixedCS                 (NULL),
targetCS                (NULL),
eop                     (NULL)
{
   objectTypeNames.push_back(wxT("HarmonicField"));
   parameterCount = HarmonicFieldParamCount;
   r = s = t = u = 0.0;
   
   FileManager *fm = FileManager::Instance();
   potPath = fm->GetAbsPathname(bodyName + wxT("_POT_PATH"));
   
}


//------------------------------------------------------------------------------
//  ~HarmonicField()
//------------------------------------------------------------------------------
/**
 * This method destroys the HarmonicField object.
 * (destructor).
 *
 */
//------------------------------------------------------------------------------
HarmonicField::~HarmonicField(void)
{
}


//------------------------------------------------------------------------------
//  HarmonicField(const HarmonicField& hf)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the HarmonicField class with the state equal
 * to the input object (copy constructor).
 *
 * @param <hf> the HarmonicField object to copy.
 */
//------------------------------------------------------------------------------
HarmonicField::HarmonicField(const HarmonicField& hf) :
PhysicalModel           (hf),
hMinitialized           (false),
maxDegree               (hf.maxDegree),
maxOrder                (hf.maxOrder),
degree                  (hf.degree),
order                   (hf.order),
r                       (0.0),
s                       (0.0),
t                       (0.0),
u                       (0.0),
filename                (hf.filename),
fileRead                (false),
usingDefaultFile        (hf.usingDefaultFile),
isFirstTimeDefault      (hf.isFirstTimeDefault),
inputCSName             (hf.inputCSName),
fixedCSName             (hf.fixedCSName),
targetCSName            (hf.targetCSName),
potPath                 (hf.potPath),
inputCS                 (NULL),
fixedCS                 (NULL),
targetCS                (NULL),
eop                     (hf.eop)
{
}


//------------------------------------------------------------------------------
//  operator=(const HarmonicField& hf)
//------------------------------------------------------------------------------
/**
 * This method sets the state of the input HarmonicField class to 'this' object.
 *
 * @param <hf> the HarmonicField object whose values to use.
 */
//------------------------------------------------------------------------------
HarmonicField& HarmonicField::operator=(const HarmonicField& hf)
{
   if (&hf == this)
      return *this;

   PhysicalModel::operator=(hf);
   hMinitialized  = false;  // or hf.hMinitialized?
   maxDegree      = hf.maxDegree;
   maxOrder       = hf.maxOrder;
   degree         = hf.degree;
   order          = hf.order;
   r              = 0.0;     // or hf.r?
   s              = 0.0;     // or hf.s?
   t              = 0.0;     // or hf.t?
   u              = 0.0;     // or hf.u?
   filename       = hf.filename;
   fileRead       = false;
   usingDefaultFile   = hf.usingDefaultFile;
   isFirstTimeDefault = hf.isFirstTimeDefault;
   inputCSName    = hf.inputCSName;
   fixedCSName    = hf.fixedCSName;
   targetCSName   = hf.targetCSName;
   potPath        = hf.potPath;
   inputCS        = hf.inputCS;
   fixedCS        = hf.fixedCS;
   targetCS       = hf.targetCS;
   eop            = hf.eop;
   return *this;
}

//------------------------------------------------------------------------------
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * This method initializes this HarmonicField object.
 *
 * @return flag indicating success of initialization.
 */
//------------------------------------------------------------------------------
bool HarmonicField::Initialize()
{
   if (!PhysicalModel::Initialize())
      return false;

   // Set body to use the same potential file (loj: 3/24/06)
   // if we want to use mu and radius from this file later
   //body->SetPotentialFilename(filename);
   
   if (solarSystem == NULL) throw ODEModelException(
                            wxT("Solar System undefined for Harmonic Field ") 
                             + instanceName);
   if (!inputCS) throw ODEModelException(
                 wxT("Input coordinate system undefined for Harmonic Field ")
                  + instanceName);
   if (!fixedCS) throw ODEModelException(
                 wxT("Body fixed coordinate system undefined for Harmonic Field ")
                  + instanceName);
   if (!targetCS) targetCS = inputCS;

   if (!eop) throw ODEModelException(
             wxT("EOP file is undefined for Harmonic Field ") + instanceName);

   hMinitialized = true;
   if (usingDefaultFile && isFirstTimeDefault)
   {
      MessageInterface::ShowMessage(wxT("Using default potential file \"%s\" for GravityField object \"%s\"\n"),
            filename.c_str(), instanceName.c_str());
      isFirstTimeDefault = false;
   }
   return true;
}


//------------------------------------------------------------------------------
//  bool SetDegreeOrder(Integer deg, Integer ord)
//------------------------------------------------------------------------------
/**
 * This method sets the degree and order for this HarmonicField object.
 *
 * @param <deg> degree to use.
 * @param <ord> order to use.
 *
 * @return flag indicating success of the operation.
 */
//------------------------------------------------------------------------------
bool HarmonicField::SetDegreeOrder(Integer deg, Integer ord)
{
    bool retval = true;

    if (deg <= maxDegree)
        degree = deg;
    else
    {
        degree = maxDegree;
        retval = false;
        MessageInterface::ShowMessage(
          wxT("In HarmonicField, Potential Degree exceeds maximum degree in model"));
    }

    if ((ord <= deg) && (ord <= maxOrder))
        order = ord;
    else
    {
        order = (deg < maxOrder ? deg : maxOrder );
        retval = false;
        MessageInterface::ShowMessage(
          wxT("In HarmonicField, Potential Order exceeds valid range in model"));
    }

    return retval;
}


//------------------------------------------------------------------------------
//  bool SetFilename(const wxString &fn)
//------------------------------------------------------------------------------
/**
 * This method sets the filename for this HarmonicField object.
 *
 * @param <fn> file name to use to get coefficients, etc.
 *
 * @return flag indicating success of the operation.
 */
//------------------------------------------------------------------------------
bool HarmonicField::SetFilename(const wxString &fn)
{
   #ifdef DEBUG_HARMONIC_FIELD_FILENAME
   MessageInterface::ShowMessage
      (wxT("HarmonicField::SetFilename() for %s\n   filename = %s\n   newname  = %s\n"),
       bodyName.c_str(), filename.c_str(), fn.c_str());
   MessageInterface::ShowMessage(wxT("   potPath  = %s\n"), potPath.c_str());
   #endif
   
   bool hasDefaultIndicator = false;
   wxString newfn;
   if (fn.substr(0, 6) == wxT("DFLT__"))  // Must match Interpreter static const wxString defaultIndicator
   {
      newfn               = fn.substr(6,fn.npos-6);
      hasDefaultIndicator = true;
   }
   else
   {
      newfn               = fn;
      hasDefaultIndicator = false;
   }
   if (filename != newfn)
   {
      #ifdef DEBUG_HARMONIC_FIELD_FILENAME
      MessageInterface::ShowMessage
         (wxT("HarmonicField::SetFilename(): hasDefaultIndicator = %s, newfn = %s\n"),
          (hasDefaultIndicator? wxT("true") : wxT("false")), newfn.c_str());
      MessageInterface::ShowMessage(wxT("   potPath  = %s\n"), potPath.c_str());
      #endif
      // Add default pathname if none specified
      if (newfn.find(wxT("/")) == newfn.npos && newfn.find(wxT("\\")) == newfn.npos)
      {
         try
         {
            filename = potPath + newfn;
         }
         catch (GmatBaseException &e)
         {
            filename = newfn;
            MessageInterface::ShowMessage(e.GetFullMessage());
         }
      }
      else
      {
         filename = newfn;
      }
      
      std::ifstream potfile(filename.char_str());
      if (!potfile) 
      {
         throw ODEModelException
            (wxT("The file name \"") + filename + wxT("\" does not exist."));
      }
      
      if (body != NULL)
         body->SetPotentialFilename(filename);
   }
   
   #ifdef DEBUG_HARMONIC_FIELD_FILENAME
   MessageInterface::ShowMessage(wxT("   filename = %s\n"), filename.c_str());
   #endif
   
   fileRead = false;
   usingDefaultFile = hasDefaultIndicator;
   return true;
}


//---------------------------------
// inherited methods from GmatBase
//---------------------------------
//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
* @see GmatBase
 */
//------------------------------------------------------------------------------
wxString HarmonicField::GetParameterText(const Integer id) const
{
   if ((id >= PhysicalModelParamCount) && (id < HarmonicFieldParamCount))
      return PARAMETER_TEXT[id - PhysicalModelParamCount];
   return PhysicalModel::GetParameterText(id);
}

//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
* @see GmatBase
 */
//------------------------------------------------------------------------------
Integer HarmonicField::GetParameterID(const wxString &str) const
{
   wxString useStr = str;
   if (useStr == wxT("Model"))
      useStr = wxT("PotentialFile");
 
   for (Integer i = PhysicalModelParamCount; i < HarmonicFieldParamCount; i++)
   {
      if (useStr == PARAMETER_TEXT[i - PhysicalModelParamCount])
         return i;
   }
   return PhysicalModel::GetParameterID(str);
}

//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
* @see GmatBase
 */
//------------------------------------------------------------------------------
Gmat::ParameterType HarmonicField::GetParameterType(const Integer id) const
{
   if ((id >= PhysicalModelParamCount) && (id < HarmonicFieldParamCount))
      return PARAMETER_TYPE[id - PhysicalModelParamCount];
   return PhysicalModel::GetParameterType(id);
}

//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
* @see GmatBase
 */
//------------------------------------------------------------------------------
wxString HarmonicField::GetParameterTypeString(const Integer id) const
{
   return GmatBase::PARAM_TYPE_STRING[GetParameterType(id)];   
}

//------------------------------------------------------------------------------
// Integer GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Accessor method used to obtain a parameter value
 *
 * @param <id>    Integer ID for the requested parameter
 */
//------------------------------------------------------------------------------
Integer HarmonicField::GetIntegerParameter(const Integer id) const
{
   if (id == MAX_DEGREE)  return maxDegree;
   if (id == MAX_ORDER)   return maxOrder;
   if (id == DEGREE)      return degree;
   if (id == ORDER)       return order;

   return PhysicalModel::GetIntegerParameter(id);
}


//------------------------------------------------------------------------------
// Integer SetIntegerParameter(const Integer id, const Integer value)
//------------------------------------------------------------------------------
/**
 * Accessor method used to set a parameter value
 *
 * @param    <id>    Integer ID for the parameter
 * @param    <value> The new value for the parameter
 */
//------------------------------------------------------------------------------
Integer HarmonicField::SetIntegerParameter(const Integer id, const Integer value)
{
   if (id == MAX_DEGREE)  return (maxDegree = value);
   if (id == MAX_ORDER)   return (maxOrder  = value);
   if (id == DEGREE)
   {
      if (value >= 0)
         degree = value;
      else
      {
         wxString buffer;
         buffer << value;
         throw ODEModelException(
            wxT("The value of \"") + buffer + wxT("\" for field \"") 
            + GetParameterText(id).c_str() + wxT("\" on object \"") 
            + instanceName + wxT("\" is not an allowed value.\n")
            wxT("The allowed values are: [Integer >= 0 ")
            wxT("and < the maximum specified by the model, Order <= Degree]."));
      }
      return degree;
   }
   if (id == ORDER)
   {
      if (value >= 0)
         order = value;
      else
      {
         wxString buffer;
         buffer << value;
         throw ODEModelException(
            wxT("The value of \"") + buffer + wxT("\" for field \"") 
            + GetParameterText(id).c_str() + wxT("\" on object \"") 
            + instanceName + wxT("\" is not an allowed value.\n")
            wxT("The allowed values are: [Integer >= 0 ")
            wxT("and < the maximum specified by the model, Order <= Degree]."));
      }
      return order;
   }

   return PhysicalModel::SetIntegerParameter(id, value);
}


//------------------------------------------------------------------------------
// Integer GetIntegerParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Accessor method used to get a parameter value
 *
 * @param    <label>  parameter label
 *
 * @return the value of the parameter
 */
//------------------------------------------------------------------------------
Integer HarmonicField::GetIntegerParameter(const wxString &label) const
{
   return GetIntegerParameter(GetParameterID(label));
}

//------------------------------------------------------------------------------
// Integer SetIntegerParameter(const wxString &label, const Integer value)
//------------------------------------------------------------------------------
/**
 * Accessor method used to get a parameter value
 *
 * @param    <label> Integer ID for the parameter
 * @param    <value> The new value for the parameter
 */
//------------------------------------------------------------------------------
Integer HarmonicField::SetIntegerParameter(const wxString &label,
                                           const Integer value)
{
   return SetIntegerParameter(GetParameterID(label), value);
}

//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Accessor method used to get a parameter value
 *
 * @param    <id>  Integer ID for the parameter
 *
 * @return the value of the parameter
 */
//------------------------------------------------------------------------------
wxString HarmonicField::GetStringParameter(const Integer id) const
{
   if (id == FILENAME)
   {
      //return filename;
      wxString::size_type index = filename.find_last_of(wxT("/\\"));
      
      // if path not found, just write filename
      if (index == filename.npos)
         return filename;
      else
      {
         // if actual pathname is the same as the default path, write only name part
         wxString actualPath = filename.substr(0, index+1);
         wxString fname = filename;
         if (potPath == actualPath)
            fname = filename.substr(index+1);
         
         return fname;
      }
   }
   
   if (id == INPUT_COORD_SYSTEM)  return inputCSName;
   if (id == FIXED_COORD_SYSTEM)  return fixedCSName;
   if (id == TARGET_COORD_SYSTEM) return targetCSName;
   return PhysicalModel::GetStringParameter(id);
}

//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
/**
 * Accessor method used to get a parameter value
 *
 * @param    <id>    Integer ID for the parameter
 * @param    <value> The new value for the parameter
 */
//------------------------------------------------------------------------------
bool HarmonicField::SetStringParameter(const Integer id,
                                       const wxString &value)
{
   if (id == FILENAME)
   {
      wxString newValue = value;
      // if value has no file extension, add .cof as default (loj: 2008.10.14)
      if (value.find(wxT(".")) == value.npos)
         newValue = value + wxT(".cof");
      
      return SetFilename(newValue);  
   }
   if (id == INPUT_COORD_SYSTEM)
   {
      #ifdef DEBUG_HARMONIC_FIELD
         
         MessageInterface::ShowMessage(
                wxT("Setting input coordinate system name to \"%s\"\n"), value.c_str());
      #endif
      inputCSName = value;
      
      return true;
   }
   if (id == FIXED_COORD_SYSTEM)
   {
      #ifdef DEBUG_HARMONIC_FIELD
         
         MessageInterface::ShowMessage(
                wxT("Setting fixed coordinate system name to \"%s\"\n"), value.c_str());
      #endif
      fixedCSName = value;
      
      return true;
   }
   if (id == TARGET_COORD_SYSTEM)
   {
      #ifdef DEBUG_HARMONIC_FIELD
         
         MessageInterface::ShowMessage(
                wxT("Setting target coordinate system name to \"%s\"\n"), value.c_str());
      #endif
      targetCSName = value;
      
      return true;
   }
   if (id == BODY_NAME)
   {      
      if (PhysicalModel::SetStringParameter(id, value))
      {
         // set default potential file path for the body
         FileManager *fm = FileManager::Instance();
         try
         {
            potPath = fm->GetAbsPathname(bodyName + wxT("_POT_PATH"));
         }
         catch (BaseException &ex)
         {
            MessageInterface::ShowMessage(wxT("**** WARNING ****: %s\n"),
                  ex.GetFullMessage().c_str());
         }
         
         #ifdef DEBUG_HARMONIC_FIELD
         MessageInterface::ShowMessage
            (wxT("Setting potential file path to \"%s\"\n"), potPath.c_str());
         #endif
         
         fixedCSName = value + wxT("Fixed");
         return true;
      }
      return false;
   }
   
   return PhysicalModel::SetStringParameter(id, value);
}

//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Accessor method used to get a parameter value
 *
 * @param    <label>  Integer ID for the parameter
 *
 * @return the value of the parameter
 */
//------------------------------------------------------------------------------
wxString HarmonicField::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}

//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
/**
 * Accessor method used to get a parameter value
 *
 * @param    <label> Integer ID for the parameter
 * @param    <value> The new value for the parameter
 */
//------------------------------------------------------------------------------
bool HarmonicField::SetStringParameter(const wxString &label,
                                       const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
//  GmatBase* GetRefObject(const Gmat::ObjectType type,
//                         const wxString &name)
//------------------------------------------------------------------------------
/**
 * This method returns a reference object from the HarmonicField class.
 *
 * @param type  type of the reference object requested
 * @param name  name of the reference object requested
 *
 * @return pointer to the reference object requested.
 *
 */
//------------------------------------------------------------------------------
GmatBase* HarmonicField::GetRefObject(const Gmat::ObjectType type,
                                      const wxString &name)
{
   switch (type)
   {
      case Gmat::COORDINATE_SYSTEM:
         if ((inputCS) && (name == inputCSName))
            return inputCS;
         if ((fixedCS) && (name == fixedCSName))
            return fixedCS;
         if ((targetCS) && (name == targetCSName))
            return targetCS;
         break;

      default:
         break;
   }
   
   // Not handled here -- invoke the next higher GetRefObject call
   return PhysicalModel::GetRefObject(type, name);
}


//------------------------------------------------------------------------------
//  const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Returns the names of the reference object. (Derived classes should implement
 * this as needed.)
 *
 * @param <type> reference object type.  Gmat::UnknownObject returns all of the
 *               ref objects.
 *
 * @return The names of the reference object.
 */
//------------------------------------------------------------------------------
const StringArray& HarmonicField::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   static StringArray refs;
   
   // Start from an empty list
   refs.clear();
   
   if (type == Gmat::UNKNOWN_OBJECT)
   {
      refs = PhysicalModel::GetRefObjectNameArray(type);
      //refs.clear(); // why would I want to clear this????
      
      refs.push_back(inputCSName);
      refs.push_back(fixedCSName);
      refs.push_back(targetCSName);
      
      #ifdef DEBUG_REFERENCE_SETTING
         MessageInterface::ShowMessage(wxT("+++ReferenceObjects:\n"));
         for (StringArray::iterator i = refs.begin(); i != refs.end(); ++i)
            MessageInterface::ShowMessage(wxT("   %s\n"), i->c_str());
      #endif
      
      return refs;
   }
   
   if (type == Gmat::COORDINATE_SYSTEM)
   {
      refs = PhysicalModel::GetRefObjectNameArray(type);
      
      refs.push_back(inputCSName);
      refs.push_back(fixedCSName);
      refs.push_back(targetCSName);
      
      #ifdef DEBUG_REFERENCE_SETTING
         MessageInterface::ShowMessage(wxT("+++ReferenceObjects:\n"));
         for (StringArray::iterator i = refs.begin(); i != refs.end(); ++i)
            MessageInterface::ShowMessage(wxT("   %s\n"), i->c_str());
      #endif
      
      return refs;
   }
   
   // Not handled here -- invoke the next higher GetRefObject call
   return PhysicalModel::GetRefObjectNameArray(type);
}


//------------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                    const wxString &name)
//------------------------------------------------------------------------------
/**
 * This method sets a reference object for the HarmonicField class.
 *
 * @param obj   pointer to the reference object
 * @param type  type of the reference object
 * @param name  name of the reference object
 *
 * @return true if successful; otherwise, false.
 *
 */
//------------------------------------------------------------------------------
bool HarmonicField::SetRefObject(GmatBase *obj,
                                 const Gmat::ObjectType type,
                                 const wxString &name)
{
   if (obj->IsOfType(Gmat::COORDINATE_SYSTEM))
   {
      if (name == inputCSName)
      {
         #ifdef DEBUG_REFERENCE_SETTING
            MessageInterface::ShowMessage(wxT("Setting %s as input CS for %s\n"),
                                          name.c_str(), instanceName.c_str());
         #endif
         inputCS = (CoordinateSystem*) obj;
      }
      if (name == fixedCSName)
      {
         #ifdef DEBUG_REFERENCE_SETTING
            MessageInterface::ShowMessage(wxT("Setting %s as body fixed CS for %s\n"),
                                          name.c_str(), instanceName.c_str());
         #endif
         fixedCS = (CoordinateSystem*) obj;
      }
      if (name == targetCSName)
      {
         #ifdef DEBUG_REFERENCE_SETTING
            MessageInterface::ShowMessage(wxT("Setting %s as target CS for %s\n"),
                                          name.c_str(), instanceName.c_str());
         #endif
         targetCS = (CoordinateSystem*) obj;
      }
      return true;
   }
   
   // Not handled here -- invoke the next higher SetRefObject call
   return PhysicalModel::SetRefObject(obj, type, name);
}


//------------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                    const wxString &name)
//------------------------------------------------------------------------------
/**
 * This method sets the force model origin, and derived the base coordinate 
 * system names from that SpacePoint.
 *
 * @param toBody  pointer to the force model origin.
 */
//------------------------------------------------------------------------------
void HarmonicField::SetForceOrigin(CelestialBody* toBody)
{
   PhysicalModel::SetForceOrigin(toBody);
   
   wxString originName = toBody->GetName();
   inputCSName = originName + wxT("MJ2000Eq");
   targetCSName = inputCSName;
}



//------------------------------------------------------------------------------
//  void SetEopFile(EopFile *eopF)
//------------------------------------------------------------------------------
/**
 * Method to set EopFile for this system.
 *
 * @param eopF  the eopFile.
 *
 */
//------------------------------------------------------------------------------
void HarmonicField::SetEopFile(EopFile *eopF)
{
   eop = eopF;
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
bool HarmonicField::IsParameterReadOnly(const Integer id) const
{
   if (id < PhysicalModelParamCount)
      return PhysicalModel::IsParameterReadOnly(id);
   
   if (id >= HarmonicFieldParamCount)
      throw ODEModelException(
         wxT("Attempting to determine accessibility of a parameter outside of the ")
         wxT("scope of a HarmonicField object."));
   
   if ((id == DEGREE) || (id == ORDER) || (id == FILENAME))
      return false;
   
   return true;
}

//---------------------------------
// protected methods
//---------------------------------
// N/A
