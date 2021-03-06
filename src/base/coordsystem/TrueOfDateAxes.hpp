//$Id: TrueOfDateAxes.hpp 9513 2011-04-30 21:23:06Z djcinsb $
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
// Author: Wendy C. Shoan/GSFC/MAB
// Created: 2005/05/03
//
/**
 * Definition of the TrueOfDateAxes class.  This is the base class for those
 * DynamicAxes classes that implement True Of Date axis systems.
 *
 */
//------------------------------------------------------------------------------

#ifndef TrueOfDateAxes_hpp
#define TrueOfDateAxes_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "DynamicAxes.hpp"
#include "A1Mjd.hpp"

class GMAT_API TrueOfDateAxes : public DynamicAxes
{
public:

   // default constructor
   TrueOfDateAxes(const wxString &itsType,
                  const wxString &itsName = wxT(""));
   // copy constructor
   TrueOfDateAxes(const TrueOfDateAxes &tod);
   // operator = for assignment
   const TrueOfDateAxes& operator=(const TrueOfDateAxes &tod);
   // destructor
   virtual ~TrueOfDateAxes();
   
   // initializes the TrueOfDateAxes
   virtual bool Initialize(); 

   virtual GmatCoordinate::ParameterUsage UsesEopFile() const;
   virtual GmatCoordinate::ParameterUsage UsesItrfFile() const;
   virtual GmatCoordinate::ParameterUsage UsesNutationUpdateInterval() const;

protected:

   enum
   {
      TrueOfDateAxesParamCount = DynamicAxesParamCount
   };
    

};
#endif // TrueOfDateAxes_hpp
