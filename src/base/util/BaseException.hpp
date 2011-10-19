//$Id: BaseException.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              BaseException
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
// Author: Darrel J. Conway
// Created: 2003/09/09
//
/**
 * This class provides base exception class, from which all GMAT exceptions must 
 * be derived.
 */
//------------------------------------------------------------------------------
#ifndef BaseException_hpp
#define BaseException_hpp

#include "gmatdefs.hpp"
#include <string>

class GMAT_API BaseException
{
public:
   virtual wxString GetFullMessage() const;
   virtual wxString GetDetails() const;
   virtual bool IsFatal() const;
   virtual void SetMessage(const wxString &message);
   virtual void SetDetails(const wxString &details, ...);
//   virtual void SetDetails(const wxString &details);
   virtual void SetFatal(bool fatal);
   virtual Gmat::MessageType GetMessageType();
   virtual void SetMessageType(Gmat::MessageType mt);
   
   const BaseException& operator=(const wxString &newMessage);
   
   
   static const int MAX_MESSAGE_LENGTH = 3000;
   
protected:
   BaseException(const wxString& message = wxT(""),
         const wxString &details = wxT(""),
         // Change to this when the repeated instances are fixed:
//         Gmat::MessageType mt = Gmat::ERROR_);
         // Change to this if it's problematic:
         Gmat::MessageType mt = Gmat::GENERAL_);
   BaseException(const BaseException& be);
   virtual ~BaseException();
   const BaseException& operator=(const BaseException& be);
   
private:
   wxString theMessage;
   wxString theDetails;
   Gmat::MessageType msgType;
   bool isFatal;
};
#endif // BaseException_hpp

