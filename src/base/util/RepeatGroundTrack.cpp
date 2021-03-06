//
//------------------------------------------------------------------------------
//                                  RepeatGroundTrack
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
// Author:  Evelyn Hull
// Created: 2011/07/28
//
/**
 * Implements the Repeat Ground Track base class.
 */
//------------------------------------------------------------------------------

#include "RepeatGroundTrack.hpp"
#include "GmatDefaults.hpp" 
#include "math.h"
#include "GmatConstants.hpp"
#include "RealUtilities.hpp"

//------------------------------------------------------------------------------
// RepeatGroundTrack()
//------------------------------------------------------------------------------
// A constructor.
//------------------------------------------------------------------------------
RepeatGroundTrack::RepeatGroundTrack()
{
}

//------------------------------------------------------------------------------
// ~RepeatGroundTrack()
//------------------------------------------------------------------------------
RepeatGroundTrack::~RepeatGroundTrack()
{
}

Real RepeatGroundTrack::GetSMA()
{
   return elements.SMA;
}

Real RepeatGroundTrack::GetALT()
{
   return (elements.SMA-GmatSolarSystemDefaults::PLANET_EQUATORIAL_RADIUS[2]);
}

Real RepeatGroundTrack::GetECC()
{
   return elements.ECC;
}

Real RepeatGroundTrack::GetINC()
{
   return elements.INC;
}

Real RepeatGroundTrack::GetROA()
{
	return elements.SMA*(1+elements.ECC);
}

Real RepeatGroundTrack::GetROP()
{
   return elements.SMA*(1-elements.ECC);
}

Real RepeatGroundTrack::GetP()
{
   return elements.SMA*(1-pow(elements.ECC,2));
}


bool RepeatGroundTrack::IsError()
{
   return isError;
}

wxString RepeatGroundTrack::GetError()
{
   return errormsg;
}

void RepeatGroundTrack::CalculateRepeatGroundTrack(bool eccVal, Real ECC, 
      bool incVal, Real INC, bool rtrVal, Real revsToRepeat, 
      bool dtrVal, Real daysToRepeat, bool rpdVal, Real revsPerDay)
{
   Real SMA, diff, tol, newSMA, p, raanDot, aopDot, maDot, n, revsPerDay_low;
   Real revsToRepeat_low, daysToRepeat_high, daysToRepeat_low;
   int count = 0;
   Real radiusEarth = GmatSolarSystemDefaults::PLANET_EQUATORIAL_RADIUS[2];
   Real muEarth = GmatSolarSystemDefaults::PLANET_MU[2];
   Real J2 = 0.0010826267;
   Real omegaEarth  = 0.000072921158533;
   isError = false;

   if (!eccVal)
   {
	  errormsg = wxT("ECC must be selected");
	  isError = true;
      return;
   }
   else if (!incVal)
   {
	  errormsg = wxT("INC must be selected");
	  isError = true;
      return;
   }

   if ((rtrVal && (revsToRepeat <= 0)) || (dtrVal && (daysToRepeat <= 0)) 
      || (rpdVal && (revsPerDay <= 0)))
   {
      SMA = -1;
      errormsg = 
	     wxT("Please do not give Repetition inputs that are less than or equal to zero");
	  isError = true;
      return;
   }
    

   if (rpdVal && rtrVal)
      daysToRepeat = revsToRepeat/revsPerDay;
   else if (rpdVal && dtrVal)
      revsToRepeat = revsPerDay*daysToRepeat;
   else if (rtrVal && dtrVal)
      revsPerDay = revsToRepeat/daysToRepeat;
   else
   {
      errormsg = 
	     wxT("Please select two of days to repeat, revs to repeat, and revs per day");
      isError = true;
      return;
   }


   if ((ECC<0.0) || (ECC>1.0))
   {
      errormsg = 
	     wxT("Eccentricity value out of range, please choose e greater than or equal to 0 and less than 1");
	  isError = true;
	  return;
   }
   else if ((INC<0.0) || (INC>180.0))
   {
      errormsg = 
	     wxT("Inclination value out of range, please choose i greater than or equal to 0 and less than 180");
	  isError = true;
	  return;
   }
   else if (revsToRepeat<1)
   {
      errormsg = 
	     wxT("Revolutions to Repeat value out of range, please choose revolutions to repeat greater than or equal to 1");
	  isError = true;
	  return;
   }     
   else if (daysToRepeat<1)
   {
      errormsg = 
	     wxT("Days to Repeat value out of range, please choose days to repeat greater than or equal to 1");
	  isError = true;
	  return;
   }
   else if (revsPerDay<0)
   {
      errormsg = 
	     wxT("Revs Per Day value out of range, please choose revolutions per day greater than or equal to 0");
	  isError = true;
	  return;
   }

   //  Loop Initialization
   n      = revsPerDay*omegaEarth;
   tol    = 1e-8;
   count  = 0;
   diff   = 1;
   SMA = pow((muEarth*pow((1/n),2)),(1/3.0));
   newSMA = SMA;

   //  Perform the Iteration:  Vallado, Algorithm 70 
   while (diff > tol && count <= 50)
   {
      count = count + 1;
      SMA = newSMA;
      p = SMA*(1 - pow(ECC,2));
      raanDot = -3*n*J2/2*pow((radiusEarth/p),2)*cos(INC);
      aopDot = 3*n*J2/4*pow((radiusEarth/p),2)*(4 - 5*pow(sin(INC),2));
      maDot = 
	     3*n*J2/4*pow((radiusEarth/p),2)*
		 sqrt(1 - pow(ECC,2))*(2 - 3*pow(sin(INC),2)); 
      n = revsPerDay*(omegaEarth - raanDot) - (maDot + aopDot);
      newSMA = pow((muEarth*pow((1/n),2)),(1/3.0));
      diff = GmatMathUtil::Abs(SMA - newSMA);
   }

   if (SMA < radiusEarth)
   {
      revsPerDay_low = sqrt(muEarth)/(pow(radiusEarth,(3/2.0))*omegaEarth);
      if (rpdVal && rtrVal)
	  {
         revsToRepeat_low = revsPerDay_low*daysToRepeat;
         errormsg = 
		    wxT("No repeat ground track orbit exists with those parameters, try revs per day less than ") 
			+ GmatStringUtil::ToString(revsPerDay_low) 
			+ wxT(", or revs to repeat less than ") 
			+ GmatStringUtil::ToString(revsToRepeat_low);
	  }
      else if (rpdVal && dtrVal)
	  {
         daysToRepeat_high = revsToRepeat/revsPerDay_low;
         errormsg = 
		    wxT("No repeat ground track orbit exists with those parameters, try revs per day less than ") 
			+ GmatStringUtil::ToString(revsPerDay_low) 
			+ wxT(", or days to repeat greater than ") 
			+ GmatStringUtil::ToString(daysToRepeat_high);
	  }
      else if (rtrVal && dtrVal)
	  {
         revsToRepeat_low = revsPerDay_low*daysToRepeat;
         daysToRepeat_low = revsToRepeat_low/revsPerDay;
         errormsg = 
		    wxT("No repeat ground track orbit exists with those parameters, try revs to repeat less than ") 
			+ GmatStringUtil::ToString(revsToRepeat_low) 
			+ wxT(", or days to repeat greater than ") 
			+ GmatStringUtil::ToString(daysToRepeat_low);
	  }
   }

   elements.SMA = newSMA;
   elements.ECC = ECC;
   elements.INC = INC;
}
