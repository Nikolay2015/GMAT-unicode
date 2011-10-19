//$Id: TimeTypes.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                TimeTypes
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
// Author: Linda Jun, NASA/GSFC
// Created: 2006/05/05
//
/**
 * Provides declarations for date & time types.
 */
//------------------------------------------------------------------------------

#include "TimeTypes.hpp"
#include "StringUtil.hpp"
#include "GmatConstants.hpp"
#include "UtilityException.hpp"
#include <ctime>                   // for time()


//------------------------------------------------------------------------------
// bool IsValidMonthName(const wxString &str)
//------------------------------------------------------------------------------
/*
 * Checks if input string is valid month short name. Input month name should
 *   be month name abbreviation.  For example, wxT("Jan"), wxT("Feb"), etc.
 *
 * @param  str  input month string
 *
 * @return return true if input string is valid month name
 */
//------------------------------------------------------------------------------ 
bool GmatTimeUtil::IsValidMonthName(const wxString &str)
{
   for (int i=0; i<12; i++)
   {
      if (str == GmatTimeConstants::MONTH_NAME_TEXT[i])
         return true;
   }
   
   return false;
}


//------------------------------------------------------------------------------
// wxString GetMonthName(Integer month)
//------------------------------------------------------------------------------
/*
 * Return corresponding Month Name for integer month.
 *
 * @param  month  input month
 *
 * @return corresponding Month Name for integer month.
 * 
 * @exception <TimeException> thrown if input month is out of range
 */
//------------------------------------------------------------------------------
wxString GmatTimeUtil::GetMonthName(Integer month)
{
   if (month < 1 || month > 12) 
   {
      throw TimeException
         (wxT("Cannot get Month Name for ") + GmatStringUtil::ToString(month));
   }
   
   return GmatTimeConstants::MONTH_NAME_TEXT[month-1];
}


//------------------------------------------------------------------------------
// Integer GetMonth(const wxString &monthName)
//------------------------------------------------------------------------------
/*
 * Return corresponding month for input Month Name. Input month name should
 *   be month name abbreviation.  For example, wxT("Jan"), wxT("Feb"), etc.
 *
 * @param  monthName  input month name
 *
 * @return corresponding Integer month for Month Name.
 *         -1, if name not found
 * 
 * @exception <TimeException> thrown if input month name is out of range
 */
//------------------------------------------------------------------------------
Integer GmatTimeUtil::GetMonth(const wxString &monthName)
{
   for (int i=0; i<12; i++)
   {
      if (monthName == GmatTimeConstants::MONTH_NAME_TEXT[i])
         return i + 1;
   }
   
   return -1;
}


//------------------------------------------------------------------------------
// wxString FormatCurrentTime(Integer format = 1)
//------------------------------------------------------------------------------
/*
 * Returns the current time in specified format.
 *
 * @param  format  Used in formating current time (1)
 *                 1 = wxT("Wed Apr 16 12:30:22 2008")
 *                 2 = wxT("2008-04-16T12:30:22")
 *                 3 = wxT("2008-04-16 12:30:22")
 *
 */
//------------------------------------------------------------------------------
wxString GmatTimeUtil::FormatCurrentTime(Integer format)
{
   time_t currTime = time(NULL);

   if (format == 1)
   {
      wxString currTimeStr = wxString::FromAscii(ctime(&currTime));
      return currTimeStr;
   }
   else
   {
      tm *loctime = localtime(&currTime);
      char timeBuf[20];
      if (format == 2)
         strftime(timeBuf, 20, "%Y-%m-%dT%I:%M:%S", loctime);
      else
         strftime(timeBuf, 20, "%Y-%m-%d %I:%M:%S", loctime);
      return wxString::FromAscii(timeBuf);
   }
}

//------------------------------------------------------------------------------
// wxString GetGregorianFormat()
//------------------------------------------------------------------------------
/**
 * Return gregorian time format
 */
//------------------------------------------------------------------------------
wxString GmatTimeUtil::GetGregorianFormat()
{
   return wxT("DD MMM YYYY HH:MM:SS.mmm");
}


