//$Id: GmatBase.cpp 10025 2011-11-29 23:31:31Z djcinsb $
//------------------------------------------------------------------------------
//                                  GmatBase
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
// Author: Darrel J. Conway
// Created: 2003/09/25
//
/**
 * Implementation for the base class for all GMAT extensible objects
 *
 * The following class hierarchy trees use this class as their basis:
 *
 *     Asset (hence Spacecraft and GroundStation)
 *     CelestialBody
 *     Propagator
 *     PhysicalModel (hence Force and ForceModel)
 *     PropConfig
 *     Parameter
 *     Command
 *
 * Every class that users can use to derive new classes, or that need to be
 * accessed through generic pointers, should be derived from this class to
 * ensure consistent interfaces accessed by the GMAT control systems (i.e. the
 * Moderator, FactoryManager, Configuration, Interpreter, and Sandbox, along
 * with the GUIInterpreter).
 */
//------------------------------------------------------------------------------


#include "GmatBase.hpp"
#include "GmatGlobal.hpp"  // for GetDataPrecision()
#include <sstream>         // for StringStream

#include "MessageInterface.hpp"

//#define DEBUG_OBJECT_TYPE_CHECKING
//#define DEBUG_COMMENTS
//#define DEBUG_COMMENTS_ATTRIBUTE
//#define DEBUG_GENERATING_STRING
//#define DEBUG_OWNED_OBJECT_STRINGS
//#define DEBUG_WRITE_PARAM
//#define DEBUG_CLOAKING

/// Set the static wxT("undefined") parameters
//const Real        GmatBase::REAL_PARAMETER_UNDEFINED = -987654321.0123e-45;
const Real        GmatBase::REAL_PARAMETER_UNDEFINED = GmatRealConstants::REAL_UNDEFINED;
const Integer     GmatBase::INTEGER_PARAMETER_UNDEFINED = -987654321;
const UnsignedInt GmatBase::UNSIGNED_INT_PARAMETER_UNDEFINED = 987654321;
const wxString GmatBase::STRING_PARAMETER_UNDEFINED = wxT("STRING_PARAMETER_UNDEFINED");
const StringArray       GmatBase::STRINGARRAY_PARAMETER_UNDEFINED = StringArray(1,
                        GmatBase::STRING_PARAMETER_UNDEFINED);
const IntegerArray      GmatBase::INTEGERARRAY_PARAMETER_UNDEFINED = IntegerArray(1,
                        GmatBase::INTEGER_PARAMETER_UNDEFINED);
const UnsignedIntArray  GmatBase::UNSIGNED_INTARRAY_PARAMETER_UNDEFINED = UnsignedIntArray(1,
                        GmatBase::UNSIGNED_INT_PARAMETER_UNDEFINED);
const Rvector     GmatBase::RVECTOR_PARAMETER_UNDEFINED = Rvector(1,
                  GmatBase::REAL_PARAMETER_UNDEFINED);
const Rmatrix     GmatBase::RMATRIX_PARAMETER_UNDEFINED = Rmatrix(1,1,
                  GmatBase::REAL_PARAMETER_UNDEFINED);


const Gmat::ParameterType GmatBase::PARAMETER_TYPE[GmatBaseParamCount] =
      {
            Gmat::RMATRIX_TYPE,
      };

const wxString GmatBase::PARAMETER_LABEL[GmatBaseParamCount] =
      {
            wxT("Covariance"),
      };




/**
 * Build the list of type names
 *
 * This list needs to be synchronized with the Gmat::ParameterType list found in
 * base/include/gmatdefs.hpp
 */
const wxString
GmatBase::PARAM_TYPE_STRING[Gmat::TypeCount] =
{
   wxT("Integer"),     wxT("UnsignedInt"), wxT("UnsignedIntArray"), wxT("IntegerArray"), wxT("Real"),
   wxT("RealElement"), wxT("String"),      wxT("StringArray"),      wxT("Boolean"),      wxT("BooleanArray"),
   wxT("Rvector"),     wxT("Rmatrix"),     wxT("Time"),             wxT("Object"),       wxT("ObjectArray"),
   wxT("OnOff"),       wxT("Enumeration"), wxT("Filename")
};

/**
 * Build the list of object type names
 *
 * This list needs to be synchronized with the Gmat::ObjectType list found in
 * base/include/gmatdefs.hpp
 */
const wxString
GmatBase::OBJECT_TYPE_STRING[Gmat::UNKNOWN_OBJECT - Gmat::SPACECRAFT+1] =
{
   wxT("Spacecraft"),    wxT("Formation"),        wxT("SpaceObject"),      wxT("GroundStation"),    wxT("Burn"),
   wxT("ImpulsiveBurn"), wxT("FiniteBurn"),       wxT("Command"),          wxT("Propagator"),       wxT("ODEModel"),
   wxT("PhysicalModel"), wxT("TransientForce"),   wxT("Interpolator"),     wxT("SolarSystem"),      wxT("SpacePoint"),
   wxT("CelestialBody"), wxT("CalculatedPoint"),  wxT("LibrationPoint"),   wxT("Barycenter"),       wxT("Atmosphere"),
   wxT("Parameter"),     wxT("Variable"),         wxT("Array"),            wxT("String"),           wxT("StopCondition"),
   wxT("Solver"),        wxT("Subscriber"),       wxT("ReportFile"),       wxT("XYPlot"),           wxT("OrbitView"),
   wxT("EphemerisFile"), wxT("PropSetup"),        wxT("Function"),         wxT("FuelTank"),         wxT("Thruster"),
   wxT("Hardware"),      wxT("CoordinateSystem"), wxT("AxisSystem"),       wxT("Attitude"),         wxT("MathNode"),
   wxT("MathTree"),      wxT("BodyFixedPoint"),   wxT("Event"),            wxT("EventLocator"),     wxT("MeasurementModel"),
   wxT("CoreMeasurement"),wxT("TrackingData"),    wxT("TrackingSystem"),   wxT("DataStream"),       wxT("DataFile"),
   wxT("ObType"),        wxT("Interface"),        wxT("MediaCorrection"),  wxT("Sensor"),           wxT("RFHardware"),
   wxT("Antenna"),       wxT("UnknownObject")
};
/**
 * Build the list of automatic global settings
 *
 * This list needs to be synchronized with the Gmat::ObjectType list found in
 * base/include/gmatdefs.hpp
 *
 * Current automatic global objects: Propagator, CoordinateSystem, Function, PropSetup
 */
// There is some propblem if we make ODEModel global, so turned it off (loj: 2008.11.10)
const bool
GmatBase::AUTOMATIC_GLOBAL_FLAGS[Gmat::UNKNOWN_OBJECT - Gmat::SPACECRAFT+1] =
{
   false,     false,     false,     false,     false,
   false,     false,     false,     true ,     false,
   false,     false,     false,     false,     false,
   true,      true,      true,      true,      false,
   false,     false,     false,     false,     false,
   false,     false,     false,     false,     false,
   false,     true,      true,      false,     false,
   false,     true,      false,     false,     false,
   false,     false,     false,     false,     false,
   false,     false,     false,     false,     false,
   false,     false,     false,     false,     false,
   false,     false,
};


/// initialize the count of instances
Integer GmatBase::instanceCount = 0;

//-------------------------------------
// public methods
//-------------------------------------

//---------------------------------------------------------------------------
//  GmatBase(Gmat::ObjectTypes typeId, wxString &typeStr, wxString &nomme)
//---------------------------------------------------------------------------
/**
 * Constructs base GmatBase structures used in GMAT extensible classes.
 *
 * @param <typeId> Gmat::ObjectTypes enumeration for the object.
 * @param <typeStr> GMAT script string associated with this type of object.
 * @param <nomme> Optional name for the object.  Defaults to wxT("").
 *
 * @note There is no parameter free constructor for GmatBase.  Derived classes
 *       must pass in the typeId and typeStr parameters.
 */
//------------------------------------------------------------------------------
GmatBase::GmatBase(const Gmat::ObjectType typeId, const wxString &typeStr,
                   const wxString &nomme) :
   parameterCount    (GmatBaseParamCount),
   typeName          (typeStr),
   instanceName      (nomme),
   type              (typeId),
   ownedObjectCount  (0),
   callbackExecuting (false),
   commentLine       (wxT("")),
   inlineComment     (wxT("")),
   showPrefaceComment(true),
   showInlineComment (true),
   cloaking          (false)
{
   attributeCommentLines.clear();
   attributeInlineComments.clear();
   
   if (type == Gmat::COMMAND)
   {
      deprecatedMessageFormat =
         wxT("***WARNING*** \"%s\" field of ") + typeName + wxT(" is deprecated and will ")
         wxT("be removed from a future build; please use \"%s\" instead.\n");
      errorMessageFormat =
         wxT("The value of \"%s\" for field \"%s\" on command \"")
         + typeName +  wxT("\" is not an allowed value.\n")
         wxT("The allowed values are: [%s]");
      errorMessageFormatUnnamed =
         wxT("The value of \"%s\" for field \"%s\" on a command of type \"")
         + typeName +  wxT("\" is not an allowed value.\n")
         wxT("The allowed values are: [%s]");
   }
   else
   {
      deprecatedMessageFormat =
         wxT("*** WARNING *** \"%s\" field of ") + typeName + wxT(" on object \"%s\" is ")
         wxT("deprecated and will be removed from a future build; please use \"%s\" ")
         wxT("instead.\n");
      errorMessageFormat =
         wxT("The value of \"%s\" for field \"%s\" on object \"")
         + instanceName +  wxT("\" is not an allowed value.\n")
         wxT("The allowed values are: [%s]");
      errorMessageFormatUnnamed =
         wxT("The value of \"%s\" for field \"%s\" on an object of type \"")
         + typeName +  wxT("\" is not an allowed value.\n")
         wxT("The allowed values are: [%s]");
   }
   
   // Set the isGlobal flag appropriately
   isGlobal = AUTOMATIC_GLOBAL_FLAGS[type - Gmat::SPACECRAFT];
   isLocal = false;
   
   // one more instance - add to the instanceCount
   ++instanceCount;
}


//---------------------------------------------------------------------------
//  ~GmatBase()
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
GmatBase::~GmatBase()
{
   // subtract this instance from the instanceCount
   --instanceCount;
}


//---------------------------------------------------------------------------
//  GmatBase(const GmatBase &a)
//---------------------------------------------------------------------------
/**
 * Copy Constructor for base GmatBase structures.
 *
 * @param <a> The original that is being copied.
 */
//------------------------------------------------------------------------------
GmatBase::GmatBase(const GmatBase &a) :
    parameterCount            (a.parameterCount),
    typeName                  (a.typeName),
    //instanceName    (wxT("CopyOf")+a.instanceName),
    instanceName              (a.instanceName),
    type                      (a.type),
    ownedObjectCount          (a.ownedObjectCount),
    generatingString          (a.generatingString),
    objectTypes               (a.objectTypes),
    objectTypeNames           (a.objectTypeNames),
    isGlobal                  (a.isGlobal),
    isLocal                   (a.isLocal),
    callbackExecuting         (false),
    errorMessageFormat        (a. errorMessageFormat),
    errorMessageFormatUnnamed (a. errorMessageFormatUnnamed),
    deprecatedMessageFormat   (a. deprecatedMessageFormat),
    commentLine               (a.commentLine),
    inlineComment             (a.inlineComment),
    attributeCommentLines     (a.attributeCommentLines),
    attributeInlineComments   (a.attributeInlineComments),
    showPrefaceComment        (a.showPrefaceComment),
    showInlineComment         (a.showInlineComment),
    cloaking                  (a.cloaking),
    covarianceList            (a.covarianceList),
    covarianceIds             (a.covarianceIds),
    covarianceSizes           (a.covarianceSizes),
    covariance                (a.covariance)
{
   // one more instance - add to the instanceCount
   ++instanceCount;
}


//---------------------------------------------------------------------------
//  GmatBase& operator=(const GmatBase &a)
//---------------------------------------------------------------------------
/**
 * Assignment operator for GmatBase structures.
 *
 * @param <a> The original that is being copied.
 *
 * @return Reference to this object
 */
//------------------------------------------------------------------------------
GmatBase& GmatBase::operator=(const GmatBase &a)
{
   // Don't do anything if copying self
   if (&a == this)
      return *this;

   parameterCount            = a.parameterCount;
   typeName                  = a.typeName;
   // We don't want to copy instanceName (loj: 2008.02.15)
   //instanceName     = a.instanceName;
   type                      = a.type;
   ownedObjectCount          = a.ownedObjectCount;
   generatingString          = a.generatingString;
   objectTypes               = a.objectTypes;
   objectTypeNames           = a.objectTypeNames;
   // don't want to modify isGlobal here
   isLocal                   = a.isLocal;
   callbackExecuting         = false;
   errorMessageFormat        = a. errorMessageFormat;
   errorMessageFormatUnnamed = a. errorMessageFormatUnnamed;
   deprecatedMessageFormat   = a.deprecatedMessageFormat;
   commentLine               = a.commentLine;
   inlineComment             = a.inlineComment;
   attributeCommentLines     = a.attributeCommentLines;
   attributeInlineComments   = a.attributeInlineComments;
   showPrefaceComment        = a.showPrefaceComment;
   showInlineComment         = a.showInlineComment;
   cloaking                  = a.cloaking;
   covarianceList            = a.covarianceList;
   covarianceIds             = a.covarianceIds;
   covarianceSizes           = a.covarianceSizes;
   covariance                = a.covariance;

   return *this;
}


//---------------------------------------------------------------------------
//  Gmat::ObjectTypes GetType() const
//---------------------------------------------------------------------------
/**
 * Retrieve the enumerated base type for the object
 *
 * @return Enumeration value for this object
 */
//------------------------------------------------------------------------------
Gmat::ObjectType GmatBase::GetType() const
{
   return type;
}


//---------------------------------------------------------------------------
//  bool SetName(wxString &who, const std;:string &oldName = wxT(""))
//---------------------------------------------------------------------------
/**
* Set the name for this instance.
 *
 * @param <who> the object's name.
 * @param <oldName> the object's old name to be set mainly for subscribers
 * @return true if the name was changed, false if an error was encountered.
 *
 * @note Some classes are unnamed.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetName(const wxString &who, const wxString &oldName)
{
   instanceName = who;
   return true;
}

//---------------------------------------------------------------------------
//  Integer GetParameterCount() const
//---------------------------------------------------------------------------
/**
 * Find out how many parameters are accessible for this object.
 *
 * @return The number of parameters.
 */
//------------------------------------------------------------------------------
Integer GmatBase::GetParameterCount() const
{
   return parameterCount;
}


//---------------------------------------------------------------------------
//  bool IsOfType(Gmat::ObjectType ofType)
//---------------------------------------------------------------------------
/**
* Detects if the object is a specified type.
 *
 * @param <ofType> The type that is being checked.
 *
 * @return true is the class was derived from the type, false if not.
 */
//---------------------------------------------------------------------------
bool GmatBase::IsOfType(Gmat::ObjectType ofType) const
{
   #ifdef DEBUG_OBJECT_TYPE_CHECKING
   MessageInterface::ShowMessage
      (wxT("Checking to see if %s is of type %d (Actual type is %d)\n"),
       instanceName.c_str(), ofType, type);
   #endif

   if (std::find(objectTypes.begin(), objectTypes.end(), ofType) !=
       objectTypes.end())
   {
      #ifdef DEBUG_OBJECT_TYPE_CHECKING
      MessageInterface::ShowMessage(wxT("   Object %s is the requested type\n"),
                                    instanceName.c_str());
      #endif
      return true;
   }

   #ifdef DEBUG_OBJECT_TYPE_CHECKING
   MessageInterface::ShowMessage
      (wxT("   Not the requested type; current types are ["));
   for (ObjectTypeArray::iterator i = objectTypes.begin();
        i != objectTypes.end(); ++i)
   {
      if (i != objectTypes.begin())
         MessageInterface::ShowMessage(wxT(", "));
      MessageInterface::ShowMessage(wxT("%d"), *i);
   }
   MessageInterface::ShowMessage(wxT("]\n"));
   #endif

   return false;
}


//---------------------------------------------------------------------------
//  bool IsOfType(wxString typeDescription)
//---------------------------------------------------------------------------
/**
* Detects if the object is a specified type.
 *
 * @param <typeDescription> The string describing the type.
 *
 * @return true is the class was derived from the type, false if not.
 */
//---------------------------------------------------------------------------
bool GmatBase::IsOfType(wxString typeDescription) const
{
   #ifdef DEBUG_OBJECT_TYPE_CHECKING
   MessageInterface::ShowMessage
      (wxT("Checking to see if %s is of type %s (Actual type is %s)\n"),
       instanceName.c_str(), typeDescription.c_str(), typeName.c_str());
   #endif

   if (std::find(objectTypeNames.begin(), objectTypeNames.end(),
                 typeDescription) != objectTypeNames.end())
   {
      #ifdef DEBUG_OBJECT_TYPE_CHECKING
      MessageInterface::ShowMessage(wxT("   Object %s is the requested type\n"),
                                    instanceName.c_str());
      #endif
      return true;
   }

   #ifdef DEBUG_OBJECT_TYPE_CHECKING
   MessageInterface::ShowMessage
      (wxT("   Not the requested type; current types are ["));
   for (StringArray::iterator i = objectTypeNames.begin();
        i != objectTypeNames.end(); ++i)
   {
      if (i != objectTypeNames.begin())
         MessageInterface::ShowMessage(wxT(", "));
      MessageInterface::ShowMessage(wxT("%s"), i->c_str());
   }
   MessageInterface::ShowMessage(wxT("]\n"));
   #endif

   return false;
}

//---------------------------------------------------------------------------
// void GmatBase::SetShowPrefaceComment(bool show = true)
//---------------------------------------------------------------------------
/*
 * Sets show preface comment flag.
 */
//---------------------------------------------------------------------------
void GmatBase::SetShowPrefaceComment(bool show)
{
   showPrefaceComment = show;
}

//---------------------------------------------------------------------------
// void SetShowInlineComment(bool show = true)
//---------------------------------------------------------------------------
/*
 * Sets show inline comment flag.
 */
//---------------------------------------------------------------------------
void GmatBase::SetShowInlineComment(bool show)
{
   showInlineComment = show;
}

//---------------------------------------------------------------------------
// bool GetShowPrefaceComment()
//---------------------------------------------------------------------------
/*
 * @return Show preface comment flag
 */
//---------------------------------------------------------------------------
bool GmatBase::GetShowPrefaceComment()
{
   return showPrefaceComment;
}

//---------------------------------------------------------------------------
// bool GetShowInlineComment()
//---------------------------------------------------------------------------
/*
 * @return Show inline comment flag
 */
//---------------------------------------------------------------------------
bool GmatBase::GetShowInlineComment()
{
   return showInlineComment;
}


//---------------------------------------------------------------------------
//  wxString GetRefObjectName(const Gmat::ObjectType type) const
//---------------------------------------------------------------------------
/**
 * Returns the name of the reference object. (Derived classes should implement
 * this as needed.)
 *
 * @param <type> reference object type.
 *
 * @return The name of the reference object.
 */
//------------------------------------------------------------------------------
wxString GmatBase::GetRefObjectName(const Gmat::ObjectType type) const
{
   throw GmatBaseException(wxT("Reference Object not defined for ") + typeName +
                           wxT(" named \"") + instanceName + wxT("\"\n"));
}


//------------------------------------------------------------------------------
// virtual bool HasRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Returns flag indicating whether GetRefObjectTypeArray() is implemented or not.
 * Currently this method is used in the Interpreter::FinalPass() for validation.
 *
 * @note Derived classes should override this if GetRefObjectTypeArray() is
 *       implemented
 */
//------------------------------------------------------------------------------
bool GmatBase::HasRefObjectTypeArray()
{
   return false;
}


//---------------------------------------------------------------------------
//  const ObjectTypeArray& GetRefObjectTypeArray()
//---------------------------------------------------------------------------
/**
 * Returns the types of the reference object. (Derived classes should implement
 * this as needed.)
 *
 * @return The types of the reference object.
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& GmatBase::GetRefObjectTypeArray()
{
   // should return empty array
   refObjectTypes.clear();
   return refObjectTypes;
}


//---------------------------------------------------------------------------
//  const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//---------------------------------------------------------------------------
/**
 * Returns the names of the reference object. (Derived classes should implement
 * this as needed.)
 *
 * @param <type> reference object type.
 *
 * @return The names of the reference object.
 */
//------------------------------------------------------------------------------
const StringArray& GmatBase::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   // Changed to return empty array (LOJ: 2010.05.13)
   refObjectNames.clear();
   return refObjectNames;
}

//---------------------------------------------------------------------------
//  bool SetRefObjectName(const Gmat::ObjectType type, const wxString &name)
//---------------------------------------------------------------------------
/**
 * Sets the name of the reference object.  (Derived classes should implement
 * this as needed.)
 *
 * @param <type> type of the reference object.
 * @param <name> name of the reference object.
 *
 * @return success of the operation.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetRefObjectName(const Gmat::ObjectType type,
                                const wxString &name)
{
   wxString objName = typeName;
   if (GetType() != Gmat::COMMAND)
      objName = objName + wxT(" named \"") + instanceName + wxT("\"");

   throw GmatBaseException(wxT("SetRefObjectName(") + GetObjectTypeString(type) +
                           wxT(", ") + name + wxT(") not defined for ") + objName);
}

//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
/**
 * Interface used to support user actions.
 *
 * @param <type> reference object type.
 * @param <oldName> object name to be renamed.
 * @param <newName> new object name.
 *
 * @return true if object name changed, false if not.
 */
//------------------------------------------------------------------------------
bool GmatBase::RenameRefObject(const Gmat::ObjectType type,
                               const wxString &oldName,
                               const wxString &newName)
{
   MessageInterface::ShowMessage
      (wxT("*** ERROR *** The object type:%s needs RenameRefObject() ")
       wxT("implementation.\n"), GetTypeName().c_str());

   return false;
}

//---------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name)
//---------------------------------------------------------------------------
/**
 * Returns the reference object pointer.  (Derived classes should implement
 * this method as needed.)
 *
 * @param type type of the reference object.
 * @param name name of the reference object.
 *
 * @return reference object pointer.
 */
//------------------------------------------------------------------------------
GmatBase* GmatBase::GetRefObject(const Gmat::ObjectType type,
                                 const wxString &name)
{
   wxString objName = typeName;
   if (GetType() != Gmat::COMMAND)
      objName = objName + wxT(" named \"") + instanceName + wxT("\"");

   throw GmatBaseException(wxT("GetRefObject(") + GetObjectTypeString(type) +
                           wxT(", ") + name + wxT(") not defined for ") + objName);
}


//---------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name,
//                        const Integer index)
//---------------------------------------------------------------------------
/**
 * Returns the reference object pointer.  (Derived classes should implement
 * this method as needed.)
 *
 * @param type type of the reference object.
 * @param name name of the reference object.
 * @param index Index into the object array.
 *
 * @return reference object pointer.
 */
//------------------------------------------------------------------------------
GmatBase* GmatBase::GetRefObject(const Gmat::ObjectType type,
                                 const wxString &name, const Integer index)
{
   wxString indexString;
   indexString << index;
   wxString objName = typeName;
   if (GetType() != Gmat::COMMAND)
      objName = objName + wxT(" named \"") + instanceName + wxT("\"");

   throw GmatBaseException(wxT("GetRefObject(") + GetObjectTypeString(type) +
                           wxT(", ") + name + wxT(", ") + indexString +
                           wxT(") not defined for ") + objName);
}

//---------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name)
//---------------------------------------------------------------------------
/**
 * Sets the reference object.  (Derived classes should implement
 * this method as needed.)
 *
 * @param <obj> reference object pointer.
 * @param <type> type of the reference object.
 * @param <name> name of the reference object.
 *
 * @return success of the operation.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                            const wxString &name)
{
   wxString objName = typeName;
   if (GetType() != Gmat::COMMAND)
      objName = objName + wxT(" named \"") + instanceName + wxT("\"");

   throw GmatBaseException(wxT("SetRefObject(*obj, ") + GetObjectTypeString(type) +
                           wxT(", ") + name + wxT(") not defined for ") + objName);
}


//---------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                   const wxString &name, const Integer index)
//---------------------------------------------------------------------------
/**
 * Sets the reference object.  (Derived classes should implement
 * this method as needed.)
 *
 * @param obj reference object pointer.
 * @param type type of the reference object.
 * @param name name of the reference object.
 * @param index Index into the object array.
 *
 * @return success of the operation.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                            const wxString &name, const Integer index)
{
   wxString indexString;
   indexString << index;
   wxString objName = typeName;
   if (GetType() != Gmat::COMMAND)
      objName = objName + wxT(" named \"") + instanceName + wxT("\"");

   throw GmatBaseException(wxT("SetRefObject(*obj, ") + GetObjectTypeString(type) +
                           wxT(", ") + name + wxT(", ") + indexString +
                           wxT(") not defined for ") + objName);
}


//---------------------------------------------------------------------------
//  ObjectArray& GetRefObjectArray(const Gmat::ObjectType type)
//---------------------------------------------------------------------------
/**
 * Obtains an array of GmatBase pointers by type.
 *
 * @param type The type of objects requested
 *
 * @return Reference to the array.  This default method returns an empty vector.
 */
//------------------------------------------------------------------------------
ObjectArray& GmatBase::GetRefObjectArray(const Gmat::ObjectType type)
{
   static ObjectArray oa;
   oa.clear();
   return oa;
}


//---------------------------------------------------------------------------
//  ObjectArray& GetRefObjectArray(const Gmat::ObjectType type)
//---------------------------------------------------------------------------
/**
 * Obtains an array of GmatBase pointers based on a string (e.g. the typename).
 *
 * @param typeString The string used to find the objects requested.
 *
 * @return Reference to the array.  This default method returns an empty vector.
 */
//------------------------------------------------------------------------------
ObjectArray& GmatBase::GetRefObjectArray(const wxString& typeString)
{
   return GetRefObjectArray(Gmat::UNKNOWN_OBJECT);
}


//---------------------------------------------------------------------------
// bool IsOwnedObject(Integer id) const
//---------------------------------------------------------------------------
/*
 * Finds out if object property of given id is owned object
 * This method was added so that Interpreter can querry and create
 * proper owned objects.
 *
 * Any subclass should override this method as necessary.
 *
 * @param  id   The object property id to check for owned object
 *
 * @return  true if it is owned object
 */
//---------------------------------------------------------------------------
bool GmatBase::IsOwnedObject(Integer id) const
{
   return false;
}


//---------------------------------------------------------------------------
//  Integer GetOwnedObjectCount()
//---------------------------------------------------------------------------
/**
 * Find out how many GmatBase objects belong to this instance.
 *
 * This method is used find out how many unnamed objects belong to the current
 * instance.  The Interpreter subsystem used this method to count these objects,
 * so that it can access them one by one for reading and writing.
 *
 * One example of this use is the PropSetup class; each PropSetup contains an
 * unnamed Integrator.  The Integrator parameters are accessed for reading and
 * writing scripts. This method determines how many objects are available for
 * access.  The objects are then accessed using GetOwnedObject(Integer).  The
 * input parameter to GetOwnedObject method is found by calling this method,
 * and then looping through all of the owned objects, accessing the related
 * parameters for each.
 *
 * @return The number of owned objects.
 */
//------------------------------------------------------------------------------
Integer GmatBase::GetOwnedObjectCount()
{
   return ownedObjectCount;
}

//---------------------------------------------------------------------------
//  GmatBase* GetOwnedObject(Integer whichOne)
//---------------------------------------------------------------------------
/**
 * Access GmatBase objects belonging to this instance.
 *
 * This method is used to access the unnamed objects owned by a class derived
 * from GmatBase.  The Interpreter subsystem used this method to access
 * parameters for these objects.
 *
 * One example of this use is the PropSetup class; each PropSetup contains an
 * unnamed Integrator.  The Integrator parameters are accessed for reading and
 * writing scripts using this method.
 *
 * The input parameter to GetOwnedObject an integer that identifies which owned
 * object is requested.
 *
 * @param <whichOne> The index for this owned object.
 *
 * @return Pointer to the owned object.
 */
//---------------------------------------------------------------------------
GmatBase* GmatBase::GetOwnedObject(Integer whichOne)
{
   throw GmatBaseException
      (wxT("No owned objects for this instance \"") + instanceName + wxT("\" of type \"") +
       typeName + wxT("\""));
}

//------------------------------------------------------------------------------
//  bool SetIsGlobal(bool globalFlag)
//------------------------------------------------------------------------------
/**
 * Method to set the isGlobal flag for an object.
 *
 * @param globalFlag  flag indicating whether or not this object is global
 *
 * @return value of isGlobal flag.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetIsGlobal(bool globalFlag)
{
   isGlobal = globalFlag;
   return isGlobal;
}

//------------------------------------------------------------------------------
//  bool GetIsGlobal()
//------------------------------------------------------------------------------
/**
 * Method to return the isGlobal flag for an object.
 *
 * @return value of isGlobal flag (i.e. whether or not this object is Global)
 */
//------------------------------------------------------------------------------
bool GmatBase::GetIsGlobal() const
{
   return isGlobal;
}

//------------------------------------------------------------------------------
//  bool SetIsLocal(bool localFlag)
//------------------------------------------------------------------------------
/**
 * Method to set the isLocal flag for an object. Local object means that
 * the object was passed to a function or created inside a function.
 *
 * @param localFlag  flag indicating whether or not this object is local
 *
 * @return value of isLocal flag.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetIsLocal(bool localFlag)
{
   isLocal = localFlag;
   return isLocal;
}

//------------------------------------------------------------------------------
//  bool IsLocal()
//------------------------------------------------------------------------------
/**
 * Method to return the isLocal flag for an object.
 *
 * @return value of isLocal flag (i.e. whether or not this object is Local)
 */
//------------------------------------------------------------------------------
bool GmatBase::IsLocal() const
{
   return isLocal;
}

//------------------------------------------------------------------------------
// bool GmatBase::IsObjectCloaked() const
//------------------------------------------------------------------------------
/**
 * Method to return the cloaking flag for an object.
 *
 * @return value of cloaking flag (i.e. whether or not this object is cloaked)
 */
//------------------------------------------------------------------------------
bool GmatBase::IsObjectCloaked() const
{
   if (!cloaking) return false;
   
   #ifdef DEBUG_CLOAKING
      MessageInterface::ShowMessage(
            wxT("Entering GmatBase::IsObjectCloaked for object %s - there are %d parameters\n"),
            instanceName.c_str(), parameterCount);
   #endif
   for (Integer ii = 0; ii < parameterCount; ii++)
      if (!IsParameterCloaked(ii))
      {
         #ifdef DEBUG_CLOAKING
            MessageInterface::ShowMessage(
                  wxT("in GmatBase::IsObjectCloaked for object %s - parameter %d (%s) is not cloaked\n"),
                  instanceName.c_str(), ii, (GetParameterText(ii)).c_str());
         #endif
         return false; 
      }
      #ifdef DEBUG_CLOAKING
         MessageInterface::ShowMessage(
               wxT("Exiting GmatBase::IsObjectCloaked returning true, as all parameters are cloaked\n"));
      #endif
   return true;
}

bool GmatBase::SaveAllAsDefault()
{
   return true;     
}

bool GmatBase::SaveParameterAsDefault(const Integer id)
{
   return true;
}

bool GmatBase::SaveParameterAsDefault(const wxString &label)
{
   return SaveParameterAsDefault(GetParameterID(label));
}


//------------------------------------------------------------------------------
//  bool ExecuteCallback()
//------------------------------------------------------------------------------
/**
 * Default implementation for method that executes a callback function.
 *
 * @return true if successful; otherwise, return false (this default
 * implementation returns false).
 */
//------------------------------------------------------------------------------
bool GmatBase::ExecuteCallback()
{
   return false;
}

//------------------------------------------------------------------------------
//  bool IsCallbackExecuting()
//------------------------------------------------------------------------------
/**
 * Returns a flag indicating whether or not the callback is executing.
 *
 * @return true if executing; otherwise, else false
 */
//------------------------------------------------------------------------------
bool GmatBase::IsCallbackExecuting()
{
   return callbackExecuting;
}

//---------------------------------------------------------------------------
// bool PutCallbackData(wxString &data)
//---------------------------------------------------------------------------
bool GmatBase::PutCallbackData(wxString &data)
{
   return false;
}

//---------------------------------------------------------------------------
// wxString GetCallbackResults()
//---------------------------------------------------------------------------
wxString GmatBase::GetCallbackResults()
{
   return wxT("no data");
}


//---------------------------------------------------------------------------
//  void Copy(GmatBase *inst)
//---------------------------------------------------------------------------
/**
 * Set this instance to match the one passed in.
 *
 * The default throws an exception.  Users must override this method for classes
 * that allow copying from a script.
 *
 * @param <inst> The object that is being copied.
 */
void GmatBase::Copy(const GmatBase*)
{
   throw GmatBaseException(wxT("Cannot copy objects of type ") + typeName);
}


//------------------------------------------------------------------------------
//  bool Validate()
//------------------------------------------------------------------------------
/**
 * Performs any pre-run validation that the object needs.
 *
 * @param msg This receives any message after calling this
 * @return true unless validation fails.
 */
//------------------------------------------------------------------------------
bool GmatBase::Validate()
{
   return true;
}

//------------------------------------------------------------------------------
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * Performs any pre-run initialization that the object needs.
 *
 * @return true unless initialization fails.
 */
//------------------------------------------------------------------------------
bool GmatBase::Initialize()
{
   return true;
}

//------------------------------------------------------------------------------
//  void GmatBase::SetSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 * Sets the internal solar system pointer for objects that have one.
 *
 * @note Derived classes that need the solar system must override this method.
 */
//------------------------------------------------------------------------------
void GmatBase::SetSolarSystem(SolarSystem *ss)
{
   ;     // Do nothing by default
}

//------------------------------------------------------------------------------
//  void GmatBase::SetInternalCoordSystem(CoordinateSystem *cs)
//------------------------------------------------------------------------------
/**
 * Sets the internal CoordinateSystem pointer for objects that have one.
 *
 * @note Derived classes that need the internal coordinate system must override
 *       this method.
 */
//------------------------------------------------------------------------------
void GmatBase::SetInternalCoordSystem(CoordinateSystem *cs)
{
   ;     // Do nothing by default
}

//------------------------------------------------------------------------------
//  bool RequiresJ2000Body()
//------------------------------------------------------------------------------
/**
 * Identifies objects that need to have the J2000 body set in the Sandbox.
 *
 * @return true if the J2000 body needs to be set, false if not.
 */
//------------------------------------------------------------------------------
bool GmatBase::RequiresJ2000Body()
{
   return false;
}


//---------------------------------------------------------------------------
//  Gmat::ParameterType GetParameterType(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve the enumerated type of the object.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The enumeration for the type of the parameter, or
 *         UNKNOWN_PARAMETER_TYPE.
 */
//------------------------------------------------------------------------------
Gmat::ParameterType GmatBase::GetParameterType(const Integer id) const
{
   if ((id < GmatBaseParamCount) && (id >= 0))
      return PARAMETER_TYPE[id];

   return Gmat::UNKNOWN_PARAMETER_TYPE;
}


//---------------------------------------------------------------------------
//  wxString GetParameterTypeString(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve the string associated with a parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return Text description for the type of the parameter, or the empty
 *         string (wxT("")).
 */
//------------------------------------------------------------------------------
wxString GmatBase::GetParameterTypeString(const Integer id) const
{
   wxString retval = wxT("");
   Gmat::ParameterType t = GetParameterType(id);
   if (t != Gmat::UNKNOWN_PARAMETER_TYPE)
      retval = PARAM_TYPE_STRING[t];
   return retval;
}


//---------------------------------------------------------------------------
//  wxString GetParameterText(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve the description for the parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return String description for the requested parameter.
 *
 * @note The parameter strings should not include any white space
 */
//------------------------------------------------------------------------------
wxString GmatBase::GetParameterText(const Integer id) const
{
   if ((id < GmatBaseParamCount) && (id >= 0))
      return PARAMETER_LABEL[id];

   wxString indexString;
   indexString << id;
   throw GmatBaseException(wxT("Parameter id = ") + indexString +
                           wxT(" not defined on object ") + instanceName);
}


//---------------------------------------------------------------------------
//  wxString GetParameterUnit(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve the unit for the parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return unit for the requested parameter.
 */
//------------------------------------------------------------------------------
wxString GmatBase::GetParameterUnit(const Integer id) const
{
   return wxT("");
}


//---------------------------------------------------------------------------
//  Integer GetParameterID(const wxString &str) const
//---------------------------------------------------------------------------
/**
 * Retrieve the ID for the parameter given its description.
 *
 * @param <str> Description for the parameter.
 *
 * @return the parameter ID, or -1 if there is no associated ID.
 */
//---------------------------------------------------------------------------
Integer GmatBase::GetParameterID(const wxString &str) const
{
   for (Integer i = 0; i < GmatBaseParamCount; ++i)
      if (str == PARAMETER_LABEL[i])
      {
         return i;
      }
   
   throw GmatBaseException
      (wxT("GmatBase::GetParameterID() The object named \"") + GetName() +
       wxT("\" of type \"") + GetTypeName() + wxT("\" has no parameter defined with \"") +
       str + wxT("\""));
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
bool GmatBase::IsParameterReadOnly(const Integer id) const
{
   if ((id < 0) || (id >= parameterCount))
   {
      wxString errmsg;
      errmsg << wxT("GmatBase::IsParameterReadOnly() No parameter defined with id ")
             << id << wxT(" on ") << typeName << wxT(" named \"") << instanceName <<wxT("\"");
      throw GmatBaseException(errmsg);
   }
   
   // Hide covariance if it hasn't been defined
   if (id == COVARIANCE)
   {
      if (covarianceList.size() == 0)
         return true;
   }
   
   return false;
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <label> Description for the parameter.
 *
 * @return true if the parameter is read only, false (the default) if not.
 */
//---------------------------------------------------------------------------
bool GmatBase::IsParameterReadOnly(const wxString &label) const
{
   return IsParameterReadOnly(GetParameterID(label));
}

//---------------------------------------------------------------------------
//  bool IsParameterEnabled(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is enabled for GUI access.
 *
 * @param <id> ID for the parameter.
 *
 * @return true if the parameter is enabled, false if not
 */
//---------------------------------------------------------------------------
bool GmatBase::IsParameterEnabled(const Integer id) const
{
   return true;
}


//---------------------------------------------------------------------------
//  bool IsParameterEnabled(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is enabled for GUI access.
 *
 * @param <label> Description for the parameter.
 *
 * @return true if the parameter is enabled, false if not
 */
//---------------------------------------------------------------------------
bool GmatBase::IsParameterEnabled(const wxString &label) const
{
   return IsParameterEnabled(GetParameterID(label));
}

//---------------------------------------------------------------------------
//  bool IsParameterCloaked(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is cloaked.
 *
 * @param <id> ID for the parameter.
 *
 * @return true if the parameter is cloaked, false if not
 */
//---------------------------------------------------------------------------
bool GmatBase::IsParameterCloaked(const Integer id) const
{
   return false;
}

//---------------------------------------------------------------------------
//  bool IsParameterCloaked(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is cloaked.
 *
 * @param <label> Description for the parameter.
 *
 * @return true if the parameter is cloaked, false if not
 */
//---------------------------------------------------------------------------
bool GmatBase::IsParameterCloaked(const wxString &label) const
{
   return IsParameterCloaked(GetParameterID(label));
}

//---------------------------------------------------------------------------
//  bool IsParameterEqualToDefault(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is equal to default
 *
 * @param <id> ID for the parameter.
 *
 * @return true if the parameter is equal to default, false if not
 */
//---------------------------------------------------------------------------
bool GmatBase::IsParameterEqualToDefault(const Integer id) const
{
   return false;
}

//---------------------------------------------------------------------------
//  bool IsParameterEqualToDefault(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is equal to default
 *
 * @param <label> Description for the parameter.
 *
 * @return true if the parameter is equal to default, false if not
 */
//---------------------------------------------------------------------------
bool GmatBase::IsParameterEqualToDefault(const wxString &label) const
{
   return IsParameterEqualToDefault(GetParameterID(label));
}

//---------------------------------------------------------------------------
//  bool IsParameterVisible(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is visible from the GUI
 *
 * @param <id> ID for the parameter.
 *
 * @return true if the parameter is visible from the GUI, false if not
 */
//---------------------------------------------------------------------------
bool GmatBase::IsParameterVisible(const Integer id) const
{
   return true;
}

//---------------------------------------------------------------------------
//  bool IsParameterVisible(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is visible from the GUI
 *
 * @param <label> Description for the parameter.
 *
 * @return true if the parameter is visible from the GUI, false if not
 */
//---------------------------------------------------------------------------
bool GmatBase::IsParameterVisible(const wxString &label) const
{
   return IsParameterVisible(GetParameterID(label));
}

//------------------------------------------------------------------------------
// bool GmatBase::ParameterAffectsDynamics(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Determines if a parameter update affects propagation, and therfore forces a
 * reload of parameters used in propagation
 *
 * @param id The ID of the parameter
 *
 * @return true if the parameter affects propagation, false otherwise
 */
//------------------------------------------------------------------------------
bool GmatBase::ParameterAffectsDynamics(const Integer id) const
{
   return false;
}


bool GmatBase::ParameterDvInitializesNonzero(const Integer id, const Integer r,
      const Integer c) const
{
   return false;
}


Real GmatBase::ParameterDvInitialValue(const Integer id, const Integer r,
      const Integer c) const
{
   return 0.0;
}


bool GmatBase::ParameterUpdatesAfterSuperposition(const Integer id) const
{
   return false;
}


//---------------------------------------------------------------------------
// Gmat::ObjectType GetPropertyObjectType(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieves object type of parameter of given id.
 *
 * @param <id> ID for the parameter.
 *
 * @return parameter ObjectType
 */
//---------------------------------------------------------------------------
Gmat::ObjectType GmatBase::GetPropertyObjectType(const Integer id) const
{
   return Gmat::UNKNOWN_OBJECT;
}


//---------------------------------------------------------------------------
// const StringArray& GetPropertyEnumStrings(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieves eumeration symbols of parameter of given id.
 *
 * @param <id> ID for the parameter.
 *
 * @return list of enumeration symbols
 */
//---------------------------------------------------------------------------
const StringArray& GmatBase::GetPropertyEnumStrings(const Integer id) const
{
   static StringArray enumStrings;
   return enumStrings;
}


//---------------------------------------------------------------------------
// const StringArray& GetPropertyEnumStrings(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Retrieves eumeration symbols of parameter of given label
 *
 * @param <id> ID for the parameter.
 *
 * @return list of enumeration symbols
 */
//---------------------------------------------------------------------------
const StringArray& GmatBase::GetPropertyEnumStrings(const wxString &label) const
{
   return GetPropertyEnumStrings(GetParameterID(label));
}


//---------------------------------------------------------------------------
//  Real GetRealParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for a Real parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
Real GmatBase::GetRealParameter(const Integer id) const
{
   wxString indexString;
   indexString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot get real parameter with ID ") +
                           indexString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  Real SetRealParameter(const Integer id, const Real value)
//---------------------------------------------------------------------------
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
Real GmatBase::SetRealParameter(const Integer id, const Real value)
{
   wxString indexString;
   indexString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot set real parameter with ID ") +
                           indexString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  Real GetRealParameter(const Integer id, Integer index) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for a Real parameter.
 *
 * @param id The integer ID for the parameter.
 * @param index Index for objecs in arrays.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
Real GmatBase::GetRealParameter(const Integer id, const Integer index) const
{
   wxString indexString, idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   indexString << index;
   throw GmatBaseException(wxT("Cannot get real parameter with ID ") +
                           idString + wxT(" and index ") + indexString +
                           wxT(" on ") + typeName + wxT(" named \"") + instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  Real GetRealParameter(const Integer id, const Integer row,
//                        const Integer col) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for a Real parameter.
 *
 * @param id The integer ID for the parameter.
 * @param row Row index for parameters in arrays.
 * @param col Column index for parameters in arrays.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
Real GmatBase::GetRealParameter(const Integer id, const Integer row,
                                const Integer col) const
{
   if (id == COVARIANCE)
   {
      #ifdef DEBUG_COVARIANCE
         MessageInterface::ShowMessage(wxT("Getting covariance[%d,%d] = %le\n"),
               row, col, covariance(row,col));
      #endif
      return covariance(row,col);
   }

   wxString indexString, idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   indexString << wxT(", row ") << row << wxT(" and column ") << col;
   throw GmatBaseException(wxT("Cannot get real array element with ID ") +
                           idString + indexString +
                           wxT(" on ") + typeName + wxT(" named \"") + instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  Real SetRealParameter(const Integer id, const Real value, Integer index)
//---------------------------------------------------------------------------
/**
 * Set the value for a Real parameter.
 *
 * @param <id>    The integer ID for the parameter.
 * @param <value> The new parameter value.
 * @param <row>   The row in which the data is set.
 * @param <col>   The column in which the data is set.
 *
 * @return the parameter value at the end of this call, or throw an exception
 *         if the parameter id is invalid or the parameter type is not Real, or
 *         the parameter is not in an array.
 */
//---------------------------------------------------------------------------
Real GmatBase::SetRealParameter(const Integer id, const Real value,
                                const Integer row, const Integer col)
{
   if (id == COVARIANCE)
   {
      #ifdef DEBUG_COVARIANCE
         MessageInterface::ShowMessage(wxT("Setting covariance[%d,%d] = %le\n"),
               row, col, value);
      #endif
      covariance(row,col) = value;
      return covariance(row,col);
   }

   wxString idString;
   idString << id << wxT(" and label ") << GetParameterText(id);
   throw GmatBaseException(wxT("Cannot set real parameter with ID ") +
                           idString + wxT(" using array indices on ") +
                           typeName + wxT(" named \"") + instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  Real SetRealParameter(const Integer id, const Real value, Integer index)
//---------------------------------------------------------------------------
/**
 * Set the value for a Real parameter.
 *
 * @param id The integer ID for the parameter.
 * @param value The new parameter value.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return the parameter value at the end of this call, or
 *         REAL_PARAMETER_UNDEFINED if the parameter id is invalid or the
 *         parameter type is not Real.
 */
//------------------------------------------------------------------------------
Real GmatBase::SetRealParameter(const Integer id, const Real value,
                                const Integer index)
{
   wxString indexString, idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   indexString << index;
   throw GmatBaseException(wxT("Cannot set real parameter with ID ") +
                           idString + wxT(" and index ") + indexString +
                           wxT(" on ") + typeName + wxT(" named \"") + instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  Integer GetIntegerParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
* Retrieve the value for an Integer parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
Integer GmatBase::GetIntegerParameter(const Integer id) const
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot get integer parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  Integer SetIntegerParameter(const Integer id, const Integer value)
//---------------------------------------------------------------------------
/**
* Set the value for an Integer parameter.
 *
 * @param <id> The integer ID for the parameter.
 * @param <value> The new parameter value.
 *
 * @return the parameter value at the end of this call, or
 *         INTEGER_PARAMETER_UNDEFINED if the parameter id is invalid or the
 *         parameter type is not an Integer.
 */
//------------------------------------------------------------------------------
Integer GmatBase::SetIntegerParameter(const Integer id, const Integer value)
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot set integer parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  Integer GetIntegerParameter(const Integer id, const Integer index) const
//---------------------------------------------------------------------------
/**
* Retrieve the value for an Integer parameter.
 *
 * @param id The integer ID for the parameter.
 * @param index Index for objecs in arrays.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
Integer GmatBase::GetIntegerParameter(const Integer id,
                                      const Integer index) const
{
   wxString indexString, idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   indexString << index;
   throw GmatBaseException(wxT("Cannot get integer parameter with ID ") +
                           idString + wxT(" and index ") + indexString +
                           wxT(" on ") + typeName + wxT(" named \"") + instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  Integer SetIntegerParameter(const Integer id, const Integer value,
//                              const Integer index)
//---------------------------------------------------------------------------
/**
* Set the value for an Integer parameter.
 *
 * @param id The integer ID for the parameter.
 * @param value The new parameter value.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return the parameter value at the end of this call, or
 *         INTEGER_PARAMETER_UNDEFINED if the parameter id is invalid or the
 *         parameter type is not an Integer.
 */
//------------------------------------------------------------------------------
Integer GmatBase::SetIntegerParameter(const Integer id, const Integer value,
                                      const Integer index)
{
   wxString indexString, idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   indexString << index;
   throw GmatBaseException(wxT("Cannot set integer parameter with ID ") +
                           idString + wxT(" and index ") + indexString +
                           wxT(" on ") + typeName + wxT(" named \"") + instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  UnsignedInt GetUnsignedIntParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for an UnsignedInt parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
UnsignedInt GmatBase::GetUnsignedIntParameter(const Integer id) const
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot get unsigned integer parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  UnsignedInt SetUnsignedIntParameter(const Integer id,
//                                      const UnsignedInt value)
//---------------------------------------------------------------------------
/**
 * Set the value for an UnsignedInt parameter.
 *
 * @param <id> The integer ID for the parameter.
 * @param <value> The new parameter value.
 *
 * @return the parameter value at the end of this call, or
 *         UNSIGNED_INT_PARAMETER_UNDEFINED if the parameter id is invalid or
 *         the parameter type is not an UnsignedInt.
 */
//------------------------------------------------------------------------------
UnsignedInt GmatBase::SetUnsignedIntParameter(const Integer id,
                                              const UnsignedInt value)
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot set unsigned integer parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  UnsignedInt GetUnsignedIntParameter(const Integer id, const Integer index) const
//---------------------------------------------------------------------------
/**
* Retrieve the value for an UnsignedInt parameter.
 *
 * @param <id> The integer ID for the parameter.
 * @param index Index for parameters in arrays.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
UnsignedInt GmatBase::GetUnsignedIntParameter(const Integer id,
                                              const Integer index) const
{
   wxString indexString, idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   indexString << index;
   throw GmatBaseException(wxT("Cannot get unsigned integer parameter with ID ") +
                           idString + wxT(" and index ") + indexString +
                           wxT(" on ") + typeName + wxT(" named \"") + instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  UnsignedInt SetUnsignedIntParameter(const Integer id,
//                                      const UnsignedInt value,
//                                      const Integer index)
//---------------------------------------------------------------------------
/**
* Set the value for an UnsignedInt parameter.
 *
 * @param <id> The integer ID for the parameter.
 * @param <value> The new parameter value.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return the parameter value at the end of this call, or
 *         UNSIGNED_INT_PARAMETER_UNDEFINED if the parameter id is invalid or
 *         the parameter type is not an UnsignedInt.
 */
//------------------------------------------------------------------------------
UnsignedInt GmatBase::SetUnsignedIntParameter(const Integer id,
                                              const UnsignedInt value,
                                              const Integer index)
{
   wxString indexString, idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   indexString << index;
   throw GmatBaseException(wxT("Cannot set unsigned integer parameter with ID ") +
                           idString + wxT(" and index ") + indexString +
                           wxT(" on ") + typeName + wxT(" named \"") + instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  const UnsignedIntArray& GetUnsignedIntArrayParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Access an array of unsigned int data.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The requested UnsignedIntArray; throws if the parameter is not a
 *         UnsignedIntArray.
 */
//------------------------------------------------------------------------------
const UnsignedIntArray& GmatBase::GetUnsignedIntArrayParameter(const Integer id) const
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot get unsigned integer array parameter with ")
                           wxT(" ID ") + idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}

//---------------------------------------------------------------------------
//  const IntegerArray& GetIntegerArrayParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Access an array of integer data.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The requested IntegerArray; throws if the parameter is not a
 *         IntegerArray.
 */
const IntegerArray& GmatBase::GetIntegerArrayParameter(const Integer id) const
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot get integer array parameter with ")
                           wxT(" ID ") + idString +
                           wxT(" on ") + typeName + wxT(" named ") + instanceName);
}

//---------------------------------------------------------------------------
//  const IntegerArray& GetIntegerArrayParameter(const Integer id,
//                                             const Integer index) const
//---------------------------------------------------------------------------
/**
 * Access an array of integer data.
 *
 * @param id The integer ID for the parameter.
 * @param index The index when multiple IntegerArrays are supported.
 *
 * @return The requested IntegerArray; throws if the parameter is not a
 *         IntegerArray.
 */
const IntegerArray& GmatBase::GetIntegerArrayParameter(const Integer id,
                                               const Integer index) const
{
   wxString indexString, idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   indexString << index;
   throw GmatBaseException(wxT("Cannot get integer array parameter with ID ") +
                           idString + wxT(" and index ") + indexString +
                           wxT(" on ") + typeName + wxT(" named ") + instanceName);
}

//---------------------------------------------------------------------------
//  const Rvector& GetRvectorParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
* Retrieve the value for an Rvector parameter.
*
* @param <id> The integer ID for the parameter.
*
* @return The parameter's value.
*/
//------------------------------------------------------------------------------
const Rvector& GmatBase::GetRvectorParameter(const Integer id) const
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot get Rvector parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  const Rvector& SetRvectorParameter(const Integer id, const Rvector& value)
//---------------------------------------------------------------------------
/**
* Set the value for an Rvector parameter.
*
* @param <id> The integer ID for the parameter.
* @param <value> The new parameter value.
*
* @return the parameter value at the end of this call, or
*         RVECTOR_PARAMETER_UNDEFINED if the parameter id is invalid or the
*         parameter type is not Rvector.
*/
//------------------------------------------------------------------------------
const Rvector& GmatBase::SetRvectorParameter(const Integer id,
                                             const Rvector& value)
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot set Rvector parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}

//---------------------------------------------------------------------------
//  const Rmatrix& GetRmatrixParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
* Retrieve the value for an Rmatrix parameter.
*
* @param <id> The integer ID for the parameter.
*
* @return The parameter's value.
*/
//------------------------------------------------------------------------------
const Rmatrix& GmatBase::GetRmatrixParameter(const Integer id) const
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot get Rmatrix parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  const Rmatrix& SetRmatrixParameter(const Integer id, const Rmatrix& value)
//---------------------------------------------------------------------------
/**
* Set the value for an Rmatrix parameter.
*
* @param <id> The integer ID for the parameter.
* @param <value> The new parameter value.
*
* @return the parameter value at the end of this call, or
*         RMATRIX_PARAMETER_UNDEFINED if the parameter id is invalid or the
*         parameter type is not Rmatrix.
*/
//------------------------------------------------------------------------------
const Rmatrix& GmatBase::SetRmatrixParameter(const Integer id,
                                             const Rmatrix& value)
{
   return RMATRIX_PARAMETER_UNDEFINED;
}


//---------------------------------------------------------------------------
//  wxString GetStringParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve a string parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The string stored for this parameter, or throw ab=n exception if
 *         there is no string association.
 */
//------------------------------------------------------------------------------
wxString GmatBase::GetStringParameter(const Integer id) const
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot get string parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  bool SetStringParameter(const Integer id, const wxString &value)
//---------------------------------------------------------------------------
/**
 * Change the value of a string parameter.
 *
 * @param <id> The integer ID for the parameter.
 * @param <value> The new string for this parameter.
 *
 * @return true if the string is stored, throw if the parameter is not stored.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetStringParameter(const Integer id, const wxString &value)
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot set string parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  wxString GetStringParameter(const Integer id, const Integer index) const
//---------------------------------------------------------------------------
/**
 * Retrieve a string parameter.
 *
 * @param id The integer ID for the parameter.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return The string stored for this parameter, or the empty string if there
 *         is no string association.
 */
//------------------------------------------------------------------------------
wxString GmatBase::GetStringParameter(const Integer id,
                                         const Integer index) const
{
   wxString indexString, idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   indexString << index;
   throw GmatBaseException(wxT("Cannot get string parameter with ID ") +
                           idString + wxT(" and index ") + indexString +
                           wxT(" on ") + typeName + wxT(" named \"") + instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  bool SetStringParameter(const Integer id, const wxString &value,
//                          const Integer index)
//---------------------------------------------------------------------------
/**
 * Change the value of a string parameter.
 *
 * @param id The integer ID for the parameter.
 * @param value The new string for this parameter.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return true if the string is stored, false if not.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetStringParameter(const Integer id, const wxString &value,
                                  const Integer index)
{
   wxString indexString, idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   indexString << index;
   throw GmatBaseException(wxT("Cannot set string parameter with ID ") +
                           idString + wxT(" and index ") + indexString +
                           wxT(" on ") + typeName + wxT(" named \"") + instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  const StringArray& GetStringArrayParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Access an array of string data.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The requested StringArray; throws if the parameter is not a
 *         StringArray.
 */
//------------------------------------------------------------------------------
const StringArray& GmatBase::GetStringArrayParameter(const Integer id) const
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot get string array parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  const StringArray& GetStringArrayParameter(const Integer id,
//                                             const Integer index) const
//---------------------------------------------------------------------------
/**
 * Access an array of string data.
 *
 * @param id The integer ID for the parameter.
 * @param index The index when multiple StringArrays are supported.
 *
 * @return The requested StringArray; throws if the parameter is not a
 *         StringArray.
 */
//------------------------------------------------------------------------------
const StringArray& GmatBase::GetStringArrayParameter(const Integer id,
                                               const Integer index) const
{
   wxString indexString, idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   indexString << index;
   throw GmatBaseException(wxT("Cannot get string array parameter with ID ") +
                           idString + wxT(" and index ") + indexString +
                           wxT(" on ") + typeName + wxT(" named \"") + instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  wxString GetOnOffParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve a On/Off parameter.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The string stored for this parameter, or throw an exception if
 *         there is no string association.
 */
//------------------------------------------------------------------------------
wxString GmatBase::GetOnOffParameter(const Integer id) const
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot get On/Off parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  bool SetOnOffParameter(const Integer id, const wxString &value)
//---------------------------------------------------------------------------
/**
 * Change the value of a string parameter.
 *
 * @param <id> The integer ID for the parameter.
 * @param <value> The new string for this parameter.
 *
 * @return true if the string is stored, throw if the parameter is not stored.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetOnOffParameter(const Integer id, const wxString &value)
{
   wxString idString;
   idString << id << wxT(": \"") << GetParameterText(id) << wxT("\"");
   throw GmatBaseException(wxT("Cannot set On/Off parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  const wxString GetCommentLine() const
//---------------------------------------------------------------------------
const wxString GmatBase::GetCommentLine() const
{
   return commentLine;
}

//---------------------------------------------------------------------------
//  void SetCommentLine(const wxString &comment)
//---------------------------------------------------------------------------
void GmatBase::SetCommentLine(const wxString &comment)
{
   commentLine = comment;
}

//---------------------------------------------------------------------------
//  const wxString GetInlineComment() const
//---------------------------------------------------------------------------
const wxString GmatBase::GetInlineComment() const
{
   return inlineComment;
}

//---------------------------------------------------------------------------
//  void SetInlineComment(const wxString &comment)
//---------------------------------------------------------------------------
void GmatBase::SetInlineComment(const wxString &comment)
{
   inlineComment = comment;
}

//---------------------------------------------------------------------------
//  const wxString GetAttributeCommentLine(Integer index))
//---------------------------------------------------------------------------
const wxString GmatBase::GetAttributeCommentLine(Integer index)
{
   if (index >= (Integer)attributeCommentLines.size())
   {
      #ifdef DEBUG_COMMENTS_ATTRIBUTE
      MessageInterface::ShowMessage
         (wxT("Attribute comment name:%s index:%d has not been retrieved.\n"),
          instanceName.c_str(), index);
      #endif
      return wxT("");
   }

   #ifdef DEBUG_COMMENTS_ATTRIBUTE
   MessageInterface::ShowMessage
      (wxT("Getting Attribute comment name:%s index:%d - %s.\n"),
       instanceName.c_str(), index, attributeCommentLines[index].c_str());
   #endif
   return attributeCommentLines[index];
}

//---------------------------------------------------------------------------
//  void SetAttributeCommentLine(Integer index, const wxString &comment)
//---------------------------------------------------------------------------
void GmatBase::SetAttributeCommentLine(Integer index,
                                       const wxString &comment)
{
   if (index >= (Integer)attributeCommentLines.size())
   {
      #ifdef DEBUG_COMMENTS_ATTRIBUTE
      MessageInterface::ShowMessage(wxT("Attribute comment index:%d - %s - has not been set. Size=%d\n"),
         index, comment.c_str(), (Integer)attributeCommentLines.size());
      #endif
      return;
   }

   #ifdef DEBUG_COMMENTS_ATTRIBUTE
   MessageInterface::ShowMessage
      (wxT("Setting Attribute comment index:%d - %s.\n"), index, comment.c_str());
   #endif

   attributeCommentLines[index] = comment;

}

//---------------------------------------------------------------------------
//  const wxString GetInlineAttributeComment(Integer index)
//---------------------------------------------------------------------------
const wxString GmatBase::GetInlineAttributeComment(Integer index)
{
   if (index >= (Integer)attributeInlineComments.size())
   {
      #ifdef DEBUG_COMMENTS_ATTRIBUTE
      MessageInterface::ShowMessage
         (wxT("Inline attribute comment name:%s index:%d has not been retrieved.\n"),
          instanceName.c_str(), index);
      #endif
      return wxT("");
   }

   #ifdef DEBUG_COMMENTS_ATTRIBUTE
   MessageInterface::ShowMessage
      (wxT("Getting Inline attribute comment name:%s index:%d - %s.\n"),
       instanceName.c_str(), index, attributeInlineComments[index].c_str());
   #endif

   return attributeInlineComments[index];
}

//---------------------------------------------------------------------------
//  void SetInlineAttributeComment(Integer index, const wxString &comment)
//---------------------------------------------------------------------------
void GmatBase::SetInlineAttributeComment(Integer index,
                                         const wxString &comment)
{
   if (index >= (Integer)attributeInlineComments.size())
   {
      #ifdef DEBUG_COMMENTS_ATTRIBUTE
      MessageInterface::ShowMessage
         (wxT("Inline attribute comment - %s - has not been set. Size=%d\n"),
          comment.c_str(), (Integer)attributeInlineComments.size());
      #endif
      return;
   }

   #ifdef DEBUG_COMMENTS_ATTRIBUTE
   MessageInterface::ShowMessage
      (wxT("Setting Inline attribute comment - %s.\n"), comment.c_str());
   #endif
   attributeInlineComments[index] = comment;
}

//---------------------------------------------------------------------------
//  bool GetBooleanParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve a boolean parameter.
 *
 * @param  id  The integer ID for the parameter.
 *
 * @return the boolean value for this parameter, or throw an exception if the
 *         parameter access in invalid.
 */
//------------------------------------------------------------------------------
bool GmatBase::GetBooleanParameter(const Integer id) const
{
   wxString idString;
   idString << id;
   throw GmatBaseException(wxT("Cannot get boolean parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
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
//------------------------------------------------------------------------------
bool GmatBase::SetBooleanParameter(const Integer id, const bool value)
{
   wxString idString;
   idString << id;
   throw GmatBaseException(wxT("Cannot set boolean parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  bool GetBooleanParameter(const Integer id, const Integer index) const
//---------------------------------------------------------------------------
/**
 * Retrieve a boolean parameter.
 *
 * @param id The integer ID for the parameter.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return the boolean value for this parameter, or false if the parameter is
 *         not boolean.
 */
//------------------------------------------------------------------------------
bool GmatBase::GetBooleanParameter(const Integer id, const Integer index) const
{
   wxString indexString, idString;
   idString << id;
   indexString << index;
   throw GmatBaseException(wxT("Cannot get boolean parameter with ID ") +
                           idString + wxT(" and index ") + indexString +
                           wxT(" on ") + typeName + wxT(" named \"") + instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  bool SetBooleanParameter(const Integer id, const bool value,
//                           const Integer index)
//---------------------------------------------------------------------------
/**
 * Sets the value for a boolean parameter.
 *
 * @param id The integer ID for the parameter.
 * @param value The new value for the parameter.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return the boolean value for this parameter, or false if the parameter is
 *         not boolean.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetBooleanParameter(const Integer id, const bool value,
                                   const Integer index)
{
   wxString indexString, idString;
   idString << id;
   indexString << index;
   throw GmatBaseException(wxT("Cannot set boolean parameter with ID ") +
                           idString + wxT(" and index ") + indexString +
                           wxT(" on ") + typeName + wxT(" named \"") + instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//const BooleanArray& GetBooleanArrayParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for a BooleanArray parameter.
 *
 * @param id The integer ID for the parameter.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
const BooleanArray& GmatBase::GetBooleanArrayParameter(const Integer id) const
{
   wxString idString;
   idString << id;
   throw GmatBaseException(wxT("Cannot get BooleanArray parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//const BooleanArray& GetBooleanArrayParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for a BooleanArray parameter.
 *
 * @param label The string label for the parameter.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
const BooleanArray& GmatBase::GetBooleanArrayParameter(const wxString &label) const
{
   Integer id = GetParameterID(label);
   return GetBooleanArrayParameter(id);
}


//---------------------------------------------------------------------------
//  bool SetBooleanArrayParameter(const Integer id, const BooleanArray &valueArray)
//---------------------------------------------------------------------------
/**
 * Sets the value for a BooleanArray parameter.
 *
 * @param id The integer ID for the parameter.
 * @param valueArray The string of new value list enclosed with brackets,
 *                  such as [true false]
 *
 * @return true if value set successfully, or throw an exception if the
 *         parameter is invalid or not boolean.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetBooleanArrayParameter(const Integer id,
                                        const BooleanArray &valueArray)
{
   wxString idString;
   idString << id;
   throw GmatBaseException(wxT("Cannot set BooleanArray parameter with ID ") +
                           idString + wxT(" on ") + typeName + wxT(" named \"") +
                           instanceName + wxT("\""));
}


//---------------------------------------------------------------------------
//  bool SetBooleanArrayParameter(const wxString &label,
//                                const BooleanArray &valueArray)
//---------------------------------------------------------------------------
/**
 * Sets the value for a BooleanArray parameter.
 *
 * @param  label  The (string) label for the parameter.
 * @param  valueArray  The string of new value list enclosed with brackets,
 *                    such as [true false]
 *
 * @return true if value set successfully, or throw an exception if the
 *         parameter is invalid or not boolean.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetBooleanArrayParameter(const wxString &label,
                                        const BooleanArray &valueArray)
{
   Integer id = GetParameterID(label);
   return SetBooleanArrayParameter(id, valueArray);
}


//---------------------------------------------------------------------------
//  Real GetRealParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for a Real parameter.
 *
 * @param <label> The (string) label for the parameter.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
Real GmatBase::GetRealParameter(const wxString &label) const
{
   Integer id = GetParameterID(label);
   return GetRealParameter(id);
}


//---------------------------------------------------------------------------
//  Real SetRealParameter(const wxString &label, const Real value)
//---------------------------------------------------------------------------
/**
 * Set the value for a Real parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param <value> The new parameter value.
 *
 * @return the parameter value at the end of this call, or
 *         REAL_PARAMETER_UNDEFINED if the parameter id is invalid or the
 *         parameter type is not Real.
 */
//------------------------------------------------------------------------------
Real GmatBase::SetRealParameter(const wxString &label, const Real value)
{
   Integer id = GetParameterID(label);
   return SetRealParameter(id, value);
}


//---------------------------------------------------------------------------
//  Real GetRealParameter(const wxString &label, const Integer index) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for a Real parameter.
 *
 * @param label The (string) label for the parameter.
 * @param index Index for parameters in arrays.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
Real GmatBase::GetRealParameter(const wxString &label,
                                const Integer index) const
{
   Integer id = GetParameterID(label);
   return GetRealParameter(id, index);
}


//---------------------------------------------------------------------------
//  Real GetRealParameter(const wxString &label, const Integer row,
//                        const Integer col) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for a Real parameter from an array.
 *
 * @param label The (string) label for the parameter.
 * @param row Row index for parameters in arrays.
 * @param col Column index for parameters in arrays.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
Real GmatBase::GetRealParameter(const wxString &label, const Integer row,
                                const Integer col) const
{
   Integer id = GetParameterID(label);
   return GetRealParameter(id, row, col);
}



//---------------------------------------------------------------------------
//  Real SetRealParameter(const wxString &label, const Real value,
//                        const Integer index)
//---------------------------------------------------------------------------
/**
 * Set the value for a Real parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param <value> The new parameter value.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return the parameter value at the end of this call, or
 *         REAL_PARAMETER_UNDEFINED if the parameter id is invalid or the
 *         parameter type is not Real.
 */
//------------------------------------------------------------------------------
Real GmatBase::SetRealParameter(const wxString &label, const Real value,
                                const Integer index)
{
   Integer id = GetParameterID(label);
   return SetRealParameter(id, value, index);
}


//---------------------------------------------------------------------------
//  Real SetRealParameter(const wxString &label, const Real value,
//                        const Integer row, const Integer col)
//---------------------------------------------------------------------------
/**
 * Set the value for a Real parameter in an array.
 *
 * @param <label> The (string) label for the parameter.
 * @param <value> The new parameter value.
 * @param <row>   The row that receives the value.
 * @param <col>   The column that receives the value value.
 *
 * @return the parameter value at the end of this call.
 */
//---------------------------------------------------------------------------
Real GmatBase::SetRealParameter(const wxString &label, const Real value,
                      const Integer row, const Integer col)
{
   Integer id = GetParameterID(label);
   return SetRealParameter(id, value, row, col);
}


//---------------------------------------------------------------------------
//  Integer GetIntegerParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
* Retrieve the value for an Integer parameter.
 *
 * @param <label> The (string) label for the parameter.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
Integer GmatBase::GetIntegerParameter(const wxString &label) const
{
   Integer id = GetParameterID(label);
   return GetIntegerParameter(id);
}


//---------------------------------------------------------------------------
//  Integer SetIntegerParameter(const wxString &label, const Integer value)
//---------------------------------------------------------------------------
/**
* Set the value for an Integer parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param <value> The new parameter value.
 *
 * @return the parameter value at the end of this call, or
 *         INTEGER_PARAMETER_UNDEFINED if the parameter id is invalid or the
 *         parameter type is not an Integer.
 */
//------------------------------------------------------------------------------
Integer GmatBase::SetIntegerParameter(const wxString &label,
                                      const Integer value)
{
   Integer id = GetParameterID(label);
   return SetIntegerParameter(id, value);
}

//---------------------------------------------------------------------------
//  Integer GetIntegerParameter(const wxString &label,
//                              const Integer index) const
//---------------------------------------------------------------------------
/**
* Retrieve the value for an Integer parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param index Index for parameters in arrays.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
Integer GmatBase::GetIntegerParameter(const wxString &label,
                                      const Integer index) const
{
   Integer id = GetParameterID(label);
   return GetIntegerParameter(id, index);
}


//---------------------------------------------------------------------------
//  Integer SetIntegerParameter(const wxString &label, const Integer value,
//                              const Integer index)
//---------------------------------------------------------------------------
/**
* Set the value for an Integer parameter.
 *
 * @param label The (string) label for the parameter.
 * @param value The new parameter value.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return the parameter value at the end of this call, or
 *         INTEGER_PARAMETER_UNDEFINED if the parameter id is invalid or the
 *         parameter type is not an Integer.
 */
//------------------------------------------------------------------------------
Integer GmatBase::SetIntegerParameter(const wxString &label,
                                      const Integer value,
                                      const Integer index)
{
   Integer id = GetParameterID(label);
   return SetIntegerParameter(id, value, index);
}

//---------------------------------------------------------------------------
//  UnsignedInt GetUnsignedIntParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for an UnsignedInt parameter.
 *
 * @param <label> The (string) label for the parameter.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
UnsignedInt GmatBase::GetUnsignedIntParameter(const wxString &label) const
{
   Integer id = GetParameterID(label);
   return GetUnsignedIntParameter(id);
}


//---------------------------------------------------------------------------
//  UnsignedInt SetUnsignedIntParameter(const wxString &label,
//                                      const UnsignedInt value)
//---------------------------------------------------------------------------
/**
* Set the value for an UnsignedInt parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param <value> The new parameter value.
 *
 * @return the parameter value at the end of this call, or
 *         UNSIGNED_INT_PARAMETER_UNDEFINED if the parameter id is invalid
 *         or the parameter type is not an UnsignedInt.
 */
//------------------------------------------------------------------------------
UnsignedInt GmatBase::SetUnsignedIntParameter(const wxString &label,
                                              const UnsignedInt value)
{
   Integer id = GetParameterID(label);
   return SetUnsignedIntParameter(id, value);
}

//---------------------------------------------------------------------------
//  UnsignedInt GetUnsignedIntParameter(const wxString &label,
//                                      const Integer index) const
//---------------------------------------------------------------------------
/**
 * Retrieve the value for an UnsignedInt parameter.
 *
 * @param label The (string) label for the parameter.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return The parameter's value.
 */
//------------------------------------------------------------------------------
UnsignedInt GmatBase::GetUnsignedIntParameter(const wxString &label,
                                              const Integer index) const
{
   Integer id = GetParameterID(label);
   return GetUnsignedIntParameter(id, index);
}


//---------------------------------------------------------------------------
//  UnsignedInt SetUnsignedIntParameter(const wxString &label,
//                                      const UnsignedInt value,
//                                      const Integer index)
//---------------------------------------------------------------------------
/**
* Set the value for an UnsignedInt parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param <value> The new parameter value.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return the parameter value at the end of this call, or
 *         UNSIGNED_INT_PARAMETER_UNDEFINED if the parameter id is invalid
 *         or the parameter type is not an UnsignedInt.
 */
//------------------------------------------------------------------------------
UnsignedInt GmatBase::SetUnsignedIntParameter(const wxString &label,
                                              const UnsignedInt value,
                                              const Integer index)
{
   Integer id = GetParameterID(label);
   return SetUnsignedIntParameter(id, value, index);
}

//---------------------------------------------------------------------------
//  const UnsignedIntArray&
//  GetUnsignedIntArrayParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Access an array of unsigned int data.
 *
 * @param <label> The (string) label for the parameter.
 *
 * @return The requested UnsignedIntArray; throws if the parameter is not a
 *         UnsignedIntArray.
 */
//------------------------------------------------------------------------------
const UnsignedIntArray&
GmatBase::GetUnsignedIntArrayParameter(const wxString &label) const
{
   Integer id = GetParameterID(label);
   return GetUnsignedIntArrayParameter(id);
}

//---------------------------------------------------------------------------
// const  Rvector& GetRvectorParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
* Retrieve the value for an Rvector parameter.
*
* @param <label> The (string) label for the parameter.
*
* @return The parameter's value.
*/
//------------------------------------------------------------------------------
const Rvector& GmatBase::GetRvectorParameter(const wxString &label) const
{
   Integer id = GetParameterID(label);
   return GetRvectorParameter(id);
}


//---------------------------------------------------------------------------
//  const Rvector& SetRvectorParameter(const wxString &label,
//                                     const Rvector& value)
//---------------------------------------------------------------------------
/**
* Set the value for an Rvector parameter.
*
* @param <label> The (string) label for the parameter.
* @param <value> The new parameter value.
*
* @return the parameter value at the end of this call, or
*         RVECTOR_PARAMETER_UNDEFINED if the parameter id is invalid or the
*         parameter type is not Rvector.
*/
//------------------------------------------------------------------------------
const Rvector& GmatBase::SetRvectorParameter(const wxString &label,
                                             const Rvector& value)
{
   Integer id = GetParameterID(label);
   return SetRvectorParameter(id, value);
}

//---------------------------------------------------------------------------
//  const Rmatrix& GetRmatrixParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
* Retrieve the value for an Rmatrix parameter.
*
* @param <label> The (string) label for the parameter.
*
* @return The parameter's value.
*/
//------------------------------------------------------------------------------
const Rmatrix& GmatBase::GetRmatrixParameter(const wxString &label) const
{
   Integer id = GetParameterID(label);
   return GetRmatrixParameter(id);
}


//---------------------------------------------------------------------------
//  const Rmatrix& SetRmatrixParameter(const wxString &label,
//                                     const Rmatrix& value)
//---------------------------------------------------------------------------
/**
* Set the value for an Rmatrix parameter.
*
* @param <label> The (string) label for the parameter.
* @param <value> The new parameter value.
*
* @return the parameter value at the end of this call, or
*         RMATRIX_PARAMETER_UNDEFINED if the parameter id is invalid or the
*         parameter type is not Rmatrix.
*/
//------------------------------------------------------------------------------
const Rmatrix& GmatBase::SetRmatrixParameter(const wxString &label,
                                             const Rmatrix& value)
{
   Integer id = GetParameterID(label);
   return SetRmatrixParameter(id, value);
}

//---------------------------------------------------------------------------
//  wxString GetStringParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Retrieve a string parameter.
 *
 * @param <label> The (string) label for the parameter.
 *
 * @return The string stored for this parameter, or the empty string if there
 *         is no string association.
 */
//------------------------------------------------------------------------------
wxString GmatBase::GetStringParameter(const wxString &label) const
{
   Integer id = GetParameterID(label);
   return GetStringParameter(id);
}

//---------------------------------------------------------------------------
//  bool SetStringParameter(const wxString &label, const wxString &value)
//---------------------------------------------------------------------------
/**
 * Change the value of a string parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param <value> The new string for this parameter.
 *
 * @return true if the string is stored, false if not.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetStringParameter(const wxString &label,
                                  const wxString &value)
{
   Integer id = GetParameterID(label);
   return SetStringParameter(id, value);
}


//---------------------------------------------------------------------------
//  wxString GetStringParameter(const wxString &label,
//                                 const Integer index) const
//---------------------------------------------------------------------------
/**
 * Retrieve a string parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param index Index for parameters in arrays.
 *
 * @return The string stored for this parameter, or the empty string if there
 *         is no string association.
 */
//------------------------------------------------------------------------------
wxString GmatBase::GetStringParameter(const wxString &label,
                                         const Integer index) const
{
   Integer id = GetParameterID(label);
   return GetStringParameter(id, index);
}

//---------------------------------------------------------------------------
//  bool SetStringParameter(const wxString &label, const wxString &value,
//                          const Integer index)
//---------------------------------------------------------------------------
/**
 * Change the value of a string parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param <value> The new string for this parameter.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return true if the string is stored, false if not.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetStringParameter(const wxString &label,
                                  const wxString &value,
                                  const Integer index)
{
   Integer id = GetParameterID(label);
   return SetStringParameter(id, value, index);
}


//---------------------------------------------------------------------------
//  const StringArray& GetStringArrayParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Access an array of string data.
 *
 * @param label The (string) label for the parameter.
 *
 * @return The requested StringArray; throws if the parameter is not a
 *         StringArray.
 */
//------------------------------------------------------------------------------
const StringArray& GmatBase::GetStringArrayParameter(
                                          const wxString &label) const
{
   Integer id = GetParameterID(label);
   return GetStringArrayParameter(id);
}


//---------------------------------------------------------------------------
//  const StringArray& GetStringArrayParameter(const wxString &label,
//                                             const Integer index) const
//---------------------------------------------------------------------------
/**
 * Access an array of string data.
 *
 * @param label The (string) label for the parameter.
 * @param index Which string array if more than one is supported.
 *
 * @return The requested StringArray; throws if the parameter is not a
 *         StringArray.
 */
//------------------------------------------------------------------------------
const StringArray& GmatBase::GetStringArrayParameter(const wxString &label,
                                               const Integer index) const
{
   Integer id = GetParameterID(label);
   return GetStringArrayParameter(id, index);
}

//---------------------------------------------------------------------------
//  bool GetBooleanParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Retrieve a boolean parameter.
 *
 * @param <label> The (string) label for the parameter.
 *
 * @return the boolean value for this parameter, or false if the parameter is
 *         not boolean.
 */
//------------------------------------------------------------------------------
bool GmatBase::GetBooleanParameter(const wxString &label) const
{
   Integer id = GetParameterID(label);
   return GetBooleanParameter(id);
}


//---------------------------------------------------------------------------
//  bool SetBooleanParameter(const wxString &label, const bool value)
//---------------------------------------------------------------------------
/**
 * Sets the value for a boolean parameter.
 *
 * @param <label> The (string) label for the parameter.
 *
 * @return the boolean value for this parameter, or false if the parameter is
 *         not boolean.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetBooleanParameter(const wxString &label, const bool value)
{
   Integer id = GetParameterID(label);
   return SetBooleanParameter(id, value);
}


//---------------------------------------------------------------------------
//  bool GetBooleanParameter(const wxString &label, const Integer index) const
//---------------------------------------------------------------------------
/**
 * Retrieve a boolean parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param index Index for parameters in arrays.
 *
 * @return the boolean value for this parameter, or false if the parameter is
 *         not boolean.
 */
//------------------------------------------------------------------------------
bool GmatBase::GetBooleanParameter(const wxString &label,
                                   const Integer index) const
{
   Integer id = GetParameterID(label);
   return GetBooleanParameter(id, index);
}


//---------------------------------------------------------------------------
//  bool SetBooleanParameter(const wxString &label, const bool value)
//---------------------------------------------------------------------------
/**
 * Sets the value for a boolean parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param index Index for parameters in arrays.  Use -1 or the index free
 *              version to add the value to the end of the array.
 *
 * @return the boolean value for this parameter, or false if the parameter is
 *         not boolean.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetBooleanParameter(const wxString &label, const bool value,
                                   const Integer index)
{
   Integer id = GetParameterID(label);
   return SetBooleanParameter(id, value, index);
}


//---------------------------------------------------------------------------
//  wxString GetOnOffParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * Retrieve an On/Off parameter.
 *
 * @param <label> The (string) label for the parameter.
 *
 * @return The wxT("On/Off") string stored for this parameter.
 */
//------------------------------------------------------------------------------
wxString GmatBase::GetOnOffParameter(const wxString &label) const
{
   Integer id = GetParameterID(label);
   return GetOnOffParameter(id);
}


//---------------------------------------------------------------------------
//  bool SetStringParameter(const wxString &label, const wxString &value)
//---------------------------------------------------------------------------
/**
 * Change the value of an On/Off parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param <value> The new wxT("On/Off") string for this parameter.
 *
 * @return true if the string is stored, false if not.
 */
//------------------------------------------------------------------------------
bool GmatBase::SetOnOffParameter(const wxString &label,
                                 const wxString &value)
{
   Integer id = GetParameterID(label);
   return SetOnOffParameter(id, value);
}


//---------------------------------------------------------------------------
//  bool TakeAction(const wxString &action, const wxString &actionData)
//---------------------------------------------------------------------------
/**
 * Interface used to support user actions.
 *
 * @param <action> The string descriptor for the requested action.
 * @param <actionData> Optional data used for the action.
 *
 * @return true if the action was performed, false if not.
 */
//------------------------------------------------------------------------------
bool GmatBase::TakeAction(const wxString &action,
                          const wxString &actionData)
{
   return false;
}

//---------------------------------------------------------------------------
//  bool TakeRequiredAction(const Integer id))
//---------------------------------------------------------------------------
/**
 * Tells object to take whatever action it needs to take before the value
 * of the specified parameter is set (e.g. clearing out arrays)
 *
 * @param <id> parameter for which to take prerequisite action.
 *
 * @return true if the action was performed (or none needed), false if not.
 */
//------------------------------------------------------------------------------
bool GmatBase::TakeRequiredAction(const Integer id)
{
   return true;
}

//---------------------------------------------------------------------------
//  bool TakeRequiredAction(const wxString &label)
//---------------------------------------------------------------------------
/**
 * Tells object to take whatever action it needs to take before the value
 * of the specified parameter is set (e.g. clearing out arrays)
 *
 * @param <label> parameter for which to take prerequisite action.
 *
 * @return true if the action was performed (or none needed), false if not.
 */
//------------------------------------------------------------------------------
bool GmatBase::TakeRequiredAction(const wxString &label)
{
   Integer id = GetParameterID(label);
   return TakeRequiredAction(id);
}


//------------------------------------------------------------------------------
// const std::vector<Gmat::ObjectType>& GetTypesForList(const Integer id)
//------------------------------------------------------------------------------
/**
 * Retrieves a list of types that need to be shown on a GUI for a parameter
 *
 * @param id The parameter ID
 *
 * @return The list of types
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& GmatBase::GetTypesForList(const Integer id)
{
   listedTypes.clear();
   return listedTypes;
}

//------------------------------------------------------------------------------
// const std::vector<Gmat::ObjectType>&
//       GetTypesForList(const wxString &label)
//------------------------------------------------------------------------------
/**
 * Retrieves a list of types that need to be shown on a GUI for a parameter
 *
 * @param label The parameter's identifying string
 *
 * @return The list of types
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& GmatBase::GetTypesForList(const wxString &label)
{
   return GetTypesForList(GetParameterID(label));
}


//------------------------------------------------------------------------------
// const wxString& GetGeneratingString(Gmat::WriteMode mode = Gmat::SCRIPTING,
//       const wxString &prefix = wxT(""), const wxString &useName = wxT(""))
//------------------------------------------------------------------------------
/**
 * Produces a string, possibly multi-line, containing the text that produces an
 * object.
 *
 * @param mode Specifies the type of serialization requested.
 * @param prefix Optional prefix appended to the object's name
 * @param useName Name that replaces the object's name.
 *
 * @return A string containing the text.
 */
//------------------------------------------------------------------------------
const wxString& GmatBase::GetGeneratingString(Gmat::WriteMode mode,
                                                 const wxString &prefix,
                                                 const wxString &useName)
{
   #ifdef DEBUG_GENERATING_STRING
   MessageInterface::ShowMessage
      (wxT("GmatBase::GetGeneratingString() <%p><%s>'%s' entered, mode=%d, prefix='%s', ")
       wxT("useName='%s', \n"), this, GetTypeName().c_str(), GetName().c_str(), mode,
       prefix.c_str(), useName.c_str());
   MessageInterface::ShowMessage
      (wxT("   showPrefaceComment=%d, commentLine=<%s>\n   showInlineComment=%d ")
       wxT("inlineComment=<%s>\n"),  showPrefaceComment, commentLine.c_str(),
       showInlineComment, inlineComment.c_str());
   #endif

   // don't write anything for cloaked objects, unless we're in SHOW_SCRIPT mode
   if ((mode != Gmat::SHOW_SCRIPT) && IsObjectCloaked())
   {
      generatingString = wxT("");
      return generatingString;
   }
   
   wxString data;

   wxString preface = wxT(""), nomme;

   if ((mode == Gmat::SCRIPTING) || (mode == Gmat::OWNED_OBJECT) ||
       (mode == Gmat::SHOW_SCRIPT))
      inMatlabMode = false;
   if (mode == Gmat::MATLAB_STRUCT || mode == Gmat::EPHEM_HEADER)
      inMatlabMode = true;

   if (useName != wxT(""))
      nomme = useName;
   else
      nomme = instanceName;

   if (mode == Gmat::SCRIPTING    || mode == Gmat::SHOW_SCRIPT ||
       mode == Gmat::EPHEM_HEADER || mode == Gmat::NO_COMMENTS)
   {
      wxString tname = typeName;
      if (tname == wxT("PropSetup"))
         tname = wxT("Propagator");

      if (mode == Gmat::EPHEM_HEADER)
      {
         data << tname << wxT(" = ") << wxT("'") << nomme << wxT("';\n");
         preface = wxT("");
      }
      else if (mode == Gmat::NO_COMMENTS)
      {
         #ifdef DEBUG_GENERATING_STRING
         MessageInterface::ShowMessage(wxT("==> Do not show comments\n"));
         #endif
         if (!cloaking)
            data << wxT("Create ") << tname << wxT(" ") << nomme << wxT(";\n");
         else
            data << wxT("");
         preface = wxT("GMAT ");
      }
      else
      {
         if (showPrefaceComment)
         {
            if ((commentLine != wxT("")) &&
                ((mode == Gmat::SCRIPTING) || (mode == Gmat::OWNED_OBJECT) ||
                 (mode == Gmat::SHOW_SCRIPT)))
               data << commentLine;
         }
         if (!cloaking)
            data << wxT("Create ") << tname << wxT(" ") << nomme << wxT(";");
         else
            data << wxT("");

         if (showInlineComment)
         {
            if ((inlineComment != wxT("")) &&
                ((mode == Gmat::SCRIPTING) || (mode == Gmat::OWNED_OBJECT) ||
                 (mode == Gmat::SHOW_SCRIPT)))
               data << inlineComment << wxT("\n");
            else
               data << wxT("\n");
         }
         else
         {
            data << wxT("\n");
         }

         preface = wxT("GMAT ");
      }
   }

   nomme += wxT(".");

   if (mode == Gmat::OWNED_OBJECT) 
   {
      preface = prefix;
      nomme = wxT("");
   }

   preface += nomme;
   WriteParameters(mode, preface, data);

   generatingString = data;
   return generatingString;
}


//------------------------------------------------------------------------------
// StringArray GetGeneratingStringArray(Gmat::WriteMode mode = Gmat::SCRIPTING,
//             const wxString &prefix = wxT(""), const wxString &useName = wxT(""))
//------------------------------------------------------------------------------
/**
 * Produces a string array containing the line-by-line text that produces an
 * object.
 *
 * @param mode Specifies the type of serialization requested.
 * @param prefix Optional prefix appended to the object's name
 * @param useName Name that replaces the object's name.
 *
 * @return A string array containing the text.
 *
 * @note The current implementation just calls GetGeneratingString, and returns
 *       a multiline string in the first element of the string array.  A later
 *       update is needed to break that string into multiple entries in the
 *       string array.
 */
//------------------------------------------------------------------------------
StringArray GmatBase::GetGeneratingStringArray(Gmat::WriteMode mode,
                                               const wxString &prefix,
                                               const wxString &useName)
{
   StringArray sar;
   wxString genstr = GetGeneratingString(mode, prefix, useName);
   wxString text;
   Integer start = 0, end = 0, len = genstr.length();

   while (end < len) {
      if (genstr[end] == wxT('\n')) {
         text = genstr.substr(start, end - start);
         sar.push_back(text);
         start = end+1;
      }
      ++end;
   }

   return sar;
}


//------------------------------------------------------------------------------
//  void FinalizeCreation()
//------------------------------------------------------------------------------
/**
 * Completes any post-construction steps needed before the object can be used.
 *
 * This method performs initialization of GmatBase properties that depend on the
 * features of the derived classes.  Derived classes touch some of the base
 * class properties -- the parameterCount, for example.  This method is called
 * after the object creation process is complete, so that any of the object's
 * base-class properties can be updated to reflect the object's actual
 * properties.
 */
//------------------------------------------------------------------------------
void GmatBase::FinalizeCreation()
{
   PrepCommentTables();
}


//------------------------------------------------------------------------------
// virtual wxString BuildPropertyName(GmatBase *ownedObj)
//------------------------------------------------------------------------------
/*
 * Builds property name of given owned object.
 * This method is called when special naming of owned object property is
 * required when writing object. For example, ForceModel requires additional
 * name Earth for GravityField as in FM.GravityField.Earth.Degree.
 *
 * @param obj The owned object that needs special property naming
 * @return The property name
 */
//------------------------------------------------------------------------------
wxString GmatBase::BuildPropertyName(GmatBase *ownedObj)
{
   return wxT("");
}


//------------------------------------------------------------------------------
// wxString GetLastErrorMessage()
//------------------------------------------------------------------------------
/**
 * Retrieves the last error message
 *
 * @return The format string.
 */
//------------------------------------------------------------------------------
wxString GmatBase::GetLastErrorMessage()
{
   return lastErrorMessage;
}


//------------------------------------------------------------------------------
// wxString GetErrorMessageFormat()
//------------------------------------------------------------------------------
/**
 * Retrieves the error message format string.
 *
 * @return The format string.
 */
//------------------------------------------------------------------------------
wxString GmatBase::GetErrorMessageFormat()
{
   return errorMessageFormat;
}


//------------------------------------------------------------------------------
// void SetErrorMessageFormat(const wxString &fmt)
//------------------------------------------------------------------------------
/**
 * Sets the error message format string.
 *
 * @param fmt The format string.
 */
//------------------------------------------------------------------------------
void GmatBase::SetErrorMessageFormat(const wxString &fmt)
{
   errorMessageFormat = fmt;
}

//-------------------------------------
// public static functions
//-------------------------------------

//---------------------------------------------------------------------------
//  static Integer GetInstanceCount()
//---------------------------------------------------------------------------
/**
 * Find out how many GmatBase objects have been instantiated.
 *
 * @return The number of instantiated objects.
 */
Integer GmatBase::GetInstanceCount()
{
   return GmatBase::instanceCount;
}

//---------------------------------------------------------------------------
// static Gmat::ObjectType GetObjectType(const wxString &typeString)
//---------------------------------------------------------------------------
/**
 * @param <typeString> object type string
 *
 * @return object type for given type string
 *
 */
//---------------------------------------------------------------------------
Gmat::ObjectType GmatBase::GetObjectType(const wxString &typeString)
{
   for (int i=0; i<Gmat::UNKNOWN_OBJECT - Gmat::SPACECRAFT; i++)
      if (OBJECT_TYPE_STRING[i] == typeString)
         return (Gmat::ObjectType)(i + Gmat::SPACECRAFT);

   return Gmat::UNKNOWN_OBJECT;
}

//---------------------------------------------------------------------------
// static wxString GetObjectTypeString(Gmat::ObjectType type)
//---------------------------------------------------------------------------
/**
 * @param <type> object type
 *
 * @return object type string for given type
 *
 */
//---------------------------------------------------------------------------
wxString GmatBase::GetObjectTypeString(Gmat::ObjectType type)
{
   return OBJECT_TYPE_STRING[type - Gmat::SPACECRAFT];
}

//------------------------------------------------------------------------------
// static Integer GmatBase::GetDataPrecision()
//------------------------------------------------------------------------------
Integer GmatBase::GetDataPrecision()
{
   return GmatGlobal::Instance()->GetDataPrecision();
}

//------------------------------------------------------------------------------
// static Integer GmatBase::GetTimePrecision()
//------------------------------------------------------------------------------
Integer GmatBase::GetTimePrecision()
{
   return GmatGlobal::Instance()->GetTimePrecision();
}


// todo: comments
Integer GmatBase::GetPropItemID(const wxString &whichItem)
{
   return -1;
}


Integer GmatBase::SetPropItem(const wxString &propItem)
{
   return Gmat::UNKNOWN_STATE;
}

StringArray GmatBase::GetDefaultPropItems()
{
   StringArray empty;
   return empty;
}

Real* GmatBase::GetPropItem(const Integer item)
{
   return NULL;
}

Integer GmatBase::GetPropItemSize(const Integer item)
{
   return -1;
}

bool GmatBase::PropItemNeedsFinalUpdate(const Integer item)
{
   return false;
}

bool GmatBase::HasAssociatedStateObjects()
{
   return false;
}

wxString GmatBase::GetAssociateName(UnsignedInt val)
{
   return wxT("");
}


Integer GmatBase::SetEstimationParameter(const wxString &param)
{
   return -1;
}

//------------------------------------------------------------------------------
// Integer GmatBase::GetEstimationParameterID(const wxString &param)
//------------------------------------------------------------------------------
/**
 * This method...
 *
 * @param param The text name of the estimation parameter
 *
 * @return The ID used in estimation for the parameter
 */
//------------------------------------------------------------------------------
Integer GmatBase::GetEstimationParameterID(const wxString &param)
{
   Integer id = type * ESTIMATION_TYPE_ALLOCATION; // Base for estimation ID

   try
   {
      id += GetParameterID(param);
   }
   catch (BaseException &)
   {
      return -1;
   }

   return id;
}


//------------------------------------------------------------------------------
// bool GmatBase::IsEstimationParameterValid(Integer id)
//------------------------------------------------------------------------------
/**
 * Derived classes override this method when they provide the methods used by
 * the estimators to build the elements needed for estimation.
 *
 * @param id The estimation parameter ID for the parameter
 *
 * @return true if estimation can proceed with the input parameter in the
 *         estimation state and related matrices, false otherwise
 */
//------------------------------------------------------------------------------
bool GmatBase::IsEstimationParameterValid(Integer id)
{
   return false;
}


Integer GmatBase::GetEstimationParameterSize(Integer id)
{
   Integer retval = 1;

   return retval;
}

Real* GmatBase::GetEstimationParameterValue(Integer id)
{
   return NULL;
}


//-------------------------------------
// protected methods
//-------------------------------------

//---------------------------------------------------------------------------
//  void CopyParameters(const GmatBase &a)
//---------------------------------------------------------------------------
/**
 * Copies the parameters from one object into this object.
 *
 * @param <a> The object that is being copied.
 */
//---------------------------------------------------------------------------
void GmatBase::CopyParameters(const GmatBase &a)
{
   Integer i, iVal;
   Real rVal;
   wxString sVal;
   bool bVal;
   Gmat::ParameterType parmType;

   for (i = 0; i < parameterCount; ++i) {
      parmType = a.GetParameterType(i);

      if (parmType == Gmat::REAL_TYPE)
      {
         rVal = a.GetRealParameter(i);
         SetRealParameter(i, rVal);
      }

      if (parmType == Gmat::INTEGER_TYPE)
      {
         iVal = a.GetIntegerParameter(i);
         SetIntegerParameter(i, iVal);
      }

      if ((parmType == Gmat::STRING_TYPE) || (parmType == Gmat::FILENAME_TYPE))
      {
         sVal = a.GetStringParameter(i);
         SetStringParameter(i, sVal);
      }

      if (parmType == Gmat::BOOLEAN_TYPE)
      {
         bVal = a.GetBooleanParameter(i);
         SetBooleanParameter(i, bVal);
      }
   }
}

//------------------------------------------------------------------------------
// void WriteParameters(Gmat::WriteMode mode, wxString &prefix,
//                      wxString &stream)
//------------------------------------------------------------------------------
/**
 * Code that writes the parameter details for an object.
 *
 * @param mode   The writing mode.
 * @param prefix Starting portion of the script string used for the parameter.
 * @param stream The stringstream the object is written to.
 */
//------------------------------------------------------------------------------
void GmatBase::WriteParameters(Gmat::WriteMode mode, wxString &prefix,
                               wxString &stream)
{
   #ifdef DEBUG_WRITE_PARAM
   MessageInterface::ShowMessage
      (wxT("GmatBase::WriteParameters() <%p><%s>'%s' entered, mode=%d, prefix='%s'\n"),
       this, GetTypeName().c_str(), GetName().c_str(), mode, prefix.c_str());
   #endif
   
   Integer i;
   Gmat::ParameterType parmType;
   wxString value;
   
   // Allow objects to muck with parameter counts, and use the mucked up value
   Integer paramCount = GetParameterCount();

   // Create parameter write order if it is empty (LOJ: 2009.02.13)
   if (parameterWriteOrder.empty())
   {
      for (i = 0; i < paramCount; ++i)
         parameterWriteOrder.push_back(i);
   }
   else
   {
      if ((Integer)parameterWriteOrder.size() < paramCount)
      {
         // Add GmatBase parameters since it's count is no longer 0 (LOJ: 2009.11.20)
         for (i = 0; i < GmatBaseParamCount; ++i)
            parameterWriteOrder.push_back(i);
         
         if ((Integer)parameterWriteOrder.size() != paramCount)
         {
            GmatBaseException gbe;
            gbe.SetDetails(wxT("GmatBase::WriteParameters(), there are more actual ")
                           wxT("parameters [%d] than ordered parameters [%d]\n"),
                           parameterCount, parameterWriteOrder.size());
         }
      }
   }
   
   Integer id;
   for (i = 0; i < paramCount; ++i)
   {
      id = parameterWriteOrder[i];
      
      #ifdef DEBUG_WRITE_PARAM
      MessageInterface::ShowMessage
         (wxT("   %2d, checking %s, type=%s, %s    %s\n"), i, GetParameterText(id).c_str(),
          PARAM_TYPE_STRING[GetParameterType(id)].c_str(),
          (IsParameterReadOnly(id) ? wxT("ReadOnly") : wxT("Writable")),
          (IsParameterCloaked(id) ? wxT("Cloaked") : wxT("Uncloaked")));
      #endif
      
      // in SHOW_SCRIPT, we write cloaked parameters; otherwise, we don't
      bool parameterIsToBeWritten = !IsParameterReadOnly(id);
      if (mode != Gmat::SHOW_SCRIPT) 
         parameterIsToBeWritten = parameterIsToBeWritten &&
                                  (!IsParameterCloaked(id));
      
      if (parameterIsToBeWritten)
      {
         parmType = GetParameterType(id);

         // Handle STRINGARRAY_TYPE or OBJECTARRAY_TYPE
         if (parmType == Gmat::STRINGARRAY_TYPE ||
             parmType == Gmat::OBJECTARRAY_TYPE)
         {
            #ifdef DEBUG_ARRAY_TYPE_PARAMETER_WRITING
               MessageInterface::ShowMessage(wxT("String array management for %s\n"),
                     GetParameterText(id).c_str());
            #endif
            bool writeQuotes = inMatlabMode || parmType == Gmat::STRINGARRAY_TYPE;

            StringArray sar = GetStringArrayParameter(id);
            if (sar.size() > 0)
            {
               wxString attCmtLn = GetAttributeCommentLine(id);

               if ((attCmtLn != wxT("")) && ((mode == Gmat::SCRIPTING) ||
                                        (mode == Gmat::OWNED_OBJECT) ||
                                        (mode == Gmat::SHOW_SCRIPT)))
               {
                  stream << attCmtLn.c_str();
               }

               stream << prefix << GetParameterText(id) << wxT(" = {");

               for (StringArray::iterator n = sar.begin(); n != sar.end(); ++n)
               {
                  if (n != sar.begin())
                     stream << wxT(", ");
                  if (writeQuotes) //(inMatlabMode)
                     stream << wxT("'");
                  stream << (*n);
                  if (writeQuotes)
                     stream << wxT("'");
               }

               attCmtLn  = GetInlineAttributeComment(id);

               if ((attCmtLn != wxT("")) && ((mode == Gmat::SCRIPTING) ||
                                        (mode == Gmat::OWNED_OBJECT) ||
                                        (mode == Gmat::SHOW_SCRIPT)))
               {
                  stream << wxT("};") << attCmtLn << wxT("\n");
               }
               else
                  stream << wxT("};\n");
            }
         }
         else
         {
            // Skip unhandled types
            if ( (parmType != Gmat::RMATRIX_TYPE) &&
                 (parmType != Gmat::UNKNOWN_PARAMETER_TYPE) )
            {
               // Fill in the l.h.s.
               value = wxT("");
               WriteParameterValue(id, value);
               if (value != wxT(""))
               {
                  wxString attCmtLn = GetAttributeCommentLine(id);

                  if ((attCmtLn != wxT("")) && ((mode == Gmat::SCRIPTING) ||
                     (mode == Gmat::OWNED_OBJECT) || (mode == Gmat::SHOW_SCRIPT)))
                     stream << attCmtLn;

                  // REAL_ELEMENT_TYPE is handled specially in WriteParameterValue()
                  if (parmType == Gmat::REAL_ELEMENT_TYPE)
                     stream << value;
                  else
                     stream << prefix << GetParameterText(id)
                            << wxT(" = ") << value << wxT(";");

                  // overwrite tmp variable for attribute cmt line
                  attCmtLn = GetInlineAttributeComment(id);

                  if ((attCmtLn != wxT("")) && ((mode == Gmat::SCRIPTING) ||
                      (mode == Gmat::OWNED_OBJECT) || (mode == Gmat::SHOW_SCRIPT)))
                     stream << attCmtLn << wxT("\n");
                  else
                     stream << wxT("\n");
               }
            }
         }
      }
   }
   
   GmatBase *ownedObject;
   wxString nomme, newprefix;
   
   // Get owned object count since subclasses may have GetOwnedObjectCount()
   Integer ownedObjCount = GetOwnedObjectCount();
   #ifdef DEBUG_OWNED_OBJECT_STRINGS
   MessageInterface::ShowMessage
      (wxT("<%p><%s>'%s' has %d owned objects\n"), this, GetTypeName().c_str(),
       instanceName.c_str(), ownedObjCount);
   #endif
   
   // don't include the owned objects (celestial bodies) for the Solar System 
   if (IsOfType(wxT("SolarSystem")))  return;
   
   for (i = 0; i < ownedObjCount; ++i)
   {
      newprefix = prefix;
      ownedObject = GetOwnedObject(i);
      nomme = ownedObject->GetName();

      #ifdef DEBUG_OWNED_OBJECT_STRINGS
      MessageInterface::ShowMessage
         (wxT("   id %d has owned object of type %s and name \"%s\"\n"), i,
          ownedObject->GetTypeName().c_str(), ownedObject->GetName().c_str());
      #endif

      // if owned object is a propagator, don't append the propagator name
      if (ownedObject->GetType() != Gmat::PROPAGATOR)
      {
         // Call new method BuildPropertyName() first to handle additional property
         // name for owned object in general way. For example, additional wxT("Earth") in
         // wxT("FM.GravityField.Earth.Degree"), (loj: 2008.01.25)
         wxString ownedPropName = BuildPropertyName(ownedObject);

         #ifdef DEBUG_OWNED_OBJECT_STRINGS
         MessageInterface::ShowMessage
            (wxT("   ownedPropName=<%s>, name=<%s>\n"), ownedPropName.c_str(), nomme.c_str());
         #endif

         if (ownedPropName != wxT(""))
            newprefix += ownedPropName + wxT(".");
         else if (nomme != wxT(""))
            newprefix += nomme + wxT(".");
      }

      #ifdef DEBUG_OWNED_OBJECT_STRINGS
      MessageInterface::ShowMessage
         (wxT("   Calling ownedObject->GetGeneratingString() with newprefix='%s'\n"),
          newprefix.c_str());
      #endif

      stream << ownedObject->GetGeneratingString(Gmat::OWNED_OBJECT, newprefix);
   }
}


//------------------------------------------------------------------------------
// void WriteParameterValue(Integer id, wxString stream)
//------------------------------------------------------------------------------
/**
 * Writes out parameters in the GMAT script syntax.
 *
 * @param obj Pointer to the object containing the parameter.
 * @param id  ID for the parameter that gets written.
 */
//------------------------------------------------------------------------------
void GmatBase::WriteParameterValue(Integer id, wxString &stream)
{
   Gmat::ParameterType tid = GetParameterType(id);
   Integer precision = GmatGlobal::Instance()->GetDataPrecision();

   #ifdef DEBUG_WRITE_PARAM
   MessageInterface::ShowMessage
      (wxT("   %2d, writing  %s, type=%s\n"), id, GetParameterText(id).c_str(),
       PARAM_TYPE_STRING[tid].c_str());
   #endif

   switch (tid)
   {
   // Objects write out a string without quotes
   case Gmat::OBJECT_TYPE:
      if (inMatlabMode)
         stream << wxT("'");
      stream << GetStringParameter(id);
      if (inMatlabMode)
         stream << wxT("'");
      break;

   // Symbolic constatns write out a string without quotes
   case Gmat::ENUMERATION_TYPE:
      if (inMatlabMode)
         stream << wxT("'");
      stream << GetStringParameter(id);
      if (inMatlabMode)
         stream << wxT("'");
      break;

   // Strings write out a string with quotes (loj: 2008.03.26)
   // Do not write blank string (loj: 2009.09.22)
   case Gmat::FILENAME_TYPE:
   case Gmat::STRING_TYPE:
      {
         wxString strVal = GetStringParameter(id);
         if (inMatlabMode || (!inMatlabMode && strVal != wxT("")))
            stream << wxT("'") << strVal << wxT("'");
         
         break;
      }
   // On/Off constatns write out a string without quotes
   case Gmat::ON_OFF_TYPE:
      if (inMatlabMode)
         stream << wxT("'");
      stream << GetOnOffParameter(id);
      if (inMatlabMode)
         stream << wxT("'");
      break;

   case Gmat::INTEGER_TYPE:
      stream << GetIntegerParameter(id);
      break;

   case Gmat::UNSIGNED_INT_TYPE:
      stream << GetUnsignedIntParameter(id);
      break;

   case Gmat::UNSIGNED_INTARRAY_TYPE:
      {
         UnsignedIntArray arr = GetUnsignedIntArrayParameter(id);
         stream << wxT("[ ");
         for (UnsignedInt i=0; i<arr.size(); i++)
            stream << arr[i] << wxT(" ");
         stream << wxT("]");
      }
      break;

   case Gmat::REAL_TYPE:
   case Gmat::TIME_TYPE: // Treat TIME_TYPE as Real
      stream << GetRealParameter(id);
      break;

   case Gmat::REAL_ELEMENT_TYPE:
      {
         Integer row = GetIntegerParameter(wxT("NumRows"));
         Integer col = GetIntegerParameter(wxT("NumCols"));
         for (Integer i = 0; i < row; ++i)
         {
            // Do not write if value is zero since default is zero
            for (Integer j = 0; j < col; ++j)
            {
               if (GetRealParameter(id, i, j) != 0.0)
               {
                  stream << wxT("GMAT ") << instanceName << wxT("(") << i+1 << wxT(", ") << j+1 <<
                     wxT(") = ") << GetRealParameter(id, i, j) << wxT(";\n");
               }
            }
         }
      }
      break;

   case Gmat::RVECTOR_TYPE:
      {
         Rvector rv = GetRvectorParameter(id);
         stream << wxT("[ ") << rv.ToString(precision) << wxT(" ]");
      }
      break;

   case Gmat::RMATRIX_TYPE:
      {
         Rmatrix ra = GetRmatrixParameter(id);
         Integer r, c;
         ra.GetSize(r, c);

         stream << wxT("[ ");
         for (Integer i = 0; i < r; ++i)
         {
            for (Integer j = 0; j < c; ++j)
               stream << ra.GetElement(i,j) << wxT(" ");
            if (i < r-1)
               stream << wxT("; ");
         }
         stream << wxT("]");
      }
      break;

   case Gmat::BOOLEAN_TYPE:
      stream << ((GetBooleanParameter(id)) ? wxT("true") : wxT("false"));
      break;

   case Gmat::BOOLEANARRAY_TYPE:
      {
         BooleanArray arr = GetBooleanArrayParameter(id);
         stream << wxT("[ ");
         for (UnsignedInt i=0; i<arr.size(); i++)
            stream << (arr[i] ? wxT("true") : wxT("false")) << wxT(" ");
         stream << wxT("]");
      }
      break;
      
   case Gmat::STRINGARRAY_TYPE:
      #ifdef DEBUG_ARRAY_TYPE_PARAMETER_WRITING
         MessageInterface::ShowMessage(wxT("String array management:WriteParmValue ")
               wxT("for %s\n"), GetParameterText(id).c_str());
      #endif

//      {
//         StringArray sa = GetStringArrayParameter(id);
//         stream << wxT("{ ");
//         for (UnsignedInt i = 0; i < sa.size(); ++i)
//         {
//            stream << wxT("'") << sa[i] << wxT("'");
//            if (i < sa.size() - 1)
//               stream << wxT(", ");
//         }
//         stream << wxT(" }");
//      }
//      break;

   default:
      MessageInterface::ShowMessage
         (wxT("Writing of \"%s\" type is not handled yet.\n"),
          PARAM_TYPE_STRING[tid].c_str());
      break;
   }
}

//-------------------------------------
// private methods
//-------------------------------------

//------------------------------------------------------------------------------
//  void PrepCommentTables()
//------------------------------------------------------------------------------
/**
 * Sets the size and initializes the parameter-by-parameter comment tables.
 */
//------------------------------------------------------------------------------
void GmatBase::PrepCommentTables()
{
   attributeCommentLines.resize(parameterCount);
   attributeInlineComments.resize(parameterCount);
   for (Integer i = 0; i < parameterCount; ++i)
   {
      attributeCommentLines[i] = wxT("");
      attributeInlineComments[i] = wxT("");
   }
}


bool GmatBase::HasDynamicParameterSTM(Integer parameterId)
{
   return false;
}

Rmatrix* GmatBase::GetParameterSTM(Integer parameterId)
{
   return NULL;
}

Integer GmatBase::HasParameterCovariances(Integer parameterId)
{
   return -1;
}

Rmatrix* GmatBase::GetParameterCovariances(Integer parameterId)
{
   return NULL; //&covariance;
}

Covariance* GmatBase::GetCovariance()
{
   return &covariance;
}
