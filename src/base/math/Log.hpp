//------------------------------------------------------------------------------
//                              Log
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
// Author: LaMont Ruley
// Created: 2006/04/20
//
/**
 * Declares Log class.
 */
//------------------------------------------------------------------------------
#ifndef Log_hpp
#define Log_hpp

#include "MathFunction.hpp"
#include "RealUtilities.hpp"

class GMAT_API Log : public MathFunction
{
public:
   Log(const wxString &nomme);
   virtual ~Log();
   Log(const Log &copy);
                 
   // inherited from GmatBase
   virtual GmatBase* Clone() const;

   virtual void GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount);
   virtual bool ValidateInputs(); 
   virtual Real Evaluate();
};

#endif // Log_hpp
