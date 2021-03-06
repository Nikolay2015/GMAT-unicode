//$Id: TimeReal.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                TimeReal
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
// Author: Linda Jun
// Created: 2004/04/28
//
/**
 * Declares TimeReal class which provides base class for time realated Real
 * Parameters
 */
//------------------------------------------------------------------------------
#ifndef TimeReal_hpp
#define TimeReal_hpp

#include "gmatdefs.hpp"
#include "RealVar.hpp"
#include "TimeData.hpp"


class GMAT_API TimeReal : public RealVar, public TimeData
{
public:

   TimeReal(const wxString &name, const wxString &typeStr, 
            GmatBase *obj, const wxString &desc, const wxString &unit);
   TimeReal(const TimeReal &copy);
   TimeReal& operator=(const TimeReal &right);
   virtual ~TimeReal();

   // methods inherited from Parameter
   virtual Real EvaluateReal();
   
   virtual Integer GetNumRefObjects() const;
   virtual bool Validate();
   virtual bool Initialize();
   virtual bool AddRefObject(GmatBase *obj, bool replaceName = false);
   
   // methods inherited from GmatBase
   virtual bool RenameRefObject(const Gmat::ObjectType type,
                                const wxString &oldName,
                                const wxString &newName);
   
   virtual wxString GetRefObjectName(const Gmat::ObjectType type) const;
   virtual const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual bool SetRefObjectName(const Gmat::ObjectType type,
                                 const wxString &name);
   virtual GmatBase* GetRefObject(const Gmat::ObjectType type,
                                  const wxString &name);
   virtual bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                             const wxString &name = wxT(""));
protected:
   

};

#endif // TimeReal_hpp
