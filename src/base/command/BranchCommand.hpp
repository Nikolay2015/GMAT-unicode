//$Id: BranchCommand.hpp 9791 2011-08-24 16:17:06Z wendys-dev $
//------------------------------------------------------------------------------
//                               BranchCommand
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
 * Definition for the Command classes that branch (Target, If, While, etc).
 */
//------------------------------------------------------------------------------


#ifndef BranchCommand_hpp
#define BranchCommand_hpp


#include "GmatCommand.hpp"
#include <vector>


class GMAT_API BranchCommand : public GmatCommand
{
public:
   BranchCommand(const wxString &typeStr);
   virtual ~BranchCommand();
   BranchCommand(const BranchCommand& bc);
   BranchCommand&          operator=(const BranchCommand& bc);
   
   void                    AddBranch(GmatCommand *cmd, Integer which = 0);
   void                    AddToFrontOfBranch(GmatCommand *cmd,
                                              Integer which = 0);
   bool                    ExecuteBranch(Integer which = 0);
   
   // Inherited methods that need refinements to handle the branching
   virtual bool            Append(GmatCommand *cmd);
   virtual bool            Insert(GmatCommand *cmd, GmatCommand *prev);
   virtual GmatCommand*    Remove(GmatCommand *cmd);
   virtual void            BuildCommandSummaryString(bool commandCompleted = true);
   // Insert into the main sequence, not into a branch
   virtual bool            InsertRightAfter(GmatCommand *cmd);
   
   virtual void            SetSolarSystem(SolarSystem *ss);
   virtual void            SetInternalCoordSystem(CoordinateSystem *cs);
   virtual void            SetObjectMap(std::map<wxString, GmatBase*> *map);
   virtual void            SetGlobalObjectMap(std::map<wxString, GmatBase*> *map);
   
   virtual const wxString&
                           GetGeneratingString(Gmat::WriteMode mode =
                                               Gmat::SCRIPTING,
                                               const wxString &prefix = wxT(""),
                                               const wxString &useName = wxT(""));
   
   virtual bool            RenameRefObject(const Gmat::ObjectType type,
                                           const wxString &oldName,
                                           const wxString &newName);
   
   virtual GmatCommand*    GetNext();
   virtual GmatCommand*    GetChildCommand(Integer whichOne = 0);
   virtual void            SetTransientForces(std::vector<PhysicalModel*> *tf);
   virtual bool            Initialize();
   virtual bool            TakeAction(const wxString &action, 
                                      const wxString &actionData = wxT(""));
   virtual bool            Execute();
   virtual void            RunComplete();
   // method to handle GmatFunctions
   const std::vector<GmatCommand*>
                           GetCommandsWithGmatFunctions();
   virtual bool            HasAFunction();
   virtual void            SetCallingFunction(FunctionManager *fm);
   virtual bool            IsExecuting();

   virtual Integer         GetCloneCount();
   virtual GmatBase*       GetClone(Integer cloneIndex);

protected:
      
   // no additional parameters to add at this time
   enum
   {
      BranchCommandParamCount = GmatCommandParamCount,
   };
   
   /// The managed branch(es).
   std::vector <GmatCommand *> branch;
   /// Flag used to indicate if the command is finished executing
   bool                    commandComplete;
   /// Flag used to indicate a run is being executed
   bool                    commandExecuting;
   /// Flag used to indicate a branch is being executed
   bool                    branchExecuting;
   /// the branch that is executing
   Integer                 branchToExecute;
   /// The branch that is being filled while the command sequence is being built
   Integer                 branchToFill;
   /// Local container used to return the full sequence from the branches
   wxString             fullString;
   /// Counter to track how deep the nesting is
   Integer                 nestLevel;
   /// Currently executing member of the branch.  NULL if branch not executing.
   GmatCommand             *current;
   
   std::vector<GmatCommand*>             
                           cmdsWithFunctions;
   
   bool  ShiftBranches(GmatCommand *startWith, Integer ofBranchNumber);
   void  SetPreviousCommand(GmatCommand *cmd, GmatCommand *prev, bool skipBranchEnd);
};

#endif // BranchCommand_hpp
