//$Id: AngularParameters.cpp 9828 2011-09-01 15:57:08Z wendys-dev $
//------------------------------------------------------------------------------
//                             File: AngularParameters.cpp
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
// Created: 2004/03/18
//
/**
 * Implements Angular related parameter classes.
 *   SemilatusRectum, AngularMomemtumMag, AngularMomentumX, AngularMomentumY,
 *   AngularMomentumZ, BetaAngle
 */
//------------------------------------------------------------------------------

#include "AngularParameters.hpp"

//==============================================================================
//                              SemilatusRectum
//==============================================================================
/**
 * Implements SemilatusRectum parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SemilatusRectum(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
SemilatusRectum::SemilatusRectum(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("SemilatusRectum"), obj, wxT("Semilatus Rectum"), wxT("Km"), GmatParam::ORIGIN)
{
   mDepObjectName = wxT("Earth");
   SetRefObjectName(Gmat::SPACE_POINT, wxT("Earth"));
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, wxT("EarthMJ2000Eq"));
}


//------------------------------------------------------------------------------
// SemilatusRectum(const SemilatusRectum &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
SemilatusRectum::SemilatusRectum(const SemilatusRectum &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// const SemilatusRectum& operator=(const SemilatusRectum &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const SemilatusRectum&
SemilatusRectum::operator=(const SemilatusRectum &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~SemilatusRectum()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
SemilatusRectum::~SemilatusRectum()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool SemilatusRectum::Evaluate()
{
   mRealValue = OrbitData::GetAngularReal(SEMILATUS_RECTUM);    
    
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* SemilatusRectum::Clone(void) const
{
   return new SemilatusRectum(*this);
}


//==============================================================================
//                              AngularMomentumMag
//==============================================================================
/**
 * Implements AngularMomentumMag parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// AngularMomentumMag(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
AngularMomentumMag::AngularMomentumMag(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("HMAG"), obj, wxT("Angular Momentum Mag"), wxT("Km^2/s"), GmatParam::ORIGIN)
{
   mDepObjectName = wxT("Earth");
   SetRefObjectName(Gmat::SPACE_POINT, wxT("Earth"));
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, wxT("EarthMJ2000Eq"));
}


//------------------------------------------------------------------------------
// AngularMomentumMag(const AngularMomentumMag &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
AngularMomentumMag::AngularMomentumMag(const AngularMomentumMag &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// const AngularMomentumMag& operator=(const AngularMomentumMag &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const AngularMomentumMag&
AngularMomentumMag::operator=(const AngularMomentumMag &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~AngularMomentumMag()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
AngularMomentumMag::~AngularMomentumMag()
{
}

//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool AngularMomentumMag::Evaluate()
{
   mRealValue = OrbitData::GetAngularReal(HMAG);    
   
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* AngularMomentumMag::Clone(void) const
{
   return new AngularMomentumMag(*this);
}


//==============================================================================
//                              AngularMomentumX
//==============================================================================
/**
 * Implements AngularMomentumX parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// AngularMomentumX(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
AngularMomentumX::AngularMomentumX(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("HX"), obj, wxT("Angular Momentum X"), wxT("Km^2/s"), GmatParam::COORD_SYS)
{
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
}


//------------------------------------------------------------------------------
// AngularMomentumX(const AngularMomentumX &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
AngularMomentumX::AngularMomentumX(const AngularMomentumX &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// const AngularMomentumX& operator=(const AngularMomentumX &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const AngularMomentumX&
AngularMomentumX::operator=(const AngularMomentumX &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~AngularMomentumX()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
AngularMomentumX::~AngularMomentumX()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool AngularMomentumX::Evaluate()
{
   mRealValue = OrbitData::GetAngularReal(HX);    
   
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* AngularMomentumX::Clone(void) const
{
   return new AngularMomentumX(*this);
}


//==============================================================================
//                              AngularMomentumY
//==============================================================================
/**
 * Implements AngularMomentumY parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// AngularMomentumY(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
AngularMomentumY::AngularMomentumY(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("HY"), obj, wxT("Angular Momentum Y"), wxT("Km^2/s"), GmatParam::COORD_SYS)
{
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
}


//------------------------------------------------------------------------------
// AngularMomentumY(const AngularMomentumY &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
AngularMomentumY::AngularMomentumY(const AngularMomentumY &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// const AngularMomentumY& operator=(const AngularMomentumY &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const AngularMomentumY&
AngularMomentumY::operator=(const AngularMomentumY &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~AngularMomentumY()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
AngularMomentumY::~AngularMomentumY()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool AngularMomentumY::Evaluate()
{
   mRealValue = OrbitData::GetAngularReal(HY);    
   
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}

//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* AngularMomentumY::Clone(void) const
{
   return new AngularMomentumY(*this);
}


//==============================================================================
//                              AngularMomentumZ
//==============================================================================
/**
 * Implements AngularMomentumZ parameter class.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// AngularMomentumZ(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
AngularMomentumZ::AngularMomentumZ(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("HZ"), obj, wxT("Angular Momentum Z"), wxT("Km^2/s"), GmatParam::COORD_SYS)
{
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
}


//------------------------------------------------------------------------------
// AngularMomentumZ(const AngularMomentumZ &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
AngularMomentumZ::AngularMomentumZ(const AngularMomentumZ &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// const AngularMomentumZ& operator=(const AngularMomentumZ &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const AngularMomentumZ&
AngularMomentumZ::operator=(const AngularMomentumZ &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~AngularMomentumZ()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
AngularMomentumZ::~AngularMomentumZ()
{
}


//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool AngularMomentumZ::Evaluate()
{
   mRealValue = OrbitData::GetAngularReal(HZ);    
   
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* AngularMomentumZ::Clone(void) const
{
   return new AngularMomentumZ(*this);
}


//==============================================================================
//                              BetaAngle
//==============================================================================
/**
 * Implements BetaAngle parameter class. BetaAngle is the angle between the
 * orbit normal vector, and the vector from the celestial body to the sun.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// BetaAngle(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
BetaAngle::BetaAngle(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("BetaAngle"), obj, wxT("Beta Angle"), wxT("deg"), GmatParam::ORIGIN)
{
   mDepObjectName = wxT("Earth");
   SetRefObjectName(Gmat::SPACE_POINT, wxT("Earth"));
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, wxT("EarthMJ2000Eq"));
}


//------------------------------------------------------------------------------
// BetaAngle(const BetaAngle &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
BetaAngle::BetaAngle(const BetaAngle &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// const BetaAngle& operator=(const BetaAngle &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const BetaAngle&
BetaAngle::operator=(const BetaAngle &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~BetaAngle()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
BetaAngle::~BetaAngle()
{
}

//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool BetaAngle::Evaluate()
{
   mRealValue = OrbitData::GetOtherAngleReal(BETA_ANGLE);
   
   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* BetaAngle::Clone(void) const
{
   return new BetaAngle(*this);
}

//==============================================================================
//                              RLA
//==============================================================================
/**
 * Implements RLA parameter class. RLA is the right ascension of the outgoing
 * asymptote of a hyperbolic trajectory.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// RLA(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
RLA::RLA(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("RLA"), obj, wxT("Right Ascension Outgoing Asymptote"), wxT("deg"), GmatParam::COORD_SYS)
{
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
}


//------------------------------------------------------------------------------
// RLA(const RLA &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
RLA::RLA(const RLA &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// const RLA& operator=(const RLA &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const RLA& RLA::operator=(const RLA &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~RLA()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
RLA::~RLA()
{
}

//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool RLA::Evaluate()
{
   mRealValue = OrbitData::GetAngularReal(HYPERBOLIC_RLA);

   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* RLA::Clone(void) const
{
   return new RLA(*this);
}

//==============================================================================
//                              DLA
//==============================================================================
/**
 * Implements DLA parameter class. DLA is the declination of the outgoing
 * asymptote of a hyperbolic trajectory.
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// DLA(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
DLA::DLA(const wxString &name, GmatBase *obj)
   : OrbitReal(name, wxT("DLA"), obj, wxT("Declination Outgoing Asymptote"), wxT("deg"), GmatParam::COORD_SYS)
{
   mDepObjectName = wxT("EarthMJ2000Eq");
   SetRefObjectName(Gmat::COORDINATE_SYSTEM, mDepObjectName);
}


//------------------------------------------------------------------------------
// DLA(const DLA &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
DLA::DLA(const DLA &copy)
   : OrbitReal(copy)
{
}


//------------------------------------------------------------------------------
// const DLA& operator=(const DLA &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const DLA& DLA::operator=(const DLA &right)
{
   if (this != &right)
      OrbitReal::operator=(right);

   return *this;
}


//------------------------------------------------------------------------------
// ~DLA()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
DLA::~DLA()
{
}

//-------------------------------------
// Inherited methods from Parameter
//-------------------------------------

//------------------------------------------------------------------------------
// virtual bool Evaluate()
//------------------------------------------------------------------------------
/**
 * Evaluates value of the parameter.
 *
 * @return true if parameter value successfully evaluated; false otherwise
 */
//------------------------------------------------------------------------------
bool DLA::Evaluate()
{
   mRealValue = OrbitData::GetAngularReal(HYPERBOLIC_DLA);

   if (mRealValue == GmatOrbitConstants::ORBIT_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//-------------------------------------
// methods inherited from GmatBase
//-------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* DLA::Clone(void) const
{
   return new DLA(*this);
}
