//$Id: EventLocator.cpp 10031 2011-12-01 21:30:50Z djcinsb $
//------------------------------------------------------------------------------
//                           EventLocator
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
 * Implementation of the the event locator base class
 */
//------------------------------------------------------------------------------


#include "EventLocator.hpp"
#include "EventException.hpp"
#include "FileManager.hpp"      // for GetPathname()
#include "MessageInterface.hpp"


//#define DEBUG_DUMPEVENTDATA

#ifdef DEBUG_DUMPEVENTDATA
   #include <fstream>
   std::ofstream dumpfile("LocatorData.txt");
#endif

//------------------------------------------------------------------------------
// Static data
//------------------------------------------------------------------------------
const wxString
EventLocator::PARAMETER_TEXT[EventLocatorParamCount - GmatBaseParamCount] =
{
   wxT("Spacecraft"),        // SATNAMES,
   wxT("Tolerance"),         // TOLERANCE,
   wxT("Filename"),           // EVENT_FILENAME,
   wxT("IsActive"),           // IS_ACTIVE
   wxT("ShowPlot"),           // SHOW_PLOT
   wxT("Epoch"),             // EPOCH (Read only)
   wxT("EventFunction")      // EVENT_FUNCTION (Read only)
};

const Gmat::ParameterType
EventLocator::PARAMETER_TYPE[EventLocatorParamCount - GmatBaseParamCount] =
{
//   Gmat::STRINGARRAY_TYPE,       // SATNAMES,
   Gmat::OBJECTARRAY_TYPE,       // SATNAMES,
   Gmat::REAL_TYPE,              // TOLERANCE,
   Gmat::FILENAME_TYPE,          // EVENT_FILENAME,
   Gmat::BOOLEAN_TYPE,           // IS_ACTIVE
   Gmat::BOOLEAN_TYPE,           // SHOW_PLOT
   Gmat::REAL_TYPE,              // EPOCH (Read only)
   Gmat::RVECTOR_TYPE            // EVENT_FUNCTION (Read only)
};


//------------------------------------------------------------------------------
// Public Methods
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// EventLocator(const wxString &typeStr, const wxString &nomme) :
//------------------------------------------------------------------------------
/**
 * Default constructor
 *
 * @param typeStr Type name of the locator
 * @param nomme Name of this instance of the locator
 */
//------------------------------------------------------------------------------
EventLocator::EventLocator(const wxString &typeStr,
      const wxString &nomme) :
   GmatBase       (Gmat::EVENT_LOCATOR, typeStr, nomme),
   filename       (wxT("LocatedEvents.txt")),
   efCount        (0),
   lastData       (NULL),
   lastEpochs     (NULL),
   isActive       (true),
   showPlot       (false),
   eventTolerance (1.0e-3),
   solarSys       (NULL)
{
   objectTypes.push_back(Gmat::EVENT_LOCATOR);
   objectTypeNames.push_back(wxT("EventLocator"));
}


//------------------------------------------------------------------------------
// ~EventLocator()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
EventLocator::~EventLocator()
{
//   MessageInterface::ShowMessage("Deleting event locator %s <%p>\n",
//         instanceName.c_str(), this);

   if (lastData != NULL)
      delete [] lastData;

   if (lastEpochs != NULL)
      delete [] lastEpochs;

   // todo: Delete the member EventFunctions
}


//------------------------------------------------------------------------------
// EventLocator(const EventLocator& el)
//------------------------------------------------------------------------------
/**
 * Copy constructor
 *
 * @param el The locator being copied here
 */
//------------------------------------------------------------------------------
EventLocator::EventLocator(const EventLocator& el) :
   GmatBase          (el),
   filename          (el.filename),
   efCount           (0),
   lastData          (NULL),
   lastEpochs        (NULL),
   isActive          (el.isActive),
   showPlot          (el.showPlot),
   satNames          (el.satNames),
   targets           (el.targets),
   eventTolerance    (el.eventTolerance),
   solarSys          (el.solarSys)
{
}


//------------------------------------------------------------------------------
// EventLocator& EventLocator::operator=(const EventLocator& el)
//------------------------------------------------------------------------------
/**
 * Assignment operator
 *
 * @param el The locator being copied here
 *
 * @return this instance, set to match el.
 */
//------------------------------------------------------------------------------
EventLocator& EventLocator::operator=(const EventLocator& el)
{
   if (this != &el)
   {
      GmatBase::operator =(el);

      filename       = el.filename;
      efCount        = 0;
      if (lastData != NULL)
         delete [] lastData;
      if (lastEpochs != NULL)
         delete [] lastEpochs;
      lastData       = NULL;
      lastEpochs     = NULL;
      isActive       = el.isActive;
      showPlot       = el.showPlot;
      satNames       = el.satNames;
      targets        = el.targets;
      eventTolerance = el.eventTolerance;
      solarSys       = el.solarSys;

      eventFunctions.clear();
      maxSpan.clear();
      lastSpan.clear();
      stateIndices.clear();
      associateIndices.clear();
   }

   return *this;
}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieves the text description of the object parameter with input id
 *
 * @param id The ID for the parameter
 *
 * @return The string used to script the parameter
 */
//------------------------------------------------------------------------------
wxString EventLocator::GetParameterText(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < EventLocatorParamCount)
      return PARAMETER_TEXT[id - GmatBaseParamCount];
   return GmatBase::GetParameterText(id);
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * Retrieves the parameter ID associated with a string
 *
 * @param str The string associated with the ID
 *
 * @return The ID
 */
//------------------------------------------------------------------------------
Integer EventLocator::GetParameterID(const wxString &str) const
{
   for (Integer i = GmatBaseParamCount; i < EventLocatorParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - GmatBaseParamCount])
         return i;
   }

   return GmatBase::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieves the type of a parameter
 *
 * @param id The ID for the parameter
 *
 * @return The parameter's type
 */
//------------------------------------------------------------------------------
Gmat::ParameterType EventLocator::GetParameterType(const Integer id) const
{
   if (id >= GmatBaseParamCount && id < EventLocatorParamCount)
      return PARAMETER_TYPE[id - GmatBaseParamCount];

   return GmatBase::GetParameterType(id);
}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieves a text description of the type of a parameter
 *
 * @param id The ID for the parameter
 *
 * @return The parameter's type description
 */
//------------------------------------------------------------------------------
wxString EventLocator::GetParameterTypeString(const Integer id) const
{
   return GmatBase::PARAM_TYPE_STRING[GetParameterType(id)];
}


//------------------------------------------------------------------------------
// bool IsParameterReadOnly(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Determines if the parameter is read only or writable when saved
 *
 * @param id The ID for the parameter
 *
 * @return true if the parameter is not writable, false if it is
 */
//------------------------------------------------------------------------------
bool EventLocator::IsParameterReadOnly(const Integer id) const
{
   if ((id == IS_ACTIVE) || (id == EPOCH) || (id == EVENT_FUNCTION))
      return true;

   return GmatBase::IsParameterReadOnly(id);
}


//------------------------------------------------------------------------------
// bool IsParameterReadOnly(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Determines if the parameter is read only or writable when saved
 *
 * @param label The string description of the parameter
 *
 * @return true if the parameter is not writable, false if it is
 */
//------------------------------------------------------------------------------
bool EventLocator::IsParameterReadOnly(const wxString &label) const
{
   return IsParameterReadOnly(GetParameterID(label));
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a real parameter
 *
 * @param id The ID of the parameter
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
Real EventLocator::GetRealParameter(const Integer id) const
{
   if (id == TOLERANCE)
      return eventTolerance;

   if (id == EPOCH)
   {
      if (targets.size() > 0)
         return targets[0]->GetEpoch();
      return -1.0;
   }

   return GmatBase::GetRealParameter(id);
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * Sets the value for a real parameter
 *
 * @param id The ID of the parameter
 * @param value The new value for the parameter
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
Real EventLocator::SetRealParameter(const Integer id, const Real value)
{
   if (id == TOLERANCE)
   {
      if (value > 0.0)
         eventTolerance = value;
      else
         throw EventException(
               wxT("Located event tolerance value must be a real number >= 0.0"));

      return eventTolerance;
   }

   if (id == EPOCH)
   {
      return 0.0;
   }

   return GmatBase::SetRealParameter(id, value);
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a real parameter from a vector of Reals
 *
 * @param id The ID of the parameter
 * @param index The index into the vector
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
Real EventLocator::GetRealParameter(const Integer id, const Integer index) const
{
   if (id == EVENT_FUNCTION)
   {
      Real retval = 0.0;
      if (index < (Integer)eventFunctions.size())
      {
         Real *data = eventFunctions[index]->Evaluate();
         retval = data[1];
      }
      return retval;       // todo Return the event function value
   }
   return GmatBase::GetRealParameter(id, index);
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const Integer id, const Integer row,
//       const Integer col) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a real parameter from an array of Reals
 *
 * @param id The ID of the parameter
 * @param row The row index into the array
 * @param col The column index into the array
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
Real EventLocator::GetRealParameter(const Integer id, const Integer row,
                                      const Integer col) const
{
   return GmatBase::GetRealParameter(id, row, col);
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value,
//       const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets the value for a real parameter in a vector of Reals
 *
 * @param id The ID of the parameter
 * @param value The new value for the parameter
 * @param index The index into the vector
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
Real EventLocator::SetRealParameter(const Integer id, const Real value,
      const Integer index)
{
   if (id == EVENT_FUNCTION)
      return 0.0;       // todo Return the event function value

   return GmatBase::SetRealParameter(id, value, index);
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value, const Integer row,
//       const Integer col)
//------------------------------------------------------------------------------
/**
 * Sets the value for a real parameter in an array of Reals
 *
 * @param id The ID of the parameter
 * @param value The new value for the parameter
 * @param row The row index into the array
 * @param col The column index into the array
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
Real EventLocator::SetRealParameter(const Integer id, const Real value,
                                      const Integer row, const Integer col)
{
   return GmatBase::SetRealParameter(id, value, row, col);
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a real parameter
 *
 * @param label The script string of the parameter
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
Real EventLocator::GetRealParameter(const wxString &label) const
{
   return GetRealParameter(GetParameterID(label));
}

//------------------------------------------------------------------------------
// Real SetRealParameter(const wxString &label, const Real value)
//------------------------------------------------------------------------------
/**
 * Sets the value for a real parameter
 *
 * @param label The script string of the parameter
 * @param value The new value for the parameter
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
Real EventLocator::SetRealParameter(const wxString &label,
                                      const Real value)
{
   return SetRealParameter(GetParameterID(label), value);
}

//------------------------------------------------------------------------------
// Real GetRealParameter(const wxString &label, const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a real parameter from a vector of Reals
 *
 * @param label The script string of the parameter
 * @param index The index into the vector
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
Real EventLocator::GetRealParameter(const wxString &label,
                                      const Integer index) const
{
   return GetRealParameter(GetParameterID(label), index);
}

//------------------------------------------------------------------------------
// Real SetRealParameter(const wxString &label, const Real value,
//       const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets the value for a real parameter in a vector of Reals
 *
 * @param label The script string of the parameter
 * @param value The new value for the parameter
 * @param index The index into the vector
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
Real EventLocator::SetRealParameter(const wxString &label,
                                      const Real value,
                                      const Integer index)
{
   return SetRealParameter(GetParameterID(label), value, index);
}

//------------------------------------------------------------------------------
// Real GetRealParameter(const wxString &label, const Integer row,
//       const Integer col) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a real parameter from an array of Reals
 *
 * @param label The script string of the parameter
 * @param row The row index into the array
 * @param col The column index into the array
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
Real EventLocator::GetRealParameter(const wxString &label,
                                      const Integer row,
                                      const Integer col) const
{
   return GetRealParameter(GetParameterID(label), row, col);
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const wxString &label, const Real value,
//       const Integer row, const Integer col)
//------------------------------------------------------------------------------
/**
 * Sets the value for a real parameter in an array of Reals
 *
 * @param label The script string of the parameter
 * @param value The new value for the parameter
 * @param row The row index into the array
 * @param col The column index into the array
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
Real EventLocator::SetRealParameter(const wxString &label,
                                      const Real value, const Integer row,
                                      const Integer col)
{
   return SetRealParameter(GetParameterID(label), value, row, col);
}

const Rvector& EventLocator::GetRvectorParameter(const Integer id) const
{
   if (id == EVENT_FUNCTION)
      return functionValues;

   return GmatBase::GetRvectorParameter(id);
}

const Rvector& EventLocator::SetRvectorParameter(const Integer id,
      const Rvector &value)
{
   if (id == EVENT_FUNCTION)
   {
      Integer size = (value.GetSize() < functionValues.GetSize() ?
            value.GetSize() : functionValues.GetSize());
      for (Integer i = 0; i < size; ++i)
         functionValues[i] = value[i];
      return functionValues;
   }

   return GmatBase::SetRvectorParameter(id, value);
}

const Rvector& EventLocator::GetRvectorParameter(const wxString &label) const
{
   return GetRvectorParameter(GetParameterID(label));
}

const Rvector& EventLocator::SetRvectorParameter(const wxString &label,
      const Rvector &value)
{
   return SetRvectorParameter(GetParameterID(label), value);
}

//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a string parameter
 *
 * @param id The ID of the parameter
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
wxString EventLocator::GetStringParameter(const Integer id) const
{
   if (id == EVENT_FILENAME)
      return filename;

   return GmatBase::GetStringParameter(id);
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
/**
 * Sets the value for a string parameter
 *
 * @param id The ID of the parameter
 * @param value The new value for the parameter
 *
 * @return true if the string was set, false if not
 */
//------------------------------------------------------------------------------
bool EventLocator::SetStringParameter(const Integer id,
      const wxString &value)
{
   if (id == EVENT_FILENAME)
   {
      if (value != wxT(""))
      {
         filename = value;
         return true;
      }
      return false;
   }

   if (id == SATNAMES)
   {
      if (find(satNames.begin(), satNames.end(), value) == satNames.end())
      {
         satNames.push_back(value);
         targets.push_back(NULL);
      }
      return true;
   }

   return GmatBase::SetStringParameter(id, value);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a string parameter from a vector of strings
 *
 * @param id The ID of the parameter
 * @param index The index into the vector
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
wxString EventLocator::GetStringParameter(const Integer id,
      const Integer index) const
{
   if (id == SATNAMES)
   {
      if (index < (Integer)satNames.size())
         return satNames[index];
      else
         throw EventException(
               wxT("Index out of range when trying to access spacecraft name for ") +
               instanceName);
   }

   return GmatBase::GetStringParameter(id, index);
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value,
//       const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets the value for a string parameter in a vector of strings
 *
 * @param id The ID of the parameter
 * @param value The new value for the parameter
 * @param index The index into the vector
 *
 * @return true if the string was set, false if not
 */
//------------------------------------------------------------------------------
bool EventLocator::SetStringParameter(const Integer id,
      const wxString &value, const Integer index)
{
   if (id == SATNAMES)
   {
      if (index < (Integer)satNames.size())
      {
         satNames[index] = value;
         return true;
      }
      else
      {
         satNames.push_back(value);
         targets.push_back(NULL);
         return true;
      }
   }

   return GmatBase::SetStringParameter(id, value, index);
}


//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a StringArray parameter
 *
 * @param id The ID of the parameter
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
const StringArray& EventLocator::GetStringArrayParameter(const Integer id) const
{
   if (id == SATNAMES)
      return satNames;

   return GmatBase::GetStringArrayParameter(id);
}


//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const Integer id,
//       const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a StringArray parameter from a vector of StringArrays
 *
 * @param id The ID of the parameter
 * @param index The index of the StringArray in the vector
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
const StringArray& EventLocator::GetStringArrayParameter(const Integer id,
                                             const Integer index) const
{
   return GmatBase::GetStringArrayParameter(id, index);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a string parameter
 *
 * @param label The script string of the parameter
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
wxString EventLocator::GetStringParameter(const wxString &label) const
{
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
/**
 * Sets the value for a string parameter
 *
 * @param label The script string of the parameter
 * @param value The new value for the parameter
 *
 * @return true if the string was set, false if not
 */
//------------------------------------------------------------------------------
bool EventLocator::SetStringParameter(const wxString &label,
                                        const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label,
//       const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a string parameter from a vector of strings
 *
 * @param label The script string of the parameter
 * @param index The index into the vector
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
wxString EventLocator::GetStringParameter(const wxString &label,
      const Integer index) const
{
   return GetStringParameter(GetParameterID(label), index);
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value,
//       const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets the value for a string parameter in a vector of strings
 *
 * @param label The script string of the parameter
 * @param value The new value for the parameter
 * @param index The index into the vector
 *
 * @return true if the string was set, false if not
 */
//------------------------------------------------------------------------------
bool EventLocator::SetStringParameter(const wxString &label,
      const wxString &value, const Integer index)
{
   return SetStringParameter(GetParameterID(label), value, index);
}

//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a StringArray parameter
 *
 * @param label The script string of the parameter
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
const StringArray& EventLocator::GetStringArrayParameter(const wxString &label) const
{
   return GetStringArrayParameter(GetParameterID(label));
}

//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const wxString &label,
//       const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a StringArray parameter from a vector of StringArrays
 *
 * @param label The script string of the parameter
 * @param index The index of the StringArray in the vector
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
const StringArray& EventLocator::GetStringArrayParameter(const wxString &label,
                                             const Integer index) const
{
   return GetStringArrayParameter(GetParameterID(label), index);
}


//------------------------------------------------------------------------------
// bool GetBooleanParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a Boolean parameter
 *
 * @param id The ID of the parameter
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
bool EventLocator::GetBooleanParameter(const Integer id) const
{
   if (id == IS_ACTIVE)
      return isActive;
   if (id == SHOW_PLOT)
      return showPlot;

   return GmatBase::GetBooleanParameter(id);
}


//------------------------------------------------------------------------------
// bool SetBooleanParameter(const Integer id, const bool value)
//------------------------------------------------------------------------------
/**
 * Sets the value for a Boolean parameter
 *
 * @param id The ID of the parameter
 * @param value The new value for the parameter
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
bool EventLocator::SetBooleanParameter(const Integer id, const bool value)
{
   if (id == IS_ACTIVE)
   {
      isActive = value;
      return isActive;
   }
   if (id == SHOW_PLOT)
   {
      showPlot = value;
      return showPlot;
   }
   return GmatBase::SetBooleanParameter(id, value);
}


//------------------------------------------------------------------------------
// bool GetBooleanParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a Boolean parameter from a vector of Booleans
 *
 * @param id The ID of the parameter
 * @param index The index into the vector
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
bool EventLocator::GetBooleanParameter(const Integer id,
      const Integer index) const
{
   return GmatBase::GetBooleanParameter(id, index);
}


//------------------------------------------------------------------------------
// bool SetBooleanParameter(const Integer id, const bool value,
//       const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets the value for a Boolean parameter in a vector of Booleans
 *
 * @param id The ID of the parameter
 * @param value The new value for the parameter
 * @param index The index into the vector
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
bool EventLocator::SetBooleanParameter(const Integer id, const bool value,
      const Integer index)
{
   return GmatBase::SetBooleanParameter(id, value, index);
}


//------------------------------------------------------------------------------
// bool GetBooleanParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a Boolean parameter
 *
 * @param label The script string of the parameter
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
bool EventLocator::GetBooleanParameter(const wxString &label) const
{
   return GetBooleanParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetBooleanParameter(const wxString &label, const bool value)
//------------------------------------------------------------------------------
/**
 * Sets the value for a Boolean parameter
 *
 * @param label The script string of the parameter
 * @param value The new value for the parameter
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
bool EventLocator::SetBooleanParameter(const wxString &label,
      const bool value)
{
   return SetBooleanParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// bool GetBooleanParameter(const wxString &label, const Integer index) const
//------------------------------------------------------------------------------
/**
 * Retrieves the value for a Boolean parameter from a vector of Booleans
 *
 * @param label The script string of the parameter
 * @param index The index into the vector
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
bool EventLocator::GetBooleanParameter(const wxString &label,
      const Integer index) const
{
   return GetBooleanParameter(GetParameterID(label), index);
}


//------------------------------------------------------------------------------
// bool SetBooleanParameter(const wxString &label, const bool value,
//       const Integer index)
//------------------------------------------------------------------------------
/**
 * Sets the value for a Boolean parameter in a vector of Booleans
 *
 * @param label The script string of the parameter
 * @param value The new value for the parameter
 * @param index The index into the vector
 *
 * @return The parameter value
 */
//------------------------------------------------------------------------------
bool EventLocator::SetBooleanParameter(const wxString &label,
      const bool value, const Integer index)
{
   return SetBooleanParameter(GetParameterID(label), value, index);
}


//------------------------------------------------------------------------------
// bool TakeAction(const std::string &action, const std::string &actionData)
//------------------------------------------------------------------------------
/**
 * Performs a custom action on the object.
 *
 * Event locators use this method to clear arrays in the locator.
 *
 * @param action The string specifying the action to be taken
 *
 * @return true on success, false on failure
 */
//------------------------------------------------------------------------------
bool EventLocator::TakeAction(const wxString &action,
      const wxString &actionData)
{
   if (action == wxT("Clear"))
   {
      if ((actionData == wxT("Spacecraft")) || (actionData == wxT("")))
      {
         satNames.clear();
         targets.clear();
      }
      return true;
   }

   return GmatBase::TakeAction(action, actionData);
}

//------------------------------------------------------------------------------
// const ObjectTypeArray& GetTypesForList(const Integer id)
//------------------------------------------------------------------------------
/**
 * Retrieves a list of types that need to be shown on a GUI for a parameter
 *
 * @param id The parameter ID
 *
 * @return The list of types
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& EventLocator::GetTypesForList(const Integer id)
{
   if (id == SATNAMES)
   {
      if (find(listedTypes.begin(), listedTypes.end(), Gmat::SPACECRAFT) ==
            listedTypes.end())
         listedTypes.push_back(Gmat::SPACECRAFT);
      return listedTypes;
   }

   return GmatBase::GetTypesForList(id);
}

//------------------------------------------------------------------------------
// const ObjectTypeArray& GetTypesForList(const wxString &label)
//------------------------------------------------------------------------------
/**
 * Retrieves a list of types that need to be shown on a GUI for a parameter
 *
 * @param label The parameter's identifying string
 *
 * @return The list of types
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& EventLocator::GetTypesForList(const wxString &label)
{
   return GetTypesForList(GetParameterID(label));
}


//------------------------------------------------------------------------------
// void SetSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 * Sets the solar system pointer
 *
 * @param ss The solar system
 */
//------------------------------------------------------------------------------
void EventLocator::SetSolarSystem(SolarSystem *ss)
{
   solarSys = ss;
}


//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Retrieves the list of reference objects
 *
 * @param type The type of the requested objects; UNKNOWN_OBJECT for all
 *
 * @return A StringArray of object names
 */
//------------------------------------------------------------------------------
const StringArray& EventLocator::GetRefObjectNameArray(
      const Gmat::ObjectType type)
{
   refObjectNames.clear();
   if (type == Gmat::UNKNOWN_OBJECT || type == Gmat::SPACEOBJECT)
   {
      // Get ref. objects for requesting type from the parent class
      GmatBase::GetRefObjectNameArray(type);

      // Add ref. objects for requesting type from this class
      refObjectNames.insert(refObjectNames.begin(), satNames.begin(),
            satNames.end());

      #ifdef DEBUG_EVENTLOCATOR_OBJECT
         MessageInterface::ShowMessage
            (wxT("EventLocator::GetRefObjectNameArray() this=<%p>'%s' returning %d ")
             wxT("ref. object names\n"), this, GetName().c_str(),
             refObjectNames.size());
         for (UnsignedInt i=0; i<refObjectNames.size(); i++)
            MessageInterface::ShowMessage(wxT("   '%s'\n"),
                  refObjectNames[i].c_str());
      #endif

      return refObjectNames;
   }

   return GmatBase::GetRefObjectNameArray(type);
}


//------------------------------------------------------------------------------
// bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//       const wxString &name)
//------------------------------------------------------------------------------
/**
 * Sets a reference object on this object
 *
 * @param obj The reference object
 * @param type The type of the object
 * @param name The name of the object
 *
 * @return true on success, false on failure
 */
//------------------------------------------------------------------------------
bool EventLocator::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                              const wxString &name)
{
   for (UnsignedInt i = 0; i < satNames.size(); ++i)
   {
      if (satNames[i] == name)
      {
         if (obj->IsOfType(Gmat::SPACEOBJECT))
         {
            targets[i] = (SpaceObject*)obj;
            return true;
         }
         return false;
      }
   }
   return GmatBase::SetRefObject(obj, type, name);
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 * Prepares the locator for use
 *
 * @return true if initialization succeeds, false on failure
 */
//------------------------------------------------------------------------------
bool EventLocator::Initialize()
{
   bool retval = false;

   StringArray badInits;
   efCount = eventFunctions.size();

   // Loop through the event functions, evaluating each and storing their data
   for (UnsignedInt i = 0; i < efCount; ++i)
   {
      if (eventFunctions[i]->Initialize() == false)
         badInits.push_back(eventFunctions[i]->GetName());
   }

   if (badInits.size() == 0)
      retval = true;
   else
   {
      wxString errorList;
      for (UnsignedInt i = 0; i < badInits.size(); ++i)
         errorList = errorList + wxT("   ") + badInits[i] + wxT("\n");
      throw EventException(wxT("These event functions failed to initialize:\n") +
            errorList);
   }

   if (lastData != NULL)
      delete [] lastData;
   if (lastEpochs != NULL)
      delete [] lastEpochs;

   if (efCount > 0)
   {
      lastData = new Real[efCount * 3];
      lastEpochs = new GmatEpoch[efCount];
      for (UnsignedInt i = 0; i < efCount; ++i)
         lastEpochs[i] = -1.0;

      if (stateIndices.size() == 0)
         stateIndices.insert(stateIndices.begin(), efCount, -1);
      if (associateIndices.size() == 0)
         associateIndices.insert(associateIndices.begin(), efCount, -1);
   }

   functionValues.SetSize(efCount);
   for (UnsignedInt i = 0; i < efCount; ++i)
      functionValues[i] = 0.0;

   return retval;
}


//------------------------------------------------------------------------------
// Real GetTolerance()
//------------------------------------------------------------------------------
/**
 * Retrieves the locator's tolerance setting
 *
 * @return The tolerance
 */
//------------------------------------------------------------------------------
Real EventLocator::GetTolerance()
{
   return eventTolerance;
}


//------------------------------------------------------------------------------
// Real *Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates the EventFunctions and returns their values and derivatives.
 *
 * #param atEpoch  The epoch of the desired evaluation.  Set to -1 to pull epoch
 *                 off the participants.
 * #param forState The Cartesian state of the desired evaluation.  Set to -1 to
 *                 pull state off the participants.
 *
 * @return The event function data from the evaluation
 */
//------------------------------------------------------------------------------
Real *EventLocator::Evaluate(GmatEpoch atEpoch, Real *forState)
{
   Real *vals;

   #ifdef DEBUG_EVENTLOCATION
      MessageInterface::ShowMessage(wxT("Evaluating %d event functions; ")
            wxT("locator %s\n"), eventFunctions.size(), instanceName.c_str());
   #endif

   UnsignedInt i3;

   // Loop through the event functions, evaluating each and storing their data
   for (UnsignedInt i = 0; i < eventFunctions.size(); ++i)
   {
      i3 = i * 3;
      if (atEpoch == -1.0)
      {
         // Evaluate for event location
         vals = eventFunctions[i]->Evaluate();

         #ifdef DEBUG_DUMPEVENTDATA
            dumpfile.precision(15);
            dumpfile << vals[0] << wxT(" ") << vals[1] << wxT(" ") << vals[2] << wxT(" ");
         #endif
      }
      else
      {
         // Evaluate for integration
         Real *theState = &(forState[associateIndices[i]]);
         vals = eventFunctions[i]->Evaluate(atEpoch, theState);
      }

      // Load the returned data into lastData
      lastData[  i3  ] = vals[0];
      lastData[i3 + 1] = vals[1];
      lastData[i3 + 2] = vals[2];
   }

   #ifdef DEBUG_DUMPEVENTDATA
      dumpfile << wxT("\n");
   #endif

   return lastData;
}


//------------------------------------------------------------------------------
// UnsignedInt GetFunctionCount()
//------------------------------------------------------------------------------
/**
 * Determine the number of event functions in the locator
 *
 * @return The number of functions
 */
//------------------------------------------------------------------------------
UnsignedInt EventLocator::GetFunctionCount()
{
   return eventFunctions.size();
}


//------------------------------------------------------------------------------
// void BufferEvent(Integer forEventFunction)
//------------------------------------------------------------------------------
/**
 * Saves data for an event boundary in the event table
 *
 * @param forEventFunction Index of the function that is to be saved
 */
//------------------------------------------------------------------------------
void EventLocator::BufferEvent(Integer forEventFunction)
{
   // Build a LocatedEvent structure
   LocatedEvent *theEvent = new LocatedEvent;

   Real *theData = eventFunctions[forEventFunction]->GetData();
   theEvent->epoch = theData[0];
   lastEpochs[forEventFunction] = theEvent->epoch;
   theEvent->eventValue = theData[1];
   theEvent->type = eventFunctions[forEventFunction]->GetTypeName();
   theEvent->participants = eventFunctions[forEventFunction]->GetName();
   theEvent->boundary = eventFunctions[forEventFunction]->GetBoundaryType();
   theEvent->isEntry = eventFunctions[forEventFunction]->IsEventEntry();

   #ifdef DEBUG_EVENTLOCATION
      MessageInterface::ShowMessage(wxT("Adding event to event table:\n   ")
            wxT("%-20s%-30s%-15s%15.9lf\n"), theEvent->type.c_str(),
            theEvent->participants.c_str(), theEvent->boundary.c_str(),
            theEvent->epoch);
   #endif

   eventTable.AddEvent(theEvent);
}


//------------------------------------------------------------------------------
// void EventLocator::BufferEvent(Real epoch, wxString type, bool isStart)
//------------------------------------------------------------------------------
/**
 * Saves data for an event boundary in the event table
 *
 * @param epoch The epoch of the event
 * @param type The event type
 * @param isStart Flag indicating if event is an entry (true) or exit (false)
 *
 * @note: This method is not used in the current code, and not yet implemented
 */
//------------------------------------------------------------------------------
void EventLocator::BufferEvent(Real epoch, wxString type, bool isStart)
{
   throw EventException(wxT("BufferEvent(epoch, type, isStart) is not yet ")
         wxT("supported"));
}


//------------------------------------------------------------------------------
// void ReportEventData()
//------------------------------------------------------------------------------
/**
 * Writes the event data to file and optionally displays the event data plot.
 */
//------------------------------------------------------------------------------
void EventLocator::ReportEventData()
{
   wxString fullFileName;

   if ((filename.find(wxT('/'), 0) == wxString::npos) &&
       (filename.find(wxT('\\'), 0) == wxString::npos))
   {
      FileManager *fm = FileManager::Instance();
      wxString outPath = fm->GetAbsPathname(FileManager::OUTPUT_PATH);

      // Check for terminating '/' and add if needed
      Integer len = outPath.length();
      if ((outPath[len-1] != wxT('/')) && (outPath[len-1] != wxT('\\')))
         outPath = outPath + wxT("/");

      fullFileName = outPath + filename;
   }
   else
      fullFileName = filename;

   eventTable.WriteToFile(fullFileName);
   if (showPlot)
      eventTable.ShowPlot();
}


//------------------------------------------------------------------------------
// void ReportEventStatistics()
//------------------------------------------------------------------------------
/**
 * Writes the event data statistics to file.
 */
//------------------------------------------------------------------------------
void EventLocator::ReportEventStatistics()
{
   throw EventException(wxT("ReportEventStatistics() is not yet supported"));
}


//------------------------------------------------------------------------------
// Real* GetEventData(wxString type, Integer whichOne)
//------------------------------------------------------------------------------
/**
 * Retrieves data for a specified event.
 *
 * @param type The type of the event (Not yet used)
 * @param whichOne Index into the type for the specific event (Not yet used)
 *
 * @return The event data
 */
//------------------------------------------------------------------------------
Real* EventLocator::GetEventData(wxString type, Integer whichOne)
{
   return lastData;
}


//------------------------------------------------------------------------------
// void UpdateEventTable(SortStyle how)
//------------------------------------------------------------------------------
/**
 * Updates the data in the event table, possibly sorting as well
 *
 * @param how The sorting style
 */
//------------------------------------------------------------------------------
void EventLocator::UpdateEventTable(SortStyle how)
{
   throw EventException(wxT("UpdateEventTable(how) is not yet supported"));
}

//------------------------------------------------------------------------------
// GmatEpoch GetLastEpoch(Integer index)
//------------------------------------------------------------------------------
/**
 * Retrieves the last epoch for a given event
 *
 * @param index Index of the event
 *
 * @return The epoch
 *
 * @note: This method may need refactoring.
 */
//------------------------------------------------------------------------------
GmatEpoch EventLocator::GetLastEpoch(Integer index)
{
   return lastEpochs[index];
}


bool EventLocator::HasAssociatedStateObjects()
{
   return true;
}

wxString EventLocator::GetAssociateName(UnsignedInt val)
{
   wxString retval = wxT("");
   if (val < efCount)
      retval = eventFunctions[val]->GetPrimaryName();
   return retval;
}


//------------------------------------------------------------------------------
// wxString GetTarget(UnsignedInt forFunction)
//------------------------------------------------------------------------------
/**
 * Retrieves the name of the target object
 *
 * @param forFunction Index of the event function being referenced
 *
 * @return The name of the target object
 */
//------------------------------------------------------------------------------
wxString EventLocator::GetTarget(UnsignedInt forFunction)
{
   if (forFunction > efCount)
      throw EventException(wxT("Requested event function target is not in the ")
            wxT("locator named ") + instanceName);

   return eventFunctions[forFunction]->GetPrimaryName();
}

StringArray EventLocator::GetDefaultPropItems()
{
   StringArray propItems;
   propItems.push_back(wxT("EventFunction"));
   return propItems;
}

Integer EventLocator::SetPropItem(const wxString &propItem)
{
   if (propItem == wxT("EventFunction"))
      return Gmat::EVENT_FUNCTION_STATE;

   return Gmat::UNKNOWN_STATE;
}

Integer EventLocator::GetPropItemSize(const Integer item)
{
   if (item == Gmat::EVENT_FUNCTION_STATE)
      return efCount;

   return GmatBase::GetPropItemSize(item);
}

//------------------------------------------------------------------------------
// void SetStateIndices(UnsignedInt forFunction, Integer index,
//       Integer associate)
//------------------------------------------------------------------------------
/**
 * Sets the indices of the parameters in the state vector for an event function
 *
 * @param forFunction Index of the event function being referenced
 * @param index The location of the event function in the state vector
 * @param associate The index of the start of state data for the target object
 */
//------------------------------------------------------------------------------
void EventLocator::SetStateIndices(UnsignedInt forFunction, Integer index,
      Integer associate)
{
   if (forFunction > stateIndices.size())
      throw EventException(wxT("Event locator ") + instanceName +
            wxT("does not have the requested event function (has it been ")
            wxT("initialized?)"));

   stateIndices[forFunction]     = index;
   associateIndices[forFunction] = associate;
}
