//$Id: SpiceInterface.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                              SpiceInterface
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
// Author: Wendy C. Shoan
// Created: 2009.02.24 / 2010.04.20
//
/**
 * Implementation of the SpiceInterface, which loads/unloads SPICE data (kernel)
 * files.  This class calls the JPL-supplied CSPICE routines.
 */
//------------------------------------------------------------------------------


#include "SpiceInterface.hpp"
#include "A1Mjd.hpp"
#include "StringUtil.hpp"
#include "MessageInterface.hpp"
#include "TimeTypes.hpp"
#include "TimeSystemConverter.hpp"
#include "UtilityException.hpp"
#include "FileManager.hpp"

//#define DEBUG_SPK_LOADING
//#define DEBUG_SPK_READING
//#define DEBUG_SPK_GETSTATE
//#define DEBUG_SPK_PLANETS

//---------------------------------
// static data
//---------------------------------
const Integer SpiceInterface::DEFAULT_NAIF_ID           = -123456789;
const Integer SpiceInterface::DEFAULT_NAIF_ID_REF_FRAME = -123456789;

const wxString
SpiceInterface::VALID_ABERRATION_FLAGS[9] =
{
   wxT("NONE"),    // Apply no correction
   // the following 4 options apply to the 'reception' case, in which photons
   // depart from the target's location at time et-lt and arrive at the 
   // observer's location at et (input time)
   wxT("LT"),      // Correct for one-way light time
   wxT("LT+S"),    // Correct for one-way light time and stellar aberration
   wxT("CN"),      // Converged Newtonian light time correction
   wxT("CN+S"),    // Converged Newtonian light time and stellar aberration correction
   // the following 4 options apply to the 'transmission' case, in which photons
   // depart from the observer's location at time et and arrive at the 
   // target's location at et+lt (input time)
   wxT("XLT"),     // Correct for one-way light time
   wxT("XLT+S"),   // Correct for one-way light time and stellar aberration
   wxT("XCN"),     // Converged Newtonian light time correction
   wxT("XCN+S"),   // Converged Newtonian light time and stellar aberration correction
};

const Integer SpiceInterface::NUM_VALID_FRAMES = 1; // for now, only wxT("J2000")

const wxString
SpiceInterface::VALID_FRAMES[12] =
{
   wxT("J2000"),   // default frame
   wxT("NONE"),   // TBD
   wxT("NONE"),   // TBD
   wxT("NONE"),   // TBD
   wxT("NONE"),   // TBD
   wxT("NONE"),   // TBD
   wxT("NONE"),   // TBD
   wxT("NONE"),   // TBD
   wxT("NONE"),   // TBD
   wxT("NONE"),   // TBD
   wxT("NONE"),   // TBD
   wxT("NONE"),   // TBD
};

const Integer SpiceInterface::MAX_SHORT_MESSAGE         = 320;
const Integer SpiceInterface::MAX_EXPLAIN_MESSAGE       = 320;
const Integer SpiceInterface::MAX_LONG_MESSAGE          = MAX_LONG_MESSAGE_VALUE;
const Integer SpiceInterface::MAX_CHAR_COMMENT          = 4000;

/// array of files (kernels) currently loaded
StringArray    SpiceInterface::loadedKernels;
/// counter of number of instances created
Integer        SpiceInterface::numInstances = 0;
/// the name (full path) of the leap second kernel to use
wxString    SpiceInterface::lsKernel = wxT("");


//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
//  SpiceInterface()
//------------------------------------------------------------------------------
/**
 * This method creates an object of the SpiceInterface class
 * (default constructor).
 *
 */
//------------------------------------------------------------------------------
SpiceInterface::SpiceInterface() :
   kernelNameSPICE         (NULL)
{
   InitializeInterface();
   numInstances++;
}

//------------------------------------------------------------------------------
//  SpiceInterface(const SpiceInterface &copy)
//------------------------------------------------------------------------------
/**
 * This method creates an object of the SpiceInterface class, by copying
 * the input object.
 * (copy constructor).
 *
* @param <copy> SpiceInterface object to copy.
  */
//------------------------------------------------------------------------------
SpiceInterface::SpiceInterface(const SpiceInterface &copy) :
   kernelNameSPICE         (NULL)
{
   numInstances++;
   // the kernels are all loaded into that one kernel pool, but we need to copy the names
   loadedKernels = copy.loadedKernels;
   lsKernel      = copy.lsKernel;
}

//------------------------------------------------------------------------------
//  SpiceInterface& operator=(const SpiceInterface &copy)
//------------------------------------------------------------------------------
/**
 * This method copies the data from the input object to the object.
 *
 * @param <copy> the SpiceInterface object whose data to assign to wxT("this")
 *                 SpiceInterface.
 *
 * @return wxT("this") SpiceInterface with data of input SpiceInterface reader.
 */
//------------------------------------------------------------------------------
SpiceInterface& SpiceInterface::operator=(const SpiceInterface &copy)
{
   if (&copy == this)
      return *this;

   kernelNameSPICE          = NULL;
   // the kernels are all loaded into that one kernel pool, but we need to copy the names
   loadedKernels.clear();
   loadedKernels = copy.loadedKernels;
   lsKernel      = copy.lsKernel;

   // don't modify numInstances - we are not creating a new instance here
   return *this;
}


//------------------------------------------------------------------------------
// ~SpiceInterface()
//------------------------------------------------------------------------------
/**
 * This method is the destructor for the SpiceInterface.
 *
 */
//------------------------------------------------------------------------------
SpiceInterface::~SpiceInterface()
{
   numInstances--;
   if (numInstances <= 0) UnloadAllKernels();
}

//------------------------------------------------------------------------------
//  bool LoadKernel(const wxString &fileName)
//------------------------------------------------------------------------------
/**
 * This method loads the input file into the SPICE kernel pool.
 *
 * @param <fileName>  full path of the file (kernel) to load.
 *
 * @return success flag.
 *
 */
//------------------------------------------------------------------------------
bool SpiceInterface::LoadKernel(const wxString &fileName)
{
   #ifdef DEBUG_SPK_LOADING
         MessageInterface::ShowMessage(wxT("SpiceInterface: Attempting to load kernel %s <---------\n"),
               fileName.c_str());
   #endif
   for (StringArray::iterator jj = loadedKernels.begin();
        jj != loadedKernels.end(); ++jj)
      if ((*jj) == fileName)
      {
//         MessageInterface::ShowMessage(wxT("Spice kernel %s has already been loaded.\n"),
//               (*jj).c_str());
         return false;
      }
   kernelNameSPICE = fileName.char_str();
   furnsh_c(fileName.char_str());
   if (failed_c() == SPICETRUE)
   {
//      ConstSpiceChar option[] = wxT("SHORT"); // retrieve short error message, for now
//      SpiceInt       numChar  = MAX_SHORT_MESSAGE;
//      SpiceChar      err[MAX_SHORT_MESSAGE];
      #ifdef DEBUG_SPK_LOADING
            MessageInterface::ShowMessage(wxT("SpiceInterface: Error loading kernel %s <---------\n"),
                  fileName.c_str());
      #endif
      ConstSpiceChar option[] = "LONG"; // retrieve long error message, for now
      SpiceInt       numChar  = MAX_LONG_MESSAGE;
      SpiceChar      err[MAX_LONG_MESSAGE];
      getmsg_c(option, numChar, err);
      wxString errStr(wxString::FromAscii(err));
      wxString errmsg = wxT("Error loading kernel \"");
      errmsg += fileName + wxT("\".  Message received from CSPICE is: ");
      errmsg += errStr + wxT("\n");
      reset_c();
      throw UtilityException(errmsg);
   }
   #ifdef DEBUG_SPK_LOADING
      else
      {
         MessageInterface::ShowMessage(wxT("SpiceInterface Successfully loaded kernel %s <---------\n"),
               fileName.c_str());
      }
   #endif
   loadedKernels.push_back(fileName);
   
   return true;
}

//------------------------------------------------------------------------------
//  bool LoadKernels(const StringArray &fileNames)
//------------------------------------------------------------------------------
/**
 * This method loads the input files into the SPICE kernel pool.
 *
 * @param <fileNames>  array of kernels to load.
 *
 * @todo Find files if full path is not given - reference to appropriate
 *       path from startup file
 *
 * @return success flag.
 *
 */
//------------------------------------------------------------------------------
bool SpiceInterface::LoadKernels(const StringArray &fileNames)
{
   for (unsigned int ii = 0; ii < fileNames.size(); ii++)
      LoadKernel(fileNames.at(ii));
   return true;
}

//------------------------------------------------------------------------------
//  bool UnloadKernel(const wxString &fileName)
//------------------------------------------------------------------------------
/**
 * This method unloads the input file from the SPICE kernel pool.
 *
 * @param <fileName>  full path of the file (kernel) to unload.
 *
 * @return success flag.
 *
 */
//------------------------------------------------------------------------------
bool SpiceInterface::UnloadKernel(const wxString &fileName)
{
   bool found = false;
   for (StringArray::iterator jj = loadedKernels.begin();
        jj != loadedKernels.end(); ++jj)
      if ((*jj) == fileName)
      {
         found = true;
         loadedKernels.erase(jj);
         break;
      }
   if (!found)
   {
//      MessageInterface::ShowMessage(
//            wxT("SpiceInterface::UnloadKernel() - kernel %s is not currently loaded.\n"),
//            fileName.c_str());
      return false;
   }
   #ifdef DEBUG_SPK_LOADING
      MessageInterface::ShowMessage(wxT("Now attempting to unload kernel %s\n"),
            fileName.c_str());
   #endif
   kernelNameSPICE = fileName.char_str();
   unload_c(fileName.char_str());
   if (failed_c())
   {
//      ConstSpiceChar option[] = wxT("SHORT"); // retrieve short error message, for now
//      SpiceInt       numChar  = MAX_SHORT_MESSAGE;
//      SpiceChar      err[MAX_SHORT_MESSAGE];
      ConstSpiceChar option[] = "LONG"; // retrieve long error message, for now
      SpiceInt       numChar  = MAX_LONG_MESSAGE;
      SpiceChar      err[MAX_LONG_MESSAGE];
      getmsg_c(option, numChar, err);
      wxString errStr(wxString::FromAscii(err));
      wxString errmsg = wxT("Error unloading kernel \"");
      errmsg += fileName + wxT("\".  Message received from CSPICE is: ");
      errmsg += errStr + wxT("\n");
      reset_c();
      throw UtilityException(errmsg);
   }

   return true; 
}

//------------------------------------------------------------------------------
//  bool UnloadKernels(const StringArray &fileNames)
//------------------------------------------------------------------------------
/**
 * This method unloads the input files into the SPICE kernel pool.
 *
 * @param <fileNames>  array of kernels to load.
 *
 * @todo Find files if full path is not given - reference to appropriate
 *       path from startup file
 *
 * @return success flag.
 *
 */
//------------------------------------------------------------------------------
bool SpiceInterface::UnloadKernels(const StringArray &fileNames)
{
   for (unsigned int ii = 0; ii < fileNames.size(); ii++)
      UnloadKernel(fileNames.at(ii));
   return true;
}

//------------------------------------------------------------------------------
//  bool UnloadAllKernels()
//------------------------------------------------------------------------------
/**
 * This method unloads all loaded kernels from the SPICE kernel pool.
 *
 * @return success flag.
 *
 */
//------------------------------------------------------------------------------
bool SpiceInterface::UnloadAllKernels()
{
   for (StringArray::iterator jj = loadedKernels.begin();
        jj != loadedKernels.end(); ++jj)
   {
      #ifdef DEBUG_SPK_LOADING
         MessageInterface::ShowMessage(wxT("Now attempting to unload kernel %s\n"),
               (*jj).c_str());
      #endif
      kernelNameSPICE = (*jj).char_str();
      unload_c((*jj).char_str());
      if (failed_c())
      {
//         ConstSpiceChar option[] = wxT("SHORT"); // retrieve short error message, for now
//         SpiceInt       numChar  = MAX_SHORT_MESSAGE;
//         SpiceChar      err[MAX_SHORT_MESSAGE];
         ConstSpiceChar option[] = "LONG"; // retrieve long error message, for now
         SpiceInt       numChar  = MAX_LONG_MESSAGE;
         SpiceChar      err[MAX_LONG_MESSAGE];
         getmsg_c(option, numChar, err);
         wxString errStr(wxString::FromAscii(err));
         wxString errmsg = wxT("Error unloading kernel \"");
         errmsg += (*jj) + wxT("\".  Message received from CSPICE is: ");
         errmsg += errStr + wxT("\n");
         reset_c();
         throw UtilityException(errmsg);
      }
      // @todo - handle exceptional conditions (SPICE signals) here ...
   }
   loadedKernels.clear();
   return true;
}

//------------------------------------------------------------------------------
//  bool IsLoaded(const wxString &fileName)
//------------------------------------------------------------------------------
/**
 * This method checks to see if the input file is loaded into the kernel pool.
 *
 * @param <fileName>  full path of the file (kernel) to check for.
 *
 * @return true if the file (kernel) has been loaded;
 *         false otherwise
 *
 */
//------------------------------------------------------------------------------
bool SpiceInterface::IsLoaded(const wxString &fileName)
{
   #ifdef DEBUG_SPK_LOADING
      MessageInterface::ShowMessage(wxT("IsLoaded::Now attempting to find kernel name %s\n"), fileName.c_str());
   #endif
   for (StringArray::iterator jj = loadedKernels.begin();
        jj != loadedKernels.end(); ++jj)
   {
      if ((*jj) == fileName) return true;
   }
   #ifdef DEBUG_SPK_LOADING
      MessageInterface::ShowMessage(wxT("IsLoaded::kernel name %s NOT ALREADY LOADED\n"), fileName.c_str());
   #endif
   return false;
}



//------------------------------------------------------------------------------
//  StringArray GetValidAberrationCorrectionFlags()
//------------------------------------------------------------------------------
/**
 * This method returns a string array containing all valid aberration flags.
 *
  * @return array of  strings representing valid SPICE aberration flags.
 *
 */
//------------------------------------------------------------------------------
StringArray SpiceInterface::GetValidAberrationCorrectionFlags()
{
   StringArray aberr;
   for (unsigned int ii = 0; ii < 9; ii++)
      aberr.push_back(VALID_ABERRATION_FLAGS[ii]);
   return aberr;
}

//------------------------------------------------------------------------------
//  StringArray GetValidFrames()
//------------------------------------------------------------------------------
/**
 * This method returns a string array containing all valid frames.
 *
  * @return array of  strings representing valid (built-in) SPICE frames.
 *
 */
//------------------------------------------------------------------------------
StringArray SpiceInterface::GetValidFrames()
{
   StringArray frames;
   for (Integer ii = 0; ii < NUM_VALID_FRAMES; ii++)
      frames.push_back(VALID_FRAMES[ii]);
   return frames;
}

//------------------------------------------------------------------------------
//  void SetLeapSecondKernel(const wxString &lsk)
//------------------------------------------------------------------------------
/**
 * This method sets the leap second kernel, loading it into the kernel pool.
 *
 * @param <lsk>  full path of the leap second  kernel to load.
 *
 *
 */
//------------------------------------------------------------------------------
void SpiceInterface::SetLeapSecondKernel(const wxString &lsk)
{
   #ifdef DEBUG_SPK_LOADING
      MessageInterface::ShowMessage(wxT("NOW loading LSK kernel %s\n"), lsk.c_str());
   #endif
   lsKernel = lsk;
   if (!IsLoaded(lsKernel))   LoadKernel(lsKernel);
}

//------------------------------------------------------------------------------
//  Integer GetNaifID(const wxString &forBody, bool popupMsg)
//------------------------------------------------------------------------------
/**
 * This method returns the NAIF Id of an object, given its name.
 *
 * @param <forBody>  name of the object.
 * @param <popupMsg> indicates whether or not to put up a Popup message if there
 *                   is an error
 *
 * @return corresponding NAIF id; or 0 if not found
 *
 */
//------------------------------------------------------------------------------
Integer SpiceInterface::GetNaifID(const wxString &forBody, bool popupMsg)
{
   SpiceBoolean   found;
   SpiceInt       id;
   ConstSpiceChar *bodyName = forBody.char_str();
   bodn2c_c(bodyName, &id, &found);
   if (found == SPICEFALSE)
   {
      if (popupMsg)
      {
         wxString warnmsg = wxT("Cannot find NAIF ID for object ");
         warnmsg += forBody + wxT(".  Insufficient data available.  Another SPICE Kernel may be necessary.");
         MessageInterface::PopupMessage(Gmat::WARNING_, warnmsg);
      }
      return 0;
   }
   #ifdef DEBUG_SPK_READING
      MessageInterface::ShowMessage(wxT("NAIF ID for body %s has been found: it is %d\n"),
                                    forBody.c_str(), (Integer) id);
   #endif
   return (Integer) id;
}


//------------------------------------------------------------------------------
//  Real SpiceTimeToA1(SpiceDouble spiceTime)
//------------------------------------------------------------------------------
/**
 * This method converts from a SPICE time (seconds from J2000 ET) to A.1.
 *
 * @param <spiceTime>  SPICE time (seconds since J2000 ET).
 *
 * @return resulting A.1 time
 *
 */
//------------------------------------------------------------------------------
Real SpiceInterface::SpiceTimeToA1(SpiceDouble spiceTime)
{
   SpiceDouble j2ET    = j2000_c();
   SpiceDouble julianOffset = GmatTimeConstants::JD_JAN_5_1941 - j2ET;
   Real        tdbTime = (spiceTime / GmatTimeConstants::SECS_PER_DAY) - julianOffset;

   Real        a1Time  = TimeConverterUtil::Convert(tdbTime, TimeConverterUtil::TDBMJD,
                         TimeConverterUtil::A1MJD, GmatTimeConstants::JD_JAN_5_1941);

   return a1Time;
}


//------------------------------------------------------------------------------
//  SpiceDouble A1ToSpiceTime(Real a1Time)
//------------------------------------------------------------------------------
/**
 * This method converts from a n A.1 time to SPICE time (seconds from J2000 ET).
 *
 * @param <spiceTime>  A.1 time .
 *
 * @return resulting SPICE time
 *
 */
//------------------------------------------------------------------------------
SpiceDouble SpiceInterface::A1ToSpiceTime(Real a1Time)
{
   SpiceDouble j2ET      = j2000_c();
   Real        tdbTime   = TimeConverterUtil::Convert(a1Time, TimeConverterUtil::A1MJD,
                           TimeConverterUtil::TDBMJD, GmatTimeConstants::JD_JAN_5_1941);
   SpiceDouble julianOffset = GmatTimeConstants::JD_JAN_5_1941 - j2ET;
   SpiceDouble spiceTime = (tdbTime + julianOffset) *
                           GmatTimeConstants::SECS_PER_DAY;

   return spiceTime;
}

//---------------------------------
// protected methods
//---------------------------------

// ---------------------
// static methods
// ---------------------
//------------------------------------------------------------------------------
//  void InitializeInterface()
//------------------------------------------------------------------------------
/**
 * This method initializes the static data for the reader, and sets up
 * SPICE error handling.
 *
 * @param <forBody>  name of the object.
 *
 * @return corresponding NAIF id; or 0 if not found
 *
 */
//------------------------------------------------------------------------------
//void SpiceInterface::InitializeReader()
void SpiceInterface::InitializeInterface()
{
   if (numInstances == 0)
   {
      loadedKernels.clear();
      // Get path for output
      FileManager *fm = FileManager::Instance();
      wxString outPath = fm->GetAbsPathname(FileManager::OUTPUT_PATH) + wxT("GMATSpiceKernelError.txt");
      // need to get rid of const-ness to convert to SpiceChar*
      char *pathChar;
      pathChar = new char[outPath.length() + 1];
      strcpy(pathChar, outPath.char_str());

      // set output file for cspice methods
      SpiceChar      *spiceErrorFileFullPath = pathChar;
      errdev_c("SET", 1840, spiceErrorFileFullPath);

      // set actions for cspice error writing
      char  report[4] = "ALL";
      char  action[7] = "RETURN";
      errprt_c("SET", 1840, report);
      erract_c("SET", 1840, action);

      delete [] pathChar;
   }
}


