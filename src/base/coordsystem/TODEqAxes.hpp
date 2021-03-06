//$Id: TODEqAxes.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  TODEqAxes
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
 * Definition of the TODEqAxes class.
 *
 */
//------------------------------------------------------------------------------

#ifndef TODEqAxes_hpp
#define TODEqAxes_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "AxisSystem.hpp"
#include "TrueOfDateAxes.hpp"

class GMAT_API TODEqAxes : public TrueOfDateAxes
{
public:

   // default constructor
   TODEqAxes(const wxString &itsName = wxT(""));
   // copy constructor
   TODEqAxes(const TODEqAxes &tod);
   // operator = for assignment
   const TODEqAxes& operator=(const TODEqAxes &tod);
   // destructor
   virtual ~TODEqAxes();
   
   // method to initialize the data
   virtual bool Initialize();

   // all classes derived from GmatBase must supply this Clone method;
   // this must be implemented in the 'leaf' classes
   virtual GmatBase*       Clone() const;

   // Parameter access methods - overridden from GmatBase
   /* placeholder - may be needed later
   virtual wxString     GetParameterText(const Integer id) const;     
   virtual Integer         GetParameterID(const wxString &str) const; 
   virtual Gmat::ParameterType
                           GetParameterType(const Integer id) const;
   virtual wxString     GetParameterTypeString(const Integer id) const;
    */
   
protected:

   enum
   {
      TODEqAxesParamCount = TrueOfDateAxesParamCount,
   };
   
   //static const wxString PARAMETER_TEXT[TODEqAxesParamCount - 
   //                                        TrueOfDateAxesParamCount];
   
   //static const Gmat::ParameterType PARAMETER_TYPE[TODEqAxesParamCount - 
   //                                                TrueOfDateAxesParamCount];
   
   virtual void CalculateRotationMatrix(const A1Mjd &atEpoch,
                                        bool forceComputation = false);

   // no additional data at this time
};
#endif // TODEqAxes_hpp
