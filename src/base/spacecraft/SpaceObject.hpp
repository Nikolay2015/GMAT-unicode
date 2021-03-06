//$Id: SpaceObject.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              SpaceObject
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CI63P
//
// Author: Darrel J. Conway, Thinking Systems, Inc.
// Created: 2004/7/24
//
/**
 * Defines the base class used for spacecraft and formations. 
 */
//------------------------------------------------------------------------------


#ifndef SpaceObject_hpp
#define SpaceObject_hpp

#include "GmatBase.hpp"
#include "SpacePoint.hpp"
#include "GmatState.hpp"
#include "SpaceObjectException.hpp"

class GMAT_API SpaceObject : public SpacePoint
{
public:
   SpaceObject(Gmat::ObjectType typeId, const wxString &typeStr,
               const wxString &instName);
   virtual ~SpaceObject();
   SpaceObject(const SpaceObject& so);
   SpaceObject&         operator=(const SpaceObject& so);
   
   virtual GmatState&   GetState();
   virtual Real         GetEpoch();
   virtual Real         SetEpoch(const Real ep);
   virtual bool         IsManeuvering();
   virtual void         IsManeuvering(bool mnvrFlag);
   virtual bool         ParametersHaveChanged();
   virtual void         ParametersHaveChanged(bool flag);
   
   virtual Integer GetParameterID(const wxString &str) const;
   virtual Real GetRealParameter(const Integer id) const;
   virtual Real GetRealParameter(const wxString &label) const;
   virtual Real SetRealParameter(const Integer id, const Real value);
   virtual Real SetRealParameter(const wxString &label, const Real value);
   
   virtual Real         GetRealParameter(const Integer id, const Integer row,
                                         const Integer col) const;
   virtual Real         GetRealParameter(const wxString &label, 
                                         const Integer row, 
                                         const Integer col) const;
   virtual Real         SetRealParameter(const Integer id, const Real value,
                                         const Integer row, const Integer col);
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value, const Integer row,
                                         const Integer col);
   virtual Real         SetRealParameter(const Integer id,
                                         const Real value,
                                         const Integer index);

   /// @todo Waiting for CoordinateSystems in Spacecraft, then see if needed
   virtual void SetOriginName(wxString cbName);
   virtual const wxString GetOriginName();
   virtual void SetOrigin(SpacePoint *cb);
   virtual SpacePoint* GetOrigin();
   
   virtual const Rvector6 GetMJ2000State(const A1Mjd &atTime);
   virtual const Rvector3 GetMJ2000Position(const A1Mjd &atTime);
   virtual const Rvector3 GetMJ2000Velocity(const A1Mjd &atTime);
   
   virtual wxString GetParameterText(const Integer id) const;
   virtual Gmat::ParameterType
                       GetParameterType(const Integer id) const;
   virtual wxString GetParameterTypeString(const Integer id) const;

   virtual void ClearLastStopTriggered();
   virtual void SetLastStopTriggered(const wxString &stopCondName);
   virtual const wxString GetLastStopTriggered();
   virtual bool WasLastStopTriggered(const wxString &stopCondName);
   
   bool HasEphemPropagated();
   void HasEphemPropagated(bool tf);
   
   /** 
    * Start on a fix for bug 648; these methods are not currently used, but
    * are in place for use when the single step publishing issues are ready 
    * to be worked.
    */
   void         HasPublished(bool tf);
   bool         HasPublished();

protected:
   /// The spacecraft state
   GmatState         state;
   /// true when a finite burn needs to be applied to this SpaceObject
   bool              isManeuvering;
   /// Reference SpacePoint for the data
   wxString       originName;
   /// Reference SpacePoint for the data
   SpacePoint        *origin;
   /// Flag indicating if the force model parms have changed
   bool              parmsChanged;
   /// The names of the last set of stopping conditions met
   StringArray       lastStopTriggered;
   /// Flag indicating if object has published data - not currently used
   bool              hasPublished;
   /// Flag indicating if the object has been propagated via an ephem
   bool					hasEphemPropagated;

   /// Enumerated parameter IDs   
   enum
   {
       EPOCH_PARAM = SpacePointParamCount,
       SpaceObjectParamCount
   };
   /// Array of supported parameters
   static const wxString PARAMETER_TEXT[SpaceObjectParamCount - 
                                           SpacePointParamCount];
   /// Array of parameter types
   static const Gmat::ParameterType PARAMETER_TYPE[SpaceObjectParamCount - 
                                                   SpacePointParamCount];
   
};

#endif // SpaceObject_hpp
