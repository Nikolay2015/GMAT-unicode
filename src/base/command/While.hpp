//$Id: While.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                While
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P
//
// Author:  Wendy Shoan/GSFC/MAB
// Created: 2004/08/10
//
/**
* Definition for the While command class
 */
//------------------------------------------------------------------------------


#ifndef While_hpp
#define While_hpp

#include "gmatdefs.hpp"
#include "ConditionalBranch.hpp"
#include "Parameter.hpp"

/**
* Command that manages processing for entry to the While statement
 *
 * The While command manages the While statement.
 *
 */
class GMAT_API While : public ConditionalBranch
{
public:
   // default constructor
   While();
   // copy constructor
   While(const While &wc);
   // operator =
   While& operator=(const While &wc);
   // destructor
   virtual ~While(void);

   // Inherited methods that need some enhancement from the base class
   virtual bool         Append(GmatCommand *cmd);

   // Methods used to run the command
   virtual bool         Execute();

   // inherited from GmatBase
   // inherited from GmatBase
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   virtual Integer      GetIntegerParameter(const Integer id) const;
   virtual Integer      SetIntegerParameter(const Integer id,
                                            const Integer value);
   virtual Integer      GetIntegerParameter(const wxString &label) const;
   virtual Integer      SetIntegerParameter(const wxString &label,
                                            const Integer value);
   virtual GmatBase*    Clone() const;
   virtual const wxString&
                        GetGeneratingString(Gmat::WriteMode mode,
                                            const wxString &prefix,
                                            const wxString &useName);

protected:

      enum
   {
      NEST_LEVEL = ConditionalBranchParamCount,
      WhileParamCount
   };
   
   static const wxString PARAMETER_TEXT[WhileParamCount - ConditionalBranchParamCount];
   
   static const Gmat::ParameterType PARAMETER_TYPE[WhileParamCount - ConditionalBranchParamCount];
   
   /// Counter to track how deep the If nesting is
   Integer                  nestLevel;
   /// The object array used in GetRefObjectArray()
   //ObjectArray              objectArray;

};
#endif  // While_hpp
