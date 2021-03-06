//$Id: Divide.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  Divide
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
// Created: 2006/03/31
//
/**
 * Implements Divide class.
 */
//------------------------------------------------------------------------------

#include "Divide.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_INPUT_OUTPUT 1

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// Divide()
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
Divide::Divide(const wxString &nomme)
   : MathFunction(wxT("Divide"), nomme)
{
}


//------------------------------------------------------------------------------
// ~Divide()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
Divide::~Divide()
{
}


//------------------------------------------------------------------------------
//  Divide(const Divide &copy)
//------------------------------------------------------------------------------
/**
 * Constructs the Divide object (copy constructor).
 * 
 * @param <copy> Object that is copied
 */
//------------------------------------------------------------------------------
Divide::Divide(const Divide &copy) :
   MathFunction      (copy)
{
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Clone of the Divide operation.
 *
 * @return clone of the Divide operation.
 *
 */
//------------------------------------------------------------------------------
GmatBase* Divide::Clone() const
{
   return (new Divide(*this));
}


//------------------------------------------------------------------------------
// void GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
//------------------------------------------------------------------------------
void Divide::GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
{
   #if DEBUG_INPUT_OUTPUT
   MessageInterface::ShowMessage
      (wxT("Divide::GetOutputInfo() this=<%p><%s><%s>\n"), this, GetTypeName().c_str(),
       GetName().c_str());
   #endif
   
   Integer type1, row1, col1; // Left node
   Integer type2, row2, col2; // Right node
   
   // Get the type(Real or Matrix), # rows and # columns of the left node
   if (leftNode)
      leftNode->GetOutputInfo(type1, row1, col1);
   else
      throw MathException(wxT("Left node is NULL in ") + GetTypeName() +
                          wxT("::GetOutputInfo()\n"));
   
   // Get the type(Real or Matrix), # rows and # columns of the right node
   if (rightNode)
      rightNode->GetOutputInfo(type2, row2, col2);
   else
      throw MathException(wxT("Right node is NULL in ") + GetTypeName() +
                          wxT("::GetOutputInfo()\n"));
   
   #if DEBUG_INPUT_OUTPUT
   MessageInterface::ShowMessage
      (wxT("Divide::GetOutputInfo() type1=%d, row1=%d, col1=%d, type2=%d, ")
       wxT("row2=%d, col2=%d\n"), type1, row1, col1, type2, row2, col2);
   #endif
   
   type = type1;
   rowCount = row1;
   colCount = col1;
   
   if ((type1 == Gmat::RMATRIX_TYPE) && (type2 == Gmat::RMATRIX_TYPE))
   {
      // Check for 1x1
      if (row1 == 1 && col1 == 1)
      {
         type = type2;
         rowCount = row2;
         colCount = col2;
      }
      else if (row2 == 1 && col2 == 1)
      {
         type = type1;
         rowCount = row1;
         colCount = col1;            
      }
   }
   else if (type2 == Gmat::RMATRIX_TYPE)
   {
      type = type2;
      rowCount = row2;
      colCount = col2;
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
bool Divide::ValidateInputs()
{
   #ifdef DEBUG_INPUT_OUTPUT
   MessageInterface::ShowMessage
      (wxT("\Divide::ValidateInputs() '%s' entered\n"), GetName().c_str());
   #endif
   
   if (leftNode == NULL)
      throw MathException(wxT("Divide() - Missing input arguments"));
   
   if (rightNode == NULL)
      throw MathException(wxT("Divide() - Not enough input arguments"));
   
   Integer type1, row1, col1; // Left node
   Integer type2, row2, col2; // Right node
   bool retval = false;
   
   // Get the type(Real or Matrix), # rows and # columns of the left node
   leftNode->GetOutputInfo(type1, row1, col1);
   
   // Get the type(Real or Matrix), # rows and # columns of the right node
   rightNode->GetOutputInfo(type2, row2, col2);
   
   #ifdef DEBUG_INPUT_OUTPUT
   MessageInterface::ShowMessage
      (wxT("Divide::ValidateInputs() type1=%d, row1=%d, col1=%d, ")
       wxT("type2=%d, row2=%d, col2=%d\n"), type1, row1, col1, type2, row2, col2);
   #endif
   
   if ((type1 == Gmat::REAL_TYPE) && (type2 == Gmat::REAL_TYPE))
      retval = true;
   else if ((type1 == Gmat::RMATRIX_TYPE) && (type2 == Gmat::RMATRIX_TYPE))
      if ((row1 == row2) && (col1 == col2))
         retval = true;
      else if ((row1 == 1 && col1 == 1) || (row2 == 1 && col2 == 1))
         retval = true;
      else
         retval = false; 
   else
      retval = true;
   
   #ifdef DEBUG_INPUT_OUTPUT
   MessageInterface::ShowMessage
      (wxT("Divide::ValidateInputs() '%s' returning %d\n"), GetName().c_str(), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// Real Evaluate()
//------------------------------------------------------------------------------
/**
 * @return the product of left and right nodes
 *
 */
//------------------------------------------------------------------------------
Real Divide::Evaluate()
{
   return leftNode->Evaluate() / rightNode->Evaluate();
}


//------------------------------------------------------------------------------
// Rmatrix MatrixEvaluate()
//------------------------------------------------------------------------------
/**
 * @return the division of left node by the right node
 *
 */
//------------------------------------------------------------------------------
Rmatrix Divide::MatrixEvaluate()
{
   #ifdef DEBUG_INPUT_OUTPUT
   MessageInterface::ShowMessage
      (wxT("\Divide::MatrixEvaluate() '%s' entered\n"), GetName().c_str());
   #endif
   
   Integer type1, row1, col1; // Left node matrix
   Integer type2, row2, col2; // Right node matrix
   Rmatrix div;
   
   // Get the type(Real or Matrix), # rows and # columns of the left node
   leftNode->GetOutputInfo(type1, row1, col1);
   
   // Get the type(Real or Matrix), # rows and # columns of the right node
   rightNode->GetOutputInfo(type2, row2, col2);
   
   // Divide matrix by matrix
   if( type1 == Gmat::RMATRIX_TYPE && type2 == Gmat::RMATRIX_TYPE)
      div = leftNode->MatrixEvaluate() / rightNode->MatrixEvaluate();
   
   // Divide scalar by matrix
   else if( type1 == Gmat::REAL_TYPE && type2 == Gmat::RMATRIX_TYPE)
      div = leftNode->Evaluate() / rightNode->MatrixEvaluate();
   
   // Divide matrix by scalar
   else if( type1 == Gmat::RMATRIX_TYPE && type2 == Gmat::REAL_TYPE)
      div = leftNode->MatrixEvaluate() / rightNode->Evaluate();
   
   #ifdef DEBUG_INPUT_OUTPUT
   MessageInterface::ShowMessage
      (wxT("Divide::MatrixEvaluate() '%s' returning %s\n"), GetName().c_str(), div.ToString().c_str());
   #endif
   
   return div;
}

