//$Id: PenUp.cpp 9826 2011-08-31 22:16:42Z lindajun $
//------------------------------------------------------------------------------
//                                 PenUp
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Darrel J. Conway
// Created: 2004/02/26
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
/**
 * Class implementation for the PenUp command
 */
//------------------------------------------------------------------------------


#include "PenUp.hpp"
#include "MessageInterface.hpp"
#include "StringUtil.hpp"

//#define DEBUG_PENUP


//------------------------------------------------------------------------------
// PenUp()
//------------------------------------------------------------------------------
/**
 * Constructor
 */
//------------------------------------------------------------------------------
PenUp::PenUp() :
   GmatCommand    (wxT("PenUp"))//,
   //plotName       (""),
   //thePlot        (NULL)
{
   plotNameList.clear();
   thePlotList.clear();
}


//------------------------------------------------------------------------------
// ~PenUp()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
PenUp::~PenUp()
{
   plotNameList.clear();
   thePlotList.clear();
}


//------------------------------------------------------------------------------
// PenUp(const PenUp &c)
//------------------------------------------------------------------------------
/**
 * Copy constructor.
 * 
 * @param <c> The instance that gets copied.
 */
//------------------------------------------------------------------------------
PenUp::PenUp(const PenUp &c) :
   GmatCommand    (c),
   plotNameList   (c.plotNameList),
   thePlotList    (c.thePlotList)
   //plotName       (c.plotName),
   //thePlot        (NULL)
{
}


//------------------------------------------------------------------------------
// PenUp& operator=(const PenUp &c)
//------------------------------------------------------------------------------
/**
 * Assignment operator
 * 
 * @param <c> The command that gets copied.
 * 
 * @return A reference to this instance.
 */
//------------------------------------------------------------------------------
PenUp& PenUp::operator=(const PenUp &c)
{
   if (&c != this)
   {
      plotNameList = c.plotNameList;
      thePlotList.clear();
      //plotName = c.plotName;
      //thePlot = NULL;
   }
   
   return *this;
}


//------------------------------------------------------------------------------
// GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * Override of the GmatBase clone method.
 * 
 * @return A new copy of this instance.
 */
//------------------------------------------------------------------------------
GmatBase* PenUp::Clone() const
{
   return new PenUp(*this);
}

//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by the Achieve.
 *
 * @return the list of object types.
 * 
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& PenUp::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::SUBSCRIBER);
   return refObjectTypes;
}



//------------------------------------------------------------------------------
// const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Accesses arrays of names for referenced objects.
 * 
 * @param type Type of object requested.
 * 
 * @return the StringArray containing the referenced object names.
 */
//------------------------------------------------------------------------------
const StringArray& PenUp::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   // There are only subscribers, so ignore object type
   return plotNameList;
}


//------------------------------------------------------------------------------
// bool InterpretAction()
//------------------------------------------------------------------------------
bool PenUp::InterpretAction()
{
   plotNameList.clear();
   thePlotList.clear();
   
   Integer loc = generatingString.find(wxT("PenUp"), 0) + 5; //, end;
   wxString str = generatingString;
   while (str[loc] == ' ')
      ++loc;

   // this command, for compatability with MATLAB, should not have
   // parentheses (except to indicate array elements), brackets, or braces
   if (!GmatStringUtil::HasNoBrackets(generatingString, false))
   {
      wxString msg = 
         wxT("The PenUp command is not allowed to contain brackets, braces, or ")
         wxT("parentheses");
      throw CommandException(msg);
   }

   // Find the Subscriber list
   //end = generatingString.find(" ", loc);
   //plotName = generatingString.substr(loc, end-loc);
   //plotNameList.push_back(plotName);
   wxString sub = generatingString.substr(loc, generatingString.size()-loc);
   StringArray parts = GmatStringUtil::SeparateBy(sub,wxT(" "), false);
   Integer partsSz = (Integer) parts.size();
   #ifdef DEBUG_PENUP
      MessageInterface::ShowMessage(wxT("In PenUp::InterpretAction, parts = \n"));
      for (Integer jj = 0; jj < partsSz; jj++)
         MessageInterface::ShowMessage(wxT("   %s\n"), parts.at(jj).c_str());
   #endif
   if (partsSz < 1) // 'PenUp' already found
      throw CommandException(wxT("Missing field in PenUp command"));
   for (Integer ii = 0; ii < partsSz; ii++)
      plotNameList.push_back(parts.at(ii));
   
   #ifdef DEBUG_PENUP
      MessageInterface::ShowMessage(wxT("Plots to be PenUped:\n"));
      for (unsigned int ii = 0; ii < plotNameList.size(); ii++)
         MessageInterface::ShowMessage(wxT("   %s\n"), (plotNameList.at(ii)).c_str());
   #endif

   return true;
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 * Method that initializes the internal data structures.
 * 
 * @return true if initialization succeeds.
 */
//------------------------------------------------------------------------------
bool PenUp::Initialize()
{
   #ifdef DEBUG_PENUP
      MessageInterface::ShowMessage(wxT("PenUp::Initialize() entered\n"));
   #endif
      
   GmatCommand::Initialize();
   
   GmatBase *sub;
   thePlotList.clear();
   
   for (unsigned int ii = 0; ii < plotNameList.size(); ii++)
   {
      if ((sub = FindObject(plotNameList.at(ii))) != NULL) 
      {
         if (sub->GetTypeName() == wxT("XYPlot") ||
             sub->GetTypeName() == wxT("OrbitView") ||
             sub->GetTypeName() == wxT("GroundTrackPlot"))
            //thePlotList.push_back((XyPlot*) sub);
            thePlotList.push_back((Subscriber*) sub);
            //thePlot = (XyPlot*)sub;
         else
            throw CommandException(
               wxT("Object named \"") + plotNameList.at(ii) +
               wxT("\" should be an XYPlot, OrbitView or GroundTrackPlot to use the ")
               wxT("PenUp command for this object, but it is a ") + 
               sub->GetTypeName());      
      }
      else 
      {
         MessageInterface::ShowMessage
            (wxT("PenUp command cannot find Plot \"%s\"; command has no effect.")
            wxT("\n"), (plotNameList.at(ii)).c_str());
         return false;
      }
   }

   return true;
}


//---------------------------------------------------------------------------
//  bool GmatCommand::Execute()
//---------------------------------------------------------------------------
/**
 * The method that is fired to perform the GmatCommand.
 *
 * Derived classes implement this method to perform their actions on
 * GMAT objects.
 *
 * @return true if the GmatCommand runs to completion, false if an error
 *         occurs.
 */
//---------------------------------------------------------------------------
bool PenUp::Execute()
{
   for (unsigned int ii = 0; ii < thePlotList.size(); ii++)
   {
      if (thePlotList.at(ii))
         if (!(thePlotList.at(ii)->TakeAction(wxT("PenUp")))) return false;
   }
   //if (thePlot)
   //   thePlot->TakeAction("PenUp");
   return true;
}
