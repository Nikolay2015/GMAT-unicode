//$Id: Log10.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  Log10
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
// Created: 2006/04/14
//
/**
 * Implements Log10 class.
 */
//------------------------------------------------------------------------------

#include "Log10.hpp"
#include "MessageInterface.hpp"

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// Log10()
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
Log10::Log10(const wxString &nomme)
   : MathFunction(wxT("Log10"), nomme)
{
}


//------------------------------------------------------------------------------
// ~Log10()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
Log10::~Log10()
{
}


//------------------------------------------------------------------------------
//  Log10(const Log10 &copy)
//------------------------------------------------------------------------------
/**
 * Constructs the Log10 object (copy constructor).
 * 
 * @param <copy> Object that is copied
 */
//------------------------------------------------------------------------------
Log10::Log10(const Log10 &copy) :
   MathFunction      (copy)
{
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Clone of the Log10 operation.
 *
 * @return clone of the Log10 operation.
 *
 */
//------------------------------------------------------------------------------
GmatBase* Log10::Clone() const
{
   return (new Log10(*this));
}


//------------------------------------------------------------------------------
// void GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
//------------------------------------------------------------------------------
void Log10::GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
{
   Integer type1, row1, col1; // Left node
   
   // Get the type(Real or Matrix), # rows and # columns of the left node
   leftNode->GetOutputInfo(type1, row1, col1);
   
   if (type1 != Gmat::REAL_TYPE)
      throw MathException(wxT("Left is not scalar, so cannot do Sqrt().\n"));  
   else
   {
      type = type1;
      rowCount = row1;
      colCount = col1;
   }
}


//------------------------------------------------------------------------------
// bool ValidateInputs()
//------------------------------------------------------------------------------
/**
 * This method calls its subnodes and checks to be sure that the subnodes return
 * compatible data for the function.
 */
//------------------------------------------------------------------------------
bool Log10::ValidateInputs()
{
   if (leftNode == NULL)
      throw MathException(wxT("Log10() - Missing input arguments.\n"));
   
   Integer type1, row1, col1; // Left node
   
   // Get the type(Real or Matrix), # rows and # columns of the left node
   leftNode->GetOutputInfo(type1, row1, col1);
   
   if (type1 == Gmat::REAL_TYPE)
      return true;
   else
      return false;
}


//------------------------------------------------------------------------------
// Real Evaluate()
//------------------------------------------------------------------------------
/**
 * @return the base 10 logarithm.
 *
 */
//------------------------------------------------------------------------------
Real Log10::Evaluate()
{
   return GmatMathUtil::Log10(leftNode->Evaluate());
}

