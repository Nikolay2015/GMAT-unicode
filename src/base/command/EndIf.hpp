//$Id: EndIf.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                             EndIf
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
// Created: 2004/01/30
//
/**
 * Definition for the closing line of the IF Statement
 */
//------------------------------------------------------------------------------


#ifndef EndIf_hpp
#define EndIf_hpp


#include "GmatCommand.hpp"


class GMAT_API EndIf : public GmatCommand
{
public:
   EndIf();
   virtual ~EndIf();
    
   EndIf(const EndIf& ic);
   EndIf&              operator=(const EndIf& ic);
    
   virtual bool            Initialize();
   virtual bool            Execute();

   virtual bool            Insert(GmatCommand *cmd, GmatCommand *prev);

   // inherited from GmatBase
   virtual bool            RenameRefObject(const Gmat::ObjectType type,
                                           const wxString &oldName,
                                           const wxString &newName);   
   virtual GmatBase*       Clone() const;
   const wxString&      GetGeneratingString(Gmat::WriteMode mode,
                                               const wxString &prefix,
                                               const wxString &useName);
};


#endif // EndIf_hpp
