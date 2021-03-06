//$Id: FunctionFactory.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              FunctionFactory
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
// Author: Allison Greene
// Created: 2004/9/22
//
/**
 * Implementation code for the FunctionFactory class, responsible for creating
 * Function objects.
 */
//------------------------------------------------------------------------------


#include "FunctionFactory.hpp"
#include "MatlabFunction.hpp"
#include "GmatFunction.hpp"

//---------------------------------
//  public methods
//---------------------------------

//------------------------------------------------------------------------------
//  CreateFunction(const wxString &ofType, const wxString &withName)
//------------------------------------------------------------------------------
/**
 * This method creates and returns an object of the requested Function class
 *
 * @param <ofType> the Function object to create and return.
 */
//------------------------------------------------------------------------------
Function* FunctionFactory::CreateFunction(const wxString &ofType,
         const wxString &withName)
{
   if (ofType == wxT("MatlabFunction"))
      return new MatlabFunction(withName);
   else if (ofType == wxT("GmatFunction"))
      return new GmatFunction(withName);
   // add more here .......
   else {
      return NULL;   // doesn't match any known type of Function
   }
   
}


//------------------------------------------------------------------------------
//  FunctionFactory()
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class FunctionFactory.
 * (default constructor)
 *
 */
//------------------------------------------------------------------------------
FunctionFactory::FunctionFactory() :
    Factory     (Gmat::FUNCTION)
{
   if (creatables.empty())
   {
      creatables.push_back(wxT("MatlabFunction"));
      creatables.push_back(wxT("GmatFunction"));
   }
}

//------------------------------------------------------------------------------
//  FunctionFactory(StringArray createList)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the class FunctionFactory.
 *
 * @param <createList> list of creatable Function objects
 *
 */
//------------------------------------------------------------------------------
FunctionFactory::FunctionFactory(StringArray createList) :
    Factory     (createList, Gmat::FUNCTION)
{
}

//------------------------------------------------------------------------------
//  FunctionFactory(const FunctionFactory& fact)
//------------------------------------------------------------------------------
/**
   * This method creates an object of the class FunctionFactory (called by
   * copy constructors of derived classes).  (copy constructor)
   *
   * @param <fact> the factory object to copy to wxT("this") factory.
   */
//------------------------------------------------------------------------------
FunctionFactory::FunctionFactory(const FunctionFactory& fact) :
    Factory     (fact)
{
   if (creatables.empty())
   {
      creatables.push_back(wxT("MatlabFunction"));
      creatables.push_back(wxT("GmatFunction"));
   }
}

//------------------------------------------------------------------------------
//  FunctionFactory& operator= (const FunctionFactory& fact)
//------------------------------------------------------------------------------
/**
   * Assignment operator for the FunctionFactory base class.
   *
   * @param <fact> the FunctionFactory object whose data to assign to wxT("this") factory.
   *
   * @return wxT("this") FunctionFactory with data of input factory fact.
   */
//------------------------------------------------------------------------------
FunctionFactory& FunctionFactory::operator=(const FunctionFactory& fact)
{
   Factory::operator=(fact);
   return *this;
}

//------------------------------------------------------------------------------
// ~FunctionFactory()
//------------------------------------------------------------------------------
/**
   * Destructor for the FunctionFactory class.
   *
   */
//------------------------------------------------------------------------------
FunctionFactory::~FunctionFactory()
{
   // deletes handled by Factory destructor
}

//---------------------------------
//  protected methods
//---------------------------------

//---------------------------------
//  private methods
//---------------------------------

