//$Id: EndTarget.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                EndTarget
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
// Created: 2004/01/23
//
/**
 * Definition for the closing line of a Targeter loop
 */
//------------------------------------------------------------------------------


#ifndef EndTarget_hpp
#define EndTarget_hpp


#include "GmatCommand.hpp"


class GMAT_API EndTarget : public GmatCommand
{
public:
   EndTarget();
   virtual ~EndTarget();
    
   EndTarget(const EndTarget& et);
   EndTarget&              operator=(const EndTarget& et);
    
   virtual bool            Initialize();
   virtual bool            Execute();
    
   virtual bool            Insert(GmatCommand *cmd, GmatCommand *prev);

   // inherited from GmatBase
   virtual bool            RenameRefObject(const Gmat::ObjectType type,
                                           const wxString &oldName,
                                           const wxString &newName);   
   virtual GmatBase*       Clone() const;
   virtual const wxString&
                           GetGeneratingString(Gmat::WriteMode mode,
                                               const wxString &prefix,
                                               const wxString &useName);
};


#endif // EndTarget_hpp
