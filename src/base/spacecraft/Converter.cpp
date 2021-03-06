//$Id: Converter.cpp 9513 2011-04-30 21:23:06Z djcinsb $ 
//------------------------------------------------------------------------------
//                                 Converter
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author:  Joey Gurganus
// Created: 2004/03/22
//
/**
 * Implements the Converter base class
 */
//------------------------------------------------------------------------------

#include "Converter.hpp"

//-------------------------------------
// public methods
//-------------------------------------

//---------------------------------------------------------------------------
//  Converter()
//---------------------------------------------------------------------------
/**
 * Creates default constructor.
 *
 */
Converter::Converter()  
{
    type = wxT("unknown");
}

//---------------------------------------------------------------------------
//  Converter(wxString &type)
//---------------------------------------------------------------------------
/**
 * Creates constructors with parameters.
 *
 * @param <typeStr> GMAT script string associated with this type of object.
 *
 */
Converter::Converter(const wxString &type) :
    type  (type)
{
}

//---------------------------------------------------------------------------
//  Converter(const Converter &converter)
//---------------------------------------------------------------------------
/**
 * Copy Constructor for base Converter structures.
 *
 * @param <converter> The original that is being copied.
 */
Converter::Converter(const Converter &converter) :
    type (converter.type)
{
}

//---------------------------------------------------------------------------
//  ~Converter()
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
Converter::~Converter()
{
}

//---------------------------------------------------------------------------
//  Converter& operator=(const Converter &a)
//---------------------------------------------------------------------------
/**
 * Assignment operator for Converter structures.
 *
 * @param <a> The original that is being copied.
 *
 * @return Reference to this object
 */
Converter& Converter::operator=(const Converter &converter)
{
    // Don't do anything if copying self
    if (&converter == this)
        return *this;

    // Duplicate the member data        
    type    = converter.type;

    return *this;
}

//---------------------------------------------------------------------------
//  wxString GetType()
//---------------------------------------------------------------------------
/**
 * Get the type name.
 * 
 * @return the type name 
 *
 */
wxString Converter::GetType()
{
    return type;
}

//---------------------------------------------------------------------------
//  void SetType(const wxString &type)
//---------------------------------------------------------------------------
/**
 * Set the type name.
 * 
 * @param <type>  Type name 
 *
 */
void Converter::SetType(const wxString &type)
{
    this->type = type;
}
