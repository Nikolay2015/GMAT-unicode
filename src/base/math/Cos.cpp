//$Id: Cos.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  Cos
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
 * Implements Cos class.
 */
//------------------------------------------------------------------------------

#include "Cos.hpp"
#include "MessageInterface.hpp"

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// Cos()
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
Cos::Cos(const wxString &nomme)
   : MathFunction(wxT("Cos"), nomme)
{
}


//------------------------------------------------------------------------------
// ~Cos()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
Cos::~Cos()
{
}


//------------------------------------------------------------------------------
//  Cos(const Cos &copy)
//------------------------------------------------------------------------------
/**
 * Constructs the Cos object (copy constructor).
 * 
 * @param <copy> Object that is copied
 */
//------------------------------------------------------------------------------
Cos::Cos(const Cos &copy) :
   MathFunction      (copy)
{
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Clone of the Cos operation.
 *
 * @return clone of the Cos operation.
 *
 */
//------------------------------------------------------------------------------
GmatBase* Cos::Clone() const
{
   return (new Cos(*this));
}


//------------------------------------------------------------------------------
// void GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
//------------------------------------------------------------------------------
void Cos::GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
{
   //loj: only left side is available
   
   Integer type1, row1, col1; // Left node
//    Integer type2, row2, col2; // Right node
   
   // Get the type(Real or Matrix), # rows and # columns of the left node
   leftNode->GetOutputInfo(type1, row1, col1);
   
//    // Get the type(Real or Matrix), # rows and # columns of the right node
//    rightNode->GetOutputInfo(type2, row2, col2);

   //if ((type1 != type2) || (row1 != row2) || (col1 != col2))
   if (type1 != Gmat::REAL_TYPE)
      throw MathException(wxT("Left is not scalar, so cannot do Cos().\n"));    
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
bool Cos::ValidateInputs()
{
   if (leftNode == NULL)
      throw MathException(wxT("Cos() - Missing input arguments.\n"));
   
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
 * @return the Cos of left node
 *
 */
//------------------------------------------------------------------------------
Real Cos::Evaluate()
{
   return GmatMathUtil::Cos(leftNode->Evaluate());
}
