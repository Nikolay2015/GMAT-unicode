//$Id: MJ2000EqAxes.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  MJ2000EqAxes
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
// Created: 2005/01/05
//
/**
 * Definition of the MJ2000EqAxes class.
 *
 */
//------------------------------------------------------------------------------

#ifndef MJ2000EqAxes_hpp
#define MJ2000EqAxes_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "AxisSystem.hpp"
#include "InertialAxes.hpp"

class GMAT_API MJ2000EqAxes : public InertialAxes
{
public:

   // default constructor
   MJ2000EqAxes(const wxString &itsName = wxT(""));
   // copy constructor
   MJ2000EqAxes(const MJ2000EqAxes &mj2000);
   // operator = for assignment
   const MJ2000EqAxes& operator=(const MJ2000EqAxes &mj2000);
   // destructor
   virtual ~MJ2000EqAxes();
   
   // method to initialize the data
   virtual bool Initialize();
   
   // all classes derived from GmatBase must supply this Clone method;
   // this must be implemented in the 'leaf' classes
   virtual GmatBase*       Clone(void) const;

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
      MJ2000EqAxesParamCount = InertialAxesParamCount,
   };
   
   //static const wxString PARAMETER_TEXT[MJ2000EqAxesParamCount - 
   //                                        InertialAxesParamCount];
   
   //static const Gmat::ParameterType PARAMETER_TYPE[MJ2000EqAxesParamCount - 
   //                                                InertialAxesParamCount];
   
   virtual void CalculateRotationMatrix(const A1Mjd &atEpoch,
                                        bool forceComputation = false);

   // no additional data at this time
};
#endif // MJ2000EqAxes_hpp
