//$Id: BurnParameters.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                            File: BurnParameters.cpp
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
// Author: Linda Jun
// Created: 2005/05/27
//
/**
 * Implements Burn related Parameter classes.
 *   ImpBurnElements
 */
//------------------------------------------------------------------------------

#include "BurnParameters.hpp"
#include "ColorTypes.hpp"

//#define DEBUG_BURN_PARAM 1


// To use preset colors, uncomment this line:
//#define USE_PREDEFINED_COLORS

//==============================================================================
//                              ImpBurnElements
//==============================================================================
/**
 * Implements ImpulsiveBurn element Parameters.
 *    Element1, Element2, Element3
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// ImpBurnElements(const wxString &type, const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <type> type of the parameter
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
ImpBurnElements::ImpBurnElements(const wxString &type, const wxString &name,
                                 GmatBase *obj)
   : BurnReal(name, type, Gmat::IMPULSIVE_BURN, obj,
              wxT("ImpulsiveBurn Element"), wxT("Km/s"), GmatParam::NO_DEP, false)
{
   #ifdef USE_PREDEFINED_COLORS
      mColor = GmatColor::RED32;
   #endif
   
   if (type == wxT("Element1") || type == wxT("V"))
      mElementId = ELEMENT1;
   else if (type == wxT("Element2") || type == wxT("N"))
      mElementId = ELEMENT2;
   else if (type == wxT("Element3") || type == wxT("B"))
      mElementId = ELEMENT3;
   else
      mElementId = -1;
}


//------------------------------------------------------------------------------
// ImpBurnElements(const ImpBurnElements &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
ImpBurnElements::ImpBurnElements(const ImpBurnElements &copy)
   : BurnReal(copy)
{
   mElementId = copy.mElementId;
}


//------------------------------------------------------------------------------
// ImpBurnElements& operator=(const ImpBurnElements &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
ImpBurnElements& ImpBurnElements::operator=(const ImpBurnElements &right)
{
   if (this != &right)
   {
      BurnReal::operator=(right);
      mElementId = right.mElementId;
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// ~ImpBurnElements()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
ImpBurnElements::~ImpBurnElements()
{
}


//-------------------------------------
// methods inherited from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool ImpBurnElements::Evaluate()
{
   mRealValue = BurnData::GetReal(mElementId);

   #ifdef DEBUG_BURN_EVAL
   MessageInterface::ShowMessage
      (wxT("ImpBurnElements::Evaluate() mRealValue=%f\n"), mRealValue);
   #endif
   
   if (mRealValue == BurnData::BURN_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual void SetReal(Real val)
//------------------------------------------------------------------------------
/**
 * Sets value to the owner of the parameter.
 *
 */
//------------------------------------------------------------------------------
void ImpBurnElements::SetReal(Real val)
{
   BurnData::SetReal(mElementId, val);
   RealVar::SetReal(val);
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* ImpBurnElements::Clone(void) const
{
   return new ImpBurnElements(*this);
}


