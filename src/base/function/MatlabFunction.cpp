//$Id: MatlabFunction.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  MatlabFunction
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Allison Greene
// Created: 2004/09/22
//
/**
 * Definition for the MatlabFunction class.
 */
//------------------------------------------------------------------------------

#include "MatlabFunction.hpp"
#include "FileManager.hpp"       // for GetCurrentPath()
#include "FileUtil.hpp"          // for ParseFileName()
#include "StringUtil.hpp"        // for Trim()
#include "MessageInterface.hpp"

//#define DEBUG_MATLAB_FUNCTION


//------------------------------------------------------------------------------
// MatlabFunction(wxString &name)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the MatlabFunction.
 *
 */
//------------------------------------------------------------------------------
MatlabFunction::MatlabFunction(const wxString &name) :
   Function(wxT("MatlabFunction"), name)
{
   // for initial function path, use FileManager
   FileManager *fm = FileManager::Instance();
   wxString pathname;
   
   try
   {
      // matlab uses directory path
      pathname = fm->GetMatlabFunctionPath(name);
      functionPath = pathname;
   }
   catch (GmatBaseException &e)
   {
      // Use exception to remove Visual C++ warning
      e.GetMessageType();
      #ifdef DEBUG_MATLAB_FUNCTION
      MessageInterface::ShowMessage(e.GetFullMessage());
      #endif
      
      // see if there is FUNCTION_PATH
      try
      {
         pathname = fm->GetFullPathname(wxT("FUNCTION_PATH"));
         functionPath = pathname;
      }
      catch (GmatBaseException &e)
      {
         // Use exception to remove Visual C++ warning
         e.GetMessageType();
         #ifdef DEBUG_MATLAB_FUNCTION
         MessageInterface::ShowMessage(e.GetFullMessage());
         #endif
      }
   }
   
   #ifdef DEBUG_MATLAB_FUNCTION
   MessageInterface::ShowMessage
      (wxT("   Matlab functionPath=<%s>\n"), functionPath.c_str());
   MessageInterface::ShowMessage
      (wxT("   Matlab functionName=<%s>\n"), functionName.c_str());
   #endif
}


//------------------------------------------------------------------------------
// ~MatlabFunction()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
MatlabFunction::~MatlabFunction()
{
}


//------------------------------------------------------------------------------
// MatlabFunction(const MatlabFunction &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> object being copied
 */
//------------------------------------------------------------------------------
MatlabFunction::MatlabFunction(const MatlabFunction &copy) :
   Function      (copy)
{
}


//------------------------------------------------------------------------------
// MatlabFunction& MatlabFunction::operator=(const MatlabFunction &right)
//------------------------------------------------------------------------------
/**
 * The assignment operator
 *
 * @param <right> object being copied
 *
 * @return reference to this object
 */
//------------------------------------------------------------------------------
MatlabFunction& MatlabFunction::operator=(const MatlabFunction& right)
{
    if (this == &right)
        return *this;

    Function::operator=(right);

    return *this;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the MatlabFunction.
 *
 * @return clone of the MatlabFunction.
 */
//------------------------------------------------------------------------------
GmatBase* MatlabFunction::Clone() const
{
   return (new MatlabFunction(*this));
}


//---------------------------------------------------------------------------
// void Copy(const GmatBase* orig)
//---------------------------------------------------------------------------
/**
 * Sets this object to match another one.
 * 
 * @param orig The original that is being copied.
 */
//---------------------------------------------------------------------------
void MatlabFunction::Copy(const GmatBase* orig)
{
   operator=(*((MatlabFunction *)(orig)));
}


//------------------------------------------------------------------------------
//  bool SetStringParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * Sets the value for a wxString parameter.
 * 
 * @param <id> Integer ID of the parameter.
 * @param <value> New value for the parameter.
 * 
 * @return If value of the parameter was set.
 */
//------------------------------------------------------------------------------
bool MatlabFunction::SetStringParameter(const Integer id, const wxString &value)
{
   #ifdef DEBUG_FUNCTION_SET
   MessageInterface::ShowMessage
      (wxT("MatlabFunction::SetStringParameter() entered, id=%d, value=%s\n"), id, value.c_str());
   #endif
   
   switch (id)
   {
   case FUNCTION_PATH:
      {
         // Compose full path if it has relative path.
         // Assuming if first char has '.', it has relative path.
         wxString temp = GmatStringUtil::Trim(value);
         if (temp[0] == wxT('.'))
         {
            FileManager *fm = FileManager::Instance();
            wxString currPath = fm->GetCurrentPath();
            
            #ifdef DEBUG_FUNCTION_SET
            MessageInterface::ShowMessage(wxT("   currPath=%s\n"), currPath.c_str());
            #endif
            
            functionPath = currPath + temp.substr(1);
         }
         else
         {
            functionPath = value;
         }
         
         // Remove path
         functionName = GmatFileUtil::ParseFileName(functionPath);
         
         #ifdef DEBUG_FUNCTION_SET
         MessageInterface::ShowMessage
            (wxT("   functionPath=<%s>\n"), functionPath.c_str());
         MessageInterface::ShowMessage
            (wxT("   functionName=<%s>\n"), functionName.c_str());
         #endif
         
         return true;
      }
   case FUNCTION_NAME:
      {
         // Remove path if it has one
         functionName = GmatFileUtil::ParseFileName(functionPath);
         return true;
      }
   default:
      return Function::SetStringParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
bool MatlabFunction::SetStringParameter(const wxString &label,
                                        const wxString &value)
{
   return SetStringParameter(GetParameterID(label), value);
}

