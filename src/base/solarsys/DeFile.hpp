//$Id: DeFile.hpp 9707 2011-07-19 00:13:22Z djcinsb $
//------------------------------------------------------------------------------
//                                  DeFile
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
// Created: 2004/02/17
//
/**
 * This is the class that handles access to a DE file.
 *
 * @todo : complete Convert method for converting to binary
 *
 * @note For Build 2, conversion from ASCII to Binary not done
 *       (assume  preprocessed).  Also, only 200 and 405 formats are handled.
 *
 * @note Code reused from or based on JPL/JSC (D. Hoffman) code.
 *
 * @note If an ASCII file is input on creation, the file will be converted to
 *       a file in native binary format, for efficiency.  This conversion
 *       assumes that there is an appropriate header file in the same directory
 *       as the ASCII file.  The header file should be called header.FMT, where
 *       FMT is the format (e.g. 200, etc.) of the DE file.
 */
//------------------------------------------------------------------------------
#ifndef DeFile_hpp
#define DeFile_hpp

#include "gmatdefs.hpp"
#include "A1Mjd.hpp"
#include "PlanetaryEphem.hpp"

#include <stdio.h> // for FILE, etc. (for JPL/JSC code (Hoffman))

class GMAT_API DeFile : public PlanetaryEphem
{
public:

   // default constructor
   DeFile(Gmat::DeFileType ofType, wxString fileName = wxT(""),
          Gmat::DeFileFormat fmt = Gmat::DE_BINARY);  
   // copy constructor
   DeFile(const DeFile& def);
   // operator=
   DeFile& operator=(const DeFile& sdef);
   // destructor
   ~DeFile();

   //loj: added so that constructor won't throw an exception
   // method to initialize the DeFile - must be done before De file can be read
   void Initialize();
   
   
   // method to return the body ID for the requested body
   Integer  GetBodyID(wxString bodyName);

   // method to return the ASCII file name (unknown if created with
   // BINARY file)
   wxString GetAsciiFileName() const;

   // method to return the BINARY file name
   wxString GetBinaryFileName() const;

   // method to return the type of De File
   Gmat::DeFileType GetDeFileType() const;

   // method to return the position and velocity of the specified body
   // at the specified time
   Real* GetPosVel(Integer forBody, A1Mjd atTime, 
                   bool overrideTimeSystem = false);
   
   /// method to return angles and rates (for Luna ONLY!!)                
   void  GetAnglesAndRates(A1Mjd atTime, Real* angles, Real* rates, 
                           bool overrideTimeSystem = false);

   // method to return the day-of-year and year of the start time of the
   // DE file.
   Integer* GetStartDayAndYear();

   // method to convert an ASCII file to a binary file; this method assumes that
   // there is  an appropriate header file in the same directory as the
   // ASCII file.  @todo - code this method, as its implementation is currently TBD
   wxString Convert(wxString deFileNameAscii); 


   // static values for the IDs for each celestial body, as used on teh DE files.
   static const Integer SUN_ID;
   static const Integer MERCURY_ID;
   static const Integer VENUS_ID;
   static const Integer EARTH_ID;
   static const Integer MOON_ID;
   static const Integer MARS_ID;
   static const Integer JUPITER_ID;
   static const Integer SATURN_ID;
   static const Integer URANUS_ID;
   static const Integer NEPTUNE_ID;
   static const Integer PLUTO_ID;
   static const Integer SS_BARY_ID;
   static const Integer EM_BARY_ID;
   static const Integer NUTATIONS_ID;
   static const Integer LIBRATIONS_ID;

   static const Real    JD_MJD_OFFSET;// = GmatTimeConstants::JD_JAN_5_1941;
   // seconds offset to get from A1 to TDT (TT)
   static const Real    TT_OFFSET;//     = 32.184;

   static const Integer ARRAY_SIZE_200 = 826;
   static const Integer ARRAY_SIZE_405 = 1018;
   static const Integer MAX_ARRAY_SIZE = 1018;

   static const Integer FAILURE        = 1; // from JPL/JSC ephem_types.h
   static const Integer SUCCESS        = 0; // from JPL/JSC ephem_types.h


protected:

   // structs representing the state date (positiona nd velocity)
   // (from JPL/JSC code - Hoffman)
   struct GMAT_API stateData{
         double Position[3];
         double Velocity[3];
      };

   typedef struct stateData stateType;
   
   // structs representing the format of the header records
   // (from JPL/JSC code - Hoffman)
   // wcs - added constructors, and operator=
#pragma pack(push, 1)
   struct GMAT_API recOneData 
   {
      recOneData()  // default constructor
      {
         int i, j;
         for (i=0;i<3;i++)
         {
            strcpy(label[i], "\0");
            timeData[i]  = 0.0;
            libratPtr[i] = 0;
            for (j=0;j<12;j++) coeffPtr[j][i] = 0;
         }
         for (i=0;i<400;i++)    strcpy(constName[i], "\0");
         numConst        = 0;
         AU              = 0.0;
         EMRAT           = 0.0;
         #if (USE_64_BIT_LONGS == 1)
            DENUM           = (int) 0;
         #else
            DENUM           = (long int) 0;
         #endif
      }
      recOneData(const recOneData& r)  // copy constructor
      {
         int i, j;
         for (i=0;i<3;i++)
         {
            strcpy(label[i],r.label[i]);
            timeData[i]  = r.timeData[i];
            libratPtr[i] = r.libratPtr[i];
            for (j=0;j<12;j++) coeffPtr[j][i] = r.coeffPtr[j][i];
         }
         for (i=0;i<400;i++)    strcpy(constName[i], r.constName[i]);
         numConst        = r.numConst;
         AU              = r.AU;
         EMRAT           = r.EMRAT;
         DENUM           = r.DENUM;
      }
      recOneData& operator=(const recOneData& r) 
      {
         int i, j;
         for (i=0;i<3;i++)
         {
            strcpy(label[i], r.label[i]);
            timeData[i]  = r.timeData[i];
            libratPtr[i] = r.libratPtr[i];
            for (j=0;j<12;j++) coeffPtr[j][i] = r.coeffPtr[j][i];
         }
	 for (i=0;i<400;i++)    strcpy(constName[i], r.constName[i]);
         numConst        = r.numConst;
         AU              = r.AU;
         EMRAT           = r.EMRAT;
         DENUM           = r.DENUM;
         return *this;
      }
      // data
      char label[3][84];
      char constName[400][6];
      double timeData[3];
      #if (USE_64_BIT_LONGS == 1)
         int numConst;
      #else
         long int numConst;
      #endif
      double AU;
      double EMRAT;
      #if (USE_64_BIT_LONGS == 1)
         int coeffPtr[12][3];
         int DENUM;
         int libratPtr[3];
         int RSize; //loj: 4/14/04 added
      #else
         long int coeffPtr[12][3];
         long int DENUM;
         long int libratPtr[3];
         long int RSize; //loj: 4/14/04 added
      #endif
   };

   struct GMAT_API recTwoData 
   {
      // default constructor
      recTwoData()
      {
         int i;
         for (i=0;i<400;i++) constValue[i] = 0.0;
      }
      // copy constructor
      recTwoData(const recTwoData& r)
      {
         int i;
         for (i=0;i<400;i++) constValue[i] = r.constValue[i];
      }
      // operator=
      recTwoData& operator=(const recTwoData& r)
      {
         int i;
         for (i=0;i<400;i++) constValue[i] = r.constValue[i];
         return *this;
      }
      //data
      double constValue[400];
   };
#pragma pack(pop)
    
   typedef struct recOneData recOneType;
   typedef struct recTwoData recTwoType;

   // structs representing the formats of the header records - need ARRAY_SIZE?
   struct GMAT_API headerOne 
   {
      recOneType data;
      char pad[MAX_ARRAY_SIZE*sizeof(double) - sizeof(recOneType)];  // MAX
   };

   struct GMAT_API headerTwo 
   {
      recTwoType data;
      char pad[MAX_ARRAY_SIZE*sizeof(double) - sizeof(recTwoType)];  // MAX
   };

   typedef struct headerOne headOneType;
   typedef struct headerTwo headTwoType;


private:

   wxString theFileName;
   Gmat::DeFileFormat theFileFormat;
   
   /// ASCII file name - will be unknown if DeFile created with binary file
   wxString asciiFileName;

   /// BINARY file name - may be input on cretion, or may be result of
   /// conversion of input ASCIi file; date will be read from this file.
   wxString binaryFileName; 

   /// type of DE file
   Gmat::DeFileType defType;

   /// array size for the file format we're using
   Integer arraySize;

   /// data from JPL/JSC code (Hoffman) ephem_read.c
   headOneType  H1;
   headTwoType  H2;
   recOneType   R1;
   FILE        *Ephemeris_File;
   double       Coeff_Array[MAX_ARRAY_SIZE];   // MAX
   double       T_beg , T_end , T_span;
   /// The base epoch for internal time calculations
   double       baseEpoch;
   double       mFileBeg;
   /// File beginning in ModJulian format
   double       mA1FileBeg;

   #if (USE_64_BIT_LONGS == 1)
      int numConst;
   #else
      long int numConst;
   #endif
   
   Integer EPHEMERIS; // caps because of reuse from JPL/JSC code (Hoffman)

   

   // method to initialize the DeFile - must be done before De file can be read
   void InitializeDeFile(wxString fName, Gmat::DeFileFormat fileFmt);


   // --------------- methods from JPL/JSC code (Hoffman) ______________________


   void   Read_Coefficients( double Time );
   int    Initialize_Ephemeris( char *fileName );
   void   Interpolate_Libration( double Time, int Target, double Libration[3], double rates[3] );
   void   Interpolate_Nutation( double Time, int Target, double Nutation[2] );
   void   Interpolate_Position( double Time, int Target, double Position[3] );
   void   Interpolate_State( double Time, int Target, stateType *p );
   double Find_Value( char name[], char name_array[400][6], double value_array[400] );
   double Gregorian_to_Julian( int year, int month, int day, int hour, int min, double seconds );
   int    mod( int x, int y );
   int    Read_File_Line( FILE *inFile, int filter, char lineBuffer[82]);
   int    Read_Group_Header( FILE *inFile );

};
#endif // DeFile_hpp
