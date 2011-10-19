//$Id: Cowell.hpp 9513 2011-04-30 21:23:06Z djcinsb $
// **************************************************************************
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Waka Waktola
// Created: 2005/05/16
//
/**
 * @note This code is a shell for the Cowell integrator.  The integrator is not
 *       currently implemented.
 */
// **************************************************************************

#ifndef Cowell_hpp
#define Cowell_hpp

#include "gmatdefs.hpp"
#include "Integrator.hpp"

/**
 * The Cowell predictor-corrector integrator
 * 
 * This code is a shell for the Cowell integrator.  The integrator is not 
 * yet implemented.
 * 
 */
class GMAT_API Cowell : public PredictorCorrector
{
public:
   Cowell(const wxString &typeStr, const wxString &nomme = wxT(""));
   virtual ~Cowell(void);
   Cowell(const Cowell& bs);
   Cowell& operator=(const Cowell& bs);
    
   virtual GmatBase* Clone(void) const;
   virtual bool Initialize();
   virtual bool Step(Real dt);
   virtual bool Step(void);
   virtual bool RawStep(void);
   virtual Real EstimateError(void);
   virtual bool AdaptStep(Real maxerror);

   // Parameter accessor methods -- overridden from GmatBase
   virtual wxString         GetParameterText(const Integer id) const;
   virtual Integer             GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType GetParameterType(const Integer id) const;
   virtual wxString         GetParameterTypeString(const Integer id) const;
   virtual Real    GetRealParameter(const Integer id) const;
   virtual Real    GetRealParameter(const wxString &label) const;
   virtual Real    SetRealParameter(const Integer id, const Real value);
   virtual Real    SetRealParameter(const wxString &label, const Real value);
        
protected:

   enum
   {  /// Count of the parameters for this class
      CowellParamCount = IntegratorParamCount
   };
   
   // save for possible later use
   //static const wxString
   //PARAMETER_TEXT[CowellParamCount - IntegratorParamCount];
   //static const Gmat::ParameterType
   //PARAMETER_TYPE[CowellParamCount - IntegratorParamCount];

private:
   /// The estimated state
   Real *estimatedState;
};
#endif
