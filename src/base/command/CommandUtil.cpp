//$Id: CommandUtil.cpp 10034 2011-12-02 21:44:39Z lindajun $
//------------------------------------------------------------------------------
//                                 CommandUtil
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Linda Jun
// Created: 2005/11/28
//
/**
 * This file provides methods to get whole mission sequence string.
 */
//------------------------------------------------------------------------------

#include "CommandUtil.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_MATCHING_END
//#define DEBUG_GET_PARENT
//#define DEBUG_GET_FIRST
//#define DEBUG_COMMAND_SEQ_STRING
//#define DEBUG_COMMAND_FIND_OBJECT
//#define DEBUG_COMMAND_DELETE
//#define DEBUG_SEQUENCE_CLEARING

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//------------------------------------------------------------------------------
// GmatCommand* GetFirstCommand(GmatCommand *cmd)
//------------------------------------------------------------------------------
/*
 * Returns first command in the command sequence
 *
 * @param  cmd  Command which search begins from
 */
//------------------------------------------------------------------------------
GmatCommand* GmatCommandUtil::GetFirstCommand(GmatCommand *cmd)
{
   #ifdef DEBUG_GET_FIRST
   ShowCommand(wxT("GmatCommandUtil::GetFirstCommand() entered, cmd = "), cmd);
   #endif
   
   GmatCommand *prevCmd = cmd;
   
   while (cmd != NULL)
   {
      cmd = cmd->GetPrevious();
      
      #ifdef DEBUG_GET_FIRST
      ShowCommand(wxT("   previous command = "), cmd);
      #endif
      
      if (cmd != NULL)
         prevCmd = cmd;
   }
   
   return prevCmd;
}


//------------------------------------------------------------------------------
// GmatCommand* GetLastCommand(GmatCommand *cmd)
//------------------------------------------------------------------------------
/*
 * Returns last command in the command sequence
 *
 * @param  cmd  Command which search begins from
 */
//------------------------------------------------------------------------------
GmatCommand* GmatCommandUtil::GetLastCommand(GmatCommand *cmd)
{
   GmatCommand *nextCmd = cmd;
   
   while (cmd != NULL)
   {
      #ifdef DEBUG_GET_LAST
      ShowCommand(wxT("===> GmatCommandUtil::GetLastCommand() cmd = "), cmd);
      #endif
      
      cmd = cmd->GetNext();
      
      if (cmd != NULL)
         nextCmd = cmd;
   }
   
   return nextCmd;
}


//------------------------------------------------------------------------------
// GmatCommand* GetNextCommand(GmatCommand *cmd)
//------------------------------------------------------------------------------
/*
 * Returns next non-ScriptEvent command in the sequence. If command is BeginScript,
 * it will return next command of matching EndScrpt.
 *
 * @param  cmd  Command which search begins from
 * @return next non-ScriptEvent command
 */
//------------------------------------------------------------------------------
GmatCommand* GmatCommandUtil::GetNextCommand(GmatCommand *cmd)
{
   if (cmd == NULL)
      return NULL;
   
   if (cmd->GetTypeName() != wxT("BeginScript"))
      return cmd->GetNext();
   
   GmatCommand *endScript = GetMatchingEnd(cmd);
   
   if (endScript == NULL)
      return NULL;
   else
      return endScript->GetNext();
   
}


//------------------------------------------------------------------------------
// GmatCommand* GetPreviousCommand(GmatCommand *from, GmatCommand *cmd)
//------------------------------------------------------------------------------
/*
 * Searchs command from the "from" command and returns previous command.
 *
 * @param from The command to search from
 * @return Returns previous command or NULL if command not found
 */
//------------------------------------------------------------------------------
GmatCommand* GmatCommandUtil::GetPreviousCommand(GmatCommand *from,
                                                 GmatCommand *cmd)
{
   GmatCommand *current = from;
   GmatCommand *prevCmd = NULL;
   GmatCommand *child = NULL;
   Integer branch = 0;
   
   while (current != NULL)
   {
      if (current == cmd)
         return prevCmd;
      
      // check branch commands
      while ((current->GetChildCommand(branch)) != NULL)
      {
         child = current->GetChildCommand(branch);
         
         while (child != NULL)
         {
            #ifdef DEBUG_PREV_COMMAND
            ShowCommand(wxT("   child = "), child);
            #endif
            
            if (child == cmd)
               return prevCmd;
            
            prevCmd = child;
            child = child->GetNext();
         }
         
         branch++;
      }
      
      prevCmd = current;
      current = current->GetNext();
      
   }
   
   return NULL;
}


//------------------------------------------------------------------------------
// GmatCommand* GetMatchingEnd(GmatCommand *cmd)
//------------------------------------------------------------------------------
/*
 * Returns matching EndScript of BeginScrpt or matching EndBranch of BranchCommand
 *
 * @param  cmd  BeginScript command which search begins from
 * @return  Matching end command, NULL if matching end command not found
 */
//------------------------------------------------------------------------------
GmatCommand* GmatCommandUtil::GetMatchingEnd(GmatCommand *cmd)
{
   if (cmd == NULL)
      return NULL;
   
   #ifdef DEBUG_MATCHING_END
   ShowCommand
      (wxT("===> GmatCommandUtil::GetMatchingEnd() cmd = "), cmd);
   #endif
   
   if (cmd->GetTypeName() != wxT("BeginScript") && !cmd->IsOfType(wxT("BranchCommand")))
      return NULL;
   
   GmatCommand *current = cmd;
   
   if (cmd->GetTypeName() == wxT("BeginScript"))
   {
      Integer scriptEventCount = 0;
   
      while (current != NULL)
      {
         if (current->GetTypeName() == wxT("BeginScript"))
            scriptEventCount++;
      
         if (current->GetTypeName() == wxT("EndScript"))
            scriptEventCount--;

         #ifdef DEBUG_MATCHING_END
         MessageInterface::ShowMessage
            (wxT(wxT("     scriptEventCount=%d, current=<%p><%s>\n")), scriptEventCount, current,
             current->GetTypeName().c_str());
         #endif
      
         if (scriptEventCount == 0)
            break;
      
         current = current->GetNext();
      }
      
      #ifdef DEBUG_MATCHING_END
      ShowCommand(wxT("===> GmatCommandUtil::GetMatchingEnd() returning "), current);
      #endif
   
      return current;
   }
   else
   {
      GmatCommand *child = NULL;
      Integer branch = 0;
      bool elseFound = false;
      
      while ((current->GetChildCommand(branch)) != NULL)
      {
         child = current->GetChildCommand(branch);
         
         while (child != NULL)
         {
            #ifdef DEBUG_MATCHING_END
            ShowCommand(wxT("   child = "), child);
            #endif
            
            if (child->IsOfType(wxT("BranchEnd")))
            {
               if (child->GetTypeName() == wxT("Else"))
               {
                  elseFound = true;
                  branch++;
               }
               
               break;
            }
            
            child = child->GetNext();
         }
         
         if (elseFound && branch == 1)
         {
            elseFound = false;
            continue;
         }
         else
            break;
      }
      
      #ifdef DEBUG_MATCHING_END
      ShowCommand(wxT("===> GmatCommandUtil::GetMatchingEnd() returning "), child);
      #endif
      
      return child;
   }
}


//------------------------------------------------------------------------------
// GmatCommand* GetParentCommand(GmatCommand *top, GmatCommand *cmd)
//------------------------------------------------------------------------------
/**
 * Returns outer most parent command from the commans sequence string at top node.
 */
//------------------------------------------------------------------------------
GmatCommand* GmatCommandUtil::GetParentCommand(GmatCommand *top, GmatCommand *cmd)
{
   #ifdef DEBUG_GET_PARENT
   ShowCommand
      (wxT("===> GmatCommandUtil::GetParentCommand() top = "), top, wxT(", cmd = "), cmd);
   #endif
   
   GmatCommand *current = top;
   GmatCommand *parent = NULL;
   
   while (current != NULL)
   {
      #ifdef DEBUG_GET_PARENT
      ShowCommand(wxT("     current = "), current);
      #endif
      
      if (current == cmd)
      {
         parent = top;
         break;
      }
      
      if ((current->GetChildCommand(0)) != NULL)
      {
         parent = GetSubParent(current, cmd);
         
         #ifdef DEBUG_GET_PARENT
         ShowCommand(wxT("     parent = "), current);
         #endif
         
         if (parent != NULL)
            break;
      }
      
      current = current->GetNext();
   }
   
   #ifdef DEBUG_GET_PARENT
   ShowCommand(wxT("===> GmatCommandUtil::GetParentCommand() returning "), parent);
   #endif
   
   return parent;
}


//------------------------------------------------------------------------------
// GmatCommand* GetSubParent(GmatCommand *brCmd, GmatCommand *cmd)
//------------------------------------------------------------------------------
/**
 * Returns immediate parent command from the nested branch command.
 */
//------------------------------------------------------------------------------
GmatCommand* GmatCommandUtil::GetSubParent(GmatCommand *brCmd, GmatCommand *cmd)
{
   #ifdef DEBUG_GET_PARENT
   ShowCommand
      (wxT("     GmatCommandUtil::GetSubParent() brCmd = "), brCmd, wxT(", cmd = "), cmd);
   #endif
   
   GmatCommand *current = brCmd;
   GmatCommand *child;
   GmatCommand *subParent = NULL;
   Integer childNo = 0;
   
   while((child = current->GetChildCommand(childNo)) != NULL)
   {
      while ((child != NULL) && (child != current))
      {
         if (child == cmd)
         {
            #ifdef DEBUG_GET_PARENT
            ShowCommand
               (wxT("     GmatCommandUtil::GetSubParent() returning "), current);
            #endif
            return current;
         }
         
         if (child->GetChildCommand() != NULL)
            subParent = GetSubParent(child, cmd);
         
         if (subParent != NULL)
            return subParent;
         
         child = child->GetNext();
         
         #ifdef DEBUG_GET_PARENT
         ShowCommand
            (wxT("     GmatCommandUtil::GetSubParent() child = "), child);
         #endif
      }
      
      ++childNo;
   }
   
   #ifdef DEBUG_GET_PARENT
   MessageInterface::ShowMessage
      (wxT("     GmatCommandUtil::GetSubParent() returning NULL\n"));
   #endif
   
   return NULL;
}


//------------------------------------------------------------------------------
// GmatCommand* RemoveCommand(GmatCommand *cmd)
//------------------------------------------------------------------------------
/*
 * Removes a command from the command sequence. The caller has to delete the command.
 *
 * If deleting branch command,
 * it will remove and delete all children from the branch. If deleting ScriptEvent,
 * it will remove and delete all commands including EndScrpt between BeginScrint
 * and EndScript.
 *
 * @param  cmd  Command which search begins from
 * @return removed command
 */
//------------------------------------------------------------------------------
GmatCommand* GmatCommandUtil::RemoveCommand(GmatCommand *seq, GmatCommand *cmd)
{
   #ifdef DEBUG_COMMAND_DELETE
   ShowCommand(wxT("==========> CommandUtil::RemoveCommand() removing "), cmd,
               wxT(" from "), seq);
   #endif
   
   if (cmd == NULL)
      return NULL;
   
   GmatCommand *remvCmd;
   if (cmd->GetTypeName() != wxT("BeginScript"))
   {
      GmatCommand *remvCmd = seq->Remove(cmd);
      
      #ifdef DEBUG_COMMAND_DELETE
      ShowCommand(wxT("   Removed = "), remvCmd);
      #endif
      
      #ifdef DEBUG_COMMAND_DELETE
      ShowCommand(wxT("==========> CommandUtil::RemoveCommand() Returning "), remvCmd);
      #endif
      
      return remvCmd;
   }
   
   //-------------------------------------------------------
   // Remove commands inside Begin/EndScript block
   //-------------------------------------------------------

   // Check for previous command, it should not be NULL,
   // since "NoOp" is the first command
   
   GmatCommand *prevCmd = cmd->GetPrevious();
   if (prevCmd == NULL)
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_, wxT("CommandUtil::RemoveCommand() *** INTERNAL ERROR *** \n")
          wxT("The previous command cannot be NULL.\n"));
      return NULL;
   }
   
   ////GmatCommand *first = GetFirstCommand();
   GmatCommand *first = seq;
   
   #ifdef DEBUG_COMMAND_DELETE
   wxString cmdString1 = GmatCommandUtil::GetCommandSeqString(first);
   MessageInterface::ShowMessage(wxT("     ==> Current sequence:"));
   MessageInterface::ShowMessage(cmdString1);
   #endif
   
   GmatCommand *current = cmd->GetNext();
   
   #ifdef DEBUG_COMMAND_DELETE
   GmatCommand *nextCmd = GmatCommandUtil::GetNextCommand(cmd);
   ShowCommand(wxT("     prevCmd = "), prevCmd, wxT(" nextCmd = "), nextCmd);
   #endif
   
   // Get matching EndScript for BeginScript
   GmatCommand *endScript = GmatCommandUtil::GetMatchingEnd(cmd);
   
   #ifdef DEBUG_COMMAND_DELETE
   ShowCommand(wxT("     endScript = "), endScript);
   #endif
   
   GmatCommand* next;
   while (current != NULL)
   {
      #ifdef DEBUG_COMMAND_DELETE
      ShowCommand(wxT("     current = "), current);
      #endif
      
      if (current == endScript)
         break;
      
      next = current->GetNext();
      
      #ifdef DEBUG_COMMAND_DELETE
      ShowCommand(wxT("     removing and deleting "), current);
      #endif
      
      remvCmd = cmd->Remove(current);
      
      // per kw report - check remvCmd first
      if (remvCmd != NULL)
      {
         remvCmd->ForceSetNext(NULL);
         
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (remvCmd, remvCmd->GetTypeName(), wxT("CommandUtil::RemoveCommand()"));
         #endif
         delete remvCmd;
      }
      
      current = next;
   }
   
   //-------------------------------------------------------
   // Remove and delete EndScript
   //-------------------------------------------------------
   #ifdef DEBUG_COMMAND_DELETE
   ShowCommand(wxT("     removing and deleting "), current);
   #endif
   
   remvCmd = cmd->Remove(current);
   
   // per kw report - check remvCmd first
   if (remvCmd != NULL)
   {
      remvCmd->ForceSetNext(NULL);
      
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (remvCmd, remvCmd->GetTypeName(), wxT("CommandUtil::RemoveCommand()"));
      #endif
      delete remvCmd;
      remvCmd = NULL;
   }
   
   next = cmd->GetNext();
   
   #ifdef DEBUG_COMMAND_DELETE
   ShowCommand(wxT("     next    = "), next, wxT(" nextCmd = "), nextCmd);
   #endif
   
   //-------------------------------------------------------
   // Remove and delete BeginScript
   //-------------------------------------------------------
   #ifdef DEBUG_COMMAND_DELETE
   ShowCommand(wxT("     removing and deleting "), cmd);
   #endif
   
   // Remove BeginScript
   remvCmd = first->Remove(cmd);
   
   // Set next command NULL
   cmd->ForceSetNext(NULL);
   if (cmd != NULL)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (cmd, cmd->GetTypeName(), wxT("CommandUtil::RemoveCommand()"));
      #endif
      delete cmd;
      cmd = NULL;
   }
   
   #ifdef DEBUG_COMMAND_DELETE
   wxString cmdString2 = GmatCommandUtil::GetCommandSeqString(first);
   MessageInterface::ShowMessage(wxT("     ==> sequence after delete:"));
   MessageInterface::ShowMessage(cmdString2);
   ShowCommand(wxT("==========> CommandUtil::RemoveCommand() Returning cmd = "), cmd);
   #endif
   
   // Just return cmd, it should be deleted by the caller.
   return cmd;
}


//------------------------------------------------------------------------------
// bool IsElseFoundInIf(GmatCommand *ifCmd)
//------------------------------------------------------------------------------
/**
 * Checks if Else command exist in the first children of If branch command.
 *
 * @return true if Else command is found in the If command, flase otherwise
 */
//------------------------------------------------------------------------------
bool GmatCommandUtil::IsElseFoundInIf(GmatCommand *ifCmd)
{
   if (ifCmd == NULL)
      return false;
   
   #ifdef DEBUG_IF_ELSE
   ShowCommand
      (wxT("===> GmatCommandUtil::IsElseFoundInIf() ifCmd = "), ifCmd);
   #endif
   
   if (!ifCmd->IsOfType(wxT("If")))
   {
      #ifdef DEBUG_IF_ELSE
      MessageInterface::ShowMessage
         (wxT("IsElseFoundInIf() returning false, it is not If command\n"));
      #endif
      return false;
   }
   
   GmatCommand *current = ifCmd;
   GmatCommand *child = NULL;
   Integer branch = 0;
   
   // Check only one level first branch
   child = current->GetChildCommand(branch);
   
   while (child != NULL)
   {
      #ifdef DEBUG_IF_ELSE
      ShowCommand(wxT("   child = "), child);
      #endif
      
      if (child->IsOfType(wxT("BranchEnd")))
      {
         if (child->GetTypeName() == wxT("Else"))
         {
            #ifdef DEBUG_IF_ELSE
            MessageInterface::ShowMessage(wxT("IsElseFoundInIf() returning true\n"));
            #endif
            return true;
         }            
         break;
      }
      
      child = child->GetNext();
   }
   
   #ifdef DEBUG_IF_ELSE
   MessageInterface::ShowMessage(wxT("IsElseFoundInIf() returning false\n"));
   #endif
   
   return false;
}


//------------------------------------------------------------------------------
// bool ClearCommandSeq(GmatCommand *seq, bool leaveFirstCmd, bool callRunComplete)
//------------------------------------------------------------------------------
/*
 * Deletes whole command sequence.
 *
 * @param  seq  First command of the command sequence
 * @param  leaveFirstCmd  Set this flag to true if the first command should be
 *                        left undeleted (true)
 * @param  callRunComplete  Set this flag to true if RunComplete() should be
 *                        called for all commands (true)
 */
//------------------------------------------------------------------------------
bool GmatCommandUtil::ClearCommandSeq(GmatCommand *seq, bool leaveFirstCmd,
                                      bool callRunComplete)
{
   #ifdef DEBUG_SEQUENCE_CLEARING
   MessageInterface::ShowMessage(wxT("CommandUtil::ClearCommandSeq() entered\n"));
   #endif
   
   GmatCommand *cmd = seq, *removedCmd = NULL;
   
   if (cmd == NULL)
   {
      #ifdef DEBUG_SEQUENCE_CLEARING
      MessageInterface::ShowMessage
         (wxT("CommandUtil::ClearCommandSeq() exiting, first command is NULL\n"));
      #endif
      return true;
   }
   
   #ifdef DEBUG_SEQUENCE_CLEARING
   GmatCommand *current = cmd;
   MessageInterface::ShowMessage(wxT("\nClearing this command list:\n"));
   while (current)
   {
      ShowCommand(wxT("   "), current);
      current = current->GetNext();
   }
   MessageInterface::ShowMessage(wxT("\n"));
   #endif
   
   cmd = cmd->GetNext();
   while (cmd)
   {
      if (callRunComplete)
      {
         // Be sure we're in an idle state first
         #ifdef DEBUG_SEQUENCE_CLEARING
         MessageInterface::ShowMessage
            (wxT("   Calling %s->RunComplete\n"), cmd->GetTypeName().c_str());
         #endif
         
         cmd->RunComplete();
      }
      
      removedCmd = RemoveCommand(seq, cmd);
      
      if (removedCmd != NULL)
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (removedCmd, removedCmd->GetTypeName(), wxT("CommandUtil::ClearCommandSeq()"));
         #endif
         delete removedCmd;
      }
      removedCmd = NULL;
      cmd = seq->GetNext();
   }
   
   // if first command is to be delete
   if (!leaveFirstCmd)
   {
      #ifdef DEBUG_SEQUENCE_CLEARING
      MessageInterface::ShowMessage(wxT("   seq=<%p>\n"), seq);
      #endif
      
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         (seq, seq->GetTypeName(), wxT("CommandUtil::ClearCommandSeq()"));
      #endif
      delete seq;
      seq = NULL;
   }
   
   #ifdef DEBUG_SEQUENCE_CLEARING
   MessageInterface::ShowMessage(wxT("CommandUtil::ClearCommandSeq() returning true\n"));
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// bool IsAfter(GmatCommand *cmd1, GmatCommand *cmd2)
//------------------------------------------------------------------------------
/*
 * Returns true if cmd1 is after cmd2 in the sequence.
 *
 * @param  cmd1  First command
 * @param  cmd2  Second command
 *
 * @return true if cmd1 is after cmd2 in the sequence, false otherwise
 */
//------------------------------------------------------------------------------
bool GmatCommandUtil::IsAfter(GmatCommand *cmd1, GmatCommand *cmd2)
{
   if (cmd1 == NULL || cmd2 == NULL)
      return false;
   
   GmatCommand *current = cmd2;
   
   while (current != NULL)
   {
      if (current == cmd1)
         return true;
      
      current = current->GetNext();
   }
   
   return false;
}


//------------------------------------------------------------------------------
// bool FindObject(GmatCommand *cmd, Gmat::ObjectType type,
//                 const wxString &objName, wxString &cmdName)
//------------------------------------------------------------------------------
/*
 * Finds if object name is referenced in anywhere in the command sequence.
 *
 * @param  objType  The type of the named object
 * @param  objName  The object name to look for
 * @param  cmdName  The command name contains the object name if found
 *
 * @return  true  if object name found, false, otherwise
 */
//------------------------------------------------------------------------------
bool GmatCommandUtil::FindObject(GmatCommand *cmd, Gmat::ObjectType objType,
                                 const wxString &objName, wxString &cmdName)
{
   if (cmd == NULL)
      return false;
   
   GmatCommand *current = cmd;
   wxString cmdstr = cmd->GetTypeName();
   
   #ifdef DEBUG_COMMAND_FIND_OBJECT
   MessageInterface::ShowMessage
      (wxT("===> GmatCommandUtil::FindObject() entered, objType=%d, objName='%s', ")
       wxT("cmd=<%p><%s>\n"), objType, objName.c_str(), cmd, cmdstr.c_str());
   #endif
   
   
   while (current != NULL)
   {
      cmdstr = wxT("--- ") + current->GetTypeName() + wxT("\n");
      
      #ifdef DEBUG_COMMAND_FIND_OBJECT
      MessageInterface::ShowMessage(cmdstr);
      #endif
      
      try
      {
         StringArray names = current->GetRefObjectNameArray(objType);
         
         for (UnsignedInt i=0; i<names.size(); i++)
         {
            #ifdef DEBUG_COMMAND_FIND_OBJECT
            MessageInterface::ShowMessage(wxT("names[%d]=%s\n"), i, names[i].c_str());
            #endif
            
            if (names[i] == objName)
            {
               cmdName = current->GetTypeName();
               return true;
            }
         }
      }
      catch (BaseException &e)
      {
         // Use exception to remove Visual C++ warning
         e.GetMessageType();
         #ifdef DEBUG_COMMAND_FIND_OBJECT
         MessageInterface::ShowMessage(wxT("*** INTERNAL WARNING *** ") + e.GetFullMessage());
         #endif
      }
      
      // go through sub commands
      if ((current->GetChildCommand(0)) != NULL)
      {
         if (FindObjectFromSubCommands(current, 0, objType, objName, cmdName))
            return true;
      }
      
      current = current->GetNext();
   }
   
   #ifdef DEBUG_COMMAND_FIND_OBJECT
   MessageInterface::ShowMessage
      (wxT("===> GmatCommandUtil::FindObject() returning false\n"));
   #endif
   return false;
}



//------------------------------------------------------------------------------
// bool FindObjectFromSubCommands(GmatCommand *brCmd, Integer level,
//                                Gmat::ObjectType objType,
//                                const wxString &objName, wxString &cmdName)
//------------------------------------------------------------------------------
/*
 * Finds if object name is referenced in anywhere in the command sequence.
 *
 * @param  objType  The type of the named object
 * @param  objName  The object name to look for
 * @param  cmdName  The command name contains the object name if found
 *
 * @return  true  if object name found, false, otherwise
 */
//------------------------------------------------------------------------------
bool GmatCommandUtil::FindObjectFromSubCommands(GmatCommand *brCmd, Integer level,
                                                Gmat::ObjectType objType, 
                                                const wxString &objName,
                                                wxString &cmdName)
{
   GmatCommand* current = brCmd;
   Integer childNo = 0;
   GmatCommand* nextInBranch = NULL;
   GmatCommand* child;
   wxString cmdstr;
   
   while((child = current->GetChildCommand(childNo)) != NULL)
   {
      nextInBranch = child;
      
      while ((nextInBranch != NULL) && (nextInBranch != current))
      {         
         #ifdef DEBUG_COMMAND_FIND_OBJECT
         for (int i=0; i<=level; i++)
            cmdstr = wxT("---") + cmdstr;         
         cmdstr = wxT("--- ") + nextInBranch->GetTypeName() + wxT("\n");        
         MessageInterface::ShowMessage(wxT("%s"), cmdstr.c_str());
         #endif
         
         try
         {
            StringArray names = nextInBranch->GetRefObjectNameArray(objType);
            
            for (UnsignedInt i=0; i<names.size(); i++)
            {
               #ifdef DEBUG_COMMAND_FIND_OBJECT
               MessageInterface::ShowMessage(wxT("names[%d]=%s\n"), i, names[i].c_str());
               #endif
               
               if (names[i] == objName)
               {
                  cmdName = nextInBranch->GetTypeName();
                  return true;
               }
            }
         }
         catch (BaseException &e)
         {
            // Use exception to remove Visual C++ warning
            e.GetMessageType();
            #ifdef DEBUG_COMMAND_FIND_OBJECT
            MessageInterface::ShowMessage(wxT("*** INTERNAL WARNING *** ") + e.GetFullMessage());
            #endif
         }
         
         if (nextInBranch->GetChildCommand() != NULL)
            if (FindObjectFromSubCommands(nextInBranch, level+1, objType, objName, cmdName))
               return true;
         
         nextInBranch = nextInBranch->GetNext();
      }
      
      ++childNo;
   }
   
   return false;
}


//------------------------------------------------------------------------------
// wxString GetCommandSeqString(GmatCommand *cmd, bool showAddr = true,
//                bool showGenStr = false, const wxString &indentStr = wxT("---"))
//------------------------------------------------------------------------------
/*
 * Returns string of command sequence given by cmd.
 *
 * @notes: Do not use %s for command string output, it may crash when it encounters
 * comment with % in the scripts
 */
//------------------------------------------------------------------------------
wxString GmatCommandUtil::
GetCommandSeqString(GmatCommand *cmd, bool showAddr, bool showGenStr,
                    const wxString &indentStr)
{
   wxString buf;
   GmatCommand *current = cmd;
   wxString cmdseq, cmdstr, genStr;
   cmdstr = wxT("\n---------- Mission Sequence ----------\n");
   cmdseq.append(cmdstr);
   buf[0] = wxT('\0');
   
   #ifdef DEBUG_COMMAND_SEQ_STRING
   MessageInterface::ShowMessage
      (wxT("===> GmatCommandUtil::GetCommandSeqString(%p)\n"), cmd);
   MessageInterface::ShowMessage(wxT("%s"), cmdstr.c_str());
   #endif
   
   
   while (current != NULL)
   {
      if (showAddr)
         buf.Printf( wxT("(%p)"), current);
      
      genStr = wxT("");
      if (showGenStr)
      {
         if (current->GetTypeName() == wxT("BeginScript"))
            genStr = wxT("<BeginScript>");
         else if (current->GetTypeName() == wxT("EndScript"))
            genStr = wxT("<EndScript>");
         else
            genStr = wxT(" <") + current->GetGeneratingString(Gmat::NO_COMMENTS) + wxT(">");
      }
		else
		{
			// Show only summary name
			genStr = wxT("(") + current->GetSummaryName() + wxT(")");
		}
		
      // if indentation string is not blank, use it from the first level
      if (indentStr.find(wxT(" ")) == indentStr.npos)
         cmdstr = indentStr + wxT(" ") + wxString(buf) + current->GetTypeName() + genStr + wxT("\n");
      else
         cmdstr = wxString(buf) + current->GetTypeName() + genStr + wxT("\n");
      
      cmdseq.append(cmdstr);
      
      #ifdef DEBUG_COMMAND_SEQ_STRING
      MessageInterface::ShowMessage(wxT("%s"), cmdstr.c_str());
      #endif
      
      if ((current->GetChildCommand(0)) != NULL)
         GetSubCommandString(current, 0, cmdseq, showAddr, showGenStr, indentStr);
      
      current = current->GetNext();
   }
   
   cmdseq.append(wxT("\n"));
   
   return cmdseq;
}


//------------------------------------------------------------------------------
// void GetSubCommandString(GmatCommand* brCmd, Integer level, wxString &cmdseq,
//                          bool showAddr = true, bool showGenStr = false,
//                          const wxString &indentStr = wxT("---"))
//------------------------------------------------------------------------------
void GmatCommandUtil::
GetSubCommandString(GmatCommand* brCmd, Integer level, wxString &cmdseq,
                    bool showAddr, bool showGenStr, const wxString &indentStr)
{
   wxString buf;
   GmatCommand* current = brCmd;
   Integer childNo = 0;
   GmatCommand* nextInBranch;
   GmatCommand* child;
   wxString cmdstr, genStr;
   buf[0] = wxT('\0');
   
   while((child = current->GetChildCommand(childNo)) != NULL)
   {
      nextInBranch = child;
      
      while ((nextInBranch != NULL) && (nextInBranch != current))
      {
         for (int i=0; i<=level; i++)
         {
            cmdseq.append(indentStr);
            
            #ifdef DEBUG_COMMAND_SEQ_STRING
            MessageInterface::ShowMessage(indentStr);
            #endif
         }
         
         if (showAddr)
            buf.Printf( wxT("(%p)"), nextInBranch);
         
         genStr = wxT("");
			if (showGenStr)
			{
				if (nextInBranch->GetTypeName() == wxT("BeginScript"))
					genStr = wxT("<BeginScript>");
				else if (nextInBranch->GetTypeName() == wxT("EndScript"))
					genStr = wxT("<EndScript>");
				else
					genStr = wxT(" <") + nextInBranch->GetGeneratingString(Gmat::NO_COMMENTS) + wxT(">");
         }
			else
			{
				// Show only summary name
				genStr = wxT("(") + nextInBranch->GetSummaryName() + wxT(")");
			}
			
         // if indentation string is not blank, use it from the first sub level
         if (indentStr.find(wxT(" ")) == indentStr.npos)
            cmdstr = indentStr + wxT(" ") + buf + nextInBranch->GetTypeName() + genStr + wxT("\n");
         else
            cmdstr = buf + nextInBranch->GetTypeName() + genStr + wxT("\n");
         
         cmdseq.append(cmdstr);
         
         #ifdef DEBUG_COMMAND_SEQ_STRING
         MessageInterface::ShowMessage(wxT("%s"), cmdstr.c_str());
         #endif
         
         if (nextInBranch->GetChildCommand() != NULL)
            GetSubCommandString(nextInBranch, level+1, cmdseq, showAddr, showGenStr, indentStr);
         
         nextInBranch = nextInBranch->GetNext();
      }
      
      ++childNo;
   }
}


//------------------------------------------------------------------------------
// void ShowCommand(const wxString &title1, GmatCommand *cmd1,
//                   const wxString &title2, GmatCommand *cmd2)
//------------------------------------------------------------------------------
void GmatCommandUtil::ShowCommand(const wxString &title1, GmatCommand *cmd1,
                                  const wxString &title2, GmatCommand *cmd2)
{
   if (title2 == wxT(""))
   {
      if (cmd1 == NULL)
         MessageInterface::ShowMessage(wxT("%s<%p><NULL>\n"), title1.c_str(), cmd1);
      else
         MessageInterface::ShowMessage
            (wxT("%s<%p><%s>\n"), title1.c_str(), cmd1, cmd1->GetTypeName().c_str());
   }
   else
   {
      if (cmd2 == NULL)
         MessageInterface::ShowMessage
            (wxT("%s<%p>NULL%s<%p>NULL\n"), title1.c_str(), cmd1, title2.c_str(), cmd2);
      else
         MessageInterface::ShowMessage
            (wxT("%s<%p><%s>%s<%p><%s>\n"), title1.c_str(), cmd1, cmd1->GetTypeName().c_str(),
             title2.c_str(), cmd2, cmd2->GetTypeName().c_str());
   }
}

