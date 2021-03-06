//$Id$
//------------------------------------------------------------------------------
//                              AttitudeException
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
// Author: Wendy Shoan/GSFC
// Created: 2006.02.16
//
/**
 * Implementation code for the SolarSystemException class.
 */
//------------------------------------------------------------------------------
#include "AttitudeException.hpp"

//---------------------------------
//  public methods
//---------------------------------

//------------------------------------------------------------------------------
//  AttitudeException(wxString details)
//------------------------------------------------------------------------------
/**
* Constructs an AttitudeException object (default constructor).
 */
//------------------------------------------------------------------------------

AttitudeException::AttitudeException(wxString details) :
BaseException  (wxT("Attitude exception: "), details)
{

}
AttitudeException::~AttitudeException()
{
}
