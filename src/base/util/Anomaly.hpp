//$Id: Anomaly.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                             Anomaly 
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
// Arthor:  Joey Gurganus 
// Created: 2005/02/17 
//
/**
 * Definition for the Anomaly class to compute the true, mean, and eccentric
 * anomaly using semi-major axis and eccentricity.
 *
 */
//------------------------------------------------------------------------------
#ifndef Anomaly_hpp
#define Anomaly_hpp

#include "gmatdefs.hpp"
#include "GmatGlobal.hpp"         // for DATA_PRECISION

class GMAT_API Anomaly 
{
public:
   
   enum AnomalyType
   {
      TA,
      MA,
      EA,
      HA,
      AnomalyTypeCount
   };
   
   Anomaly();
   Anomaly(Real sma, Real ecc, Real value, AnomalyType type = TA,
           bool valueInRadians = false);
   Anomaly(Real sma, Real ecc, Real value, const wxString &type = wxT("TA"),
           bool valueInRadians = false);
   Anomaly(const Anomaly &anomaly);
   Anomaly& operator=(const Anomaly &anomaly);
   virtual ~Anomaly();
   
   void Set(Real sma, Real ecc, Real value, AnomalyType type,
            bool valueInRadians = false);
   void Set(Real sma, Real ecc, Real value, const wxString &type,
            bool valueInRadians = false);
   
   Real GetSMA() const { return mSma; }
   Real GetECC() const { return mEcc; }
   void SetSMA(const Real sma) { mSma = sma; }
   void SetECC(const Real ecc) { mEcc = ecc; }
   
   Real GetValue(bool valueInRadians = false) const;
   Real GetValue(AnomalyType type, bool inRadians = false) const;
   Real GetValue(const wxString &type, bool inRadians = false) const;
   void SetValue(Real value, bool valueInRadians = false);
   
   AnomalyType GetType() const { return mType; }
   AnomalyType GetType(const wxString &typeStr) const;
   void SetType(AnomalyType type) { mType = type; }
   wxString GetTypeString() const;
   void SetType(const wxString &type);
   
   Real GetTrueAnomaly(bool inRadians = false) const;
   Real GetMeanAnomaly(bool inRadians = false) const;
   Real GetEccentricAnomaly(bool inRadians = false) const;
   Real GetHyperbolicAnomaly(bool inRadians = false) const;
   
   bool IsInvalid(const wxString &typeStr) const;
   
   Real Convert(AnomalyType toType, bool inRadians = false) const;   
   Real Convert(const wxString &toType, bool inRadians = false) const;
   
   Anomaly ConvertToAnomaly(AnomalyType toType, bool inRadians = false);
   Anomaly ConvertToAnomaly(const wxString &toType, bool inRadians = false);
   
   wxString ToString(Integer precision = GmatGlobal::DATA_PRECISION);
   
   static AnomalyType GetAnomalyType(const wxString &typeStr);
   static wxString GetTypeString(const wxString &type);
   static wxString GetLongTypeString(const wxString &type);
   static Integer GetTypeCount() { return AnomalyTypeCount; }
   static const wxString* GetLongTypeNameList();
   
protected:
   
private:

   Real mSma;              // SemimajorAxis
   Real mEcc;              // Eccentricity
   Radians mAnomalyInRad;  // in radians
   AnomalyType mType;
   
   static const wxString ANOMALY_LONG_TEXT[AnomalyTypeCount];
   static const wxString ANOMALY_SHORT_TEXT[AnomalyTypeCount];
   
};
#endif // Anomaly_hpp
