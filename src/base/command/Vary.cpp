//$Id: Vary.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                   Vary
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P
//
// Author: Darrel J. Conway
// Created: 2004/01/22
//
/**
 * Definition for the Vary command class
 */
//------------------------------------------------------------------------------


#include "Vary.hpp"
#include "DifferentialCorrector.hpp"
#include "Parameter.hpp"
#include "StringUtil.hpp"  // for Replace()
#include "MessageInterface.hpp"
#include "TextParser.hpp"
#include <sstream>         // for wxStringstream


//#define DEBUG_VARIABLE_RANGES
//#define DEBUG_VARY_EXECUTE
//#define DEBUG_VARY_PARAMS
//#define DEBUG_VARY_PARSING
//#define DEBUG_WRAPPER_CODE
//#define DEBUG_RENAME
//#define DEBUG_VARY_GEN_STRING

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//------------------------------------------------------------------------------
//  static data
//------------------------------------------------------------------------------
const wxString Vary::PARAMETER_TEXT[VaryParamCount -
                                       GmatCommandParamCount] = 
{
   wxT("SolverName"),
   wxT("Variable"),
   wxT("InitialValue"),
   wxT("Perturbation"),
   wxT("Lower"),
   wxT("Upper"),
   wxT("MaxStep"),
   wxT("AdditiveScaleFactor"),
   wxT("MultiplicativeScaleFactor")   
};

const Gmat::ParameterType Vary::PARAMETER_TYPE[VaryParamCount -
                                               GmatCommandParamCount] = 
{
   Gmat::STRING_TYPE,
   Gmat::STRING_TYPE,
   Gmat::STRING_TYPE,
   Gmat::STRING_TYPE,
   Gmat::STRING_TYPE,
   Gmat::STRING_TYPE,
   Gmat::STRING_TYPE,
   Gmat::STRING_TYPE,
   Gmat::STRING_TYPE,
};
 

//------------------------------------------------------------------------------
//  Vary(void)
//------------------------------------------------------------------------------
/**
 * Creates a Vary command.  (default constructor)
 */
//------------------------------------------------------------------------------
Vary::Vary() :
   GmatCommand                   (wxT("Vary")),
   solverName                    (wxT("")),
   solver                        (NULL),
   variableName                  (wxT("")),
   variable                      (NULL),
   initialValueName              (wxT("0.0")),
   initialValue                  (NULL),
   currentValue                  (0.0),
   perturbationName              (wxT("0.001")),
   perturbation                  (NULL),
   variableLowerName             (wxT("-9.999999e300")),
   variableLower                 (NULL),
   variableUpperName             (wxT("9.999999e300")),
   variableUpper                 (NULL),
   variableMaximumStepName       (wxT("9.999999e300")),
   variableMaximumStep           (NULL),
   additiveScaleFactorName       (wxT("0.0")),
   additiveScaleFactor           (NULL),
   multiplicativeScaleFactorName (wxT("1.0")),
   multiplicativeScaleFactor     (NULL),
   variableID                    (-1),
   solverDataFinalized           (false)
{
   settables.push_back(wxT("Perturbation")); 
   settables.push_back(wxT("MaxStep"));
   settables.push_back(wxT("Lower"));  
   settables.push_back(wxT("Upper"));  
   settables.push_back(wxT("AdditiveScaleFactor"));
   settables.push_back(wxT("MultiplicativeScaleFactor"));
//    //parameterCount += 7;
//    additiveScaleFactor.push_back(0.0);
//    multiplicativeScaleFactor.push_back(1.0);
   parameterCount = VaryParamCount;
}


    
//------------------------------------------------------------------------------
//  Vary(const Vary& t)
//------------------------------------------------------------------------------
/**
 * Constructor that replicates a Vary command.  (Copy constructor)
 *
 * @return A reference to this instance.
 */
//------------------------------------------------------------------------------
Vary::Vary(const Vary& t) :
   GmatCommand                   (t),
   solverName                    (t.solverName),
   solver                        (NULL),
   variableName                  (t.variableName),
   variable                      (NULL),
   initialValueName              (t.initialValueName),
   initialValue                  (NULL),
   currentValue                  (0.0),
   perturbationName              (t.perturbationName),
   perturbation                  (NULL),
   variableLowerName           (t.variableLowerName),
   variableLower               (NULL),
   variableUpperName           (t.variableUpperName),
   variableUpper               (NULL),
   variableMaximumStepName       (t.variableMaximumStepName),
   variableMaximumStep           (NULL),
   additiveScaleFactorName       (t.additiveScaleFactorName),
   additiveScaleFactor           (NULL),
   multiplicativeScaleFactorName (t.multiplicativeScaleFactorName),
   multiplicativeScaleFactor     (NULL),
   variableID                    (-1),
   solverDataFinalized           (t.solverDataFinalized)
{
    parameterCount = t.parameterCount;
}


//------------------------------------------------------------------------------
//  Vary& operator=(const Vary& t)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the Vary command.
 *
 * @return A reference to this instance.
 */
//------------------------------------------------------------------------------
Vary& Vary::operator=(const Vary& t)
{
   if (this == &t)
      return *this;
        
   solverName                    = t.solverName;
   variableName                  = t.variableName;
   initialValueName              = t.initialValueName;
   currentValue                  = 0.0;
   perturbationName              = t.perturbationName;
   variableLowerName             = t.variableLowerName;
   variableUpperName             = t.variableUpperName;
   variableMaximumStepName       = t.variableMaximumStepName;
   additiveScaleFactorName       = t.additiveScaleFactorName;
   multiplicativeScaleFactorName = t.multiplicativeScaleFactorName;

   // Initialize wrappers to NULL
   ClearWrappers();

   solver = NULL;
   variableID = -1;
   solverDataFinalized = t.solverDataFinalized;
    
   return *this;
}

//------------------------------------------------------------------------------
//  ~Vary(void)
//------------------------------------------------------------------------------
/**
 * Destroys the Vary command.  (destructor)
 */
//------------------------------------------------------------------------------
Vary::~Vary()
{
   ClearWrappers();
}



//------------------------------------------------------------------------------
//  GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the Vary.
 *
 * @return clone of the Vary.
 *
 */
//------------------------------------------------------------------------------
GmatBase* Vary::Clone() const
{
   return (new Vary(*this));
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
 * @param mode    Specifies the type of serialization requested.
 * @param prefix  Optional prefix appended to the object's name.  (Used to
 *                indent commands)
 * @param useName Name that replaces the object's name.  (Not used in
 *                commands)
 *
 * @return The script line that, when interpreted, defines this Vary command.
 */
//------------------------------------------------------------------------------
const wxString& Vary::GetGeneratingString(Gmat::WriteMode mode,
                                             const wxString &prefix,
                                             const wxString &useName)
{
   #ifdef DEBUG_VARY_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("Vary::GetGeneratingString() <%p>'%s' entered\n"), this, GetTypeName().c_str());
   #endif
   
   // Build the local string
   wxString details;
   
   wxString gen = prefix + wxT("Vary ") + solverName + wxT("(");
   
   // loj: added check for NULL pointer to avoid crash and we can still
   // get generating string (2008.05.22)
   // Iterate through the variables
   if (variable)
      details << variable->GetDescription() << wxT(" = ");
   else
      details << wxT("Unknown-Variable") << wxT(" = ");
   
   if (initialValue)
      if (initialValue->GetWrapperType() != Gmat::NUMBER_WT)
         details << initialValue->GetDescription() <<  wxT(", ");
      else
         details << initialValue->EvaluateReal() <<  wxT(", ");
   else
      details << wxT("Unknown-InitialValue")  <<  wxT(", ");
   
   details << wxT("{");
   Integer addCount = 0;
   
   if (solver)
   {
      #ifdef DEBUG_VARY_GEN_STRING
      MessageInterface::ShowMessage
         (wxT("   solver = <%p><%s>'%s'\n"), solver, solver->GetTypeName().c_str(),
          solver->GetName().c_str());
      #endif
      
      Integer id = solver->GetParameterID(wxT("AllowVariablePertSetting"));
      if (solver->GetBooleanParameter(id))
      {
         details << wxT("Perturbation = ");
         if (perturbation)
            details << perturbation->GetDescription();
         else
            details << wxT("Unknown-Perturbation");
         
         addCount++;
      }
      
      id = solver->GetParameterID(wxT("AllowRangeSettings"));
      if (solver->GetBooleanParameter(id))
      {
         if (addCount > 0)
            details << wxT(", ");
         
         details << wxT("Lower = ");
         if (variableLower)
            details << variableLower->GetDescription();
         else
            details << wxT("Unknown-VariableLower");
         
         details << wxT(", Upper = ");
         if (variableUpper)
            details << variableUpper->GetDescription();
         else
            details << wxT("Unknown-VariableUpper");
         
         addCount++;
      }
      
      id = solver->GetParameterID(wxT("AllowStepsizeSetting"));
      if (solver->GetBooleanParameter(id))
      {
         if (addCount > 0)
            details << wxT(", ");
         
         details << wxT("MaxStep = ");
         
         if (variableMaximumStep)
            details << variableMaximumStep->GetDescription();
         else
            details << wxT("Unknown-VariableMaximumStep");
         
         addCount++;
      }
      
      id = solver->GetParameterID(wxT("AllowScaleSetting"));
      if (solver->GetBooleanParameter(solver->GetParameterID(wxT("AllowScaleSetting"))))
      {
         if (addCount > 0)
            details << wxT(", ");
         
         details << wxT("AdditiveScaleFactor = ");
         if (additiveScaleFactor)
            details << additiveScaleFactor->GetDescription();
         else
            details << wxT("Unknown-AdditiveScaleFactor");
         
         details << wxT(", MultiplicativeScaleFactor = ");
         if (multiplicativeScaleFactor)
            details << multiplicativeScaleFactor->GetDescription();
         else
            details << wxT("Unknown-MultiplicativeScaleFactor");
      }
   }
   else
   {
      #ifdef DEBUG_VARY_GEN_STRING
      MessageInterface::ShowMessage(wxT("   solver is NULL\n"));
      #endif

      details << wxT("SOLVER IS NOT SET");
   }
   
   gen += details;
   generatingString = gen + wxT("});");
   
   #ifdef DEBUG_VARY_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("Vary::GetGeneratingString() <%p>'%s' returning, '%s'\n"),
       this, GetTypeName().c_str(), generatingString.c_str());
   #endif
   
   // Then call the base class method to handle comments
   return GmatCommand::GetGeneratingString(mode, prefix, useName);
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
bool Vary::RenameRefObject(const Gmat::ObjectType type,
                           const wxString &oldName,
                           const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("Vary::RenameRefObject() type=%d, oldName=%s, newName=%s\n"),
       type, oldName.c_str(), newName.c_str());
   #endif

   if (type == Gmat::SOLVER)
   {
      if (solverName == oldName)
         solverName = newName;
   }
   
   // make sure the wrappers know to rename any objects they may be using
   if (variable)
   {
      variable->RenameObject(oldName, newName);
      variableName           = variable->GetDescription();
   }
   if (initialValue)        
   {
      initialValue->RenameObject(oldName, newName);
      initialValueName       = initialValue->GetDescription();
   }
   if (perturbation)        
   {
      perturbation->RenameObject(oldName, newName);
      perturbationName       = perturbation->GetDescription();
   }
   if (variableLower)     
   {
      variableLower->RenameObject(oldName, newName);
      variableLowerName    = variableLower->GetDescription();
   }
   if (variableUpper)     
   {
      variableUpper->RenameObject(oldName, newName);
      variableUpperName    = variableUpper->GetDescription();
   }
   if (variableMaximumStep) 
   {
      variableMaximumStep->RenameObject(oldName, newName);
      variableMaximumStepName= variableMaximumStep->GetDescription();
   }
   if (additiveScaleFactor) 
   {    
      additiveScaleFactor->RenameObject(oldName, newName);
      additiveScaleFactorName  = additiveScaleFactor->GetDescription();
   }
   if (multiplicativeScaleFactor) 
   {
      multiplicativeScaleFactor->RenameObject(oldName, newName);
       multiplicativeScaleFactorName    
                      = multiplicativeScaleFactor->GetDescription();
   }
   
   return true;
}


//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by the Vary.
 *
 * @return the list of object types.
 * 
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& Vary::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::SOLVER);
   return refObjectTypes;
}


//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref objects used by the Vary.
 *
 * @param <type> The type of object desired, or Gmat::UNKNOWN_OBJECT for the
 *               full list.
 * 
 * @return the list of object names.
 * 
 */
//------------------------------------------------------------------------------
const StringArray& Vary::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   refObjectNames.clear();
   
   if (type == Gmat::UNKNOWN_OBJECT ||
       type == Gmat::SOLVER)
   {
      refObjectNames.push_back(solverName);
   }
   
   return refObjectNames;
}


//---------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//---------------------------------------------------------------------------
wxString Vary::GetParameterText(const Integer id) const
{
   if ((id >= GmatCommandParamCount) && (id < VaryParamCount))
      return PARAMETER_TEXT[id - GmatCommandParamCount];

   return GmatCommand::GetParameterText(id);
}


//---------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//---------------------------------------------------------------------------
Integer Vary::GetParameterID(const wxString &str) const
{
   for (Integer i = GmatCommandParamCount; i < VaryParamCount; ++i)
   {
      if (str == PARAMETER_TEXT[i - GmatCommandParamCount])
         return i;
   }

   return GmatCommand::GetParameterID(str);
}


//---------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//---------------------------------------------------------------------------
Gmat::ParameterType Vary::GetParameterType(const Integer id) const
{
   if ((id >= GmatCommandParamCount) && (id < VaryParamCount))
      return PARAMETER_TYPE[id - GmatCommandParamCount];

    return GmatCommand::GetParameterType(id);
}


//---------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//---------------------------------------------------------------------------
wxString Vary::GetParameterTypeString(const Integer id) const
{
   return GmatCommand::PARAM_TYPE_STRING[GetParameterType(id)];
}


//---------------------------------------------------------------------------
// Real GetRealParameter(const Integer id) const
//---------------------------------------------------------------------------
Real Vary::GetRealParameter(const Integer id) const
{
   if (id == INITIAL_VALUE)
      if (initialValue)
         return initialValue->EvaluateReal();
        
   if (id == PERTURBATION)
      if (perturbation)
         return perturbation->EvaluateReal();
        
   if (id == VARIABLE_LOWER)
      if (variableLower)
         return variableLower->EvaluateReal();
        
   if (id == VARIABLE_UPPER)
      if (variableUpper)
         return variableUpper->EvaluateReal();
        
   if (id == VARIABLE_MAXIMUM_STEP)
      if (variableMaximumStep)
         return variableMaximumStep->EvaluateReal();
            
   if (id == ADDITIVE_SCALE_FACTOR)
      if (additiveScaleFactor)
         return additiveScaleFactor->EvaluateReal();
         
   if (id == MULTIPLICATIVE_SCALE_FACTOR)
      if (multiplicativeScaleFactor)
         return multiplicativeScaleFactor->EvaluateReal();
         
    return GmatCommand::GetRealParameter(id);
}


//---------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value)
//---------------------------------------------------------------------------
Real Vary::SetRealParameter(const Integer id, const Real value)
{
   #ifdef DEBUG_VARY_PARAMS
      MessageInterface::ShowMessage(
      wxT("Vary::Setting value of %f for parameter %s\n"), value,
      (GetParameterText(id)).c_str());
   #endif

   return GmatCommand::SetRealParameter(id, value);
}


//---------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//---------------------------------------------------------------------------
wxString Vary::GetStringParameter(const Integer id) const
{
   //if (id == solverNameID)
   if (id == SOLVER_NAME)
      return solverName;
        
   //if (id == variableNameID)
   if (id == VARIABLE_NAME)
      return variableName;
    
   if (id == INITIAL_VALUE)
      return initialValueName;
    
   if (id == PERTURBATION)
      return perturbationName;
    
   if (id == VARIABLE_LOWER)
      return variableLowerName;
    
   if (id == VARIABLE_UPPER)
      return variableUpperName;
    
   if (id == VARIABLE_MAXIMUM_STEP)
      return variableMaximumStepName;
    
   if (id == ADDITIVE_SCALE_FACTOR)
      return additiveScaleFactorName;
    
   if (id == MULTIPLICATIVE_SCALE_FACTOR)
      return multiplicativeScaleFactorName;
    
   return GmatCommand::GetStringParameter(id);
}


//---------------------------------------------------------------------------
//  wxString GetStringParameter(const wxString &label) const
//---------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
wxString Vary::GetStringParameter(const wxString &label) const
{
   Integer id = GetParameterID(label);
   return GetStringParameter(id);
}


//---------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//---------------------------------------------------------------------------
bool Vary::SetStringParameter(const Integer id, const wxString &value)
{
   if (id == SOLVER_NAME)
   {
      solverName = value;
      return true;
   }
   
   if (id == VARIABLE_NAME)
   {
      variableName = value;
      return true;
   }
   
   if ((id == VARIABLE_NAME) || (id == INITIAL_VALUE) || 
       (id == PERTURBATION) || (id == VARIABLE_LOWER) || 
       (id == VARIABLE_UPPER) || (id == VARIABLE_MAXIMUM_STEP) || 
       (id == ADDITIVE_SCALE_FACTOR) || (id == MULTIPLICATIVE_SCALE_FACTOR))
   {
      if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), value) == 
          wrapperObjectNames.end())
         wrapperObjectNames.push_back(value);
      
      // Also need to store the names for later mapping
      switch (id)
      {
         case INITIAL_VALUE:
            initialValueName = value;
            break;
         
         case PERTURBATION:
            perturbationName = value;
            break;
         
         case VARIABLE_LOWER:
            variableLowerName = value;
            break;
            
         case VARIABLE_UPPER:
            variableUpperName = value;
            break;
         
         case VARIABLE_MAXIMUM_STEP:
            variableMaximumStepName = value;
            break;
         
         case ADDITIVE_SCALE_FACTOR:
            additiveScaleFactorName = value;
            break;
         
         case MULTIPLICATIVE_SCALE_FACTOR:
            multiplicativeScaleFactorName = value;
            break;
         
         default:
            throw GmatBaseException(wxT("Unknown property for Vary command\n"));
      }
      
      return true;
   }
    
   return GmatCommand::SetStringParameter(id, value);
}


//---------------------------------------------------------------------------
//  bool SetStringParameter(const wxString &label, const wxString &value)
//---------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
bool Vary::SetStringParameter(const wxString &label, const wxString &value)
{
   Integer id = GetParameterID(label);
   return SetStringParameter(id, value);
}


//------------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                    const wxString &name = "")
//------------------------------------------------------------------------------
/**
 * Sets referenced objects.
 *
 * @param obj reference object pointer.
 * @param type type of the reference object.
 * @param name name of the reference object.
 *
 * @return success of the operation.
 */
//------------------------------------------------------------------------------
bool Vary::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                        const wxString &name)
{
   if (type == Gmat::SOLVER)
   {
      if (solverName == obj->GetName()) 
      {
         solver = (Solver*)obj;
         return true;
      }
      return false;
   }
   return GmatCommand::SetRefObject(obj, type, name);
}


//------------------------------------------------------------------------------
//  void Vary::InterpretAction()
//------------------------------------------------------------------------------
/**
 * Parses the command string and builds the corresponding command structures.
 *
 * The Vary command has the following syntax:
 *
 *     Vary myDC(Burn1.V = 0.5, {Pert = 0.0001, MaxStep = 0.05, 
 *               Lower = 0.0, Upper = 3.14159);
 *
 * where 
 *
 * 1. myDC is a Solver used to Vary a set of variables to achieve the
 * corresponding goals, 
 * 2. Burn1.V is the parameter that is varied, and
 * 3. The settings in the braces specify features about how the variable can
 * be changed.
 * 
 * This method breaks the script line into the corresponding pieces, and stores 
 * the name of the Solver so it can be set to point to the correct object 
 * during initialization.
 */
//------------------------------------------------------------------------------
bool Vary::InterpretAction()
{
   // Clean out any old data
   wrapperObjectNames.clear();
   ClearWrappers();

   StringArray chunks = InterpretPreface();

   #ifdef DEBUG_VARY_PARSING
      MessageInterface::ShowMessage(wxT("Preface chunks as\n"));
      for (StringArray::iterator i = chunks.begin(); i != chunks.end(); ++i)
         MessageInterface::ShowMessage(wxT("   \"%s\"\n"), i->c_str());
      MessageInterface::ShowMessage(wxT("\n"));
   #endif

   if (chunks.size() <= 1)
      throw CommandException(wxT("Missing information for Vary command.\n"));
   
   if (chunks[1].at(0) == wxT('('))
      throw CommandException(wxT("Missing solver name for Vary command.\n"));
   
   if ((chunks[1].find(wxT("[")) != chunks[1].npos) || (chunks[1].find(wxT("]")) != chunks[1].npos))
      throw CommandException(wxT("Brackets not allowed in Vary command"));

   if (!GmatStringUtil::AreAllBracketsBalanced(chunks[1], wxT("({)}")))      
      throw CommandException
         (wxT("Parentheses, braces, or brackets are unbalanced"));

      // Find and set the solver object name
   // This is the only setting in Vary that is not in a wrapper
   StringArray currentChunks = parser.Decompose(chunks[1], wxT("()"), false);
   SetStringParameter(SOLVER_NAME, currentChunks[0]);
   
   // The remaining text in the instruction is the variable definition and 
   // parameters, all contained in currentChunks[1].  Deal with those next.
   //currentChunks = parser.SeparateBrackets(currentChunks[1], "()", ", ");
   //wxString noLeftBrace  = GmatStringUtil::RemoveAll(currentChunks[1],'{');
   //wxString noRightBrace = GmatStringUtil::RemoveAll(noLeftBrace,'}');
   //wxString noSpaces     = GmatStringUtil::RemoveAll(noRightBrace,' ');
   wxString noSpaces2     = GmatStringUtil::RemoveAll(currentChunks[1],wxT(' '));
   currentChunks = parser.Decompose(noSpaces2, wxT("()"), true, true);
   //currentChunks = parser.Decompose(currentChunks[1], "()", true, true);
   
   #ifdef DEBUG_VARY_PARSING
      //MessageInterface::ShowMessage(
      //   "Vary:  noSpaces = %s\n", noSpaces.c_str());
      MessageInterface::ShowMessage(
         wxT("Vary: after Decompose, current chunks = \n"));
      for (Integer jj = 0; jj < (Integer) currentChunks.size(); jj++)
         MessageInterface::ShowMessage(wxT("   %s\n"),
                                       currentChunks[jj].c_str());
   #endif

   // First chunk is the variable and initial value
   wxString lhs, rhs;
   if (!SeparateEquals(currentChunks[0], lhs, rhs, true))
      // Variable takes default initial value
      //rhs = "0.0";
   {
      throw CommandException(wxT("The variable \"") + lhs + 
         wxT("\" is missing the \"=\" operator or an initial value required for a ") + typeName + 
         wxT(" command.\n"));
   }
      
   variableName = lhs;
   variableID = -1;
   
   initialValueName = rhs;
   
   #ifdef DEBUG_VARY_PARSING
      MessageInterface::ShowMessage(
         wxT("Vary:  setting variableName to %s\n"), variableName.c_str());
      MessageInterface::ShowMessage(
         wxT("Vary:  setting initialValueName to %s\n"), initialValueName.c_str());
   #endif
   
   if (currentChunks.size() > 1)
   {
      wxString noSpaces     = GmatStringUtil::RemoveAll(currentChunks[1],wxT(' '));
      // Now deal with the settable parameters
      //currentChunks = parser.SeparateBrackets(currentChunks[1], "{}", ",", false);
      currentChunks = parser.SeparateBrackets(noSpaces, wxT("{}"), wxT(","), true);
      
      #ifdef DEBUG_VARY_PARSING
         MessageInterface::ShowMessage(
            wxT("Vary: After SeparateBrackets, current chunks = \n"));
         for (Integer jj = 0; jj < (Integer) currentChunks.size(); jj++)
            MessageInterface::ShowMessage(wxT("   %s\n"),
                                          currentChunks[jj].c_str());
      #endif
      
      // currentChunks now holds all of the pieces - no need for more separation  
      
      for (StringArray::iterator i = currentChunks.begin(); 
           i != currentChunks.end(); ++i)
      {
         bool isOK = SeparateEquals(*i, lhs, rhs, true);
         #ifdef DEBUG_VARY_PARSING
            MessageInterface::ShowMessage(wxT("Setting Vary properties\n"));
            MessageInterface::ShowMessage(wxT("   \"%s\" = \"%s\"\n"), lhs.c_str(), rhs.c_str());
         #endif
         if (!isOK || lhs.empty() || rhs.empty())
            throw CommandException(wxT("The setting \"") + lhs + 
               wxT("\" is missing the \"=\" operator or a value required for a ") + typeName + 
               wxT(" command.\n"));
         
         if (IsSettable(lhs))
            SetStringParameter(GetParameterID(lhs), rhs);
         else
            throw CommandException(wxT("The setting \"") + lhs + 
               wxT("\" is not a valid setting for a ") + typeName + 
               wxT(" command.\n"));
      }
   }
   return true;
}


//------------------------------------------------------------------------------
// const StringArray& GetWrapperObjectNameArray()
//------------------------------------------------------------------------------
const StringArray& Vary::GetWrapperObjectNameArray()
{
   wrapperObjectNames.clear();

   wrapperObjectNames.push_back(variableName);
   
   if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), 
       initialValueName) == wrapperObjectNames.end())
      wrapperObjectNames.push_back(initialValueName);

   if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), 
       perturbationName) == wrapperObjectNames.end())
      wrapperObjectNames.push_back(perturbationName);
       
   if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), 
       variableLowerName) == wrapperObjectNames.end())
      wrapperObjectNames.push_back(variableLowerName);
       
   if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), 
       variableUpperName) == wrapperObjectNames.end())
      wrapperObjectNames.push_back(variableUpperName);
       
   if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), 
       variableMaximumStepName) == wrapperObjectNames.end())
      wrapperObjectNames.push_back(variableMaximumStepName);
       
   if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), 
       additiveScaleFactorName) == wrapperObjectNames.end())
      wrapperObjectNames.push_back(additiveScaleFactorName);
       
   if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), 
       multiplicativeScaleFactorName) == wrapperObjectNames.end())
      wrapperObjectNames.push_back(multiplicativeScaleFactorName);
   
   return wrapperObjectNames;
}


//------------------------------------------------------------------------------
// bool SetElementWrapper(ElementWrapper *toWrapper, const wxString &withName)
//------------------------------------------------------------------------------
bool Vary::SetElementWrapper(ElementWrapper *toWrapper, const wxString &withName)
{
   #ifdef DEBUG_WRAPPER_CODE
   MessageInterface::ShowMessage
      (wxT("Vary::SetElementWrapper() <%p> entered, toWrapper=<%p>'%s', withName='%s'\n"),
       this, toWrapper, toWrapper ? toWrapper->GetDescription().c_str() : wxT("NULL"),
       withName.c_str());
   #endif
   
   bool retval = false;
   
   if (toWrapper == NULL) return false;
   
   if (toWrapper->GetWrapperType() == Gmat::ARRAY_WT)
   {
      throw CommandException(wxT("A value of type \"Array\" on command \"") + typeName + 
                  wxT("\" is not an allowed value.\nThe allowed values are:")
                  wxT(" [ Real Number, Variable, Array Element, or Parameter ]. ")); 
   }
   CheckDataType(toWrapper, Gmat::REAL_TYPE, wxT("Vary"), true);
   
   
   #ifdef DEBUG_WRAPPER_CODE   
   MessageInterface::ShowMessage(wxT("   Setting wrapper \"%s\" on Vary command\n"), 
      withName.c_str());
   #endif
   
   if (variableName == withName)
   {
      if (variable != NULL &&
          !IsThereSameWrapperName(VARIABLE_NAME, withName))
         CollectOldWrappers(&variable);
      
      variable = toWrapper;
      retval = true;
   }
   
   if (initialValueName == withName)
   {
      if (initialValue != NULL &&
          !IsThereSameWrapperName(INITIAL_VALUE, withName))
         CollectOldWrappers(&initialValue);
      
      initialValue = toWrapper;
      retval = true;
   }
   
   if (perturbationName == withName)
   {
      if (perturbation != NULL&&
          !IsThereSameWrapperName(PERTURBATION, withName))
         CollectOldWrappers(&perturbation);
      
      perturbation = toWrapper;
      retval = true;
   }
   
   if (variableLowerName == withName)
   {
      if (variableLower != NULL &&
          !IsThereSameWrapperName(VARIABLE_LOWER, withName))
         CollectOldWrappers(&variableLower);
      
      variableLower = toWrapper;
      retval = true;
   }
   
   if (variableUpperName == withName)
   {
      if (variableUpper != NULL &&
          !IsThereSameWrapperName(VARIABLE_UPPER, withName))
         CollectOldWrappers(&variableUpper);
      
      variableUpper = toWrapper;
      retval = true;
   }
   
   if (variableMaximumStepName == withName) 
   {
      if (variableMaximumStep != NULL &&
          !IsThereSameWrapperName(VARIABLE_MAXIMUM_STEP, withName))
         CollectOldWrappers(&variableMaximumStep);
      
      variableMaximumStep = toWrapper;
      retval = true;
   }
   
   if (additiveScaleFactorName == withName)
   {
      if (additiveScaleFactor != NULL &&
          !IsThereSameWrapperName(ADDITIVE_SCALE_FACTOR, withName))
         CollectOldWrappers(&additiveScaleFactor);
      
      additiveScaleFactor = toWrapper;
      retval = true;
   }
   
   if (multiplicativeScaleFactorName == withName)
   {
      if (multiplicativeScaleFactor != NULL &&
          !IsThereSameWrapperName(MULTIPLICATIVE_SCALE_FACTOR, withName))
         CollectOldWrappers(&multiplicativeScaleFactor);
      
      multiplicativeScaleFactor = toWrapper;
      retval = true;
   }
   
   //@todo In nested function, there are still bad wrapper pointers so until it is resolved
   // delete if not in function. (tested with Func_BasicOptimizerTest.script,
   // Func_Pic_Achieve.script)
   //@todo Func_BasicOptimizerTest.script still shows 6 memory leak trace (LOJ: 2009.11.03)
   //+++ <026116C0> F Validator::CreateOtherWrapper()  ew = new VariableWrapper() 
   //+++ <026733F8> G Validator::CreateOtherWrapper()  ew = new VariableWrapper() 
   //+++ <02669DF8> 8 Validator::CreateElementWrapper()  ew = new NumberWrapper() 
   //+++ <026116C0> F Validator::CreateOtherWrapper()  ew = new VariableWrapper() 
   //+++ <026733F8> G Validator::CreateOtherWrapper()  ew = new VariableWrapper() 
   //+++ <02669DF8> 8 Validator::CreateElementWrapper()  ew = new NumberWrapper() 
   
   //Commented out since old wrappers are deleted in ClearWrappers() which is called
   // from the Validator::ValidateCommand() (LOJ: 2009.11.03)
   //if (currentFunction == NULL)
   //   DeleteOldWrappers();
   
   #ifdef DEBUG_WRAPPER_CODE
   MessageInterface::ShowMessage
      (wxT("Vary::SetElementWrapper() <%p> returning %d\n"), this, retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// void ClearWrappers()
//------------------------------------------------------------------------------
void Vary::ClearWrappers()
{
   #ifdef DEBUG_WRAPPER_CODE
   MessageInterface::ShowMessage
      (wxT("Vary::ClearWrappers() entered, has %d old wrappers\n"), oldWrappers.size());
   #endif
   
   // For element wrapper cleanup
   ClearOldWrappers();
   CollectOldWrappers(&variable);
   CollectOldWrappers(&initialValue);
   CollectOldWrappers(&perturbation);
   CollectOldWrappers(&variableLower);
   CollectOldWrappers(&variableUpper);
   CollectOldWrappers(&variableMaximumStep);
   CollectOldWrappers(&additiveScaleFactor);
   CollectOldWrappers(&multiplicativeScaleFactor);
   DeleteOldWrappers();
}


//------------------------------------------------------------------------------
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * Performs the initialization needed to run the Varyer.
 *
 * @return true if the Command is initialized, false if an error occurs.
 */
//------------------------------------------------------------------------------
bool Vary::Initialize()
{
   bool retval = GmatCommand::Initialize();

   if (solver == NULL)
      throw CommandException(wxT("solver not initialized for Vary command\n  \"")
                             + generatingString + wxT("\"\n"));

   Integer id = solver->GetParameterID(wxT("Variables"));
   solver->SetStringParameter(id, variableName);
        
   // The solver cannot be finalized until all of the loop is initialized
   solverDataFinalized = false;

   // Set references for the wrappers   
   #ifdef DEBUG_VARY_PARAMS
      MessageInterface::ShowMessage(wxT("Setting refs for variable\n"));
   #endif
   if (SetWrapperReferences(*variable) == false)
      return false;
   CheckDataType(variable, Gmat::REAL_TYPE, wxT("Vary"));
   #ifdef DEBUG_VARY_PARAMS
      MessageInterface::ShowMessage(wxT("Setting refs for initial value\n"));
   #endif
   if (SetWrapperReferences(*initialValue) == false)
      return false;
   CheckDataType(initialValue, Gmat::REAL_TYPE, wxT("Vary"));
   #ifdef DEBUG_VARY_PARAMS
      MessageInterface::ShowMessage(wxT("Setting refs for perturbation\n"));
   #endif
   if (SetWrapperReferences(*perturbation) == false)
      return false;
   CheckDataType(perturbation, Gmat::REAL_TYPE, wxT("Vary"));
   #ifdef DEBUG_VARY_PARAMS
      MessageInterface::ShowMessage(wxT("Setting refs for minimum\n"));
   #endif
   if (SetWrapperReferences(*variableLower) == false)
      return false;
   CheckDataType(variableLower, Gmat::REAL_TYPE, wxT("Vary"));
   #ifdef DEBUG_VARY_PARAMS
      MessageInterface::ShowMessage(wxT("Setting refs for maximum\n"));
   #endif
   if (SetWrapperReferences(*variableUpper) == false)
      return false;
   CheckDataType(variableUpper, Gmat::REAL_TYPE, wxT("Vary"));
   #ifdef DEBUG_VARY_PARAMS
      MessageInterface::ShowMessage(wxT("Setting refs for max step\n"));
   #endif
   if (SetWrapperReferences(*variableMaximumStep) == false)
      return false;
   CheckDataType(variableMaximumStep, Gmat::REAL_TYPE, wxT("Vary"));
   #ifdef DEBUG_VARY_PARAMS
      MessageInterface::ShowMessage(wxT("Setting refs for additive scale factor\n"));
   #endif
   if (SetWrapperReferences(*additiveScaleFactor) == false)
      return false;
   CheckDataType(additiveScaleFactor, Gmat::REAL_TYPE, wxT("Vary"));
   #ifdef DEBUG_VARY_PARAMS
      MessageInterface::ShowMessage(wxT("Setting refs for mult. scale factor\n"));
   #endif
   if (SetWrapperReferences(*multiplicativeScaleFactor) == false)
      return false;
   CheckDataType(multiplicativeScaleFactor, Gmat::REAL_TYPE, wxT("Vary"));

   #ifdef DEBUG_VARY_PARAMS
      MessageInterface::ShowMessage(wxT("Vary command Initialization complete\n"));
   #endif

   return retval;
}


//------------------------------------------------------------------------------
//  bool Execute()
//------------------------------------------------------------------------------
/**
 * Vary the variables defined for this Varying loop.
 *
 * This method (will eventually) runs the Varyer state machine in order to
 * determine the variable values needed to achieve the user specified 
 * goals.
 *
 * @return true if the Command runs to completion, false if an error
 *         occurs.
 */
//------------------------------------------------------------------------------
bool Vary::Execute()
{
   bool retval = true;

   #ifdef DEBUG_VARY_EXECUTE
      MessageInterface::ShowMessage
         (wxT("Vary::Execute() solverDataFinalized=%d\n"), solverDataFinalized);
   #endif
    
   if (!solverDataFinalized) 
   {
      #ifdef DEBUG_VARY_EXECUTE
         MessageInterface::ShowMessage
            (wxT("Vary::Execute() - running code for the first time through <<<\n"));
      #endif
      // First time through, tell the solver about the variables
      Real varData[6], asf, msf;
      asf = additiveScaleFactor->EvaluateReal();
      msf = 1.0 / multiplicativeScaleFactor->EvaluateReal();

      varData[5] = initialValue->EvaluateReal();                // Initial value

      // scale by using Eq. 13.5 of Architecture document
      varData[0] = (varData[5] + asf) / msf;
      varData[1] = (perturbation->EvaluateReal()) / msf;         // pert
      varData[2] = (variableLower->EvaluateReal() + asf) / msf;  // minimum
      varData[3] = (variableUpper->EvaluateReal() + asf) / msf;  // maximum
      varData[4] = (variableMaximumStep->EvaluateReal()) / msf;  // largest step
      
      #ifdef DEBUG_VARY_EXECUTE
         MessageInterface::ShowMessage(
            wxT("For variable \"%s\", data is [%15.9lf %15.9lf %15.9lf %15.9lf ")
            wxT("%15.9lf]\n"), variableName.c_str(), varData[0], varData[1], varData[2], 
            varData[3], varData[4]);
      #endif      
   
      variableID = solver->SetSolverVariables(varData, variableName);

      #ifdef DEBUG_VARY_EXECUTE
         MessageInterface::ShowMessage(wxT("Solver variables were set\n"));
      #endif

      solverDataFinalized = true;
      BuildCommandSummary(true);
      #ifdef DEBUG_VARY_EXECUTE
         MessageInterface::ShowMessage
            (wxT("Vary::Execute - exiting with retval = %s\n"),
            (retval? wxT("true") : wxT("False")));
      #endif
      return retval;
   }
   
   Real var = solver->GetSolverVariable(variableID);
   // scale using Eq. 13.6 of Architecture document
   var = var / multiplicativeScaleFactor->EvaluateReal() -
      additiveScaleFactor->EvaluateReal();
   
   #ifdef DEBUG_VARIABLE_RANGES
      MessageInterface::ShowMessage(
         wxT("Setting %s to %.12le; allowed range is [%.12le, %.12le]\n"),
         variableName.c_str(), var, variableLower->EvaluateReal(), 
         variableUpper->EvaluateReal());
   #endif

   variable->SetReal(var);
   solver->SetUnscaledVariable(variableID, var);
   
   BuildCommandSummary(true);
   #ifdef DEBUG_VARY_EXECUTE
      MessageInterface::ShowMessage
         (wxT("Vary::Execute - exiting with retval = %s\n"),
          (retval? wxT("true") : wxT("False")));
   #endif
   return retval;
}


//------------------------------------------------------------------------------
// void RunComplete()
//------------------------------------------------------------------------------
void Vary::RunComplete()
{
   #ifdef DEBUG_VARY_EXECUTE
      MessageInterface::ShowMessage(
      wxT("In Vary::RunComplete, solverDataFinalized = %s, ... now setting it to false\n"),
      (solverDataFinalized? wxT("true") : wxT("false")));
   #endif
   solverDataFinalized = false;
   GmatCommand::RunComplete();
}


//------------------------------------------------------------------------------
// bool TakeAction(const wxString &action, const wxString &actionData)
//------------------------------------------------------------------------------
/**
 * Performs an action specific to Vary commands
 *
 * @param action Text trigger for the requested action
 * @param actionData Additional data needed to perform the action (defaults to
 *                   the empty string)
 *
 * @return true if an action was performed, false if not
 */
//------------------------------------------------------------------------------
bool Vary::TakeAction(const wxString &action, const wxString &actionData)
{
   if (action == wxT("SolverReset"))
   {
      // Prep to refresh the solver data if called again
      RefreshData();
      return true;
   }

   return GmatCommand::TakeAction(action, actionData);
}


//------------------------------------------------------------------------------
// void SetInitialValue(Solver *theSolver)
//------------------------------------------------------------------------------
/**
 * Updates initial variable values from a solver control sequence.
 * 
 * This method is used to change the initial value of the variable -- for 
 * example, once a targeter has found a solution, a call to this method sets the 
 * initial value of the variable to match the solved-for value.
 * 
 * @param theSolver The Solver class that requested the variable updates.  
 *                  Updates are only applied if this Vary command uses the same
 *                  Solver.
 */
//------------------------------------------------------------------------------
void Vary::SetInitialValue(Solver *theSolver)
{
   MessageInterface::ShowMessage(wxT("Setting initial value\n"));
   if (solver == theSolver)
   {
      Real var = solver->GetSolverVariable(variableID);
      initialValue->SetReal(var);
      MessageInterface::ShowMessage(
            wxT("   Solvers matched; solution value %.12lf reset IV for %s to %.12lf\n"),
            var, initialValueName.c_str(), initialValue->EvaluateReal());
      
      if (initialValue->GetWrapperType() == Gmat::NUMBER_WT)
      {
         wxString numString;
         numString << var;
         initialValueName = numString;
      }
   }
}


//------------------------------------------------------------------------------
// bool IsThereSameWrapperName(const wxString &wrapperName)
//------------------------------------------------------------------------------
/*
 * Checks if there is the same wrapper name. Wrapper name can be a number.
 *
 * @return  true if the same wrapper name found, false otherwise
 */
//------------------------------------------------------------------------------
bool Vary::IsThereSameWrapperName(int param, const wxString &wrapperName)
{
   #ifdef DEBUG_WRAPPER_CODE
   MessageInterface::ShowMessage
      (wxT("GmatCommand::IsThereSameWrapperName() <%p>'%s' entered, wrapperName='%s'\n"),
       this, GetTypeName().c_str(), wrapperName.c_str());
   #endif
   
   bool retval = false;
   
   // Check for the same wrapper is used for other properties
   for (int i = VARIABLE_NAME; i < VaryParamCount; i++)
   {
      if (i == param)
         continue;

      if (GetStringParameter(i) == wrapperName)
      {
         retval = true;
         break;
      }
   }
   
   #ifdef DEBUG_WRAPPER_CODE
   MessageInterface::ShowMessage
      (wxT("GmatCommand::IsThereSameWrapperName() <%p>'%s' wrapperName='%s' returning %d\n"),
       this, GetTypeName().c_str(), wrapperName.c_str(), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// void RefreshData()
//------------------------------------------------------------------------------
/**
 * Updates the variable data with the current values from the Mission Control
 * Sequence.
 */
//------------------------------------------------------------------------------
void Vary::RefreshData()
{
   Real varData[5], asf, msf;
   asf = additiveScaleFactor->EvaluateReal();
   msf = 1.0 / multiplicativeScaleFactor->EvaluateReal();

   varData[0] = initialValue->EvaluateReal();                // Initial value

   // scale by using Eq. 13.5 of Architecture document
   varData[0] = (varData[0] + asf) / msf;
   varData[1] = (perturbation->EvaluateReal()) / msf;         // pert
   varData[2] = (variableLower->EvaluateReal() + asf) / msf;  // minimum
   varData[3] = (variableUpper->EvaluateReal() + asf) / msf;  // maximum
   varData[4] = (variableMaximumStep->EvaluateReal()) / msf;  // largest step

   #ifdef DEBUG_VARY_EXECUTE
      MessageInterface::ShowMessage(
         wxT("For variable \"%s\", data is [%15.9lf %15.9lf %15.9lf %15.9lf ")
         wxT("%15.9lf]\n"), variableName.c_str(), varData[0], varData[1], varData[2],
         varData[3], varData[4]);
   #endif

   solver->RefreshSolverVariables(varData, variableName);
}
