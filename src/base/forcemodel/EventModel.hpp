//$Id$
//------------------------------------------------------------------------------
//                           EventModel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under NASA Prime
// Contract NNG10CP02C, Task Order 28
//
// Author: Darrel J. Conway, Thinking Systems, Inc.
// Created: Sep 7, 2011
//
/**
 * Definition of the ...
 */
//------------------------------------------------------------------------------


#ifndef EventModel_hpp
#define EventModel_hpp

#include "PhysicalModel.hpp"

/**
 * This PhysicalModel connects EventFunctions to Integrators.
 *
 * The EventModel class is a container class that takes a collection of
 * EventLocator objects, accesses the contained EventFunctions, and generates
 * derivative data for each EventFunction for use by GMAT's propagation
 * subsystem.
 */
class EventModel: public PhysicalModel
{
public:
   EventModel(const wxString &typeStr, const wxString &nomme = wxT(""));
   virtual ~EventModel();
   EventModel(const EventModel& em);
   EventModel& operator=(const EventModel& em);

   virtual bool Initialize();

//   // Support for extra derivative calcs - the EventFunction contributions here
//   virtual const IntegerArray& GetSupportedDerivativeIds();
//   virtual const StringArray&  GetSupportedDerivativeNames();

   // Methods used by the ODEModel to set the state indexes, etc
   virtual bool SupportsDerivative(Gmat::StateElementId id);
   virtual bool SetStart(Gmat::StateElementId id, Integer index,
                         Integer quantity);

   virtual bool GetDerivatives(Real * state, Real dt = 0.0, Integer order = 1,
         const Integer id = -1);

private:
   /// The list of active EventLocators
   std::vector<EventLocator*> *events;
};

#endif /* EventModel_hpp */