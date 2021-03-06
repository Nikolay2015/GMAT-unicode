//$Id: MathFunction.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                   MathFunction
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P.
//
// Author: Waka Waktola
// Created: 2006/04/04
//
/**
 * Defines the Math functions class for Math in scripts.
 */
//------------------------------------------------------------------------------

#include "MathFunction.hpp"
#include "MessageInterface.hpp"

//------------------------------------------------------------------------------
//  MathFunction(wxString typeStr, wxString nomme)
//------------------------------------------------------------------------------
/**
 * Constructs the MathFunction object (default constructor).
 * 
 * @param <typeStr> String text identifying the object type
 * @param <nomme>   Name for the object
 */
//------------------------------------------------------------------------------
MathFunction::MathFunction(const wxString &typeStr, const wxString &nomme) :
   MathNode    (typeStr, nomme),
   leftNode    (NULL),
   rightNode   (NULL)
{
   isFunction = true;
   objectTypeNames.push_back(wxT("MathFunction"));   
}

//------------------------------------------------------------------------------
//  ~MathFunction(void)
//------------------------------------------------------------------------------
/**
 * Destroys the MathFunction object (destructor).
 */
//------------------------------------------------------------------------------
MathFunction::~MathFunction()
{
   // all math nodes are deleted in MathTree destructor (loj: 2008.11.03)
   //MessageInterface::ShowMessage
   //   (wxT("==> MathFunction::~MathFunction() deleting %s, %s\n"), GetTypeName().c_str(),
   //    GetName().c_str());
   
//    MathNode *left = GetLeft();
//    MathNode *right = GetRight();
   
//    if (left != NULL)
//       delete left;
   
//    if (right != NULL)
//       delete right;
}

//------------------------------------------------------------------------------
//  MathFunction(const MathFunction &mf)
//------------------------------------------------------------------------------
/**
 * Constructs the MathFunction object (copy constructor).
 * 
 * @param <mf> Object that is copied
 */
//------------------------------------------------------------------------------
MathFunction::MathFunction(const MathFunction &mf) :
   MathNode   (mf),
   leftNode   (mf.leftNode),
   rightNode  (mf.rightNode)
{
}

//------------------------------------------------------------------------------
//  MathFunction& operator=(const MathFunction &mf)
//------------------------------------------------------------------------------
/**
 * Sets one MathFunction object to match another (assignment operator).
 * 
 * @param <mf> The object that is copied.
 * 
 * @return this object, with the parameters set as needed.
 */
//------------------------------------------------------------------------------
MathFunction& MathFunction::operator=(const MathFunction &mf)
{
   if (this == &mf)
      return *this;
   
   GmatBase::operator=(mf);
   leftNode   = mf.leftNode;
   rightNode  = mf.rightNode;
   
   return *this;
}


//------------------------------------------------------------------------------
// bool SetChildren(MathNode *leftChild, MathNode *rightChild)()
//------------------------------------------------------------------------------
bool MathFunction::SetChildren(MathNode *leftChild, MathNode *rightChild)
{
   if (leftChild)
      leftNode = leftChild;
   else
      leftNode = NULL;
   
   if (rightChild)
      rightNode = rightChild;
   else
      rightNode = NULL;
   
   return true;
}


//------------------------------------------------------------------------------
// bool GetLeft()
//------------------------------------------------------------------------------
MathNode* MathFunction::GetLeft()
{
   return leftNode;
}


//------------------------------------------------------------------------------
// bool GetRight()
//------------------------------------------------------------------------------
MathNode* MathFunction::GetRight()
{
   return rightNode;
}


//------------------------------------------------------------------------------
// Real Evaluate()
//------------------------------------------------------------------------------
Real MathFunction::Evaluate()
{
   throw MathException(GetTypeName() + wxT(" cannot return Real"));
}


//------------------------------------------------------------------------------
// Rmatrix MatrixEvaluate()
//------------------------------------------------------------------------------
Rmatrix MathFunction::MatrixEvaluate()
{
   throw MathException(GetTypeName() + wxT(" cannot return Matrix"));
}
