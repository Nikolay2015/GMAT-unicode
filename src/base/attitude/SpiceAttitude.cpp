//$Id:$
//------------------------------------------------------------------------------
//                               SpiceAttitude
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under
// FDSS Task order 28.
//
// Author: Wendy C. Shoan/GSFC
// Created: 2010.04.16
//
/**
 * Class implementation for the SpiceAttitude class.
 */
//------------------------------------------------------------------------------

#include <iostream>
#include <sstream>
#include <iomanip>
#include "Attitude.hpp"
#include "AttitudeException.hpp"
#include "SpiceAttitude.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_SPICE_ATTITUDE
//#define DEBUG_SPICE_ATTITUDE_GET_SET

//---------------------------------
// static data
//---------------------------------
const wxString
SpiceAttitude::PARAMETER_TEXT[SpiceAttitudeParamCount - AttitudeParamCount] =
{
   wxT("AttitudeKernelName"),
   wxT("SCClockKernelName"),
   wxT("FrameKernelName"),
};

const Gmat::ParameterType
SpiceAttitude::PARAMETER_TYPE[SpiceAttitudeParamCount - AttitudeParamCount] =
{
   Gmat::STRINGARRAY_TYPE,
   Gmat::STRINGARRAY_TYPE,
   Gmat::STRINGARRAY_TYPE,
};

const Integer SpiceAttitude::UNDEFINED_NAIF_ID           = -123456789;
const Integer SpiceAttitude::UNDEFINED_NAIF_ID_REF_FRAME = -123456789;

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  SpiceAttitude(const wxString &itsName)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the SpiceAttitude class
 * (default Constructor).
 */
//------------------------------------------------------------------------------
SpiceAttitude::SpiceAttitude(const wxString &attName) :
   Attitude(wxT("SpiceAttitude"),attName),
   scName          (wxT("")),
   naifId          (UNDEFINED_NAIF_ID),
   refFrameNaifId  (UNDEFINED_NAIF_ID_REF_FRAME)
{
   parameterCount = SpiceAttitudeParamCount;
   objectTypeNames.push_back(wxT("SpiceAttitude"));
   attitudeModelName = wxT("SpiceAttitude");
   #ifdef __USE_SPICE__
      reader = new SpiceAttitudeKernelReader();
   #endif
   ck.clear();
   sclk.clear();
   fk.clear();
 }

 //------------------------------------------------------------------------------
//  SpiceAttitude(const SpiceAttitude &att)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the SpiceAttitude class as a copy of the
 * specified SpiceAttitude class.
 * (copy constructor)
 *
 * @param <att> SpiceAttitude object to copy.
 */
//------------------------------------------------------------------------------
SpiceAttitude::SpiceAttitude(const SpiceAttitude& att) :
   Attitude(att),
   scName           (att.scName),
   naifId           (att.naifId),
   refFrameNaifId   (att.refFrameNaifId)
{
   ck.clear();
   sclk.clear();
   fk.clear();
   ck   = att.ck;
   sclk = att.sclk;
   fk   = att.fk;
   #ifdef __USE_SPICE__
      reader = (att.reader)->Clone();
   #endif

}

//------------------------------------------------------------------------------
//  SpiceAttitude& operator= (const SpiceAttitude& att)
//------------------------------------------------------------------------------
/**
 * Assignment operator for the SpiceAttitude class.
 *
 * @param att the SpiceAttitude object whose data to assign to "this"
 *            SpiceAttitude.
 *
 * @return "this" SpiceAttitude with data of input SpiceAttitude att.
 */
//------------------------------------------------------------------------------
SpiceAttitude& SpiceAttitude::operator=(const SpiceAttitude& att)
{
   if (&att == this)
      return *this;
   Attitude::operator=(att);
   ck.clear();
   sclk.clear();
   fk.clear();
   scName         = att.scName;
   naifId         = att.naifId;
   refFrameNaifId = att.refFrameNaifId;
   ck             = att.ck;
   sclk           = att.sclk;
   fk             = att.fk;
   #ifdef __USE_SPICE__
   if (reader) delete reader;
   reader         = (att.reader)->Clone();
   #endif
   return *this;
}

//------------------------------------------------------------------------------
//  ~SpiceAttitude()
//------------------------------------------------------------------------------
/**
 * Destructor for the SpiceAttitude class.
 */
//------------------------------------------------------------------------------
SpiceAttitude::~SpiceAttitude()
{
   #ifdef __USE_SPICE__
      if (reader) delete reader;
   #endif
   ck.clear();
   sclk.clear();
   fk.clear();
}


//---------------------------------------------------------------------------
//  bool Initialize()
//---------------------------------------------------------------------------
 /**
 * Initializes the SpiceAttitude.
 *
 * @return Success flag.
 *
  */
//---------------------------------------------------------------------------
bool SpiceAttitude::Initialize()
{
   #ifdef DEBUG_SPICE_ATTITUDE
      MessageInterface::ShowMessage(wxT("Entering SpiceAttitude::Initialize\n"));
   #endif
   bool isOK = Attitude::Initialize();
   if (!isOK) return false;

   if (scName == wxT(""))
   {
      wxString errmsg = wxT("Error - object name not set on SpiceAttitude object.\n");
      throw AttitudeException(errmsg);
   }
   if (ck.empty())
   {
      wxString errmsg = wxT("Error - no CK pointing kernel(s) set on SpiceAttitude for object ");
      errmsg += scName + wxT("\n");
      throw AttitudeException(errmsg);
   }
   if (sclk.empty())
    {
       wxString errmsg = wxT("Error - no SCLK clock kernel(s) set on SpiceAttitude for object ");
       errmsg += scName + wxT("\n");
       throw AttitudeException(errmsg);
    }
   if (fk.empty())
    {
      wxString warnmsg = wxT("Warning - no FK frame kernel(s) set on SpiceAttitude for object ");
      warnmsg += scName + wxT(".  A Frame Kernel may be necessary.");
      MessageInterface::PopupMessage(Gmat::WARNING_, warnmsg);
    }

   #ifdef __USE_SPICE__
      // Load the CK kernel(s)
      for (StringArray::iterator j = ck.begin(); j != ck.end(); ++j)
      {
         reader->LoadKernel(*j);
      }
      // Load the SCLK kernel(s)
      for (StringArray::iterator j = sclk.begin(); j != sclk.end(); ++j)
      {
         reader->LoadKernel(*j);
      }
      // Load the FK kernel(s), if any
      for (StringArray::iterator j = fk.begin(); j != fk.end(); ++j)
      {
         reader->LoadKernel(*j);
      }
   #endif
   if (naifId == UNDEFINED_NAIF_ID)
   {
      #ifdef __USE_SPICE__
         naifId = reader->GetNaifID(scName);
         #ifdef DEBUG_SPICE_ATTITUDE
            MessageInterface::ShowMessage(
                  wxT("Retrieved NAIF ID for %s -> %d\n"), scName.c_str(), naifId);
         #endif
         if (naifId == 0)
         {
            wxString errmsg = wxT("Error - NAIF ID not available for object \n");
            errmsg += scName + wxT("\n");
            throw AttitudeException(errmsg);
         }
      #else
         wxString errmsg = wxT("Error - NAIF ID not set on SpiceAttitude object.\n");
         throw AttitudeException(errmsg);
      #endif
   }

   if (refFrameNaifId == UNDEFINED_NAIF_ID_REF_FRAME)
   {
      wxString errmsg = wxT("Error - NAIF ID for object reference frame not set on SpiceAttitude object.\n");
      throw AttitudeException(errmsg);
   }


   return true;
}

//------------------------------------------------------------------------------
//  GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the SpiceAttitude.
 *
 * @return clone of the SpiceAttitude.
 *
 */
//------------------------------------------------------------------------------
GmatBase* SpiceAttitude::Clone(void) const
{
   return (new SpiceAttitude(*this));
}

//------------------------------------------------------------------------------
//  void SetObjectID(const wxString &objName, Integer objNaifId)
//------------------------------------------------------------------------------
/**
 * This method sets the object ID information (name, NAIF ID).
 *
 * @param objName   name of the object for which to get the attitude
 * @param objNaifId NAIF ID of the object
 *
 */
//------------------------------------------------------------------------------
void SpiceAttitude::SetObjectID(const wxString &objName, Integer objNaifId, Integer objRefFrameNaifId)
{
   scName         = objName;
   naifId         = objNaifId;
   refFrameNaifId = objRefFrameNaifId;
}

//---------------------------------------------------------------------------
//  const Rvector&   GetQuaternion(Real atTime)
//---------------------------------------------------------------------------
 /**
 * Returns the attitude at time atTime as a quaternion.
 *
 * @param atTime  time at which to compute the attitude.
 *
 * @return the quaternion representation of the attitude, computed at
 *         time atTime.
 */
//---------------------------------------------------------------------------
const Rvector&   SpiceAttitude::GetQuaternion(Real atTime)
{
   ComputeCosineMatrixAndAngularVelocity(atTime);
   attitudeTime     = atTime;
   quaternion       = Attitude::ToQuaternion(cosMat);
   return quaternion;
}

//---------------------------------------------------------------------------
//  const Rvector3&   GetEulerAngles(Real tTime)
//---------------------------------------------------------------------------
 /**
 * Returns the attitude at time atTime as an array of Euler angles.  This
 * method assumes the previously set Euler sequence.
 *
 * @param atTime  time at which to compute the attitude.
 *
 * @return the euler angle representation of the attitude, computed at
 *         time atTime (radians).
 */
//---------------------------------------------------------------------------
const Rvector3&  SpiceAttitude::GetEulerAngles(Real atTime)
{
   ComputeCosineMatrixAndAngularVelocity(atTime);
   attitudeTime   = atTime;
   eulerAngles    = Attitude::ToEulerAngles(cosMat,
                              (Integer) eulerSequenceArray.at(0),
                              (Integer) eulerSequenceArray.at(1),
                              (Integer) eulerSequenceArray.at(2));
   return eulerAngles;
}

//---------------------------------------------------------------------------
//  const Rvector3&   GetEulerAngles(Real atTime,
//                                   Integer seq1, Integer seq2,
//                                   Integer seq3)
//---------------------------------------------------------------------------
 /**
 * Returns the attitude at time atTime as an array of Euler angles.  This
 * method uses the Euler sequence passed in here.
 *
 * @param atTime  time at which to compute the attitude.
 * @param seq1    euler Sequence 1
 * @param seq2    euler sequence 2
 * @param seq3    euler sequence 3
 *
 * @return the euler angle representation of the attitude, computed at
 *         time atTime (radians).
 * @todo - figure out what to do if the time is the same, but the
 *         Euler sequence is different that the last time it was computed.
 */
//---------------------------------------------------------------------------
const Rvector3&  SpiceAttitude::GetEulerAngles(Real atTime,  Integer seq1,
                                          Integer seq2, Integer seq3)
{
   ComputeCosineMatrixAndAngularVelocity(atTime);
   attitudeTime   = atTime;
   eulerAngles    = Attitude::ToEulerAngles(cosMat, seq1, seq2, seq3);
   return eulerAngles;
}

//---------------------------------------------------------------------------
//  const Rmatrix33&   GetCosineMatrix(Real atTime)
//---------------------------------------------------------------------------
 /**
 * Returns the attitude at time atTime as direction cosine matrix.
 *
 * @param atTime  time at which to compute the attitude.
 *
 * @return the direction cosine matrix representation of the attitude,
 *         computed at time atTime.
 */
//---------------------------------------------------------------------------
const Rmatrix33& SpiceAttitude::GetCosineMatrix(Real atTime)
{
   ComputeCosineMatrixAndAngularVelocity(atTime);
   attitudeTime = atTime;

   return cosMat;
}

//---------------------------------------------------------------------------
//  const Rvector3&   GetAngularVelocity(Real atTime)
//---------------------------------------------------------------------------
 /**
 * Returns the attitude rates at time atTime as an angular velocity.
 *
 * @param atTime  time at which to compute the attitude.
 *
 * @return the angular velocity representation of the attitude rates,
 *         computed at time atTime (radians/second).
 */
//---------------------------------------------------------------------------
const Rvector3& SpiceAttitude::GetAngularVelocity(Real atTime)
{
   ComputeCosineMatrixAndAngularVelocity(atTime);
   attitudeTime = atTime;
   return angVel;
}

//---------------------------------------------------------------------------
//  const Rvector3&   GetEulerAngleRates(Real atTime)
//---------------------------------------------------------------------------
 /**
 * Returns the attitude rates at time atTime as an array of euler
 * angle rates.
 *
 * @param atTime  time at which to compute the attitude.
 *
 * @return the euler angle rates representation of the attitude rates,
 *         computed at time atTime (radians/second).
 */
//---------------------------------------------------------------------------
const Rvector3& SpiceAttitude::GetEulerAngleRates(Real atTime)
{
   ComputeCosineMatrixAndAngularVelocity(atTime);
   attitudeTime        = atTime;
   eulerAngles         = GetEulerAngles(atTime);
   eulerAngleRates     = Attitude::ToEulerAngleRates(angVel,
                         eulerAngles,
                         (Integer) eulerSequenceArray.at(0),
                         (Integer) eulerSequenceArray.at(1),
                         (Integer) eulerSequenceArray.at(2));
   return eulerAngleRates;
}


//------------------------------------------------------------------------------
//  wxString  GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter text, given the input parameter ID.
 *
 * @param <id> Id for the requested parameter text.
 *
 * @return parameter text for the requested parameter.
 *
 */
//------------------------------------------------------------------------------
wxString SpiceAttitude::GetParameterText(const Integer id) const
{
   if (id >= AttitudeParamCount && id < SpiceAttitudeParamCount)
      return PARAMETER_TEXT[id - AttitudeParamCount];
   return Attitude::GetParameterText(id);
}

//------------------------------------------------------------------------------
//  Integer  GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter ID, given the input parameter string.
 *
 * @param <str> string for the requested parameter.
 *
 * @return ID for the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Integer SpiceAttitude::GetParameterID(const wxString &str) const
{
   for (Integer i = AttitudeParamCount; i < SpiceAttitudeParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - AttitudeParamCount])
         return i;
   }

   return Attitude::GetParameterID(str);
}

//------------------------------------------------------------------------------
//  Gmat::ParameterType  GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type, given the input parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return parameter type of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
Gmat::ParameterType SpiceAttitude::GetParameterType(const Integer id) const
{
   if (id >= AttitudeParamCount && id < SpiceAttitudeParamCount)
      return PARAMETER_TYPE[id - AttitudeParamCount];

   return Attitude::GetParameterType(id);
}

//------------------------------------------------------------------------------
//  wxString  GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * This method returns the parameter type string, given the input parameter ID.
 *
 * @param <id> ID for the requested parameter.
 *
 * @return parameter type string of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
wxString SpiceAttitude::GetParameterTypeString(const Integer id) const
{
   return Attitude::PARAM_TYPE_STRING[GetParameterType(id)];
}


//------------------------------------------------------------------------------
//  wxString  GetStringParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
/**
 * This method returns the string parameter value, given the input
 * parameter ID.
 *
 * @param id    ID for the requested parameter.
 * @param index index into the array of strings
 *
 * @return  string value of the requested parameter.
 *
 */
//------------------------------------------------------------------------------
wxString  SpiceAttitude::GetStringParameter(const Integer id,
                                        const Integer index) const
{
   if (id == ATTITUDE_KERNEL_NAME)
   {
      if ((index < 0) || (index >= (Integer) ck.size()))
      {
         wxString errmsg = wxT("Error attempting to retrieve CK kernel name for object ");
         errmsg += scName + wxT(" - index out-of-bounds.\n");
         throw AttitudeException(errmsg);
      }
      return ck.at(index);
   }
   if (id == SC_CLOCK_KERNEL_NAME)
   {
      if ((index < 0) || (index >= (Integer) sclk.size()))
      {
         wxString errmsg = wxT("Error attempting to retrieve SCLK kernel name for object ");
         errmsg += scName + wxT(" - index out-of-bounds.\n");
         throw AttitudeException(errmsg);
      }
      return sclk.at(index);
   }
   if (id == FRAME_KERNEL_NAME)
   {
      if ((index < 0) || (index >= (Integer) fk.size()))
      {
         wxString errmsg = wxT("Error attempting to retrieve FK kernel name for object ");
         errmsg += scName + wxT(" - index out-of-bounds.\n");
         throw AttitudeException(errmsg);
      }
      return fk.at(index);
   }
   return Attitude::GetStringParameter(id, index);
}

//------------------------------------------------------------------------------
//  bool  SetStringParameter(const Integer id, const wxString value,
//                           const Integer index)
//------------------------------------------------------------------------------
/**
 * This method sets the string parameter value, given the input
 * parameter ID.
 *
 * @param id ID for the requested parameter.
 * @param value string value for the requested parameter.
 * @param index index into the string array
 *
 * @exception <SolarSystemException> thrown if value is out of range
 *
 * @return  success flag.
 *
 */
//------------------------------------------------------------------------------
bool SpiceAttitude::SetStringParameter(const Integer id,
                                        const wxString &value,
                                        const Integer index)
{
   #ifdef DEBUG_SPICE_ATTITUDE_GET_SET
      MessageInterface::ShowMessage(
            wxT("Entering SetStringParameter with id = %d, value = \"%s\", index = %d\n"),
            id, value.c_str(), index);
   #endif
   if (id == ATTITUDE_KERNEL_NAME)
   {
      if ((index < 0) || (index > (Integer) ck.size()))
      {
         wxString errmsg = wxT("Error attempting to set CK kernel name for object ");
         errmsg += scName +  wxT(" - index out-of-bounds.\n");
         throw AttitudeException(errmsg);
      }
      if (index == (Integer) ck.size())  ck.push_back(value);
      else                               ck.at(index) = value;
      return true;
   }
   if (id == SC_CLOCK_KERNEL_NAME)
   {
      if ((index < 0) || (index > (Integer) sclk.size()))
      {
         wxString errmsg = wxT("Error attempting to set SCLK kernel name for object ");
         errmsg += scName + wxT(" - index out-of-bounds.\n");
         throw AttitudeException(errmsg);
      }
      if (index == (Integer) sclk.size())  sclk.push_back(value);
      else                                 sclk.at(index) = value;
      return true;
   }
   if (id == FRAME_KERNEL_NAME)
   {
      if ((index < 0) || (index > (Integer) fk.size()))
      {
         wxString errmsg = wxT("Error attempting to set FK kernel name for object ");
         errmsg += scName +  wxT(" - index out-of-bounds.\n");
         throw AttitudeException(errmsg);
      }
      if (index == (Integer) fk.size())  fk.push_back(value);
      else                               fk.at(index) = value;
      return true;
   }
   return Attitude::SetStringParameter(id, value, index);
}

bool SpiceAttitude::SetStringParameter(const wxString label,
                                       const wxString &value,
                                       const Integer index)
{
   return SetStringParameter(GetParameterID(label), value, index);
}

//---------------------------------------------------------------------------
//  const StringArray& GetStringArrayParameter(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Gets the requested string array.
 *
 * @param id The integer ID for the parameter.
 *
 * @return The requested StringArray; throws an exception if the parameter is
 * not a StringArray.
 */
//---------------------------------------------------------------------------
const StringArray& SpiceAttitude::GetStringArrayParameter(const Integer id) const
{
   if (id == ATTITUDE_KERNEL_NAME)
      return ck;
   else if (id == SC_CLOCK_KERNEL_NAME)
      return sclk;
   else if (id == FRAME_KERNEL_NAME)
      return fk;

   return Attitude::GetStringArrayParameter(id);
}



//---------------------------------
//  protected methods
//---------------------------------

//------------------------------------------------------------------------------
//  virtual void ComputeCosineMatrixAndAngularVelocity(Real atTime)
//------------------------------------------------------------------------------
/**
 * This method computes the current CosineMatrix at the input time atTime.
 *
 * @param atTime the A1Mjd time at which to compute the attitude.
 *
 */
//------------------------------------------------------------------------------
void SpiceAttitude::ComputeCosineMatrixAndAngularVelocity(Real atTime)
{
   #ifdef __USE_SPICE__
      reader->GetTargetOrientation(scName, naifId, refFrameNaifId, atTime, cosMat, angVel);
   #else
      wxString errmsg = wxT("Error - attempting to use SpiceAttitude when SPICE is not included in the GMAT build.\n");
      throw AttitudeException(errmsg);
   #endif

}


//---------------------------------
//  private methods
//---------------------------------
// none

