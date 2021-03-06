//$Id: GroundStation.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                            GroundStation
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
// Author: Darrel J. Conway, Thinking Systems, Inc.
// Created: 2008/08/01
// Modified:
//    2010.06.03 Tuan Nguyen
//      - Add AddHardware parameter and verify added hardware
//    2010.03.15 Thomas Grubb
//      - Changed visiblity of PARAMETER_TEXT, PARAMETER_TYPE, and enum from
//        protected to public
//      - Overrode Copy method
//
/**
 * Defines the Groundstation class used to model ground based tracking stations.
 */
//------------------------------------------------------------------------------

#ifndef GroundStation_hpp
#define GroundStation_hpp

#include "SpacePoint.hpp"
#include "BodyFixedPoint.hpp"
#include "LatLonHgt.hpp"
#include "CoordinateSystem.hpp"
#include "CoordinateConverter.hpp"
#include "Hardware.hpp"


class GMAT_API GroundStation : public BodyFixedPoint
{
public:
   GroundStation(const wxString &itsName);
   virtual ~GroundStation();
   GroundStation(const GroundStation& gs);
   GroundStation& operator=(const GroundStation& gs);

   // all leaf classes derived from GmatBase must supply this Clone method
   virtual GmatBase*       Clone() const;
   virtual void            Copy(const GmatBase* orig);

   // Access methods derived classes can override
   virtual wxString  GetParameterText(const Integer id) const;
   virtual wxString  GetParameterUnit(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;

   virtual bool         IsParameterReadOnly(const Integer id) const;
   virtual bool         IsParameterReadOnly(const wxString &label) const;

   virtual wxString  GetStringParameter(const Integer id) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value);
   virtual wxString  GetStringParameter(const Integer id,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value,
                                           const Integer index);
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value);
   virtual wxString  GetStringParameter(const wxString &label,
                                           const Integer index) const;
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value,
                                           const Integer index);

   // made changes by Tuan Nguyen
   virtual const StringArray&
                        GetStringArrayParameter(const Integer id) const;
   virtual const StringArray&
                        GetStringArrayParameter(const wxString &label) const;

   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   virtual GmatBase*    GetRefObject(const Gmat::ObjectType type,
                                        const wxString &name);
   virtual bool         SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                                        const wxString &name = wxT(""));

   virtual ObjectArray& GetRefObjectArray(const Gmat::ObjectType type);
   virtual ObjectArray& GetRefObjectArray(const wxString& typeString);


   virtual bool         HasRefObjectTypeArray();
   virtual const        ObjectTypeArray& GetRefObjectTypeArray();


   virtual bool         Initialize();

//   virtual Integer         GetEstimationParameterID(const wxString &param);
//   virtual Integer         SetEstimationParameter(const wxString &param);
   virtual bool            IsEstimationParameterValid(const Integer id);
   virtual Integer         GetEstimationParameterSize(const Integer id);
   virtual Real*           GetEstimationParameterValue(const Integer id);

   virtual bool            IsValidID(const wxString &id);

protected:
   /// Ground station ID
   wxString          stationId;

   // Added hardware of the ground station
   StringArray	         hardwareNames;       // made changes by Tuan Nguyen
   ObjectArray          hardwareList;        // made changes by Tuan Nguyen

//   bool              	SetHardware(GmatBase *obj, StringArray &hwNames,
//                                 ObjectArray &hwArray);		// made changes by Tuan Nguyen

   // Override GetGenString to handle the changeable names for the parameters
   virtual const wxString&
                        GetGeneratingString(
                           Gmat::WriteMode mode = Gmat::SCRIPTING,
                           const wxString &prefix = wxT(""),
                           const wxString &useName = wxT(""));
   virtual void         WriteParameters(Gmat::WriteMode mode,
                           wxString &prefix, wxString &stream);

public:
   /// Published parameters for ground stations
   enum
   {
      STATION_ID = BodyFixedPointParamCount,
      ADD_HARDWARE,								// made changes by Tuan Nguyen
      GroundStationParamCount,
   };

   static const wxString
      PARAMETER_TEXT[GroundStationParamCount - BodyFixedPointParamCount];
   static const Gmat::ParameterType
      PARAMETER_TYPE[GroundStationParamCount - BodyFixedPointParamCount];

private:
   bool 		        VerifyAddHardware();			// made changes by Tuan Nguyen
};

#endif /*GroundStation_hpp*/
