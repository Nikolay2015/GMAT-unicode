//$Id: AtmosphereFactory.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                         AtmosphereFactory
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
// Author: Wendy Shoan
// Created: 2004/08/12
//
/**
*  This class is the factory class for Atmospheres.
 */
//------------------------------------------------------------------------------
#ifndef AtmosphereFactory_hpp
#define AtmosphereFactory_hpp

#include "gmatdefs.hpp"
#include "Factory.hpp"
#include "AtmosphereModel.hpp"

class GMAT_API AtmosphereFactory : public Factory
{
public:
   AtmosphereModel*  CreateAtmosphereModel(const wxString &ofType,
                                           const wxString &withName = wxT(""),
                                           const wxString &forBody = wxT("Earth"));

   // default constructor
   AtmosphereFactory();
   // constructor
   AtmosphereFactory(StringArray createList);
   // copy constructor
   AtmosphereFactory(const AtmosphereFactory& fact);
   // assignment operator
   AtmosphereFactory& operator= (const AtmosphereFactory& fact);

   // destructor
   ~AtmosphereFactory();

protected:
      // protected data

private:


};

#endif // AtmosphereFactory_hpp




