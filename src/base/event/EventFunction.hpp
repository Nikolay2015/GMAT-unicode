//$Id: EventFunction.hpp 9969 2011-10-21 22:56:49Z djcinsb $
//------------------------------------------------------------------------------
//                           EventFunction
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
// Created: Jul 6, 2011
//
/**
 * Base class defining interfaces for event functions.
 */
//------------------------------------------------------------------------------


#ifndef EventFunction_hpp
#define EventFunction_hpp

#include "GmatBase.hpp"
#include "SpaceObject.hpp"

/**
 * Base class for event function classes
 *
 * The event function classes define continuous, differentiable functions that
 * pass through zero at locations that match event boundaries, and that are
 * (by default, but overridable) positive definite for the interval over which
 * the event is occurring.
 *
 * An example of the event functions in GMAT is the umbra event function, which
 * passes from negative values to positive when a spacecraft enters the umbral
 * cone of an obscuring body, and from positive to negative on exit.
 */
class GMAT_API EventFunction //: public GmatBase
{
public:
   EventFunction(const wxString &typeStr);
   virtual ~EventFunction();
   EventFunction(const EventFunction& ef);
   EventFunction& operator=(const EventFunction& ef);

   // Methods used to identify the EventFunction
   wxString GetTypeName();
   wxString GetName();
   wxString GetPrimaryName();

   virtual bool SetPrimary(SpaceObject *so);
   virtual bool Initialize();
   virtual Real* Evaluate(GmatEpoch atEpoch = -1.0, Real* forState = NULL) = 0;
   Real* GetData();
   virtual wxString GetBoundaryType();
   virtual bool IsEventEntry();

protected:
   /// Event function type
   wxString       typeName;
   /// Event function descriptor
   wxString       instanceName;
   /// Current/most recently evaluated epoch, event value, derivative
   Real              eventData[3];
   /// SpaceObject that plays the role of “target” in the event computations.
   SpaceObject       *primary;
   /// Event boundary type; defaults to "Entry" or "Exit"
   wxString       boundaryType;
   /// Boolean indicating is the boundary is an entry or exit
   bool              isStart;
};

#endif /* EventFunction_hpp */
