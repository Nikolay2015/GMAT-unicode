//$Id: EndFor.hpp 9513 2011-04-30 21:23:06Z djcinsb $
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


#ifndef EndFor_hpp
#define EndFor_hpp


#include "GmatCommand.hpp"


class GMAT_API EndFor : public GmatCommand
{
public:
   EndFor(void);
   virtual ~EndFor(void);
   
   EndFor(const EndFor& ef);
   EndFor&          operator=(const EndFor& ef);
   
   virtual bool            Initialize(void);
   virtual bool            Execute(void);
   
   virtual bool            Insert(GmatCommand *cmd, GmatCommand *prev);
   
   // inherited from GmatBase
   virtual bool            RenameRefObject(const Gmat::ObjectType type,
                                           const wxString &oldName,
                                           const wxString &newName);   
   virtual GmatBase*       Clone(void) const;
   const wxString&      GetGeneratingString(Gmat::WriteMode mode,
                                               const wxString &prefix,
                                               const wxString &useName);
};


#endif // EndFor_hpp
