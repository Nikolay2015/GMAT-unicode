//$Id: TrueOfDateAxes.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  TrueOfDateAxes
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under 
// MOMS Task order 124.
//
// Author: Wendy C. Shoan
// Created: 2005/05/03
//
/**
 * Implementation of the TrueOfDateAxes class.  This is the base class for those
 * DynamicAxes classes that implement True Of Date axis systems.
 *
 */
//------------------------------------------------------------------------------

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "TrueOfDateAxes.hpp"
#include "DynamicAxes.hpp"

//---------------------------------
// static data
//---------------------------------
// none at this time

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  TrueOfDateAxes(const wxString &itsType,
//                 const wxString &itsName);
//---------------------------------------------------------------------------
/**
 * Constructs base TrueOfDateAxes structures used in derived classes
 * (default constructor).
 *
 * @param itsType GMAT script string associated with this type of object.
 * @param itsName Optional name for the object.  Defaults to "".
 *
 * @note There is no parameter free constructor for TrueOfDateAxes.  Derived 
 *       classes must pass in the typeId and typeStr parameters.
 */
//---------------------------------------------------------------------------
TrueOfDateAxes::TrueOfDateAxes(const wxString &itsType,
                               const wxString &itsName) :
DynamicAxes(itsType,itsName)
{
   objectTypeNames.push_back(wxT("TrueOfDateAxes"));
   parameterCount = TrueOfDateAxesParamCount;
}

//---------------------------------------------------------------------------
//  TrueOfDateAxes(const TrueOfDateAxes &tod);
//---------------------------------------------------------------------------
/**
 * Constructs base TrueOfDateAxes structures used in derived classes, by copying 
 * the input instance (copy constructor).
 *
 * @param tod  TrueOfDateAxes instance to copy to create "this" instance.
 */
//---------------------------------------------------------------------------
TrueOfDateAxes::TrueOfDateAxes(const TrueOfDateAxes &tod) :
DynamicAxes(tod)
{
}

//---------------------------------------------------------------------------
//  TrueOfDateAxes& operator=(const TrueOfDateAxes &tod)
//---------------------------------------------------------------------------
/**
 * Assignment operator for TrueOfDateAxes structures.
 *
 * @param tod The original that is being copied.
 *
 * @return Reference to this object
 */
//---------------------------------------------------------------------------
const TrueOfDateAxes& TrueOfDateAxes::operator=(const TrueOfDateAxes &tod)
{
   if (&tod == this)
      return *this;
   DynamicAxes::operator=(tod);   
   return *this;
}
//---------------------------------------------------------------------------
//  ~TrueOfDateAxes()
//---------------------------------------------------------------------------
/**
 * Destructor.
 */
//---------------------------------------------------------------------------
TrueOfDateAxes::~TrueOfDateAxes()
{
}


//---------------------------------------------------------------------------
//  bool Initialize()
//---------------------------------------------------------------------------
/**
 * Initialization method for this TrueOfDateAxes.
 *
 */
//---------------------------------------------------------------------------
bool TrueOfDateAxes::Initialize()
{
   DynamicAxes::Initialize();
//   if (originName == SolarSystem::EARTH_NAME) // so can use for other bodies as origin  WCS 2010.05.24
      InitializeFK5();
   return true;
}

GmatCoordinate::ParameterUsage TrueOfDateAxes::UsesEopFile() const
{
   return GmatCoordinate::REQUIRED;
}

GmatCoordinate::ParameterUsage TrueOfDateAxes::UsesItrfFile() const
{
   return GmatCoordinate::REQUIRED;
}

GmatCoordinate::ParameterUsage TrueOfDateAxes::UsesNutationUpdateInterval() const
{
   if (originName == SolarSystem::EARTH_NAME) 
      return GmatCoordinate::REQUIRED;
   return DynamicAxes::UsesNutationUpdateInterval();
}


//------------------------------------------------------------------------------
// public methods inherited from GmatBase
//------------------------------------------------------------------------------
