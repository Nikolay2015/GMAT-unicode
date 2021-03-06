//$Id: TimeParameters.cpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                             File: TimeParameters.cpp
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
// Created: 2004/03/12
//
/**
 * Implements Time related parameter classes.
 *   CurrA1MJD, A1ModJulian, A1Gregorian, TAIModJulian, TAIGregorian, TTModJulian,
 *   TTGregorian, TDBModJulian, TDBGregorian, TCBModJulian, TCBGregorian,
 *   UTCModJulian, UGCGregorian, ElapsedDays, ElapsedDaysFromStart, ElapsedSecs,
 *   ElapsedSecsFromStart
 */
//------------------------------------------------------------------------------
#include "TimeParameters.hpp"
#include "MessageInterface.hpp"

//==============================================================================
//                              CurrA1MJD
//==============================================================================

//------------------------------------------------------------------------------
// CurrA1MJD(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
CurrA1MJD::CurrA1MJD(const wxString &name, GmatBase *obj)
   : TimeReal(name, wxT("CurrA1MJD"), obj, wxT("A1 Mod. Julian Days"), wxT("day"))
{
}


//------------------------------------------------------------------------------
// CurrA1MJD::CurrA1MJD(const CurrA1MJD &param)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <param> the parameter to make copy of
 */
//------------------------------------------------------------------------------
CurrA1MJD::CurrA1MJD(const CurrA1MJD &param)
   : TimeReal(param)
{
}

//------------------------------------------------------------------------------
// CurrA1MJD& CurrA1MJD::operator=(const CurrA1MJD &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
CurrA1MJD& CurrA1MJD::operator=(const CurrA1MJD &right)
{
   if (this != &right)
      TimeReal::operator=(right);
      
   return *this;
}

//------------------------------------------------------------------------------
// CurrA1MJD::~CurrA1MJD()
//------------------------------------------------------------------------------
CurrA1MJD::~CurrA1MJD()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool CurrA1MJD::Evaluate()
{
   mRealValue = GetCurrentTimeReal(A1_MJD);
   
   if (mRealValue == TIME_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* CurrA1MJD::Clone(void) const
{
   return new CurrA1MJD(*this);
}


//==============================================================================
//                              A1ModJulian
//==============================================================================

//------------------------------------------------------------------------------
// A1ModJulian(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
A1ModJulian::A1ModJulian(const wxString &name, GmatBase *obj)
   : TimeReal(name, wxT("A1ModJulian"), obj, wxT("A1 Mod. Julian Days"), wxT("day"))
{
}


//------------------------------------------------------------------------------
// A1ModJulian::A1ModJulian(const A1ModJulian &param)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <param> the parameter to make copy of
 */
//------------------------------------------------------------------------------
A1ModJulian::A1ModJulian(const A1ModJulian &param)
   : TimeReal(param)
{
}

//------------------------------------------------------------------------------
// A1ModJulian& A1ModJulian::operator=(const A1ModJulian &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
A1ModJulian& A1ModJulian::operator=(const A1ModJulian &right)
{
   if (this != &right)
      TimeReal::operator=(right);
      
   return *this;
}

//------------------------------------------------------------------------------
// A1ModJulian::~A1ModJulian()
//------------------------------------------------------------------------------
A1ModJulian::~A1ModJulian()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool A1ModJulian::Evaluate()
{
   mRealValue = GetCurrentTimeReal(A1_MJD);

   #ifdef DEBUG_TIMEPARAM_EVAL
   MessageInterface::ShowMessage
      (wxT("A1ModJulian::Evaluate() <%p>'%s' evaluated to %f, spacecraft=<%p>'%s'\n"),
       this, GetName().c_str(), mRealValue, GetSpacecraft(),
       GetSpacecraft()->GetName().c_str());
   #endif
   
   if (mRealValue == TIME_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* A1ModJulian::Clone(void) const
{
   return new A1ModJulian(*this);
}


//==============================================================================
//                              A1Gregorian
//==============================================================================

//------------------------------------------------------------------------------
// A1Gregorian(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
A1Gregorian::A1Gregorian(const wxString &name, GmatBase *obj)
   : TimeString(name, wxT("A1Gregorian"), obj, wxT("A1 Gregorian Date"), wxT(""))
{
}


//------------------------------------------------------------------------------
// A1Gregorian::A1Gregorian(const A1Gregorian &param)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <param> the parameter to make copy of
 */
//------------------------------------------------------------------------------
A1Gregorian::A1Gregorian(const A1Gregorian &param)
   : TimeString(param)
{
}

//------------------------------------------------------------------------------
// A1Gregorian& A1Gregorian::operator=(const A1Gregorian &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
A1Gregorian& A1Gregorian::operator=(const A1Gregorian &right)
{
   if (this != &right)
      TimeString::operator=(right);
      
   return *this;
}

//------------------------------------------------------------------------------
// A1Gregorian::~A1Gregorian()
//------------------------------------------------------------------------------
A1Gregorian::~A1Gregorian()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool A1Gregorian::Evaluate()
{
   mStringValue = GetCurrentTimeString(A1_MJD);
   
   if (mStringValue == TIME_STRING_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* A1Gregorian::Clone(void) const
{
   return new A1Gregorian(*this);
}


//==============================================================================
//                              TAIModJulian
//==============================================================================

//------------------------------------------------------------------------------
// TAIModJulian(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
TAIModJulian::TAIModJulian(const wxString &name, GmatBase *obj)
   : TimeReal(name, wxT("TAIModJulian"), obj, wxT("TA1 Mod. Julian Days"), wxT("day"))
{
}


//------------------------------------------------------------------------------
// TAIModJulian::TAIModJulian(const TAIModJulian &param)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <param> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TAIModJulian::TAIModJulian(const TAIModJulian &param)
   : TimeReal(param)
{
}

//------------------------------------------------------------------------------
// TAIModJulian& TAIModJulian::operator=(const TAIModJulian &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TAIModJulian& TAIModJulian::operator=(const TAIModJulian &right)
{
   if (this != &right)
      TimeReal::operator=(right);
      
   return *this;
}

//------------------------------------------------------------------------------
// TAIModJulian::~TAIModJulian()
//------------------------------------------------------------------------------
TAIModJulian::~TAIModJulian()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool TAIModJulian::Evaluate()
{
   mRealValue = GetCurrentTimeReal(TAI_MJD);
   
   if (mRealValue == TIME_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* TAIModJulian::Clone(void) const
{
   return new TAIModJulian(*this);
}


//==============================================================================
//                              TAIGregorian
//==============================================================================

//------------------------------------------------------------------------------
// TAIGregorian(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
TAIGregorian::TAIGregorian(const wxString &name, GmatBase *obj)
   : TimeString(name, wxT("TAIGregorian"), obj, wxT("TA1 Gregorian Date"), wxT(""))
{
}


//------------------------------------------------------------------------------
// TAIGregorian::TAIGregorian(const TAIGregorian &param)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <param> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TAIGregorian::TAIGregorian(const TAIGregorian &param)
   : TimeString(param)
{
}

//------------------------------------------------------------------------------
// TAIGregorian& TAIGregorian::operator=(const TAIGregorian &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TAIGregorian& TAIGregorian::operator=(const TAIGregorian &right)
{
   if (this != &right)
      TimeString::operator=(right);
      
   return *this;
}

//------------------------------------------------------------------------------
// TAIGregorian::~TAIGregorian()
//------------------------------------------------------------------------------
TAIGregorian::~TAIGregorian()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool TAIGregorian::Evaluate()
{
   mStringValue = GetCurrentTimeString(TAI_MJD);
   
   if (mStringValue == TIME_STRING_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* TAIGregorian::Clone(void) const
{
   return new TAIGregorian(*this);
}


//==============================================================================
//                              TTModJulian
//==============================================================================

//------------------------------------------------------------------------------
// TTModJulian(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
TTModJulian::TTModJulian(const wxString &name, GmatBase *obj)
   : TimeReal(name, wxT("TTModJulian"), obj, wxT("TT Mod. Julian Days"), wxT("day"))
{
}


//------------------------------------------------------------------------------
// TTModJulian::TTModJulian(const TTModJulian &param)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <param> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TTModJulian::TTModJulian(const TTModJulian &param)
   : TimeReal(param)
{
}

//------------------------------------------------------------------------------
// TTModJulian& TTModJulian::operator=(const TTModJulian &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TTModJulian& TTModJulian::operator=(const TTModJulian &right)
{
   if (this != &right)
      TimeReal::operator=(right);
      
   return *this;
}

//------------------------------------------------------------------------------
// TTModJulian::~TTModJulian()
//------------------------------------------------------------------------------
TTModJulian::~TTModJulian()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool TTModJulian::Evaluate()
{
   mRealValue = GetCurrentTimeReal(TT_MJD);
   
   if (mRealValue == TIME_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* TTModJulian::Clone(void) const
{
   return new TTModJulian(*this);
}


//==============================================================================
//                              TTGregorian
//==============================================================================

//------------------------------------------------------------------------------
// TTGregorian(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
TTGregorian::TTGregorian(const wxString &name, GmatBase *obj)
   : TimeString(name, wxT("TTGregorian"), obj, wxT("TT Gregorian Date"), wxT(""))
{
}


//------------------------------------------------------------------------------
// TTGregorian::TTGregorian(const TTGregorian &param)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <param> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TTGregorian::TTGregorian(const TTGregorian &param)
   : TimeString(param)
{
}

//------------------------------------------------------------------------------
// TTGregorian& TTGregorian::operator=(const TTGregorian &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TTGregorian& TTGregorian::operator=(const TTGregorian &right)
{
   if (this != &right)
      TimeString::operator=(right);
      
   return *this;
}

//------------------------------------------------------------------------------
// TTGregorian::~TTGregorian()
//------------------------------------------------------------------------------
TTGregorian::~TTGregorian()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool TTGregorian::Evaluate()
{
   mStringValue = GetCurrentTimeString(TT_MJD);
   
   if (mStringValue == TIME_STRING_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* TTGregorian::Clone(void) const
{
   return new TTGregorian(*this);
}


//==============================================================================
//                              TDBModJulian
//==============================================================================

//------------------------------------------------------------------------------
// TDBModJulian(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
TDBModJulian::TDBModJulian(const wxString &name, GmatBase *obj)
   : TimeReal(name, wxT("TDBModJulian"), obj, wxT("TDB Mod. Julian Days"), wxT("day"))
{
}


//------------------------------------------------------------------------------
// TDBModJulian::TDBModJulian(const TDBModJulian &param)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <param> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TDBModJulian::TDBModJulian(const TDBModJulian &param)
   : TimeReal(param)
{
}


//------------------------------------------------------------------------------
// TDBModJulian& TDBModJulian::operator=(const TDBModJulian &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TDBModJulian& TDBModJulian::operator=(const TDBModJulian &right)
{
   if (this != &right)
      TimeReal::operator=(right);
      
   return *this;
}


//------------------------------------------------------------------------------
// TDBModJulian::~TDBModJulian()
//------------------------------------------------------------------------------
TDBModJulian::~TDBModJulian()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool TDBModJulian::Evaluate()
{
   mRealValue = GetCurrentTimeReal(TDB_MJD);
   
   if (mRealValue == TIME_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* TDBModJulian::Clone(void) const
{
   return new TDBModJulian(*this);
}


//==============================================================================
//                              TDBGregorian
//==============================================================================

//------------------------------------------------------------------------------
// TDBGregorian(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
TDBGregorian::TDBGregorian(const wxString &name, GmatBase *obj)
   : TimeString(name, wxT("TDBGregorian"), obj, wxT("TDB Gregorian Date"), wxT(""))
{
}


//------------------------------------------------------------------------------
// TDBGregorian::TDBGregorian(const TDBGregorian &param)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <param> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TDBGregorian::TDBGregorian(const TDBGregorian &param)
   : TimeString(param)
{
}


//------------------------------------------------------------------------------
// TDBGregorian& TDBGregorian::operator=(const TDBGregorian &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TDBGregorian& TDBGregorian::operator=(const TDBGregorian &right)
{
   if (this != &right)
      TimeString::operator=(right);
      
   return *this;
}


//------------------------------------------------------------------------------
// TDBGregorian::~TDBGregorian()
//------------------------------------------------------------------------------
TDBGregorian::~TDBGregorian()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool TDBGregorian::Evaluate()
{
   mStringValue = GetCurrentTimeString(TDB_MJD);
   
   if (mStringValue == TIME_STRING_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* TDBGregorian::Clone(void) const
{
   return new TDBGregorian(*this);
}


//==============================================================================
//                              TCBModJulian
//==============================================================================

//------------------------------------------------------------------------------
// TCBModJulian(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
TCBModJulian::TCBModJulian(const wxString &name, GmatBase *obj)
   : TimeReal(name, wxT("TCBModJulian"), obj, wxT("TCB Mod. Julian Days"), wxT("day"))
{
}


//------------------------------------------------------------------------------
// TCBModJulian::TCBModJulian(const TCBModJulian &param)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <param> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TCBModJulian::TCBModJulian(const TCBModJulian &param)
   : TimeReal(param)
{
}


//------------------------------------------------------------------------------
// TCBModJulian& TCBModJulian::operator=(const TCBModJulian &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TCBModJulian& TCBModJulian::operator=(const TCBModJulian &right)
{
   if (this != &right)
      TimeReal::operator=(right);
      
   return *this;
}


//------------------------------------------------------------------------------
// TCBModJulian::~TCBModJulian()
//------------------------------------------------------------------------------
TCBModJulian::~TCBModJulian()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool TCBModJulian::Evaluate()
{
   mRealValue = GetCurrentTimeReal(TCB_MJD);
   
   if (mRealValue == TIME_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* TCBModJulian::Clone(void) const
{
   return new TCBModJulian(*this);
}


//==============================================================================
//                              TCBGregorian
//==============================================================================

//------------------------------------------------------------------------------
// TCBGregorian(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
TCBGregorian::TCBGregorian(const wxString &name, GmatBase *obj)
   : TimeString(name, wxT("TCBGregorian"), obj, wxT("TCB Gregorian Date"), wxT(""))
{
}


//------------------------------------------------------------------------------
// TCBGregorian::TCBGregorian(const TCBGregorian &param)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <param> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TCBGregorian::TCBGregorian(const TCBGregorian &param)
   : TimeString(param)
{
}


//------------------------------------------------------------------------------
// TCBGregorian& TCBGregorian::operator=(const TCBGregorian &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
TCBGregorian& TCBGregorian::operator=(const TCBGregorian &right)
{
   if (this != &right)
      TimeString::operator=(right);
      
   return *this;
}


//------------------------------------------------------------------------------
// TCBGregorian::~TCBGregorian()
//------------------------------------------------------------------------------
TCBGregorian::~TCBGregorian()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool TCBGregorian::Evaluate()
{
   mStringValue = GetCurrentTimeString(TCB_MJD);
   
   if (mStringValue == TIME_STRING_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* TCBGregorian::Clone(void) const
{
   return new TCBGregorian(*this);
}


//==============================================================================
//                              UTCModJulian
//==============================================================================

//------------------------------------------------------------------------------
// UTCModJulian(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
UTCModJulian::UTCModJulian(const wxString &name, GmatBase *obj)
   : TimeReal(name, wxT("UTCModJulian"), obj, wxT("UTC Mod. Julian Days"), wxT("day"))
{
}


//------------------------------------------------------------------------------
// UTCModJulian::UTCModJulian(const UTCModJulian &param)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <param> the parameter to make copy of
 */
//------------------------------------------------------------------------------
UTCModJulian::UTCModJulian(const UTCModJulian &param)
   : TimeReal(param)
{
}

//------------------------------------------------------------------------------
// UTCModJulian& UTCModJulian::operator=(const UTCModJulian &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
UTCModJulian& UTCModJulian::operator=(const UTCModJulian &right)
{
   if (this != &right)
      TimeReal::operator=(right);
      
   return *this;
}

//------------------------------------------------------------------------------
// UTCModJulian::~UTCModJulian()
//------------------------------------------------------------------------------
UTCModJulian::~UTCModJulian()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool UTCModJulian::Evaluate()
{
   mRealValue = GetCurrentTimeReal(UTC_MJD);
   
   if (mRealValue == TIME_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* UTCModJulian::Clone(void) const
{
   return new UTCModJulian(*this);
}


//==============================================================================
//                              UTCGregorian
//==============================================================================

//------------------------------------------------------------------------------
// UTCGregorian(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
UTCGregorian::UTCGregorian(const wxString &name, GmatBase *obj)
   : TimeString(name, wxT("UTCGregorian"), obj, wxT("UTC Gregorian Date"), wxT(""))
{
}


//------------------------------------------------------------------------------
// UTCGregorian::UTCGregorian(const UTCGregorian &param)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <param> the parameter to make copy of
 */
//------------------------------------------------------------------------------
UTCGregorian::UTCGregorian(const UTCGregorian &param)
   : TimeString(param)
{
}

//------------------------------------------------------------------------------
// UTCGregorian& UTCGregorian::operator=(const UTCGregorian &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
UTCGregorian& UTCGregorian::operator=(const UTCGregorian &right)
{
   if (this != &right)
      TimeString::operator=(right);
      
   return *this;
}

//------------------------------------------------------------------------------
// UTCGregorian::~UTCGregorian()
//------------------------------------------------------------------------------
UTCGregorian::~UTCGregorian()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool UTCGregorian::Evaluate()
{
   mStringValue = GetCurrentTimeString(UTC_MJD);
   
   if (mStringValue == TIME_STRING_UNDEFINED)
      return false;
   else
      return true;
}


//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* UTCGregorian::Clone(void) const
{
   return new UTCGregorian(*this);
}


//==============================================================================
//                              ElapsedDays
//==============================================================================

//---------------------------------
// static data
//---------------------------------
const wxString
ElapsedDays::PARAMETER_TEXT[ElapsedDaysParamCount - ParameterParamCount] =
{
   wxT("InitialEpoch")
};

const Gmat::ParameterType
ElapsedDays::PARAMETER_TYPE[ElapsedDaysParamCount - ParameterParamCount] =
{
   Gmat::REAL_TYPE
};

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// ElapsedDays(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
ElapsedDays::ElapsedDays(const wxString &name, GmatBase *obj)
   : TimeReal(name, wxT("ElapsedDays"), obj, wxT("Elapsed Days"), wxT("Day"))
{
   // GmatBase data
   parameterCount = ElapsedDaysParamCount;
}


//------------------------------------------------------------------------------
// ElapsedDays(const ElapsedDays &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
ElapsedDays::ElapsedDays(const ElapsedDays &copy)
    : TimeReal(copy)
{
}


//------------------------------------------------------------------------------
// const ElapsedDays& operator=(const ElapsedDays &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const ElapsedDays& ElapsedDays::operator=(const ElapsedDays &right)
{
   if (this != &right)
   {
      TimeReal::operator=(right);
   }
   return *this;
}


//------------------------------------------------------------------------------
// ~ElapsedDays()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
ElapsedDays::~ElapsedDays()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool ElapsedDays::Evaluate()
{
   mRealValue = GetElapsedTimeReal(DAYS);
   
   if (mRealValue == TIME_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//--------------------------------------
// Inherited methods from GmatBase
//--------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* ElapsedDays::Clone(void) const
{
   return new ElapsedDays(*this);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Gmat::ParameterType ElapsedDays::GetParameterType(const Integer id) const
{
   if (id >= ParameterParamCount && id < ElapsedDaysParamCount)
      return PARAMETER_TYPE[id - ParameterParamCount];
   else
      return TimeReal::GetParameterType(id);

}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
wxString ElapsedDays::GetParameterTypeString(const Integer id) const
{
   if (id >= ParameterParamCount && id < ElapsedDaysParamCount)
      return GmatBase::PARAM_TYPE_STRING[GetParameterType(id - ParameterParamCount)];
   else
      return TimeReal::GetParameterTypeString(id);

}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
wxString ElapsedDays::GetParameterText(const Integer id) const
{
   if (id >= ParameterParamCount && id < ElapsedDaysParamCount)
      return PARAMETER_TEXT[id - ParameterParamCount];
   else
      return TimeReal::GetParameterText(id);
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <id> parameter index.
 *
 * @return true if the parameter is read only, false (the default) if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool ElapsedDays::IsParameterReadOnly(const Integer id) const
{
   if (id == INITIAL_EPOCH)
      return true;
   
   return TimeReal::IsParameterReadOnly(id);
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Integer ElapsedDays::GetParameterID(const wxString &str) const
{
   for (int i = ParameterParamCount; i < ElapsedDaysParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - ParameterParamCount])
         return i;
   }
   
   return TimeReal::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedDays::GetRealParameter(const Integer id) const
{
   switch (id)
   {
   case INITIAL_EPOCH:
       return GetInitialEpoch();
   default:
       return TimeReal::GetRealParameter(id);
   }
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedDays::GetRealParameter(const wxString &label) const
{
   if (label == wxT("InitialEpoch"))
      return GetInitialEpoch();
   else
      return TimeReal::GetRealParameter(label);
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedDays::SetRealParameter(const Integer id, const Real value)
{
   switch (id)
   {
   case INITIAL_EPOCH:
      SetInitialEpoch(value);
      return value;
   default:
      return TimeReal::SetRealParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const std:string &label, const Real value)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedDays::SetRealParameter(const wxString &label, const Real value)
{
   if (label == wxT("InitialEpoch"))
   {
      SetInitialEpoch(value);
      return value;
   }
   else
   {
      return TimeReal::SetRealParameter(label, value);
   }
}


//==============================================================================
//                              ElapsedDaysFromStart
//==============================================================================

//---------------------------------
// static data
//---------------------------------
const wxString
ElapsedDaysFromStart::PARAMETER_TEXT[ElapsedDaysFromStartParamCount - ParameterParamCount] =
{
   wxT("InitialEpoch")
};

const Gmat::ParameterType
ElapsedDaysFromStart::PARAMETER_TYPE[ElapsedDaysFromStartParamCount - ParameterParamCount] =
{
   Gmat::REAL_TYPE
};

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// ElapsedDaysFromStart(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
ElapsedDaysFromStart::ElapsedDaysFromStart(const wxString &name, GmatBase *obj)
   : TimeReal(name, wxT("ElapsedDaysFromStart"), obj, wxT("Elapsed Days"), wxT("Day"))
{
   // GmatBase data
   parameterCount = ElapsedDaysFromStartParamCount;
}


//------------------------------------------------------------------------------
// ElapsedDaysFromStart(const ElapsedDaysFromStart &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
ElapsedDaysFromStart::ElapsedDaysFromStart(const ElapsedDaysFromStart &copy)
    : TimeReal(copy)
{
}


//------------------------------------------------------------------------------
// const ElapsedDaysFromStart& operator=(const ElapsedDaysFromStart &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const ElapsedDaysFromStart& ElapsedDaysFromStart::operator=(const ElapsedDaysFromStart &right)
{
   if (this != &right)
   {
      TimeReal::operator=(right);
   }
   return *this;
}


//------------------------------------------------------------------------------
// ~ElapsedDaysFromStart()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
ElapsedDaysFromStart::~ElapsedDaysFromStart()
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
 * @return true if parameter value successfully evaluated.
 */
//------------------------------------------------------------------------------
bool ElapsedDaysFromStart::Evaluate()
{
   mRealValue = GetElapsedTimeReal(DAYS);
   
   if (mRealValue == TIME_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//--------------------------------------
// Inherited methods from GmatBase
//--------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* ElapsedDaysFromStart::Clone(void) const
{
   return new ElapsedDaysFromStart(*this);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Gmat::ParameterType ElapsedDaysFromStart::GetParameterType(const Integer id) const
{
   if (id >= ParameterParamCount && id < ElapsedDaysFromStartParamCount)
      return PARAMETER_TYPE[id - ParameterParamCount];
   else
      return TimeReal::GetParameterType(id);

}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
wxString ElapsedDaysFromStart::GetParameterTypeString(const Integer id) const
{
   if (id >= ParameterParamCount && id < ElapsedDaysFromStartParamCount)
      return GmatBase::PARAM_TYPE_STRING[GetParameterType(id - ParameterParamCount)];
   else
      return TimeReal::GetParameterTypeString(id);

}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
wxString ElapsedDaysFromStart::GetParameterText(const Integer id) const
{
   if (id >= ParameterParamCount && id < ElapsedDaysFromStartParamCount)
      return PARAMETER_TEXT[id - ParameterParamCount];
   else
      return TimeReal::GetParameterText(id);
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <id> parameter index.
 *
 * @return true if the parameter is read only, false (the default) if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool ElapsedDaysFromStart::IsParameterReadOnly(const Integer id) const
{
   if (id == INITIAL_EPOCH)
      return true;
   
   return TimeReal::IsParameterReadOnly(id);
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Integer ElapsedDaysFromStart::GetParameterID(const wxString &str) const
{
   for (int i = ParameterParamCount; i < ElapsedDaysFromStartParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - ParameterParamCount])
         return i;
   }
   
   return TimeReal::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedDaysFromStart::GetRealParameter(const Integer id) const
{
   switch (id)
   {
   case INITIAL_EPOCH:
       return GetInitialEpoch();
   default:
       return TimeReal::GetRealParameter(id);
   }
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedDaysFromStart::GetRealParameter(const wxString &label) const
{
   if (label == wxT("InitialEpoch"))
      return GetInitialEpoch();
   else
      return TimeReal::GetRealParameter(label);
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedDaysFromStart::SetRealParameter(const Integer id, const Real value)
{
   switch (id)
   {
   case INITIAL_EPOCH:
      SetInitialEpoch(value);
      return value;
   default:
      return TimeReal::SetRealParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const std:string &label, const Real value)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedDaysFromStart::SetRealParameter(const wxString &label, const Real value)
{
   if (label == wxT("InitialEpoch"))
   {
      SetInitialEpoch(value);
      return value;
   }
   else
   {
      return TimeReal::SetRealParameter(label, value);
   }
}


//==============================================================================
//                              ElapsedSecs
//==============================================================================

//---------------------------------
// static data
//---------------------------------
const wxString
ElapsedSecs::PARAMETER_TEXT[ElapsedSecsParamCount - ParameterParamCount] =
{
   wxT("InitialEpoch")
};

const Gmat::ParameterType
ElapsedSecs::PARAMETER_TYPE[ElapsedSecsParamCount - ParameterParamCount] =
{
   Gmat::REAL_TYPE
};


//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// ElapsedSecs(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
ElapsedSecs::ElapsedSecs(const wxString &name, GmatBase *obj)
   : TimeReal(name, wxT("ElapsedSecs"), obj, wxT("Elapsed Seconds"), wxT("s"))
{
   // GmatBase data
   parameterCount = ElapsedSecsParamCount;
}


//------------------------------------------------------------------------------
// ElapsedSecs(const ElapsedSecs &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
ElapsedSecs::ElapsedSecs(const ElapsedSecs &copy)
   : TimeReal(copy)
{
}


//------------------------------------------------------------------------------
// const ElapsedSecs& operator=(const ElapsedSecs &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const ElapsedSecs& ElapsedSecs::operator=(const ElapsedSecs &right)
{
   if (this != &right)
   {
      TimeReal::operator=(right);
   }
   return *this;
}


//------------------------------------------------------------------------------
// ~ElapsedSecs()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
ElapsedSecs::~ElapsedSecs()
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
bool ElapsedSecs::Evaluate()
{
   mRealValue = GetElapsedTimeReal(SECS);
   
   if (mRealValue == TIME_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//--------------------------------------
// Inherited methods from GmatBase
//--------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* ElapsedSecs::Clone(void) const
{
   return new ElapsedSecs(*this);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Gmat::ParameterType ElapsedSecs::GetParameterType(const Integer id) const
{
   if (id >= ParameterParamCount && id < ElapsedSecsParamCount)
      return PARAMETER_TYPE[id - ParameterParamCount];
   else
      return TimeReal::GetParameterType(id);

}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
wxString ElapsedSecs::GetParameterTypeString(const Integer id) const
{
   if (id >= ParameterParamCount && id < ElapsedSecsParamCount)
      return GmatBase::PARAM_TYPE_STRING[GetParameterType(id - ParameterParamCount)];
   else
      return TimeReal::GetParameterTypeString(id);

}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
wxString ElapsedSecs::GetParameterText(const Integer id) const
{
   if (id >= ParameterParamCount && id < ElapsedSecsParamCount)
      return PARAMETER_TEXT[id - ParameterParamCount];
   else
      return TimeReal::GetParameterText(id);
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <id> parameter index.
 *
 * @return true if the parameter is read only, false (the default) if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool ElapsedSecs::IsParameterReadOnly(const Integer id) const
{
   if (id == INITIAL_EPOCH)
      return true;
   
   return TimeReal::IsParameterReadOnly(id);
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Integer ElapsedSecs::GetParameterID(const wxString &str) const
{
   for (int i = ParameterParamCount; i < ElapsedSecsParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - ParameterParamCount])
         return i;
   }
   
   return TimeReal::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedSecs::GetRealParameter(const Integer id) const
{
   switch (id)
   {
   case INITIAL_EPOCH:
      return GetInitialEpoch();
   default:
      return TimeReal::GetRealParameter(id);
   }
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedSecs::GetRealParameter(const wxString &label) const
{
   if (label == wxT("InitialEpoch"))
      return GetInitialEpoch();
   else
      return TimeReal::GetRealParameter(label);
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedSecs::SetRealParameter(const Integer id, const Real value)
{
   switch (id)
   {
   case INITIAL_EPOCH:
      SetInitialEpoch(value);
      return value;
   default:
      return TimeReal::SetRealParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const std:string &label, const Real value)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedSecs::SetRealParameter(const wxString &label, const Real value)
{
   if (label == wxT("InitialEpoch"))
   {
      SetInitialEpoch(value);
      return value;
   }
   else
   {
      return TimeReal::SetRealParameter(label, value);
   }
}


//==============================================================================
//                              ElapsedSecsFromStart
//==============================================================================

//---------------------------------
// static data
//---------------------------------
const wxString
ElapsedSecsFromStart::PARAMETER_TEXT[ElapsedSecsFromStartParamCount - ParameterParamCount] =
{
   wxT("InitialEpoch")
};

const Gmat::ParameterType
ElapsedSecsFromStart::PARAMETER_TYPE[ElapsedSecsFromStartParamCount - ParameterParamCount] =
{
   Gmat::REAL_TYPE
};

//---------------------------------
// public methods
//---------------------------------

//------------------------------------------------------------------------------
// ElapsedSecsFromStart(const wxString &name, GmatBase *obj)
//------------------------------------------------------------------------------
/**
 * Constructor.
 *
 * @param <name> name of the parameter
 * @param <obj> reference object pointer
 */
//------------------------------------------------------------------------------
ElapsedSecsFromStart::ElapsedSecsFromStart(const wxString &name, GmatBase *obj)
   : TimeReal(name, wxT("ElapsedSecsFromStart"), obj, wxT("Elapsed Seconds"), wxT("s"))
{
   // GmatBase data
   parameterCount = ElapsedSecsFromStartParamCount;
}


//------------------------------------------------------------------------------
// ElapsedSecsFromStart(const ElapsedSecsFromStart &copy)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * @param <copy> the parameter to make copy of
 */
//------------------------------------------------------------------------------
ElapsedSecsFromStart::ElapsedSecsFromStart(const ElapsedSecsFromStart &copy)
   : TimeReal(copy)
{
}


//------------------------------------------------------------------------------
// const ElapsedSecsFromStart& operator=(const ElapsedSecsFromStart &right)
//------------------------------------------------------------------------------
/**
 * Assignment operator.
 *
 * @param <right> the parameter to make copy of
 */
//------------------------------------------------------------------------------
const ElapsedSecsFromStart& ElapsedSecsFromStart::operator=(const ElapsedSecsFromStart &right)
{
   if (this != &right)
   {
      TimeReal::operator=(right);
   }
   return *this;
}


//------------------------------------------------------------------------------
// ~ElapsedSecsFromStart()
//------------------------------------------------------------------------------
/**
 * Destructor.
 */
//------------------------------------------------------------------------------
ElapsedSecsFromStart::~ElapsedSecsFromStart()
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
bool ElapsedSecsFromStart::Evaluate()
{
   mRealValue = GetElapsedTimeReal(SECS);
   
   if (mRealValue == TIME_REAL_UNDEFINED)
      return false;
   else
      return true;
}


//--------------------------------------
// Inherited methods from GmatBase
//--------------------------------------

//------------------------------------------------------------------------------
// virtual GmatBase* Clone(void) const
//------------------------------------------------------------------------------
/**
 * Method used to create a copy of the object
 */
//------------------------------------------------------------------------------
GmatBase* ElapsedSecsFromStart::Clone(void) const
{
   return new ElapsedSecsFromStart(*this);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Gmat::ParameterType ElapsedSecsFromStart::GetParameterType(const Integer id) const
{
   if (id >= ParameterParamCount && id < ElapsedSecsFromStartParamCount)
      return PARAMETER_TYPE[id - ParameterParamCount];
   else
      return TimeReal::GetParameterType(id);

}


//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
wxString ElapsedSecsFromStart::GetParameterTypeString(const Integer id) const
{
   if (id >= ParameterParamCount && id < ElapsedSecsFromStartParamCount)
      return GmatBase::PARAM_TYPE_STRING[GetParameterType(id - ParameterParamCount)];
   else
      return TimeReal::GetParameterTypeString(id);

}


//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
wxString ElapsedSecsFromStart::GetParameterText(const Integer id) const
{
   if (id >= ParameterParamCount && id < ElapsedSecsFromStartParamCount)
      return PARAMETER_TEXT[id - ParameterParamCount];
   else
      return TimeReal::GetParameterText(id);
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <id> parameter index.
 *
 * @return true if the parameter is read only, false (the default) if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool ElapsedSecsFromStart::IsParameterReadOnly(const Integer id) const
{
   if (id == INITIAL_EPOCH)
      return true;
   
   return TimeReal::IsParameterReadOnly(id);
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Integer ElapsedSecsFromStart::GetParameterID(const wxString &str) const
{
   for (int i = ParameterParamCount; i < ElapsedSecsFromStartParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - ParameterParamCount])
         return i;
   }
   
   return TimeReal::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedSecsFromStart::GetRealParameter(const Integer id) const
{
   switch (id)
   {
   case INITIAL_EPOCH:
      return GetInitialEpoch();
   default:
      return TimeReal::GetRealParameter(id);
   }
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const wxString &label) const
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedSecsFromStart::GetRealParameter(const wxString &label) const
{
   if (label == wxT("InitialEpoch"))
      return GetInitialEpoch();
   else
      return TimeReal::GetRealParameter(label);
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedSecsFromStart::SetRealParameter(const Integer id, const Real value)
{
   switch (id)
   {
   case INITIAL_EPOCH:
      SetInitialEpoch(value);
      return value;
   default:
      return TimeReal::SetRealParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const std:string &label, const Real value)
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
Real ElapsedSecsFromStart::SetRealParameter(const wxString &label, const Real value)
{
   if (label == wxT("InitialEpoch"))
   {
      SetInitialEpoch(value);
      return value;
   }
   else
   {
      return TimeReal::SetRealParameter(label, value);
   }
}



