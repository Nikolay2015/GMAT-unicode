//$Id: MathTree.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                   MathTree
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P.
//
// Author: Allison Greene
// Created: 2006/04/17
//
/**
 * Defines the MathTree base class used for Math in Scripts.
 */
//------------------------------------------------------------------------------
#include "MathTree.hpp"
#include "MathFunction.hpp"
#include "MathElement.hpp"
#include "FunctionRunner.hpp"
#include "StringUtil.hpp"            // for GetArrayIndex()
#include "InterpreterException.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_MATH_TREE 1
//#define DEBUG_MATH_TREE_INIT 1
//#define DEBUG_MATH_TREE_EVAL 1
//#define DEBUG_MATH_WRAPPERS
//#define DEBUG_RENAME
//#define DEBUG_FUNCTION

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//------------------------------------------------------------------------------
//  MathTree()
//------------------------------------------------------------------------------
/**
 * Constructs the MathTree object (default constructor).
 */
//------------------------------------------------------------------------------
MathTree::MathTree(const wxString &typeStr, const wxString &nomme) :
   GmatBase(Gmat::MATH_TREE, typeStr, nomme),
   theTopNode(NULL)
{
}


//------------------------------------------------------------------------------
//  ~MathTree(void)
//------------------------------------------------------------------------------
/**
 * Destroys the MathTree object (destructor).
 */
//------------------------------------------------------------------------------
MathTree::~MathTree()
{
   // Need to delete all math nodes
   if (theTopNode)
   {
      nodesToDelete.clear();
      DeleteNode(theTopNode);
      
      for (std::vector<MathNode*>::iterator i = nodesToDelete.begin();
           i != nodesToDelete.end(); ++i)
      {
         if ((*i) != NULL)
         {
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Remove
               ((*i), (*i)->GetName(), wxT("MathTree::~MathTree()"), (*i)->GetTypeName() +
                wxT(" deleting math node"));
            #endif
            
            delete (*i);
            (*i) = NULL;
         }
      }
      nodesToDelete.clear();
   }
}


//------------------------------------------------------------------------------
//  MathTree(const MathTree &mt)
//------------------------------------------------------------------------------
/**
 * Constructs the MathTree object (copy constructor).
 * 
 * @param <mn> Object that is copied
 */
//------------------------------------------------------------------------------
MathTree::MathTree(const MathTree &mt) :
   GmatBase           (mt),
   theTopNode         (mt.theTopNode),
   theObjectMap       (NULL),
   theGlobalObjectMap (NULL)
{
}


//------------------------------------------------------------------------------
//  MathTree& operator=(const MathTree &mt)
//------------------------------------------------------------------------------
/**
 * Sets one MathTree object to match another (assignment operator).
 * 
 * @param <mt> The object that is copied.
 * 
 * @return this object, with the parameters set as needed.
 */
//------------------------------------------------------------------------------
MathTree& MathTree::operator=(const MathTree &mt)
{
   if (this == &mt)
      return *this;
    
   GmatBase::operator=(mt);
   
   theTopNode         = mt.theTopNode;
   theObjectMap       = NULL;
   theGlobalObjectMap = NULL;
   
   return *this;
}


//------------------------------------------------------------------------------
// const StringArray& GetGmatFunctionNames()
//------------------------------------------------------------------------------
const StringArray& MathTree::GetGmatFunctionNames()
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathTree::GetGmatFunctionNames() theTopNode type=%s, desc='%s'returning ")
       wxT("%d GmatFunctions\n"), theTopNode->GetTypeName().c_str(),
       theTopNode->GetName().c_str(), theGmatFunctionNames.size());
   for (UnsignedInt i=0; i<theGmatFunctionNames.size(); i++)
      MessageInterface::ShowMessage(wxT("   '%s'\n"), theGmatFunctionNames[i].c_str());
   
   #endif
   
   return theGmatFunctionNames;
}


//------------------------------------------------------------------------------
// void SetGmatFunctionNames(StringArray funcList)
//------------------------------------------------------------------------------
void MathTree::SetGmatFunctionNames(StringArray funcList)
{
   theGmatFunctionNames = funcList;
}


//------------------------------------------------------------------------------
// std::vector<Function*> GetFunctions() const
//------------------------------------------------------------------------------
std::vector<Function*> MathTree::GetFunctions() const
{
   return theFunctions;
}


//------------------------------------------------------------------------------
// void SetFunction(Function *function)
//------------------------------------------------------------------------------
void MathTree::SetFunction(Function *function)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathTree::SetFunction() function=<%p>, name='%s'\n"), function,
       function->GetName().c_str());
   #endif
   
   if (theTopNode == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   Calling SetFunctionToRunner() theTopNode type=%s, desc='%s'\n"),
       theTopNode->GetTypeName().c_str(), theTopNode->GetName().c_str());
   #endif
   
   SetFunctionToRunner(theTopNode, function);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetFunction() returning\n"));
   #endif
}

//------------------------------------------------------------------------------
// void SetCallingFunction();
//------------------------------------------------------------------------------
void MathTree::SetCallingFunction(FunctionManager *fm)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathTree::SetCallingFunction() fm=<%p>, name='%s'\n"), fm,
            (fm->GetFunctionName()).c_str());
   #endif
   
   if (theTopNode == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   Calling SetCallingFunction() theTopNode type=%s, desc='%s'\n"),
       theTopNode->GetTypeName().c_str(), theTopNode->GetName().c_str());
   #endif
   
   SetCallingFunctionToRunner(theTopNode, fm);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetCallingFunction() returning\n"));
   #endif
}


//------------------------------------------------------------------------------
// MathNode* GetTopNode()
//------------------------------------------------------------------------------
MathNode* MathTree::GetTopNode()
{
   return theTopNode;
}


//------------------------------------------------------------------------------
// void SetTopNode(MathNode *node)
//------------------------------------------------------------------------------
void MathTree::SetTopNode(MathNode *node)
{
   theTopNode = node;
}


//------------------------------------------------------------------------------
// void SetMathWrappers(std::map<wxString, ElementWrapper*> *wrapperMap)
//------------------------------------------------------------------------------
void MathTree::SetMathWrappers(std::map<wxString, ElementWrapper*> *wrapperMap)
{
   if (theTopNode == NULL)
      return;
   
   theWrapperMap = wrapperMap;
   
   #ifdef DEBUG_MATH_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("MathTree::SetMathWrappers() theWrapperMap=%p, theTopNode=%s, %s\n"),
       theWrapperMap, theTopNode->GetTypeName().c_str(), theTopNode->GetName().c_str());
   #endif
   
   SetMathElementWrappers(theTopNode);
}


//------------------------------------------------------------------------------
// void Evaluate() const
//------------------------------------------------------------------------------
Real MathTree::Evaluate()
{
   #ifdef DEBUG_MATH_TREE_EVAL
   MessageInterface::ShowMessage
      (wxT("MathTree::Evaluate() theTopNode=%s, %s\n"), theTopNode->GetTypeName().c_str(),
       theTopNode->GetName().c_str());
   #endif
   return theTopNode->Evaluate();
}


//------------------------------------------------------------------------------
// Rmatrix *MatrixEvaluate()
//------------------------------------------------------------------------------
Rmatrix MathTree::MatrixEvaluate()
{
   #ifdef DEBUG_MATH_TREE
   MessageInterface::ShowMessage
      (wxT("MathTree::MatrixEvaluate() theTopNode=%s, %s\n"), theTopNode->GetTypeName().c_str(),
       theTopNode->GetName().c_str());
   #endif
   return theTopNode->MatrixEvaluate();
}


//------------------------------------------------------------------------------
// bool Initialize(ObjectMap *objectMap, ObjectMap *globalObjectMap)
//------------------------------------------------------------------------------
bool MathTree::Initialize(ObjectMap *objectMap, ObjectMap *globalObjectMap)
{   
   if (theTopNode == NULL)
   {
      #ifdef DEBUG_MATH_TREE_INIT
      MessageInterface::ShowMessage
         (wxT("MathTree::Initialize() theTopNode is NULL, so just returning true\n"));
      #endif
      return true;
   }
   
   theObjectMap       = objectMap;
   theGlobalObjectMap = globalObjectMap;
   
   #ifdef DEBUG_MATH_TREE_INIT
   MessageInterface::ShowMessage
      (wxT("MathTree::Initialize() theTopNode=%s, %s\n"), theTopNode->GetTypeName().c_str(),
       theTopNode->GetName().c_str());
   #endif
   
   return InitializeParameter(theTopNode);
}


//------------------------------------------------------------------------------
// void Finalize()
//------------------------------------------------------------------------------
void MathTree::Finalize()
{   
   if (theTopNode == NULL)
   {
      #ifdef DEBUG_MATH_TREE_FINALIZE
      MessageInterface::ShowMessage
         (wxT("MathTree::Finalize() theTopNode is NULL, so just returning true\n"));
      #endif
      return;
   }
   
   #ifdef DEBUG_MATH_TREE_FINALIZE
   MessageInterface::ShowMessage
      (wxT("MathTree::Finalize() theTopNode=%s, %s\n"), theTopNode->GetTypeName().c_str(),
       theTopNode->GetName().c_str());
   #endif
   
   FinalizeFunctionRunner(theTopNode);
}


//------------------------------------------------------------------------------
// void GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount) const
//------------------------------------------------------------------------------
void MathTree::GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount) 
{
    theTopNode->GetOutputInfo(type, rowCount, colCount);
}


//------------------------------------------------------------------------------
//  void SetObjectMap(ObjectMap *map)
//------------------------------------------------------------------------------
/**
 * Called by the Assignment to set the local asset store used by the GmatCommand
 * 
 * @param <map> Pointer to the local object map
 */
//------------------------------------------------------------------------------
void MathTree::SetObjectMap(ObjectMap *map)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetObjectMap() map=%p\n"), map);
   #endif
   
   if (theTopNode == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   Calling SetObjectMapToRunner() theTopNode type='%s', desc='%s'\n"),
       theTopNode->GetTypeName().c_str(), theTopNode->GetName().c_str());
   #endif
   
   SetObjectMapToRunner(theTopNode, map);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetObjectMap() returning\n"));
   #endif
}


//------------------------------------------------------------------------------
//  void SetGlobalObjectMap(ObjectMap *map)
//------------------------------------------------------------------------------
/**
 * Called by the Assignment to set the global asset store used by the GmatCommand
 * 
 * @param <map> Pointer to the global object map
 */
//------------------------------------------------------------------------------
void MathTree::SetGlobalObjectMap(ObjectMap *map)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetGlobalObjectMap() map=%p\n"), map);
   #endif
   
   if (theTopNode == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   Calling SetGlobalObjectMapToRunner() theTopNode type='%s', desc='%s'\n"),
       theTopNode->GetTypeName().c_str(), theTopNode->GetName().c_str());
   #endif
   
   SetGlobalObjectMapToRunner(theTopNode, map);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetGlobalObjectMap() returning\n"));
   #endif
}


//------------------------------------------------------------------------------
// void SetSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
void MathTree::SetSolarSystem(SolarSystem *ss)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetSolarSystem() ss=%p\n"), ss);
   #endif
   
   if (theTopNode == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   Calling SetSolarSystemToRunner() theTopNode type='%s', desc='%s'\n"),
       theTopNode->GetTypeName().c_str(), theTopNode->GetName().c_str());
   #endif
   
   SetSolarSystemToRunner(theTopNode, ss);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetSolarSystem() returning\n"));
   #endif
}


//------------------------------------------------------------------------------
// void SetInternalCoordSystem(CoordinateSystem *cs)
//------------------------------------------------------------------------------
void MathTree::SetInternalCoordSystem(CoordinateSystem *cs)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetInternalCoordSystem() cs=%p\n"), cs);
   #endif
   
   if (theTopNode == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   Calling SetInternalCoordSystemToRunner() theTopNode type='%s', desc='%s'\n"),
       theTopNode->GetTypeName().c_str(), theTopNode->GetName().c_str());
   #endif
   
   SetInternalCoordSystemToRunner(theTopNode, cs);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetInternalCoordSystem() returning\n"));
   #endif
}


//------------------------------------------------------------------------------
// void SetTransientForces(std::vector<PhysicalModel*> *tf)
//------------------------------------------------------------------------------
void MathTree::SetTransientForces(std::vector<PhysicalModel*> *tf)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetTransientForces() tf=%p\n"), tf);
   #endif
   
   if (theTopNode == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   Calling SetTransientForcesToRunner() theTopNode type=%s, desc='%s'\n"),
       theTopNode->GetTypeName().c_str(), theTopNode->GetName().c_str());
   #endif
   
   SetTransientForcesToRunner(theTopNode, tf);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetTransientForces() returning\n"));
   #endif
}


//------------------------------------------------------------------------------
// void SetPublisher(Publisher *pub)
//------------------------------------------------------------------------------
void MathTree::SetPublisher(Publisher *pub)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetPublisher() pub=<%p>\n"), pub);
   #endif
   
   if (theTopNode == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   Calling SetPublisherToRunner() theTopNode type='%s', desc='%s'\n"),
       theTopNode->GetTypeName().c_str(), theTopNode->GetName().c_str());
   #endif
   
   SetPublisherToRunner(theTopNode, pub);
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage(wxT("MathTree::SetPublisher() returning\n"));
   #endif
}


//---------------------------------------------------------------------------
// virtual bool RenameRefObject(const Gmat::ObjectType type,
//                 const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
/*
 * Renames referenced objects
 *
 * @param <type> type of the reference object.
 * @param <oldName> old name of the reference object.
 * @param <newName> new name of the reference object.
 *
 * @return always true to indicate RenameRefObject() was implemented.
 */
//---------------------------------------------------------------------------
bool MathTree::RenameRefObject(const Gmat::ObjectType type,
                               const wxString &oldName,
                               const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("MathTree::RenameRefObject() oldName=<%s>, newName=<%s>\n"), oldName.c_str(),
       newName.c_str());
   #endif
   
   if (theTopNode)
      RenameParameter(theTopNode, type, oldName, newName);
   
   return true;
}


//------------------------------------------------------------------------------
// virtual const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
const StringArray& MathTree::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   theAllParamArray.clear();
   
   if (theTopNode)
      CreateParameterNameArray(theTopNode);
   
   return theAllParamArray;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the MathTree.
 *
 * @return clone of the MathTree.
 *
 */
//------------------------------------------------------------------------------
GmatBase* MathTree::Clone(void) const
{
   return (new MathTree(*this));
}


//------------------------------------------------------------------------------
// bool InitializeParameter(MathNode *node)
//------------------------------------------------------------------------------
bool MathTree::InitializeParameter(MathNode *node)
{   
   if (node == NULL)
   {
      #ifdef DEBUG_MATH_TREE_INIT
      MessageInterface::ShowMessage
         (wxT("MathTree::InitializeParameter() theTopNode is NULL, so just returning true\n"));
      #endif
      return true;
   }
   
   #ifdef DEBUG_MATH_TREE_INIT
   MessageInterface::ShowMessage
      (wxT("MathTree::InitializeParameter() node=%s, %s\n   IsFunction=%d, IsNumber=%d, ")
       wxT("IsFunctionInput=%d\n"), node->GetTypeName().c_str(), node->GetName().c_str(),
       node->IsFunction(), node->IsNumber(), node->IsFunctionInput());
   #endif
   
   if (!node->IsFunction())
   {
      if (node->IsNumber())
         return true;
      
      if (node->IsFunctionInput())
         return true;
      
      // Now MathElement can have more than one ref objects due to GmatFunction
      // input arguments.
      StringArray refNames = node->GetRefObjectNameArray(Gmat::PARAMETER);
      wxString undefNames;
      
      for (UnsignedInt i=0; i<refNames.size(); i++)
      {
         #ifdef DEBUG_MATH_TREE_INIT
         MessageInterface::ShowMessage
            (wxT("MathTree::InitializeParameter() refNames[%d]=%s\n"), i, refNames[i].c_str());
         #endif
         
         // Handle array index
         Integer row, col;
         wxString newName;
         GmatStringUtil::GetArrayIndex(refNames[i], row, col, newName);
         
         if (theObjectMap->find(newName) != theObjectMap->end())
         {
            node->SetRefObject((*theObjectMap)[newName], Gmat::PARAMETER,
                               newName);
            
            #ifdef DEBUG_MATH_TREE_INIT
            MessageInterface::ShowMessage
               (wxT("MathTree::InitializeParameter() Found %s from theObjectMap\n"),
                refNames[i].c_str());
            #endif
         }
         else if (theGlobalObjectMap->find(newName) != theGlobalObjectMap->end())
         {
            node->SetRefObject((*theGlobalObjectMap)[newName], Gmat::PARAMETER,
                               newName);
            
            #ifdef DEBUG_MATH_TREE_INIT
            MessageInterface::ShowMessage
               (wxT("MathTree::InitializeParameter() Found %s from theGlobalObjectMap\n"),
                refNames[i].c_str());
            #endif
         }
         else
         {
            #ifdef DEBUG_MATH_TREE_INIT
            MessageInterface::ShowMessage
               (wxT("MathTree::InitializeParameter() Unable to find ") + newName +
                wxT(" from theObjectMap or theGlobalObjectMap\n"));
            #endif
            
            undefNames = undefNames + wxT(", ") + newName;
         }
      }
      
      if (undefNames == wxT(""))
         return true;
      else
         throw InterpreterException(wxT("Undefined variable(s) \"") + undefNames + wxT("\" used"));
   }
   else
   {
      MathNode *left = node->GetLeft();
      bool result1 = InitializeParameter(left);
      
      MathNode *right = node->GetRight();
      bool result2 = InitializeParameter(right);
      
      return (result1 && result2);
   }
}


//------------------------------------------------------------------------------
// void FinalizeFunctionRunner(MathNode *node)
//------------------------------------------------------------------------------
void MathTree::FinalizeFunctionRunner(MathNode *node)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathTree::FinalizeFunctionRunner() node=%p\n"), node);
   #endif
   
   if (node == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   node type='%s', name='%s'\n"), node->GetTypeName().c_str(),
       node->GetName().c_str());
   #endif
   
   if (!node->IsFunction())
      return;
   
   if (node->IsOfType(wxT("FunctionRunner")))
      ((FunctionRunner*)(node))->Finalize();
   
   MathNode *left = node->GetLeft();   
   FinalizeFunctionRunner(left);
   
   MathNode *right = node->GetRight();
   FinalizeFunctionRunner(right);
}


//------------------------------------------------------------------------------
// void SetMathElementWrappers(MathNode *node)
//------------------------------------------------------------------------------
void MathTree::SetMathElementWrappers(MathNode *node)
{
   if (node == NULL)
      return;
   
   if (!node->IsFunction())
   {
      if (node->IsNumber())
         return;
      
      #ifdef DEBUG_MATH_WRAPPERS
      MessageInterface::ShowMessage
         (wxT("   Setting wrappers to '%s'\n"), node->GetName().c_str());
      #endif
      
      ((MathElement*)(node))->SetMathWrappers(theWrapperMap);
   }
   else
   {
      MathNode *left = node->GetLeft();
      SetMathElementWrappers(left);
      
      MathNode *right = node->GetRight();
      SetMathElementWrappers(right);
   }
}


//------------------------------------------------------------------------------
// void SetFunctionToRunner(MathNode *node, Function *function)
//------------------------------------------------------------------------------
void MathTree::SetFunctionToRunner(MathNode *node, Function *function)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathTree::SetFunctionToRunner() node=%p, function=%p\n"), node, function);
   #endif
   
   if (node == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   node type='%s', name='%s'\n"), node->GetTypeName().c_str(),
       node->GetName().c_str());
   #endif
   
   if (!node->IsFunction())
      return;
   
   if (node->IsOfType(wxT("FunctionRunner")))
      ((FunctionRunner*)(node))->SetFunction(function);
   
   MathNode *left = node->GetLeft();   
   SetFunctionToRunner(left, function);
   
   MathNode *right = node->GetRight();
   SetFunctionToRunner(right, function);
}

//------------------------------------------------------------------------------
// void SetCallingFunctionToRunner(MathNode *node, Function *fm)
//------------------------------------------------------------------------------
void MathTree::SetCallingFunctionToRunner(MathNode *node, FunctionManager *fm)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathTree::SetCallingFunctionToRunner() node=%p, function=%p\n"), node, fm);
   #endif
   
   if (node == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   node type='%s', name='%s'\n"), node->GetTypeName().c_str(),
       node->GetName().c_str());
   #endif
   
   if (!node->IsFunction())
      return;
   
   if (node->IsOfType(wxT("FunctionRunner")))
      ((FunctionRunner*)(node))->SetCallingFunction(fm);
   
   MathNode *left = node->GetLeft();   
   SetCallingFunctionToRunner(left, fm);
   
   MathNode *right = node->GetRight();
   SetCallingFunctionToRunner(right, fm);
}


//------------------------------------------------------------------------------
// void SetObjectMapToRunner(MathNode *node, ObjectMap *map)
//------------------------------------------------------------------------------
void MathTree::SetObjectMapToRunner(MathNode *node, ObjectMap *map)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathTree::SetObjectMapToRunner() node=%p, map=%p\n"), node, map);
   #endif
   
   if (node == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   node type='%s', desc='%s'\n"), node->GetTypeName().c_str(),
       node->GetName().c_str());
   #endif
   
   if (!node->IsFunction())
      return;
   
   if (node->IsOfType(wxT("FunctionRunner")))
      ((FunctionRunner*)(node))->SetObjectMap(map);
   
   MathNode *left = node->GetLeft();   
   SetObjectMapToRunner(left, map);
   
   MathNode *right = node->GetRight();
   SetObjectMapToRunner(right, map);
}


//------------------------------------------------------------------------------
// void SetGlobalObjectMapToRunner(MathNode *node, ObjectMap *map)
//------------------------------------------------------------------------------
void MathTree::SetGlobalObjectMapToRunner(MathNode *node, ObjectMap *map)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathTree::SetGlobalObjectMapToRunner() node=%p, map=%p\n"), node, map);
   #endif
   
   if (node == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   node type='%s', desc='%s'\n"), node->GetTypeName().c_str(),
       node->GetName().c_str());
   #endif
   
   if (!node->IsFunction())
      return;
   
   if (node->IsOfType(wxT("FunctionRunner")))
      ((FunctionRunner*)(node))->SetGlobalObjectMap(map);
   
   MathNode *left = node->GetLeft();   
   SetGlobalObjectMapToRunner(left, map);
   
   MathNode *right = node->GetRight();
   SetGlobalObjectMapToRunner(right, map);
}


//------------------------------------------------------------------------------
// void SetSolarSystemToRunner(MathNode *node, SolarSystem *ss)
//------------------------------------------------------------------------------
void MathTree::SetSolarSystemToRunner(MathNode *node, SolarSystem *ss)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathTree::SetSolarSystemToRunner() node=%p, ss=%p\n"), node, ss);
   #endif
   
   if (node == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   node type='%s', desc='%s'\n"), node->GetTypeName().c_str(),
       node->GetName().c_str());
   #endif
   
   if (!node->IsFunction())
      return;
   
   if (node->IsOfType(wxT("FunctionRunner")))
      ((FunctionRunner*)(node))->SetSolarSystem(ss);
   
   MathNode *left = node->GetLeft();   
   SetSolarSystemToRunner(left, ss);
   
   MathNode *right = node->GetRight();
   SetSolarSystemToRunner(right, ss);
}


//------------------------------------------------------------------------------
// void SetInternalCoordSystemToRunner(MathNode *node, CoordinateSystem *cs)
//------------------------------------------------------------------------------
void MathTree::SetInternalCoordSystemToRunner(MathNode *node, CoordinateSystem *cs)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathTree::SetInternalCoordSystemToRunner() node=%p, cs=%p\n"), node, cs);
   #endif
   
   if (node == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   node type='%s', desc='%s'\n"), node->GetTypeName().c_str(),
       node->GetName().c_str());
   #endif
   
   if (!node->IsFunction())
      return;
   
   if (node->IsOfType(wxT("FunctionRunner")))
      ((FunctionRunner*)(node))->SetInternalCoordSystem(cs);
   
   MathNode *left = node->GetLeft();   
   SetInternalCoordSystemToRunner(left, cs);
   
   MathNode *right = node->GetRight();
   SetInternalCoordSystemToRunner(right, cs);
}


//------------------------------------------------------------------------------
// void SetTransientForcesToRunner(MathNode *node, std::vector<PhysicalModel*> *tf)
//------------------------------------------------------------------------------
void MathTree::SetTransientForcesToRunner(MathNode *node, std::vector<PhysicalModel*> *tf)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathTree::SetTransientForcesToRunner() node=%p, tf=%p\n"), node, tf);
   #endif
   
   if (node == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   node type='%s', desc='%s'\n"), node->GetTypeName().c_str(),
       node->GetName().c_str());
   #endif
   
   if (!node->IsFunction())
      return;
   
   if (node->IsOfType(wxT("FunctionRunner")))
      ((FunctionRunner*)(node))->SetTransientForces(tf);
   
   MathNode *left = node->GetLeft();   
   SetTransientForcesToRunner(left, tf);
   
   MathNode *right = node->GetRight();
   SetTransientForcesToRunner(right, tf);
}


//------------------------------------------------------------------------------
// void SetPublisherToRunner(MathNode *node, Publisher *pub)
//------------------------------------------------------------------------------
void MathTree::SetPublisherToRunner(MathNode *node, Publisher *pub)
{
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("MathTree::SetPublisherToRunner() node=<%p>, pub=<%p>\n"), node, pub);
   #endif
   
   if (node == NULL)
      return;
   
   #ifdef DEBUG_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   node type='%s', desc='%s'\n"), node->GetTypeName().c_str(),
       node->GetName().c_str());
   #endif
   
   if (!node->IsFunction())
      return;
   
   if (node->IsOfType(wxT("FunctionRunner")))
      ((FunctionRunner*)(node))->SetPublisher(pub);
   
   MathNode *left = node->GetLeft();
   SetPublisherToRunner(left, pub);
   
   MathNode *right = node->GetRight();
   SetPublisherToRunner(right, pub);
}


//------------------------------------------------------------------------------
// bool RenameParameter(MathNode *node, const Gmat::ObjectType type,
//                      const wxString &oldName, const wxString &newName)
//------------------------------------------------------------------------------
bool MathTree::RenameParameter(MathNode *node, const Gmat::ObjectType type,
                               const wxString &oldName,
                               const wxString &newName)
{
   #ifdef DEBUG_RENAME
   MessageInterface::ShowMessage
      (wxT("MathTree::RenameParameter() oldName=<%s>, newName=<%s>, node=<%s>\n"),
       oldName.c_str(), newName.c_str(), node ? node->GetName().c_str() : wxT("NULL"));
   #endif
   
   if (node == NULL)
      return true;
   
   wxString nodeName = node->GetName();
   
   if (nodeName.find(oldName) != nodeName.npos)
   {
      #ifdef DEBUG_RENAME
      MessageInterface::ShowMessage(wxT("   old nodeName=<%s>\n"), nodeName.c_str());
      #endif
      
      nodeName = GmatStringUtil::ReplaceName(nodeName, oldName, newName);
      node->SetName(nodeName);
      
      #ifdef DEBUG_RENAME
      MessageInterface::ShowMessage(wxT("   new nodeName=<%s>\n"), node->GetName().c_str());
      #endif
   }
   
   if (!node->IsFunction())
   {
      if (!node->IsNumber())
         node->RenameRefObject(type, oldName, newName);
   }
   else
   {
      //loj: ================ try getting node name here
      MathNode *left = node->GetLeft();
      RenameParameter(left, type, oldName, newName);
      
      MathNode *right = node->GetRight();
      RenameParameter(right, type, oldName, newName);
   }
   
   return true;
}


//------------------------------------------------------------------------------
// void CreateParameterNameArray(MathNode *node)
//------------------------------------------------------------------------------
void MathTree::CreateParameterNameArray(MathNode *node)
{
   theParamArray.clear();
   
   if (node == NULL)
      return;
   
   #ifdef DEBUG_MATH_WRAPPERS
   MessageInterface::ShowMessage
      (wxT("MathTree::CreateParameterNameArray() node=%s\n"), node->GetName().c_str());
   #endif
   
   if (!node->IsFunction())
   {
      // if math element is not a number
      if (!node->IsNumber())
      {
         StringArray refs = node->GetRefObjectNameArray(Gmat::PARAMETER);
         for (UnsignedInt i=0; i<refs.size(); i++)
         {
            #ifdef DEBUG_MATH_WRAPPERS
            MessageInterface::ShowMessage(wxT("   [%d] %s\n"), i, refs[i].c_str());
            #endif
            
            // add if not found in the all ref array
            if (find(theAllParamArray.begin(), theAllParamArray.end(), refs[i]) ==
                theAllParamArray.end())
               theAllParamArray.push_back(refs[i]);
         }
      }
   }
   else
   {
      #ifdef __USE_ARGLIST_FOR_REFNAMES__
      // add function input arguments if node is a FunctionRunner
      if (node->IsOfType(wxT("FunctionRunner")))
      {
         #ifdef DEBUG_MATH_WRAPPERS
         MessageInterface::ShowMessage
            (wxT("   It is a FunctionRunner of '%s'\n"), node->GetName().c_str());
         #endif
         
         StringArray inputs = ((FunctionRunner*)node)->GetInputs();
         for (UnsignedInt i=0; i<inputs.size(); i++)
         {
            #ifdef DEBUG_MATH_WRAPPERS
            MessageInterface::ShowMessage(wxT("   inputs[%d]='%s'\n"), i, inputs[i].c_str());
            #endif
            
            if (inputs[i] == wxT(""))
               continue;
            
            if (find(theAllParamArray.begin(), theAllParamArray.end(), inputs[i]) ==
                theAllParamArray.end())
               theAllParamArray.push_back(inputs[i]);
         }
      }
      #endif
      
      MathNode *left = node->GetLeft();
      CreateParameterNameArray(left);
      
      MathNode *right = node->GetRight();
      CreateParameterNameArray(right);
   }
}


//------------------------------------------------------------------------------
// void DeleteNode(MathNode *node)
//------------------------------------------------------------------------------
void MathTree::DeleteNode(MathNode *node)
{
   if (node == NULL)
      return;
   
   // add node to delete   
   nodesToDelete.push_back(node);
   
   MathNode *left = node->GetLeft();
   if (left != NULL)
      DeleteNode(left);
   
   MathNode *right = node->GetRight();
   if (right != NULL)
      DeleteNode(right);
}

