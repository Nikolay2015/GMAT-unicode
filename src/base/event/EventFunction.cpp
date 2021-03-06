//$Id: EventFunction.cpp 10013 2011-11-16 18:46:12Z djcinsb $
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

#include "gmatdefs.hpp"
#include "EventFunction.hpp"
#include "EventException.hpp"

//------------------------------------------------------------------------------
// EventFunction(const wxString &typeStr)
//------------------------------------------------------------------------------
/**
 * Default constructor
 */
//------------------------------------------------------------------------------
EventFunction::EventFunction(const wxString &typeStr) :
   typeName          (typeStr),
   instanceName      (wxT("Base EventFunction (Name not set)")),
   eventData         (NULL),
   dataSize          (3),
   primary           (NULL),
   boundaryType      (wxT("Undetermined")),
   isStart           (false)
{
}


//------------------------------------------------------------------------------
// ~EventFunction()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
EventFunction::~EventFunction()
{
   if (eventData)
      delete [] eventData;
}


//------------------------------------------------------------------------------
// EventFunction(const EventFunction& ef)
//------------------------------------------------------------------------------
/**
 * Copy constructor
 *
 * @param ef The event function that is copied here
 */
//------------------------------------------------------------------------------
EventFunction::EventFunction(const EventFunction& ef) :
   typeName          (ef.typeName),
   instanceName      (ef.instanceName),
   eventData         (NULL),
   dataSize          (ef.dataSize),
   primary           (ef.primary),
   boundaryType      (ef.boundaryType),
   isStart           (ef.isStart)
{
}


//------------------------------------------------------------------------------
// EventFunction& operator=(const EventFunction& ef)
//------------------------------------------------------------------------------
/**
 * Assignment operator
 *
 * @param ef The event function that is copied here
 *
 * @return this instance, set to match ef.
 */
//------------------------------------------------------------------------------
EventFunction& EventFunction::operator=(const EventFunction& ef)
{
   if (this != &ef)
   {
      typeName     = ef.typeName;
      instanceName = ef.instanceName;
      dataSize     = ef.dataSize;

      if (eventData != NULL)
         delete [] eventData;
      if (ef.eventData != NULL)
      {
         eventData = new Real[dataSize];
         for (UnsignedInt i = 0; i < dataSize; ++i)
            eventData[i] = ef.eventData[i];
      }
      primary      = ef.primary;
      boundaryType = ef.boundaryType;
      isStart      = ef.isStart;
   }

   return *this;
}


//------------------------------------------------------------------------------
// wxString GetTypeName()
//------------------------------------------------------------------------------
/**
 * Retrieves the type name assigned to the event function.
 *
 * This string is used to identify the type of event being computed.  Examples
 * include Umbra, Contact, Penumbra, and Antumbra.
 *
 * @return String name for the event type
 */
//------------------------------------------------------------------------------
wxString EventFunction::GetTypeName()
{
   return typeName;
}


//------------------------------------------------------------------------------
// wxString GetName()
//------------------------------------------------------------------------------
/**
 * Retrieves the instance specific string describing the event.
 *
 * @return The name associated with the event function instance.
 *
 * @note This method is not currently used.
 */
//------------------------------------------------------------------------------
wxString EventFunction::GetName()
{
   return instanceName;
}

wxString EventFunction::GetPrimaryName()
{
   wxString retval = wxT("");

   if (primary != NULL)
      retval = primary->GetName();

   return retval;
}


//------------------------------------------------------------------------------
// bool SetPrimary(SpaceObject *so)
//------------------------------------------------------------------------------
/**
 * Sets the primary (target) object for the event function.
 *
 * The primary for event functions is the body -- usually a spacecraft -- that
 * is being evaluated for status of the event.
 *
 * @param so The object being evaluated
 *
 * @return true on success, false on failure
 */
//------------------------------------------------------------------------------
bool EventFunction::SetPrimary(SpaceObject *so)
{
   primary = so;
   return (so != NULL);
}


//------------------------------------------------------------------------------
// bool EventFunction::Initialize()
//------------------------------------------------------------------------------
/**
 * Prepares the event function for use
 *
 * @return true if initialization succeeds, false if it fails
 */
//------------------------------------------------------------------------------
bool EventFunction::Initialize()
{
   if (primary == NULL)
      throw EventException(wxT("Unable to initialize the ") + typeName +
            wxT(" EventFunction; the primary is not set."));

   // (Re)allocate the data array
   if (eventData != NULL)
      delete [] eventData;
   eventData = new Real[dataSize];

   return true;
}


//------------------------------------------------------------------------------
// Real* GetData()
//------------------------------------------------------------------------------
/**
 * Retrieves the most recently calculated event data
 *
 * @return The data, in a Real array sized to match the size of the event
 *         function data (typically 3 elements: epoch, value, and derivative)
 */
//------------------------------------------------------------------------------
Real* EventFunction::GetData()
{
   return eventData;
}


//------------------------------------------------------------------------------
// wxString EventFunction::GetBoundaryType()
//------------------------------------------------------------------------------
/**
 * Retrieves a string describing the type of boundary encountered
 *
 * @return The description, typically "Entry", "Exit", "Rise", or "Set"
 */
//------------------------------------------------------------------------------
wxString EventFunction::GetBoundaryType()
{
   // Default: slope positive at entry
   if (eventData[2] > 0.0)
      boundaryType = wxT("Entry");
   else
      boundaryType = wxT("Exit");

   return boundaryType;
}


//------------------------------------------------------------------------------
// bool EventFunction::IsEventEntry()
//------------------------------------------------------------------------------
/**
 * Retrieves a flag indicating if the event boundary is an entry or an exit
 *
 * @return true if the boundary is an entry location, false if an exit.
 */
//------------------------------------------------------------------------------
bool EventFunction::IsEventEntry()
{
   return (eventData[2] > 0.0 ? true : false);
}
