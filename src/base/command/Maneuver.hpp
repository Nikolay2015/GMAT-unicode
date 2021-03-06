//$Id: Maneuver.hpp 9731 2011-07-21 22:29:19Z djcinsb $
//------------------------------------------------------------------------------
//                                 Maneuver
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P
//
// Author: Darrel J. Conway
// Created: 2004/01/20
//
/**
 * Definition for the Maneuver command class
 */
//------------------------------------------------------------------------------


#ifndef Maneuver_hpp
#define Maneuver_hpp

#include "GmatCommand.hpp"
#include "Burn.hpp"
#include "Spacecraft.hpp"


/**
 * This class implements the Maneuver command.  Maneuvers are performed using
 * pre-configured Burn objects.  A typical segment of script that implements
 * a maneuver for a spacecraft named Sat1 looks like this:
 *
 *     Create ImpulsiveBurn burn;
 *     burn.Type = VNB;
 *     burn.Element1 = 0.125;         % km/s
 *     ...
 *     Maneuver burn(Sat1);
 */
class GMAT_API Maneuver : public GmatCommand
{
public:
   Maneuver();
   virtual ~Maneuver();
   
   Maneuver(const Maneuver& m);
   Maneuver&            operator=(const Maneuver& m);
   
   // Methods used for configuration
   virtual bool         SetObject(const wxString &name,
                                  const Gmat::ObjectType type,
                                  const wxString &associate = wxT(""),
                                  const Gmat::ObjectType associateType =
                                  Gmat::UNKNOWN_OBJECT);
   
   virtual bool         SetObject(GmatBase *obj,
                                  const Gmat::ObjectType type);
   
   
   // inherited from GmatBase
   virtual GmatBase* Clone() const;
   virtual const wxString&
                        GetGeneratingString(Gmat::WriteMode mode = Gmat::SCRIPTING,
                                            const wxString &prefix = wxT(""),
                                            const wxString &useName = wxT(""));
   
   virtual bool         RenameRefObject(const Gmat::ObjectType type,
                                        const wxString &oldName,
                                        const wxString &newName);
   
   virtual const ObjectTypeArray&
                        GetRefObjectTypeArray();
   virtual const StringArray&
                        GetRefObjectNameArray(const Gmat::ObjectType type);
   
   // Parameter accessor methods -- overridden from GmatBase
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value);
   
   // Methods used to run the command
   virtual bool         InterpretAction();
   
   virtual bool         Initialize();
   virtual bool         Execute();
   
protected:
   /// The burn object used for the maneuver
   wxString              burnName;
   /// The burn object used for the maneuver
   Burn                    *burn;
   /// The name of the spacecraft that gets maneuvered
   wxString             satName;
   /// The spacecraft
   Spacecraft              *sat;
   /// Flag used to tell if the summary can be built yet
   bool                    firedOnce;

   // Parameter IDs 
   /// ID for the burn object
   Integer                 burnNameID;
   /// ID for the spacecraft name
   Integer                 satNameID;

   virtual void            BuildCommandSummaryString(bool commandCompleted = true);
};

#endif // Maneuver_hpp
