//$Id: Norm.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  Norm
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
// Created: 2006/04/20
//
/**
 * Implements Norm class.
 */
//------------------------------------------------------------------------------

#include "Norm.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_NORM

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// Norm()
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
Norm::Norm(const wxString &nomme)
   : MathFunction(wxT("Norm"), nomme)
{
}


//------------------------------------------------------------------------------
// ~Norm()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
Norm::~Norm()
{
}


//------------------------------------------------------------------------------
//  Norm(const Norm &copy)
//------------------------------------------------------------------------------
/**
 * Constructs the Norm object (copy constructor).
 * 
 * @param <copy> Object that is copied
 */
//------------------------------------------------------------------------------
Norm::Norm(const Norm &copy) :
   MathFunction      (copy)
{
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Clone of the Norm operation.
 *
 * @return clone of the Norm operation.
 *
 */
//------------------------------------------------------------------------------
GmatBase* Norm::Clone() const
{
   return (new Norm(*this));
}


//------------------------------------------------------------------------------
// void GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
//------------------------------------------------------------------------------
void Norm::GetOutputInfo(Integer &type, Integer &rowCount, Integer &colCount)
{
   type = Gmat::REAL_TYPE;
   rowCount = 1;
   colCount = 1;

   #ifdef DEBUG_NORM
   MessageInterface::ShowMessage
      (wxT("Norm::GetOutputInfo() type=%d, rowCount=%d, colCount=%d\n"),
       type, rowCount, colCount);
   #endif
}


//------------------------------------------------------------------------------
// bool ValidateInputs()
//------------------------------------------------------------------------------
/**
 * This method calls its subnodes and checks to be sure that the subnodes return
 * compatible data for the function.
 */
//------------------------------------------------------------------------------
bool Norm::ValidateInputs()
{
   Integer type1, row1, col1; // Left node
   
   if (leftNode == NULL)
      throw MathException(wxT("Norm() - Missing input arguments"));
   
   #ifdef DEBUG_NORM
   MessageInterface::ShowMessage
      (wxT("Norm::ValidateInputs() left=%s, %s\n"),
       leftNode->GetTypeName().c_str(), leftNode->GetName().c_str());
   #endif
   
   // Get the type(Real or Matrix), # rows and # columns of the left node
   // Input can be a scalar or matrix, so just call leftNode (LOJ: 2010.07.28)
   leftNode->GetOutputInfo(type1, row1, col1);
   
   #ifdef DEBUG_NORM
   MessageInterface::ShowMessage
      (wxT("Norm::ValidateInputs() returning true, type=%d, row=%d, col=%d\n"), 
       type1, row1, col1);
   #endif
   
   return true;
}


//------------------------------------------------------------------------------
// Real Evaluate()
//------------------------------------------------------------------------------
/**
 * @return the Norm of left node
 *
 */
//------------------------------------------------------------------------------
Real Norm::Evaluate()
{
   Integer type, rowCount, colCount;
   leftNode->GetOutputInfo(type, rowCount, colCount);
   
   if(type == Gmat::RMATRIX_TYPE)
   {
      if (rowCount == 1)
      {
        return (leftNode->MatrixEvaluate()).GetRow(0).Norm();
      }
      else if (colCount == 1)
      {
        return (leftNode->MatrixEvaluate()).GetColumn(0).Norm();
      }
      else
         throw MathException(
            wxT("Norm::Evaluate():: Can only be done on a vector or a scalar.  ")
            wxT("This is a matrix.\n"));
   }
   else
   {
      // Norm of a scalar should be the absolute value of the scalar.
      Real result = leftNode->Evaluate();
      return GmatMathUtil::Abs(result);
   }
}
