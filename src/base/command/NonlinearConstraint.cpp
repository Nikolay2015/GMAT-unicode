//$Id$
//------------------------------------------------------------------------------
//                                 NonlinearConstraint
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
// Author: Wendy Shoan (GSFC/MAB)
// Created: 2006.08.14
//
/**
 * Definition for the NonlinearConstraint command class
 */
//------------------------------------------------------------------------------

#include "NonlinearConstraint.hpp"
#include "StringUtil.hpp"  // for ToReal()
#include "MessageInterface.hpp"

//#define DEBUG_NONLINEAR_CONSTRAINT_PARSE 1
//#define DEBUG_NONLINEAR_CONSTRAINT_INIT 1
//#define DEBUG_NONLINEAR_CONSTRAINT_EXEC 1
//#define DEBUG_NLC_VALUES
//#define DEBUG_NONLINEAR_CONSTRAINT_PARAM
//#define DEBUG_NLC_PARSING
//#define DEBUG_NLC_WRAPPER_CODE


//---------------------------------
// static data
//---------------------------------
const wxString
   NonlinearConstraint::PARAMETER_TEXT[NonlinearConstraintParamCount - GmatCommandParamCount] =
   {
      wxT("OptimizerName"),
      wxT("ConstraintArg1"),
      wxT("Operator"),
      wxT("ConstraintArg2"),
      wxT("Tolerance"),
   };
   
const Gmat::ParameterType
   NonlinearConstraint::PARAMETER_TYPE[NonlinearConstraintParamCount - GmatCommandParamCount] =
   {
      Gmat::STRING_TYPE,
      Gmat::STRING_TYPE,
      Gmat::STRING_TYPE,
      Gmat::STRING_TYPE,
      Gmat::REAL_TYPE,
   };

const wxString // order must match enum Operator 
   NonlinearConstraint::OP_STRINGS[3] =
   {
      wxT("<="),
      wxT(">="),
      wxT("="),
   };


//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  NonlinearConstraint()
//------------------------------------------------------------------------------
/**
 * Creates an NonlinearConstraint command.  (default constructor)
 */
//------------------------------------------------------------------------------
NonlinearConstraint::NonlinearConstraint() :
   GmatCommand             (wxT("NonlinearConstraint")),
   optimizerName           (wxT("")),
   optimizer               (NULL),
   arg1Name                (wxT("")),
   arg1                    (NULL),
   constraintValue         (-999.99),
   arg2Name                (wxT("")),
   arg2                    (NULL),
   isInequality            (false),
   isIneqString            (wxT("EqConstraint")),
   desiredValue            (0.0),
   op                      (EQUAL),
   tolerance               (0.0),
   optimizerDataFinalized  (false),
   constraintId            (-1),
   interpreted             (false)
{
   parameterCount = NonlinearConstraintParamCount;
   // nothing to add to settables here ... yet
   // settables.push_back("Tolerance");
}


//------------------------------------------------------------------------------
//  NonlinearConstraint(const NonlinearConstraint& nlc)
//------------------------------------------------------------------------------
/**
 * Constructor that replicates a NonlinearConstraint command.  (Copy constructor)
 *
 * @param t Command that is replicated here.
 */
//------------------------------------------------------------------------------
NonlinearConstraint::NonlinearConstraint(const NonlinearConstraint& nlc) :
   GmatCommand             (nlc),
   optimizerName           (nlc.optimizerName),
   optimizer               (NULL),
   arg1Name                (nlc.arg1Name),
   arg1                    (NULL),
   constraintValue         (nlc.constraintValue),
   arg2Name                (nlc.arg2Name),
   arg2                    (NULL),
   isInequality            (nlc.isInequality),
   isIneqString            (nlc.isIneqString),
   desiredValue            (nlc.desiredValue),
   op                      (nlc.op),
   tolerance               (nlc.tolerance),
   optimizerDataFinalized  (false),
   constraintId            (-1),
   interpreted             (false)
{
   parameterCount = NonlinearConstraintParamCount;
}


//------------------------------------------------------------------------------
//  NonlinearConstraint& operator=(const NonlinearConstraint& nlc)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the NonlinearConstraint command.
 *
 * @param t Command that is replicated here.
 *
 * @return A reference to this instance.
 */
//------------------------------------------------------------------------------
NonlinearConstraint& NonlinearConstraint::operator=(const NonlinearConstraint& nlc)
{
   if (this == &nlc)
      return *this;
    
   GmatCommand::operator=(nlc);
   optimizerName          = nlc.optimizerName;
   optimizer              = NULL;
   arg1Name               = nlc.arg1Name;
   arg1                   = NULL;
   constraintValue        = nlc.constraintValue;
   arg2Name               = nlc.arg2Name;
   arg2                   = NULL;
   isInequality           = nlc.isInequality;
   isIneqString           = nlc.isIneqString;
   desiredValue           = nlc.desiredValue;
   op                     = nlc.op;
   tolerance              = nlc.tolerance;
   optimizerDataFinalized = false;
   constraintId           = -1;
   interpreted            = false;

   return *this;
}

//------------------------------------------------------------------------------
//  ~NonlinearConstraint()
//------------------------------------------------------------------------------
/**
 * Destroys the NonlinearConstraint command.  (destructor)
 */
//------------------------------------------------------------------------------
NonlinearConstraint::~NonlinearConstraint()
{
   ClearWrappers();
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the NonlinearConstraint.
 *
 * @return clone of the NonlinearConstraint.
 */
//------------------------------------------------------------------------------
GmatBase* NonlinearConstraint::Clone() const
{
   return (new NonlinearConstraint(*this));
}


//------------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//------------------------------------------------------------------------------
/**
 * Renames referenced objects.
 *
 * @param type Type of the object that is renamed.
 * @param oldName The current name for the object.
 * @param newName The name the object has when this operation is complete.
 *
 * @return true on success.
 */
//------------------------------------------------------------------------------
bool NonlinearConstraint::RenameRefObject(const Gmat::ObjectType type,
                               const wxString &oldName,
                               const wxString &newName)
{
   if (type == Gmat::SOLVER)
   {
      if (optimizerName == oldName)
         optimizerName = newName;
   }
   if (arg1) 
   {
      arg1->RenameObject(oldName, newName);
      arg1Name  = arg1->GetDescription();
   }
   if (arg2) 
   {
      arg2->RenameObject(oldName, newName);
      arg2Name  = arg2->GetDescription();
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
const ObjectTypeArray& NonlinearConstraint::GetRefObjectTypeArray()
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
const StringArray& NonlinearConstraint::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   refObjectNames.clear();
   
   if (type == Gmat::UNKNOWN_OBJECT ||
       type == Gmat::SOLVER)
   {
      refObjectNames.push_back(optimizerName);
   }
 
   return refObjectNames;
}


// Parameter accessors

//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter text, given the input parameter ID.
 *
 * @param id Id for the requested parameter text.
 *
 * @return parameter text for the requested parameter.
 */
//------------------------------------------------------------------------------
wxString NonlinearConstraint::GetParameterText(const Integer id) const
{
   if (id >= GmatCommandParamCount && id < NonlinearConstraintParamCount)
      return PARAMETER_TEXT[id - GmatCommandParamCount];
   return GmatCommand::GetParameterText(id);
}


//------------------------------------------------------------------------------
//  Integer  GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter ID, given the input parameter string.
 *
 * @param str string for the requested parameter.
 *
 * @return ID for the requested parameter.
 */
//------------------------------------------------------------------------------
Integer NonlinearConstraint::GetParameterID(const wxString &str) const
{
   for (Integer i = GmatCommandParamCount; i < NonlinearConstraintParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatCommandParamCount])
         return i;
   }

   return GmatCommand::GetParameterID(str);
}


//------------------------------------------------------------------------------
//  Gmat::ParameterType  GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type, given the input parameter ID.
 *
 * @param id ID for the requested parameter.
 *
 * @return parameter type of the requested parameter.
 */
//------------------------------------------------------------------------------
Gmat::ParameterType NonlinearConstraint::GetParameterType(const Integer id) const
{
   if (id >= GmatCommandParamCount && id < NonlinearConstraintParamCount)
      return PARAMETER_TYPE[id - GmatCommandParamCount];

   return GmatCommand::GetParameterType(id);
}


//------------------------------------------------------------------------------
//  wxString  GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type string, given the input parameter ID.
 *
 * @param id ID for the requested parameter.
 *
 * @return parameter type string of the requested parameter.
 */
//------------------------------------------------------------------------------
wxString NonlinearConstraint::GetParameterTypeString(const Integer id) const
{
   return GmatCommand::PARAM_TYPE_STRING[GetParameterType(id)];
}


//------------------------------------------------------------------------------
//  Real  GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the Real parameter value, given the input parameter ID.
 *
 * @param id ID for the requested parameter value.
 *
 * @return Real value of the requested parameter.
 */
//------------------------------------------------------------------------------
Real NonlinearConstraint::GetRealParameter(const Integer id) const
{
   if (id == TOLERANCE)
      return tolerance;

   if (id == CONSTRAINT_ARG1)
      if (arg1)
         return arg1->EvaluateReal();
    
   if (id == CONSTRAINT_ARG2)
      if (arg2)
         return arg2->EvaluateReal();
    
   return GmatCommand::GetRealParameter(id);
}


//------------------------------------------------------------------------------
//  Real  SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * This method sets the Real parameter value, given the input parameter ID.
 *h
 * @param id    ID for the parameter whose value to change.
 * @param value value for the parameter.
 *
 * @return Real value of the requested parameter.
 */
//------------------------------------------------------------------------------
Real NonlinearConstraint::SetRealParameter(const Integer id, const Real value)
{
   if (id == TOLERANCE)
   {
      tolerance = value;
      return tolerance;
   }
   
   return GmatCommand::SetRealParameter(id, value);
}


//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter ID.
 *
 * @param id ID for the requested parameter.
 *
 * @return  string value of the requested parameter.
 */
//------------------------------------------------------------------------------
wxString NonlinearConstraint::GetStringParameter(const Integer id) const
{
   if (id == OPTIMIZER_NAME)
      return optimizerName;
        
   if (id == CONSTRAINT_ARG1)
      return arg1Name;
      
   if (id == CONSTRAINT_ARG2)
      return arg2Name;
      
   if (id == OPERATOR)
   {
      return OP_STRINGS[(Integer)op];
   }
        
   return GmatCommand::GetStringParameter(id);
}


//------------------------------------------------------------------------------
//  wxString  SetStringParameter(const Integer id, const wxString value)
//------------------------------------------------------------------------------
/**
 * This method sets the string parameter value, given the input
 * parameter ID.
 *
 * @param id    ID for the requested parameter.
 * @param value string value for the requested parameter.
 *
 * @return  success flag.
 */
//------------------------------------------------------------------------------
bool NonlinearConstraint::SetStringParameter(const Integer id, 
                                             const wxString &value)
{
   #ifdef DEBUG_NONLINEAR_CONSTRAINT_PARAM
   MessageInterface::ShowMessage
      (wxT("NonlinearConstraint::SetStringParameter() id=%d, value=%s\n"), 
         id, value.c_str());
   #endif
   
   if (id == OPTIMIZER_NAME) 
   {
      optimizerName = value;
      //interpreted   = false;
      return true;
   }

   if (id == CONSTRAINT_ARG1) 
   {
      arg1Name       = value;
      if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), value) == 
          wrapperObjectNames.end())
         wrapperObjectNames.push_back(value);
      //interpreted    = false;
      return true;
   }
   
   if (id == CONSTRAINT_ARG2) 
   {
      arg2Name       = value;
      if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), value) == 
          wrapperObjectNames.end())
         wrapperObjectNames.push_back(value);
      //interpreted    = false;
      return true;
   }
   
   if (id == OPERATOR)
   {
      if (value == wxT("<="))      op = LESS_THAN_OR_EQUAL;
      else if (value == wxT(">=")) op = GREATER_THAN_OR_EQUAL;
      else if (value == wxT("="))  op = EQUAL;
      else
      {
         wxString errmsg = wxT("The conditional operator \"") + value;
         errmsg            += wxT("\" is not allowed in a NonlinearConstraint command.\n");
         errmsg            += wxT("The allowed values are [=, <=, >=]\n");
         throw CommandException(errmsg);
      }
      //interpreted   = false;
      return true;
   }

   return GmatCommand::SetStringParameter(id, value);
}

//------------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                                   const wxString &name = "")
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
bool NonlinearConstraint::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                            const wxString &name)
{   
   if (type == Gmat::SOLVER) 
   {
      if (optimizerName == obj->GetName()) 
      {
         optimizer = (Solver*)obj;
         return true;
      }
      return false;
   }
   return GmatCommand::SetRefObject(obj, type, name);
}


//------------------------------------------------------------------------------
//  bool InterpretAction()
//------------------------------------------------------------------------------
/**
 * Parses the command string and builds the corresponding command structures.
 *
 * The NonlinearConstraint command has the following syntax:
 *
 *     NonlinearConstraint myOpt(Sat1.SMA = 21545.0, {Tolerance = 0.1});
 *     NonlinearConstraint myOpt(Sat1.SMA = Var1, {Tolerance = 0.1});
 *     NonlinearConstraint myOpt(Sat1.SMA = Arr1(1,1), {Tolerance = 0.1});
 *     NonlinearConstraint myOpt(Sat1.SMA = Arr1(I,J), {Tolerance = 0.1});
 *
 * where myOpt is a Solver used to optimize a set of variables to the
 * corresponding constraints.  This method breaks the script line into the 
 * corresponding pieces, and stores the name of the Solver so it can be set to
 * point to the correct object during initialization.
 */
//------------------------------------------------------------------------------
bool NonlinearConstraint::InterpretAction()
{
   bool tooMuch = false;
   // Clean out any old data
   wrapperObjectNames.clear();
   ClearWrappers();

   StringArray chunks = InterpretPreface();

   #ifdef DEBUG_NLC_PARSING
      MessageInterface::ShowMessage(wxT("Preface chunks as\n"));
      for (StringArray::iterator i = chunks.begin(); i != chunks.end(); ++i)
         MessageInterface::ShowMessage(wxT("   \"%s\"\n"), i->c_str());
      MessageInterface::ShowMessage(wxT("\n"));
   #endif
   
   // Find and set the solver object name
   // This is the only setting in Vary that is not in a wrapper
   StringArray currentChunks = parser.Decompose(chunks[1], wxT("()"), false);
   if (!GmatStringUtil::HasNoBrackets(currentChunks.at(0), false))
      throw CommandException(
            wxT("Solver name for NonlinearConstraint command may not contain brackets, braces, or parentheses.")); 
   SetStringParameter(OPTIMIZER_NAME, currentChunks[0]);
   if (currentChunks.size() < 2)
      throw CommandException(wxT("Missing field or value for NonlinearConstraint command."));
   if (currentChunks.size() > 2) tooMuch = true;
   wxString cc = GmatStringUtil::Strip(currentChunks[1]);
   Integer ccEnd = cc.size() - 1;
   if ((tooMuch) || (cc.at(0) != wxT('(')) || (cc.at(ccEnd) != wxT(')')))
      throw CommandException(
           wxT("Missing parentheses, or unexpected characters found, around logical expression argument to NonlinearConstraint command."));
   if (!GmatStringUtil::IsBracketBalanced(cc, wxT("()")))
      throw CommandException(wxT("Parentheses unbalanced in NonlinearConstraint command."));
   // @todo - if tolerance allowed later, will need to not check for braces here ...
   if ((cc.find(wxT('[')) != cc.npos) || (cc.find(wxT(']')) != cc.npos) ||
       (cc.find(wxT('{')) != cc.npos) || (cc.find(wxT('}')) != cc.npos) )
      throw CommandException(wxT("NonlinearConstraint command may not contain brackets or braces."));
   
   wxString noSpaces2     = GmatStringUtil::RemoveAll(cc,wxT(' '));
   //wxString noSpaces2     = GmatStringUtil::RemoveAll(currentChunks[1],' ');
   currentChunks = parser.SeparateBrackets(noSpaces2, wxT("()"), wxT(","), false);
   
   #ifdef DEBUG_NLC_PARSING
      MessageInterface::ShowMessage(
         wxT("NLC:  noSpaces2 = %s\n"), noSpaces2.c_str());
      MessageInterface::ShowMessage(
         wxT("NLC: after SeparateBrackets, current chunks = \n"));
      for (Integer jj = 0; jj < (Integer) currentChunks.size(); jj++)
         MessageInterface::ShowMessage(wxT("   %s\n"),
                                       currentChunks[jj].c_str());
   #endif
   bool testForMore = false;
   if ((Integer) currentChunks.size() > 1) testForMore = true;
   Integer end;
   wxString constraintStr = currentChunks[0];
   Integer opSize = 1;
   if ((end = constraintStr.find(wxT(">="), 0)) != (Integer) constraintStr.npos)
   {
      op = GREATER_THAN_OR_EQUAL;
      isInequality = true;
      isIneqString = wxT("IneqConstraint");
      opSize       = 2;
      #ifdef DEBUG_NONLINEAR_CONSTRAINT_INIT
         MessageInterface::ShowMessage(wxT("NLC:InterpretAction: greater_than_or_equal\n"));
      #endif
   }
   else if ((end = constraintStr.find(wxT("<="), 0)) != (Integer) constraintStr.npos)
   {
      op = LESS_THAN_OR_EQUAL;
      isInequality = true;
      isIneqString = wxT("IneqConstraint");
      opSize       = 2;
      #ifdef DEBUG_NONLINEAR_CONSTRAINT_INIT
         MessageInterface::ShowMessage(wxT("NLC:InterpretAction: less_than_or_equal\n"));
      #endif
   }
   else if ((end = constraintStr.find(wxT("=>"), 0)) != (Integer) constraintStr.npos)
   {
      wxString errmsg = wxT("The string \"=>\" is an invalid conditonal operator");
      errmsg            += wxT(" in a NonlinearConstraint command.\n");
      errmsg            += wxT("The allowed values are [=, <=, >=]\n");
      throw CommandException(errmsg);
   }
   else if ((end = constraintStr.find(wxT("=<"), 0)) != (Integer) constraintStr.npos)
   {
      wxString errmsg = wxT("The string \"=<\" is an invalid conditonal operator");
      errmsg            += wxT(" in a NonlinearConstraint command.\n");
      errmsg            += wxT("The allowed values are [=, <=, >=]\n");
      throw CommandException(errmsg);
   }
   else if ((end = constraintStr.find(wxT("=="), 0)) != (Integer) constraintStr.npos)
   {
      wxString errmsg = wxT("The string \"==\" is an invalid conditonal operator");
      errmsg            += wxT(" in a NonlinearConstraint command.\n");
      errmsg            += wxT("The allowed values are [=, <=, >=]\n");
      throw CommandException(errmsg);
   }
   else if ((end = constraintStr.find(wxT("="), 0)) != (Integer) constraintStr.npos)
   {
      //end = constraintStr.find("=", 0);
      op = EQUAL;
      isInequality = false;
      isIneqString = wxT("EqConstraint");
      #ifdef DEBUG_NONLINEAR_CONSTRAINT_INIT
         MessageInterface::ShowMessage(wxT("NLC:InterpretAction: equal\n"));
      #endif
   }
   else
   {
      wxString errmsg = wxT("The conditional operator is missing or invalid ");
      errmsg            += wxT(" in a NonlinearConstraint command.\n");
      errmsg            += wxT("The allowed values are [=, <=, >=]\n");
      throw CommandException(errmsg);
   }
   arg1Name = constraintStr.substr(0,end);
   arg2Name = constraintStr.substr(end+opSize, (constraintStr.npos - end + opSize - 1));
   #ifdef DEBUG_NLC_PARSING
      MessageInterface::ShowMessage(wxT("... arg1Name = %s\n"), arg1Name.c_str());
      MessageInterface::ShowMessage(wxT("... operator = %s\n"), OP_STRINGS[(Integer)op].c_str());
      MessageInterface::ShowMessage(wxT("... arg2Name = %s\n"), arg2Name.c_str());
   #endif
   // Currently, this should not happen ..... 
   if (testForMore)
   {
      wxString noSpaces     = GmatStringUtil::RemoveAll(currentChunks[1],wxT(' '));
      currentChunks = parser.SeparateBrackets(noSpaces, wxT("{}"), wxT(","), false);
      
      #ifdef DEBUG_NLC_PARSING
         MessageInterface::ShowMessage(
            wxT("NLC: After SeparateBrackets, current chunks = \n"));
         for (Integer jj = 0; jj < (Integer) currentChunks.size(); jj++)
            MessageInterface::ShowMessage(wxT("   %s\n"),
                                          currentChunks[jj].c_str());
      #endif
      
      // currentChunks now holds all of the pieces - no need for more separation  
      
      wxString lhs, rhs;
      for (StringArray::iterator i = currentChunks.begin(); 
           i != currentChunks.end(); ++i)
      {
         SeparateEquals(*i, lhs, rhs);
         
         if (IsSettable(lhs))
            SetStringParameter(GetParameterID(lhs), rhs);
         else
         {
            throw CommandException(wxT("The setting \"") + lhs + 
               wxT("\" is not a valid setting for a ") + typeName + 
               wxT(" command.\n"));
         }
      }
   }
   
   interpreted = true;
  
   return true;
}


//------------------------------------------------------------------------------
// const StringArray& GetWrapperObjectNameArray()
//------------------------------------------------------------------------------
const StringArray& NonlinearConstraint::GetWrapperObjectNameArray()
{
   wrapperObjectNames.clear();

   wrapperObjectNames.push_back(arg1Name);
   
   if (find(wrapperObjectNames.begin(), wrapperObjectNames.end(), 
       arg2Name) == wrapperObjectNames.end())
      wrapperObjectNames.push_back(arg2Name);

   return wrapperObjectNames;
}


//------------------------------------------------------------------------------
// bool SetElementWrapper(ElementWrapper *toWrapper, const wxString &withName)
//------------------------------------------------------------------------------
bool NonlinearConstraint::SetElementWrapper(ElementWrapper *toWrapper,
                                            const wxString &withName)
{
   bool retval = false;

   if (toWrapper == NULL) return false;
   
   if (toWrapper->GetWrapperType() == Gmat::ARRAY_WT)
   {
      throw CommandException(wxT("A value of type \"Array\" on command \"") + typeName + 
                  wxT("\" is not currently an allowed value.\nThe allowed values are:")
                  wxT(" [ Real Number, Variable, Array Element, or Parameter ]. ")); 
   }
   //if (toWrapper->GetWrapperType() == Gmat::STRING_OBJECT_WT)
   //{
   //   throw CommandException("A value of type \"String Object\" on command \"" + typeName + 
   //               "\" is not an allowed value.\nThe allowed values are:"
   //               " [ Real Number, Variable, Array Element, or Parameter ]. "); 
   //}
   CheckDataType(toWrapper,Gmat::REAL_TYPE, wxT("NonlinearConstraint"), true);
   
   #ifdef DEBUG_NLC_WRAPPER_CODE   
   MessageInterface::ShowMessage(wxT("   Setting wrapper \"%s\" on NLC command\n"), 
      withName.c_str());
   #endif

   if (arg1Name == withName)
   {
      arg1   = toWrapper;
      retval = true;
   }
   
   if (arg2Name == withName)
   {
      arg2   = toWrapper;
      retval = true;
   }
       
   return retval;
}


//------------------------------------------------------------------------------
// void ClearWrappers()
//------------------------------------------------------------------------------
void NonlinearConstraint::ClearWrappers()
{
   std::vector<ElementWrapper*> temp;
   if (arg1)
   {
      temp.push_back(arg1);
      arg1 = NULL;
   }
   if (arg2)
   {
      if (find(temp.begin(), temp.end(), arg2) == temp.end())
      {
         temp.push_back(arg2);
         arg2 = NULL;
      }
   }
   
   ElementWrapper *wrapper;
   for (UnsignedInt i = 0; i < temp.size(); ++i)
   {
      wrapper = temp[i];
      delete wrapper;
   }
}


//------------------------------------------------------------------------------
//  bool Initialize()
//------------------------------------------------------------------------------
/**
 * Performs the initialization needed to run the NonlinearConstraint command.
 *
 * @return true if the GmatCommand is initialized, false if an error occurs.
 */
//------------------------------------------------------------------------------
bool NonlinearConstraint::Initialize()
{
   #if DEBUG_NONLINEAR_CONSTRAINT_INIT
   MessageInterface::ShowMessage
      (wxT("NonlinearConstraint::Initialize() entered. interpreted=%d, optimizer=%p, ")
       wxT("arg1=%p, arg2=%p\n"), interpreted, optimizer, arg1, arg2);
   #endif

   // Commented out since this caused crash when this command created
   // from the GUI and run (loj: 2007.05.15)
   //if (!interpreted)
   //   if (!InterpretAction())
   //      throw CommandException(
   //         "NonlinearConstraint: error interpreting input data\n");
   
   bool retval = GmatCommand::Initialize();

   if (optimizer == NULL)
      throw CommandException(
         wxT("Optimizer not initialized for NonlinearConstraint command\n  \"")
         + generatingString + wxT("\"\n"));
   
   // Set references for the wrappers   
   #ifdef DEBUG_NONLINEAR_CONSTRAINT_PARAM
      MessageInterface::ShowMessage(wxT("Setting refs for arg1\n"));
   #endif
   if (SetWrapperReferences(*arg1) == false)
      return false;
   CheckDataType(arg1, Gmat::REAL_TYPE, wxT("NonlinearConstraint"));
   #ifdef DEBUG_NONLINEAR_CONSTRAINT_PARAM
      MessageInterface::ShowMessage(wxT("Setting refs for arg2\n"));
   #endif
   if (SetWrapperReferences(*arg2) == false)
      return false;
   CheckDataType(arg2, Gmat::REAL_TYPE, wxT("NonlinearConstraint"));
   // The optimizer cannot be finalized until all of the loop is initialized
   optimizerDataFinalized = false;

   #if DEBUG_NONLINEAR_CONSTRAINT_INIT
   MessageInterface::ShowMessage
      (wxT("NonlinearConstraint::Initialize() exiting. optimizer=%p, arg1=%p, arg2 = %p\n"), 
      optimizer, arg1, arg2);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
//  bool Execute()
//------------------------------------------------------------------------------
/**
 * NonlinearConstraint the variables defined for this targeting loop.
 *
 * This method (will eventually) feeds data to the targeter state machine in 
 * order to determine the variable values needed to achieve the user specified
 * goals.
 *
 * @return true if the GmatCommand runs to completion, false if an error
 *         occurs.
 */
//------------------------------------------------------------------------------
bool NonlinearConstraint::Execute()
{
   #ifdef DEBUG_NONLINEAR_CONSTRAINT_EXEC
   MessageInterface::ShowMessage
      (wxT("NonlinearConstraint::Execute() optimizerDataFinalized=%d\n, optimizer=%s, ")
       wxT("arg1=%p, arg2=%p\n"), optimizerDataFinalized, optimizer, arg1, arg2);
   MessageInterface::ShowMessage
      (wxT("   arg1Name=%s\n"), arg1Name.c_str());
   MessageInterface::ShowMessage
      (wxT("   arg2Name=%s\n"), arg2Name.c_str());
   #endif
   
   bool retval = true;
   if (!optimizerDataFinalized) 
   {
      // Tell the optimizer about the constraint
      Real conData[1];
      conData[0] = 54321.0;  // objective function value here .........
      constraintId = optimizer->SetSolverResults(conData, arg1Name, 
                     isIneqString);

      optimizerDataFinalized = true;
      return retval;
   }
   
   //#ifdef DEBUG_NLC_VALUES
   //   MessageInterface::ShowMessage("NLC:Execute - desiredValue = %.12f\n",
   //      desiredValue);
   //#endif
   
   Real constraintValue = -999.99;
   // Evaluate variable and pass it to the optimizer
   if ( (arg1 != NULL) && (arg2 != NULL) )
   {
      desiredValue    = arg2->EvaluateReal();
      constraintValue = arg1->EvaluateReal();
      #ifdef DEBUG_NLC_VALUES
         MessageInterface::ShowMessage(wxT("NLC:Execute - desiredValue = %.12f\n"),
            desiredValue);
         MessageInterface::ShowMessage(wxT("NLC:Execute - (1) constraintValue = %.12f\n"),
            constraintValue);
      #endif
      switch (op)
      {
         case EQUAL:
         case LESS_THAN_OR_EQUAL:
            constraintValue = constraintValue - desiredValue;
            break;
         case GREATER_THAN_OR_EQUAL:
            constraintValue = desiredValue - constraintValue;
            break;
         default:
            break;
      }
      #ifdef DEBUG_NLC_VALUES
         MessageInterface::ShowMessage(wxT("NLC:Execute - (2) constraintValue now = %.12f\n"),
            constraintValue);
      #endif
      optimizer->SetResultValue(constraintId, constraintValue, isIneqString);
      #ifdef DEBUG_NONLINEAR_CONSTRAINT_EXEC
         MessageInterface::ShowMessage
            (wxT("   constraint=%s, %p\n"), constraint->GetTypeName().c_str(), constraint);
         MessageInterface::ShowMessage(wxT("   Parameter target: %s constraintValue = %lf\n"),
            constraint->GetTypeName().c_str(), constraintValue);
      #endif
   }
   else 
   {
      // isn't this an error???
      //constraintValue = -999.99;  // what to do here?
      //constraintValue = constraintObject->GetRealParameter(parmId);  // again,no clue
      optimizer->SetResultValue(constraintId, constraintValue, isIneqString);
      #ifdef DEBUG_NONLINEAR_CONSTRAINT_EXEC
         MessageInterface::ShowMessage(wxT("   Object target: constraintValue = %lf\n"), 
         constraintValue);
      #endif
   }

   BuildCommandSummary(true);
   
   return retval;
}


//------------------------------------------------------------------------------
// void RunComplete()
//------------------------------------------------------------------------------
void NonlinearConstraint::RunComplete()
{
   #ifdef DEBUG_NLC_EXECUTE
      MessageInterface::ShowMessage(
      wxT("In NLC::RunComplete, optimizerDataFinalized = %s, ... now setting it to false\n"),
      (solverDataFinalized? wxT("true") : wxT("false")));
   #endif
   optimizerDataFinalized = false;
   GmatCommand::RunComplete();
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
 * @param mode    Specifies the type of serialization requested.  (Not yet used
 *                in commands)
 * @param prefix  Optional prefix appended to the object's name.  (Not yet used
 *                in commands)
 * @param useName Name that replaces the object's name.  (Not yet used in
 *                commands)
 *
 * @return The script line that, when interpreted, defines this NonlinearConstraint command.
 */
//------------------------------------------------------------------------------
const wxString& NonlinearConstraint::GetGeneratingString(Gmat::WriteMode mode,
                                            const wxString &prefix,
                                            const wxString &useName)
{
   // Build the local string
   //wxStringstream tol;  // may need tolerance in the future
   //tol << tolerance;
   wxString opString = OP_STRINGS[(Integer)op];
   //if (op == EQUAL)                   opString = "=";
   //else if (op == LESS_THAN_OR_EQUAL) opString = "<=";
   //else                               opString = ">=";
   //wxString gen = prefix + "NonlinearConstraint " + optimizerName + "(" + 
   //                  constraintName +  opString + nlcParmName + 
   //                  ", {Tolerance = " + tol.str() + "}";
   wxString gen = prefix + wxT("NonlinearConstraint ") + optimizerName + wxT("(") + 
                     arg1Name +  opString + arg2Name; 
 
   generatingString = gen + wxT(");");
   // Then call the base class method
   return GmatCommand::GetGeneratingString(mode, prefix, useName);
}


