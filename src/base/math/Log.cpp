//$Id: Log.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  Log
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
 * Implements Log class.
 */
//------------------------------------------------------------------------------

#include "Log.hpp"
#include "MessageInterface.hpp"

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// Log()
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
Log::Log(const wxString &nomme)
   : MathFunction(wxT("Log"), nomme)
{
}


//------------------------------------------------------------------------------
// ~Log()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
Log::~Log()
{
}


//------------------------------------------------------------------------------
//  Log(const Log &copy)
//------------------------------------------------------------------------------
/**
 * Constructs the Log object (copy constructor).
 * 
 * @param <copy> Object that is copied
 */
//------------------------------------------------------------------------------
Log::Log(const Log &copy) :
   MathFunction      (copy)
{
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Clone of the Log operation.
 *
 * @return clone of the Log operation.
 *
 */
//------------------------------------------------------------------------------
GmatBase* Log::Clone() const
{
   return (new Log(*this));
}


//------------------------------------------------------------------------------
// void GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
//------------------------------------------------------------------------------
void Log::GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
{
   Integer type1, row1, col1; // Left node
   
   // Get the type(Real or Matrix), # rows and # columns of the left node
   leftNode->GetOutputInfo(type1, row1, col1);
   
   if (type1 != Gmat::REAL_TYPE)
      throw MathException(wxT("Left is not scalar, so cannot do Log().\n"));  
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
bool Log::ValidateInputs()
{
   if (leftNode == NULL)
      throw MathException(wxT("Log() - Missing input arguments.\n"));
   
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
 * @return the Log of left node
 *
 */
//------------------------------------------------------------------------------
Real Log::Evaluate()
{
   return GmatMathUtil::Log(leftNode->Evaluate());
}
