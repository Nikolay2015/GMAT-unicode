//$Id: GmatCommand.cpp 9815 2011-08-30 14:19:48Z wendys-dev $
//------------------------------------------------------------------------------
//                                  GmatCommand
//------------------------------------------------------------------------------
// GMAT: General MiHeaderssion Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Darrel J. Conway
// Created: 2003/09/23
//
/**
 * Implementation code for the GmatCommand base class
 */
//------------------------------------------------------------------------------


#include "GmatCommand.hpp"       // class's header file
#include "CommandException.hpp"
#include "StateConverter.hpp"
#include "CoordinateConverter.hpp"
#include "MessageInterface.hpp"  // MessageInterface
#include "TimeSystemConverter.hpp"
#include "GmatDefaults.hpp"
#include "GmatConstants.hpp"
#include "Keplerian.hpp"
#include "RealUtilities.hpp"
#include "CalculationUtilities.hpp"

#include <algorithm>             // for find()
#include <sstream>               // for command summary generation
#include <cmath>                 // for Cartesian to Keplerian conversion; 
                                 // remove it when the real conversions are
                                 // used.
#include <iostream>
#include <iomanip>

//#define DEBUG_COMMAND_DEALLOCATION
//#define DEBUG_COMMAND_SUMMARY_LIST
//#define DEBUG_COMMAND_INIT 1
//#define DEBUG_BUILD_CMD_SUMMARY 1
//#define DEBUG_COMMAND_APPEND 1
//#define DEBUG_COMMAND_INSERT 1
//#define DEBUG_COMMAND_REMOVE 1
//#define DEBUG_COMMAND_SET 1
//#define DEBUG_RUN_COMPLETE 1
//#define DEBUG_WRAPPER_CODE
//#define DEBUG_FIND_OBJECT
//#define DEBUG_SEPARATE
//#define DEBUG_GEN_STRING 1
//#define DEBUG_IS_FUNCTION
//#define DEBUG_INTERPRET_PREFACE
//#define DEBUG_CMD_CALLING_FUNCTION
//#define DEBUG_COMMAND_SUMMARY_STATE
//#define DEBUG_COMMAND_SUMMARY_REF_DATA
//#define DEBUG_COMMAND_SUMMARY_TYPE
//#define DEBUG_MISSION_SUMMARY

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//---------------------------------
//  static members
//---------------------------------
const wxString 
   GmatCommand::PARAMETER_TEXT[GmatCommandParamCount - GmatBaseParamCount] =
   {
      wxT("Comment"),
      wxT("Summary"),
      wxT("MissionSummary")
   };


const Gmat::ParameterType 
   GmatCommand::PARAMETER_TYPE[GmatCommandParamCount - GmatBaseParamCount] =
   {
      Gmat::STRING_TYPE,
      Gmat::STRING_TYPE
   };

Integer GmatCommand::satEpochID = -1;
Integer GmatCommand::satCdID;
Integer GmatCommand::satDragAreaID;
Integer GmatCommand::satCrID;
Integer GmatCommand::satSRPAreaID;
Integer GmatCommand::satTankID;
Integer GmatCommand::satThrusterID;
Integer GmatCommand::satDryMassID;
Integer GmatCommand::satTotalMassID;


//---------------------------------
//  public methods
//---------------------------------

//------------------------------------------------------------------------------
//  GmatCommand(const wxString &typeStr)
//------------------------------------------------------------------------------
/**
 * Constructs GmatCommand core structures (default constructor).
 */
//------------------------------------------------------------------------------
GmatCommand::GmatCommand(const wxString &typeStr) :
   GmatBase             (Gmat::COMMAND, typeStr),
   initialized          (false),
   currentFunction      (NULL),
   callingFunction      (NULL),
   next                 (NULL),
   previous             (NULL),
   level                (-1),   // Not set
   objectMap            (NULL),
   globalObjectMap      (NULL),
   solarSys             (NULL),
   triggerManagers      (NULL),
   internalCoordSys     (NULL),
   forces               (NULL),
   events               (NULL),
   publisher            (NULL),
   streamID             (-1),
   depthChange          (0),
   commandChangedState  (false),
   commandSummary       (wxT("")),
   summaryCoordSysName  (wxT("EarthMJ2000Eq")),
   summaryCoordSys      (NULL),
   summaryForEntireMission (false),
   missionPhysicsBasedOnly (false),
   physicsBasedCommand  (false),
   includeInSummary     (true),
   summaryName          (wxT("Unnamed")),
//   comment              (wxT("")),
   commandChanged       (false),
   cloneCount           (0),
   epochData            (NULL),
   stateData            (NULL),
   parmData             (NULL)
{
   generatingString = wxT("");
   parameterCount = GmatCommandParamCount;
   objectTypes.push_back(Gmat::COMMAND);
   objectTypeNames.push_back(typeStr);
   
   commandNameList.push_back(typeStr);
   parser.Initialize(commandNameList);
   
   BuildCommandSummary(false);
}


//------------------------------------------------------------------------------
//  ~GmatCommand()
//------------------------------------------------------------------------------
/**
 * Destructs GmatCommand core structures.
 *
 * The GmatCommand destructor calls the destructor for the next GmatCommand in
 * the sequence.  Therefore, when a GmatCommand sequence is cleared, all the
 * Sandbox needs to do is call the destructor on the first GmatCommand in the
 * sequence, and the remaining GmatCommands are all destructed.
 *
 * @note The branching GmatCommands will also destroy all of the GmatCommands in
 *       the subbranch(es)
 */
//------------------------------------------------------------------------------
GmatCommand::~GmatCommand()
{
   #ifdef DEBUG_COMMAND_DEALLOCATION
      wxString nextStr = wxT("NULL");
      if (next)
         nextStr = next->GetTypeName();

      ShowCommand(wxT(""), wxT("~GmatCommand() this="), this, wxT(", next="), next);
      //MessageInterface::ShowMessage
      //   (wxT("In GmatCommand::~GmatCommand() this=%s, next=%s\n"),
      //    this->GetTypeName().c_str(), nextStr.c_str());
   #endif
      
   // We need to clean up summary data, so removed return if BranchEnd (loj: 2008.12.19)
   if (!this->IsOfType(wxT("BranchEnd")))
   {      
      // Delete the subsequent GmatCommands
      if (next) {
         #ifdef DEBUG_COMMAND_DEALLOCATION
         MessageInterface::ShowMessage(wxT("   Deleting (%p)%s\n"), next, 
                                       next->GetTypeName().c_str());
         #endif
         
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (next, next->GetTypeName(), this->GetTypeName() +
             wxT("::~GmatCommand() deleting child command"));
         #endif
         delete next;   
      }
   }
   
   // Clean up the summary buffers
   if (epochData)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (epochData, wxT("epochData"), this->GetTypeName() +
          wxT("::~GmatCommand() deleting epochData"));
      #endif
      delete [] epochData;
      epochData = NULL;
   }
   if (stateData)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (stateData, wxT("stateData"), this->GetTypeName() +
          wxT("::~GmatCommand() deleting stateData"));
      #endif
      delete [] stateData;
      stateData = NULL;
   }
   if (parmData)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (parmData, wxT("parmData"), this->GetTypeName() +
          wxT("::~GmatCommand() deleting parmData"));
      #endif
      delete [] parmData;
      parmData = NULL;
   }
}


//------------------------------------------------------------------------------
//  GmatCommand(const GmatCommand &c)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * The copy constructor for GmatCommands copies object lists and associations
 * from the "original" GmatCommand, c, but not its navigation parameters (next, 
 * level), nor does it initialize the GmatCommand.
 *
 * @param c GmatCommand object whose values to use to make "this" copy.
 */
//------------------------------------------------------------------------------
GmatCommand::GmatCommand(const GmatCommand &c) :
   GmatBase             (c),
   initialized          (false),
   association          (c.association),
   objects              (c.objects),
   currentFunction      (c.currentFunction),
   callingFunction      (c.callingFunction),
   next                 (NULL),
   level                (-1),   // Not set
   objectMap            (c.objectMap),
   globalObjectMap      (c.globalObjectMap),
   solarSys             (c.solarSys),
   internalCoordSys     (c.internalCoordSys),
   forces               (c.forces),
   events               (c.events),
   publisher            (c.publisher),
   streamID             (c.streamID),
   depthChange          (c.depthChange),
   commandChangedState  (c.commandChangedState),
   commandSummary       (c.commandSummary),
   summaryCoordSysName  (c.summaryCoordSysName),
   summaryCoordSys      (NULL),
   summaryForEntireMission (c.summaryForEntireMission),
   missionPhysicsBasedOnly (c.missionPhysicsBasedOnly),
   physicsBasedCommand  (c.physicsBasedCommand),
   includeInSummary     (c.includeInSummary),
   summaryName          (c.summaryName),
//   comment              (c.comment),
   commandChanged       (c.commandChanged),
   settables            (c.settables),
   cloneCount           (0),
   epochData            (NULL),
   stateData            (NULL),
   parmData             (NULL)
{
   generatingString = c.generatingString;
}


//------------------------------------------------------------------------------
//  GmatCommand operator=(const GmatCommand &c)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * The assignment operator for GmatCommands copies the object lists and
 * associations from the "other" GmatCommand, c, but not its navigation
 * parameters (next, level, depthChange, commandChangedState).   This operator
 * clears all object associations, and leaves "this" object in an uninitialized
 * state.
 *
 * @param c GmatCommand object whose values to use to set "this" GmatCommand
 *          object.
 *
 * @return GmatCommand object.
 */
//------------------------------------------------------------------------------
GmatCommand& GmatCommand::operator=(const GmatCommand &c)
{
   if (&c == this)
      return *this;

   GmatBase::operator=(c);
   initialized         = false;
   currentFunction     = c.currentFunction;
   callingFunction     = c.callingFunction;
   objects.clear();
   association.clear();
   ClearObjects();             // Drop any previously set object pointers
   
   objects             = c.objects;
   association         = c.association;
   
   objectMap           = c.objectMap;
   globalObjectMap     = c.globalObjectMap;
   solarSys            = c.solarSys;
   internalCoordSys    = c.internalCoordSys;
   forces              = c.forces;
   events              = c.events;
   publisher           = c.publisher;
   generatingString    = c.generatingString;
   streamID            = c.streamID;
//   comment             = c.comment;
   commandChangedState = c.commandChangedState;
   commandSummary      = c.commandSummary;
   summaryCoordSysName = c.summaryCoordSysName;
   summaryCoordSys     = c.summaryCoordSys;
   summaryForEntireMission = c.summaryForEntireMission;
   missionPhysicsBasedOnly = c.missionPhysicsBasedOnly;
   physicsBasedCommand = c.physicsBasedCommand;
   includeInSummary    = c.includeInSummary;
   summaryName         = c.summaryName;
   commandChanged      = c.commandChanged;
   cloneCount          = 0;
   settables           = c.settables;
   
   epochData           = NULL;
   stateData           = NULL;
   parmData            = NULL;
   
   return *this;
}


//------------------------------------------------------------------------------
//  void SetGeneratingString(const wxString &gs)
//------------------------------------------------------------------------------
/**
 * Method used to store the string that was parsed to build this GmatCommand.
 *
 * This method is used to store the GmatCommand string so that the script can be
 * written to a file without inverting the steps used to set up the internal
 * object data.
 *
 * @param <gs> The script line that was interpreted to define this GmatCommand.
 */
//------------------------------------------------------------------------------
void GmatCommand::SetGeneratingString(const wxString &gs)
{
   // Drop the leading white space
   UnsignedInt start = 0, end;
   while ((start < gs.length()) && (gs[start] == wxT(' ')))
      ++start;
   generatingString = gs.substr(start);
   
   end = gs.length()-1;
   while ((end > 0) && (gs[end] == wxT(' ')))
      --end;

   generatingString = gs.substr(0, end+1);
}

//------------------------------------------------------------------------------
//  const wxString& GetGeneratingString()
//------------------------------------------------------------------------------
/**
 * Method used to retrieve the string that was parsed to build this GmatCommand.
 *
 * This method is used to retrieve the GmatCommand string from the script that
 * was parsed to build the GmatCommand.  It is used to save the script line, so
 * that the script can be written to a file without inverting the steps taken to
 * set up the internal object data.  As a side benefit, the script line is
 * available in the GmatCommand structure for debugging purposes.
 *
 * @param mode    Specifies the type of serialization requested. (Not yet used
 *                in commands)
 * @param prefix  Optional prefix appended to the object's name.  (Not yet used
 *                in commands)
 * @param useName Name that replaces the object's name.  (Not yet used in
 *                commands)
 *
 * @return The script line that was interpreted to define this GmatCommand.
 */
//------------------------------------------------------------------------------
const wxString& GmatCommand::GetGeneratingString(Gmat::WriteMode mode,
                                            const wxString &prefix,
                                            const wxString &useName)
{
   static wxString empty;
   if (generatingString == wxT("")) {
      if (typeName == wxT("NoOp"))
         return generatingString;
      empty = wxT("% Generating string not set for ") + typeName + wxT(" command.");
      return empty;
   }
   
   if (mode == Gmat::NO_COMMENTS)
      return generatingString;
   
   
   wxString commentLine = GetCommentLine();
   wxString inlineComment = GetInlineComment();
   
   #if DEBUG_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("GmatCommand::GetGeneratingString() entered, generatingString =\n<%s>\n"),
       generatingString.c_str());
   MessageInterface::ShowMessage
      (wxT("   commentLine = <%s>\n   inlineComment = <%s>\n"),
       commentLine.c_str(), inlineComment.c_str());   
   #endif
   
   // write preface comment
   if (showPrefaceComment)
   {
      wxString gen;
      if (commentLine != wxT(""))
      {
         TextParser tp;
         StringArray textArray = tp.DecomposeBlock(commentLine);
         
         if (textArray.size() > 0 && textArray[0] != wxT(""))
         {
            // handle multiple comment lines
            for (UnsignedInt i=0; i<textArray.size(); i++)
            {
               gen << prefix << textArray[i];
               if (textArray[i].find(wxT("\n")) == commentLine.npos &&
                   textArray[i].find(wxT("\r")) == commentLine.npos)
                  gen << wxT("\n");
            }
         }
      }
      
      InsertCommandName(generatingString);      
      generatingString = gen + generatingString;
   }
   
   
   if (showInlineComment)
      if (inlineComment != wxT(""))
         generatingString = generatingString + inlineComment;
   
   #if DEBUG_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("===> return\n<%s>\n\n"), generatingString.c_str());
   #endif
   
   return generatingString;
}

// Added Set/GetCurrentFunction() because we need to add function name
// in the error message ((LOJ: 2008.09.09)

//------------------------------------------------------------------------------
// virtual void SetCurrentFunction(Function *function)
//------------------------------------------------------------------------------
void GmatCommand::SetCurrentFunction(Function *function)
{
   currentFunction = function;
}

//------------------------------------------------------------------------------
// virtual Function* GetCurrentFunction()
//------------------------------------------------------------------------------
Function* GmatCommand::GetCurrentFunction()
{
   return currentFunction;
}

//------------------------------------------------------------------------------
// void SetCallingFunction();
//------------------------------------------------------------------------------
void GmatCommand::SetCallingFunction(FunctionManager *fm)
{
   #ifdef DEBUG_CMD_CALLING_FUNCTION
      MessageInterface::ShowMessage(
            wxT("NOW setting calling function on command of type %s\n"),
            (GetTypeName()).c_str());
   #endif
   callingFunction = fm;
}

//------------------------------------------------------------------------------
// const StringArray& GetWrapperObjectNameArray()
//------------------------------------------------------------------------------
const StringArray& GmatCommand::GetWrapperObjectNameArray()
{
   return wrapperObjectNames;
}

//------------------------------------------------------------------------------
// bool SetElementWrapper(ElementWrapper* toWrapper,
//                        const wxString &withName)
//------------------------------------------------------------------------------
bool GmatCommand::SetElementWrapper(ElementWrapper* toWrapper,
                                    const wxString &withName)
{
   return false;
}

//------------------------------------------------------------------------------
// void ClearWrappers()
//------------------------------------------------------------------------------
void GmatCommand::ClearWrappers()
{
   // this default implementation does nothing
}

//------------------------------------------------------------------------------
// void CheckDataType(ElementWrapper* forWrapper, Gmat::ParameterType needType,
//                    const wxString &cmdName, bool ignoreUnsetReference)
//------------------------------------------------------------------------------
void GmatCommand::CheckDataType(ElementWrapper* forWrapper,
                                Gmat::ParameterType needType,
                                const wxString &cmdName,
                                bool ignoreUnsetReference)
{
   if (forWrapper == NULL)
   {
      wxString cmdEx = wxT("Reference object not set for command ") + 
                          cmdName;
      cmdEx += wxT(".\n");
      throw CommandException(cmdEx);
   }
   bool typeOK = true;
   Gmat::ParameterType baseType;
   wxString         baseStr;
   wxString         desc = forWrapper->GetDescription();
   try
   {
      baseType = forWrapper->GetDataType();
      baseStr  = PARAM_TYPE_STRING[baseType];
      if ((baseType != needType) && 
          !((needType == Gmat::REAL_TYPE) && (baseType == Gmat::INTEGER_TYPE)))
         typeOK = false;
   }
   catch (BaseException &be)
   {
      // Use exception to remove Visual C++ warning
      be.GetMessageType();
      // will need to check data type of object property 
      // wrappers on initialization
      if (!ignoreUnsetReference)
      {
         wxString errmsg = wxT("Reference not set for \"") + desc;
         errmsg += wxT("\", cannot check for correct data type.");
         throw CommandException(errmsg);
      }
   }
   
   if (!typeOK)
   {
      throw CommandException(wxT("A value of \"") + desc + wxT("\" of base type \"") +
                  baseStr + wxT("\" on command \"") + cmdName + 
                  wxT("\" is not an allowed value.\nThe allowed values are:")
                  wxT(" [ Object Property (Real), Real Number, Variable, Array Element, or Parameter ]. ")); 
   }
}


//------------------------------------------------------------------------------
//  bool SetObject(const wxString &name, const Gmat::ObjectType type,
//                 const wxString &associate, 
//                 const Gmat::ObjectType associateType)
//------------------------------------------------------------------------------
/**
 * Stores the object names and types required for the GmatCommand.
 *
 * Objects associated with a GmatCommand are not necessarily available at the
 * time the GmatCommand is parsed from the script.  This GmatCommand builds the
 * data store of object names that will be used to execute the GmatCommand when
 * the script is run.
 * 
 * @param name The name of the object.
 * @param type The (enumerated) base type assigned to the object.
 * @param associate Optional second object associated with the named object.
 *                  Defaults to the empty string ("").
 * @param associateType Type for the second object; defaults to
 *                      Gmat::UNKNOWN_OBJECT.
 *
 * @return true object is stored successfully, or it it is already registered
 *         with the same type, false otherwise.
 *
 * @note associate and associateType are provided to allow derived classes to
 *       override this method for GmatCommands that link two types of objects
 *       together -- for example, the Propagate GmatCommand links a propagator
 *       setup to a spacecraft.  These parameters are not used in this default
 *       implementation.
 */
//------------------------------------------------------------------------------
bool GmatCommand::SetObject(const wxString &name,
                        const Gmat::ObjectType type,
                        const wxString &associate,
                        const Gmat::ObjectType associateType)
{
   // Check to see if it is already in the object list
   StringArray::iterator item = std::find(objects.begin(), objects.end(), name);
   if (item != objects.end()) 
   {
      if (type != association[*item])  // make sure the types match
         return false;
   }
   else
   {
      // Add it to the lists
      objects.push_back(name);
      association[name] = type; 
   }
   
   return true;
}


//------------------------------------------------------------------------------
//  GmatBase* GetObject(const Gmat::ObjectType type, const wxString objName)
//------------------------------------------------------------------------------
/**
 * Retrieves the objects used by the GmatCommand.
 *
 * Objects associated with a GmatCommand are not necessarily available at the
 * time the GmatCommand is parsed from the script.  This GmatCommand builds the
 * data store of object names that will be used to execute the GmatCommand when
 * the script is run.
 * 
 * @param type    The (enumerated) base type assigned to the object.
 * @param objName The name of the object.
 *
 * @return A pointer to the requested object, obtained from the Configuration 
 *         Manager if it is a configured object, or from the local pointer if it
 *         is an internal object.
 */
//------------------------------------------------------------------------------
GmatBase* GmatCommand::GetGmatObject(const Gmat::ObjectType type, 
                                  const wxString objName)
{
   return NULL;
}


//------------------------------------------------------------------------------
//  bool SetObject(GmatBase *obj, const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Stores pointers to objects required exclusively for the current GmatCommand.
 *
 * Some GmatCommands require helper objects to execute successfully -- for
 * example, the Propagate GmatCommand uses a StoppingCondition object to
 * evaluate when the GmatCommand finishes execution.  Use this method to assign
 * these GmatCommand specific objects.
 *
 * @param obj Pointer to the object.
 * @param type The (enumerated) base type assigned to the object.
 *
 * @return true when object is stored successfully, false otherwise.
 */
//------------------------------------------------------------------------------
bool GmatCommand::SetObject(GmatBase *obj, const Gmat::ObjectType type)
{
   return false;
}


//------------------------------------------------------------------------------
//  void SetSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 * Called by the Sandbox to set the local solar system for the GmatCommand
 * 
 * @param <ss> Pointer to the local solar system
 */
//------------------------------------------------------------------------------
void GmatCommand::SetSolarSystem(SolarSystem *ss)
{
   #ifdef DEBUG_COMMAND_SET
   MessageInterface::ShowMessage
      (wxT("GmatCommand::SetSolarSystem() entered, ss=<%p>\n"), ss);
   #endif
   
   solarSys = ss;
}


//------------------------------------------------------------------------------
// void SetTriggerManagers(std::vector<TriggerManager*> *trigs)
//------------------------------------------------------------------------------
/**
 * Sets the trigger manager pointers
 *
 * @param trigs A vector of TriggerManager pointers passed in from the local
 *              Sandbox.
 */
//------------------------------------------------------------------------------
void GmatCommand::SetTriggerManagers(std::vector<TriggerManager*> *trigs)
{
   triggerManagers = trigs;
}

//------------------------------------------------------------------------------
// virtual void SetInternalCoordSystem(CoordinateSystem *cs)
//------------------------------------------------------------------------------
/*
 * Sets internal coordinate system which state is propagated.
 *
 * @note Current Internal coordinate system used is MJ2000Eq of j2000body as
 *       origin and current j2000body is Earth.
 */
//------------------------------------------------------------------------------
void GmatCommand::SetInternalCoordSystem(CoordinateSystem *cs)
{
   #ifdef DEBUG_COMMAND_SET
   MessageInterface::ShowMessage
      (wxT("GmatCommand::SetInternalCoordSystem() entered, cs=<%p>\n"), cs);
   #endif
   
   internalCoordSys = cs;
}


//------------------------------------------------------------------------------
// virtual void SetSummary(const wxString &csName, bool entireMission = false,
//                         bool physicsOnly = false)
//------------------------------------------------------------------------------
/*
 * Sets the name of the coordinate system and flags to use for the Command
 * Summary display.
 *
 */
//------------------------------------------------------------------------------
void GmatCommand::SetupSummary(const wxString &csName, bool entireMission, bool physicsOnly)

{
   #ifdef DEBUG_COMMAND_SET
   MessageInterface::ShowMessage
      (wxT("GmatCommand::SetupSummary() entered, cs=<%s>\n"), csName.c_str());
   #endif

   summaryCoordSysName     = csName;
   summaryForEntireMission = entireMission;
   missionPhysicsBasedOnly = physicsOnly;
}

void GmatCommand::SetSummaryName(const wxString &sumName)
{
   summaryName = sumName;
}

wxString GmatCommand::GetSummaryName()
{
   return summaryName;
}


//------------------------------------------------------------------------------
//  void SetObjectMap(std::map<wxString, GmatBase *> *map)
//------------------------------------------------------------------------------
/**
 * Called by the Sandbox to set the local asset store used by the GmatCommand
 * 
 * @param <map> Pointer to the local object map
 */
//------------------------------------------------------------------------------
void GmatCommand::SetObjectMap(std::map<wxString, GmatBase *> *map)
{
   objectMap = map;
}


//------------------------------------------------------------------------------
// virtual ObjectMap* GetObjectMap()
//------------------------------------------------------------------------------
ObjectMap* GmatCommand::GetObjectMap()
{
   return objectMap;
}


//------------------------------------------------------------------------------
//  void SetGlobalObjectMap(std::map<wxString, GmatBase *> *map)
//------------------------------------------------------------------------------
/**
 * Called by the Sandbox to set the global asset store used by the GmatCommand
 * 
 * @param <map> Pointer to the local object map
 */
//------------------------------------------------------------------------------
void GmatCommand::SetGlobalObjectMap(std::map<wxString, GmatBase *> *map)
{
   globalObjectMap = map;
}


//------------------------------------------------------------------------------
//  void SetTransientForces(std::vector<PhysicalModel*> *tf)
//------------------------------------------------------------------------------
/**
 * Passes the transient force vector into the commands that need them
 * 
 * @param tf The vector of transient forces
 * 
 * @note The default behavior in the GmatCommands is to ignore the vector.
 */
//------------------------------------------------------------------------------
void GmatCommand::SetTransientForces(std::vector<PhysicalModel*> *tf)
{
   forces = tf;
}


//------------------------------------------------------------------------------
//  void SetEventLocators(std::vector<EventLocator*> *els)
//------------------------------------------------------------------------------
/**
 * Passes the EventLocator collection into the commands that need them
 *
 * @param tf The vector of EventLocators
 *
 * @note The default behavior in the GmatCommands is to ignore the vector.
 */
//------------------------------------------------------------------------------
void GmatCommand::SetEventLocators(std::vector<EventLocator*> *els)
{
   events = els;
}


//------------------------------------------------------------------------------
//  void SetPublisher(Publisher *p)
//------------------------------------------------------------------------------
/**
 * Sets the Publisher used for data generated by the GmatCommand
 * 
 * @param <p> Pointer to the Publisher
 */
//------------------------------------------------------------------------------
void GmatCommand::SetPublisher(Publisher *p)
{
   publisher = p;
}


//------------------------------------------------------------------------------
//  Publisher* GetPublisher()
//------------------------------------------------------------------------------
/**
 * Returns the Publisher used for data generated by the GmatCommand
 */
//------------------------------------------------------------------------------
Publisher* GmatCommand::GetPublisher()
{
   return publisher;
}


//------------------------------------------------------------------------------
// const StringArray& GetObjectList()
//------------------------------------------------------------------------------
/**
 * Returns a list of objects referenced by the command, so that the command can
 * be validated as only referencing existing objects.
 *
 * Derived classes can override this method to build different lists, or to add
 * to the objects list.
 *
 * @return The list of objects
 */
//------------------------------------------------------------------------------
const StringArray& GmatCommand::GetObjectList()
{
   return objects;
}

//------------------------------------------------------------------------------
// bool Validate()
//------------------------------------------------------------------------------
/**
 * Performs internal validation of the command
 *
 * This method is called during the final pass of script reading to verify that
 * the command has internal consistency.
 *
 * @return true if the command is internally consistent, false if not.
 */
//------------------------------------------------------------------------------
bool GmatCommand::Validate()
{
   return GmatBase::Validate();
}


//------------------------------------------------------------------------------
//  wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Gets the name of the parameter with the input id.
 * 
 * @param <id> Integer id for the parameter.
 * 
 * @return The string name of the parameter.
 */
//------------------------------------------------------------------------------
wxString GmatCommand::GetParameterText(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < GmatCommandParamCount)
      return PARAMETER_TEXT[id - GmatBaseParamCount];
      
   return GmatBase::GetParameterText(id);
}


//------------------------------------------------------------------------------
//  Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * Gets the id corresponding to a named parameter.
 * 
 * @param <str> Name of the parameter.
 * 
 * @return The ID.
 */
//------------------------------------------------------------------------------
Integer GmatCommand::GetParameterID(const wxString &str) const
{
   for (Integer i = GmatBaseParamCount; i < GmatCommandParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatBaseParamCount])
         return i;
   }

   return GmatBase::GetParameterID(str);
}


//------------------------------------------------------------------------------
//  Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieve the enumerated type of the object.
 *
 * @param <id> The integer ID for the parameter.
 *
 * @return The enumeration for the type of the parameter.
 */
//------------------------------------------------------------------------------
Gmat::ParameterType GmatCommand::GetParameterType(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < GmatCommandParamCount)
      return PARAMETER_TYPE[id - GmatBaseParamCount];
      
   return GmatBase::GetParameterType(id);
}



//------------------------------------------------------------------------------
//  wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Gets the text description for the type of a parameter.
 * 
 * @param <id> Integer ID of the parameter.
 * 
 * @return The text description of the type of the parameter.
 */
//------------------------------------------------------------------------------
wxString GmatCommand::GetParameterTypeString(const Integer id) const
{
   return GmatBase::PARAM_TYPE_STRING[GetParameterType(id)];
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 * 
 * For GmatCommands, this is a bit of a misnomer because the comment field is 
 * not read-only.  It is accessed only in special circumstances, though -- 
 * comment lines come before the command in the script -- and therefore gets
 * special treatment in the Interpreters.
 *
 * @param <id> Description for the parameter.
 *
 * @return true if the parameter is read only, false (the default) if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool GmatCommand::IsParameterReadOnly(const Integer id) const
{
   if ((id >= GmatBaseParamCount) && (id < GmatCommandParamCount))
      return true;

   return GmatBase::IsParameterReadOnly(id);
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 * 
 * @param <label> Description for the parameter.
 *
 * @return true if the parameter is read only, false (the default) if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool GmatCommand::IsParameterReadOnly(const wxString &label) const
{
   return IsParameterReadOnly(GetParameterID(label));
}


//------------------------------------------------------------------------------
//  wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieve a string parameter.
 *
 * @param <id>    The integer ID for the parameter.
 *
 * @return The string stored for this parameter.
 */
//------------------------------------------------------------------------------
wxString GmatCommand::GetStringParameter(const Integer id) const
{
//   if (id == COMMENT)
//   {
//      return comment;
//   }
         
   if (id == SUMMARY)
   {
      // This call is not const, so need to break const-ness here:
      ((GmatCommand*)this)->BuildCommandSummaryString();
      return commandSummary;
   }
   
   if (id == MISSION_SUMMARY)
   {
      // This call is not const, so need to break const-ness here:
      wxString missionSummary =
         ((GmatCommand*)this)->BuildMissionSummaryString(this);
      return missionSummary;
   }
         
   return GmatBase::GetStringParameter(id);
}

   
//------------------------------------------------------------------------------
//  wxString GetStringParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieve a string parameter.
 *
 * @param <id>    The integer ID for the parameter.
 * @param <index> Array index for parameters.
 *
 * @return The string stored for this parameter.
 */
//------------------------------------------------------------------------------
wxString GmatCommand::GetStringParameter(const Integer id, 
                                            const Integer index) const
{
   return GmatBase::GetStringParameter(id, index);
}

   
//------------------------------------------------------------------------------
//  Real SetStringParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * Sets the value for a wxString parameter.
 * 
 * @param <id>    Integer ID of the parameter.
 * @param <value> New value for the parameter.
 * 
 * @return The value of the parameter.
 */
//------------------------------------------------------------------------------
bool GmatCommand::SetStringParameter(const Integer id, const wxString &value)
{
//   if (id == COMMENT)
//   {
//      comment = value;
//      return true;
//   }
         
   if (id == SUMMARY)
   {
      return false;
   }
         
   if (id == MISSION_SUMMARY)
   {
      return false;
   }
         
   return GmatBase::SetStringParameter(id, value);
}


//------------------------------------------------------------------------------
//  Real SetStringParameter(const Integer id, const Real value, 
//                          const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets the value for a wxString parameter.
 * 
 * @param <id>    Integer ID of the parameter.
 * @param <value> New value for the parameter.
 * @param <index> array index for the parameter.
 * 
 * @return The value of the parameter.
 */
//------------------------------------------------------------------------------
bool GmatCommand::SetStringParameter(const Integer id, const wxString &value,
                                     const Integer index)
{
   return GmatBase::SetStringParameter(id, value, index);
}


//------------------------------------------------------------------------------
//  wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Retrieve a string parameter.
 *
 * @param <label> The (string) label for the parameter.
 *
 * @return The string stored for this parameter.
 */
//------------------------------------------------------------------------------
wxString GmatCommand::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
//  wxString GetStringParameter(const wxString &label,
//                                 const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieve a string parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param <index> array index for the parameter.
 *
 * @return The string stored for this parameter.
 */
//------------------------------------------------------------------------------
wxString GmatCommand::GetStringParameter(const wxString &label,
                                            const Integer index) const
{
   return GetStringParameter(GetParameterID(label), index);
}


//------------------------------------------------------------------------------
//  bool SetStringParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
/**
 * Sets the value for a wxString parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param <value> New value for the parameter.
 *
 * @return The string stored for this parameter.
 */
//------------------------------------------------------------------------------
bool GmatCommand::SetStringParameter(const wxString &label, 
                                     const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
//  bool SetStringParameter(const wxString &label, const wxString &value,
//                          const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets the value for a wxString parameter.
 *
 * @param <label> The (string) label for the parameter.
 * @param <value> New value for the parameter.
 * @param <index> array index for the parameter.
 *
 * @return The string stored for this parameter.
 */
//------------------------------------------------------------------------------
bool GmatCommand::SetStringParameter(const wxString &label, 
                                     const wxString &value,
                                     const Integer index)
{
   return SetStringParameter(GetParameterID(label), value, index);
}


//------------------------------------------------------------------------------
//  bool SetCondition(const wxString &lhs, 
//                    const wxString &operation, 
//                    const wxString &rhs, 
//                    Integer atIndex)
//------------------------------------------------------------------------------
/**
 * Sets a condition for the command, at index atIndex (-999 to add to the end
 * of the list).
 * 
 * @param lhs       string for the left hand side of the condition
 * @param operation string for the operator
 * @param rhs       string for the right hand side of the condition
 * @param atIndex   where in the list to place the condition (-999 means to
 *                    add it to the end of the list (default))
 *
 * @return true if the condition was set, false if not.  The default
 *         (implemented here) returns false.
 *
 * @note See subclasses for more meaningful implementations.
 */
//------------------------------------------------------------------------------
bool GmatCommand::SetCondition(const wxString &lhs, 
                               const wxString &operation, 
                               const wxString &rhs, 
                               Integer atIndex)
{
   return false;
}

//------------------------------------------------------------------------------
//  bool SetConditionOperator(const wxString &op, 
//                            Integer atIndex)
//------------------------------------------------------------------------------
/**
 * Sets a logical operator for the command, at index atIndex (-999 to add to the
 * end of the list) - this is the operator connecting conditions, when there are
 * multiple conditions.
 * 
 * @param op      string for the logical operator
 * @param atIndex where in the list to place the logical operator (-999 means
 *                to add it to the end of the list (default))
 *
 * @return true if the operator was set, false if not.  The default
 *         (implemented here) returns false.
 *
 * @note See subclasses for more meaningful implementations.
 */
//------------------------------------------------------------------------------
bool GmatCommand::SetConditionOperator(const wxString &op, 
                                       Integer atIndex)
{
   return false;
}

//------------------------------------------------------------------------------
//  bool RemoveCondition(Integer atIndex)
//------------------------------------------------------------------------------
/**
 * Removes the condition for the command, at index atIndex.
 * 
 * @param atIndex where in the list to remove the condition from.
 *
 * @return true if the condition was removed, false if not.  The default
 *         (implemented here) returns false.
 *
 * @note See subclasses for more meaningful implementations.
 */
//------------------------------------------------------------------------------
bool GmatCommand::RemoveCondition(Integer atIndex)
{
   return false;
}

//------------------------------------------------------------------------------
//  bool RemoveConditionOperator(Integer atIndex)
//------------------------------------------------------------------------------
/**
 * Removes the logical operator for the command, at index atIndex.
 * 
 * @param atIndex where in the list to remove the logical operator from.
 *
 * @return true if the operator was removed, false if not.  The default
 *         (implemented here) returns false.
 *
 * @note See subclasses for more meaningful implementations.
 */
//------------------------------------------------------------------------------
bool GmatCommand::RemoveConditionOperator(Integer atIndex)
{
   return false;
}


//------------------------------------------------------------------------------
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * Initializes the GmatCommand structures at the start of a run.
 *
 * This method is called by the Sandbox when it receives a run GmatCommand from
 * the Moderator.  This method calls AssignObjects to set the local object
 * pointers prior to execution.
 *
 * Derived GmatCommand objects can use this method to set up any internal data 
 * structures needed to run the GmatCommand.  If this method is overridden, the
 * base class method provided here should be called to ensure that AssignObjects
 * is called at the start of the run.
 *
 * @return true if the GmatCommand is initialized, false if an error occurs.
 */
//------------------------------------------------------------------------------
bool GmatCommand::Initialize()
{
   #if DEBUG_COMMAND_INIT
   MessageInterface::ShowMessage
      (wxT("GmatCommand::Initialize() %s entering\n   epochData=%p, stateData=%p, ")
       wxT("parmData=%p, satVector.size()=%d\n"), GetTypeName().c_str(), epochData,
       stateData, parmData, satVector.size());
   #endif
   
   // Check to be sure the basic infrastructure is in place
   if (objectMap == NULL)
   {
      wxString errorstr(wxT("Object map has not been initialized for "));
      errorstr += GetTypeName();
      throw CommandException(errorstr);
   }
   
   if (globalObjectMap == NULL)
   {
      wxString errorstr(wxT("Global object map has not been initialized for "));
      errorstr += GetTypeName();
      throw CommandException(errorstr);
   }
   
   if (solarSys == NULL)
   {
      wxString errorstr(wxT("Solar system has not been initialized for "));
      errorstr += GetTypeName();
      throw CommandException(errorstr);
   }
   
   initialized = AssignObjects();
   if (publisher == NULL)
      publisher = Publisher::Instance();
   
   if (epochData != NULL)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (epochData, wxT("epochData"), this->GetTypeName() + wxT(" deleting epochData"));
      #endif
      delete [] epochData;
      epochData = NULL;
   }
   
   if (stateData != NULL)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (stateData, wxT("stateData"), this->GetTypeName() + wxT(" deleting stateData"));
      #endif
      delete [] stateData;
      stateData = NULL;
   }
   
   if (parmData != NULL)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (parmData, wxT("parmData"), this->GetTypeName() + wxT(" deleting parmData"));
      #endif
      delete [] parmData;
      parmData = NULL;
   }
   
   satVector.clear();
   satsInMaps = 0;
   
   #if DEBUG_COMMAND_INIT
   MessageInterface::ShowMessage
      (wxT("GmatCommand::Initialize() %s leaving\n   epochData=%p, stateData=%p, ")
       wxT("parmData=%p, satVector.size()=%d\n"), GetTypeName().c_str(), epochData,
       stateData, parmData, satVector.size());
   #endif
   
   return initialized;
}


//------------------------------------------------------------------------------
//  GmatCommand* GetNext()
//------------------------------------------------------------------------------
/**
 * Accesses the next GmatCommand to be executed in the GmatCommand sequence
 * 
 * @return Pointer to the next GmatCommand
 */
//------------------------------------------------------------------------------
GmatCommand* GmatCommand::GetNext()
{
   return next;
}


//------------------------------------------------------------------------------
//  GmatCommand* GetPrevious()
//------------------------------------------------------------------------------
/**
 * Accesses the previous GmatCommand in the GmatCommand sequence
 * 
 * @return Pointer to the previous GmatCommand
 */
//------------------------------------------------------------------------------
GmatCommand* GmatCommand::GetPrevious()
{
   return previous;
}


//------------------------------------------------------------------------------
// bool ForceSetNext(GmatCommand *toCmd)
//------------------------------------------------------------------------------
bool GmatCommand::ForceSetNext(GmatCommand *toCmd) // dangerous!
{
   //ShowCommand(wxT("GmatCommand::", "ForceSetNext() this = ", this, " toCmd = "), toCmd);
   next = toCmd;
   return true;
}


//------------------------------------------------------------------------------
// bool ForceSetPrevious(GmatCommand *toCmd)
//------------------------------------------------------------------------------
bool GmatCommand::ForceSetPrevious(GmatCommand *toCmd) // dangerous!
{
   //ShowCommand("GmatCommand::", "ForceSetPrevious() this = ", this, " toCmd = ", toCmd);
   previous = toCmd;
   return true;
}


//------------------------------------------------------------------------------
//  bool Append(GmatCommand *cmd)
//------------------------------------------------------------------------------
/**
 * Adds GmatCommand at the end of the GmatCommand sequence
 * 
 * @param cmd Pointer to GmatCommand that is added at the end
 *
 * @return true on success, false on failure.
 */
//------------------------------------------------------------------------------
bool GmatCommand::Append(GmatCommand *cmd)
{
   #if DEBUG_COMMAND_APPEND > 1
   ShowCommand(wxT("GmatCommand::"), wxT("Append() this = "), this, wxT(" next = "), next);
   ShowCommand(wxT("GmatCommand::"), wxT("Append() cmd = "), cmd);
   #endif
   
   if (cmd == this)
      throw CommandException(wxT("Attempting to add GmatCommand already in list"));
   
   if (next)
   {
      #ifdef DEBUG_COMMAND_APPEND
      ShowCommand(wxT("GmatCommand::"), wxT(" appending "), cmd, wxT(" to "), next);
      #endif
      next->Append(cmd);
   }
   else
   {
      // Always set the command changed flag when a command is added to the list
      commandChanged = true;
      
      #ifdef DEBUG_COMMAND_APPEND
      ShowCommand(wxT("GmatCommand::"), wxT(" Setting next of "), this, wxT(" to "), cmd);
      #endif
      
      next = cmd;
      
      // Do not set previous command if this is branch end 
      if (!this->IsOfType(wxT("BranchEnd")))
      {
         #ifdef DEBUG_COMMAND_APPEND
         ShowCommand(wxT("GmatCommand::"), wxT(" Setting previous of "), cmd, wxT(" to "), this);
         #endif
         
         cmd->previous = this;
      }
   }
   
   return true;
}


//------------------------------------------------------------------------------
//  bool Insert(GmatCommand *cmd, GmatCommand *prev)
//------------------------------------------------------------------------------
/**
 * Inserts GmatCommand into the GmatCommand sequence
 * 
 * @param cmd  Pointer to GmatCommand that is inserted
 * @param prev Pointer to GmatCommand preceding this GmatCommand
 *
 * @return true on success, false on failure.
 */
//------------------------------------------------------------------------------
bool GmatCommand::Insert(GmatCommand *cmd, GmatCommand *prev)
{
   #ifdef DEBUG_COMMAND_INSERT
   ShowCommand(wxT("GmatCommand::"), wxT("Insert() this = "), this, wxT(" next = "), next);
   ShowCommand(wxT("GmatCommand::"), wxT("Insert() cmd = "), cmd, wxT(" prev = "), prev);
   #endif
   
   if (this == prev)
   {
      GmatCommand *temp = next;
      
      if (!next)
         return Append(cmd);
      
      #ifdef DEBUG_COMMAND_APPEND
      ShowCommand(wxT("GmatCommand::"), wxT(" Setting next of "), this, wxT(" to "), cmd);
      #endif
      
      next = cmd;
      
      #if DEBUG_COMMAND_INSERT
      ShowCommand(wxT("GmatCommand::"), wxT(" Setting previous of "), cmd, wxT(" to "), prev);
      #endif
      
      cmd->previous = prev;
      
      return next->Append(temp); // assumes cmd->next is NULL and next != NULL
   }
   
   if (next == NULL)
      return false;
   
   return next->Insert(cmd, prev);
}


//------------------------------------------------------------------------------
//  GmatCommand* Remove(GmatCommand *cmd)
//------------------------------------------------------------------------------
/**
 * Removes GmatCommand from the GmatCommand sequence
 * 
 * @param cmd Pointer to GmatCommand that is inserted
 * 
 * @return Pointer to the removed GmatCommand or NULL if the GmatCommand was not
 *         found.
 */
//------------------------------------------------------------------------------
GmatCommand* GmatCommand::Remove(GmatCommand *cmd)
{
   #ifdef DEBUG_COMMAND_REMOVE
   ShowCommand(wxT("GmatCommand::"), wxT("Remove() this = "), this, wxT(" cmd = "), cmd);
   ShowCommand(wxT("GmatCommand::"), wxT("Remove() next = "), next, wxT(" prev = "), previous);
   #endif
   
   if (this->IsOfType(wxT("BranchEnd")))
   {
      return NULL;
   }
   
   if (this == cmd)
   {  // NULL the next pointer
      next = NULL;
      return this;
   }
   
   if (next == NULL)
      return NULL;
   
   if (next == cmd)
   {
      GmatCommand *temp = next;
      
      #ifdef DEBUG_COMMAND_REMOVE
      ShowCommand(wxT("GmatCommand::"), wxT(" Setting next of "), this, wxT(" to "), next->GetNext());
      #endif
      
      next = next->GetNext();
      
      // Set previous command
      if (next != NULL)
      {
         #ifdef DEBUG_COMMAND_REMOVE
         ShowCommand(wxT("GmatCommand::"), wxT(" Setting previous of "), next, wxT(" to "), this);
         #endif
         
         next->previous = this;
      }
      
      temp->Remove(cmd);
      
      return temp;
   }
   
   return next->Remove(cmd);
}


//------------------------------------------------------------------------------
//  GmatCommand* GetChildCommand(Integer whichOne)
//------------------------------------------------------------------------------
/**
 * Returns nested commands.
 *
 * This method returns the entry point for the commands in a branch off of the
 * main command sequence.  The base implementation here always returns NULL; 
 * see the BranchCommand implementation for a less trivial example.
 *
 * @param whichOne Determines which child command entry point is returned.  
 * 
 * @return The pointer to the first GmatCommand in the branch.
 */
//------------------------------------------------------------------------------
GmatCommand* GmatCommand::GetChildCommand(Integer whichOne)
{
   return NULL;
}


//------------------------------------------------------------------------------
//  Integer GetVariableCount()
//------------------------------------------------------------------------------
/**
 * Counts up variables.
 * 
 * @return The number of variables in this command and its children.
 */
//------------------------------------------------------------------------------
Integer GmatCommand::GetVariableCount()
{
   return 0;
}


//------------------------------------------------------------------------------
//  Integer GetGoalCount()
//------------------------------------------------------------------------------
/**
 * Counts up goals.
 * 
 * @return The number of goals in this command and its children.
 */
//------------------------------------------------------------------------------
Integer GmatCommand::GetGoalCount()
{
   return 0;
}


//------------------------------------------------------------------------------
//  bool AssignObjects()
//------------------------------------------------------------------------------
/**
 * Sets the internal object pointers prior to a run.
 *
 * This default implementation doesn't do anything.  
 *
 * Derived classes should override this method to set the internal data pointers 
 * needed to execute the GmatCommand by accessing the various maps of objects
 * and setting corresponding internal pointers.
 * 
 * @return true if the pointers are assigned "correctly", false otherwise.
 */
//------------------------------------------------------------------------------
bool GmatCommand::AssignObjects()
{
   return true;
}


//------------------------------------------------------------------------------
//  bool ClearObjects()
//------------------------------------------------------------------------------
/**
 * Clears the internal object pointers.
 *
 * This default implementation doesn't do anything.  
 *
 * Derived classes should override this method to "uninitialize" internal 
 * pointers when the assignment operator is called on this GmatCommand.
 * 
 * @return true if the pointers are assigned "correctly", false otherwise.
 */
//------------------------------------------------------------------------------
bool GmatCommand::ClearObjects()
{
   return true;
}


//------------------------------------------------------------------------------
//  bool GmatCommand::InterpretAction()
//------------------------------------------------------------------------------
/**
 * Performs command actions.
 *
 * This default implementation always returns false because the base class does
 * not perform any actions.
 *
 * @return true if an action was taken, false otherwise.
 */
//------------------------------------------------------------------------------
bool GmatCommand::InterpretAction()
{
   return false;
}


//------------------------------------------------------------------------------
//  Integer DepthIncrement()
//------------------------------------------------------------------------------
/**
 * Indicates the change in branch depth for subsequent commands.
 *
 * This default implementation always returns false because the base class does
 * not perform any actions.
 *
 * @return 0 if the depth stays the same, 1 if the depth increases, or -1 if
 *           it decreases 
 */
//------------------------------------------------------------------------------
Integer GmatCommand::DepthIncrement()
{
   return depthChange;
}


//------------------------------------------------------------------------------
//  bool HasPropStateChanged()
//------------------------------------------------------------------------------
/**
 * Indicates that the propagation data was changed by the command.
 *
 * Some commands can change propagation data in a manner that requires that the
 * propagators and force models need to rebuild their data structures, or that 
 * multistep integrators need to perform a restart before proceeding.  The
 * Sandbox manages the communication to perform propagator restarts.  It checks
 * for state changes of this type by calling this method, and then using the 
 * reference object methods to determine which object (or objects) is affected 
 * by the change.
 *
 * @return true if propagation state has changed, false if there was no change.
 */
//------------------------------------------------------------------------------
bool GmatCommand::HasPropStateChanged()
{
   return commandChangedState;
}


//------------------------------------------------------------------------------
//  bool SkipInterrupt()
//------------------------------------------------------------------------------
/**
 * Allows select commands to avoid polling for user interrupts.
 * 
 * @return true if the command can skip polling; false if polling is needed.
 */
bool GmatCommand::SkipInterrupt()
{
   return false;
}


//------------------------------------------------------------------------------
//  void RunComplete()
//------------------------------------------------------------------------------
/**
 * Tells the sequence that the run was ended, possibly before reaching the end.
 *
 * Some commands -- for instance, the BranchCommands -- track state during a 
 * run.  This method is called when the sequence is terminated by the Moderator
 * to reset the commands to an idle state.
 */
//------------------------------------------------------------------------------
void GmatCommand::RunComplete()
{
   #if DEBUG_RUN_COMPLETE
   MessageInterface::ShowMessage
      (wxT("GmatCommand::RunComplete for (%p)%s\n"), this, typeName.c_str());
   #endif
   
   // Reset stream ID and initialized flag
   streamID = -1;
   initialized = false;
   
   if (this->IsOfType(wxT("BranchEnd")))
      return;
   
   // By default, just call RunComplete on the next command.
   if (next)
   {
      #if DEBUG_RUN_COMPLETE
      MessageInterface::ShowMessage(wxT("   Next cmd is a <%p><%s>'%s'\n"), next,
                                    (next->GetTypeName()).c_str(), next->GetGeneratingString().c_str());
      if (next->IsOfType(wxT("BranchEnd")))
         MessageInterface::ShowMessage
            (wxT("   .. and that cmd is a branchEnd!!!!!!!!! %s\n"),
             (next->GetTypeName()).c_str());
      #endif
      
      // Branch command ends point back to start; this line prevents looping
      if (!next->IsOfType(wxT("BranchEnd")))
         next->RunComplete();
   }
}


//------------------------------------------------------------------------------
//  void BuildCommandSummary(bool commandCompleted)
//------------------------------------------------------------------------------
/**
 * Stores the data used for the summary string for the command
 *
 * Inherited commands override this method for specialized summary data.
 * 
 * @param <commandCompleted> True if the command ran successfully.
 */
//------------------------------------------------------------------------------
void GmatCommand::BuildCommandSummary(bool commandCompleted)
{
   #if DEBUG_BUILD_CMD_SUMMARY
   MessageInterface::ShowMessage
      (wxT("GmatCommand::BuildCommandSummary() %s, commandCompleted=%d, ")
       wxT("objectMap=%p, globalObjectMap=%p\n    epochData=%p, stateData=%p, parmData=%p, ")
       wxT("satVector.size()=%d\n"), GetTypeName().c_str(), commandCompleted, objectMap,
       globalObjectMap, epochData, stateData, parmData, satVector.size());
   #endif
   
   if (epochData == NULL)
   {
      satsInMaps = 0;
      if (objectMap != NULL)
      {
         // Build summary data for each spacecraft in the object list
         GmatBase *obj;
         for (std::map<wxString, GmatBase *>::iterator i =
               objectMap->begin(); i != objectMap->end(); ++i)
         {
            #ifdef DEBUG_COMMAND_SUMMARY_LIST
               MessageInterface::ShowMessage(wxT("Examining %s\n"), i->first.c_str());
            #endif
            obj = i->second;

            if (obj == NULL)
               continue;
            
            if (obj->GetTypeName() == wxT("Spacecraft"))
            {
               satVector.push_back((SpaceObject*)obj);
               if (satEpochID == -1)
               {
                  satEpochID = obj->GetParameterID(wxT("A1Epoch"));
                  satCdID = obj->GetParameterID(wxT("Cd"));
                  satDragAreaID = obj->GetParameterID(wxT("DragArea"));
                  satCrID = obj->GetParameterID(wxT("Cr"));
                  satSRPAreaID = obj->GetParameterID(wxT("SRPArea"));
                  satTankID = obj->GetParameterID(wxT("Tanks"));
                  satThrusterID = obj->GetParameterID(wxT("Thrusters"));
                  satDryMassID = obj->GetParameterID(wxT("DryMass"));
                  satTotalMassID = obj->GetParameterID(wxT("TotalMass"));
               }

               ++satsInMaps;
            }
         }
      }
      if (globalObjectMap != NULL)
      {
         // Add summary data for each spacecraft in the global object list
         GmatBase *obj;
         for (std::map<wxString, GmatBase *>::iterator i =
               globalObjectMap->begin(); i != globalObjectMap->end(); ++i)
         {
            #ifdef DEBUG_COMMAND_SUMMARY_LIST
               MessageInterface::ShowMessage(wxT("Examining %s\n"), i->first.c_str());
            #endif
            obj = i->second;

            if (obj == NULL)
               continue;
            
            if (obj->GetTypeName() == wxT("Spacecraft"))
            {
               satVector.push_back((SpaceObject*)obj);
               if (satEpochID == -1)
               {
                  satEpochID = obj->GetParameterID(wxT("A1Epoch"));
                  satCdID = obj->GetParameterID(wxT("Cd"));
                  satDragAreaID = obj->GetParameterID(wxT("DragArea"));
                  satCrID = obj->GetParameterID(wxT("Cr"));
                  satSRPAreaID = obj->GetParameterID(wxT("SRPArea"));
                  satTankID = obj->GetParameterID(wxT("Tanks"));
                  satThrusterID = obj->GetParameterID(wxT("Thrusters"));
                  satDryMassID = obj->GetParameterID(wxT("DryMass"));
                  satTotalMassID = obj->GetParameterID(wxT("TotalMass"));
               }

               ++satsInMaps;
            }
         }
      }
      if (satsInMaps > 0)
      {
         if (epochData != NULL)
         {
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Remove
               (epochData, wxT("epochData"), this->GetTypeName() + wxT("::BuildCommandSummary()"),
                wxT("deleting epochData"));
            #endif
            delete [] epochData;
         }
         if (stateData != NULL)
         {
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Remove
               (stateData, wxT("stateData"), this->GetTypeName() + wxT("::BuildCommandSummary()"),
                wxT("deleting stateData"));
            #endif
            delete [] stateData;
         }
         if (parmData != NULL)
         {
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Remove
               (parmData, wxT("parmData"), this->GetTypeName() + wxT("::BuildCommandSummary()"),
                wxT("deleting parmData"));
            #endif
            delete [] parmData;
         }
         epochData = new Real[satsInMaps];
         stateData = new Real[6*satsInMaps];
         parmData = new Real[6*satsInMaps];
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Add
            (epochData, wxT("epochData"), this->GetTypeName() + wxT("::BuildCommandSummary()"),
             wxT("epochData = new Real[satsInMaps]"));
         MemoryTracker::Instance()->Add
            (stateData, wxT("stateData"), this->GetTypeName() + wxT("::BuildCommandSummary()"),
             wxT("stateData = new Real[satsInMaps]"));
         MemoryTracker::Instance()->Add
            (parmData, wxT("parmData"), this->GetTypeName() + wxT("::BuildCommandSummary()"),
             wxT("parmData = new Real[satsInMaps]"));
         #endif
      }
   }
   
   #if DEBUG_BUILD_CMD_SUMMARY
   MessageInterface::ShowMessage
      (wxT("GmatCommand::BuildCommandSummary() Now fill in data...\n   ")
       wxT("satsInMaps=%d, satVector.size()=%d\n"), satsInMaps,
       satVector.size());
   #endif
   
   Integer i6;
   for (Integer i = 0; i < satsInMaps; ++i)
   {
      i6 = i * 6;
      epochData[i] = satVector[i]->GetRealParameter(satEpochID);
      memcpy(&stateData[i6], satVector[i]->GetState().GetState(),
            6*sizeof(Real));
      parmData[i6]   = satVector[i]->GetRealParameter(satCdID);
      parmData[i6+1] = satVector[i]->GetRealParameter(satDragAreaID);
      parmData[i6+2] = satVector[i]->GetRealParameter(satCrID);
      parmData[i6+3] = satVector[i]->GetRealParameter(satSRPAreaID);
      parmData[i6+4] = satVector[i]->GetRealParameter(satDryMassID);
      parmData[i6+5] = satVector[i]->GetRealParameter(satTotalMassID);
   }
}


//------------------------------------------------------------------------------
//  void BuildCommandSummaryString(bool commandCompleted)
//------------------------------------------------------------------------------
/**
 * Generates the summary string for the command from the buffered data.
 *
 * Inherited commands override this method for specialized summary data.
 *
 * @param <commandCompleted> True if the command ran successfully.
 */
//------------------------------------------------------------------------------
void GmatCommand::BuildCommandSummaryString(bool commandCompleted)
{
   #ifdef DEBUG_COMMAND_SUMMARY_TYPE
      MessageInterface::ShowMessage(
            wxT("   Now entering BuildCommandSummaryString with commandCompleted = %s, summaryForEntireMission = %s, missionPhysicsBasedOnly = %s, for command %s of type %s which is %s a physics-based command.\n"),
            (commandCompleted? wxT("true") : wxT("false")),(summaryForEntireMission? wxT("true") : wxT("false")), (missionPhysicsBasedOnly? wxT("true") : wxT("false")), summaryName.c_str(), typeName.c_str(),
            (physicsBasedCommand? wxT("DEFINITELY") :wxT("NOT")));
   #endif
   wxString data;
   StateConverter    stateConverter;

   /// if we're writing the entire mission summary for only physics-based commands, return if not a physics-based command
   if (summaryForEntireMission && ((missionPhysicsBasedOnly && (!physicsBasedCommand)) || !includeInSummary))
   {
      data << wxT("");
      commandSummary = data;
      return;
   }
   if (!summaryForEntireMission && (!includeInSummary))
   {
      data << wxT("No summary data for ") << typeName << wxT(" command ") << summaryName << wxT("\n");
      commandSummary = data;
      return;
   }

   // Write the separator and the name and type of the command first
   if (summaryForEntireMission)
      data << wxT("======  ");
   data << typeName << wxT(" Command: ") << summaryName << wxT("\n");

   if (((objectMap == NULL) && (globalObjectMap == NULL)) ||
       (satVector.size() == 0))
   {
         data << wxT("No command summary data available\n");
   }
   else
   {
      if (!commandCompleted)
         data << wxT("Execute the script to generate command summary data\n");
      else
      {
        // data << "-------------------------------------------"
        //      << "-------------------------------------------\n";
   
         GmatBase   *obj;
         Rvector6   rawState;
         Rvector6   cartState, cartStateInternal;
         Rvector6   kepState;
         Rvector6   modKepState;
         Rvector6   sphStateAZFPA;
         Rvector6   sphStateRADEC;
         Real       ma, ea, ha, meanMotion, orbitEnergy, c3, semilatusRectum, angularMomentum;
         Real       betaAngle, periAltitude, velPeriapsis, velApoapsis, orbitPeriod, lst, mha;
         Real       latitude, longitude, altitude, bDotT = 0.0, bDotR = 0.0, bVectorAngle = 0.0, bVectorMag = 0.0;
         Real       dla = 0.0, rla = 0.0;
         bool       isEccentric, isHyperbolic, originIsCelestialBody;
         SpacePoint *objOrigin = NULL, *cmdOrigin = NULL;
         Real       originEqRad = 0.0;
         Real       originMu    = 0.0;
         Real       originFlattening = 0.0;
         Real       originHourAngle  = 0.0;

         CoordinateConverter cc;
         A1Mjd       a1;

         // Build summary data for each spacecraft in the object list
         for (Integer i = 0; i < satsInMaps; ++i)
         {
            obj                      = satVector[i];
            a1                       = epochData[i];

            objOrigin                = ((SpaceObject*)obj)->GetOrigin(); // is this the same as the origin of the internalCoordSystem?
            summaryCoordSys          = (CoordinateSystem*) FindObject(summaryCoordSysName);
            if (summaryCoordSys == NULL)
            {
               wxString errmsg = wxT("Cannot use coordinate system ");
               errmsg += summaryCoordSysName + wxT(" to build command summary.\n");
               errmsg += wxT("Please execute script to make ") + summaryCoordSysName + wxT(" available.\n");
               throw CommandException(errmsg);
            }
            cmdOrigin                = summaryCoordSys->GetOrigin();
            if (!cmdOrigin)
               throw CommandException(wxT("Origin for summary coordinate system is NULL!!!!!\n"));

            if (!stateConverter.SetMu(summaryCoordSys))
            {
               wxString errmsg = wxT("GmatCommand::BuildCommandSummaryString: Error setting mu for StateConverter.");
               throw CommandException(errmsg);
            }

            rawState                 = &stateData[i*6];  // assumes in internalCoordSystem
            cartStateInternal        = rawState;;
            // First convert the cartesian state to the summaryCoordSys
            cc.Convert(a1, cartStateInternal, internalCoordSys, cartState, summaryCoordSys);
            // Need to convert state to all representations
            kepState        = stateConverter.FromCartesian(cartState, wxT("Keplerian"));
            modKepState     = stateConverter.FromCartesian(cartState, wxT("ModifiedKeplerian"));
            sphStateAZFPA   = stateConverter.FromCartesian(cartState, wxT("SphericalAZFPA"));
            sphStateRADEC   = stateConverter.FromCartesian(cartState, wxT("SphericalRADEC"));

            #ifdef DEBUG_COMMAND_SUMMARY_STATE
               MessageInterface::ShowMessage(wxT("Now converting from %s to %s coordinate system.\n"),
                     internalCoordSys->GetName().c_str(), summaryCoordSys->GetName().c_str());
               MessageInterface::ShowMessage(wxT("epoch = %12.10f\n"), a1.Get());
               MessageInterface::ShowMessage(wxT("cart state from array    = %s\n"), cartStateInternal.ToString().c_str());
               MessageInterface::ShowMessage(wxT("cart state in summary CS = %s\n"), cartState.ToString().c_str());
               MessageInterface::ShowMessage(wxT("keplerian state in summary CS = %s\n"), kepState.ToString().c_str());
            #endif

            isEccentric      = false;
            isHyperbolic     = false;
            ea               = 0.0;
            ha               = 0.0;
            if (kepState[1] < (1.0 - GmatOrbitConstants::KEP_ECC_TOL))
            {
               ea                   = Keplerian::TrueToEccentricAnomaly(kepState[5] * GmatMathConstants::RAD_PER_DEG,
                                      kepState[1], true) * GmatMathConstants::DEG_PER_RAD;
               isEccentric          = true;
            }
            else if (kepState[1] > (1.0 + GmatOrbitConstants::KEP_TOL)) // *** or KEP_ECC_TOL or need new tolerance for this?  1.0e-10
            {
               ha                   = Keplerian::TrueToHyperbolicAnomaly(kepState[5] * GmatMathConstants::RAD_PER_DEG,
                                      kepState[1], true) * GmatMathConstants::DEG_PER_RAD;
               isHyperbolic         = true;
            }
            ma               = Keplerian::TrueToMeanAnomaly(kepState[5] * GmatMathConstants::RAD_PER_DEG,
                               kepState[1], !isHyperbolic) * GmatMathConstants::DEG_PER_RAD;


            if (cmdOrigin->IsOfType(wxT("CelestialBody"))) // This is required to be true, for now
            {
               originIsCelestialBody    = true;
               originEqRad              = ((CelestialBody*)cmdOrigin)->GetEquatorialRadius();
               originMu                 = ((CelestialBody*)cmdOrigin)->GetGravitationalConstant();
               originFlattening         = ((CelestialBody*)cmdOrigin)->GetFlattening();
               originHourAngle          = ((CelestialBody*)cmdOrigin)->GetHourAngle(a1);
            }
            else
            {
               originIsCelestialBody    = false;
            }
            // Compute the origin-to-sun unit vector
            Rvector3 originToSun(0.0,0.0,0.0);
            if (cmdOrigin->GetName() != SolarSystem::SUN_NAME)
            {
               Rvector3 sunPos      = (solarSys->GetBody(SolarSystem::SUN_NAME))->GetMJ2000Position(a1);
               Rvector3 originPos   = cmdOrigin->GetMJ2000Position(a1);
               originToSun = sunPos - originPos;
               originToSun.Normalize();
            }
            #ifdef DEBUG_COMMAND_SUMMARY_REF_DATA
               MessageInterface::ShowMessage(wxT("----> Spacecraft Origin is %s of type %s\n"),
                     objOrigin->GetName().c_str(), objOrigin->GetTypeName().c_str());
               MessageInterface::ShowMessage(wxT("----> Command Origin is %s of type %s\n"),
                     cmdOrigin->GetName().c_str(), cmdOrigin->GetTypeName().c_str());
            #endif

            meanMotion       = GmatCalcUtil::CalculateKeplerianData(wxT("MeanMotion"), cartState, originMu);
            semilatusRectum  = GmatCalcUtil::CalculateAngularData(wxT("SemilatusRectum"), cartState, originMu, originToSun);
            angularMomentum  = GmatCalcUtil::CalculateAngularData(wxT("HMag"), cartState, originMu, originToSun);
            betaAngle        = GmatCalcUtil::CalculateAngularData(wxT("BetaAngle"), cartState, originMu, originToSun);
            orbitEnergy      = GmatCalcUtil::CalculateKeplerianData(wxT("Energy"), cartState, originMu);
            c3               = GmatCalcUtil::CalculateKeplerianData(wxT("C3Energy"), cartState, originMu);
            velPeriapsis     = GmatCalcUtil::CalculateKeplerianData(wxT("VelPeriapsis"), cartState, originMu);
            periAltitude     = modKepState[0] - originEqRad;

            if (isEccentric)
            {
               orbitPeriod       = GmatCalcUtil::CalculateKeplerianData(wxT("OrbitPeriod"), cartState, originMu);
               velApoapsis       = GmatCalcUtil::CalculateKeplerianData(wxT("VelApoapsis"), cartState, originMu);
            }
            else  // these shouldn't be written anyway
            {
               orbitPeriod       = 0.0;
               velApoapsis       = 0.0;
            }

            if (originIsCelestialBody)
            {
               #ifdef DEBUG_COMMAND_SUMMARY_REF_DATA
                  MessageInterface::ShowMessage(wxT("----> Now creating BodyFixed cs and passing into PlanetData objects.\n"));
               #endif
               // Need a origin-centered BodyFixed coordinate system here
                  CoordinateSystem *originBF = CoordinateSystem::CreateLocalCoordinateSystem(wxT("OriginBodyFixed"), wxT("BodyFixed"),
                                               cmdOrigin, NULL, NULL, ((SpacePoint*)obj)->GetJ2000Body(), solarSys);
               if (!originBF)
               {
                  wxString errmsg = wxT("Error creating BodyFixed Coordinate System for origin ");
                  errmsg += objOrigin->GetName() + wxT("\n");
                  throw CommandException(errmsg);
               }
               // Convert to the origin-centered BodyFixed Coordinate System
               Rvector6 cartBodyFixed;
               cc.Convert(a1, cartStateInternal, internalCoordSys, cartBodyFixed, originBF);


               lst                = GmatCalcUtil::CalculatePlanetData(wxT("LST"), cartBodyFixed, originEqRad, originFlattening, originHourAngle);
               mha                = GmatCalcUtil::CalculatePlanetData(wxT("MHA"), cartBodyFixed, originEqRad, originFlattening, originHourAngle);
               latitude           = GmatCalcUtil::CalculatePlanetData(wxT("Latitude"), cartBodyFixed, originEqRad, originFlattening, originHourAngle);
               longitude          = GmatCalcUtil::CalculatePlanetData(wxT("Longitude"), cartBodyFixed, originEqRad, originFlattening, originHourAngle);
               altitude           = GmatCalcUtil::CalculatePlanetData(wxT("Altitude"), cartBodyFixed, originEqRad, originFlattening, originHourAngle);
               if (isHyperbolic)
               {
                  try
                  {
                     bDotT            = GmatCalcUtil::CalculateBPlaneData(wxT("BDotR"), cartState, originMu);
                     bDotR            = GmatCalcUtil::CalculateBPlaneData(wxT("BDotT"), cartState, originMu);
                     bVectorAngle     = GmatCalcUtil::CalculateBPlaneData(wxT("BVectorAngle"), cartState, originMu);
                     bVectorMag       = GmatCalcUtil::CalculateBPlaneData(wxT("BVectorMag"), cartState, originMu);
                     dla              = GmatCalcUtil::CalculateAngularData(wxT("DLA"), cartState, originMu, originToSun);
                     rla              = GmatCalcUtil::CalculateAngularData(wxT("RLA"), cartState, originMu, originToSun);
                  }
                  catch (BaseException& be)
                  {
                     MessageInterface::ShowMessage(wxT("ERROR!! %s\n"), be.GetFullMessage().c_str());
                  }
               }
               else  // these shouldn't be written anyway
               {
                  bDotT            = 0.0;
                  bDotR            = 0.0;
                  bVectorAngle     = 0.0;
                  bVectorMag       = 0.0;
                  dla              = 0.0;
                  rla              = 0.0;
               }
            } // if originIsCelestialBody
            else  // these shouldn't be written anyway
            {
               lst                = 0.0;
               mha                = 0.0;
               latitude           = 0.0;
               longitude          = 0.0;
               altitude           = 0.0;
            }

            //TimeConverterUtil::Convert
            Real utcModJulEpoch     = TimeConverterUtil::Convert(epochData[i],
                                      TimeConverterUtil::A1MJD, TimeConverterUtil::UTCMJD,
                                      GmatTimeConstants::JD_JAN_5_1941);
            Real taiModJulEpoch     = TimeConverterUtil::Convert(epochData[i],
                                      TimeConverterUtil::A1MJD, TimeConverterUtil::TAIMJD,
                                      GmatTimeConstants::JD_JAN_5_1941);
            Real ttModJulEpoch      = TimeConverterUtil::Convert(epochData[i],
                                      TimeConverterUtil::A1MJD, TimeConverterUtil::TTMJD,
                                      GmatTimeConstants::JD_JAN_5_1941);
            Real tdbModJulEpoch     = TimeConverterUtil::Convert(epochData[i],
                                      TimeConverterUtil::A1MJD, TimeConverterUtil::TDBMJD,
                                      GmatTimeConstants::JD_JAN_5_1941);
            wxString utcString   = TimeConverterUtil::ConvertMjdToGregorian(utcModJulEpoch);
            wxString taiString   = TimeConverterUtil::ConvertMjdToGregorian(taiModJulEpoch);
            wxString ttString    = TimeConverterUtil::ConvertMjdToGregorian(ttModJulEpoch);
            wxString tdbString   = TimeConverterUtil::ConvertMjdToGregorian(tdbModJulEpoch);
//            data.flags(std::ios::left);
//            data.precision(10);
//            data.setf(std::ios::fixed,std::ios::floatfield);
//            data.fill('0');
//            data.width(20);

            // Add a between-spacecraft break
            if (i > 0)
               data << wxT("\n   =================================================")
                     wxT("=======================\n\n");

            //  Write the epoch data
            data << wxT("        Spacecraft       : ") << obj->GetName() << wxT("\n")
                  << wxT("        Coordinate System: ") << summaryCoordSysName <<  wxT("\n\n")
//                  << "        Coordinate System: EarthMJ2000Eq \n\n"

                 << wxT("        Time System   Gregorian                     ")
                 << wxT("Modified Julian  \n")
                 << wxT("        --------------------------------------------")
                 << wxT("--------------------------    \n")
                 << wxT("        UTC Epoch:    ") << utcString << wxT("      ") << utcModJulEpoch <<wxT("\n")
                 << wxT("        TAI Epoch:    ") << taiString << wxT("      ") << taiModJulEpoch <<wxT("\n")
                 << wxT("        TT  Epoch:    ") << ttString  << wxT("      ") << ttModJulEpoch  <<wxT("\n")
                 << wxT("        TDB Epoch:    ") << tdbString << wxT("      ") << tdbModJulEpoch <<wxT("\n\n");

//            data.unsetf(std::ios::fixed);
//            data.unsetf(std::ios::floatfield);
//            data.precision(15);
//            data.width(20);

            data << wxT("        Cartesian State                       ")
                 << wxT("Keplerian State\n")
                                      << wxT("        ---------------------------           ")
                                      << wxT("-------------------------------- \n")
                 << wxT("        X  = ")   << BuildNumber(cartState[0])  << wxT(" km     ")
                 << wxT("        SMA  = ") << BuildNumber(kepState[0])   << wxT(" km\n")
                 << wxT("        Y  = ")   << BuildNumber(cartState[1])  << wxT(" km     ")
                 << wxT("        ECC  = ") << BuildNumber(kepState[1])   << wxT("\n")
                 << wxT("        Z  = ")   << BuildNumber(cartState[2])  << wxT(" km     ")
                 << wxT("        INC  = ") << BuildNumber(kepState[2])   << wxT(" deg\n")
                 << wxT("        VX = ")   << BuildNumber(cartState[3])  << wxT(" km/sec ")
                 << wxT("        RAAN = ") << BuildNumber(kepState[3])   << wxT(" deg\n")
                 << wxT("        VY = ")   << BuildNumber(cartState[4])  << wxT(" km/sec ")
                 << wxT("        AOP  = ") << BuildNumber(kepState[4])   << wxT(" deg\n")
                 << wxT("        VZ = ")   << BuildNumber(cartState[5])  << wxT(" km/sec ")
                 << wxT("        TA   = ") << BuildNumber(kepState[5])   << wxT(" deg\n")
                 << wxT("                                      ")        << wxT("        MA   = ") << BuildNumber(ma) << wxT(" deg\n");
            if (isEccentric)
               data << wxT("                                      ")     << wxT("        EA   = ") << BuildNumber(ea) << wxT(" deg\n");
            else if (isHyperbolic)
               data << wxT("                                      ")     << wxT("        HA   = ") << BuildNumber(ha) << wxT(" deg\n");
            data << wxT("\n        Spherical State                       ")
                 << wxT("Other Orbit Data\n")
                 << wxT("        ---------------------------           ")
                 << wxT("--------------------------------\n")
                 << wxT("        RMAG = ")               << BuildNumber(sphStateAZFPA[0])           << wxT(" km   ")
                 << wxT("        Mean Motion        = ") << BuildNumber(meanMotion, true)           << wxT(" deg/sec\n")
                 << wxT("        RA   = ")               << BuildNumber(sphStateAZFPA[1])           << wxT(" deg  ")
                 << wxT("        Orbit Energy       = ") << BuildNumber(orbitEnergy, false)         << wxT(" Km^2/s^2\n")
                 << wxT("        DEC  = ")               << BuildNumber(sphStateAZFPA[2])           << wxT(" deg  ")
                 << wxT("        C3                 = ") << BuildNumber(c3, false)                  << wxT(" Km^2/s^2\n")
                 << wxT("        VMAG = ")               << BuildNumber(sphStateAZFPA[3])           << wxT(" km/s ")
                 << wxT("        Semilatus Rectum   = ") << BuildNumber(semilatusRectum, false)     << wxT(" km   \n")
                 << wxT("        AZI  = ")               << BuildNumber(sphStateAZFPA[4])           << wxT(" deg  ")
                 << wxT("        Angular Momentum   = ") << BuildNumber(angularMomentum, false)     << wxT(" km^2/s\n")
                 << wxT("        VFPA = ")               << BuildNumber(sphStateAZFPA[5])           << wxT(" deg  ")
                 << wxT("        Beta Angle         = ") << BuildNumber(betaAngle, false)           << wxT(" deg  \n")
                 << wxT("        RAV  = ")               << BuildNumber(sphStateRADEC[4])           << wxT(" deg  ")
                 << wxT("        Periapsis Altitude = ") << BuildNumber(periAltitude, false)        << wxT(" km   \n")
                 << wxT("        DECV = ")               << BuildNumber(sphStateRADEC[5])           << wxT(" deg  ")
                 << wxT("        VelPeriapsis       = ") << BuildNumber(velPeriapsis, false)        << wxT(" km/s\n");
            if (isEccentric)
            {
               data << wxT("                                       ") << wxT("       VelApoapsis        = ") << BuildNumber(velApoapsis, false) << wxT(" km/s \n")
                    << wxT("                                       ") << wxT("       Orbit Period       = ") << BuildNumber(orbitPeriod, false) << wxT(" s    \n");
            }
            // add planetodetic parameters, if the origin is a Celestial Body
            if (originIsCelestialBody)
            {
               // and include hyperbolic parameters, if appropriate
               if (isHyperbolic)
               {
                  data << wxT("\n        Planetodetic Properties               ")
                       << wxT("Hyperbolic Parameters\n")
                       << wxT("        ---------------------------           ")
                       << wxT("--------------------------------\n")
                       << wxT("        LST       = ")      << BuildNumber(lst, false)              << wxT(" deg  ")
                       << wxT("   BdotT          = ")      << BuildNumber(bDotT, false)            << wxT(" km   \n")
                       << wxT("        MHA       = ")      << BuildNumber(mha, false)              << wxT(" deg  ")
                       << wxT("   BdotR          = ")      << BuildNumber(bDotR, false)            << wxT(" km   \n")
                       << wxT("        Latitude  = ")      << BuildNumber(latitude, false)         << wxT(" deg  ")
                       << wxT("   B Vector Angle = ")      << BuildNumber(bVectorAngle, false)     << wxT(" deg  \n")
                       << wxT("        Longitude = ")      << BuildNumber(longitude, false)        << wxT(" deg  ")
                       << wxT("   B Vector Mag   = ")      << BuildNumber(bVectorMag, false)       << wxT(" km   \n")
                       << wxT("        Altitude  = ")      << BuildNumber(altitude, false)         << wxT(" km   ")
                       << wxT("   DLA            = ")      << BuildNumber(dla, false)              << wxT(" deg  \n")
                       << wxT("                                           ")
                       << wxT("   RLA            = ")      << BuildNumber(rla, false)              << wxT(" deg  \n");
               }
               else
               {
                  data << wxT("\n        Planetodetic Properties \n")
                       << wxT("        ---------------------------\n")
                       << wxT("        LST       = ") << BuildNumber(lst, false)           << wxT(" deg\n")
                       << wxT("        MHA       = ") << BuildNumber(mha, false)           << wxT(" deg\n")
                       << wxT("        Latitude  = ") << BuildNumber(latitude, false)      << wxT(" deg\n")
                       << wxT("        Longitude = ") << BuildNumber(longitude, false)     << wxT(" deg\n")
                       << wxT("        Altitude  = ") << BuildNumber(altitude, false)      << wxT(" km\n");
               }
            }

            data << wxT("\n\n        Spacecraft properties \n")
                 << wxT("        ------------------------------\n")
                 << wxT("        Cd                    = ") << BuildNumber(parmData[i*6],   false, 10) << wxT("\n")
                 << wxT("        Drag area             = ") << BuildNumber(parmData[i*6+1], false, 10) << wxT(" m^2\n")
                 << wxT("        Cr                    = ") << BuildNumber(parmData[i*6+2], false, 10) << wxT("\n")
                 << wxT("        Reflective (SRP) area = ") << BuildNumber(parmData[i*6+3], false, 10) << wxT(" m^2\n");

            data << wxT("        Dry mass              = ") << BuildNumber(parmData[i*6+4])            << wxT(" kg\n");
            data << wxT("        Total mass            = ") << BuildNumber(parmData[i*6+5])            << wxT(" kg\n");

            StringArray tanks = obj->GetStringArrayParameter(satTankID);
            if (tanks.size() > 0)
            {
               data << wxT("\n        Tank masses:\n");
               for (StringArray::iterator i = tanks.begin();
                    i != tanks.end(); ++i)
                  data << wxT("           ") << (*i) << wxT(":   ")
                       << BuildNumber(obj->GetRefObject(Gmat::HARDWARE, (*i))->
                             GetRealParameter(wxT("FuelMass"))) << wxT(" kg\n");
            }
            data << wxT("\n");
         }    // for i 0 -> satsInMaps
      }
   }

   commandSummary = data;
}


//------------------------------------------------------------------------------
//  void BuildMissionSummaryString()
//------------------------------------------------------------------------------
/**
 * Generates the summary string for a Mission by building the summary string for 
 * this command, and then tacking on the summary for subsequent commands.
 *
 * Inherited commands can override this method to handle branching or other 
 * special cases.
 * 
 * @param <head> The first command in the summary
 */
//------------------------------------------------------------------------------
const wxString GmatCommand::BuildMissionSummaryString(const GmatCommand* head)
{
   #ifdef DEBUG_MISSION_SUMMARY
      MessageInterface::ShowMessage(wxT("Entering BuildMissionSummaryString for command %s of type %s\n"),
            ((GmatCommand*)head)->GetSummaryName().c_str(), ((GmatCommand*)head)->GetTypeName().c_str());
   #endif
   BuildCommandSummaryString();
   wxString missionSummary = commandSummary;


   if (next && (next != head))
   {
      next->SetupSummary(summaryCoordSysName, true, missionPhysicsBasedOnly);
      #ifdef DEBUG_MISSION_SUMMARY
         MessageInterface::ShowMessage(wxT("... about to ask for summary for command %s of type %s\n"),
               next->GetSummaryName().c_str(), next->GetTypeName().c_str());
      #endif
      missionSummary += next->GetStringParameter(wxT("MissionSummary"));
   }
   
   return missionSummary;
}


//------------------------------------------------------------------------------
// const wxString BuildNumber(Real value, Integer length)
//------------------------------------------------------------------------------
/**
 * Builds a formatted string containing a Real, so the Real can be serialized to
 * the display
 *
 * @param value The Real that needs to be serialized
 * @param length The size of the desired string
 *
 * @return The formatted string
 */
//------------------------------------------------------------------------------
const wxString GmatCommand::BuildNumber(Real value, bool useExp, Integer length)
{
   wxString retval = wxT("Invalid number");

   if (length < 100)
   {
      wxString temp, defstr;
      Integer fraction = 1;

      // check for a NaN first
      if ((!GmatMathUtil::IsEqual(value, 0.0)) &&
         (GmatMathUtil::IsEqual(value, GmatRealConstants::REAL_UNDEFINED)        ||
          GmatMathUtil::IsEqual(value, GmatRealConstants::REAL_UNDEFINED_LARGE)  ||
          GmatMathUtil::IsNaN(value)))
      {
         defstr.Printf( wxT("%%%ds"), length);
         temp.Printf( defstr, wxT("NaN"));
      }
      else
      {
         if (useExp)
         {
            fraction = length - 8;
            defstr.Printf( wxT("%%%d.%de"), length, fraction);
         }
         else
         {
            Real shift = GmatMathUtil::Abs(value);
            while (shift > 10.0)
            {
               ++fraction;
               shift *= 0.1;
            }
            fraction = length - 3 - fraction;
            defstr.Printf( wxT("%%%d.%dlf"), length, fraction);
         }

         temp.Printf(defstr, value);
      }
      retval = temp;
   }

   return retval;
}


////------------------------------------------------------------------------------
////  void BuildCommandSummary(bool commandCompleted)
////------------------------------------------------------------------------------
///**
// * Generates the summary string for the command
// *
// * Inherited commands override this method for specialized summary data.
// * 
// * @param <commandCompleted> True if the command ran successfully.
// */
////------------------------------------------------------------------------------
//wxString GmatCommand::GetCommandSummary()
//{
//   return commandSummary;
//}


bool GmatCommand::HasConfigurationChanged()
{
   bool changed = commandChanged;
   if (!changed)
   {
      if (next != NULL)
         changed = next->HasConfigurationChanged();
   }
   
   return changed;
}


void GmatCommand::ConfigurationChanged(bool tf, bool setAll)
{
   commandChanged = tf;
   if (setAll)
      if (next)
         next->ConfigurationChanged(tf, setAll);
}

bool GmatCommand::HasAFunction()
{
   // this default implementation just returns false
   return false;
}


//------------------------------------------------------------------------------
// bool NeedsServerStartup()
//------------------------------------------------------------------------------
/**
 * Indicates in the engine needs to start an external process to run the command
 * 
 * The Sandbox calls this method and tells the Moderator to start the MATLAB
 * engine if a true value is returned.  Note that the method name is not MATLAB 
 * specific; future builds may provide additional interfaces so that specific
 * servers can be started -- for example, Octave -- rather than just assuming 
 * that MATLAB is the engine to start.
 * 
 * @return true if a server needs to start, false otherwise.
 */
//------------------------------------------------------------------------------
bool GmatCommand::NeedsServerStartup()
{
   return false;
}

//------------------------------------------------------------------------------
// bool IsExecuting()
//------------------------------------------------------------------------------
/**
 * Indicates whether the command is executing or not.
 *
 * @return true if command is executing
 */
//------------------------------------------------------------------------------
bool GmatCommand::IsExecuting()
{
   return false;
}


//------------------------------------------------------------------------------
// Integer GetCloneCount()
//------------------------------------------------------------------------------
/**
 * Returns the number of clones the object created, so the Assignment command
 * can change their attributes
 *
 * @return The clone count
 */
//------------------------------------------------------------------------------
Integer GmatCommand::GetCloneCount()
{
   return cloneCount;
}


//------------------------------------------------------------------------------
// Gmat* GetClone(Integer cloneIndex = 0)
//------------------------------------------------------------------------------
/**
 * Retrieves a pointer to a clone
 *
 * @param cloneIndex The index of the clone used by objects that have more than
 *                   one clone
 *
 * @return The pointer
 */
//------------------------------------------------------------------------------
GmatBase* GmatCommand::GetClone(Integer cloneIndex)
{
   return NULL;
}

//------------------------------------------------------------------------------
// virtual void InsertCommandName(wxString &genString)
//------------------------------------------------------------------------------
/**
 * Inserts command name in quotes to generatin string. It puts single quotes
 * aroud the instanceName and inserts right after command type. If instanceName
 * is blank or command type is not found, it does nothing.
 *
 * @param  genString  Input/Output generating string to insert command name
 */
//------------------------------------------------------------------------------
void GmatCommand::InsertCommandName(wxString &genString)
{
   if (instanceName != wxT(""))
   {
      wxString::size_type insertPoint = genString.find_first_of(typeName);
      wxString nameInQuotes = wxT("'") + instanceName + wxT("'");
      #if DEBUG_GEN_STRING
      MessageInterface::ShowMessage
         (wxT("   typeName = <%s>, command name = <%s>, insertPoint = %d\n"),
          typeName.c_str(), instanceName.c_str(), insertPoint);
      #endif
      if (insertPoint != genString.npos)
      {
         insertPoint = insertPoint + typeName.size();
         #if DEBUG_GEN_STRING
         MessageInterface::ShowMessage(wxT("   insertPoint = %d\n"), insertPoint);
         #endif
         genString =
            genString.insert(insertPoint, wxT(" ") + nameInQuotes);
      }
   }
}


//------------------------------------------------------------------------------
// void ShowCommand(const wxString &prefix = "",
//                  const wxString &title1, GmatCommand *cmd1,
//                  const wxString &title2 = "", GmatCommand *cmd2 = NULL)
//------------------------------------------------------------------------------
/*
 * <static method>
 * Shows command info to message window.
 */
//------------------------------------------------------------------------------
void GmatCommand::ShowCommand(const wxString &prefix,
                              const wxString &title1, GmatCommand *cmd1,
                              const wxString &title2, GmatCommand *cmd2)
{
   if (title2 == wxT(""))
   {
      MessageInterface::ShowMessage
         (wxT("%s%s: %s<%p><%s>[%s]\n"), prefix.c_str(), this->GetTypeName().c_str(),
          title1.c_str(), cmd1, cmd1 ? cmd1->GetTypeName().c_str() : wxT("NULL"),
          cmd1 ? cmd1->GetGeneratingString(Gmat::NO_COMMENTS).c_str() : wxT("NULL"));
   }
   else
   {
      MessageInterface::ShowMessage
         (wxT("%s%s:\n   %s<%p><%s>[%s]\n   %s<%p><%s>[%s]\n"), prefix.c_str(),
          this->GetTypeName().c_str(), title1.c_str(), cmd1,
          cmd1 ? cmd1->GetTypeName().c_str() : wxT("NULL"),
          cmd1 ? cmd1->GetGeneratingString(Gmat::NO_COMMENTS).c_str() : wxT("NULL"),
          title2.c_str(), cmd2, cmd2 ? cmd2->GetTypeName().c_str() : wxT("NULL"),
          cmd2 ? cmd2->GetGeneratingString(Gmat::NO_COMMENTS).c_str() : wxT("NULL"));
   }
}


//------------------------------------------------------------------------------
// virtual void ShowObjectMaps(const wxString &str)
//------------------------------------------------------------------------------
void GmatCommand::ShowObjectMaps(const wxString &str)
{
   MessageInterface::ShowMessage
      (wxT("%s\n======================================================================\n"),
       str.c_str());
   MessageInterface::ShowMessage
      (wxT("GmatCommand::ShowObjectMaps() objectMap=<%p>, globalObjectMap=<%p>\n"),
       objectMap, globalObjectMap);
   
   if (objectMap)
   {
      MessageInterface::ShowMessage
         (wxT("Here is the local object map for %s:\n"), this->GetTypeName().c_str());
      for (std::map<wxString, GmatBase *>::iterator i = objectMap->begin();
           i != objectMap->end(); ++i)
         MessageInterface::ShowMessage
            (wxT("   %30s  <%p> [%s]\n"), i->first.c_str(), i->second,
             i->second == NULL ? wxT("NULL") : (i->second)->GetTypeName().c_str());
   }
   if (globalObjectMap)
   {
      MessageInterface::ShowMessage
         (wxT("Here is the global object map for %s:\n"), this->GetTypeName().c_str());
      for (std::map<wxString, GmatBase *>::iterator i = globalObjectMap->begin();
           i != globalObjectMap->end(); ++i)
         MessageInterface::ShowMessage
            (wxT("   %30s  <%p> [%s]\n"), i->first.c_str(), i->second,
             i->second == NULL ? wxT("NULL") : (i->second)->GetTypeName().c_str());
   }
   MessageInterface::ShowMessage
      (wxT("======================================================================\n"));
}


//------------------------------------------------------------------------------
// StringArray InterpretPreface()
//------------------------------------------------------------------------------
/**
 * Method used to do the common InterpretAction startup stuff
 * 
 * @return The top level chunks from the instruction contained in the generatng 
 *         string.
 * 
 * @notes - Original by Darrel Conway
 */
//------------------------------------------------------------------------------
StringArray GmatCommand::InterpretPreface()
{
   #ifdef DEBUG_INTERPRET_PREFACE
      MessageInterface::ShowMessage(wxT("In GmatCommand::InterpretPreface, generatingString = %s\n"),
            generatingString.c_str());
   #endif
   parser.EvaluateBlock(generatingString);
   StringArray blocks = parser.DecomposeBlock(generatingString);
   StringArray chunks = parser.ChunkLine();
   
   #ifdef DEBUG_INTERPRET_PREFACE
      MessageInterface::ShowMessage(wxT("   ater call to parser, generatingString = %s\n"),
            generatingString.c_str());
      MessageInterface::ShowMessage(wxT("   and blocks are:\n"));
      for (unsigned int ii = 0; ii < blocks.size(); ii++)
         MessageInterface::ShowMessage(wxT("      %s\n"), (blocks.at(ii)).c_str());
      MessageInterface::ShowMessage(wxT("   and chunks are:\n"));
      for (unsigned int ii = 0; ii < chunks.size(); ii++)
         MessageInterface::ShowMessage(wxT("      %s\n"), (chunks.at(ii)).c_str());
   #endif
   
   // First comes the command keyword
   // @note "GMAT" keyword is automatically removed
   if (chunks[0] != typeName && typeName != wxT("GMAT"))
      throw CommandException(
         wxT("Line \"") + generatingString +
         wxT("\"\n should be a ") + typeName + wxT(" command, but the \"") + typeName +
         wxT("\" keyword is not the opening token in the line.\n"));
   
   return chunks;
}

//------------------------------------------------------------------------------
// bool IsSettable(const wxString &setDesc)
//------------------------------------------------------------------------------
/**
 * Method used to check a string and see if it is local data
 * 
 * @param <setDesc> The string being checked.
 * 
 * @return true if the string is associated with a local data member, false if
 *         it is not.
 * 
 * @notes - Original by Darrel Conway
 */
//------------------------------------------------------------------------------
bool GmatCommand::IsSettable(const wxString &setDesc)
{
   if (find(settables.begin(), settables.end(), setDesc) != settables.end())
      return true;
   return false;

}

//------------------------------------------------------------------------------
// bool SeparateEquals(const wxString &description, 
//                     wxString &lhs, wxString &rhs)
//------------------------------------------------------------------------------
/**
 * Method used to separate 'lhs = rhs' style strings into a lhs and rhs.
 * 
 * @param <description> The string that needs to be separated.
 * @param <lhs>         The resulting left hand side.
 * @param <rhs>         The resulting right hand side, or the empty string if 
 *                      there was no right side.
 * @param <checkOp>     flag indicating whether or not to check to make sure
 *                      there are no "compound" operators using equals signs
 *                      (e.g. "<=", "==", ">=")
 * 
 * @return true if the string was separated into two pieces, false if only one
 *         piece.  The method throws if more than 2 pieces were found.
 */
//------------------------------------------------------------------------------
bool GmatCommand::SeparateEquals(const wxString &description,
                                 wxString &lhs, wxString &rhs,
                                 bool checkOp)
{
   if (checkOp)
      if ( (description.find(wxT("=="),0) != description.npos) ||
           (description.find(wxT(">="),0) != description.npos) ||
           (description.find(wxT("<="),0) != description.npos) ||
           (description.find(wxT("=>"),0) != description.npos) ||
           (description.find(wxT("=<"),0) != description.npos) ||
           (description.find(wxT("~="),0) != description.npos) ||
           (description.find(wxT("=~"),0) != description.npos) )
      {
         wxString msg = wxT("The string \"") + description;
         msg += wxT("\" contains a disallowed relational operator for this command: expecting \"=\" ");
         throw CommandException(msg);
      }
   StringArray sides = parser.SeparateBy(description, wxT("= "));
   #ifdef DEBUG_SEPARATE
      MessageInterface::ShowMessage(wxT("In SeparateEquals, description = %s\n"),
      description.c_str());
      MessageInterface::ShowMessage(wxT("----  and sides = \n"));
      for (Integer ii = 0; ii < (Integer) sides.size(); ii++)
         MessageInterface::ShowMessage(wxT("      %s\n"), sides[ii].c_str());    
   #endif
   lhs = sides[0];

   if (sides.size() < 2)
      return false;

   if (sides.size() == 2)
      rhs = sides[1];
   else
      rhs = wxT("");

   if (sides.size() > 2)
   {
      wxString msg = wxT("Error decomposing the string \"");
      msg += description;
      msg += wxT("\"\nTrying to separate into lhs and rhs on \"=\" sign, but found ");
      msg += wxT("too many pieces or missing separator character(s).\n");
      throw CommandException(msg);
   }

   return true;
}


//------------------------------------------------------------------------------
// Temporary -- need to figure out how we're supposed to do transformations 
// generically
//------------------------------------------------------------------------------
void GmatCommand::CartToKep(const Rvector6 in, Rvector6 &out)
{
   GmatBase* earth = FindObject(GmatSolarSystemDefaults::EARTH_NAME);
   if (!earth)
      throw CommandException(
            wxT("GmatCommand::CartToKep failed to find object named \"") +
            GmatSolarSystemDefaults::EARTH_NAME + wxT("\" in: \n   \"") + GetGeneratingString(Gmat::NO_COMMENTS) + wxT("\"\n"));

   Real mu = ((CelestialBody*)earth)->GetGravitationalConstant();
   Real r = sqrt(in[0]*in[0]+in[1]*in[1]+in[2]*in[2]);
   Real v2 = in[3]*in[3]+in[4]*in[4]+in[5]*in[5];
   Real rdotv = in[0]*in[3] + in[1]*in[4] + in[2]*in[5];
   Real energy = 0.5*v2 - mu / r;
   /// SMA
   out[0] = -mu / (2.0 * energy);
   
   Rvector3 h, ecc;

   // angular vel = r cross v
   h[0] = in[1]*in[5] - in[2]*in[4];
   h[1] = in[2]*in[3] - in[0]*in[5];
   h[2] = in[0]*in[4] - in[1]*in[3];
   // eccentricity vector
   ecc[0] = (in[4]*h[2] - in[5]*h[1]) / mu - in[0] / r;
   ecc[1] = (in[5]*h[0] - in[3]*h[2]) / mu - in[1] / r;
   ecc[2] = (in[3]*h[1] - in[4]*h[0]) / mu - in[2] / r;
   
   // eccentricity
   out[1] = sqrt(ecc[0]*ecc[0] + ecc[1]*ecc[1] + ecc[2]*ecc[2]);
   
   // inclination
   Real hMag = sqrt(h[0]*h[0] + h[1]*h[1] + h[2]*h[2]);
   out[2] = acos(h[2]/hMag) * 180.0 / M_PI;
   
   Rvector3 nodevec;
   nodevec[0] = -h[1];
   nodevec[1] = h[0];
   nodevec[2] = 0.0;

   Real magnodevec = sqrt(nodevec[0]*nodevec[0] + nodevec[1]*nodevec[1]);

   // RAAN
   out[3] = atan2(nodevec[1], nodevec[0]) * 180.0 / M_PI;

   // AOP
   out[4] = acos((ecc[0]*nodevec[0] + ecc[1]*nodevec[1]) / (out[1]*magnodevec));
   if (ecc[2] < 0)
      out[4] = 2.0 * M_PI - out[4];
   out[4] *= 180.0 / M_PI;

   // TA
   out[5] = acos((ecc[0]*in[0] + ecc[1]*in[1] + ecc[2]*in[2]) / 
                 (out[1] * r));
   if (rdotv < 0)
      out[5] = 2.0 * M_PI - out[5];
   out[5] *= 180.0 / M_PI;
}


//------------------------------------------------------------------------------
// GmatBase* GmatCommand::FindObject(const wxString &name)
//------------------------------------------------------------------------------
GmatBase* GmatCommand::FindObject(const wxString &name)
{
   wxString newName = name;
   
   // Ignore array indexing of Array
   wxString::size_type index = name.find(wxT('('));
   if (index != name.npos)
      newName = name.substr(0, index);
   
   #ifdef DEBUG_FIND_OBJECT
   MessageInterface::ShowMessage
      (wxT("GmatCommand::FindObject() entered, name='%s', newName='%s'\n"), name.c_str(),
       newName.c_str());
   #endif
   
   #ifdef DEBUG_OBJECT_MAP
   ShowObjectMaps();
   #endif
   
   // Check for SolarSystem (loj: 2008.06.25)
   if (name == wxT("SolarSystem"))
   {
      if (solarSys)
         return solarSys;
      else
         return NULL;
   }
   
   // Check for the object in the Local Object Store (LOS) first
   if (objectMap && objectMap->find(newName) != objectMap->end())
   {
      #ifdef DEBUG_FIND_OBJECT
      MessageInterface::ShowMessage
         (wxT("GmatCommand::FindObject() '%s' found in LOS, so returning <%p>\n"),
          newName.c_str(), (*objectMap)[newName]);
      #endif
      return (*objectMap)[newName];
   }
   
   // If not found in the LOS, check the Global Object Store (GOS)
   if (globalObjectMap && globalObjectMap->find(newName) != globalObjectMap->end())
   {
      #ifdef DEBUG_FIND_OBJECT
      MessageInterface::ShowMessage
         (wxT("GmatCommand::FindObject() '%s' found in GOS, so returning <%p>\n"),
          newName.c_str(), (*globalObjectMap)[newName]);
      #endif
      return (*globalObjectMap)[newName];
   }
   
   // Let's try SolarSystem (loj: 2008.06.04)
   if (solarSys && solarSys->GetBody(newName))
   {
      #ifdef DEBUG_FIND_OBJECT
      MessageInterface::ShowMessage
         (wxT("GmatCommand::FindObject() '%s' found in SolarSystem, so returning <%p>\n"),
          newName.c_str(), (GmatBase*)(solarSys->GetBody(newName)));
      #endif
      return (GmatBase*)(solarSys->GetBody(newName));
   }
   
   #ifdef DEBUG_FIND_OBJECT
   MessageInterface::ShowMessage
      (wxT("GmatCommand::FindObject() '%s' not found, so returning NULL\n"),
       newName.c_str());
   #endif
   
   return NULL;
}


//------------------------------------------------------------------------------
// bool SetWrapperReferences(ElementWrapper &wrapper)
//------------------------------------------------------------------------------
bool GmatCommand::SetWrapperReferences(ElementWrapper &wrapper)
{
   if (&wrapper != NULL)
   {
      #ifdef DEBUG_WRAPPER_CODE
         MessageInterface::ShowMessage
            (wxT("GmatCommand::SetWrapperReferences() Setting refs for wrapper \"%s\"\n"), 
            wrapper.GetDescription().c_str());
      #endif
      StringArray onames = wrapper.GetRefObjectNames();
      
      for (StringArray::const_iterator j = onames.begin(); j != onames.end(); ++j)
      {
         wxString name = *j;
         #ifdef DEBUG_WRAPPER_CODE
         MessageInterface::ShowMessage(wxT("      name='%s', now finding object...\n"), name.c_str());
         #endif
         GmatBase *obj = FindObject(name);
         if (obj == NULL)
         {
            if (name == wxT(""))
               continue;
            
            throw CommandException(
                  wxT("GmatCommand::SetWrapperReferences failed to find object named \"") + 
                  name + wxT("\" in: \n   \"") + GetGeneratingString(Gmat::NO_COMMENTS) + wxT("\"\n"));
         }
         if (wrapper.SetRefObject(obj) == false)
         {
            MessageInterface::ShowMessage
               (wxT("GmatCommand::SetWrapperReferences failed to set object named \"%s\", ")
                wxT("name in the map is \"%s\"\n"), name.c_str(), obj->GetName().c_str());
            return false;
         }
         #ifdef DEBUG_WRAPPER_CODE
            MessageInterface::ShowMessage(wxT("      Set reference object \"%s\"\n"), 
               name.c_str());
         #endif
      }
   }
   else
      throw CommandException(wxT("GmatCommand::SetWrapperReferences was passed a ")
         wxT("NULL object instead of a wrapper in:\n   \"") + generatingString + wxT("\"\n"));
   
   #ifdef DEBUG_WRAPPER_CODE
   MessageInterface::ShowMessage(wxT("GmatCommand::SetWrapperReferences() returning true\n"));
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// void ClearOldWrappers()
//------------------------------------------------------------------------------
void GmatCommand::ClearOldWrappers()
{
   oldWrappers.clear();
}


//------------------------------------------------------------------------------
// void CollectOldWrappers(ElementWrapper **wrapper)
//------------------------------------------------------------------------------
void GmatCommand::CollectOldWrappers(ElementWrapper **wrapper)
{
   #ifdef DEBUG_WRAPPER_CODE
   MessageInterface::ShowMessage
      (wxT("GmatCommand::CollectOldWrappers() <%p>'%s' entered, wrapper=<%p>\n"),
       this, GetTypeName().c_str(), *wrapper);
   MessageInterface::ShowMessage(wxT("   There are %d old wrappers\n"), oldWrappers.size());
   #endif
   
   if (*wrapper)
   {
      if (find(oldWrappers.begin(), oldWrappers.end(), *wrapper) == oldWrappers.end())
      {
         oldWrappers.push_back(*wrapper);
         *wrapper = NULL;
      }
   }
   
   #ifdef DEBUG_WRAPPER_CODE
   MessageInterface::ShowMessage
      (wxT("GmatCommand::CollectOldWrappers() <%p>'%s' leaving\n"), this, GetTypeName().c_str());
   MessageInterface::ShowMessage(wxT("   There are %d old wrappers\n"), oldWrappers.size());
   #endif
}


//------------------------------------------------------------------------------
// void DeleteOldWrappers()
//------------------------------------------------------------------------------
void GmatCommand::DeleteOldWrappers()
{
   #ifdef DEBUG_WRAPPER_CODE
   MessageInterface::ShowMessage
      (wxT("GmatCommand::DeleteOldWrappers() <%p>'%s' entered, has %d old wrappers\n"),
       this, GetTypeName().c_str(), oldWrappers.size());
   #endif
   
   ElementWrapper *wrapper;
   WrapperArray wrappersToDelete;
   
   // Add wrappers to delete
   for (UnsignedInt i = 0; i < oldWrappers.size(); ++i)
   {
      wrapper = oldWrappers[i];
      if (find(wrappersToDelete.begin(), wrappersToDelete.end(), wrapper) ==
          wrappersToDelete.end())
      {
         wrappersToDelete.push_back(wrapper);
         #ifdef DEBUG_WRAPPER_CODE
         MessageInterface::ShowMessage(wxT("   <%p> added to wrappersToDelete\n"), wrapper);
         #endif
      }
   }
   
   // Delete wrappers
   for (UnsignedInt i = 0; i < wrappersToDelete.size(); ++i)
   {
      wrapper = wrappersToDelete[i];
      #ifdef DEBUG_WRAPPER_CODE
      MessageInterface::ShowMessage
         (wxT("   wrapper=<%p>'%s'\n", wrapper, wrapper ? wrapper->GetDescription().c_str() : "NULL"));
      #endif
      if (wrapper)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (wrapper, wrapper->GetDescription(), wxT("GmatCommand::ClearWrappers()"),
             GetTypeName() + wxT(" deleting wrapper"));
         #endif
         delete wrapper;
         wrapper = NULL;
      }
   }
   
   oldWrappers.clear();
   
   #ifdef DEBUG_WRAPPER_CODE
   MessageInterface::ShowMessage
      (wxT("GmatCommand::DeleteOldWrappers() <%p>'%s' leaving, has %d old wrappers\n"),
       this, GetTypeName().c_str(), oldWrappers.size());
   #endif
}


void GmatCommand::PrepareToPublish(bool publishAll)
{
   StringArray owners, elements;

   if (publishAll)
   {
      owners.push_back(wxT("All"));
      elements.push_back(wxT("All.epoch"));
   }

   streamID = publisher->RegisterPublishedData(this, streamID, owners,
         elements);
}


void GmatCommand::PublishData()
{
   publisher->Publish(this, streamID, NULL, 0);
}
