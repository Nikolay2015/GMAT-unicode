//$Id: PlanetaryEphem.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  PlanetaryEphem
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Wendy C. Shoan
// Created: 2004/02/18
//
/**
 * Implementation of the PlanetaryEphem class.
 *
 * @note This class was created (basically) from Swingby code.  Major mods were:
 * - changed #defines to static const parameters
 * - removed _MAX_PATH (not standard on platforms) - use new static
 *   const MAX_PATH_LEN instead
 */
//------------------------------------------------------------------------------

#include "PlanetaryEphem.hpp"
#include "TimeTypes.hpp"
#include <cstdio>  // Fix for header rearrangement in gcc 4.4

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  PlanetaryEphem(wxString withFileName)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the PlanetaryEphem class
 * (default constructor).
 *
 * @param <name> parameter indicating the full path name of the File.
 */
//------------------------------------------------------------------------------
PlanetaryEphem::PlanetaryEphem(wxString withFileName)
{
   itsName = withFileName;
   g_pef_dcb.full_path = withFileName;
   g_pef_dcb.recl           = 0;
   g_pef_dcb.fptr           = NULL;
   jdMjdOffset              = GmatTimeConstants::JD_JAN_5_1941;
}

//------------------------------------------------------------------------------
//  PlanetaryEphem(const PlanetaryEphem& pef)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the PlanetaryEphem class
 * (constructor).
 *
 * @param <pef> SlpFile object whose values to copy to the new File.
 */
//------------------------------------------------------------------------------
PlanetaryEphem::PlanetaryEphem(const PlanetaryEphem& pef)
{
   // set class data
   itsName         = pef.itsName;
   g_pef_dcb       = pef.g_pef_dcb;
   jdMjdOffset     = pef.jdMjdOffset;
}

//------------------------------------------------------------------------------
//  PlanetaryEphem& operator= (const PlanetaryEphem& pef)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the PlanetaryEphem class.
 *
 * @param <pef> the PlanetaryEphem object whose data to assign to wxT("this")
 *            File.
 *
 * @return wxT("this") PlanetaryEphem with data of input PlanetaryEphem pef.
 */
//------------------------------------------------------------------------------
PlanetaryEphem& PlanetaryEphem::operator=(const PlanetaryEphem& pef)
{
   if (this == &pef) return *this;
   itsName = pef.itsName;
   g_pef_dcb       = pef.g_pef_dcb;
   jdMjdOffset     = pef.jdMjdOffset;
   return *this;
}

//------------------------------------------------------------------------------
//  ~PlanetaryEphem()
//------------------------------------------------------------------------------
/**
 * Destructor for the PlanetaryEphem class.
 */
//------------------------------------------------------------------------------
PlanetaryEphem::~PlanetaryEphem()
{
   if (g_pef_dcb.fptr)  fclose(g_pef_dcb.fptr);   
}

//------------------------------------------------------------------------------
//  wxString GetName()
//------------------------------------------------------------------------------
/**
 * Returns the name of the file (full path name).
 *
 * @return the full path name of the file.
 */
//------------------------------------------------------------------------------
wxString PlanetaryEphem::GetName() const
{
   return itsName;
}
