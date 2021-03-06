//$Id: CoordinateSystemException.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              CoordinateSystemException
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under 
// MOMS Task order 124.
//
// Author: Wendy Shoan
// Created: 2004/12/22
//
/**
 * Implementation code for the CoordinateSystemException class.
 */
//------------------------------------------------------------------------------
#include "CoordinateSystemException.hpp"


//---------------------------------
//  public methods
//---------------------------------

//------------------------------------------------------------------------------
//  CoordinateSystemException(wxString details)
//------------------------------------------------------------------------------
/**
* Constructs an CoordinateSystemException object (default constructor).
 */
//------------------------------------------------------------------------------

CoordinateSystemException::CoordinateSystemException(wxString details) :
BaseException  (wxT("CoordinateSystem exception: "), details)
{

}

