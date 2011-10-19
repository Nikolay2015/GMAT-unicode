//$Id: Sinh.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  Sinh
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
// Author: Allison Greene
// Created: 2006/06/27
//
/**
 * Implements Sinh class.
 */
//------------------------------------------------------------------------------

#include "Sinh.hpp"
#include "MessageInterface.hpp"

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// Sinh()
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
Sinh::Sinh(const wxString &nomme)
   : MathFunction(wxT("Sinh"), nomme)
{
}


//------------------------------------------------------------------------------
// ~Sinh()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
Sinh::~Sinh()
{
}


//------------------------------------------------------------------------------
//  Sinh(const Sinh &copy)
//------------------------------------------------------------------------------
/**
 * Constructs the Sinh object (copy constructor).
 * 
 * @param <copy> Object that is copied
 */
//------------------------------------------------------------------------------
Sinh::Sinh(const Sinh &copy) :
   MathFunction      (copy)
{
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Clone of the Sinh operation.
 *
 * @return clone of the Sinh operation.
 *
 */
//------------------------------------------------------------------------------
GmatBase* Sinh::Clone() const
{
   return (new Sinh(*this));
}


//------------------------------------------------------------------------------
// void GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
//------------------------------------------------------------------------------
void Sinh::GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
{
   Integer type1, row1, col1; // Left node
   
   // Get the type(Real or Matrix), # rows and # columns of the left node
   leftNode->GetOutputInfo(type1, row1, col1);
   
   if (type1 != Gmat::REAL_TYPE)
      throw MathException(wxT("Left is not scalar, so cannot do Sinh().\n"));  
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
bool Sinh::ValidateInputs()
{
   if (leftNode == NULL)
      throw MathException(wxT("Sinh() - Missing input arguments.\n"));
   
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
 * @return the Sinh of left node
 *
 */
//------------------------------------------------------------------------------
Real Sinh::Evaluate()
{
   return GmatMathUtil::Sinh(leftNode->Evaluate());
}
