//------------------------------------------------------------------------------
//                              ACos
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
// Created: 2006/04/10
//
/**
 * Declares Acos class.
 */
//------------------------------------------------------------------------------
#ifndef Acos_hpp
#define Acos_hpp

#include "MathFunction.hpp"
#include "RealUtilities.hpp"

class GMAT_API Acos : public MathFunction
{
public:
   Acos(const wxString &nomme);
   virtual ~Acos();
   Acos(const Acos &copy);
                 
   // inherited from GmatBase
   virtual GmatBase* Clone() const;

   virtual void GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount);
   virtual bool ValidateInputs(); 
   virtual Real Evaluate();
};

#endif // Acos_hpp
