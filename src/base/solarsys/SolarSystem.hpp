//$Id: SolarSystem.hpp 9725 2011-07-20 19:14:18Z wendys-dev $
//------------------------------------------------------------------------------
//                                  SolarSystem
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Wendy C. Shoan
// Created: 2004/01/28
//
/**
 * This is the SolarSystem class.
 *
 * @note The SolarSystem will contain pointers to all of the celestial bodies
 *       currently in use; NOTE *** for Build 2, this is Sun/Earth/Moon only***
 *       For Build 3, all planets (except Sedna?) are included.
 * @note It is assumed no classes will be derived from this one.
 */
//------------------------------------------------------------------------------


// Class (initial shell only) automatically generated by Dev-C++ New Class wizard

#ifndef SolarSystem_hpp
#define SolarSystem_hpp

#include <list>
#include <string>
#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "CelestialBody.hpp"
#include "SpecialCelestialPoint.hpp"
#include "PlanetaryEphem.hpp"
#include "DeFile.hpp"
#include "GmatDefaults.hpp"
#ifdef __USE_SPICE__
#include "SpiceOrbitKernelReader.hpp"
#endif

class CoordinateSystem;

/**
 * SolarSystem class, containing pointers to all of the objects currently in
 * use.
 *
 * The SolarSystem class manages all objects that are currently defined for the
 * specified Solar System.  NOTE - For Build 2, the default Solar System
 * contains only the Sun, Earth, and Moon.
 */
class GMAT_API SolarSystem : public GmatBase
{
public:
   // class default constructor - creates default solar system
   // for Build 2 - this is Sun, Earth, Moon only
   SolarSystem(wxString withName = wxT(""));
   // copy constructor
   SolarSystem(const SolarSystem &ss);
   // operator=
   SolarSystem& operator=(const SolarSystem &ss);
   // class destructor
   ~SolarSystem();
   
   virtual bool Initialize();
   
   // method for planetary ephemeris files
   void CreatePlanetarySource(bool setDefault = true);
   
   const StringArray& GetPlanetarySourceTypes();
   const StringArray& GetPlanetarySourceNames();
   const StringArray& GetPlanetarySourceTypesInUse();
   bool  SetPlanetarySourceName(const wxString &sourceType,
                               const wxString &fileName);
   Integer SetPlanetarySourceTypesInUse(const StringArray &sourceTypes); 
   Integer GetPlanetarySourceId(const wxString &sourceType);
   wxString GetPlanetarySourceName(const wxString &sourceType);
   wxString GetCurrentPlanetarySource();
   void        SetIsSpiceAllowedForDefaultBodies(const bool allowSpice);
   bool        IsSpiceAllowedForDefaultBodies() const;

   PlanetaryEphem* GetPlanetaryEphem();
#ifdef __USE_SPICE__
   void        LoadSpiceKernels();
   SpiceOrbitKernelReader* GetSpiceOrbitKernelReader();
#endif

   
   void ResetToDefaults();
   
   // method to add a body to the solar system
   bool                 AddBody(CelestialBody* cb);
   // method to return a body of the solar system, given its name
   CelestialBody*       GetBody(wxString withName);
   // method to remove a body from the solar system
   bool                 DeleteBody(const wxString &withName);
   /// methods to return a pointer to a specific SpecialCelestialPoint
   SpecialCelestialPoint*
                        GetSpecialPoint(const wxString &withName);
   
   // method to return an array of the names of the bodies included in
   // this solar system
   const StringArray&   GetBodiesInUse() const;
   // method to return a flag indicating whether or not the specified
   // body is in use for this solar system
   bool                 IsBodyInUse(wxString theBody);
   const StringArray&   GetDefaultBodies() const;
   const StringArray&   GetUserDefinedBodies() const;
   
   // methods to get the source and analytic model flags
   Gmat::PosVelSource   GetPosVelSource() const;
   wxString          GetSourceFileName() const;
   bool                 GetOverrideTimeSystem() const;
   Real                 GetEphemUpdateInterval() const;
   StringArray          GetValidModelList(Gmat::ModelType m, 
                                          const wxString &forBody);
   
   // methods to set the source, source file, and analytic method for each
   // of the bodies in use
   bool SetSource(Gmat::PosVelSource pvSrc);
   bool SetSource(const wxString &pvSrc);
   bool SetSourceFile(PlanetaryEphem *src);
   bool SetSPKFile(const wxString &spkFile);
   bool SetLSKFile(const wxString &lskFile);
   
   bool SetOverrideTimeSystem(bool overrideIt);
   bool SetEphemUpdateInterval(Real intvl);
   bool AddValidModelName(Gmat::ModelType m, const wxString &forBody,
                          const wxString &theModel);
   bool RemoveValidModelName(Gmat::ModelType m, const wxString &forBody,
                             const wxString &theModel);
   
   // methods used by internal functions
   Rvector6 GetCelestialBodyState(const wxString &bodyName, 
                                  CoordinateSystem *cs, const A1Mjd &epoch);
   
   
   // Parameter access methods
   virtual wxString  GetParameterText(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   
   virtual Integer      GetIntegerParameter(const Integer id) const;
   virtual Integer      GetIntegerParameter(const wxString &label) const;
   virtual Real         GetRealParameter(const Integer id) const;
   virtual Real         GetRealParameter(const wxString &label) const;
   virtual Real         SetRealParameter(const Integer id,
                                         const Real value);
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value);
   virtual bool         GetBooleanParameter(const Integer id) const; 
   virtual bool         GetBooleanParameter(const wxString &label) const; 
   virtual bool         SetBooleanParameter(const Integer id,
                                            const bool value);
   virtual bool         SetBooleanParameter(const wxString &label,
                                            const bool value);
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const Integer id, 
                                           const wxString &value);
   virtual bool         SetStringParameter(const wxString &label, 
                                           const wxString &value);
   virtual const StringArray&
                        GetStringArrayParameter(const Integer id) const;
   virtual const StringArray&
                        GetStringArrayParameter(const wxString &label) const;
   
   virtual Integer      GetOwnedObjectCount();
   virtual GmatBase*    GetOwnedObject(Integer whichOne);

   virtual bool         IsParameterReadOnly(const Integer id) const;
   virtual bool         IsParameterCloaked(const Integer id) const;
   virtual bool         IsParameterEqualToDefault(const Integer id) const;
   virtual bool         SaveAllAsDefault();
   virtual bool         SaveParameterAsDefault(const Integer id);

   // all classes derived from GmatBase must supply this Clone method
   virtual SolarSystem* Clone(void) const;
   
   // required method for all subclasses that can be copied in a script
   virtual void         Copy(const GmatBase* orig);
   
   /// default names for each of the possible celestial bodies in the solar system
   static const wxString SOLAR_SYSTEM_BARYCENTER_NAME;


   static const wxString SUN_NAME;

   static const wxString MERCURY_NAME;
   
   static const wxString VENUS_NAME;
   
   static const wxString EARTH_NAME;
   static const wxString MOON_NAME;
   
   static const wxString MARS_NAME;
   static const wxString PHOBOS_NAME;
   static const wxString DEIMOS_NAME;
   
   static const wxString JUPITER_NAME;
   static const wxString METIS_NAME;
   static const wxString ADRASTEA_NAME;
   static const wxString AMALTHEA_NAME;
   static const wxString THEBE_NAME;
   static const wxString IO_NAME;
   static const wxString EUROPA_NAME;
   static const wxString GANYMEDE_NAME;
   static const wxString CALLISTO_NAME;
   
   static const wxString SATURN_NAME;
   static const wxString PAN_NAME;
   static const wxString ATLAS_NAME;
   static const wxString PROMETHEUS_NAME;
   static const wxString PANDORA_NAME;
   static const wxString EPIMETHEUS_NAME;
   static const wxString JANUS_NAME;
   static const wxString MIMAS_NAME;
   static const wxString ENCELADUS_NAME;
   static const wxString TETHYS_NAME;
   static const wxString TELESTO_NAME;
   static const wxString CALYPSO_NAME;
   static const wxString DIONE_NAME;
   static const wxString HELENE_NAME;
   static const wxString RHEA_NAME;
   static const wxString TITAN_NAME;
   static const wxString IAPETUS_NAME;
   static const wxString PHOEBE_NAME;
   
   static const wxString URANUS_NAME;
   static const wxString CORDELIA_NAME;
   static const wxString OPHELIA_NAME;
   static const wxString BIANCA_NAME;
   static const wxString CRESSIDA_NAME;
   static const wxString DESDEMONA_NAME;
   static const wxString JULIET_NAME;
   static const wxString PORTIA_NAME;
   static const wxString ROSALIND_NAME;
   static const wxString BELINDA_NAME;
   static const wxString PUCK_NAME;
   static const wxString MIRANDA_NAME;
   static const wxString ARIEL_NAME;
   static const wxString UMBRIEL_NAME;
   static const wxString TITANIA_NAME;
   static const wxString OBERON_NAME;

   static const wxString NEPTUNE_NAME;
   static const wxString NAIAD_NAME;
   static const wxString THALASSA_NAME;
   static const wxString DESPINA_NAME;
   static const wxString GALATEA_NAME;
   static const wxString LARISSA_NAME;
   static const wxString PROTEUS_NAME;
   static const wxString TRITON_NAME;

   static const wxString PLUTO_NAME;
   static const wxString CHARON_NAME;
   
   // add other moons, asteroids, comets, as needed
   // what do we do about libration points??
   

protected:
   enum
   {
      BODIES_IN_USE = GmatBaseParamCount,
      NUMBER_OF_BODIES,
      EPHEMERIS,   // deprecated!!!!!!
      EPHEMERIS_SOURCE, 
      DE_FILE_NAME,
      SPK_FILE_NAME,
      LSK_FILE_NAME,
      OVERRIDE_TIME_SYSTEM,
      EPHEM_UPDATE_INTERVAL,
      SolarSystemParamCount
   };
      

   static const wxString
      PARAMETER_TEXT[SolarSystemParamCount - GmatBaseParamCount];
   
   static const Gmat::ParameterType
      PARAMETER_TYPE[SolarSystemParamCount - GmatBaseParamCount];
      
   Gmat::PosVelSource    pvSrcForAll;
   PlanetaryEphem*       thePlanetaryEphem;
   bool                  overrideTimeForAll;
   Real                  ephemUpdateInterval;

private:
   
   wxString theCurrentPlanetarySource;
   Integer thePlanetarySourcePriority[Gmat::PosVelSourceCount];
   bool isPlanetarySourceInUse[Gmat::PosVelSourceCount];
   static const Integer HIGHEST_PRIORITY = 10;
   
   // list for planetary source
   StringArray thePlanetarySourceTypes;
   StringArray thePlanetarySourceNames;
   StringArray thePlanetarySourceTypesInUse;
   StringArray theTempFileList;
   
   DeFile *theDefaultDeFile;
   
   /// list of the celestial bodies that are included in this solar system
   std::vector<CelestialBody*> bodiesInUse;
   
   /// the names of the bodies in use
   StringArray bodyStrings;  // is this needed, or just a convenience?
   StringArray defaultBodyStrings;
   StringArray userDefinedBodyStrings;

   /// map of SpecialCelestialPoints that the SolarSystem knows about
   std::map<wxString, SpecialCelestialPoint*>  specialPoints;

#ifdef __USE_SPICE__
   SpiceOrbitKernelReader *planetarySPK;
#endif
   /// flag indicating whether or not SPICE is allowed as a position/velocity 
   /// source for default bodies
   bool        allowSpiceForDefaultBodies;
   bool        spiceAvailable;
   /// name of the SPK file for the default bodies
   wxString theSPKFilename;
   /// name of the leap second kernel
   wxString lskKernelName;
   
   // default values for parameters
   StringArray default_planetarySourceTypesInUse;  // deprecated!!
   wxString default_ephemerisSource;
   wxString default_DEFilename;
   wxString default_SPKFilename;
   wxString default_LSKFilename;
   bool        default_overrideTimeForAll;
   Real        default_ephemUpdateInterval;
   
   // method to find a body in the solar system, given its name
   CelestialBody* FindBody(wxString withName);
   void SetJ2000Body();
   void CloneBodiesInUse(const SolarSystem &ss, bool cloneSpecialPoints = true);
   void DeleteBodiesInUse(bool deleteSpecialPoints = true);
   
   // methods to create planetary source file
   void SetDefaultPlanetarySource();
   bool CreateDeFile(const Integer id, const wxString &fileName,
                     Gmat::DeFileFormat format = Gmat::DE_BINARY);
   
//   // default values for CelestialBody data
   static const Gmat::PosVelSource    PLANET_POS_VEL_SOURCE;
   static const Integer               PLANET_ORDER[GmatSolarSystemDefaults::NumberOfDefaultPlanets];
   static const Integer               PLANET_DEGREE[GmatSolarSystemDefaults::NumberOfDefaultPlanets];
   static const Integer               PLANET_NUM_GRAVITY_MODELS[GmatSolarSystemDefaults::NumberOfDefaultPlanets];
   static const Integer               PLANET_NUM_ATMOSPHERE_MODELS[GmatSolarSystemDefaults::NumberOfDefaultPlanets];
   static const Integer               PLANET_NUM_MAGNETIC_MODELS[GmatSolarSystemDefaults::NumberOfDefaultPlanets];
   static const Integer               PLANET_NUM_SHAPE_MODELS[GmatSolarSystemDefaults::NumberOfDefaultPlanets];
   static const wxString           PLANET_GRAVITY_MODELS[];
   static const wxString           PLANET_ATMOSPHERE_MODELS[];
   static const wxString           PLANET_MAGNETIC_MODELS[];
   static const wxString           PLANET_SHAPE_MODELS[]; // @todo add Shape Models
   static const Gmat::PosVelSource    MOON_POS_VEL_SOURCE[GmatSolarSystemDefaults::NumberOfDefaultMoons];
   static const Integer               MOON_ORDER[GmatSolarSystemDefaults::NumberOfDefaultMoons];
   static const Integer               MOON_DEGREE[GmatSolarSystemDefaults::NumberOfDefaultMoons];
   static const Integer               MOON_NUM_GRAVITY_MODELS[GmatSolarSystemDefaults::NumberOfDefaultMoons];
   static const Integer               MOON_NUM_ATMOSPHERE_MODELS[GmatSolarSystemDefaults::NumberOfDefaultMoons];
   static const Integer               MOON_NUM_MAGNETIC_MODELS[GmatSolarSystemDefaults::NumberOfDefaultMoons];
   static const Integer               MOON_NUM_SHAPE_MODELS[GmatSolarSystemDefaults::NumberOfDefaultMoons];
   static const wxString           MOON_GRAVITY_MODELS[];
   static const wxString           MOON_ATMOSPHERE_MODELS[];
   static const wxString           MOON_MAGNETIC_MODELS[];
   static const wxString           MOON_SHAPE_MODELS[]; // @todo add Shape Models
   static const Gmat::PosVelSource    STAR_POS_VEL_SOURCE;
   static const Integer               STAR_ORDER;
   static const Integer               STAR_DEGREE;
   static const Integer               STAR_NUM_GRAVITY_MODELS;
   static const Integer               STAR_NUM_ATMOSPHERE_MODELS;
   static const Integer               STAR_NUM_MAGNETIC_MODELS;
   static const Integer               STAR_NUM_SHAPE_MODELS;
   static const wxString           STAR_GRAVITY_MODELS;
   static const wxString           STAR_ATMOSPHERE_MODELS;
   static const wxString           STAR_MAGNETIC_MODELS;
   static const wxString           STAR_SHAPE_MODELS; // @todo add Shape Models

};

#endif // SolarSystem_hpp

