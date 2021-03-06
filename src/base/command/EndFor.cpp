//$Id: EndFor.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                             EndFor
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
// Author:  Joey Gurganus
// Created: 2004/01/29
// Modified: W. Shoan 2004.09.13 - Updated for use in Build 3
//
/**
 * Definition for the closing line of a for loop
 */
//------------------------------------------------------------------------------


#include "EndFor.hpp"
#include "BranchCommand.hpp"


//------------------------------------------------------------------------------
// EndFor::EndFor(void)
//------------------------------------------------------------------------------
EndFor::EndFor(void) :
    GmatCommand         (wxT("EndFor"))
{
   objectTypeNames.push_back(wxT("BranchEnd"));
   depthChange = -1;
}


//------------------------------------------------------------------------------
// ~EndFor(void)
//------------------------------------------------------------------------------
EndFor::~EndFor(void)
{
}
    

//------------------------------------------------------------------------------
// EndFor(const EndFor& ef) :
//------------------------------------------------------------------------------
EndFor::EndFor(const EndFor& ef) :
    GmatCommand         (ef)
{
}


//------------------------------------------------------------------------------
// EndFor& operator=(const EndFor& ef)
//------------------------------------------------------------------------------
EndFor& EndFor::operator=(const EndFor& ef)
{
   if (this == &ef)
     return *this;

   GmatCommand::operator=(ef);
   return *this;
}


//------------------------------------------------------------------------------
// bool Initialize(void)
//------------------------------------------------------------------------------
bool EndFor::Initialize(void)
{
   GmatCommand::Initialize();
   
   // Validate that next points to the owning for command
   if (!next)
     throw CommandException(wxT("EndFor Command not properly reconnected"));

   if (next->GetTypeName() != wxT("For"))
     throw CommandException(wxT("EndFor Command not connected to For Command"));
                          

   return true;    
}


//------------------------------------------------------------------------------
// bool Execute(void)
//------------------------------------------------------------------------------
bool EndFor::Execute(void)
{
   BuildCommandSummary(true);
   return true;
}


//------------------------------------------------------------------------------
// bool Insert(GmatCommand *cmd, GmatCommand *prev)
//------------------------------------------------------------------------------
bool EndFor::Insert(GmatCommand *cmd, GmatCommand *prev)
{
   // if inserting after End statement for branch command, we want to 
   // insert right after the entire If command
   if (this == prev) return ((BranchCommand*)next)->InsertRightAfter(cmd);
   return false;
}


//---------------------------------------------------------------------------
// bool RenameRefObject(const Gmat::ObjectType type,
//                      const wxString &oldName, const wxString &newName)
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
bool EndFor::RenameRefObject(const Gmat::ObjectType type,
                             const wxString &oldName,
                             const wxString &newName)
{
   // There are no renamealbe objects
   return true;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the EndFor.
 *
 * @return clone of the EndFor.
 *
 */
//------------------------------------------------------------------------------
GmatBase* EndFor::Clone(void) const
{   
   return (new EndFor(*this));
}


//------------------------------------------------------------------------------
//  const wxString GetGeneratingString()
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
 * @param <mode>    Specifies the type of serialization requested.
 * @param <prefix>  Optional prefix appended to the object's name. (Used for
 *                  indentation)
 * @param <useName> Name that replaces the object's name (Not yet used
 *                  in commands).
 *
 * @return The script line that defines this GmatCommand.
 */
//------------------------------------------------------------------------------
const wxString& EndFor::GetGeneratingString(Gmat::WriteMode mode,
                                               const wxString &prefix,
                                               const wxString &useName)
{
   generatingString = prefix + wxT("EndFor;");
   return GmatCommand::GetGeneratingString(mode, prefix, useName);
}


