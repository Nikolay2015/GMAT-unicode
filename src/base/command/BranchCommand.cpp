//$Id: BranchCommand.cpp 10015 2011-11-16 21:31:19Z djcinsb $
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
 * Base class implementation for the Command classes that branch (Target, If, 
 * While, etc).
 */
//------------------------------------------------------------------------------


#include "BranchCommand.hpp"
#include "MessageInterface.hpp"
#include "CallFunction.hpp"
#include "Assignment.hpp"
#include "CommandUtil.hpp"      // for GetCommandSeqString()
#include <sstream>              // for stringstream

//#define DEBUG_BRANCHCOMMAND_DEALLOCATION
//#define DEBUG_BRANCHCOMMAND_APPEND
//#define DEBUG_BRANCHCOMMAND_INSERT
//#define DEBUG_BRANCHCOMMAND_REMOVE
//#define DEBUG_BRANCHCOMMAND_ADD
//#define DEBUG_BRANCHCOMMAND_PREV_CMD
//#define DEBUG_BRANCHCOMMAND_EXECUTION
//#define DEBUG_BRANCHCOMMAND_GEN_STRING
//#define DEBUG_RUN_COMPLETE
//#define DEBUG_BRANCHCOMMAND_GMATFUNCTIONS
//#define DEBUG_BRANCHCOMMAND_SUMMARY

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//  BranchCommand(const wxString &typeStr)
//------------------------------------------------------------------------------
/**
 * Constructs the BranchCommand command (default constructor).
 *
 * @param <typeStr> Strinf setting the type name of the command.
 */
//------------------------------------------------------------------------------
BranchCommand::BranchCommand(const wxString &typeStr) :
   GmatCommand          (typeStr),
   branch               (1),
   commandComplete      (false),
   commandExecuting     (false),
   branchExecuting      (false),
   branchToExecute      (0),
   branchToFill         (0),
   nestLevel            (0),
   current              (NULL)
{
   depthChange = 1;
   parameterCount = BranchCommandParamCount;
   
   objectTypeNames.push_back(wxT("BranchCommand"));
}


//------------------------------------------------------------------------------
// ~BranchCommand(const wxString &typeStr)
//------------------------------------------------------------------------------
/**
 * Destroys the BranchCommand.
 */
//------------------------------------------------------------------------------
BranchCommand::~BranchCommand()
{
   #ifdef DEBUG_BRANCHCOMMAND_DEALLOCATION
      MessageInterface::ShowMessage
         (wxT("In BranchCommand::~BranchCommand() this=<%p> '%s'\n"), this,
          this->GetTypeName().c_str());
      MessageInterface::ShowMessage(wxT("branch.size()=%d\n"), branch.size());
      wxString cmdstr = GmatCommandUtil::GetCommandSeqString(this);
      MessageInterface::ShowMessage(wxT("%s\n"), cmdstr.c_str());
   #endif
   std::vector<GmatCommand*>::iterator node;
   GmatCommand *current = NULL;
   
   for (node = branch.begin(); node != branch.end(); ++node)
   {
      // Find the end for each branch and disconnect it from the start
      current = *node;
      if (current != NULL)
      {
         #ifdef DEBUG_BRANCHCOMMAND_DEALLOCATION
         ShowCommand(wxT("   "), wxT("current="), current);
         #endif
         
         // Why I need to add current != current->GetNext() to avoid
         // infinite loop? It used to work!! (loj: 2008.12.02)
         //while (current->GetNext() != this)
         while (current->GetNext() != this && current != current->GetNext())
         {
            current = current->GetNext();
            if (current == NULL)
               break;
         }
         
         // Calling Remove this way just sets the next pointer to NULL
         if (current)
         {
            #ifdef DEBUG_BRANCHCOMMAND_DEALLOCATION
               MessageInterface::ShowMessage(wxT("   Removing "), current);
            #endif
               
            current->Remove(current);
         }
         
         if (*node)
         {
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Remove
               ((*node), (*node)->GetTypeName(), (*node)->GetTypeName() +
                wxT(" deleting child command"));
            #endif
            delete *node;
            *node = NULL;
         }
      }
      else
         break;
   }
   
   #ifdef DEBUG_BRANCHCOMMAND_DEALLOCATION
      MessageInterface::ShowMessage(wxT("Finished BranchCommand::~BranchCommand()\n"));
   #endif
}


//------------------------------------------------------------------------------
//  BranchCommand(const BranchCommand& bc)
//------------------------------------------------------------------------------
/**
 * Constructs the BranchCommand command (copy constructor).
 *
 * @param <bc> The instance that is copied.
 */
//------------------------------------------------------------------------------
BranchCommand::BranchCommand(const BranchCommand& bc) :
   GmatCommand       (bc),
   branch            (1),
   commandComplete   (false),
   commandExecuting  (false),
   branchExecuting   (false),
   branchToExecute   (0),
   branchToFill      (0),
   nestLevel         (bc.nestLevel),
   current           (NULL)
{
   depthChange = 1;
   parameterCount = BranchCommandParamCount;
}


//------------------------------------------------------------------------------
//  BranchCommand& operator=(const BranchCommand& bc)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <bc> The instance that is copied.
 *
 * @return This instance, set to match the input instance.
 */
//------------------------------------------------------------------------------
BranchCommand& BranchCommand::operator=(const BranchCommand& bc)
{
   if (this != &bc)
      GmatCommand::operator=(bc);

   return *this;
}


//------------------------------------------------------------------------------
//  GmatCommand* GetNext()
//------------------------------------------------------------------------------
/**
 * Access the next command in the mission sequence.
 *
 * For BranchCommands, this method returns its own pointer while the child
 * commands are executing.
 *
 * @return The next command, or NULL if the sequence has finished executing.
 */
//------------------------------------------------------------------------------
GmatCommand* BranchCommand::GetNext()
{
   // Return the next pointer in the command sequence if this command -- 
   // includng its branches -- has finished executing.
   if ((commandExecuting) && (!commandComplete))
      return this;
   
   return next;
}


//------------------------------------------------------------------------------
//  GmatCommand* GetChildCommand(Integer whichOne)
//------------------------------------------------------------------------------
/**
 * Access the children of this command.
 *
 * @param <whichOne> Identifies which child branch is needed.
 *
 * @return The child command starting the indicated branch, or NULL if there is
 *         no such child.
 */
//------------------------------------------------------------------------------
GmatCommand* BranchCommand::GetChildCommand(Integer whichOne)
{
   if (whichOne > (Integer)(branch.size())-1)
      return NULL;
   return branch[whichOne];
}

//------------------------------------------------------------------------------
//  void SetTransientForces(std::vector<PhysicalModel*> *tf)
//------------------------------------------------------------------------------
/**
 * Tells the children about the transient forces.
 *
 * @param <tf> The transient force vector.
 */
//------------------------------------------------------------------------------
void BranchCommand::SetTransientForces(std::vector<PhysicalModel*> *tf)
{
   GmatCommand *currentPtr;
   
   std::vector<GmatCommand*>::iterator node;
   for (node = branch.begin(); node != branch.end(); ++node)
   {
      currentPtr = *node;
      while (currentPtr != this)
      {
         currentPtr->SetTransientForces(tf);
         currentPtr = currentPtr->GetNext();
         if (currentPtr == NULL)
            throw CommandException(wxT("Branch command \"") + generatingString +
                                   wxT("\" was not terminated!"));
      }
   }
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 * Performs the initialization needed to run the BranchCommand command.
 *
 * @return true if the GmatCommand is initialized, false if an error occurs.
 */
//------------------------------------------------------------------------------
bool BranchCommand::Initialize()
{
   #ifdef DEBUG_BRANCHCOMMAND_INIT
   ShowCommand(wxT("BranchCommand::Initialize() entered "), wxT("this="), this);
   #endif
   
   GmatCommand::Initialize();
   
   std::vector<GmatCommand*>::iterator node;
   GmatCommand *currentPtr;
   bool retval = true;
   
   for (node = branch.begin(); node != branch.end(); ++node)
   {
      currentPtr = *node;

      while (currentPtr != this)
      {
         #ifdef DEBUG_BRANCHCOMMAND_INIT
         ShowCommand(wxT("About to initialize child in "), wxT("child="), currentPtr);
         #endif
         if (events != NULL)
            currentPtr->SetEventLocators(events);
         if (!currentPtr->Initialize())
               retval = false;
         currentPtr = currentPtr->GetNext();
         if (currentPtr == NULL)
            throw CommandException(wxT("Branch command \"") + generatingString +
                                   wxT("\" was not terminated!"));
      }
   }
//   for (UnsignedInt i = 0; i < current.size(); ++i)
//      current[i] = NULL;
   
   commandComplete  = false;
   commandExecuting = false;
   branchExecuting = false;
   current = NULL;
   
   return retval;
}


//------------------------------------------------------------------------------
// void AddBranch(GmatCommand *cmd, Integer which)
//------------------------------------------------------------------------------
/**
 * Adds commands to a branch, starting a new branch if needed.
 *
 * @param <cmd>   The command that is added.
 * @param <which> Identifies which child branch is used.
 */
//------------------------------------------------------------------------------
void BranchCommand::AddBranch(GmatCommand *cmd, Integer which)
{
   #ifdef DEBUG_BRANCHCOMMAND_ADD
   ShowCommand(wxT("BranchCommand::"), wxT("AddBranch() cmd = "), cmd);
   MessageInterface::ShowMessage
      (wxT("   which=%d, branch.size=%d\n"), which, branch.size());
   #endif
   
   // Increase the size of the vector if it's not big enough
   if (which >= (Integer)branch.capacity())
   {
      branch.reserve(which+1);
      //current.reserve(which+1);
   }
   
   if (which == (Integer)(branch.size()))
   {
      #ifdef DEBUG_BRANCHCOMMAND_ADD
      ShowCommand(wxT("BranchCommand::"), wxT("::AddBranch() Adding to branch "), cmd);
      #endif
      
      branch.push_back(cmd);
      if (which - 1 >= 0)
         SetPreviousCommand(cmd, branch.at(which-1), true);
   }
   else if (branch[which] == NULL)
   {
      #ifdef DEBUG_BRANCHCOMMAND_ADD
      ShowCommand(wxT("BranchCommand::"), wxT(" Setting branch.at(which) to "), cmd);
      #endif
      
      branch.at(which) = cmd;
      
      // Usually this case is adding Branch command from the GUI
      SetPreviousCommand(cmd, this, false);
   }
   else
   {
      #ifdef DEBUG_BRANCHCOMMAND_ADD
      ShowCommand(wxT("BranchCommand::"), wxT(" Appending "), cmd, wxT(" to "), branch.at(which));
      #endif
      
      (branch.at(which))->Append(cmd);
      
      // We don't want to override previous command (loj: 2008.01.18)
      ////SetPreviousCommand(cmd, branch.at(which), true);
      
   }
} // AddBranch()


//------------------------------------------------------------------------------
// void AddToFrontOfBranch(GmatCommand *cmd, Integer which)
//------------------------------------------------------------------------------
/**
 * Adds commands to the beginning of a branch, starting a new branch if needed.
 *
 * @param <cmd>   The command that is added.
 * @param <which> Identifies which child branch is used.
 */
//------------------------------------------------------------------------------
void BranchCommand::AddToFrontOfBranch(GmatCommand *cmd, Integer which)
{
   
   #ifdef DEBUG_BRANCHCOMMAND_ADD
   ShowCommand(wxT("BranchCommand::"), wxT("AddToFrontOfBranch() cmd = "), cmd);
   MessageInterface::ShowMessage
      (wxT("   which=%d, branch.size=%d\n"), which, branch.size());
   #endif
   
   // Increase the size of the vector if it's not big enough
   if (which >= (Integer)branch.capacity())
      branch.reserve(which+1);
   
   if (which == (Integer)(branch.size()))
   {
      #ifdef DEBUG_BRANCHCOMMAND_ADD
      ShowCommand(wxT("BranchCommand::"), wxT("::AddToFrontOfBranch() Adding to branch "), cmd);
      #endif
      
      branch.push_back(cmd);
      SetPreviousCommand(cmd, this, true);
      
   }
   else if (branch.at(which) == NULL)
   {
      #ifdef DEBUG_BRANCHCOMMAND_ADD
      ShowCommand(wxT("BranchCommand::"), wxT(" Setting branch.at(which) to "), cmd);
      #endif
      
      branch.at(which) = cmd;
   }
   else
   {
      GmatCommand *tmp = branch.at(which);
      
      #ifdef DEBUG_BRANCHCOMMAND_ADD
      ShowCommand(wxT("BranchCommand::"), wxT(" Setting branch.at(which) to "), cmd);
      #endif
      
      branch.at(which) = cmd;
      
      #ifdef DEBUG_BRANCHCOMMAND_ADD
      ShowCommand(wxT("BranchCommand::"), wxT(" Appending "), tmp, wxT(" to "), cmd);
      #endif
      
      cmd->Append(tmp);
      SetPreviousCommand(tmp, cmd, true);
      
   }
} // AddToFrontOfBranch()


//------------------------------------------------------------------------------
// bool BranchCommand::Append(GmatCommand *cmd)
//------------------------------------------------------------------------------
/**
 * Appends a command to the mission sequence.
 *
 * Appends commands to the end of the current command stream, either by adding
 * them to a branch, or by adding them after this command based on the internal
 * BranchCommand flags.
 *
 * @param <cmd>   The command that is added.
 *
 * @return true on success, false on failure.
 */
//------------------------------------------------------------------------------
bool BranchCommand::Append(GmatCommand *cmd)
{
   #ifdef DEBUG_BRANCHCOMMAND_APPEND
   ShowCommand(wxT("BranchCommand::"), wxT("Append() this = "), this, wxT(" next = "), next);
   ShowCommand(wxT("BranchCommand::"), wxT("Append() cmd = "), cmd);
   #endif
   
   // If we are still filling in a branch, append on that branch
   if (branchToFill >= 0)
   {
      AddBranch(cmd, branchToFill);
      return true;
   }
   #ifdef DEBUG_BRANCHCOMMAND_APPEND
      MessageInterface::ShowMessage(
      wxT("In BranchCommand::Append - not appended to this command.\n"));
   #endif
      
   // Otherwise, just call the base class method
   return GmatCommand::Append(cmd);
}


//------------------------------------------------------------------------------
// bool Insert(GmatCommand *cmd, GmatCommand *prev)
//------------------------------------------------------------------------------
/**
 * Inserts a command into the mission sequence.
 *
 * Inserts commands into the command stream immediately after another command,
 * and updates the command list accordingly.
 *
 * @param <cmd>  The command that is added.
 * @param <prev> The command preceding the added one.
 *
 * @return true on success, false on failure.
 */
//------------------------------------------------------------------------------
bool BranchCommand::Insert(GmatCommand *cmd, GmatCommand *prev)
{
   #ifdef DEBUG_BRANCHCOMMAND_INSERT
   ShowCommand(wxT("BranchCommand::"), wxT("Insert() this = "), this, wxT(" next = "), next);
   ShowCommand(wxT("BranchCommand::"), wxT("Insert() cmd  = "), cmd, wxT(" prev = "), prev);
   #endif
   
   GmatCommand *currentOne   = NULL;
   GmatCommand *toShift      = NULL;
   bool        newBranch     = false;
   Integer     brNum         = -1;
   bool        foundHere     = false;
   bool        hereOrNested  = false;
   
   // if we're adding a new Else or ElseIf, we will need to add a branch 
   if ( (this->GetTypeName() == wxT("If")) && 
        ( (cmd->GetTypeName() == wxT("Else")) || 
          (cmd->GetTypeName() == wxT("ElseIf")) ) )  newBranch = true;
   
   
   // See if we're supposed to put it at the top of the first branch
   if (prev == this)
   {
      
      currentOne = branch[0];
      branch[0] = cmd;
      if (newBranch)
      {
         toShift = currentOne;
         brNum   = 0;
      }
      
      #ifdef DEBUG_BRANCHCOMMAND_INSERT
      ShowCommand(wxT("BranchCommand::"), wxT("currentOne = "), currentOne, wxT(" branch[0] = "), branch[0]);
      #endif
      
      cmd->Append(currentOne);
      foundHere = true;
      
      #ifdef DEBUG_BRANCHCOMMAND_INSERT
      ShowCommand(wxT("BranchCommand::"), wxT("currentOne->GetPrevious() = "), currentOne->GetPrevious());
      ShowCommand(wxT("BranchCommand::"), wxT("currentOne->GetNext() = "), currentOne->GetNext());
      #endif
      
      //return true;
   }
   // see if we're supposed to add it to the front of a branch
   // i.e. the prev = the last command in the previous branch
   // (e.g. an Else command)
   // check all but the last branch - End*** should take care of it
   // at that point
   if (!foundHere)
   {
      for (Integer br = 0; br < (Integer) (branch.size() - 1); ++br)
      {
         currentOne = branch.at(br);
         if (currentOne != NULL)
         {
            while (currentOne->GetNext() != this)
               currentOne = currentOne->GetNext();
            if (currentOne == prev) 
            {
               if (newBranch)
               {
                  toShift = currentOne;
                  brNum   = br;
               }
               AddToFrontOfBranch(cmd,br+1);
               foundHere = true;
               //return true;
            }
         }
      }
   }
   // If we have branches, try to insert there first
   if (!foundHere)
   {
      GmatCommand *nc = NULL;
      for (Integer which = 0; which < (Integer)branch.size(); ++which)
      {
         currentOne = branch[which];
         if (currentOne != NULL)
         {
            nc = currentOne;
            while((nc != this) && !foundHere)
            {
                // let a nested If handle it, if it is supposed to go in there
               if (newBranch && (nc == prev) && (nc->GetTypeName() != wxT("If")))
               {
                  toShift   = nc->GetNext();
                  brNum     = which;
                  foundHere = true;
               }
               nc = nc->GetNext();
            }
            //if (inNested = currentOne->Insert(cmd, prev))
            //   return true;
            hereOrNested = currentOne->Insert(cmd, prev);
            // check to see if it got added after the nested command
            if (hereOrNested && (currentOne->GetNext() == cmd))
            {
               toShift   = cmd->GetNext();
               brNum     = which;
               foundHere = true;
            }
         }
      }
   }
   
   #ifdef DEBUG_BRANCHCOMMAND_INSERT
   MessageInterface::ShowMessage
      (wxT("BranchCommand::Insert() newBranch=%d, foundHere=%d, toShift=%p\n"),
       newBranch, foundHere, toShift);
   #endif
   
   if (newBranch && foundHere && (toShift != NULL))
   {
      // make sure the new Else or ElseIf points back to the If command
      
      #ifdef DEBUG_BRANCHCOMMAND_INSERT
      ShowCommand(wxT("BranchCommand::"), wxT(" Setting next of "), cmd, wxT(" to "), this);
      #endif
      
      cmd->ForceSetNext(this);
      SetPreviousCommand(this, cmd, false);
      
      // shift all the later comamnds down one branch
      bool isOK = ShiftBranches(toShift, brNum);
      if (!isOK) 
         MessageInterface::ShowMessage
            (wxT("In BranchCommand::Insert - error adding Else/ElseIf"));
   }
   
   if (foundHere || hereOrNested) return true;

   // Otherwise, just call the base class method
   return GmatCommand::Insert(cmd, prev);
} // Insert()


//------------------------------------------------------------------------------
// GmatCommand* Remove(GmatCommand *cmd)
//------------------------------------------------------------------------------
/**
 * Removes a command from the mission sequence.
 *
 * @param <cmd>  The command that is to be removed.
 *
 * @return the removed command.
 */
//------------------------------------------------------------------------------
GmatCommand* BranchCommand::Remove(GmatCommand *cmd)
{
   #ifdef DEBUG_BRANCHCOMMAND_REMOVE
   ShowCommand(wxT("BranchCommand::"), wxT("Remove() this = "), this, wxT(" cmd = "), cmd);
   ShowCommand(wxT("BranchCommand::"), wxT("Remove() next = "), next);
   #endif
   
   if (next == cmd)
      return GmatCommand::Remove(cmd);    // Use base method to remove cmd
   
   if (cmd == this)
      return GmatCommand::Remove(cmd);    // Use base method to remove cmd
   
   GmatCommand *fromBranch = NULL;
   GmatCommand *current = NULL;
   GmatCommand *tempNext = NULL;
   
   // If we have branches, try to remove there first
   for (Integer which = 0; which < (Integer)branch.size(); ++which)
   {
      current = branch[which];
      
      if (current != NULL)
      {
         tempNext = current->GetNext();
      
         #ifdef DEBUG_BRANCHCOMMAND_REMOVE
         ShowCommand(wxT("BranchCommand::"), wxT("Remove() current = "), current, wxT(", tempNext = "), tempNext);
         #endif
         
         fromBranch = current->Remove(cmd);
         
         #ifdef DEBUG_BRANCHCOMMAND_REMOVE
         ShowCommand(wxT("BranchCommand::"), wxT("Remove() fromBranch = "), fromBranch);
         #endif
         
         if (fromBranch == current)
            branch[which] = tempNext;
         
         if (fromBranch != NULL)
         {
            // set previous command
            #ifdef DEBUG_BRANCHCOMMAND_REMOVE
            ShowCommand(wxT("BranchCommand::"), _GT" Setting previous of "), tempNext,
                        wxT(" to "), fromBranch->GetPrevious());
            #endif
            
            tempNext->ForceSetPrevious(fromBranch->GetPrevious());
            
            #ifdef DEBUG_BRANCHCOMMAND_REMOVE
            MessageInterface::ShowMessage(wxT("   Returning fromBranch\n"));
            #endif
            
            return fromBranch;
         }
      }
   }
   
   // Not in the branches, so continue with the sequence
   #ifdef DEBUG_BRANCHCOMMAND_REMOVE
   MessageInterface::ShowMessage
      (wxT("   Not in the branches, so continue with the sequence\n"));
   #endif
   
   return GmatCommand::Remove(cmd);
} // Remove()

//------------------------------------------------------------------------------
//  void BuildCommandSummaryString(bool commandCompleted)
//------------------------------------------------------------------------------
/**
 * Generates the summary string for a command by building the summary string for
 * this command, and then tacking on the summary for the branch commans\ds.
 *
 *
 * @param <commandCompleted> has the command completed execution
 */
//------------------------------------------------------------------------------
void BranchCommand::BuildCommandSummaryString(bool commandCompleted)
{
   #ifdef DEBUG_BRANCHCOMMAND_SUMMARY
      MessageInterface::ShowMessage(wxT("Entering BranchCommand::BuildMissionSummaryString for command %s of type %s\n"),
            summaryName.c_str(), typeName.c_str());
   #endif

   GmatCommand::BuildCommandSummaryString(commandCompleted);
   if (summaryForEntireMission)
   {
      wxString branchSummary = commandSummary;
      GmatCommand *current = NULL;

      // Build Command Summary string for all of the branch nodes
      for (Integer which = 0; which < (Integer)branch.size(); ++which)
      {
         current = branch[which];
         while ((current != NULL) && (current != this))
         {
            current->SetupSummary(summaryCoordSysName, summaryForEntireMission, missionPhysicsBasedOnly);
            #ifdef DEBUG_BRANCHCOMMAND_SUMMARY
               MessageInterface::ShowMessage(wxT("About to call for MissionSummary for command %s of type %s\n"),
                     (current->GetSummaryName()).c_str(), (current->GetTypeName()).c_str());
            #endif
            branchSummary += current->GetStringParameter(wxT("Summary"));
   //         missionSummary += current->GetStringParameter("MissionSummary");
            current = current->GetNext();
         }
      }

      commandSummary = branchSummary;
   }
}

//------------------------------------------------------------------------------
// bool InsertRightAfter(GmatCommand *cmd)
//------------------------------------------------------------------------------
/**
 * Inserts a command into the mission sequence right after this one.
 *
 * @param <cmd>  The command that is inserted.
 *
 * @return true on success, false on failure.
 */
//------------------------------------------------------------------------------
bool BranchCommand::InsertRightAfter(GmatCommand *cmd)
{
   if (!next) 
   {
      next = cmd;
      return true;
   }
   
   return GmatCommand::Insert(cmd, this);
}


//------------------------------------------------------------------------------
// void BranchCommand::SetSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 * Sets the solar system pointer for the child commands.
 *
 * @param <ss>  The SolarSystem instance.
 */
//------------------------------------------------------------------------------
void BranchCommand::SetSolarSystem(SolarSystem *ss)
{
   GmatCommand::SetSolarSystem(ss);
   GmatCommand *current = NULL;
   
   // Set it for all of the branch nodes
   // If we have branches, try to insert there first
   for (Integer which = 0; which < (Integer)branch.size(); ++which)
   {
      current = branch[which];
      while ((current != NULL) && (current != this))
      {
         current->SetSolarSystem(ss);;
         current = current->GetNext();
      }
   }
}

//------------------------------------------------------------------------------
// void BranchCommand::SetInternalCoordSystem(CoordinateSystem *cs)
//------------------------------------------------------------------------------
/**
 * Sets the internal coordinate system pointer for the child commands.
 *
 * @param <ss>  The CoordinateSystem instance.
 */
//------------------------------------------------------------------------------
void BranchCommand::SetInternalCoordSystem(CoordinateSystem *cs)
{
   GmatCommand::SetInternalCoordSystem(cs);
   GmatCommand *current = NULL;

   // Set it for all of the branch nodes
   // If we have branches, try to insert there first
   for (Integer which = 0; which < (Integer)branch.size(); ++which)
   {
      current = branch[which];
      while ((current != NULL) && (current != this))
      {
         current->SetInternalCoordSystem(cs);;
         current = current->GetNext();
      }
   }
}


//------------------------------------------------------------------------------
//  void SetObjectMap(std::map<wxString, Asset *> *map)
//------------------------------------------------------------------------------
/**
 * Called by the Sandbox or Function to set the local asset store used by the Command.  This
 * implementation chains through the branches and sets the asset map for each of
 * the branch nodes.
 * 
 * @param <map> Pointer to the local asset map
 */
//------------------------------------------------------------------------------
void BranchCommand::SetObjectMap(std::map<wxString, GmatBase *> *map)
{
   GmatCommand::SetObjectMap(map);
   GmatCommand *current = NULL;
   
   // Set it for all of the branch nodes
   // If we have branches, try to insert there first
   for (Integer which = 0; which < (Integer)branch.size(); ++which)
   {
      current = branch[which];
      while ((current != NULL) && (current != this))
      {
         current->SetObjectMap(map);
         current = current->GetNext();
      }
   }
}

//------------------------------------------------------------------------------
//  void SetGlobalObjectMap(std::map<wxString, Asset *> *map)
//------------------------------------------------------------------------------
/**
 * Called by the Sandbox or Function to set the global asset store used by the Command.  This
 * implementation chains through the branches and sets the asset map for each of
 * the branch nodes.
 * 
 * @param <map> Pointer to the local asset map
 */
//------------------------------------------------------------------------------
void BranchCommand::SetGlobalObjectMap(std::map<wxString, GmatBase *> *map)
{
   GmatCommand::SetGlobalObjectMap(map);
   GmatCommand *current = NULL;
   
   // Set it for all of the branch nodes
   // If we have branches, try to insert there first
   for (Integer which = 0; which < (Integer)branch.size(); ++which)
   {
      current = branch[which];
      while ((current != NULL) && (current != this))
      {
         current->SetGlobalObjectMap(map);
         current = current->GetNext();
      }
   }
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
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
bool BranchCommand::RenameRefObject(const Gmat::ObjectType type,
                                    const wxString &oldName,
                                    const wxString &newName)
{
   std::vector<GmatCommand*>::iterator node;
   for (node = branch.begin(); node != branch.end(); ++node)
   {
      current = *node;
      if (current != NULL)
      {
         #ifdef DEBUG_RENAME
         ShowCommand(wxT("BranchCommand::"), wxT("RenameRefObject() current = "), current);
         #endif
         
         current->RenameRefObject(type, oldName, newName);
         
         while (current->GetNext() != this)
         {
            current = current->GetNext();
            if (current == NULL)
               break;
            
            #ifdef DEBUG_RENAME
            ShowCommand(wxT("BranchCommand::"), wxT("RenameRefObject() current = "), current);
            #endif
            
            current->RenameRefObject(type, oldName, newName);            
         }
      }
      else
         break;
   }
   
   return true;
}


//------------------------------------------------------------------------------
//  const wxString GetGeneratingString()
//------------------------------------------------------------------------------
/**
 * Method used to retrieve the string that was parsed to build this GmatCommand.
 *
 * @param <mode>    Specifies the type of serialization requested (Not yet used
 *                  in commands).
 * @param <prefix>  Optional prefix appended to the object's name (Not yet used
 *                  in commands).
 * @param <useName> Name that replaces the object's name (Not yet used
 *                  in commands).
 *
 * @return The script line that defines this GmatCommand.
 */
//------------------------------------------------------------------------------
const wxString& BranchCommand::GetGeneratingString(Gmat::WriteMode mode,
                                                  const wxString &prefix,
                                                  const wxString &useName)
{
   fullString = prefix + generatingString;
   InsertCommandName(fullString);
   
   // We don't want BranchCommand to indent
   UnsignedInt prefixSize = prefix.size();   
   if (this->IsOfType(wxT("BranchCommand")) && prefix != wxT(""))
      fullString = fullString.substr(prefixSize);
   
   #ifdef DEBUG_BRANCHCOMMAND_GEN_STRING
   ShowCommand(wxT("BranchCommand::"), wxT("GetGeneratingString() this = "), this);
   MessageInterface::ShowMessage
      (wxT("   mode='%d', prefix='%s', useName='%s'\n"), mode, prefix.c_str(),
       useName.c_str());
   MessageInterface::ShowMessage(wxT("   fullString = '%s'\n"), fullString.c_str());
   #endif
   
   wxString indent = wxT("   ");
   
   wxString commentLine = GetCommentLine();
   wxString inlineComment = GetInlineComment();
   
   #ifdef DEBUG_BRANCHCOMMAND_GEN_STRING
   ShowCommand(wxT("BranchCommand::"), wxT("GmatCommand::GetGeneratingString() this = "), this);
   MessageInterface::ShowMessage
      (wxT("===> commentLine=<%s>, inlineComment=<%s>\n"),
       commentLine.c_str(), inlineComment.c_str());
   #endif
   
   // Handle multiple line comments, we want to indent all lines.
   if (commentLine != wxT(""))
   {
      wxString gen;
      TextParser tp;
      StringArray textArray = tp.DecomposeBlock(commentLine);
      
      // handle multiple comment lines
      for (UnsignedInt i=0; i<textArray.size(); i++)
      {
         gen << prefix << textArray[i];
         if (textArray[i].find(wxT("\n")) == commentLine.npos &&
             textArray[i].find(wxT("\r")) == commentLine.npos)
            gen << wxT("\n");
      }
      
      fullString = gen + fullString;
   }
   
   // Handle inline comment
   if (inlineComment != wxT(""))
      fullString = fullString + inlineComment;
   
   GmatCommand *current;
   wxString newPrefix = indent + prefix;   
   bool inTextMode = false;
   Integer scriptEventCount = 0;
   
   // Loop through the branches, appending the strings from commands in each
   for (Integer which = 0; which < (Integer)branch.size(); ++which)
   {
      current = branch[which];
      
      while ((current != NULL) && (current != this))
      {
         #ifdef DEBUG_BRANCHCOMMAND_GEN_STRING
         ShowCommand(wxT("BranchCommand::"), wxT("GetGeneratingString() current = "), current);
         MessageInterface::ShowMessage(wxT("   inTextMode=%d\n"), inTextMode);
         #endif
         
         // BeginScript writes its own children, so write if not in TextMode.
         // EndScript is written from BeginScript
         if (!inTextMode)
         {
            fullString += wxT("\n");
            if (current->GetNext() != this)
               fullString += current->GetGeneratingString(mode, newPrefix, useName);
            else // current is the End command for this branch command
               fullString += current->GetGeneratingString(mode, prefix, useName);
         }
         
         if (current->GetTypeName() == wxT("BeginScript"))
            scriptEventCount++;
         
         if (current->GetTypeName() == wxT("EndScript"))
            scriptEventCount--;
         
         inTextMode = (scriptEventCount == 0) ? false : true;
         
         current = current->GetNext();
      }
   }
   
   #ifdef DEBUG_BRANCHCOMMAND_GEN_STRING
   MessageInterface::ShowMessage
      (wxT("%s::GetGeneratingString() return fullString = \n<%s>\n"),
       this->GetTypeName().c_str(), fullString.c_str());
   #endif
   
   return fullString;
} // GetGeneratingString()


//------------------------------------------------------------------------------
// bool TakeAction(const wxString &action, const wxString &actionData)
//------------------------------------------------------------------------------
/**
 * Interface used to support user actions.
 *
 * @param <action> The string descriptor for the requested action.
 * @param <actionData> Optional data used for the action.
 *
 * @return true if the action was performed, false if not.
 */
//------------------------------------------------------------------------------
bool BranchCommand::TakeAction(const wxString &action,
                           const wxString &actionData)
{
   #ifdef DEBUG_BRANCHCOMMAND_ACTIONS
      MessageInterface::ShowMessage(wxT("%s: Executing the action \"%s\"\n"), 
         GetGeneratingString().c_str(), action.c_str());
   #endif
      
   if (action == wxT("ResetLoopData"))
   {
      GmatCommand *cmd; 
      for (std::vector<GmatCommand *>::iterator br = branch.begin(); 
           br != branch.end(); ++br)
      {
         cmd = *br;
         while (cmd != this)
         {
            #ifdef DEBUG_BRANCHCOMMAND_ACTIONS
               MessageInterface::ShowMessage(wxT("   Applying action to \"%s\"\n"), 
                  cmd->GetGeneratingString().c_str());
            #endif
               
            if ((cmd->GetTypeName() == wxT("Propagate")) || 
                (cmd->IsOfType(wxT("BranchCommand"))))
               cmd->TakeAction(wxT("ResetLoopData"));
            cmd = cmd->GetNext();
         }
      }      
      return true;
   }
   
   return GmatCommand::TakeAction(action, actionData);
}


//------------------------------------------------------------------------------
// bool Execute()
//------------------------------------------------------------------------------
/**
 * Executes the command.
 *
 * @return true if the Command runs to completion, false if an error
 *         occurs.
 */
//------------------------------------------------------------------------------
bool BranchCommand::Execute()
{
   commandExecuting = true;
   return true;
}


//------------------------------------------------------------------------------
// bool ExecuteBranch(Integer which)
//------------------------------------------------------------------------------
/**
 * Executes a specific branch.
 *
 * @param <which>  Specifies which branch is executed.
 *
 * @return true if the branch runs to completion, false if an error
 *         occurs.
 */
//------------------------------------------------------------------------------
bool BranchCommand::ExecuteBranch(Integer which)
{
   #ifdef DEBUG_BRANCHCOMMAND_EXECUTION
   MessageInterface::ShowMessage
      (wxT("In BranchCommand (%s) '%s', executing branch %d\n"), typeName.c_str(),
       GetGeneratingString(Gmat::NO_COMMENTS).c_str(), which);
   #endif
   bool retval = true;
   
   if (current == NULL)
      current = branch[which];
   
   #ifdef DEBUG_BRANCHCOMMAND_EXECUTION
   ShowCommand(wxT("In ExecuteBranch:"), wxT("current = "), current, wxT("this    = "), this);
   #endif
   
   if (current == this)
   {
      #ifdef DEBUG_BRANCHCOMMAND_EXECUTION
      MessageInterface::ShowMessage
         (wxT("In ExecuteBranch (%s) - current = this -> resetting\n"), 
          typeName.c_str());
      if (next)
         MessageInterface::ShowMessage
            (wxT("...... and the next one is : %s\n"), (next->GetTypeName()).c_str());
      else
         MessageInterface::ShowMessage(wxT("...... and the next one is NULL!!!\n"));
      #endif
      
      branchExecuting = false;
      //commandExecuting = false;  // ***********************
      //commandComplete  = true;   // ***********************
      current = NULL;
   }
   
   if (current != NULL)
   {
      #ifdef DEBUG_BRANCHCOMMAND_EXECUTION
      ShowCommand(wxT("   Calling in "), wxT("current = "), current);
      #endif
      
      try
      {
         // Save current command and set it after current command finished executing
         // incase for calling GmatFunction.
         GmatCommand *curcmd = current;
         if (current->Execute() == false)
            retval = false;
         
         current = curcmd;
         // check for user interruption here (loj: 2007.05.11 Added)
         if (GmatGlobal::Instance()->GetRunInterrupted())
            throw CommandException
               (wxT("Branch command \"") + generatingString + wxT("\" interrupted!"));
         
         // Check for NULL pointer here (loj: 2008.09.25 Added)
         // Why current pointer is reset to NULL running recursive function?
         // Is this an error or can it be ignored?
         // Without this change, Factorial_FR testing will not work.
         if (current == NULL)
         {
            #ifdef __THROW_EXCEPTION__            
            throw CommandException
               (wxT("Branch command \"") + generatingString + wxT("\" has NULL current pointer!"));
            #endif
         }
         
         if (current != NULL)
            current = current->GetNext();
         
         branchExecuting = true;
         // Set commandExecuting to true if branch is executing (LOJ: 2010.08.06)
         commandExecuting = true;
      }
      catch (BaseException &e)
      {
         // Use exception to remove Visual C++ warning
         e.GetMessageType();
         #ifdef DEBUG_BRANCHCOMMAND_EXECUTION
         MessageInterface::ShowMessage
            (wxT("   BranchCommand rethrowing %s\n"), e.GetFullMessage().c_str());
         #endif
         
         throw;
      }
   }
   
   #ifdef DEBUG_BRANCHCOMMAND_EXECUTION
   ShowCommand(wxT("Exiting ExecuteBranch:"), wxT("current = "), current, wxT("this    = "), this);
   MessageInterface::ShowMessage
      (wxT("   branchExecuting=%d, commandExecuting=%d, commandComplete=%d\n"),
       branchExecuting, commandExecuting, commandComplete);
   #endif
   
   return retval;
} // ExecuteBranch()


//------------------------------------------------------------------------------
//  void RunComplete()
//------------------------------------------------------------------------------
/**
 * Tells the sequence that the run was ended, possibly before reaching the end.
 *
 * BranchCommands clear the "current" pointer and call RunComplete on their
 * branches, ensuring that the command sequence has reset the branches to an
 * idle state.
 */
//------------------------------------------------------------------------------
void BranchCommand::RunComplete()
{
   #ifdef DEBUG_RUN_COMPLETE
   ShowCommand(wxT("BranchCommand::"), wxT("BranchCommand::RunComplete() this = "), this);
   #endif
   
   current = NULL;
   
   for (std::vector <GmatCommand *>::iterator i = branch.begin(); i != branch.end(); ++i)
      if (*i != NULL)
         if (!(*i)->IsOfType(wxT("BranchEnd")))
            (*i)->RunComplete();
   
   
   // This block of code causes unnecesary looping since next is handled in
   // the GmatCommand::RunComplete() (loj: 2/16/07)
   //if (next)
   //   if (!next->IsOfType("BranchEnd"))
   //      next->RunComplete();
   
   
   commandComplete = false;
   commandExecuting = false;
   branchExecuting = false;

   GmatCommand::RunComplete();
}


//------------------------------------------------------------------------------
// protected methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  bool ShiftBranches(GmatCommand *startWith, Integer ofBranchNumber)
//------------------------------------------------------------------------------
/**
 * Shifts command in the branches down one branch, starting with command 
 * startWith in branch ofBranchNumber.
 *
 * @param startingWith   first command to shift down one branch
 * @param ofBranchNumber branch where the command is located
 *
 * @return success of the operation
 *
 */
//------------------------------------------------------------------------------
bool  
BranchCommand::ShiftBranches(GmatCommand *startWith, Integer ofBranchNumber)
{   
   Integer brSz = (Integer) branch.size();
   // add another branch at the end
   branch.push_back(NULL);
   for (Integer i = brSz-1; i > ofBranchNumber; i--)
   {
      branch.at(i+1) = branch.at(i);
   }
   branch.at(ofBranchNumber+1) = startWith;
   return true;
}


//------------------------------------------------------------------------------
// void SetPreviousCommand(GmatCommand *cmd, GmatCommand *prev,
//                         bool skipBranchEnd)
//------------------------------------------------------------------------------
/*
 * Sets previous command of the command.
 *
 * @param  cmd   The command of which previous command to be set
 * @param  prev  The previous command to set to command
 * @param  skipBranchEnd  If true, it sets without checking for BranchEnd
 *
 */
//------------------------------------------------------------------------------
void BranchCommand::SetPreviousCommand(GmatCommand *cmd, GmatCommand *prev,
                                       bool skipBranchEnd)
{
   #ifdef DEBUG_BRANCHCOMMAND_PREV_CMD
   ShowCommand
      (wxT("BranchCommand::SetPreviousCommand()"), wxT(" cmd = "), cmd, wxT(" prev = "), prev);
   #endif
   
   if (skipBranchEnd && cmd->IsOfType(wxT("BranchEnd")))
   {
      #ifdef DEBUG_BRANCHCOMMAND_PREV_CMD
      MessageInterface::ShowMessage
         (wxT("   cmd is of type \"BranchEnd\", so previous is not set\n"));
      ShowCommand(wxT("   "), wxT(" previous of "), cmd, wxT(" is "), cmd->GetPrevious());
      #endif
      
      if (cmd->GetPrevious() == NULL)
         cmd->ForceSetPrevious(prev);
   }
   else
   {
      #ifdef DEBUG_BRANCHCOMMAND_PREV_CMD
      ShowCommand(wxT("BranchCommand::"), wxT(" Setting previous of "), cmd, wxT(" to "), prev);
      #endif
      
      cmd->ForceSetPrevious(prev);
   }
}

//------------------------------------------------------------------------------
// const std::vector<GmatCommand*> GetCommandsWithGmatFunctions()
//------------------------------------------------------------------------------
const std::vector<GmatCommand*> BranchCommand::GetCommandsWithGmatFunctions()
{
   #ifdef DEBUG_BRANCHCOMMAND_GMATFUNCTIONS
      MessageInterface::ShowMessage(wxT("Entering BranchCommand::GetCommandsWithGmatFunctions\n"));
   #endif
   cmdsWithFunctions.clear();
   std::vector<GmatCommand*> tmpArray;
   
   GmatCommand *brCmd, *subCmd;;
   for (unsigned int ii = 0; ii < branch.size(); ii++)
   {
      brCmd = branch.at(ii);
      subCmd = brCmd;
      while ((subCmd != NULL) && (subCmd != this))
      {
         #ifdef DEBUG_BRANCHCOMMAND_GMATFUNCTIONS
            MessageInterface::ShowMessage(
                  wxT("--- checking a Command of type %s with name %s\n"),
                  (subCmd->GetTypeName()).c_str(), (subCmd->GetName()).c_str());
         #endif
         tmpArray.clear();
         if(subCmd->IsOfType(wxT("BranchCommand")))  
         {
            tmpArray = ((BranchCommand*)subCmd)->GetCommandsWithGmatFunctions();
         }
         else if ((subCmd->IsOfType(wxT("CallFunction"))) ||
                  (subCmd->IsOfType(wxT("Assignment"))))
         {
            #ifdef DEBUG_BRANCHCOMMAND_GMATFUNCTIONS
               MessageInterface::ShowMessage(
                     wxT("--- ADDING a Command of type %s with name %s to the list\n"),
                     (subCmd->GetTypeName()).c_str(), (subCmd->GetName()).c_str());
            #endif
            tmpArray.push_back(subCmd);
         }
         for (unsigned int jj= 0; jj < tmpArray.size(); jj++)
            cmdsWithFunctions.push_back(tmpArray.at(jj));
         subCmd = subCmd->GetNext();
      }
   }
   return cmdsWithFunctions;
}

//------------------------------------------------------------------------------
// bool HasAFunction()
//------------------------------------------------------------------------------
bool BranchCommand::HasAFunction()
{
   #ifdef DEBUG_IS_FUNCTION
      if (!current)
         MessageInterface::ShowMessage(wxT("In HasAFunction and current is NULL\n"));
      else
         MessageInterface::ShowMessage(wxT("In HasAFunction and current is of type %s\n"),
               (current->GetTypeName()).c_str());
   #endif
   std::vector<GmatCommand*>::iterator node;
   GmatCommand *currentPtr;
   
   for (node = branch.begin(); node != branch.end(); ++node)
   {
      currentPtr = *node;
      while (currentPtr != this)
      {
         // if some command in the branch has a function, return true
         if (currentPtr->HasAFunction()) return true;
         currentPtr = currentPtr->GetNext();
         if (currentPtr == NULL)
            throw CommandException(wxT("Branch command \"") + generatingString +
                                   wxT("\" was not terminated!"));
      }
   }
   // otherwise, there are no GmatFunctions in this Branch Command
   return false;
}

//------------------------------------------------------------------------------
// void SetCallingFunction();
//------------------------------------------------------------------------------
void BranchCommand::SetCallingFunction(FunctionManager *fm)
{
   
   GmatCommand::SetCallingFunction(fm);
   
   std::vector<GmatCommand*>::iterator node;
   GmatCommand *currentPtr;
   
   for (node = branch.begin(); node != branch.end(); ++node)
   {
      currentPtr = *node;
      while (currentPtr != this)
      {
         currentPtr->SetCallingFunction(fm);
         currentPtr = currentPtr->GetNext();
         if (currentPtr == NULL)
            throw CommandException(wxT("Branch command \"") + generatingString +
                                   wxT("\" was not terminated!"));
      }
   }
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
bool BranchCommand::IsExecuting()
{
   return branchExecuting;
}


//------------------------------------------------------------------------------
// Integer GetCloneCount()
//------------------------------------------------------------------------------
/**
 * Determines how many clones are available in the branch command
 *
 * @return The number of clones
 */
//------------------------------------------------------------------------------
Integer BranchCommand::GetCloneCount()
{
   cloneCount = 0;

   // Count 'em up from the branch control sequence(s)
   std::vector<GmatCommand*>::iterator node;
   GmatCommand *currentPtr;

   for (node = branch.begin(); node != branch.end(); ++node)
   {
      currentPtr = *node;
      while (currentPtr != this)
      {
         cloneCount += currentPtr->GetCloneCount();
         currentPtr = currentPtr->GetNext();
         if (currentPtr == NULL)
            throw CommandException(wxT("Branch command \"") + generatingString +
                                   wxT("\" was not terminated!"));
      }
   }

   #ifdef DEBUG_CLONE_UPDATES
      MessageInterface::ShowMessage(wxT("CloneCount for branch command %s = %d\n"),
            typeName.c_str(), cloneCount);
   #endif

   return cloneCount;
}


//------------------------------------------------------------------------------
// GmatBase* GetClone(Integer cloneIndex)
//------------------------------------------------------------------------------
/**
 * Retrieves a clone pointer from the branch command
 *
 * @param cloneIndex Index to the desired clone
 *
 * @return The pointer to the clone
 */
//------------------------------------------------------------------------------
GmatBase* BranchCommand::GetClone(Integer cloneIndex)
{
   GmatBase *retptr = NULL;
   Integer currentCount = 0, nodeCount;
   std::vector<GmatCommand*>::iterator node;
   GmatCommand *currentPtr;

   if ((cloneIndex < cloneCount) && (cloneIndex >= 0))
   {
      for (node = branch.begin(); node != branch.end(); ++node)
      {
         currentPtr = *node;
         while (currentPtr != this)
         {
            nodeCount = currentPtr->GetCloneCount();

            if (cloneIndex < currentCount + nodeCount)
            {
               retptr = currentPtr->GetClone(cloneIndex - currentCount);
               break;
            }

            currentCount += nodeCount;
            currentPtr = currentPtr->GetNext();
            if (currentPtr == NULL)
               throw CommandException(wxT("Branch command \"") + generatingString +
                                      wxT("\" was not terminated!"));
         }
      }
   }

   return retptr;
}
