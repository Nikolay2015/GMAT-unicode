//$Id: OrbitData.hpp 9773 2011-08-16 19:45:11Z wendys-dev $
//------------------------------------------------------------------------------
//                                  OrbitData
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
// Created: 2004/01/09
//
/**
 * Declares Orbit related data class.
 */
//------------------------------------------------------------------------------
#ifndef OrbitData_hpp
#define OrbitData_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "RefData.hpp"
#include "Rvector6.hpp"
#include "Rmatrix66.hpp"
#include "Rmatrix33.hpp"
#include "Spacecraft.hpp"
#include "SolarSystem.hpp"
#include "CoordinateSystem.hpp"
#include "CoordinateConverter.hpp"
#include "StateConverter.hpp"

class GMAT_API OrbitData : public RefData
{
public:

   OrbitData(const wxString &name = wxT(""));
   OrbitData(const OrbitData &data);
   OrbitData& operator= (const OrbitData& right);
   virtual ~OrbitData();
   
   Rvector6 GetCartState();
   Rvector6 GetKepState();
   Rvector6 GetModKepState();
   Rvector6 GetSphRaDecState();
   Rvector6 GetSphAzFpaState();
   Rvector6 GetEquinState();
   
   void SetReal(Integer item, Real rval);
   void SetRvector6(const Rvector6 &val);
   
   Real GetCartReal(Integer item);
   Real GetCartReal(const wxString &str);
   
   Real GetKepReal(Integer item);
   Real GetKepReal(const wxString &str);
   
   Real GetOtherKepReal(Integer item);
   Real GetOtherKepReal(const wxString &str);
   
   Real GetSphRaDecReal(Integer item);
   Real GetSphRaDecReal(const wxString &str);
   
   Real GetSphAzFpaReal(Integer item);
   Real GetSphAzFpaReal(const wxString &str);
   
   Real GetAngularReal(Integer item);
   Real GetAngularReal(const wxString &str);
   
   Real GetOtherAngleReal(Integer item);
   Real GetOtherAngleReal(const wxString &str);
   
   Real GetEquinReal(Integer item);
   Real GetEquinReal(const wxString &str);
   
   const Rmatrix66& GetStmRmat66(Integer item);
   const Rmatrix33& GetStmRmat33(Integer item);
   
   // The inherited methods from RefData
   virtual bool ValidateRefObjects(GmatBase *param);
   virtual const wxString* GetValidObjectList() const;
   
protected:
   
   bool mOriginDep;
   SolarSystem* GetSolarSystem();
   CoordinateSystem* GetInternalCoordSys();
   
   void SetInternalCoordSys(CoordinateSystem *cs);
   Rvector6 GetRelativeCartState(SpacePoint *origin);
   Real GetPositionMagnitude(SpacePoint *origin);
   
   // The inherited methods from RefData
   virtual void InitializeRefObjects();
   virtual bool IsValidObjectType(Gmat::ObjectType type);
   
   Rvector6 mCartState;
   Rvector6 mKepState;
   Rvector6 mModKepState;
   Rvector6 mSphRaDecState;
   Rvector6 mSphAzFpaState;
   
   Rmatrix66  mSTM;
   Rmatrix33  mSTMSubset;
   
   Real mCartEpoch;
   Real mGravConst;
   
   Integer stateTypeId;
   
   Spacecraft *mSpacecraft;
   SolarSystem *mSolarSystem;
   SpacePoint *mOrigin;
   CoordinateSystem *mInternalCoordSystem;
   CoordinateSystem *mOutCoordSystem;
   
   StateConverter   stateConverter;

   bool        firstTimeEpochWarning;

   // only one CoordinateConverter needed
   static CoordinateConverter mCoordConverter;
   
   enum {PX, PY, PZ, VX, VY, VZ};
   enum {SMA, ECC, INC, RAAN, AOP, TA, MA, EA, HA, RADN};
   enum {RD_RMAG, RD_RRA, RD_RDEC, RD_VMAG, RD_RAV, RD_DECV};
   enum {AF_RMAG, AF_RRA, AF_RDEC, AF_VMAG, AF_AZI, AF_FPA};
   enum {MM, VEL_APOAPSIS, VEL_PERIAPSIS, ORBIT_PERIOD, RAD_APOAPSIS,
         RAD_PERIAPSIS, C3_ENERGY, ENERGY};
   enum {SEMILATUS_RECTUM, HMAG, HX, HY, HZ, BETA_ANGLE, HYPERBOLIC_RLA, HYPERBOLIC_DLA};
   enum {EQ_SMA, EY, EX, NY, NX, MLONG};
   enum {ORBIT_STM, ORBIT_STM_A, ORBIT_STM_B, ORBIT_STM_C, ORBIT_STM_D};
   
   enum
   {
      SPACECRAFT = 0,
      SOLAR_SYSTEM,
      COORD_SYSTEM,
      SPACE_POINT,
      OrbitDataObjectCount
   };
   
   static const wxString VALID_OBJECT_TYPE_LIST[OrbitDataObjectCount];
   static const Real        ORBIT_DATA_TOLERANCE;
   static const wxString VALID_ANGLE_PARAM_NAMES[HYPERBOLIC_DLA - SEMILATUS_RECTUM + 1];
   static const wxString VALID_OTHER_KEPLERIAN_PARAM_NAMES[ENERGY - MM + 1];
};
#endif // OrbitData_hpp

   
