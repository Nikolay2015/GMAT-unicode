//$Id: OrbitData.cpp 9773 2011-08-16 19:45:11Z wendys-dev $
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
 * Implements Orbit related data class.
 *   VelApoapsis, VelPeriapsis, Apoapsis, Periapsis, OrbitPeriod,
 *   RadApoapsis, RadPeriapais, C3Energy, Energy
 */
//------------------------------------------------------------------------------
#include "gmatdefs.hpp"
#include "OrbitData.hpp"
#include "ParameterException.hpp"
#include "Rvector3.hpp"
#include "RealUtilities.hpp"
#include "CalculationUtilities.hpp"
#include "GmatConstants.hpp"
#include "Linear.hpp"
#include "Keplerian.hpp"         // for Cartesian to Keplerian elements
#include "AngleUtil.hpp"
//#include wxT("SphericalRADEC.hpp")    // for friend CartesianToSphericalRADEC/AZFPA()
#include "ModKeplerian.hpp"      // for friend KeplerianToModKeplerian()
//#include wxT("Equinoctial.hpp")
#include "CelestialBody.hpp"
#include "StringUtil.hpp"        // for ToString()
#include "MessageInterface.hpp"


//#define DEBUG_ORBITDATA_INIT
//#define DEBUG_ORBITDATA_CONVERT
//#define DEBUG_ORBITDATA_RUN
//#define DEBUG_CLONE
//#define DEBUG_MA
//#define DEBUG_HA
//#define DEBUG_ORBITDATA_OBJREF_EPOCH

using namespace GmatMathUtil;

//---------------------------------
// static data
//---------------------------------

CoordinateConverter OrbitData::mCoordConverter = CoordinateConverter();

const wxString
OrbitData::VALID_OBJECT_TYPE_LIST[OrbitDataObjectCount] =
{
   wxT("Spacecraft"),
   wxT("SolarSystem"),
   wxT("CoordinateSystem"),
   wxT("SpacePoint")
};


const Real OrbitData::ORBIT_DATA_TOLERANCE     = 2.0e-10;

const wxString OrbitData::VALID_ANGLE_PARAM_NAMES[HYPERBOLIC_DLA - SEMILATUS_RECTUM + 1] =
{
   wxT("SemilatusRectum"),
   wxT("HMag"),
   wxT("HX"),
   wxT("HY"),
   wxT("HZ"),
   wxT("BetaAngle"),
   wxT("RLA"),
   wxT("DLA")
};

const wxString OrbitData::VALID_OTHER_KEPLERIAN_PARAM_NAMES[ENERGY - MM + 1] =
{
   wxT("MeanMotion"),
   wxT("VelApoapsis"),
   wxT("VelPeriapsis"),
   wxT("OrbitPeriod"),
   wxT("RadApoapsis"),
   wxT("RadPeriapsis"),
   wxT("C3Energy"),
   wxT("Energy")
};


//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// OrbitData(const wxString &name = wxT(""))
//------------------------------------------------------------------------------
/**
 * Constructor.
 */
//------------------------------------------------------------------------------
OrbitData::OrbitData(const wxString &name)
   : RefData(name),
   stateTypeId (-1)
{
   mCartState = Rvector6::RVECTOR6_UNDEFINED;
   mKepState = Rvector6::RVECTOR6_UNDEFINED;
   mModKepState = Rvector6::RVECTOR6_UNDEFINED;
   mSphRaDecState = Rvector6::RVECTOR6_UNDEFINED;
   mSphAzFpaState = Rvector6::RVECTOR6_UNDEFINED;
   mCartEpoch = 0.0;
   mGravConst = 0.0;
   
   mSpacecraft = NULL;
   mSolarSystem = NULL;
   mOrigin = NULL;
   mInternalCoordSystem = NULL;
   mOutCoordSystem = NULL;

   firstTimeEpochWarning = false;
}


//------------------------------------------------------------------------------
// OrbitData(const OrbitData &data)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <data> the OrbitData object being copied.
 */
//------------------------------------------------------------------------------
OrbitData::OrbitData(const OrbitData &data)
   : RefData(data),
   stateTypeId (data.stateTypeId)
{
   #ifdef DEBUG_CLONE
   MessageInterface::ShowMessage
      (wxT("OrbitData::OrbitData copy constructor called\n"));
   #endif
   
   mCartState = data.mCartState;
   mKepState = data.mKepState;
   mModKepState = data.mModKepState;
   mSphRaDecState = data.mSphRaDecState;
   mSphAzFpaState = data.mSphAzFpaState;
   mCartEpoch = data.mCartEpoch;
   mGravConst = data.mGravConst;
   
   mSpacecraft = data.mSpacecraft;
   mSolarSystem = data.mSolarSystem;
   mOrigin = data.mOrigin;
   mInternalCoordSystem = data.mInternalCoordSystem;
   mOutCoordSystem = data.mOutCoordSystem;

   firstTimeEpochWarning = data.firstTimeEpochWarning;
}


//------------------------------------------------------------------------------
// OrbitData& operator= (const OrbitData &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the object being copied.
 *
 * @return reference to this object
 */
//------------------------------------------------------------------------------
OrbitData& OrbitData::operator= (const OrbitData &right)
{
   if (this == &right)
      return *this;
   
   RefData::operator=(right);
   
   mCartState = right.mCartState;
   mKepState = right.mKepState;
   mModKepState = right.mModKepState;
   mSphRaDecState = right.mSphRaDecState;
   mSphAzFpaState = right.mSphAzFpaState;
   mCartEpoch = right.mCartEpoch;
   mGravConst = right.mGravConst;
   
   mSpacecraft = right.mSpacecraft;
   mSolarSystem = right.mSolarSystem;
   mOrigin = right.mOrigin;
   mInternalCoordSystem = right.mInternalCoordSystem;
   mOutCoordSystem = right.mOutCoordSystem;
   
   stateTypeId = right.stateTypeId;
   
   firstTimeEpochWarning = right.firstTimeEpochWarning;

   return *this;
}


//------------------------------------------------------------------------------
// ~OrbitData()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
OrbitData::~OrbitData()
{
   #ifdef DEBUG_ORBITDATA_DESTRUCTOR
   MessageInterface::ShowMessage(wxT("OrbitData::~OrbitData()\n"));
   #endif
}


//------------------------------------------------------------------------------
// void SetReal(Integer item, Real rval)
//------------------------------------------------------------------------------
void OrbitData::SetReal(Integer item, Real rval)
{
   #if DEBUG_ORBITDATA_SET
   MessageInterface::ShowMessage
      (wxT("OrbitData::SetReal() item=%d, rval=%f\n"), item, rval);
   #endif
   
   if (mSpacecraft == NULL)
      InitializeRefObjects();
   
   if (mSpacecraft == NULL)
   {
      MessageInterface::ShowMessage
         (wxT("*** INTERNAL ERROR *** Cannot find Spacecraft object so returning %f\n"),
          GmatOrbitConstants::ORBIT_REAL_UNDEFINED);
   }
   
   CoordinateSystem *satCS =
      (CoordinateSystem*)mSpacecraft->GetRefObject(Gmat::COORDINATE_SYSTEM, wxT(""));
   
   #if DEBUG_ORBITDATA_SET
   MessageInterface::ShowMessage
      (wxT("   Parameter CS Origin = '%s'\n"), mOutCoordSystem->GetOriginName().c_str());
   MessageInterface::ShowMessage
      (wxT("   Sat       CS Origin = '%s'\n"), satCS->GetOriginName().c_str());
   #endif
   
   if (mOutCoordSystem != satCS)
   {
      ParameterException pe;
      pe.SetDetails(wxT("Currently GMAT cannot set %s; the spacecraft '%s' ")
                    wxT("requires values to be in the '%s' coordinate system (setting ")
                    wxT("values in different coordinate systems will be implemented in ")
                    wxT("future builds)"),  mName.c_str(), mSpacecraft->GetName().c_str(),
                    satCS->GetName().c_str());
      throw pe;
   }
   
   
   switch (item)
   {
   case PX:
      mSpacecraft->SetRealParameter(mSpacecraft->GetParameterID(wxT("X")), rval);
      break;
   case PY:
      mSpacecraft->SetRealParameter(mSpacecraft->GetParameterID(wxT("Y")), rval);
      break;
   case PZ:
      mSpacecraft->SetRealParameter(mSpacecraft->GetParameterID(wxT("Z")), rval);
      break;
   case VX:
      mSpacecraft->SetRealParameter(mSpacecraft->GetParameterID(wxT("VX")), rval);
      break;
   case VY:
      mSpacecraft->SetRealParameter(mSpacecraft->GetParameterID(wxT("VY")), rval);
      break;
   case VZ:
      mSpacecraft->SetRealParameter(mSpacecraft->GetParameterID(wxT("VZ")), rval);
      break;
   default:
      throw ParameterException(wxT("OrbitData::SetReal() Unknown parameter id: ") +
                               GmatRealUtil::ToString(item));
   }
}


//------------------------------------------------------------------------------
// void SetRvector6(const Rvector6 &val)
//------------------------------------------------------------------------------
void OrbitData::SetRvector6(const Rvector6 &val)
{
   #if DEBUG_ORBITDATA_SET
   MessageInterface::ShowMessage
      (wxT("OrbitData::SetRvector6() entered, rval=[%s]\n"), rval.ToString().c_str());
   #endif
   
   if (mSpacecraft == NULL)
      InitializeRefObjects();
   
   if (mSpacecraft == NULL)
   {
      MessageInterface::ShowMessage
         (wxT("*** INTERNAL ERROR *** Cannot find Spacecraft object so returning %f\n"),
          GmatOrbitConstants::ORBIT_REAL_UNDEFINED);
   }
   
   mSpacecraft->SetState(val);
}


//------------------------------------------------------------------------------
// Rvector6 GetCartState()
//------------------------------------------------------------------------------
Rvector6 OrbitData::GetCartState()
{
   if (mSpacecraft == NULL || mSolarSystem == NULL)
      InitializeRefObjects();
   
   mCartEpoch = mSpacecraft->GetEpoch();
   mCartState.Set(mSpacecraft->GetState().GetState());
   
   #ifdef DEBUG_ORBITDATA_RUN
   MessageInterface::ShowMessage
      (wxT("OrbitData::GetCartState() '%s' mCartState=\n   %s\n"),
       mSpacecraft->GetName().c_str(), mCartState.ToString().c_str());
   #endif
   
   // if origin dependent parameter, the relative position/velocity is computed in
   // the parameter calculation, so just return prop state.
   if (mOriginDep)
      return mCartState;
   
   if (mInternalCoordSystem == NULL || mOutCoordSystem == NULL)
   {
      MessageInterface::ShowMessage
         (wxT("OrbitData::GetCartState() Internal CoordSystem or Output CoordSystem is NULL.\n"));
      
      throw ParameterException
         (wxT("OrbitData::GetCartState() internal or output CoordinateSystem is NULL.\n"));
   }
   
   //-----------------------------------------------------------------
   // convert to output CoordinateSystem
   //-----------------------------------------------------------------
   if (mInternalCoordSystem->GetName() != mOutCoordSystem->GetName())
   {
      #ifdef DEBUG_ORBITDATA_CONVERT
         MessageInterface::ShowMessage
            (wxT("OrbitData::GetCartState() mOutCoordSystem:%s(%s), Axis addr=%d\n"),
             mOutCoordSystem->GetName().c_str(),
             mOutCoordSystem->GetTypeName().c_str(),
             mOutCoordSystem->GetRefObject(Gmat::AXIS_SYSTEM, wxT("")));
         if (mOutCoordSystem->AreAxesOfType(wxT("ObjectReferencedAxes")))
               MessageInterface::ShowMessage(wxT("OrbitData::GetCartState() <-- ")
                     wxT("mOutCoordSystem IS of type ObjectReferencedAxes!!!\n"));
         else
            MessageInterface::ShowMessage(wxT("OrbitData::GetCartState() <-- ")
                  wxT("mOutCoordSystem IS NOT of type ObjectReferencedAxes!!!\n"));
         MessageInterface::ShowMessage
            (wxT("OrbitData::GetCartState() <-- Before convert: mCartEpoch=%f\n")
                  wxT("state = %s\n"), mCartEpoch, mCartState.ToString().c_str());
         MessageInterface::ShowMessage
            (wxT("OrbitData::GetCartState() <-- firstTimeEpochWarning = %s\n"),
             (firstTimeEpochWarning? wxT("true") : wxT("false")));

      #endif

      if ((mOutCoordSystem->AreAxesOfType(wxT("ObjectReferencedAxes"))) && !firstTimeEpochWarning)
      {
         GmatBase *objRefOrigin = mOutCoordSystem->GetOrigin();
         if (objRefOrigin->IsOfType(wxT("Spacecraft")))
         {
            wxString objRefScName = ((Spacecraft*) objRefOrigin)->GetName();
            if (objRefScName != mSpacecraft->GetName())
            {
               // Get the epochs of the spacecraft to see if they are different
               Real scEpoch   = mSpacecraft->GetRealParameter(wxT("A1Epoch"));
               Real origEpoch = ((Spacecraft*) objRefOrigin)->GetRealParameter(wxT("A1Epoch"));
               #ifdef DEBUG_ORBITDATA_OBJREF_EPOCH
                  MessageInterface::ShowMessage(wxT("obj ref cs sc epoch = %12.10f\n"), origEpoch);
                  MessageInterface::ShowMessage(wxT("   and the sc epoch = %12.10f\n"), scEpoch);
               #endif
               if (!GmatMathUtil::IsEqual(scEpoch, origEpoch, ORBIT_DATA_TOLERANCE))
               {
                  wxString errmsg = wxT("Warning:  In Coordinate System \"");
                  errmsg += mOutCoordSystem->GetName() + wxT("\", \"");
                  errmsg += mSpacecraft->GetName() + wxT("\" and \"");
                  errmsg += objRefScName + wxT("\" have different epochs.\n");
//                  MessageInterface::PopupMessage(Gmat::WARNING_, errmsg);
                  MessageInterface::ShowMessage(errmsg);
                  firstTimeEpochWarning = true;
               }
            }
         }
      }
      
      try
      {
         #ifdef DEBUG_ORBITDATA_CONVERT
            MessageInterface::ShowMessage(wxT("    --> Converting from %s to %s\n\n"),
                  mInternalCoordSystem->GetName().c_str(),
                  mOutCoordSystem->GetName().c_str());
         #endif
         mCoordConverter.Convert(A1Mjd(mCartEpoch), mCartState, mInternalCoordSystem,
                                 mCartState, mOutCoordSystem, true);
         #ifdef DEBUG_ORBITDATA_CONVERT
            MessageInterface::ShowMessage
               (wxT("OrbitData::GetCartState() --> After  convert: mCartEpoch=%f\n")
                wxT("state = %s\n"), mCartEpoch, mCartState.ToString().c_str());
         #endif
      }
      catch (BaseException &e)
      {
         MessageInterface::ShowMessage
            (wxT("OrbitData::GetCartState() Failed to convert to %s coordinate system.\n   %s\n"),
             mOutCoordSystem->GetName().c_str(), e.GetFullMessage().c_str());
         wxString errmsg = wxT("OrbitData::GetCartState() Failed to convert to ") ;
         errmsg += mOutCoordSystem->GetName() + wxT(" coordinate system.\n");
         errmsg += wxT("Message: ") + e.GetFullMessage() + wxT("\n");
         throw ParameterException(errmsg);
      }
   }
   
   return mCartState;
}


//------------------------------------------------------------------------------
// Rvector6 GetKepState()
//------------------------------------------------------------------------------
Rvector6 OrbitData::GetKepState()
{
   if (mSpacecraft == NULL || mSolarSystem == NULL)
      InitializeRefObjects();

   #ifdef DEBUG_ORBITDATA_STATE
   MessageInterface::ShowMessage(wxT("rbitData::GetKepState() from SC mKepState=%s\n"),
                                  mKepState.ToString().c_str());
   #endif
   
   // Call GetCartState() to convert to parameter coord system first
   Rvector6 state = GetCartState();
   mKepState = Keplerian::CartesianToKeplerian(mGravConst, state);

   #ifdef DEBUG_ORBITDATA_KEP_STATE
   MessageInterface::ShowMessage(wxT("OrbitData::GetKepState() mKepState=%s\n"),
                                 mKepState.ToString().c_str());
   #endif
   
   return mKepState;
}


//------------------------------------------------------------------------------
// Rvector6 GetModKepState()
//------------------------------------------------------------------------------
Rvector6 OrbitData::GetModKepState()
{
   if (mSpacecraft == NULL || mSolarSystem == NULL)
      InitializeRefObjects();
      
   // Call GetCartState() to convert to parameter coord system first
   Rvector6 state = GetCartState();
   Anomaly anomaly;
   Rvector6 kepState = CoordUtil::CartesianToKeplerian(state, mGravConst, anomaly);
   mModKepState = KeplerianToModKeplerian(kepState);
   
   return mModKepState;
}


//------------------------------------------------------------------------------
// Rvector6 OrbitData::GetSphRaDecState()
//------------------------------------------------------------------------------
Rvector6 OrbitData::GetSphRaDecState()
{
   if (mSpacecraft == NULL || mSolarSystem == NULL)
      InitializeRefObjects();
   
   // Call GetCartState() to convert to parameter coord system first
   Rvector6 state = GetCartState();
//   mSphRaDecState = CartesianToSphericalRADEC(state);
   mSphRaDecState = stateConverter.FromCartesian(state, wxT("SphericalRADEC"));

   #ifdef DEBUG_ORBITDATA_STATE
   MessageInterface::ShowMessage
      (wxT("OrbitData::GetSphRaDecState() mSphRaDecState=\n   %s\n"),
       mSphRaDecState.ToString().c_str());
   #endif
   
   return mSphRaDecState;
}


//------------------------------------------------------------------------------
// Rvector6 OrbitData::GetSphAzFpaState()
//------------------------------------------------------------------------------
Rvector6 OrbitData::GetSphAzFpaState()
{
   if (mSpacecraft == NULL || mSolarSystem == NULL)
      InitializeRefObjects();
   
   // Call GetCartState() to convert to parameter coord system first
   Rvector6 state = GetCartState();
   //   mSphAzFpaState = CartesianToSphericalAZFPA(state);
   mSphAzFpaState = stateConverter.FromCartesian(state, wxT("SphericalAZFPA"));

   
   return mSphAzFpaState;
}


//------------------------------------------------------------------------------
// Rvector6 OrbitData::GetEquinState()
//------------------------------------------------------------------------------
Rvector6 OrbitData::GetEquinState()
{
   if (mSpacecraft == NULL || mSolarSystem == NULL)
      InitializeRefObjects();
   
   // Call GetCartState() to convert to parameter coord system first
   Rvector6 state = GetCartState();
//   Rvector6 mEquinState = CartesianToEquinoctial(state, mGravConst);
   stateConverter.SetMu(mGravConst);
   Rvector6 mEquinState = stateConverter.FromCartesian(state, wxT("Equinoctial"));
   
   return mEquinState;
}


//------------------------------------------------------------------------------
// Real GetCartReal(Integer item)
//------------------------------------------------------------------------------
/**
 * Retrives Cartesian element
 */
//------------------------------------------------------------------------------
Real OrbitData::GetCartReal(Integer item)
{
   #ifdef DEBUG_ORBITDATA_RUN
   MessageInterface::ShowMessage(wxT("OrbitData::GetCartReal() item=%d\n"), item);
   #endif
   
   Rvector6 state = GetCartState();
   if (item >= PX && item <= VZ)
      return mCartState[item];
   else
      throw ParameterException(wxT("OrbitData::GetCartReal() Unknown parameter id: ") +
                               GmatRealUtil::ToString(item));
}


//------------------------------------------------------------------------------
// Real GetKepReal(Integer item)
//------------------------------------------------------------------------------
/**
 * Retrives Keplerian element
 */
//------------------------------------------------------------------------------
Real OrbitData::GetKepReal(Integer item)
{
   #ifdef DEBUG_ORBITDATA_RUN
   MessageInterface::ShowMessage(wxT("OrbitData::GetKepReal() item=%d\n"), item);
   #endif
   
   Rvector6 state = GetCartState();

   if (mOriginDep && mOrigin->GetName() != wxT("Earth"))
   {
      state = state - mOrigin->GetMJ2000State(mCartEpoch);
   }
   
   Rvector3 pos(state[0], state[1], state[2]);
   Rvector3 vel(state[3], state[4], state[5]);   
   Real rMag = pos.GetMagnitude();
   
   if (rMag < GmatOrbitConstants::KEP_ZERO_TOL)
      throw ParameterException
         (wxT("OrbitData::GetKepReal(") + GmatRealUtil::ToString(item) +
          wxT(") position vector is zero. pos: ") + pos.ToString() + wxT(" vel: ") +
          vel.ToString());
   
   switch (item)
   {
   case SMA:
      return Keplerian::CartesianToSMA(mGravConst, pos, vel);
   case ECC:
      return Keplerian::CartesianToECC(mGravConst, pos, vel);
   case INC:
      return Keplerian::CartesianToINC(mGravConst, pos, vel);
   case TA:
      return Keplerian::CartesianToTA(mGravConst, pos, vel);
   case EA:
      return Keplerian::CartesianToEA(mGravConst, pos, vel);
   case MA:
   {
      #ifdef DEBUG_MA
      MessageInterface::ShowMessage(wxT("In OrbitData, computing MA -------\n"));
      #endif
      return Keplerian::CartesianToMA(mGravConst, pos, vel);
   }
   case HA:
   {
      #ifdef DEBUG_HA
      MessageInterface::ShowMessage(wxT("In OrbitData, computing HA -------\n"));
      #endif
      return Keplerian::CartesianToHA(mGravConst, pos, vel);
   }
   case RAAN:
      return Keplerian::CartesianToRAAN(mGravConst, pos, vel);
   case RADN:
      {
         Real raanDeg = Keplerian::CartesianToRAAN(mGravConst, pos, vel);
         return AngleUtil::PutAngleInDegRange(raanDeg + 180, 0.0, 360.0);
      }
   case AOP:
      return Keplerian::CartesianToAOP(mGravConst, pos, vel);
      
   default:
      throw ParameterException(wxT("OrbitData::GetKepReal() Unknown parameter id: ") +
                               GmatRealUtil::ToString(item));
   }
}


//------------------------------------------------------------------------------
// Real GetOtherKepReal(Integer item)
//------------------------------------------------------------------------------
/**
 * Retrives other Keplerian element.
 */
//------------------------------------------------------------------------------
Real OrbitData::GetOtherKepReal(Integer item)
{
   #ifdef DEBUG_ORBITDATA_RUN
   MessageInterface::ShowMessage(wxT("OrbitData::GetOtherKepReal() item=%d\n"), item);
   #endif
   
   Rvector6 state = GetCartState();
   
   if (mOriginDep && mOrigin->GetName() != wxT("Earth"))
   {
      state = state - mOrigin->GetMJ2000State(mCartEpoch);
   }
   
   return GmatCalcUtil::CalculateKeplerianData(VALID_OTHER_KEPLERIAN_PARAM_NAMES[item-MM], state, mGravConst);
   
}


//------------------------------------------------------------------------------
// Real GetSphRaDecReal(Integer item)
//------------------------------------------------------------------------------
/**
 * Retrives Spherical element.
 */
//------------------------------------------------------------------------------
Real OrbitData::GetSphRaDecReal(Integer item)
{
   #ifdef DEBUG_ORBITDATA_RUN
   MessageInterface::ShowMessage(wxT("OrbitData::GetSphRaDecReal() item=%d\n"), item);
   #endif
   
   Rvector6 state = GetSphRaDecState();

   #ifdef DEBUG_ORBITDATA_RUN
   MessageInterface::ShowMessage
      (wxT("OrbitData::GetSphRaDecReal() item=%d state=%s\n"),
       item, state.ToString().c_str());
   #endif
   
   switch (item)
   {
   case RD_RMAG:
      {
         // if orgin is wxT("Earth") just return default
         if (mOrigin->GetName() == wxT("Earth"))
            return mSphRaDecState[RD_RMAG];
         else
            return GetPositionMagnitude(mOrigin);
      }
   case RD_RRA:
   case RD_RDEC:
   case RD_VMAG:
   case RD_RAV:
   case RD_DECV:
      return mSphRaDecState[item];
   default:
      throw ParameterException
         (wxT("OrbitData::GetSphRaDecReal() Unknown parameter name: ") +
          GmatRealUtil::ToString(item));
   }
}


//------------------------------------------------------------------------------
// Real GetSphAzFpaReal(Integer item)
//------------------------------------------------------------------------------
/**
 * Retrives Spherical element.
 */
//------------------------------------------------------------------------------
Real OrbitData::GetSphAzFpaReal(Integer item)
{
   #ifdef DEBUG_ORBITDATA_RUN
   MessageInterface::ShowMessage(wxT("OrbitData::GetSphAzFpaReal() item=%d\n"), item);
   #endif
   
   Rvector6 state = GetSphAzFpaState();

   #ifdef DEBUG_ORBITDATA_RUN
   MessageInterface::ShowMessage
      (wxT("OrbitData::GetSphAzFpaReal() item=%s state=%s\n"),
       item, state.ToString().c_str());
   #endif

   if (item >= AF_RMAG && item <= AF_FPA)
      return mSphAzFpaState[item];
   else
   {
      throw ParameterException(wxT("OrbitData::GetSphAzFpaReal() Unknown parameter ID: ") +
                               GmatRealUtil::ToString(item));
   }
}


//------------------------------------------------------------------------------
// Real GetAngularReal(Integer item)
//------------------------------------------------------------------------------
/**
 * Computes angular related parameter.
 */
//------------------------------------------------------------------------------
Real OrbitData::GetAngularReal(Integer item)
{
   #ifdef DEBUG_ORBITDATA_RUN
   MessageInterface::ShowMessage(wxT("OrbitData::GetAngularReal() item=%d\n"), item);
   #endif
   
   Rvector6 state = GetCartState();
   Rvector3 dummy;

   #ifdef DEBUG_ORBITDATA_RUN
   MessageInterface::ShowMessage
      (wxT("OrbitData::GetAngularReal() item=%d state=%s\n"),
       item, state.ToString().c_str());
   #endif
   
   return GmatCalcUtil::CalculateAngularData(VALID_ANGLE_PARAM_NAMES[item-SEMILATUS_RECTUM], state, mGravConst, dummy);

}


//------------------------------------------------------------------------------
// Real GetOtherAngleReal(Integer item)
//------------------------------------------------------------------------------
/**
 * Computes other angle related parameters.
 */
//------------------------------------------------------------------------------
Real OrbitData::GetOtherAngleReal(Integer item)
{
   #ifdef DEBUG_ORBITDATA_RUN
   MessageInterface::ShowMessage(wxT("OrbitData::GetOtherAngleReal() item=%d\n"), item);
   #endif
   
   Rvector6 state;
   if (mOrigin->GetName() != wxT("Earth"))
      state = GetRelativeCartState(mOrigin);
   else
      state = GetCartState();

   // compute sun unit vector from the origin
   Rvector3 sunPos = (mSolarSystem->GetBody(SolarSystem::SUN_NAME))->
      GetMJ2000Position(mCartEpoch);
   Rvector3 originPos = mOrigin->GetMJ2000Position(mCartEpoch);
   Rvector3 originToSun = sunPos - originPos;
   originToSun.Normalize();

   return GmatCalcUtil::CalculateAngularData(VALID_ANGLE_PARAM_NAMES[item-SEMILATUS_RECTUM], state, mGravConst, originToSun);
}


//------------------------------------------------------------------------------
// Real GetEquiReal(Integer item)
//------------------------------------------------------------------------------
/**
 * Retrieves Equinoctial element
 */
//------------------------------------------------------------------------------
Real OrbitData::GetEquinReal(Integer item)
{
   #ifdef DEBUG_ORBITDATA_RUN
   MessageInterface::ShowMessage(wxT("OrbitData::GetEquinReal() item=%d\n"), item);
   #endif
   
   Rvector6 state = GetCartState();

   if (mOriginDep && mOrigin->GetName() != wxT("Earth"))
   {
      state = state - mOrigin->GetMJ2000State(mCartEpoch);
   }
   
   Rvector3 pos(state[0], state[1], state[2]);
   Rvector3 vel(state[3], state[4], state[5]);   
   Real rMag = pos.GetMagnitude();
   
   if (rMag < GmatOrbitConstants::KEP_ZERO_TOL)
      throw ParameterException
         (wxT("OrbitData::GetEquiReal(") + GmatRealUtil::ToString(item) +
          wxT(") position vector is zero. pos: ") + pos.ToString() + wxT(" vel: ") +
          vel.ToString());
   
   switch (item)
   {
   case EQ_SMA:
      return Keplerian::CartesianToSMA(mGravConst, pos, vel);
   case EY:
      {
         Rvector6 equiState = GetEquinState();
         return equiState[1];
      }
   case EX:
      {
         Rvector6 equiState = GetEquinState();
         return equiState[2];
      }
   case NY:
      {
         Rvector6 equiState = GetEquinState();
         return equiState[3];
      }
   case NX:
      {
         Rvector6 equiState = GetEquinState();
         return equiState[4];
      }
   case MLONG:
      {
         Rvector6 equiState = GetEquinState();
         return equiState[5];
      }
      
   default:
      throw ParameterException(wxT("OrbitData::GetEquinReal() Unknown parameter id: ") +
                               GmatRealUtil::ToString(item));
   }
}


//------------------------------------------------------------------------------
// const Rmatrix66& GetStmRmat66(Integer item)
//------------------------------------------------------------------------------
/**
 * Retrives Spacecraft Rmatrix66 data.
 */
//------------------------------------------------------------------------------
const Rmatrix66& OrbitData::GetStmRmat66(Integer item)
{
   #ifdef DEBUG_SCDATA_GET
   MessageInterface::ShowMessage
      (wxT("OrbitData::GetStmRmat66() entered, mSpacecraft=<%p>'%s'\n"),
       mSpacecraft, mSpacecraft ? mSpacecraft->GetName().c_str() : wxT("NULL"));
   #endif
   
   if (mSpacecraft == NULL)
      InitializeRefObjects();
   
   switch (item)
   {
   case ORBIT_STM:
      {
         mSTM = mSpacecraft->GetRmatrixParameter(wxT("OrbitSTM"));
         return mSTM;
      }
   default:
      // otherwise, there is an error   
      throw ParameterException
         (wxT("OrbitData::GetStmRmat66() Unknown parameter id: ") +
          GmatStringUtil::ToString(item));
   }
}


//------------------------------------------------------------------------------
// const Rmatrix33& GetStmRmat33(Integer item)
//------------------------------------------------------------------------------
/**
 * Retrives Spacecraft Rmatrix33 data.
 */
//------------------------------------------------------------------------------
const Rmatrix33& OrbitData::GetStmRmat33(Integer item)
{
   if (mSpacecraft == NULL)
      InitializeRefObjects();
   
   mSTM = mSpacecraft->GetRmatrixParameter(wxT("OrbitSTM"));
   
   switch (item)
   {
   case ORBIT_STM_A:
      mSTMSubset = mSTM.UpperLeft();
      return mSTMSubset;
   case ORBIT_STM_B:
      mSTMSubset = mSTM.UpperRight();
      return mSTMSubset;
   case ORBIT_STM_C:
      mSTMSubset = mSTM.LowerLeft();
      return mSTMSubset;
   case ORBIT_STM_D:
      mSTMSubset = mSTM.LowerRight();
      return mSTMSubset;
   default:
      // otherwise, there is an error   
      throw ParameterException
         (wxT("OrbitData::GetStmRmat33() Unknown parameter id: ") +
          GmatStringUtil::ToString(item));
   }
}

//-------------------------------------
// Inherited methods from RefData
//-------------------------------------

//------------------------------------------------------------------------------
// virtual const wxString* GetValidObjectList() const
//------------------------------------------------------------------------------
const wxString* OrbitData::GetValidObjectList() const
{
   return VALID_OBJECT_TYPE_LIST;
}


//------------------------------------------------------------------------------
// bool ValidateRefObjects(GmatBase *param)
//------------------------------------------------------------------------------
/**
 * Validates reference objects for given parameter
 */
//------------------------------------------------------------------------------
bool OrbitData::ValidateRefObjects(GmatBase *param)
{
   int objCount = 0;
   for (int i=0; i<OrbitDataObjectCount; i++)
   {
      if (HasObjectType(VALID_OBJECT_TYPE_LIST[i]))
         objCount++;
   }

   if (objCount == OrbitDataObjectCount)
      return true;
   else
      return false;
}

//---------------------------------
// protected methods
//---------------------------------

//------------------------------------------------------------------------------
// SolarSystem* GetSolarSystem()
//------------------------------------------------------------------------------
SolarSystem* OrbitData::GetSolarSystem()
{
   return mSolarSystem;
}


//------------------------------------------------------------------------------
// CoordinateSystem* GetInternalCoordSys()
//------------------------------------------------------------------------------
CoordinateSystem* OrbitData::GetInternalCoordSys()
{
   return mInternalCoordSystem;
}


//------------------------------------------------------------------------------
// void SetInternalCoordSystem(CoordinateSystem *cs)
//------------------------------------------------------------------------------
/*
 * @param <cs> internal coordinate system what parameter data is representing.
 */
//------------------------------------------------------------------------------ 
void OrbitData::SetInternalCoordSys(CoordinateSystem *cs)
{
   mInternalCoordSystem = cs;
}



//------------------------------------------------------------------------------
// Rvector6 OrbitData::GetRelativeCartState(SpacePoint *origin)
//------------------------------------------------------------------------------
/**
 * Computes spacecraft cartesian state from the given origin.
 *
 * @param <origin> origin pointer
 *
 * @return spacecraft state from the given origin.
 */
//------------------------------------------------------------------------------
Rvector6 OrbitData::GetRelativeCartState(SpacePoint *origin)
{
   // get spacecraft state
   Rvector6 scState = GetCartState();
   
   // get origin state
   Rvector6 originState = origin->GetMJ2000State(mCartEpoch);
   
   #ifdef DEBUG_ORBITDATA_RUN
      MessageInterface::ShowMessage
         (wxT("OrbitData::GetRelativeCartState() origin=%s, state=%s\n"),
          origin->GetName().c_str(), originState.ToString().c_str());
   #endif
      
   // return relative state
   return scState - originState;
}


//------------------------------------------------------------------------------
// Real OrbitData::GetPositionMagnitude(SpacePoint *origin)
//------------------------------------------------------------------------------
/**
 * Computes position magnitude from the given origin.
 *
 * @param <origin> origin pointer
 *
 * @return position magnitude from the given origin.
 */
//------------------------------------------------------------------------------
Real OrbitData::GetPositionMagnitude(SpacePoint *origin)
{
   // get spacecraft position
   Rvector6 scState = GetCartState();
   Rvector3 scPos = Rvector3(scState[0], scState[1], scState[2]);

   // get origin position
   Rvector6 originState = origin->GetMJ2000State(mCartEpoch);
   Rvector3 originPos = Rvector3(originState[0], originState[1], originState[2]);

   // get relative position magnitude
   Rvector3 relPos = scPos - originPos;
   
   #ifdef DEBUG_ORBITDATA_RUN
      MessageInterface::ShowMessage
         (wxT("OrbitData::GetPositionMagnitude() scPos=%s, originPos=%s, relPos=%s\n"),
          scPos.ToString().c_str(), originPos.ToString().c_str(),
          relPos.ToString().c_str());
   #endif
   
   return relPos.GetMagnitude();
}


// The inherited methods from RefData
//------------------------------------------------------------------------------
// virtual void InitializeRefObjects()
//------------------------------------------------------------------------------
void OrbitData::InitializeRefObjects()
{
   #ifdef DEBUG_ORBITDATA_INIT
   MessageInterface::ShowMessage(wxT("OrbitData::InitializeRefObjects() entered.\n"));
   #endif
   
   mSpacecraft =
      (Spacecraft*)FindFirstObject(VALID_OBJECT_TYPE_LIST[SPACECRAFT]);
   
   if (mSpacecraft == NULL)
   {
      #ifdef DEBUG_ORBITDATA_INIT
      MessageInterface::ShowMessage
         (wxT("OrbitData::InitializeRefObjects() Cannot find spacecraft: ") +
          GetRefObjectName(Gmat::SPACECRAFT) + wxT(".\n") +
          wxT("Make sure Spacecraft is set to any internal parameters.\n"));
      #endif
      
      throw ParameterException
         (wxT("Cannot find spacecraft: ") + GetRefObjectName(Gmat::SPACECRAFT));
   }
   
   if (stateTypeId == -1)
      stateTypeId = mSpacecraft->GetParameterID(wxT("DisplayStateType"));
   
   mSolarSystem =
      (SolarSystem*)FindFirstObject(VALID_OBJECT_TYPE_LIST[SOLAR_SYSTEM]);
   
   if (mSolarSystem == NULL)
      throw ParameterException
         (wxT("OrbitData::InitializeRefObjects() Cannot find SolarSystem object\n"));
   
   if (mInternalCoordSystem == NULL)
      throw ParameterException
         (wxT("OrbitData::InitializeRefObjects() Cannot find internal ")
          wxT("CoordinateSystem object\n"));
   
   //-----------------------------------------------------------------
   // if dependent body name exist and it is a CelestialBody,
   // it is origin dependent parameter, set new gravity constant.
   //-----------------------------------------------------------------
   wxString originName =
      FindFirstObjectName(GmatBase::GetObjectType(VALID_OBJECT_TYPE_LIST[SPACE_POINT]));
   
   mOriginDep = false;
   
   if (originName != wxT(""))
   {
      #ifdef DEBUG_ORBITDATA_INIT
      MessageInterface::ShowMessage
         (wxT("OrbitData::InitializeRefObjects() getting originName: %s pointer.\n"),
          originName.c_str());
      #endif
      
      mOrigin =
         (SpacePoint*)FindFirstObject(VALID_OBJECT_TYPE_LIST[SPACE_POINT]);
      
      if (!mOrigin)
         throw InvalidDependencyException
            (wxT(" is a central body dependent parameter."));
      
      // override gravity constant if origin is CelestialBody
      if (mOrigin->IsOfType(Gmat::CELESTIAL_BODY))
         mGravConst = ((CelestialBody*)mOrigin)->GetGravitationalConstant();

      mOriginDep = true;
   }
   //-----------------------------------------------------------------
   // It is CoordinateSystem dependent parameter.
   // Set Origin to CoordinateSystem origin and mu to origin mu.
   //-----------------------------------------------------------------
   else
   {
      mOutCoordSystem =
         (CoordinateSystem*)FindFirstObject(VALID_OBJECT_TYPE_LIST[COORD_SYSTEM]);
      
      if (mOutCoordSystem == NULL)
      {
         #ifdef DEBUG_ORBITDATA_INIT
         MessageInterface::ShowMessage
            (wxT("OrbitData::InitializeRefObjects() Cannot find output ")
             wxT("CoordinateSystem object\n"));
         #endif
         
         throw ParameterException
            (wxT("OrbitData::InitializeRefObjects() Cannot find coordinate system.\n"));
      }
      
      
      // Set origin to out coordinate system origin for CoordinateSystem
      // dependent parameter
      mOrigin = mOutCoordSystem->GetOrigin();
      
      if (!mOrigin)
      {
         #ifdef DEBUG_ORBITDATA_INIT
         MessageInterface::ShowMessage
            (wxT("OrbitData::InitializeRefObjects() origin not found: ") +
             mOutCoordSystem->GetOriginName() + wxT("\n"));
         #endif
      
         throw ParameterException
            (wxT("OrbitData::InitializeRefObjects() The origin of CoordinateSystem \"") +
             mOutCoordSystem->GetOriginName() + wxT("\" is NULL"));
      }
      
      // get gravity constant if out coord system origin is CelestialBody
      if (mOrigin->IsOfType(Gmat::CELESTIAL_BODY))
         mGravConst = ((CelestialBody*)mOrigin)->GetGravitationalConstant();
   }
   
   #ifdef DEBUG_ORBITDATA_INIT
   MessageInterface::ShowMessage
      (wxT("OrbitData::InitializeRefObjects() exiting, mOrigin.Name=%s, mGravConst=%f, ")
       wxT("mOriginDep=%d\n"),  mOrigin->GetName().c_str(), mGravConst, mOriginDep);
   MessageInterface::ShowMessage
      (wxT("   mSpacecraft=<%p> '%s', mSolarSystem=<%p>, mOutCoordSystem=<%p>, mOrigin=<%p>\n"),
       mSpacecraft, mSpacecraft->GetName().c_str(),mSolarSystem, mOutCoordSystem, mOrigin);
   #endif
}


//------------------------------------------------------------------------------
// virtual bool IsValidObjectType(Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Checks reference object type.
 *
 * @return return true if object is valid object, false otherwise
 */
//------------------------------------------------------------------------------
bool OrbitData::IsValidObjectType(Gmat::ObjectType type)
{
   for (int i=0; i<OrbitDataObjectCount; i++)
   {
      if (GmatBase::GetObjectTypeString(type) == VALID_OBJECT_TYPE_LIST[i])
         return true;
   }
   
   return false;

}


