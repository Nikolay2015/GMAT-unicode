//$Id: OrbitReal.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                OrbitReal
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
// Created: 2004/03/29
//
/**
 * Declares OrbitReal class which provides base class for orbit realated Real
 * Parameters
 */
//------------------------------------------------------------------------------
#ifndef OrbitReal_hpp
#define OrbitReal_hpp

#include "gmatdefs.hpp"
#include "RealVar.hpp"
#include "OrbitData.hpp"


class GMAT_API OrbitReal : public RealVar, public OrbitData
{
public:

   OrbitReal(const wxString &name, const wxString &typeStr, 
             GmatBase *obj, const wxString &desc,
             const wxString &unit,  GmatParam::DepObject depObj,
             bool isSettable = false);
   OrbitReal(const OrbitReal &copy);
   OrbitReal& operator=(const OrbitReal &right);
   virtual ~OrbitReal();

   // methods inherited from Parameter
   virtual Real EvaluateReal();
   
   virtual Integer GetNumRefObjects() const;
   virtual CoordinateSystem* GetInternalCoordSystem();
   virtual void SetSolarSystem(SolarSystem *ss);
   virtual void SetInternalCoordSystem(CoordinateSystem *ss);
   virtual bool AddRefObject(GmatBase*obj, bool replaceName = false);
   virtual bool Validate();
   virtual bool Initialize();
   
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

#endif // OrbitReal_hpp
