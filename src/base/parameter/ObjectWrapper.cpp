//$Id: ObjectWrapper.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  ObjectWrapper
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. 
//
// Author: Linda Jun/GSFC
// Created: 2007/07/24
//
/**
 * Declares ObjectWrapper class.
 */
//------------------------------------------------------------------------------

#include "ObjectWrapper.hpp"
#include "ParameterException.hpp"
#include "StringUtil.hpp"         // for ToObject()
#include "MessageInterface.hpp"

//#define DEBUG_OBJECT_WRAPPER

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// ObjectWrapper()
//------------------------------------------------------------------------------
/**
 * Constructs ObjectWrapper structures
 * (default constructor).
 *
 */
//------------------------------------------------------------------------------
ObjectWrapper::ObjectWrapper() :
   ElementWrapper()
{
   theObject   = NULL;
   wrapperType = Gmat::OBJECT_WT;
}


//------------------------------------------------------------------------------
// ObjectWrapper(const ObjectWrapper &copy);
//------------------------------------------------------------------------------
/**
 * Constructs base ObjectWrapper structures used in derived classes, by 
 * copying the input instance (copy constructor).
 *
 * @param <copy>  ObjectWrapper instance to copy to create wxT("this") instance.
 */
//------------------------------------------------------------------------------
ObjectWrapper::ObjectWrapper(const ObjectWrapper &copy) :
   ElementWrapper(copy),
   theObject     (copy.theObject)
{
}


//------------------------------------------------------------------------------
// ObjectWrapper& operator=(const ObjectWrapper &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator for ObjectWrapper structures.
 *
 * @param <right> The original that is being copied.
 *
 * @return Reference to this object
 */
//------------------------------------------------------------------------------
const ObjectWrapper& ObjectWrapper::operator=(const ObjectWrapper &right)
{
   if (&right == this)
      return *this;
   
   ElementWrapper::operator=(right);
   theObject = right.theObject;

   return *this;
}


//------------------------------------------------------------------------------
// ~ObjectWrapper()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
ObjectWrapper::~ObjectWrapper()
{
}


//------------------------------------------------------------------------------
// wxString ToString()
//------------------------------------------------------------------------------
/**
 * @return ObjectWrapper value converted to wxString.
 *
 * @exception <GmatBaseException> thrown if this method is called.
 */
//------------------------------------------------------------------------------
wxString ObjectWrapper::ToString()
{
   return theObject->GetGeneratingString(Gmat::NO_COMMENTS);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetDataType() const
//------------------------------------------------------------------------------
/**
 * This method returns the data type for the ObjectWrapper object.
 *
 * @return data type for the object.
 *
 */
//------------------------------------------------------------------------------
Gmat::ParameterType ObjectWrapper::GetDataType() const
{
   return Gmat::OBJECT_TYPE;
}


//---------------------------------------------------------------------------
// GmatBase* GetRefObject(const wxString &name = wxT(""))
//---------------------------------------------------------------------------
/*
 * @see ElementWrapper
 */
//---------------------------------------------------------------------------
GmatBase* ObjectWrapper::GetRefObject(const wxString &name)
{
   return theObject;
}


//---------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj)
//---------------------------------------------------------------------------
/**
 * Method to set the reference object (Parameter) pointer on the wrapped 
 * object.
 *
 * @return true if successful; false otherwise.
 */
//---------------------------------------------------------------------------
bool ObjectWrapper::SetRefObject(GmatBase *obj)
{
   if (obj->GetName() == refObjectNames[0])
   {
      theObject = obj;
      return true;
   }
   else
      return false;
}


//---------------------------------------------------------------------------
//  bool RenameObject(const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
/**
 * Method to rename a reference object for the wrapper.
 *
 * @return true if successful; false otherwise.
 */
//---------------------------------------------------------------------------
bool ObjectWrapper::RenameObject(const wxString &oldName, 
                                 const wxString &newName)
{
   ElementWrapper::RenameObject(oldName, newName);
   description = refObjectNames[0];  
   return true;
}


//---------------------------------------------------------------------------
// Real EvaluateReal() const
//---------------------------------------------------------------------------
/**
 * Method to return the Real value of the wrapped object.
 *
 * @return Real value of the wrapped number object.
 * 
 * @note This method is not value for Strings.
 * 
 */
//---------------------------------------------------------------------------
Real ObjectWrapper::EvaluateReal() const
{
   throw ParameterException(
      wxT("EvaluateReal() method not valid for wrapper of Object type.\n"));
}


//---------------------------------------------------------------------------
// bool SetReal(const Real toValue)
//---------------------------------------------------------------------------
/**
 * Method to set the Real value of the wrapped object.
 *
 * @return true if successful; false otherwise.
 * 
 * @note This method is not valid for Object.
 */
//---------------------------------------------------------------------------
bool ObjectWrapper::SetReal(const Real toValue)
{
   throw ParameterException(
      wxT("SetReal() method not valid for wrapper of Object type.\n"));
}


//------------------------------------------------------------------------------
// GmatBase* EvaluateObject() const
//------------------------------------------------------------------------------
/**
 * Method to return the Object pointer of the ObjectWrapper object.
 *
 * @return value of the ObjectWrapper object.
 * 
 */
//------------------------------------------------------------------------------
GmatBase* ObjectWrapper::EvaluateObject() const
{
   return theObject;
}


//------------------------------------------------------------------------------
// bool SetObject(GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Method to set the object of the wrapped object.
 *
 * @param <obj> The object pointer to set
 * @return true if successful; false otherwise.
 */
//------------------------------------------------------------------------------
bool ObjectWrapper::SetObject(GmatBase *obj)
{
   if (obj == NULL)
   {
      if (theObject == NULL)
         throw ParameterException(wxT("Cannot set undefined object to undefined object"));
      else
         throw ParameterException
            (wxT("Cannot set undefined object to object of type \"") +
             theObject->GetTypeName() +  wxT("\""));         
   }
   
   if (theObject != NULL)
   {
      // Let's check the object type
      if (theObject->GetTypeName() == obj->GetTypeName())
      {
         #ifdef DEBUG_OBJECT_WRAPPER
         MessageInterface::ShowMessage
            (wxT("ObjectWrapper::SetObject() fromType=%s, toType=%s\n"),
             obj->GetTypeName().c_str(), theObject->GetTypeName().c_str());
         #endif
         
         theObject->Copy(obj);
      }
      else
      {
         ParameterException pe;
         pe.SetDetails(wxT("Cannot set object of type \"%s\" to object of type \"%s\""),
                       obj->GetTypeName().c_str(), theObject->GetTypeName().c_str());
         throw pe;
      }
   }
   else
   {
      throw ParameterException(wxT("Cannot set object \"") + obj->GetName() +
                               wxT("\" to an undefined object"));
   }
   
   return true;
}


//---------------------------------
// protected methods
//---------------------------------

//------------------------------------------------------------------------------
// void SetupWrapper()
//------------------------------------------------------------------------------
/**
 * Method to set up the Object Wrapper.
 *
 */
//------------------------------------------------------------------------------
void ObjectWrapper::SetupWrapper()
{
   refObjectNames.push_back(description);
}
