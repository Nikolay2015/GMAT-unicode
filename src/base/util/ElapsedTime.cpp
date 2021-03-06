//$Id: ElapsedTime.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              ElapsedTime
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
// Created: 2003/09/22
//
/**
 * Defines elapsed time operations. Internal elapsed time is in seconds.
 */
//------------------------------------------------------------------------------
#include <sstream>
#include "ElapsedTime.hpp"
#include "RealUtilities.hpp"

using namespace GmatTimeConstants;  // for SECS_PER_DAY, SECS_PER_HOUR, etc.
using namespace GmatMathUtil;  // for Rem(), IsEqual()
using namespace GmatMathConstants;  // for Rem(), IsEqual()

//---------------------------------
// static variables
//---------------------------------

const wxString ElapsedTime::DATA_DESCRIPTIONS[NUM_DATA] =
{
   wxT("Elapsed Time in Seconds")
};

//---------------------------------
// public
//---------------------------------

//------------------------------------------------------------------------------
// ElapsedTime::ElapsedTime(const Real &secs, const Real tol)
//------------------------------------------------------------------------------
ElapsedTime::ElapsedTime(const Real &secs, const Real tol)
   : seconds(secs), tolerance(tol)
{
}

//------------------------------------------------------------------------------
// ElapsedTime::ElapsedTime(const ElapsedTime &elapsedTime, const Real tol)
//------------------------------------------------------------------------------
ElapsedTime::ElapsedTime(const ElapsedTime &elapsedTime, const Real tol)
   : seconds(elapsedTime.seconds), tolerance(tol)
{
}

//------------------------------------------------------------------------------
// ElapsedTime& ElapsedTime::operator= (const ElapsedTime &right)
//------------------------------------------------------------------------------
ElapsedTime& ElapsedTime::operator= (const ElapsedTime &right)
{
   seconds = right.seconds;
   return *this;
}

//------------------------------------------------------------------------------
// ElapsedTime::~ElapsedTime()
//------------------------------------------------------------------------------
ElapsedTime::~ElapsedTime()
{
}

//------------------------------------------------------------------------------
// ElapsedTime ElapsedTime::operator+ (const Real &right) const
//------------------------------------------------------------------------------
ElapsedTime ElapsedTime::operator+ (const Real &right) const
{
   return ElapsedTime(seconds + right);
}

//------------------------------------------------------------------------------
// ElapsedTime ElapsedTime::operator- (const Real &right) const
//------------------------------------------------------------------------------
ElapsedTime ElapsedTime::operator- (const Real &right) const
{
   return ElapsedTime(seconds - right);
}

//------------------------------------------------------------------------------
// const ElapsedTime& ElapsedTime::operator+= (const Real &right)
//------------------------------------------------------------------------------
const ElapsedTime& ElapsedTime::operator+= (const Real &right)
{
   seconds += right;
   return *this;
}

//------------------------------------------------------------------------------
// const ElapsedTime& ElapsedTime::operator-= (const Real &right)
//------------------------------------------------------------------------------
const ElapsedTime& ElapsedTime::operator-= (const Real &right)
{
   seconds -= right;
   return *this;
}

//------------------------------------------------------------------------------
// bool ElapsedTime::operator< (const ElapsedTime &right) const
//------------------------------------------------------------------------------
bool ElapsedTime::operator< (const ElapsedTime &right) const
{
   return seconds < right.seconds;
}

//------------------------------------------------------------------------------
// bool ElapsedTime::operator> (const ElapsedTime &right) const
//------------------------------------------------------------------------------
bool ElapsedTime::operator> (const ElapsedTime &right) const
{
   return seconds > right.seconds;
}

//------------------------------------------------------------------------------
// bool ElapsedTime::operator== (const ElapsedTime &right) const
//------------------------------------------------------------------------------
bool ElapsedTime::operator== (const ElapsedTime &right) const
{
   return IsEqual(seconds, right.seconds, tolerance);
}

//------------------------------------------------------------------------------
// bool ElapsedTime::operator!= (const ElapsedTime &right) const
//------------------------------------------------------------------------------
bool ElapsedTime::operator!= (const ElapsedTime &right) const
{
   return !(IsEqual(seconds, right.seconds, tolerance));
}

//------------------------------------------------------------------------------
// bool ElapsedTime::operator>= (const ElapsedTime &right) const
//------------------------------------------------------------------------------
bool ElapsedTime::operator>= (const ElapsedTime &right) const
{
   return seconds >= right.seconds;
}

//------------------------------------------------------------------------------
// bool ElapsedTime::operator<= (const ElapsedTime &right) const
//------------------------------------------------------------------------------
bool ElapsedTime::operator<= (const ElapsedTime &right) const
{
   return seconds <= right.seconds;
}

//------------------------------------------------------------------------------
// Real ElapsedTime::Get() const
//------------------------------------------------------------------------------
Real ElapsedTime::Get() const
{
   return seconds;
}

//------------------------------------------------------------------------------
// void ElapsedTime::Set(Real secs)
//------------------------------------------------------------------------------
void ElapsedTime::Set(Real secs)
{
   seconds = secs;
}

//------------------------------------------------------------------------------
// GmatTimeUtil::ElapsedDate ElapsedTime::ToElapsedDate() const
//------------------------------------------------------------------------------
GmatTimeUtil::ElapsedDate ElapsedTime::ToElapsedDate() const
{
   Integer sign;
   Real secs = seconds;
   
   // Compute sign
   if (secs < 0)
   {
      sign = (SIGN)(-1);
      secs =  - secs;
   }
   else
   {
      sign = (SIGN)(1);
   }
   
   //Compute elapsed days, hours, minutes, seconds 
   Integer days = (Integer)(secs / SECS_PER_DAY); 
   Integer hours = (Integer)(Rem(secs, SECS_PER_DAY) / SECS_PER_HOUR); 
   Integer minutes = (Integer)(Rem(Rem(secs, SECS_PER_DAY), 
                                   SECS_PER_HOUR) / SECS_PER_MINUTE); 
   secs = Rem(Rem(Rem(secs, SECS_PER_DAY), SECS_PER_HOUR), SECS_PER_MINUTE); 
   
   return GmatTimeUtil::ElapsedDate(days, hours, minutes, secs);
}

//------------------------------------------------------------------------------
// Integer GetNumData() const
//------------------------------------------------------------------------------
Integer ElapsedTime::GetNumData() const
{
   return NUM_DATA;
}

//------------------------------------------------------------------------------
// const wxString* ElapsedTime::GetDataDescriptions() const
//------------------------------------------------------------------------------
const wxString* ElapsedTime::GetDataDescriptions() const
{
   return DATA_DESCRIPTIONS;
}

//------------------------------------------------------------------------------
// wxString* ElapsedTime::ToValueStrings()
//------------------------------------------------------------------------------
wxString* ElapsedTime::ToValueStrings()
{
   wxString ss(wxT(""));

   ss << seconds;
   stringValues[0] = ss;
   return stringValues;
}

