//$Id: Assignment.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                Assignment
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
// Created: 2004/02/03
//
/**
 * Definition of the Assignment command class.
 * 
 * The assignment class is the Command class that handles commands of the form
 * 
 *     GMAT object.parameter = value;
 *     GMAT variable = parameter;
 *     GMAT variable = equation;
 * 
 */
//------------------------------------------------------------------------------
#ifndef Assignment_hpp
#define Assignment_hpp

#include "gmatdefs.hpp"
#include "GmatCommand.hpp"
#include "MathTree.hpp"
#include "Function.hpp"

class GMAT_API Assignment : public GmatCommand
{
public:
   Assignment();
   virtual ~Assignment();
   Assignment(const Assignment& a);
   Assignment&          operator=(const Assignment& a);
   
   MathTree*            GetMathTree();
   virtual bool         HasAFunction();
   const StringArray&   GetGmatFunctionNames();
   void                 SetMathWrappers();
   
   wxString          GetLHS() { return lhs; }
   wxString          GetRHS() { return rhs; }
   
   virtual void         SetFunction(Function *function);
   virtual std::vector<Function*> GetFunctions() const;
   
   // inherited from GmatCommand
   virtual void         SetPublisher(Publisher *pub);
   virtual void         SetSolarSystem(SolarSystem *ss);
   virtual void         SetInternalCoordSystem(CoordinateSystem *cs);
   virtual void         SetTransientForces(std::vector<PhysicalModel*> *tf);
   virtual void         SetObjectMap(ObjectMap *map);
   virtual void         SetGlobalObjectMap(ObjectMap *map);
   
   virtual bool         InterpretAction();
   virtual const StringArray& GetObjectList();
   virtual bool         Validate();
   virtual bool         Initialize();
   virtual bool         Execute();
   virtual void         RunComplete();
   virtual bool         SkipInterrupt();
   virtual void         SetCallingFunction(FunctionManager *fm);
   
   virtual const StringArray& 
                        GetWrapperObjectNameArray();
   virtual bool         SetElementWrapper(ElementWrapper* toWrapper,
                                          const wxString &withName);
   virtual void         ClearWrappers();
   
   // inherited from GmatBase
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   
   virtual GmatBase*    Clone() const;
   
   virtual const wxString&
                        GetGeneratingString(
                           Gmat::WriteMode mode = Gmat::SCRIPTING,
                           const wxString &prefix = wxT(""),
                           const wxString &useName = wxT(""));

protected:
   
   /// string on the left side of the equals sign
   wxString          lhs;
   /// string on the right side of the equals sign
   wxString          rhs;
   /// ElementWrapper pointer for the lhs of the equals sign
   ElementWrapper*      lhsWrapper;
   /// ElementWrapper pointer for the rhs of the equals sign
   ElementWrapper*      rhsWrapper;
   /// MathNode pointer for RHS equation
   MathTree             *mathTree;
   /// Wrapper name and ElementWrapper pointer Map for RHS math element
   WrapperMap           mathWrapperMap;

   /// Object pointer for clone management
   GmatBase             *lhsOwner;
   /// Object parameter ID if lhs is an attribute
   Integer              lhsOwnerID;
   
   // methods
   ElementWrapper* RunMathTree();
   void HandleScPropertyChange(ElementWrapper *lhsWrapper);

   void PassToClones();
   void MatchAttribute(Integer id, GmatBase *owner, GmatBase *receiver);
};

#endif // Assignment_hpp
