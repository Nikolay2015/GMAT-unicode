//$Id: MOEEcAxes.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  MOEEcAxes
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
// Created: 2005/05/11
//
/**
 * Definition of the MOEEcAxes class.
 *
 */
//------------------------------------------------------------------------------

#ifndef MOEEcAxes_hpp
#define MOEEcAxes_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "AxisSystem.hpp"
#include "InertialAxes.hpp"

class GMAT_API MOEEcAxes : public InertialAxes
{
public:

   // default constructor
   MOEEcAxes(const wxString &itsName = wxT(""));
   // copy constructor
   MOEEcAxes(const MOEEcAxes &moe);
   // operator = for assignment
   const MOEEcAxes& operator=(const MOEEcAxes &moe);
   // destructor
   virtual ~MOEEcAxes();
   
   // method to initialize the data
   virtual bool Initialize();

   virtual GmatCoordinate::ParameterUsage UsesEpoch() const;
   //virtual GmatCoordinate::ParameterUsage UsesEopFile() const;
   //virtual GmatCoordinate::ParameterUsage UsesItrfFile() const;
   
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
      MOEEcAxesParamCount = InertialAxesParamCount,
   };
   
   //static const wxString PARAMETER_TEXT[MOEEcAxesParamCount - 
   //                                        InertialAxesParamCount];
   
   //static const Gmat::ParameterType PARAMETER_TYPE[MOEEcAxesParamCount - 
   //                                                InertialAxesParamCount];
   
   virtual void CalculateRotationMatrix(const A1Mjd &atEpoch,
                                        bool forceComputation = false);

   // no additional data at this time
};
#endif // MOEEcAxes_hpp
