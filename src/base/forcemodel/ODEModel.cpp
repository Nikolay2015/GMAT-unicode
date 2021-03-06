//$Id: ODEModel.cpp 9973 2011-10-24 23:19:47Z djcinsb $
//------------------------------------------------------------------------------
//                              ODEModel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// *** File Name : ODEModel.cpp
// *** Created   : October 1, 2002
// ***************************************************************************
// ***  Developed By  :  Thinking Systems, Inc. (www.thinksysinc.com)      ***
// ***  For:  Flight Dynamics Analysis Branch (Code 572)                   ***
// ***  Under Contract:  P.O.  GSFC S-66617-G                              ***
// ***                                                                     ***
// ***  This software is subject to the Software Usage Agreement described ***
// ***  by NASA Case Number GSC-14735-1.  The Software Usage Agreement     ***
// ***  must be included in any distribution.  Removal of this header is   ***
// ***  strictly prohibited.                                               ***
// ***                                                                     ***
// ***                                                                     ***
// ***  Header Version: July 12, 2002                                      ***
// ***************************************************************************
// Module Type               : ANSI C++ Source
// Development Environment   : Visual C++ 7.0
// Modification History      : 11/26/2002 - D. Conway, Thinking Systems, Inc.
//                             Original delivery
//
//                           : 2/28/2003 - D. Conway, Thinking Systems, Inc.
//                             Filled in some code to model cases with 
//                             discontinuous forces (e.g. SRP)
//
//                           : 10/01/2003 - W. Waktola, Missions Applications
//                                                      Branch
//                              Changes:
//                                - Updated style using GMAT cpp style guide
//                                - Changed FlightDynamicsForces class to
//                                  ODEModel class
//
//                           : 10/20/2003 - W. Waktola, Missions Applications
//                                                      Branch
//                              Changes:
//                                - All double types to Real types
//                                - All primitive int types to Integer types
//
//                           : 11/9/2003 - D. Conway, Thinking Systems, Inc.
//                             Overrode GetParameterCount so the count 
//                             increases based on the member forces
// ***************************************************************************

#include "ODEModel.hpp"
#include "Formation.hpp"
#include "MessageInterface.hpp"
#include "PropagationStateManager.hpp"
#include "TimeTypes.hpp"

#include "GravityField.hpp"
//#include wxT("PointMassForce.hpp")
//#include wxT("Formation.hpp")      // for BuildState()

#include <string.h> 
#include <algorithm>    // for find()


//#define DEBUG_ODEMODEL
//#define DEBUG_ODEMODEL_INIT
//#define DEBUG_ODEMODEL_EXE
//#define DEBUG_INITIALIZATION
//#define DEBUG_FORCE_REF_OBJ
//#define DEBUG_ODEMODEL_EPOCHS
//#define DEBUG_SATELLITE_PARAMETERS
//#define DEBUG_FIRST_CALL
//#define DEBUG_GEN_STRING
//#define DEBUG_OWNED_OBJECT_STRINGS
//#define DEBUG_INITIALIZATION
//#define DEBUG_BUILDING_MODELS
//#define DEBUG_STATE
//#define DEBUG_MASS_FLOW
//#define DEBUG_REORIGIN
//#define DEBUG_ERROR_ESTIMATE
//#define DUMP_TOTAL_DERIVATIVE
//#define DEBUG_STM_AMATRIX_DERIVS
//#define DEBUG_MU_MAP


//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//---------------------------------
// static data
//---------------------------------

#ifdef DEBUG_FIRST_CALL
static bool firstCallFired = false;
#endif

const wxString
ODEModel::PARAMETER_TEXT[ODEModelParamCount - PhysicalModelParamCount] =
{
   wxT("CentralBody"),
   wxT("PrimaryBodies"),
   wxT("PointMasses"),
   wxT("Drag"),
   wxT("SRP"),
   wxT("RelativisticCorrection"),
   wxT("ErrorControl"),
   wxT("CoordinateSystemList"),
   
   // owned object parameters
   wxT("Degree"),
   wxT("Order"),
   wxT("PotentialFile"),
   wxT("UserDefined")
};


const Gmat::ParameterType
ODEModel::PARAMETER_TYPE[ODEModelParamCount - PhysicalModelParamCount] =
{
   Gmat::OBJECT_TYPE,       // wxT("CentralBody"),
   Gmat::OBJECTARRAY_TYPE,  // wxT("PrimaryBodies"),
   Gmat::OBJECTARRAY_TYPE,  // wxT("PointMasses"),
   Gmat::OBJECT_TYPE,       // wxT("Drag"),
   Gmat::ON_OFF_TYPE,       // wxT("SRP"),
   Gmat::ON_OFF_TYPE,       // wxT("RelativisticCorrection"),
   Gmat::ENUMERATION_TYPE,  // wxT("ErrorControl"),
   Gmat::OBJECTARRAY_TYPE,  // wxT("CoordinateSystemList")
   
   // owned object parameters
   Gmat::INTEGER_TYPE,      // wxT("Degree"),
   Gmat::INTEGER_TYPE,      // wxT("Order"),
   Gmat::STRING_TYPE,       // wxT("PotentialFile"),
   Gmat::OBJECTARRAY_TYPE,  // wxT("UserDefined"),
};


// Table of alternative words used in force model scripting
std::map<wxString, wxString> ODEModel::scriptAliases;

//--------------------------------------------------------------------------------
// static methods
//--------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// void SetScriptAlias(const wxString& alias, const wxString& typeName)
//------------------------------------------------------------------------------
/**
 * Sets mapping between script descriptions of forces and their class names.
 *
 * The GMAT script document specifies descriptors for some forces that is
 * different from the actual class names, and in some cases actually uses more
 * than one name for the same force.  This method is used to build the map
 * between the script strings and the actual class names.  The constructor for
 * the Interpreter base class fills in this data (at least for now -- we might
 * move it to the Moderator's initialization once it is working and tested).
 *
 * @param alias script string used for the force.
 * @param typeName actual class name as used by the factory that creates it.
 *
 * @todo Put the initialization for force aliases in a convenient location.
 */
//------------------------------------------------------------------------------
void ODEModel::SetScriptAlias(const wxString& alias,
                                const wxString& typeName)
{
   if (scriptAliases.find(alias) == scriptAliases.end())
   {
      scriptAliases[alias] = typeName;
   }
}


//------------------------------------------------------------------------------
// wxString& GetScriptAlias(const wxString& alias)
//------------------------------------------------------------------------------
/**
 * Accesses mapping between script descriptions of forces and their class names.
 *
 * This method provides access to class names given an alias in the script.
 *
 * @param alias script string used for the force.
 *
 * @return The class name.
 */
//------------------------------------------------------------------------------
wxString& ODEModel::GetScriptAlias(const wxString& alias)
{
   static wxString type;
   type = alias;
   if (scriptAliases.find(alias) != scriptAliases.end())
   {
      type = scriptAliases[alias];
   }
   return type;
}

//---------------------------------
// public methods
//---------------------------------


//------------------------------------------------------------------------------
// ODEModel::ODEModel(Gmat::ObjectType id, const wxString &typeStr,
//                        const wxString &nomme))
//------------------------------------------------------------------------------
/**
 * The constructor
 *
 * @param modelName  Scripted name for the model
 * @param typeName   Type of model being built
 */
//------------------------------------------------------------------------------
ODEModel::ODEModel(const wxString &modelName, const wxString typeName) :
   PhysicalModel     (Gmat::ODE_MODEL, typeName, modelName),
//   previousState     (NULL),
   state             (NULL),
   psm               (NULL),
   estimationMethod  (ESTIMATE_STEP),     // Should this be removed?
   normType          (L2_DIFFERENCES),
   parametersSetOnce (false),
   centralBodyName   (wxT("Earth")),
   forceMembersNotInitialized (true),
   satCount          (0),
   stateStart        (-1),
   stateEnd          (-1),
   cartStateSize     (0),
   dynamicProperties (false),
   j2kBodyName       (wxT("Earth")),
   j2kBody           (NULL),
   earthEq           (NULL),
   earthFixed        (NULL),
   transientCount    (0)
{
   satIds[0] = satIds[1] = satIds[2] = satIds[3] = satIds[4] = 
   satIds[5] = satIds[6] = -1;
   
   objectTypes.push_back(Gmat::ODE_MODEL);
   objectTypeNames.push_back(wxT("ODEModel"));
   objectTypeNames.push_back(wxT("ForceModel")); // For backwards compatibility

   numForces = 0;
   stateSize = 6;
   dimension = 6;
   currentForce = 0;
   parameterCount = ODEModelParamCount;

   muMap.clear();
}


//------------------------------------------------------------------------------
// ODEModel::~ODEModel()
//------------------------------------------------------------------------------
/**
 * The destructor
 *
 * The destructor frees the memory used by the owned PhysicalModel instances.
 */
//------------------------------------------------------------------------------
ODEModel::~ODEModel()
{
   #ifdef DEBUG_ODEMODEL
   MessageInterface::ShowMessage
      (wxT("ODEModel destructor entered, this=<%p>'%s', has %d forces\n"),
       this, GetName().c_str(), forceList.size());
   #endif
   
//   if (previousState)
//      delete [] previousState;
   
   // Delete the owned objects
   ClearForceList();
   ClearInternalCoordinateSystems();

   muMap.clear();

   #ifdef DEBUG_FORCE_EPOCHS
      if (epochFile.is_open())
         epochFile.close();
   #endif
      
   #ifdef DEBUG_ODEMODEL
   MessageInterface::ShowMessage(wxT("ODEModel destructor exiting, has %d forces\n"),
                                 forceList.size());
   #endif
}

//------------------------------------------------------------------------------
// ODEModel::ODEModel(const ODEModel& fdf)
//------------------------------------------------------------------------------
/**
 * Copy constructor
 *
 * Copies the ODEModel along with all of the owned PhysicalModels in the model.
 * This code depends on the copy constructors in the owned models; errors in
 * those pieces will impact the owned models in this code.
 *
 * @param fdf   The original of the ODEModel that is copied
 */
//------------------------------------------------------------------------------
ODEModel::ODEModel(const ODEModel& fdf) :
   PhysicalModel              (fdf),
//   previousState              (fdf.previousState),
   state                      (NULL),
   psm                        (NULL),
   estimationMethod           (fdf.estimationMethod),
   normType                   (fdf.normType),
   parametersSetOnce          (false),
   centralBodyName            (fdf.centralBodyName),
   forceMembersNotInitialized (true),
   satCount                   (0),
   stateStart                 (fdf.stateStart),
   stateEnd                   (fdf.stateEnd),
   cartStateSize              (0),
   dynamicProperties          (false),
   j2kBodyName                (fdf.j2kBodyName),
   /// @note: Since the next three are global objects or reset by the Sandbox, 
   ///assignment works
   j2kBody                    (fdf.j2kBody),
   earthEq                    (fdf.earthEq),
   earthFixed                 (fdf.earthFixed),
   transientCount             (fdf.transientCount)
{
   #ifdef DEBUG_ODEMODEL
   MessageInterface::ShowMessage(wxT("ODEModel copy constructor entered\n"));
   #endif
   
   satIds[0] = satIds[1] = satIds[2] = satIds[3] = satIds[4] = 
   satIds[5] = satIds[6] = -1;

   numForces           = fdf.numForces;
   stateSize           = fdf.stateSize;
   dimension           = fdf.dimension;
   currentForce        = fdf.currentForce;
   forceTypeNames      = fdf.forceTypeNames;
//   previousTime        = fdf.previousTime;
   transientForceNames = fdf.transientForceNames;
   forceReferenceNames = fdf.forceReferenceNames;
   
   parameterCount = ODEModelParamCount;
   
//   spacecraft.clear();
   forceList.clear();
   internalCoordinateSystems.clear();
   muMap.clear();
   
   // Copy the forces.  May not work -- the copy constructors need to be checked
   for (std::vector<PhysicalModel *>::const_iterator pm = fdf.forceList.begin();
        pm != fdf.forceList.end(); ++pm)
   {
      #ifdef DEBUG_ODEMODEL
      GmatBase *obj = (*pm);
      MessageInterface::ShowMessage
         (wxT("   Cloning PhysicalModel <%p><%s>'%s'\n"), obj, obj->GetTypeName().c_str(),
          obj->GetName().c_str());
      #endif
      PhysicalModel *newPm = (PhysicalModel*)(*pm)->Clone();
      forceList.push_back(newPm);
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Add
         (newPm, newPm->GetName(), wxT("ODEModel::ODEModel(copy)"),
          wxT("*newPm = (*pm)->Clone()"), this);
      #endif
   }
}


//------------------------------------------------------------------------------
// ODEModel& ODEModel::operator=(const ODEModel& fdf)
//------------------------------------------------------------------------------
/**
 * The assignment operator.
 * 
 * NOTE: The ODEModel assignment operator is not yet tested.  This method 
 *       should be validated before the class is used in external code.
 * 
 * @param fdf   The original of the ODEModel that are copied
 */
//------------------------------------------------------------------------------
ODEModel& ODEModel::operator=(const ODEModel& fdf)
{
   if (&fdf == this)
        return *this;

   PhysicalModel::operator=(fdf);

   satIds[0] = satIds[1] = satIds[2] = satIds[3] = satIds[4] = 
   satIds[5] = satIds[6] = -1;
   
   state = NULL;
   psm   = NULL;
   satCount = 0;
   stateStart = fdf.stateStart;
   stateEnd   = fdf.stateEnd;

   cartStateSize       = 0;
   dynamicProperties   = false;

   numForces           = fdf.numForces;
   stateSize           = fdf.stateSize;
   dimension           = fdf.dimension;
   currentForce        = fdf.currentForce;
   forceTypeNames      = fdf.forceTypeNames;
//   previousTime        = fdf.previousTime;
//   previousState       = fdf.previousState;
   estimationMethod    = fdf.estimationMethod;
   normType            = fdf.normType;
   transientForceNames = fdf.transientForceNames;
   forceReferenceNames = fdf.forceReferenceNames;
   parametersSetOnce   = false;
//   modelEpochId        = -1;
   
   parameterCount      = ODEModelParamCount;
   centralBodyName     = fdf.centralBodyName;
   j2kBodyName         = fdf.j2kBodyName;
   
   /// @note: Since the next three are global objects or reset by the Sandbox, 
   ///assignment works
   j2kBody             = fdf.j2kBody;
   earthEq             = fdf.earthEq;
   earthFixed          = fdf.earthFixed; 
   forceMembersNotInitialized = fdf.forceMembersNotInitialized;
   transientCount      = fdf.transientCount;

   // Clear owned objects before clone
   ClearForceList();
   ClearInternalCoordinateSystems();
   
//   spacecraft.clear();
   forceList.clear();
   
   // Copy the forces.  May not work -- the copy constructors need to be checked
   for (std::vector<PhysicalModel *>::const_iterator pm = fdf.forceList.begin();
        pm != fdf.forceList.end(); ++pm)
   {
      PhysicalModel *newPm = (PhysicalModel*)(*pm)->Clone();
      forceList.push_back(newPm);
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Add
         (newPm, newPm->GetName(), wxT("ODEModel::operator="),
          wxT("*newPm = (*pm)->Clone()"), this);
      #endif
   }
   muMap = fdf.muMap;
   
   return *this;
}

//------------------------------------------------------------------------------
// void ODEModel::AddForce(PhysicalModel *pPhysicalModel)
//------------------------------------------------------------------------------
/**
 * Method used to add a new force to the force model
 *
 * This method takes the pointer to the new force and adds it to the force model
 * list for later use.  Each force should supply either first derivative 
 * information for elements 4 through 6 of a state vector and zeros for the 
 * first three elements, or second derivative information in elements 1 through 
 * 3 and zeroes in 4 through 6 for second order integrators.  The forces should 
 * have the ability to act on state vectors for formations as well, by filling 
 * in elements (6*n+4) through (6*n+6) for larger state vectors.  Some forces 
 * also affect the mass properties of the spacecraft; these elements are updated 
 * using a TBD data structure.
 * 
 * The force that is passed in is owned by this class, and should not be 
 * destroyed externally.  In addition, every force that is passed to this class
 * needs to have a copy constructor and an assignment operator so that it can be 
 * cloned for distribution to multiple sandboxes.
 * 
 * @param pPhysicalModel        The force that is being added to the force model
 * 
 * @todo Document the mass depletion approach.
 */
//------------------------------------------------------------------------------
void ODEModel::AddForce(PhysicalModel *pPhysicalModel)
{
   if (pPhysicalModel == NULL)
      throw ODEModelException(wxT("Attempting to add a NULL force to ") +
         instanceName);

   #ifdef DEBUG_ODEMODEL_INIT
      MessageInterface::ShowMessage(
         wxT("ODEModel::AddForce() <%p>'%s' entered, adding force = <%p><%s>'%s'\n"), this,
         GetName().c_str(), pPhysicalModel, pPhysicalModel->GetTypeName().c_str(),
         pPhysicalModel->GetName().c_str());
   #endif
   
   pPhysicalModel->SetDimension(dimension);
   initialized = false;
   
   // Handle the name issues
   wxString pmType = pPhysicalModel->GetTypeName();
   if (pmType == wxT("DragForce"))
      pPhysicalModel->SetName(wxT("Drag"));

   wxString forceBody = pPhysicalModel->GetBodyName();

   // Trap multiple instances
   if ((pmType == wxT("GravityField")) || (pmType == wxT("PointMassForce")))
   {
      wxString compType;
      
      for (std::vector<PhysicalModel*>::iterator i = forceList.begin();
           i != forceList.end(); ++i)
      {
         compType = (*i)->GetTypeName();
         if ((compType == wxT("GravityField")) || (compType == wxT("PointMassForce")))
         {
            if ((*i)->GetBodyName() == forceBody && (*i) != pPhysicalModel)
               throw ODEModelException(
                  wxT("Attempted to add a ") + pmType + 
                  wxT(" force to the force model for the body ") + forceBody +
                  wxT(", but there is already a ") + compType + 
                  wxT(" force in place for that body."));
         }

         if ((pmType == wxT("GravityField")) && (compType == wxT("GravityField")))
         {
            throw ODEModelException(
               wxT("Attempted to add a GravityField (aka primary body) force to ")
               wxT("the force model \"") + instanceName + wxT("\" for the body ") +
               forceBody + wxT(", but there already is a Gravity Field in the ODE ")
               wxT("Model and only one GravityField is supported per ODE Model in ")
               wxT("the current GMAT release"));
         }
      }      
   }
   
   // Check to be sure there is an associated PrimaryBody for drag forces
   if (pmType == wxT("DragForce"))
   {
      bool hasGravityField = false;
      for (std::vector<PhysicalModel*>::iterator i = forceList.begin();
           i != forceList.end(); ++i)
      {
         if ((*i)->GetTypeName() == wxT("GravityField"))
         {
            if ((*i)->GetBodyName() == forceBody)
               hasGravityField = true;
         }
      }
      if (hasGravityField == false)
         throw ODEModelException(
            wxT("Attempted to add a drag force for the body ") + forceBody +
            wxT(", but that body is not set as a primary body, so it does not ") +
            wxT("support additional forces."));
   }
   
   if (pmType == wxT("RelativisticCorrection"))
   {
      std::map<wxString, Real>::iterator pos;
      wxString rcBodyName = pPhysicalModel->GetBodyName();
      for (pos = muMap.begin(); pos != muMap.end(); ++pos)
      {
         if (pos->first == rcBodyName)
         {
            #ifdef DEBUG_MU_MAP
               MessageInterface::ShowMessage(wxT("ODEModel::AddForce ---  setting mu value of %12.10f on newly-added RC for body %s\n"),
                                             pos->second, rcBodyName.c_str());
            #endif
            pPhysicalModel->SetRealParameter(pPhysicalModel->GetParameterID(wxT("Mu")), pos->second);
         }
      }
   }


   // Add if new PhysicalModel pointer if not found in the forceList
   if (find(forceList.begin(), forceList.end(), pPhysicalModel) == forceList.end())
   {
      if (pPhysicalModel->IsTransient())
      {
         #ifdef DEBUG_ODEMODEL_INIT
            MessageInterface::ShowMessage(wxT("Adding a %s to this list:\n"),
                  pPhysicalModel->GetTypeName().c_str());
            for (UnsignedInt i = 0; i < forceList.size(); ++i)
               MessageInterface::ShowMessage(wxT("   %s\n"),
                     forceList[i]->GetTypeName().c_str());
            MessageInterface::ShowMessage(wxT("Transient count before ")
                  wxT("addition = %d\n"), transientCount);
         #endif

         ++transientCount;

         // Temporary code: prevent multiple finite burns in single force model
         if (transientCount > 1)
            throw ODEModelException(wxT("Multiple Finite burns are not allowed in ")
                  wxT("a single propagator; try breaking commands of the form")
                  wxT("\"Propagate prop(sat1, sat2)\" into two synchronized ")
                  wxT("propagators; e.g. \"Propagate Synchronized prop(sat1) ")
                  wxT("prop(sat2)\"\nexiting"));
      }
      forceList.push_back(pPhysicalModel);
   }
   numForces = forceList.size();
   
   // Update owned object count
   ownedObjectCount = numForces;
}

//------------------------------------------------------------------------------
// void DeleteForce(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Deletes named force from the force model.
 * 
 * @param name The name of the force to delete
 */
//------------------------------------------------------------------------------
void ODEModel::DeleteForce(const wxString &name)
{
   #ifdef DEBUG_ODEMODEL
   MessageInterface::ShowMessage
      (wxT("ODEModel::DeleteForce() entered, name='%s'\n"), name.c_str());
   #endif
   
   for (std::vector<PhysicalModel *>::iterator force = forceList.begin(); 
        force != forceList.end(); ++force) 
   {
      wxString pmName = (*force)->GetName();
      if (name == pmName)
      {
         PhysicalModel* pm = *force;
         forceList.erase(force);
         numForces = forceList.size();
         
         if (!pm->IsTransient())
         {
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Remove
               (pm, pm->GetName(), wxT("ODEModel::DeleteForce()"),
                wxT("deleting non-transient force of ") + pm->GetTypeName(), this);
            #endif
            delete pm;
         }
         else
            --transientCount;

         ownedObjectCount = numForces;
         return;
      }
   }
}


//------------------------------------------------------------------------------
// void DeleteForce(PhysicalModel *pPhyscialModel)
//------------------------------------------------------------------------------
/**
 * Deletes force from the force model.
 * 
 * @param pPhyscialModel The force name to delete
 */
//------------------------------------------------------------------------------
void ODEModel::DeleteForce(PhysicalModel *pPhysicalModel)
{
   for (std::vector<PhysicalModel *>::iterator force = forceList.begin();
        force != forceList.end(); ++force) 
   {
      if (*force == pPhysicalModel)
      {
         PhysicalModel* pm = *force;
         forceList.erase(force);
         numForces = forceList.size();
         
         if (!pm->IsTransient())
         {
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Remove
               (pm, pm->GetName(), wxT("ODEModel::DeleteForce()"),
                wxT("deleting non-transient force of ") + pm->GetTypeName(), this);
            #endif
            delete pm;
         }
         else
            --transientCount;

         ownedObjectCount = numForces;
         return;
      }
   }
}


//------------------------------------------------------------------------------
// bool HasForce(const wxString &name)
//------------------------------------------------------------------------------
/**
 * Search force in the force model.
 * 
 * @param  name The force name to look up
 * @return true if force exists, else false
 */
//------------------------------------------------------------------------------
bool ODEModel::HasForce(const wxString &name)
{
   for (std::vector<PhysicalModel *>::iterator force = forceList.begin(); 
       force != forceList.end(); force++) 
   {
      if (name == (*force)->GetName())
         return true;
   }
   return false; 
}


//------------------------------------------------------------------------------
// Integer GetNumForces()
//------------------------------------------------------------------------------
Integer ODEModel::GetNumForces()
{
    return numForces;
}


//------------------------------------------------------------------------------
// StringArray& GetForceTypeNames()
//------------------------------------------------------------------------------
StringArray& ODEModel::GetForceTypeNames()
{
    forceTypeNames.clear();

    for (int i=0; i<numForces; i++)
        forceTypeNames.push_back(forceList[i]->GetTypeName());
        
    return forceTypeNames;
}

//------------------------------------------------------------------------------
// wxString GetForceTypeName(Integer index)
//------------------------------------------------------------------------------
wxString ODEModel::GetForceTypeName(Integer index)
{
    StringArray typeList = GetForceTypeNames();
    
    if (index >= 0 && index < numForces)
        return typeList[index];

    return wxT("UNDEFINED_FORCE_TYPE");
}


////------------------------------------------------------------------------------
//// void ClearSpacecraft()
////------------------------------------------------------------------------------
//void ODEModel::ClearSpacecraft()
//{
//    spacecraft.clear();
//}


//------------------------------------------------------------------------------
// PhysicalModel* GetForce(Integer index) const
//------------------------------------------------------------------------------
PhysicalModel* ODEModel::GetForce(Integer index) const
{
    if (index >= 0 && index < numForces)
       return forceList[index];
    
    return NULL;
}


//------------------------------------------------------------------------------
// PhysicalModel* GetForce(wxString forcetype, Integer whichOne)
//------------------------------------------------------------------------------
/**
 * Search for a force in the force model.
 * 
 * @param  forcetype The kind of force desired.
 * @param  whichOne  Which force (zero-based index) of that type is desired.
 * 
 * @return The pointer to that force instance.
 */
//------------------------------------------------------------------------------
const PhysicalModel* ODEModel::GetForce(wxString forcetype, 
										Integer whichOne) const
{
   Integer i = 0;

   for (std::vector<PhysicalModel *>::const_iterator force = forceList.begin(); 
        force != forceList.end(); ++force) 
   {
      wxString pmName = (*force)->GetTypeName();
      if (pmName == forcetype) 
	  {
         if (whichOne <= i)
            return *force;
         else
            ++i;
      }
   }

   return NULL;
}


////------------------------------------------------------------------------------
//// bool ODEModel::AddSpaceObject(SpaceObject *so)
////------------------------------------------------------------------------------
///**
// * Sets spacecraft and formations that use this force model.
// *
// * @param so The spacecraft or formation
// *
// * @return true is the object is added to the list, false if it was already
// *         in the list, or if it is NULL.
// */
////------------------------------------------------------------------------------
//bool ODEModel::AddSpaceObject(SpaceObject *so)
//{
//    if (so == NULL)
//        return false;
//
//    if (find(spacecraft.begin(), spacecraft.end(), so) != spacecraft.end())
//        return false;
//
//    wxString soJ2KBodyName = so->GetStringParameter(wxT("J2000BodyName"));
//
//    // Set the refence body for the spacecraft states to match the J2000 body
//    // on the first spacecraft added to the force model.
//    if (spacecraft.size() == 0)
//       j2kBodyName = soJ2KBodyName;
//    else
//    {
//       if (j2kBodyName != soJ2KBodyName)
//          throw ODEModelException(
//             wxT("Force model error -- the internal reference body for all ")
//             wxT("spacecraft in a force model must be the same.\n")
//             wxT("The J2000Body for ") + so->GetName() + wxT(" is ") + soJ2KBodyName +
//             wxT(", but the force model is already using ") + j2kBodyName +
//             wxT(" as the reference body."));
//    }
//
//    spacecraft.push_back(so);
//
//    // Quick fix for the epoch update
//    satIds[0] = so->GetParameterID(wxT("A1Epoch"));
//    epoch = so->GetRealParameter(satIds[0]);
//    parametersSetOnce = false;
//    return true;
//}


void ODEModel::BufferState()
{
   state = psm->GetState();
   previousState = (*state);
}
    

//------------------------------------------------------------------------------
// void ODEModel::UpdateSpaceObject(Real newEpoch)
//------------------------------------------------------------------------------
/**
 * Updates state data for the spacecraft or formation that use this force model.
 */
//------------------------------------------------------------------------------
void ODEModel::UpdateSpaceObject(Real newEpoch)
{
   #ifdef DEBUG_ODEMODEL_EXE
      MessageInterface::ShowMessage(
            wxT("ODEModel::UpdateSpaceObject(%.12lf) called\n"), newEpoch);
   #endif

   Integer stateSize;
   Integer vectorSize;
   GmatState *state;
   ReturnFromOrigin(newEpoch);
   
   state = psm->GetState();
   stateSize = state->GetSize();
   vectorSize = stateSize * sizeof(Real);
   
   previousState = (*state);
   memcpy(state->GetState(), rawState, vectorSize);
   
   Real newepoch = epoch + elapsedTime / GmatTimeConstants::SECS_PER_DAY;
   
   // Update the epoch if it was passed in
   if (newEpoch != -1.0)
      newepoch = newEpoch;
   
   state->SetEpoch(newepoch);
   psm->MapVectorToObjects();
   
   // Update elements for each Formation
   for (UnsignedInt i = 0; i < stateObjects.size(); ++i)
      if (stateObjects[i]->IsOfType(Gmat::FORMATION))
         ((Formation*)stateObjects[i])->UpdateElements();
   
   #ifdef DEBUG_ODEMODEL_EXE
      MessageInterface::ShowMessage
            (wxT("ODEModel::UpdateSpaceObject() on \"%s\" prevElapsedTime = %f ")
             wxT("elapsedTime = %f newepoch = %f passed in epoch = %f ")
             wxT("dX's: [%.12lf] - [%.12lf] = [%.12lf]\n"), 
             GetName().c_str(), previousState.GetEpoch(), elapsedTime, 
             newepoch, newEpoch, (*state)[0], previousState[0], 
             ((*state)[0] - previousState[0]));
   #endif
}


//------------------------------------------------------------------------------
// void UpdateFromSpacecraft()
//------------------------------------------------------------------------------
/**
 * Updates the model state data from the spacecraft state -- useful to revert
 * to a previous step.
 *
 * @note This method will need to be updated when the multi-step integrators are
 *       folded into the code
 */
//------------------------------------------------------------------------------
void ODEModel::UpdateFromSpaceObject()
{
   // Update elements for each Formation
   for (UnsignedInt i = 0; i < stateObjects.size(); ++i)
      if (stateObjects[i]->IsOfType(Gmat::FORMATION))
         ((Formation*)stateObjects[i])->UpdateState();

   psm->MapObjectsToVector();
   GmatState *state = psm->GetState();
   memcpy(rawState, state->GetState(), state->GetSize() * sizeof(Real));

    // Transform to the force model origin
    MoveToOrigin();
}


//------------------------------------------------------------------------------
// void RevertSpacecraft()
//------------------------------------------------------------------------------
/**
 * Resets the model state data from the previous spacecraft state.
 *
 * @note This method will need to be updated when the multi-step integrators are
 *       folded into the code
 */
//------------------------------------------------------------------------------
void ODEModel::RevertSpaceObject()
{
   #ifdef DEBUG_ODEMODEL_EXE
      MessageInterface::ShowMessage
         (wxT("ODEModel::RevertSpacecraft() prevElapsedTime=%f elapsedTime=%f\n"),
          prevElapsedTime, elapsedTime);
   #endif

   elapsedTime = prevElapsedTime;//previousState.GetEpoch();// * GmatTimeConstants::SECS_PER_DAY;

   memcpy(rawState, previousState.GetState(), dimension*sizeof(Real));
   MoveToOrigin();
}



//------------------------------------------------------------------------------
// bool BuildModelFromMap()
//------------------------------------------------------------------------------
/**
 * Sets up the PhysicalModels in the ODEModel.
 *
 * The model information is all contained in an associated
 * PropagationStateManager.  The PSM is passed in as a parameter on this
 * call.
 *
 * The derivative model is built based on the order of the items in the state
 * vector.  State vectors are built so that like elements of different objects
 * are grouped together -- so, for example, if a state vector contains multiple
 * spacecraft, the state vector has all of the CartesianState data that is
 * integrated grouped into one block, and other pieces grouped together in
 * separate blocks.  All of the like data for a given object is grouped
 * sequentially before moving to a second object.  Thus for a state vector set
 * for the Cartesian state propagation for two spacecraft and the STM for the
 * first but not the second, the state vector has 48 elements, and it looks like
 * this (the ellipses are used for brevity here):
 *
 *    vec = [sat1.X, sat1.y,...,sat1.VZ, sat2.X,...,sat2.VZ,
 *           sat1.STM_XX, sat1.STM_XY,...,sat1.STM_VZVZ]
 *
 * The mapping between the state elements and the differential equations modeled
 * in this ODEModel are constructed using integer identifiers for the data built
 * in each PhysicalModel.
 *
 * @return true if the physical models were mapped successfully, false if not.
 */
//------------------------------------------------------------------------------
bool ODEModel::BuildModelFromMap()
{
   bool retval = false;
   #ifdef DEBUG_INITIALIZATION
      MessageInterface::ShowMessage(wxT("ODEModel::BuildModelFromMap() Entered\n"));
   #endif

   if (psm == NULL)
   {
      MessageInterface::ShowMessage(wxT("ODEModel::BuildModelFromMap():  Cannot ")
            wxT("build the model: PropStateManager is NULL\n"));
      return retval;
   }

   const std::vector<ListItem*> *map = psm->GetStateMap();

   if (map == NULL)
   {
      MessageInterface::ShowMessage(wxT("ODEModel::BuildModelFromMap():  Cannot ")
            wxT("build the model: the map is NULL for %s\n"), instanceName.c_str());
      return retval;
   }

   Integer start = 0, objectCount = 0;
   Gmat::StateElementId id = Gmat::UNKNOWN_STATE;
   GmatBase *currentObject = NULL;

   dynamicProperties = false;
   dynamicsIndex.clear();
   dynamicObjects.clear();
   dynamicIDs.clear();

   // Loop through the state map, counting objects for each type needed
   #ifdef DEBUG_INITIALIZATION
      MessageInterface::ShowMessage(wxT("ODEModel map has %d entries\n"),
            map->size());
   #endif
   for (UnsignedInt index = 0; index < map->size(); ++index)
   {
      if ((*map)[index]->dynamicObjectProperty)
      {
         dynamicProperties = true;

         dynamicsIndex.push_back(index);
         dynamicObjects.push_back((*map)[index]->object);
         dynamicIDs.push_back((*map)[index]->parameterID);
      }

      // When the elementID changes, act on the previous data processed
      if (id != (Gmat::StateElementId)((*map)[index]->elementID))
      {
         // Only build elements if an object needs them
         if (objectCount > 0)
         {
            // Build the derivative model piece for this element
            retval = BuildModelElement(id, start, objectCount);
            if (retval == false)
            {
//               throw ODEModelException(
               MessageInterface::ShowMessage(
                     wxT("Failed to build an element of the ODEModel.\n"));
               
               retval = true;
            }
         }
         // A new element type was encountered, so reset the pointers & counters
         id = (Gmat::StateElementId)((*map)[index]->elementID);
         objectCount = 0;
         start = index;
         currentObject = NULL;
      }
      // Increment the count for each new object
      if (currentObject != (*map)[index]->object)
      {
         currentObject = (*map)[index]->object;
         if (currentObject->IsOfType(Gmat::FORMATION))
         {
            Formation *form = (Formation*)currentObject;
            ObjectArray oa = form->GetRefObjectArray(Gmat::SPACEOBJECT);
            Integer fc = oa.size();
            objectCount += fc;
         }
         else
            ++objectCount;
      }
   }

   // Catch the last element
   if (objectCount > 0)
   {
      retval = BuildModelElement(id, start, objectCount);
      if (retval == false)
      {
         // throw ODEModelException(
         MessageInterface::ShowMessage(
            wxT("Failed to build an element of the ODEModel.\n"));
         retval = true;
      }
   }

   #ifdef DEBUG_BUILDING_MODELS
      // Show the state structure
      MessageInterface::ShowMessage(wxT("State vector has the following structure:\n"));
      MessageInterface::ShowMessage(wxT("   ID     Start   Count\n"));
      for (std::vector<StateStructure>::iterator i = sstruct.begin();
            i != sstruct.end(); ++i)
         MessageInterface::ShowMessage(wxT("   %4d      %2d    %d\n"), i->id, i->index,
               i->count);
   #endif
         
   #ifdef DEBUG_INITIALIZATION
      MessageInterface::ShowMessage(wxT("ODEModel::BuildModelFromMap() Finished\n"));
         #endif

   return retval;
}


//------------------------------------------------------------------------------
// bool BuildModelElement(Gmat::StateElementId id, Integer start,
//               Integer objectCount, std::map<GmatBase*,Integer> associateMap)
//------------------------------------------------------------------------------
/**
 * Constructs the derivative mapping an element of the ODEModel.
 *
 * This method tells each PhysicalModel about the information in the state
 * vector that expects derivative information used in the superposition.
 *
 * The PhysicalModel class has two methods that facilitate this mapping:
 *
 *    SupportsDerivative(Gmat::StateElementId id) -- Checks the model to see if
 *                                      it supports derivative information for a
 *                                      specific state parameter
 *
 *    SetStart(Gmat::StateElementId id, Integer index, Integer quantity) --
 *                                      Sets the start index for the
 *                                      derivative information in the state
 *                                      vector and the number of objects in the
 *                                      vector that receive this information.
 *
 * When this method is called, it loops through the PhysicalModels in the
 * ODEModel and, for each model supporting the element, registers the start
 * index and size of the portion of the state vector that receives the
 * derivative data.
 *
 * @param id      The integer ID for the element that is being registered
 * @param start   The index for the first element in the state vector
 * @param objectCount The number of objects that need the derivative data
 *
 * @return true if the derivative was set successfully for at least one
 *         PhysicalModel, false if it failed.
 */
//------------------------------------------------------------------------------
bool ODEModel::BuildModelElement(Gmat::StateElementId id, Integer start,
      Integer objectCount)
{
   bool retval = false, tf;
   Integer modelsUsed = 0;

   #ifdef DEBUG_BUILDING_MODELS
      MessageInterface::ShowMessage(wxT("Building ODEModel element; id = %d, ")
            wxT("index = %d, count = %d; force list has %d elements\n"), id, start,
            objectCount, forceList.size());
   #endif

   // Loop through the PhysicalModels, checking to see if any support the
   // indicated element.  If there is no derivative model anywhere in the PM
   // list, then the element will not change during integration, and the return
   // value for building that derivative model is false.  If support is
   // registered, then the PhysicalModel is handed information about where to
   // place the derivative data in the derivative state vector.
   for (std::vector<PhysicalModel *>::iterator i = forceList.begin();
         i != forceList.end(); ++i)
   {
      if ((*i)->SupportsDerivative(id))
      {
         tf = (*i)->SetStart(id, start, objectCount);
         if (tf == false)
            MessageInterface::ShowMessage(wxT("PhysicalModel %s was not set, even ")
                  wxT("though it registered support for derivatives of type %d\n"),
                  (*i)->GetTypeName().c_str(), id);
         else
            ++modelsUsed;

         if (retval == false)
            retval = tf;
      }
   }
    
   StateStructure newStruct;
   newStruct.id = id;
   newStruct.index = start;
   newStruct.count = objectCount;
   // newStruct.size = ??;
   sstruct.push_back(newStruct);

   // Cartesian state as a special case so error control can do RSS tests
   /// @todo Check this piece again for 6DoF
   if (id == Gmat::CARTESIAN_STATE)
   {
      cartesianCount   = objectCount;
      cartesianStart = start;
      cartStateSize  = objectCount * 6;
   }

   if (id == Gmat::ORBIT_STATE_TRANSITION_MATRIX)
   {
      fillSTM = true;
      if (stmStart == -1)
         stmStart = start;
      ++stmCount;
   }

   if (id == Gmat::ORBIT_A_MATRIX)
   {
      fillAMatrix = true;
      if (aMatrixStart == -1)
         aMatrixStart = start;
      ++aMatrixCount;
   }

   if (id == Gmat::EVENT_FUNCTION_STATE)
   {
      // todo Fill this in?
   }

   #ifdef DEBUG_BUILDING_MODELS
      MessageInterface::ShowMessage(
            wxT("ODEModel is using %d components for element %d\n"), modelsUsed, id);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool Initialize()
//------------------------------------------------------------------------------
/**
 * Initializes model and all contained models
 */
//------------------------------------------------------------------------------
bool ODEModel::Initialize()
{
   #ifdef DEBUG_INITIALIZATION
      MessageInterface::ShowMessage(wxT("ODEModel::Initialize() entered\n"));
   #endif
//   Integer stateSize = 6;      // Will change if we integrate more variables
//   Integer satCount = 1;
//   std::vector<SpaceObject *>::iterator sat;
   
   if (!solarSystem)
      throw ODEModelException(
         wxT("Cannot initialize force model; no solar system on '") + 
         instanceName + wxT("'"));

   if (j2kBodyName != wxT(""))
   {
      j2kBody = solarSystem->GetBody(j2kBodyName);
      if (j2kBody == NULL)
         throw ODEModelException(wxT("ODEModel J2000 body (") + j2kBodyName +
            wxT(") was not found in the solar system"));
   }

   #ifdef DEBUG_INITIALIZATION
      MessageInterface::ShowMessage(
         wxT("Calling PhysicalModel::Initialize(); dimension = %d\n"), dimension);
   #endif

   dimension = state->GetSize();

   if (!PhysicalModel::Initialize())
      return false;

   // Incorporate any temporary affects -- e.g. finite burn
   UpdateTransientForces();

   Integer newDim = state->GetSize();
   if (newDim != dimension)
   {
      // If dimensions don't match with transient forces, reinitialize
      dimension = newDim;
      if (!PhysicalModel::Initialize())
         return false;
   }

      #ifdef DEBUG_INITIALIZATION
      MessageInterface::ShowMessage(wxT("Configuring for state of dimension %d\n"),
            dimension);
      #endif
      
   // rawState deallocated in PhysicalModel::Initialize() method so reallocate
   rawState = new Real[dimension];
   #ifdef DEBUG_MEMORY
   MemoryTracker::Instance()->Add
      (rawState, wxT("rawState"), wxT("ODEModel::Initialize()"),
       wxT("rawState = new Real[dimension]"), this);
   #endif
   
   memcpy(rawState, state->GetState(), dimension * sizeof(Real));

   MoveToOrigin();

   // Variables used to set spacecraft parameters
   wxString parmName, stringParm;

   for (std::vector<PhysicalModel *>::iterator current = forceList.begin();
        current != forceList.end(); ++current)
   {
      #ifdef DEBUG_ODEMODEL_INIT
         wxString name, type;
         name = (*current)->GetName();
         if (name == wxT(""))
            name = wxT("unnamed");
         type = (*current)->GetTypeName();
         MessageInterface::ShowMessage
            (wxT("ODEModel::Initialize() initializing object %s of type %s\n"),
             name.c_str(), type.c_str());
      #endif
      
      (*current)->SetDimension(dimension);
      (*current)->SetState(state);
      
      // Only initialize the spacecraft independent pieces once
      if (forceMembersNotInitialized)
      {
         (*current)->SetSolarSystem(solarSystem);
         
         // Handle missing coordinate system issues for GravityFields
		  if ((*current)->IsOfType(wxT("HarmonicField")))
         {
			   SetInternalCoordinateSystem(wxT("InputCoordinateSystem"), (*current));
            SetInternalCoordinateSystem(wxT("FixedCoordinateSystem"), (*current));
            SetInternalCoordinateSystem(wxT("TargetCoordinateSystem"), (*current));

            if (body == NULL)
               body = solarSystem->GetBody(centralBodyName); // or should we get bodyName?
         }

         if ((*current)->IsOfType(wxT("DragForce")))
         {
            SetInternalCoordinateSystem(wxT("InputCoordinateSystem"), (*current));
            SetInternalCoordinateSystem(wxT("FixedCoordinateSystem"), (*current));
         }
      }

      // Initialize the forces
      if (!(*current)->Initialize())
      {
         wxString msg = wxT("Component force ");
         msg += (*current)->GetTypeName();
         msg += wxT(" failed to initialize");
         throw ODEModelException(msg);
      }
      if ((*current)->IsOfType(wxT("GravityField")))
      {
         // get the name and mu value for the muMap
         wxString itsName;
         Real        itsMu;
         ((GravityField*) (*current))->GetBodyAndMu(itsName, itsMu);
         #ifdef DEBUG_MU_MAP
            MessageInterface::ShowMessage(wxT("ODEModel::Initialize - saving mu value of %12.10f for body %s\n"),
                                          itsMu, itsName.c_str());
         #endif
         muMap[itsName] = itsMu;
      }
      (*current)->SetState(modelState);
   }
   for (std::vector<PhysicalModel *>::iterator current = forceList.begin();
        current != forceList.end(); ++current)
   {
      if ((*current)->IsOfType(wxT("RelativisticCorrection")))
      {
         std::map<wxString, Real>::iterator pos;
         wxString rcBodyName = (*current)->GetBodyName();
         for (pos = muMap.begin(); pos != muMap.end(); ++pos)
         {
            if (pos->first == rcBodyName)
            {
               #ifdef DEBUG_MU_MAP
                  MessageInterface::ShowMessage(wxT("ODEModel::Initialize ---  setting mu value of %12.10f on RC for body %s\n"),
                                                 pos->second, rcBodyName.c_str());
               #endif
               (*current)->SetRealParameter((*current)->GetParameterID(wxT("Mu")), pos->second);
            }
         }
      }
   }

   #ifdef DEBUG_FORCE_EPOCHS
      wxString epfile = wxT("ForceEpochs.txt");
      if (instanceName != wxT(""))
         epfile = instanceName + wxT("_") + epfile;
      if (!epochFile.is_open()) 
      {
         epochFile.open(epfile.c_str());
         epochFile << wxT("Epoch data for the force model '") 
                   << instanceName << wxT("'\n");
      }
   #endif

   #ifdef DEBUG_FIRST_CALL
   firstCallFired = false;
   #endif

   // Set flag stating that Initialize was successful once
   forceMembersNotInitialized = false;

   #ifdef DEBUG_INITIALIZATION
      MessageInterface::ShowMessage(wxT("ODEModel::Initialize() complete\n"));
   #endif

//   modelState = state->GetState();

   if (forceList.size() == 0)
      throw ODEModelException(wxT("The ODE model ") + instanceName +
            wxT(" is empty, so it cannot be used for propagation."));

   initialized = true;

   #ifdef DEBUG_MU_MAP
      std::map<wxString, Real>::iterator pos;
      MessageInterface::ShowMessage(wxT("----> At end of Initialize(), the muMap is:"));
      for (pos = muMap.begin(); pos != muMap.end(); ++pos)
      {
         MessageInterface::ShowMessage(wxT("      %s      %12.10f\n"), (pos->first).c_str(), pos->second);
      }
   #endif

   return true;
}


//------------------------------------------------------------------------------
// void ClearForceList()
//------------------------------------------------------------------------------
void ODEModel::ClearForceList(bool deleteTransient)
{
   #ifdef DEBUG_ODEMODEL_CLEAR
   MessageInterface::ShowMessage
      (wxT("ODEModel::ClearForceList() entered, there are %d forces\n"), forceList.size());
   #endif
   
   // Delete the owned forces
   std::vector<PhysicalModel *>::iterator ppm = forceList.begin();
   PhysicalModel *pm;
   while (ppm != forceList.end())
   {
      pm = *ppm;
      forceList.erase(ppm);
      
      #ifdef DEBUG_ODEMODEL_CLEAR
      MessageInterface::ShowMessage(wxT("   Checking if pm<%p> is transient\n"), pm);
      #endif
      
      // Transient forces are managed in the Sandbox.
      if (pm->IsTransient())
         --transientCount;

      if (!pm->IsTransient() || (deleteTransient && pm->IsTransient()))
      {
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (pm, pm->GetName(), wxT("ODEModel::~ODEModel()"),
             wxT("deleting non-transient \"") + pm->GetTypeName() +
             wxT("\" PhysicalModel"), this);
         #endif
         delete pm;
      }
      ppm = forceList.begin();
   }
   
}


//------------------------------------------------------------------------------
// void ClearInternalCoordinateSystems()
//------------------------------------------------------------------------------
/**
 * Manages the deallocation of coordinate systems used internally.
 */
//------------------------------------------------------------------------------
void ODEModel::ClearInternalCoordinateSystems()
{
   for (std::vector<CoordinateSystem*>::iterator i = 
           internalCoordinateSystems.begin();
        i != internalCoordinateSystems.end(); ++i)
   {
      #ifdef DEBUG_MEMORY
      MemoryTracker::Instance()->Remove
         ((*i), (*i)->GetName(), wxT("ODEModel::ClearInternalCoordinateSystems()"),
          wxT("deleting ICS"), this);
      #endif
      delete (*i);
   }
   internalCoordinateSystems.clear();
}

//------------------------------------------------------------------------------
// void SetInternalCoordinateSystem(const wxString csId, 
//      PhysicalModel *currentPm)
//------------------------------------------------------------------------------
/**
 * Manages the allocation of coordinate systems used internally.
 * 
 * @param <csId>        Parameter name for the coordinate system label.
 * @param <currentPm>   Force that needs the CoordinateSystem.
 */
//------------------------------------------------------------------------------
void ODEModel::SetInternalCoordinateSystem(const wxString csId,
                                           PhysicalModel *currentPm)
{
   wxString csName;
   CoordinateSystem *cs = NULL;

   #ifdef DEBUG_ODEMODEL_INIT     
      MessageInterface::ShowMessage(
         wxT("Setting internal CS with ID '%s' for force type '%s'\n"),
         csId.c_str(), currentPm->GetTypeName().c_str());
   #endif
   csName = currentPm->GetStringParameter(csId);

   try
   {
      currentPm->GetRefObject(Gmat::COORDINATE_SYSTEM, csName);
   }
   catch (BaseException &)
   {
      #ifdef DEBUG_ODEMODEL_INIT
         MessageInterface::ShowMessage(
            wxT("Adding a coordinate system named '%s' for the %s physical model\n"),
            csName.c_str(), currentPm->GetTypeName().c_str());
      #endif
      
      for (std::vector<CoordinateSystem*>::iterator i =
              internalCoordinateSystems.begin();
           i != internalCoordinateSystems.end(); ++i)
         if ((*i)->GetName() == csName)
            cs = *i;
      
      if (cs == NULL)
      {
         // We need to handle both inertial and fixed CS's here
         if (earthEq == NULL)
            throw ODEModelException(
               wxT("Error setting force model coordinate system for ") +
               instanceName + wxT(": EarthEq pointer has not been initialized!"));
         if (earthFixed == NULL)
            throw ODEModelException(
               wxT("Error setting force model coordinate system for ") +
               instanceName + wxT(": EarthFixed pointer has not been initialized!"));
         
         if (csName.find(wxT("Fixed"), 0) == wxString::npos)
         {
            cs = (CoordinateSystem *)earthEq->Clone();
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Add
               (cs, csName, wxT("ODEModel::SetInternalCoordinateSystem()"),
                wxT("cs = earthEq->Clone()"), this);
            #endif
         }
         else
         {
            cs = (CoordinateSystem *)earthFixed->Clone();
            #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Add
               (cs, csName, wxT("ODEModel::SetInternalCoordinateSystem()"),
                wxT("cs = earthFixed->Clone()"), this);
            #endif
         }
         
         cs->SetName(csName);
         cs->SetStringParameter(wxT("Origin"), centralBodyName);
         cs->SetRefObject(forceOrigin, Gmat::CELESTIAL_BODY, 
            centralBodyName);
         internalCoordinateSystems.push_back(cs);

         #ifdef DEBUG_ODEMODEL_INIT
            MessageInterface::ShowMessage(wxT("Created %s with description\n\n%s\n"), 
               csName.c_str(), 
               cs->GetGeneratingString(Gmat::SCRIPTING).c_str());
         #endif
      }
      
      cs->SetSolarSystem(solarSystem);
      cs->SetJ2000BodyName(j2kBody->GetName());
      cs->SetJ2000Body(j2kBody);
      cs->Initialize();

      #ifdef DEBUG_ODEMODEL_INIT     
         MessageInterface::ShowMessage(
            wxT("New coordinate system named '%s' has definition\n%s\n"),
            csName.c_str(), 
            cs->GetGeneratingString(Gmat::SCRIPTING, wxT("   ")).c_str());
      #endif
         
      currentPm->SetRefObject(cs, Gmat::COORDINATE_SYSTEM, csName);
   }
}


//------------------------------------------------------------------------------
// Integer ODEModel::GetOwnedObjectCount()
//------------------------------------------------------------------------------
Integer ODEModel::GetOwnedObjectCount()
{
   #ifdef DEBUG_ODEMODEL_OWNED_OBJECT
   MessageInterface::ShowMessage
      (wxT("ODEModel::GetOwnedObjectCount() this=<%p>'%s' returning %d\n"),
       this, GetName().c_str(), numForces);
   #endif
   return numForces;
}


//------------------------------------------------------------------------------
// GmatBase* ODEModel::GetOwnedObject(Integer whichOne)
//------------------------------------------------------------------------------
GmatBase* ODEModel::GetOwnedObject(Integer whichOne)
{
   if (whichOne < numForces) 
      return forceList[whichOne];
   
   return NULL;
}


//------------------------------------------------------------------------------
// virtual wxString BuildPropertyName(GmatBase *ownedObj)
//------------------------------------------------------------------------------
/*
 * Builds property name of owned object.
 * This method is called when special handling of object property name is
 * required when writing object. For example, ODEModel requires additional
 * name Earth for GravityField as in FM.GravityField.Earth.Degree.
 *
 * @param ownedObj The object of property handling
 * @return The property name
 */
//------------------------------------------------------------------------------
wxString ODEModel::BuildPropertyName(GmatBase *ownedObj)
{
   #ifdef DEBUG_OWNED_OBJECT_STRINGS
   MessageInterface::ShowMessage
      (wxT("ODEModel::BuildPropertyName() called with ownedObj type=%s\n"),
       ownedObj->GetTypeName().c_str());
   #endif
   
   return BuildForceNameString((PhysicalModel*)ownedObj);
}


//------------------------------------------------------------------------------
// void UpdateInitialData()
//------------------------------------------------------------------------------
/**
 * Updates model and all contained models to catch changes in Spacecraft, etc.
 */
//------------------------------------------------------------------------------
void ODEModel::UpdateInitialData(bool dynamicOnly)
{
   PhysicalModel *current; // = forceList[cf];  // waw: added 06/04/04

   // Variables used to set spacecraft parameters
   wxString parmName, stringParm;
   std::vector<SpaceObject *>::iterator sat;

   for (std::vector<PhysicalModel*>::iterator i = forceList.begin();
        i != forceList.end(); ++i)
   {
      stateObjects.clear();
      psm->GetStateObjects(stateObjects, Gmat::SPACEOBJECT);
      
      if (dynamicOnly)
         UpdateDynamicSpacecraftData(&stateObjects, 0);
      else
      {
         current = (*i);
         if (!parametersSetOnce)
         {
            current->ClearSatelliteParameters();
         }

         SetupSpacecraftData(&stateObjects, 0);
      }
   }
   if (!dynamicOnly)
      psm->MapObjectsToVector();

   parametersSetOnce = true;
}


//------------------------------------------------------------------------------
// void UpdateTransientForces()
//------------------------------------------------------------------------------
/**
 * Tells the transient forces in the model about the propagated SpaceObjects.
 *
 * In GMAT, a wxT("transient force") is a force that is applied based on state
 * changes made to the elements that are propagated during a run.  In other
 * words, a transient force is a force that gets applied when needed, but not
 * typically throughout the mission.  An example is a finite burn: the
 * acceleration for a finite burn is calculated and applied only when a
 * spacecraft has a thruster that has been turned on.
 *
 * @param transientSats The list of spacecraft that report active transient
 *                      forces.
 */
//------------------------------------------------------------------------------
void ODEModel::UpdateTransientForces()
{
   #ifdef DEBUG_INITIALIZATION
     MessageInterface::ShowMessage(wxT("ODEModel::UpdateTransientForces entered\n"));
   #endif
   if (psm == NULL)
   {
//      MessageInterface::ShowMessage(wxT("PSM is NULL\n"));
//      throw ODEModelException(
//            wxT("Cannot initialize ODEModel; no PropagationStateManager"));
      return;
   }
   
   const std::vector<ListItem*> *propList = psm->GetStateMap();
         
   // This version won't work for Formations.  
   // todo: make into a recursive method for Formations
   ObjectArray propObjects;
   for (std::vector<ListItem*>::const_iterator i = propList->begin(); 
        i != propList->end(); ++i)
   {
      GmatBase *obj =  (*i)->object;
      if (obj != NULL)
      {
         if (obj->IsOfType(Gmat::SPACECRAFT))
         {
            if (find(propObjects.begin(), propObjects.end(), obj) == 
                      propObjects.end())
            {
               #ifdef DEBUG_INITIALIZATION
               MessageInterface::ShowMessage
                  (wxT("ODEModel::UpdateTransientForces() Adding <%p>'%s' to propObjects\n"),
                   obj, obj->GetName().c_str());
               #endif
               propObjects.push_back(obj);
            }
         }
      }
   }
   
   transientCount = 0;
   for (std::vector<PhysicalModel *>::iterator tf = forceList.begin();
        tf != forceList.end(); ++tf) 
   {
      if ((*tf)->IsTransient())
      {
         ++transientCount;
         #ifdef DEBUG_INITIALIZATION
            MessageInterface::ShowMessage(wxT("Updating transient force %s\n"),
               (*tf)->GetName().c_str());
         #endif
         (*tf)->SetPropList(&propObjects);
      }
   }
}


//------------------------------------------------------------------------------
// Integer SetupSpacecraftData(GmatBase *sat, PhysicalModel *pm, Integer i)
//------------------------------------------------------------------------------
/**
 * Passes spacecraft parameters into the force model.
 * 
 * @param <sat>   The SpaceObject that supplies the parameters.
 * @param <pm>    The PhysicalModel receiving the data.
 * @param <i>     The index of the SpaceObject in the physical model.
 * 
 * @return For Spacecraft, the corresponding index; for formations, a count of
 *         the number of spacecraft in the formation.
 */
//------------------------------------------------------------------------------
Integer ODEModel::SetupSpacecraftData(ObjectArray *sats, Integer i)
{
   #ifdef DEBUG_SPACECRAFT_PROPERTIES
      MessageInterface::ShowMessage(wxT("ODEModel::SetupSpacecraftData(*, %d) ")
            wxT("called\n"), i);
   #endif

   Real parm;
   wxString stringParm;

   GmatBase* sat;
   Integer increment = 1;
   
   for (ObjectArray::iterator j = sats->begin(); 
        j != sats->end(); ++j)
   {
      sat = *j;

      #ifdef DEBUG_SPACECRAFT_PROPERTIES
         MessageInterface::ShowMessage(wxT("   Looking at %s\n"),
               sat->GetName().c_str());
      #endif
      // Only retrieve the parameter IDs once
      if ((satIds[1] < 0) && sat->IsOfType(wxT("Spacecraft")))
      {
         satIds[0] = sat->GetParameterID(wxT("A1Epoch"));
         if (satIds[0] < 0)
            throw ODEModelException(wxT("Epoch parameter undefined on object ") +
                                    sat->GetName());
         
         satIds[1] = sat->GetParameterID(wxT("CoordinateSystem"));
         if (satIds[1] < 0)
            throw ODEModelException(
               wxT("CoordinateSystem parameter undefined on object ") + sat->GetName());
         
         satIds[2] = sat->GetParameterID(wxT("TotalMass"));
         if (satIds[2] < 0)
            throw ODEModelException(wxT("TotalMass parameter undefined on object ") +
                                    sat->GetName());
         
         satIds[3] = sat->GetParameterID(wxT("Cd"));
         if (satIds[3] < 0)
            throw ODEModelException(wxT("Cd parameter undefined on object ") +
                                    sat->GetName());
         
         satIds[4] = sat->GetParameterID(wxT("DragArea"));
         if (satIds[4] < 0)
            throw ODEModelException(wxT("Drag Area parameter undefined on object ") +
                                    sat->GetName());
         
         satIds[5] = sat->GetParameterID(wxT("SRPArea"));
         if (satIds[5] < 0)
            throw ODEModelException(wxT("SRP Area parameter undefined on object ") +
                                    sat->GetName());
         
         satIds[6] = sat->GetParameterID(wxT("Cr"));
         if (satIds[6] < 0)
            throw ODEModelException(wxT("Cr parameter undefined on object ") +
                                    sat->GetName());
         
         stateStart = sat->GetParameterID(wxT("CartesianX"));
         stateEnd   = sat->GetParameterID(wxT("CartesianVZ"));

         #ifdef DEBUG_SATELLITE_PARAMETERS
         MessageInterface::ShowMessage(
            wxT("Parameter ID Array: [%d %d %d %d %d %d %d]; PMepoch id  = %d\n"),
            satIds[0], satIds[1], satIds[2], satIds[3], satIds[4], satIds[5], 
               satIds[6], PhysicalModel::EPOCH);
         #endif
      }
      
      PhysicalModel *pm;
      for (std::vector<PhysicalModel *>::iterator current = forceList.begin();
           current != forceList.end(); ++current)
      {
         pm = *current;
         
         if (sat->GetType() == Gmat::SPACECRAFT)
         { 
            #ifdef DEBUG_SATELLITE_PARAMETERS
               MessageInterface::ShowMessage(
                   wxT("ODEModel '%s', Member %s: %s->ParmsChanged = %s, ")
                   wxT("parametersSetOnce = %s\n"),
                   GetName().c_str(), pm->GetTypeName().c_str(), 
                   sat->GetName().c_str(), 
                   (((SpaceObject*)sat)->ParametersHaveChanged() ? wxT("true") : wxT("false")), 
                   (parametersSetOnce ? wxT("true") : wxT("false")));
            #endif
               
            // Manage the epoch ...
            parm = sat->GetRealParameter(satIds[0]);
            // Update local value for epoch
            epoch = parm;
            pm->SetRealParameter(PhysicalModel::EPOCH, parm);
            
            if (((SpaceObject*)sat)->ParametersHaveChanged() ||
                !parametersSetOnce)
            {
               #ifdef DEBUG_SATELLITE_PARAMETERS
                  MessageInterface::ShowMessage(
                     wxT("Setting parameters for %s using data from %s\n"),
                     pm->GetTypeName().c_str(), sat->GetName().c_str());
               #endif
               
               // ... Coordinate System ...
               stringParm = sat->GetStringParameter(satIds[1]);
               
               CoordinateSystem *cs =
                  (CoordinateSystem*)(sat->GetRefObject(Gmat::COORDINATE_SYSTEM, 
                      stringParm));
               if (!cs)
               {
                  wxString sataddr;
                  sataddr.Printf( wxT("%lx"), (unsigned long)sat);
                  throw ODEModelException(
                     wxT("CoordinateSystem is NULL on Spacecraft ") + sat->GetName() +
                     wxT(" at address ") + sataddr);
               }
               pm->SetSatelliteParameter(i, wxT("ReferenceBody"), cs->GetOriginName());
               
               // ... Mass ...
               parm = sat->GetRealParameter(satIds[2]);
               if (parm <= 0)
                  throw ODEModelException(wxT("Mass parameter unphysical on object ") + 
                     sat->GetName());
               pm->SetSatelliteParameter(i, wxT("Mass"), parm, satIds[2]);
               
               // ... Coefficient of drag ...
               parm = sat->GetRealParameter(satIds[3]);
               if (parm < 0)
                  throw ODEModelException(wxT("Cd parameter unphysical on object ") + 
                     sat->GetName());
               pm->SetSatelliteParameter(i, wxT("Cd"), parm, satIds[3]);
               
               // ... Drag area ...
               parm = sat->GetRealParameter(satIds[4]);
               if (parm < 0)
                  throw ODEModelException(wxT("Drag Area parameter unphysical on object ") + 
                     sat->GetName());
               pm->SetSatelliteParameter(i, wxT("DragArea"), parm, satIds[4]);
               
               // ... SRP area ...
               parm = sat->GetRealParameter(satIds[5]);
               if (parm < 0)
                  throw ODEModelException(wxT("SRP Area parameter unphysical on object ") + 
                     sat->GetName());
               pm->SetSatelliteParameter(i, wxT("SRPArea"), parm, satIds[5]);
               
               // ... and Coefficient of reflectivity
               parm = sat->GetRealParameter(satIds[6]);
               if (parm < 0)
                  throw ODEModelException(wxT("Cr parameter unphysical on object ") + 
                     sat->GetName());
               pm->SetSatelliteParameter(i, wxT("Cr"), parm, satIds[6]);
               
               ((SpaceObject*)sat)->ParametersHaveChanged(false);
            }
            increment = 1;
         }
         else if (sat->GetType() == Gmat::FORMATION) 
         {
            ObjectArray formSats;
            ObjectArray elements = sat->GetRefObjectArray(wxT("SpaceObject"));
            for (ObjectArray::iterator n = elements.begin(); n != elements.end();
                 ++n) 
            {
               if ((*n)->IsOfType(Gmat::SPACEOBJECT))
                  formSats.push_back((SpaceObject *)(*n));
               else
                  throw ODEModelException(wxT("Object \"") + sat->GetName() +
                                          wxT("\" is not a SpaceObject."));
            }
            increment = SetupSpacecraftData(&formSats, i) - i;
         }
         else
            throw ODEModelException(wxT("Setting SpaceObject parameters on unknown ")
                  wxT("type for ") + sat->GetName());
      }
      i += increment;
   }
   
   #ifdef DEBUG_SPACECRAFT_PROPERTIES
      MessageInterface::ShowMessage(wxT("   ---> %d returned\n"), i);
   #endif

   return i;
}


Integer ODEModel::UpdateDynamicSpacecraftData(ObjectArray *sats, Integer i)
{
   Real parm;
   wxString stringParm;

   GmatBase* sat;

   for (ObjectArray::iterator j = sats->begin();
        j != sats->end(); ++j)
   {
      sat = *j;

      if (satIds[1] < 0)
         throw ODEModelException(wxT("Epoch parameter undefined on object ") +
                                    sat->GetName());

      PhysicalModel *pm;
      for (std::vector<PhysicalModel *>::iterator current = forceList.begin();
           current != forceList.end(); ++current)
      {
         pm = *current;

         if (sat->GetType() == Gmat::SPACECRAFT)
         {
            #ifdef DEBUG_SATELLITE_PARAMETER_UPDATES
               MessageInterface::ShowMessage(
                   wxT("ODEModel '%s', Member %s: %s->ParmsChanged = %s, ")
                   wxT("parametersSetOnce = %s\n"),
                   GetName().c_str(), pm->GetTypeName().c_str(),
                   sat->GetName().c_str(),
                   (((SpaceObject*)sat)->ParametersHaveChanged() ? wxT("true") : wxT("false")),
                   (parametersSetOnce ? wxT("true") : wxT("false")));
            #endif

               // ... Mass ...
               parm = sat->GetRealParameter(satIds[2]);
               if (parm <= 0)
                  throw ODEModelException(wxT("Mass parameter unphysical on object ") +
                     sat->GetName());
               pm->SetSatelliteParameter(i, satIds[2], parm);

               // ... Drag area ...
               parm = sat->GetRealParameter(satIds[4]);
               if (parm < 0)
                  throw ODEModelException(wxT("Drag Area parameter unphysical on object ") +
                     sat->GetName());
               pm->SetSatelliteParameter(i, satIds[4], parm);

               // ... SRP area ...
               parm = sat->GetRealParameter(satIds[5]);
               if (parm < 0)
                  throw ODEModelException(wxT("SRP Area parameter unphysical on object ") +
                     sat->GetName());
               pm->SetSatelliteParameter(i, satIds[5], parm);
         }
         else if (sat->GetType() == Gmat::FORMATION)
         {
            ObjectArray formSats;
            ObjectArray elements = sat->GetRefObjectArray(wxT("SpaceObject"));
            for (ObjectArray::iterator n = elements.begin(); n != elements.end();
                 ++n)
            {
               if ((*n)->IsOfType(Gmat::SPACEOBJECT))
                  formSats.push_back((SpaceObject *)(*n));
               else
                  throw ODEModelException(wxT("Object \"") + sat->GetName() +
                                          wxT("\" is not a SpaceObject."));
            }
            UpdateDynamicSpacecraftData(&formSats, i);
         }
         else
            throw ODEModelException(
                                    wxT("Setting SpaceObject parameters on unknown ")
                                    wxT("type for ") + sat->GetName());
      }
      ++i;
   }

   return i;
}


//------------------------------------------------------------------------------
// bool ODEModel::GetDerivatives(Real * state, Real dt, Integer order)
//------------------------------------------------------------------------------
/**
 * Returns the accumulated superposition of forces 
 * 
 * This method applies superposition of forces in order to calculate the total
 * acceleration applied to the state vector.
 * 
 * @param    state   The current state vector
 * @param    dt      The current time interval from epoch
 * @param    order   Order of the derivative to be taken
 * @param    id      StateElementId for the requested derivative
 */
//------------------------------------------------------------------------------
bool ODEModel::GetDerivatives(Real * state, Real dt, Integer order,
      const Integer id)
{
   #ifdef DEBUG_ODEMODEL_EXE
      MessageInterface::ShowMessage(wxT("  ODEModel Input state = ["));
      for (Integer i = 0; i < dimension-1; ++i)
         MessageInterface::ShowMessage(wxT("%le, "), state[i]);
      MessageInterface::ShowMessage(wxT("%le]\n"), state[dimension - 1]);
   #endif

   if (order > 2)
   {
      return false;
   }
   if (!initialized)
   {
      return false;
   }

   if (dynamicProperties)
   {
      for (UnsignedInt i = 0; i < dynamicsIndex.size(); ++i)
      {
         #ifdef DEBUG_MASS_FLOW
            MessageInterface::ShowMessage(wxT("Updating %s on %s to %.12le\n"),
                  dynamicObjects[i]->GetParameterText(dynamicIDs[i]).c_str(),
                  dynamicObjects[i]->GetName().c_str(),
                  state[dynamicsIndex[i]]);
         #endif
         if (((dynamicIDs[i] >= stateStart) && (dynamicIDs[i] <= stateEnd)))
         {
            SpacePoint *objBody = NULL;
            if (dynamicObjects[i]->IsOfType(Gmat::SPACEOBJECT))
               objBody = ((SpaceObject*)(dynamicObjects[i]))->GetOrigin();
            if (objBody != NULL)
            {
               Rvector6 offset = objBody->GetMJ2000State(((SpaceObject*)
                     (dynamicObjects[i]))->GetEpoch() + dt /
                     GmatTimeConstants::SECS_PER_DAY);

               dynamicObjects[i]->SetRealParameter(dynamicIDs[i],
                     state[dynamicsIndex[i]] +
                     offset[dynamicIDs[i]-stateStart]);
            }
            else
               dynamicObjects[i]->SetRealParameter(dynamicIDs[i],
                     state[dynamicsIndex[i]]);
         }
         else  // Handle the other (non-Cartesian) dynamical property changes
            dynamicObjects[i]->SetRealParameter(dynamicIDs[i],
                  state[dynamicsIndex[i]]);
      }
      UpdateInitialData(true);
   }

   #ifdef DEBUG_ODEMODEL_EXE
      MessageInterface::ShowMessage(wxT("Initializing derivative array\n"));
   #endif

   #ifdef DEBUG_STATE
      MessageInterface::ShowMessage(
         wxT("Top of GetDeriv; State with dimension %d = ["), dimension);
      for (Integer i = 0; i < dimension-1; ++i)
         MessageInterface::ShowMessage(wxT("%le, "), state[i]);
      MessageInterface::ShowMessage(wxT("%le]\n"), state[dimension-1]);
   #endif
  
   PrepareDerivativeArray();

   const Real* ddt;

   #ifdef DEBUG_ODEMODEL_EXE
      MessageInterface::ShowMessage(wxT("Looping through %d PhysicalModels\n"),
            forceList.size());
   #endif

   // Apply superposition of forces/derivatives
   for (std::vector<PhysicalModel *>::iterator i = forceList.begin();
         i != forceList.end(); ++i)
   {
      #ifdef DEBUG_ODEMODEL_EXE
         MessageInterface::ShowMessage(wxT("   %s\n"), ((*i)->GetTypeName()).c_str());
      #endif
   
      ddt = (*i)->GetDerivativeArray();
      if (!(*i)->GetDerivatives(state, dt, order))
      {
         #ifdef DEBUG_ODEMODEL_EXE
            MessageInterface::ShowMessage(wxT("Derivative %s failed\n"),
                  ((*i)->GetTypeName()).c_str());
         #endif
         return false;

      }
      
      #ifdef DEBUG_ODEMODEL_EXE
      for (Integer j = 0; j < dimension; ++j)
         MessageInterface::ShowMessage(wxT("  ddt(%s[%s])[%d] = %le\n"),
            ((*i)->GetTypeName().c_str()),
            ((*i)->GetStringParameter((*i)->GetParameterID(wxT("BodyName")))).c_str(),
            j, ddt[j]);
      #endif
      
      #ifdef DEBUG_ODEMODEL_EXE
         MessageInterface::ShowMessage(wxT("  deriv = ["));
      #endif
      for (Integer j = 0; j < dimension; ++j)
      {
         deriv[j] += ddt[j];
         #ifdef DEBUG_ODEMODEL_EXE
            MessageInterface::ShowMessage(wxT(" %16.14le "), ddt[j]);
         #endif
      }
      #ifdef DEBUG_ODEMODEL_EXE
         MessageInterface::ShowMessage(wxT("]\n"));
      #endif


      #ifdef DEBUG_FIRST_CALL
         if (firstCallFired == false)
         {
            MessageInterface::ShowMessage(
               wxT("   %s(%s)::GetDerivatives --> ")
               wxT("[%.10lf %.10lf %.10lf %.16lf %.16lf %.16lf]\n"),
               forceList[0]->GetTypeName().c_str(),
               forceList[0]->GetName().c_str(),
               ddt[0], ddt[1], ddt[2], ddt[3], ddt[4], ddt[5]);
         }
      #endif
   }

   if (psm->RequiresCompletion())
      CompleteDerivativeCalculations(state);

   #ifdef DEBUG_ODEMODEL_EXE
      MessageInterface::ShowMessage(wxT("  ===============================\n"));
   #endif

   #ifdef DEBUG_FIRST_CALL
      if (firstCallFired == false)
      {
         MessageInterface::ShowMessage(
            wxT("GetDerivatives: [%.16lf %.16lf %.16lf %.16lf %.16lf %.16lf]\n"),
            deriv[0], deriv[1], deriv[2], deriv[3], deriv[4], deriv[5]);
      }

      firstCallFired = true;
   #endif

   #ifdef DEBUG_STATE
      MessageInterface::ShowMessage(
               wxT("End of GetDeriv; State with dimension %d = ["), dimension);
      for (Integer i = 0; i < dimension-1; ++i) //< state->GetSize()-1; ++i)
         MessageInterface::ShowMessage(wxT("%.12le, "), state[i]); //(*state)[i]);
      MessageInterface::ShowMessage(wxT("%.12le]\n"), state[dimension-1]); //(*state)[state->GetSize()-1]);

      MessageInterface::ShowMessage(
               wxT("   Derivative = ["));
      for (Integer i = 0; i < dimension-1; ++i) //< state->GetSize()-1; ++i)
         MessageInterface::ShowMessage(wxT("%.12le, "), deriv[i]); //(*state)[i]);
      MessageInterface::ShowMessage(wxT("%.12le]\n"), deriv[dimension-1]); //(*state)[state->GetSize()-1]);
   #endif

   #ifdef DUMP_TOTAL_DERIVATIVE
      MessageInterface::ShowMessage(
               wxT("   Derivative = ["));
      for (Integer i = 0; i < dimension-1; ++i) //< state->GetSize()-1; ++i)
         MessageInterface::ShowMessage(wxT("%.12le, "), deriv[i]); //(*state)[i]);
      MessageInterface::ShowMessage(wxT("%.12le]\n"), deriv[dimension-1]); //(*state)[state->GetSize()-1]);
   #endif

   #ifdef DEBUG_STM_AMATRIX_DERIVS
      MessageInterface::ShowMessage(wxT("Final dv array:\n"));

      for (Integer i = 0; i < 6; ++i)
      {
         MessageInterface::ShowMessage(wxT("  %lf  "), deriv[i]);
      }
      MessageInterface::ShowMessage(wxT("\n"));
      for (Integer i = 6; i < dimension; i += 6)
      {
         for (Integer j = 0; j < 6; ++j)
            MessageInterface::ShowMessage(wxT("  %le  "), deriv[i+j]);
         MessageInterface::ShowMessage(wxT("\n"));
      }
   #endif

   return true;
}


bool ODEModel::PrepareDerivativeArray()
{
   bool retval = true;

   #ifdef DEBUG_STM_AMATRIX_DERIVS
      static bool eins = false;
   #endif

   const std::vector<ListItem*> *smap = psm->GetStateMap();

   // Initialize the derivative array
   for (Integer i = 0; i < dimension; ++i)
   {
      #ifdef DEBUG_STM_AMATRIX_DERIVS
         if (eins == false)
            MessageInterface::ShowMessage(wxT("Mapping [%d] %s\n"), i,
                  ((*smap)[i]->nonzeroInit == true ? wxT("true") : wxT("false")));
      #endif

      if ((*smap)[i]->nonzeroInit)
      {
         deriv[i] = (*smap)[i]->initialValue;
      }
      else
         deriv[i] = 0.0;
   }

   #ifdef DEBUG_STM_AMATRIX_DERIVS
      if (eins == false)
      {
         MessageInterface::ShowMessage(wxT("Initial dv array:\n"));

         for (Integer i = 0; i < 6; ++i)
         {
            MessageInterface::ShowMessage(wxT("  %lf  "), deriv[i]);
         }
         MessageInterface::ShowMessage(wxT("\n"));
         for (Integer i = 6; i < dimension; i += 6)
         {
            for (Integer j = 0; j < 6; ++j)
               MessageInterface::ShowMessage(wxT("  %lf  "), deriv[i+j]);
            MessageInterface::ShowMessage(wxT("\n"));
         }
      }

      eins = true;
   #endif

   return retval;
}


bool ODEModel::CompleteDerivativeCalculations(Real *state)
{
   bool retval = true;

   // Convert A to Phi dot for STM pieces
   // \Phi\dot = A\tilde \Phi

   for (Integer i = 0; i < stmCount; ++i)
   {
      Integer i6 = stmStart + i * 36;

      // Build aTilde
      Real aTilde[36];
      for (Integer m = 0; m < 36; ++m)
         aTilde[m] = deriv[i6+m];

      if (fillSTM)
      {
         for (Integer j = 0; j < 6; ++j)
         {
            for (Integer k = 0; k < 6; ++k)
            {
               Integer element = j * 6 + k;
               deriv[i6+element] = 0.0;
               for (Integer l = 0; l < 6; ++l)
               {
                  deriv[i6+element] += aTilde[j*6+l] * state[i6+l*6+k];
               }
            }
         }
      }
   }
   return retval;
}


//------------------------------------------------------------------------------
// Real ODEModel::EstimateError(Real *diffs, Real *answer) const
//------------------------------------------------------------------------------
/**
 * Interface used to estimate the error in the current step
 * 
 * The method calculates the largest local estimate of the error from the 
 * integration given the components of the differences calculated bt the 
 * integrator.  It returns the largest error estimate found.  
 *   
 * The default implementation returns the largest single relative error 
 * component found based on the input arrays.  In other words, the 
 * implementation provided here returns the largest component of the following
 * vector:
 * 
 * \f[\vec \epsilon = |{{{EE}_n}\over{x_n^f - x_n^i}}|\f]
 *   
 * There are several alternatives that users of this class can implement: the 
 * error could be calculated based on the largest error in the individual 
 * components of the state vector, as the magnitude of the state vector (that 
 * is, the L2 (rss) norm of the error estimate vector).  The estimated error 
 * should never be negative, so a return value less than 0.0 can be used to 
 * indicate an error condition.
 *     
 * @param diffs         Array of differences calculated by the integrator.  This
 *                      array must be the same size as the state vector.
 * @param answer        Candidate new state from the integrator.
 */
//------------------------------------------------------------------------------
Real ODEModel::EstimateError(Real *diffs, Real *answer) const
{
   if (estimationMethod == ESTIMATE_IN_BASE)
      return PhysicalModel::EstimateError(diffs, answer);

   Real retval = 0.0, err, mag, vec[3];

   #ifdef DEBUG_ERROR_ESTIMATE
      MessageInterface::ShowMessage(wxT("ODEModel::EstimateError normType == %d; ")
            wxT("dimension = %d\n"), normType, dimension);
   #endif

   // Handle non-Cartesian state elements as an L1 norm
   for (int i = 0; i < cartesianStart; ++i)
   {
      // L1 norm
      mag = fabs(answer[ i ] - modelState[ i ]);
      err = fabs(diffs[i]);
      if (mag >relativeErrorThreshold)
         err = err / mag;

      #ifdef DEBUG_ERROR_ESTIMATE
         MessageInterface::ShowMessage(wxT("   {%d EstErr = %le} "), i, err);
      #endif

      if (err > retval)
      {
         retval = err;
      }
   }

   // Handle the Cartesian piece
   for (int i = cartesianStart; i < cartesianStart + cartStateSize; i += 3)
   {
      switch (normType)
      {
         case -2:
            // Code for the L2 norm, based on sep from central body
            vec[0] = 0.5 * (answer[ i ] + modelState[ i ]);
            vec[1] = 0.5 * (answer[i+1] + modelState[i+1]);
            vec[2] = 0.5 * (answer[i+2] + modelState[i+2]);

            mag = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
            err = diffs[i]*diffs[i] + diffs[i+1]*diffs[i+1] + diffs[i+2]*diffs[i+2];
            if (mag >relativeErrorThreshold)
               err = sqrt(err / mag);
            else
               err = sqrt(err);
            break;

         case -1:
            // L1 norm, based on sep from central body
            vec[0] = fabs(0.5 * (answer[ i ] + modelState[ i ]));
            vec[1] = fabs(0.5 * (answer[i+1] + modelState[i+1]));
            vec[2] = fabs(0.5 * (answer[i+2] + modelState[i+2]));

            mag = vec[0] + vec[1] + vec[2];
            err = fabs(diffs[i]) + fabs(diffs[i+1]) + fabs(diffs[i+2]);
            if (mag >relativeErrorThreshold)
               err = err / mag;
            break;

         case 0:         // Report no error here
            return 0.0;

         case 1:
            // L1 norm
            vec[0] = fabs(answer[ i ] - modelState[ i ]);
            vec[1] = fabs(answer[i+1] - modelState[i+1]);
            vec[2] = fabs(answer[i+2] - modelState[i+2]);

            mag = vec[0] + vec[1] + vec[2];
            err = fabs(diffs[i]) + fabs(diffs[i+1]) + fabs(diffs[i+2]);
            if (mag >relativeErrorThreshold)
               err = err / mag;
            break;

         case 2:
         default:
            // Code for the L2 norm
            vec[0] = answer[ i ] - modelState[ i ];
            vec[1] = answer[i+1] - modelState[i+1];
            vec[2] = answer[i+2] - modelState[i+2];

            mag = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
            err = diffs[i]*diffs[i] + diffs[i+1]*diffs[i+1] + diffs[i+2]*diffs[i+2];
            if (mag > relativeErrorThreshold)
               err = sqrt(err / mag);
            else
               err = sqrt(err);
      }

      #ifdef DEBUG_ERROR_ESTIMATE
         MessageInterface::ShowMessage(wxT("   {%d EstErr = %le} "), i, err);
      #endif

      if (err > retval)
      {
         retval = err;
      }
   }

   // Handle non-Cartesian state elements as an L1 norm
   for (int i = cartesianStart + cartStateSize; i < dimension; ++i)
   {
      // L1 norm
      mag = fabs(answer[ i ] - modelState[ i ]);
      err = fabs(diffs[i]);
      if (mag >relativeErrorThreshold)
         err = err / mag;

      #ifdef DEBUG_ERROR_ESTIMATE
         MessageInterface::ShowMessage(wxT("   {%d EstErr = %le} "), i, err);
      #endif

      if (err > retval)
      {
         retval = err;
      }
   }

   #ifdef DEBUG_ERROR_ESTIMATE
      MessageInterface::ShowMessage(wxT("   >>> Estimated Error = %le\n"), retval);
   #endif

   return retval;
}


//---------------------------------------------------------------------------
// bool TakeAction(const wxString &action, const wxString &actionData = wxT(""))
//---------------------------------------------------------------------------
bool ODEModel::TakeAction(const wxString &action, const wxString &actionData)
{
   if (action == wxT("ClearDefaultForce"))
   {
      std::vector<PhysicalModel*>::iterator oldIter = forceList.end();
      for (std::vector<PhysicalModel*>::iterator i =  forceList.begin(); 
           i != forceList.end(); ++i)
      {
         wxString pmName = (*i)->GetName();
         if (pmName == wxT("_DefaultInternalForce_"))
            oldIter = i;
      }
      
      if (oldIter != forceList.end())
      {
         PhysicalModel *oldForce = *oldIter;
         forceList.erase(oldIter);
         
         #ifdef DEBUG_DEFAULT_FORCE
         MessageInterface::ShowMessage
            (wxT("ODEModel::TakeAction() deleting default force <%p>\n"), oldForce);
         #endif
         #ifdef DEBUG_MEMORY
         MemoryTracker::Instance()->Remove
            (oldForce, oldForce->GetName(), wxT("ODEModel::DeleteForce()"),
             wxT("deleting non-transient force of ") + oldForce->GetTypeName(), this);
         #endif
         delete oldForce;
      }
   }
   
   return true;
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const wxString &oldName, const wxString &newName)
//---------------------------------------------------------------------------
/*
 * Renames referenced object name.
 *
 * @param <type> type of the reference object.
 * @param <oldName> old name of the reference object.
 * @param <newName> new name of the reference object.
 *
 * @return always true to indicate RenameRefObject() was implemented.
 */
//---------------------------------------------------------------------------
bool ODEModel::RenameRefObject(const Gmat::ObjectType type,
                                 const wxString &oldName,
                                 const wxString &newName)
{
   // There are no renamable objects
   return true;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the ODEModel.
 *
 * @return clone of the ODEModel.
 *
 */
//------------------------------------------------------------------------------
GmatBase* ODEModel::Clone() const
{
   return (new ODEModel(*this));
}


//---------------------------------------------------------------------------
// void Copy(const GmatBase* orig)
//---------------------------------------------------------------------------
/**
 * Sets this object to match another one.
 * 
 * @param orig The original that is being copied.
 */
//---------------------------------------------------------------------------
void ODEModel::Copy(const GmatBase* orig)
{
   operator=(*((ODEModel *)(orig)));
}


//------------------------------------------------------------------------------
// virtual bool HasRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
bool ODEModel::HasRefObjectTypeArray()
{
   return true;
}


//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by this class.
 *
 * @return the list of object types.
 * 
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& ODEModel::GetRefObjectTypeArray()
{
   refObjectTypes.clear();
   refObjectTypes.push_back(Gmat::SPACE_POINT);
   return refObjectTypes;
}


//------------------------------------------------------------------------------
//  const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref objects used by the member forces.
 *
 * @param <type> The type of object desired, or Gmat::UNKNOWN_OBJECT for the
 *               full list.
 * 
 * @return the list of object names.
 * 
 */
//------------------------------------------------------------------------------
const StringArray& ODEModel::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   wxString pmName;
   StringArray pmRefs;
   
   forceReferenceNames.clear();
   
   // Provide point mass body names for validation checking
   if (type == Gmat::SPACE_POINT)
   {
      forceReferenceNames = BuildBodyList(wxT("PointMassForce"));
      
      // Add central body
      if (find(forceReferenceNames.begin(), forceReferenceNames.end(),
               centralBodyName) == forceReferenceNames.end())
         forceReferenceNames.push_back(centralBodyName);
      
      return forceReferenceNames;      
   }
   
   // Provide space object names for validation checking
//   if (type == Gmat::SPACEOBJECT)
//   {
//      for (std::vector<SpaceObject *>::iterator sc = spacecraft.begin();
//           sc != spacecraft.end(); ++sc)
//      {
//         forceReferenceNames.push_back((*sc)->GetName());
//      }
//      return forceReferenceNames;
//   }
   
   // Always grab these two:
   forceReferenceNames.push_back(wxT("EarthMJ2000Eq"));
   forceReferenceNames.push_back(wxT("EarthFixed"));
   
   // Do the base class call
   try
   {
      pmName = PhysicalModel::GetRefObjectName(type);
      if (find(forceReferenceNames.begin(), forceReferenceNames.end(), 
               pmName) != forceReferenceNames.end())
         forceReferenceNames.push_back(pmName);
   }
   catch (BaseException &)
   {
      // Do nothing
   }
   
   try
   {
      pmRefs = PhysicalModel::GetRefObjectNameArray(type);

      // Add them all to the list
      for (StringArray::iterator j = pmRefs.begin(); j != pmRefs.end(); ++j)
      {
         if (find(forceReferenceNames.begin(), forceReferenceNames.end(), 
                  *j) != forceReferenceNames.end())
            forceReferenceNames.push_back(pmName);
      }
   }
   catch (BaseException &)
   {
      // Do nothing
   }
   
   // Build the list of references
   for (std::vector<PhysicalModel*>::iterator i =  forceList.begin(); 
        i != forceList.end(); ++i)
   {
      try
      {
         pmName = (*i)->GetRefObjectName(type);
         if (find(forceReferenceNames.begin(), forceReferenceNames.end(), 
                  pmName) == forceReferenceNames.end())
            forceReferenceNames.push_back(pmName);
      }
      catch (BaseException &)
      {
         // Do nothing
      }
      
      try
      {
         pmRefs = (*i)->GetRefObjectNameArray(type);
         // Add them all to the list
         for (StringArray::iterator j = pmRefs.begin(); j != pmRefs.end(); ++j)
         {
            if (find(forceReferenceNames.begin(), forceReferenceNames.end(), 
                     *j) == forceReferenceNames.end())
               forceReferenceNames.push_back(*j);
         }
      }
      catch (BaseException &)
      {
         // Do nothing
      }
   }
   
   // Add central body
   if (find(forceReferenceNames.begin(), forceReferenceNames.end(),
            centralBodyName) == forceReferenceNames.end())
      forceReferenceNames.push_back(centralBodyName);
   
   #ifdef DEBUG_FORCE_REF_OBJ
      MessageInterface::ShowMessage(wxT("Reference object names for '%s'\n"), 
         instanceName.c_str());
      for (StringArray::iterator i = forceReferenceNames.begin(); 
           i != forceReferenceNames.end(); ++i)
         MessageInterface::ShowMessage(wxT("   %s\n"), i->c_str());
   #endif
      
   // and return it
   return forceReferenceNames;
}

//------------------------------------------------------------------------------
// void SetSolarSystem(SolarSystem *ss)
//------------------------------------------------------------------------------
/**
 * Sets the solar system pointer
 * 
 * @param ss Pointer to the solar system used in the modeling.
 */
//------------------------------------------------------------------------------
void ODEModel::SetSolarSystem(SolarSystem *ss)
{
   PhysicalModel::SetSolarSystem(ss);

   if (ss == NULL)
      MessageInterface::ShowMessage(wxT("Setting NULL solar system on %s\n"), 
         instanceName.c_str());
   
   if (solarSystem != NULL)
   {
      forceOrigin = solarSystem->GetBody(centralBodyName);
      
      if (forceOrigin == NULL) 
         throw ODEModelException(
            wxT("Force model origin (") + centralBodyName + 
            wxT(") was not found in the solar system"));

      for (std::vector<PhysicalModel*>::iterator i = forceList.begin();
           i != forceList.end(); ++i)
         (*i)->SetForceOrigin(forceOrigin);
   }
}

//------------------------------------------------------------------------------
//  bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                    const wxString &name)
//------------------------------------------------------------------------------
/**
 * Sets ref objects used by the member forces.
 *
 * @param <obj>  Pointer to the refence object.
 * @param <type> The type of object being set.
 * @param <name> Name of the reference object.
 * 
 * @return true if the object was set, false if not.
 * 
 * @note This method catches exceptions, and just returns true or false to
 *       indicate success or failure.
 */
//------------------------------------------------------------------------------
bool ODEModel::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                              const wxString &name)
{
   #ifdef DEBUG_FORCE_REF_OBJ
   MessageInterface::ShowMessage
      (wxT("ODEModel::SetRefObject() <%s> entered, obj=<%p><%s><%s>, type=%d, name='%s'\n"),
       GetName().c_str(), obj, obj ? obj->GetTypeName().c_str() : wxT("NULL"),
       obj ? obj->GetName().c_str() : wxT("NULL"), type, name.c_str());
   #endif
   
   bool wasSet = false;
   
   // Handle the CS pointers we always want
   if (name == wxT("EarthMJ2000Eq"))
   {
      if (type == Gmat::COORDINATE_SYSTEM)
         earthEq = (CoordinateSystem*)obj;
      else
         throw ODEModelException(
            wxT("Object named EarthMJ2000Eq is not a coordinate system."));
   }
   if (name == wxT("EarthFixed"))
   {
      if (type == Gmat::COORDINATE_SYSTEM)
         earthFixed = (CoordinateSystem*)obj;
      else
         throw ODEModelException(
            wxT("Object named EarthFixed is not a coordinate system."));
   }

   // Attempt to set the object for the base class    
   try
   {
      if (PhysicalModel::SetRefObject(obj, type, name))
         wasSet = true;
   }
   catch (BaseException &)
   {
      // Do nothing
   }
   
   // Build the list of references
   for (std::vector<PhysicalModel*>::iterator i =  forceList.begin(); 
        i != forceList.end(); ++i)
   {
      try
      {
         if ((*i)->SetRefObject(obj, type, name))
         {
            wasSet = true;
            if (type == Gmat::COORDINATE_SYSTEM)
            {
               CoordinateSystem *cs = (CoordinateSystem*)obj;
               if (cs->GetOriginName() == wxT(""))
               {
                  // Finish setting CS data
                  cs->SetOriginName((*i)->GetBodyName());
               }
            }
         } 
      }
      catch (BaseException &)
      {
         // Do nothing
      }
   }
   
   return (wasSet);
}


//------------------------------------------------------------------------------
// Integer GetParameterCount() const
//------------------------------------------------------------------------------
Integer ODEModel::GetParameterCount() const
{
   return parameterCount;
}


// Access methods 
//------------------------------------------------------------------------------
// wxString GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
wxString ODEModel::GetParameterText(const Integer id) const
{
   if (id >= PhysicalModelParamCount && id < ODEModelParamCount)
      return PARAMETER_TEXT[id - PhysicalModelParamCount];
   else
      return PhysicalModel::GetParameterText(id);
}

//------------------------------------------------------------------------------
// Integer GetParameterID(const wxString &str) const
//------------------------------------------------------------------------------
Integer ODEModel::GetParameterID(const wxString &str) const
{
   wxString alias = str;
    
   // Script document required two different names for the primary body
   // force descriptor
   if (alias == wxT("Gravity"))
      alias = wxT("PrimaryBodies");
   
   for (int i = PhysicalModelParamCount; i < ODEModelParamCount; i++)
   {
      if (alias == PARAMETER_TEXT[i - PhysicalModelParamCount])
      {
         return i;
      }
   }
   
   return PhysicalModel::GetParameterID(str);
}

//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
Gmat::ParameterType ODEModel::GetParameterType(const Integer id) const
{
   if (id >= PhysicalModelParamCount && id < ODEModelParamCount)
      return PARAMETER_TYPE[id - PhysicalModelParamCount];
   else
      return PhysicalModel::GetParameterType(id);
}

//------------------------------------------------------------------------------
// wxString GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
wxString ODEModel::GetParameterTypeString(const Integer id) const
{
   if (id >= PhysicalModelParamCount && id < ODEModelParamCount)
      return GmatBase::PARAM_TYPE_STRING[GetParameterType(id)];
   else
      return PhysicalModel::GetParameterTypeString(id);
}

//------------------------------------------------------------------------------
// bool IsParameterReadOnly(const Integer id) const
//------------------------------------------------------------------------------
bool ODEModel::IsParameterReadOnly(const Integer id) const
{
   if (id == COORDINATE_SYSTEM_LIST || id == DEGREE || id == ORDER ||
       id == POTENTIAL_FILE)
      return true;
   
   return PhysicalModel::IsParameterReadOnly(id);
}

//------------------------------------------------------------------------------
// bool IsParameterReadOnly(const wxString &label) const
//------------------------------------------------------------------------------
bool ODEModel::IsParameterReadOnly(const wxString &label) const
{
   if (label == PARAMETER_TEXT[COORDINATE_SYSTEM_LIST-PhysicalModelParamCount])
      return true;
   
   return PhysicalModel::IsParameterReadOnly(label);
}

//------------------------------------------------------------------------------
// wxString GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
wxString ODEModel::GetStringParameter(const Integer id) const
{
   switch (id)
   {
      case CENTRAL_BODY:
         return centralBodyName;
         
      case  DRAG:
      {
         // Find the drag force
         const PhysicalModel *pm = GetForce(wxT("DragForce"));
         // No drag force, return wxT("None")
         if (pm == NULL)
            return wxT("None");
         // Get the atmosphere model from the drag force
         Integer id = pm->GetParameterID(wxT("AtmosphereModel"));
         wxString am = pm->GetStringParameter(id);
         return am;
      }
      
//       case  SRP:
//       {
//          const PhysicalModel *pm = GetForce(wxT("SolarRadiationPressure"));
//          if (pm == NULL)
//             return wxT("Off");
//          return wxT("On");
//       }
      
      case ERROR_CONTROL:
         switch (normType)
         {
            case -2:
               return wxT("RSSState");
            case -1:
               return wxT("LargestState");
            case 0:
               return wxT("None");
            case 1:
               return wxT("LargestStep");
            case 2:
               return wxT("RSSStep");
            default:
               throw ODEModelException(wxT("Unrecognized error control method."));
         }
         break;

      case POTENTIAL_FILE:
         {
            // Get actual id
            GmatBase *owner = NULL;
            Integer actualId = GetOwnedObjectId(id, &owner);
            return owner->GetStringParameter(actualId);
         }
         
      default:
         return PhysicalModel::GetStringParameter(id);
   }
}

//------------------------------------------------------------------------------
// wxString GetStringParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString ODEModel::GetStringParameter(const wxString &label) const
{
    return GetStringParameter(GetParameterID(label));
}

//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const wxString &value)
//------------------------------------------------------------------------------
bool ODEModel::SetStringParameter(const Integer id, const wxString &value)
{
   #ifdef DEBUG_FM_SET
   MessageInterface::ShowMessage
      (wxT("ODEModel::SetStringParameter() entered, id=%d, value='%s'\n"),
       id, value.c_str());
   #endif
   
   switch (id)
   {
      case CENTRAL_BODY:
         centralBodyName = value;
         bodyName = centralBodyName;
         return true;
         
      case PRIMARY_BODIES:
         return false;
         
      case POINT_MASSES:
         return false;
         
      case  DRAG:
         return false;
         
      case  USER_DEFINED:
         return false;
         
//       case  SRP:
//          return false;
         
      case ERROR_CONTROL:
         if (value == wxT("RSSState"))
         {
            normType = -2;
            return true;
         }
         if (value == wxT("LargestState"))
         {
            normType = -1;
            return true;
         }
         if (value == wxT("None"))
         {
            normType = 0;
            return true;
         }
         if (value == wxT("LargestStep"))
         {
            normType = 1;
            return true;
         }
         if (value == wxT("RSSStep"))
         {
            normType = 2;
            return true;
         }
         throw ODEModelException(wxT("Unrecognized error control method."));
         
      case POTENTIAL_FILE:
         {
            // Get actual id
            GmatBase *owner = NULL;
            Integer actualId = GetOwnedObjectId(id, &owner);
            return owner->SetStringParameter(actualId, value);
         }
      default:
         return PhysicalModel::SetStringParameter(id, value);
    }
}

//------------------------------------------------------------------------------
// bool SetStringParameter(const wxString &label,const wxString &value)
//------------------------------------------------------------------------------
bool ODEModel::SetStringParameter(const wxString &label,
                                    const wxString &value)
{
    return SetStringParameter(GetParameterID(label), value);
}

//---------------------------------------------------------------------------
//  wxString GetOnOffParameter(const Integer id) const
//---------------------------------------------------------------------------
wxString ODEModel::GetOnOffParameter(const Integer id) const
{
   switch (id)
   {
   case SRP:
      {
         const PhysicalModel *pm = GetForce(wxT("SolarRadiationPressure"));
         if (pm == NULL)
            return wxT("Off");
         return wxT("On");
      }
   case RELATIVISTIC_CORRECTION:
      {
         const PhysicalModel *pm = GetForce(wxT("RelativisticCorrection"));
         if (pm == NULL)
            return wxT("Off");
         return wxT("On");
      }
   default:
      return PhysicalModel::GetOnOffParameter(id);
   }
}


//---------------------------------------------------------------------------
//  bool SetOnOffParameter(const Integer id, const wxString &value)
//---------------------------------------------------------------------------
bool ODEModel::SetOnOffParameter(const Integer id, const wxString &value)
{
   switch (id)
   {
   case SRP:
      return true;
   case RELATIVISTIC_CORRECTION:
      return true;
   default:
      return PhysicalModel::SetOnOffParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// wxString ODEModel::GetOnOffParameter(const wxString &label) const
//------------------------------------------------------------------------------
wxString ODEModel::GetOnOffParameter(const wxString &label) const
{
   return GetOnOffParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetOnOffParameter(const wxString &label, const wxString &value)
//------------------------------------------------------------------------------
bool ODEModel::SetOnOffParameter(const wxString &label, 
                                   const wxString &value)
{
   return SetOnOffParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// const StringArray& GetStringArrayParameter(const Integer id) const
//------------------------------------------------------------------------------
const StringArray& ODEModel::GetStringArrayParameter(const Integer id) const
{
   switch (id)
   {
   case PRIMARY_BODIES:
      return BuildBodyList(wxT("GravityField"));
   case POINT_MASSES:
      return BuildBodyList(wxT("PointMassForce"));
   case COORDINATE_SYSTEM_LIST:
      return BuildCoordinateList();
   case USER_DEFINED:
      return BuildUserForceList();

   default:
      return PhysicalModel::GetStringArrayParameter(id);
    }
}


//------------------------------------------------------------------------------
// StringArray& GetStringArrayParameter(const wxString &label) const
//------------------------------------------------------------------------------
const StringArray& ODEModel::GetStringArrayParameter(const wxString &label) const
{
    return GetStringArrayParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual Integer GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
Integer ODEModel::GetIntegerParameter(const Integer id) const
{
   switch (id)
   {
      case DEGREE:
      case ORDER:
         {
            // Get actual id
            GmatBase *owner = NULL;
            Integer actualId = GetOwnedObjectId(id, &owner);
            return owner->GetIntegerParameter(actualId);
         }
   default:
      return PhysicalModel::GetIntegerParameter(id);
   }
}


//------------------------------------------------------------------------------
// virtual Integer GetIntegerParameter(const wxString &label) const
//------------------------------------------------------------------------------
Integer ODEModel::GetIntegerParameter(const wxString &label) const
{
   return GetIntegerParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual Integer SetIntegerParameter(const Integer id, const Integer value)
//------------------------------------------------------------------------------
Integer ODEModel::SetIntegerParameter(const Integer id, const Integer value)
{
   #ifdef DEBUG_FM_SET
   MessageInterface::ShowMessage
      (wxT("ODEModel::SetIntegerParameter() id=%d, value=%d\n"), id, value);
   #endif
   
   switch (id)
   {
   case DEGREE:
   case ORDER:
      {
         // Get actual id
         GmatBase *owner = NULL;
         Integer actualId = GetOwnedObjectId(id, &owner);
         #ifdef DEBUG_FM_SET
         MessageInterface::ShowMessage
            (wxT("   Calling SetIntegerParameter() on '%s' of type '%s'\n"),
             owner->GetName().c_str(), owner->GetTypeName().c_str());
         #endif
         Integer outval = owner->SetIntegerParameter(actualId, value);
         return outval;
      }
   default:
      return GmatBase::SetIntegerParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// virtual Integer SetIntegerParameter(const wxString &label, const Integer value)
//------------------------------------------------------------------------------
Integer ODEModel::SetIntegerParameter(const wxString &label, const Integer value)
{
   return SetIntegerParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// const StringArray& BuildBodyList(wxString type) const
//------------------------------------------------------------------------------
const StringArray& ODEModel::BuildBodyList(wxString type) const
{
   static StringArray bodylist;
   bodylist.clear();
   
   // Run through list of forces, adding body names for GravityField instances
   std::vector<PhysicalModel*>::const_iterator i;
   
   wxString actualType = GetScriptAlias(type);
   
   for (i = forceList.begin(); i != forceList.end(); ++i) {
       if ((*i)->GetTypeName() == actualType) {
          bodylist.push_back((*i)->GetStringParameter(wxT("BodyName")));
       }
   }
   return bodylist;
}


//------------------------------------------------------------------------------
// const StringArray& BuildCoordinateList() const
//------------------------------------------------------------------------------
const StringArray& ODEModel::BuildCoordinateList() const
{
   static StringArray cslist;
   cslist.clear();
   
   // Run through list of forces, adding body names for GravityField instances
   std::vector<PhysicalModel*>::const_iterator i;
   
   for (i = forceList.begin(); i != forceList.end(); ++i) 
   {
      if ((*i)->GetTypeName() == wxT("GravityField")) 
      {
         // For now, only build the body fixed CS's in list because others already exist.
         // If ODEModel is created inside of GmatFunction, not all CS exist, so added
         // InputCoordinateSystem and TargetCoordinateSystem to list(loj: 2008.06.25)
         cslist.push_back((*i)->GetStringParameter(wxT("InputCoordinateSystem")));
         cslist.push_back((*i)->GetStringParameter(wxT("FixedCoordinateSystem")));
         cslist.push_back((*i)->GetStringParameter(wxT("TargetCoordinateSystem")));
      }
   }
   return cslist;
}

//------------------------------------------------------------------------------
// const StringArray& BuildUserForceList() const
//------------------------------------------------------------------------------
/**
 * Builds the list of forces in the force model that are not part of the default
 * set of forces.
 * 
 * @return The list of user forces.
 */
//------------------------------------------------------------------------------
const StringArray& ODEModel::BuildUserForceList() const
{
   static StringArray uflist;
   uflist.clear();

   std::vector<PhysicalModel*>::const_iterator i;
   for (i = forceList.begin(); i != forceList.end(); ++i) 
      if ((*i)->IsUserForce())
         uflist.push_back((*i)->GetTypeName());
   
   return uflist;
}


//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Accesses an internal object used in the ODEModel.
 * 
 * This method provides access to the forces used in the ODEModel.  It is used
 * to set and read the specific force attributes -- for example, the file name 
 * used for the full field (GravityField) model.
 * 
 * @param type Base class type for the requested object.  Must be set to
 *             Gmat::PHYSICAL_MODEL for this build.
 * @param name String used for the object.
 * 
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
GmatBase* ODEModel::GetRefObject(const Gmat::ObjectType type,
                                   const wxString &name)
{
   if (type != Gmat::PHYSICAL_MODEL)
       throw ODEModelException(
          wxT("Only forces are accessed in ODEModel::GetRefObject"));
   
   // Run through list of forces, adding body names for GravityField instances
   std::vector<PhysicalModel*>::const_iterator i;
   
   wxString actualType = GetScriptAlias(name);
   
   for (i = forceList.begin(); i != forceList.end(); ++i) {
       if ((*i)->GetTypeName() == actualType) {
          return *i;
       }
   }
   return NULL;
}

                                    
//------------------------------------------------------------------------------
// GmatBase* GetRefObject(const Gmat::ObjectType type, const wxString &name, 
//                        const Integer index)
//------------------------------------------------------------------------------
/**
 * Accesses an internal object used in the ODEModel.
 * 
 * This method provides access to the forces used in the ODEModel.  It is used
 * to set and read the specific force attributes -- for example, the file name 
 * used for the full field (GravityField) model.  This version of the method 
 * provides a mechanism to access more than one object with the same type and
 * name by using an index to reach later intances.
 * 
 * @param type Base class type for the requested object.  Must be set to
 *             Gmat::PHYSICAL_MODEL for this build.
 * @param name String used for the object.
 * @param index Index into the list of objects of this type.
 * 
 * @return A pointer to the object.
 */
//------------------------------------------------------------------------------
GmatBase* ODEModel::GetRefObject(const Gmat::ObjectType type, 
                                   const wxString &name, const Integer index)
{
   if (type != Gmat::PHYSICAL_MODEL)
       throw ODEModelException(
          wxT("Only forces are accessed in ODEModel::GetRefObject"));
   
   // Run through list of forces, adding body names for GravityField instances
   std::vector<PhysicalModel*>::const_iterator i;
   wxString actualType = GetScriptAlias(name);
   Integer j = 0;
   
   for (i = forceList.begin(); i != forceList.end(); ++i) {
       if ((*i)->GetTypeName() == actualType) {
          ++j;
          if (j == index)
             return *i;       // Ignore names for forces.
       }
   }
   return NULL;
}


//------------------------------------------------------------------------------
// ObjectArray& GetRefObjectArray(const wxString& typeString)
//------------------------------------------------------------------------------
/**
 * Accesses arrays of internal objects used in the ODEModel.
 * 
 * @param typeString String used for the objects.
 * 
 * @return A reference to the ObjectArray.
 */
//------------------------------------------------------------------------------
ObjectArray& ODEModel::GetRefObjectArray(const wxString& typeString)
{
   static ObjectArray objects;
   objects.clear();
   
   // Run through list of forces, adding body names for GravityField instances
   std::vector<PhysicalModel*>::const_iterator i;
   wxString actualType = GetScriptAlias(typeString);

   if (typeString == wxT("PhysicalModel")) {
      for (i = forceList.begin(); i != forceList.end(); ++i) {
         objects.push_back(*i);       // Ignore names for forces.
      }
   }
   else {
      for (i = forceList.begin(); i != forceList.end(); ++i) {
          if ((*i)->GetTypeName() == actualType) {
             objects.push_back(*i);       // Ignore names for forces.
          }
      }
   }
   return objects;
}


//------------------------------------------------------------------------------
// wxString BuildForceNameString(PhysicalModel *force)
//------------------------------------------------------------------------------
wxString ODEModel::BuildForceNameString(PhysicalModel *force)
{
   wxString retval = wxT("UnknownForce"), forceType = force->GetTypeName();
   
   if (forceType == wxT("DragForce"))
      retval = wxT("Drag");
   if (forceType == wxT("GravityField"))
      // Changed wxT("Gravity") to wxT("GravityField") to be consistent with type name (loj: 2007.01.25)
      retval = wxT("GravityField.") + force->GetStringParameter(wxT("BodyName"));
   if (forceType == wxT("PointMassForce"))
      retval = force->GetStringParameter(wxT("BodyName"));
   if (forceType == wxT("SolarRadiationPressure"))
      retval = wxT("SRP");
   if (forceType == wxT("RelativisticCorrection"))
      retval = wxT("RelativisticCorrection");
   
   // Add others here
   
   // Handle user defined forces
   if (force->IsUserForce())
      retval = force->GetName();
      
        #ifdef DEBUG_USER_FORCES
                MessageInterface::ShowMessage(wxT("Force type %s named '%s' %s a user force\n"),
        force->GetTypeName().c_str(), force->GetName().c_str(), 
        (force->IsUserForce() ? wxT("is") : wxT("is not")));
        #endif

        return retval;
}


//------------------------------------------------------------------------------
// void MoveToOrigin(Real newEpoch)
//------------------------------------------------------------------------------
/**
 * Transforms the state vector from the internal coordinate system to the force
 * model origin.
 */
//------------------------------------------------------------------------------
void ODEModel::MoveToOrigin(Real newEpoch)
{
#ifdef DEBUG_REORIGIN
   MessageInterface::ShowMessage(wxT("ODEModel::MoveToOrigin entered\n"));
#endif
   
#ifdef DEBUG_REORIGIN
   MessageInterface::ShowMessage(
         wxT("SatCount = %d, dimension = %d, stateSize = %d\n"),cartesianCount,
         dimension, stateSize);
   MessageInterface::ShowMessage(
         wxT("StatePointers: rawState = %p, modelState = %p\n"), rawState, 
         modelState);
   MessageInterface::ShowMessage(
       wxT("ODEModel::MoveToOrigin()\n   Input state: [ "));
   for (Integer i = 0; i < dimension; ++i)
      MessageInterface::ShowMessage(wxT("%lf "), rawState[i]); 
   MessageInterface::ShowMessage(wxT("]\n   model state: [ "));
   for (Integer i = 0; i < dimension; ++i)
      MessageInterface::ShowMessage(wxT("%lf "), modelState[i]);
   MessageInterface::ShowMessage(wxT("]\n\n"));
#endif
    
   memcpy(modelState, rawState, dimension*sizeof(Real));

   if (centralBodyName != j2kBodyName)
   {
      Rvector6 cbState, j2kState, delta;
      Real now = ((newEpoch < 0.0) ? epoch : newEpoch);
      cbState = forceOrigin->GetState(now);
      j2kState = j2kBody->GetState(now);

      delta = cbState - j2kState;

      for (Integer i = 0; i < cartesianCount; ++i)
      {
         Integer i6 = cartesianStart + i * 6;
         for (int j = 0; j < 6; ++j)
            modelState[i6+j] = rawState[i6+j] - delta[j];

         #ifdef DEBUG_REORIGIN
            MessageInterface::ShowMessage(
                wxT("ODEModel::MoveToOrigin()\n")
                wxT("   Input state: [%lf %lf %lf %lf %lf %lf]\n")
                wxT("   j2k state:   [%lf %lf %lf %lf %lf %lf]\n")
                wxT("   cb state:    [%lf %lf %lf %lf %lf %lf]\n")
                wxT("   delta:       [%lf %lf %lf %lf %lf %lf]\n")
                wxT("   model state: [%lf %lf %lf %lf %lf %lf]\n\n"),
                rawState[i6], rawState[i6+1], rawState[i6+2], rawState[i6+3],
                rawState[i6+4], rawState[i6+5],
                j2kState[0], j2kState[1], j2kState[2], j2kState[3], j2kState[4],
                j2kState[5],
                cbState[0], cbState[1], cbState[2], cbState[3], cbState[4],
                cbState[5],
                delta[0], delta[1], delta[2], delta[3], delta[4], delta[5],
                modelState[i6], modelState[i6+1], modelState[i6+2],
                modelState[i6+3], modelState[i6+4], modelState[i6+5]);
         #endif
      }
   }
   
   #ifdef DEBUG_REORIGIN
      MessageInterface::ShowMessage(
          wxT("   Move Complete\n   Input state: [ "));
      for (Integer i = 0; i < dimension; ++i)
         MessageInterface::ShowMessage(wxT("%lf "), rawState[i]);
      MessageInterface::ShowMessage(wxT("]\n   model state: [ "));
      for (Integer i = 0; i < dimension; ++i)
         MessageInterface::ShowMessage(wxT("%lf "), modelState[i]);
      MessageInterface::ShowMessage(wxT("]\n\n"));

      MessageInterface::ShowMessage(wxT("ODEModel::MoveToOrigin Finished\n"));
   #endif
}


//------------------------------------------------------------------------------
// void ReturnFromOrigin(Real newEpoch)
//------------------------------------------------------------------------------
/**
 * Transforms the state vector from the force model origin to the internal 
 * coordinate system.
 */
//------------------------------------------------------------------------------
void ODEModel::ReturnFromOrigin(Real newEpoch)
{
   #ifdef DEBUG_REORIGIN
      MessageInterface::ShowMessage(wxT("ODEModel::ReturnFromOrigin entered\n"));
   #endif

   memcpy(rawState, modelState, dimension*sizeof(Real));
   if (centralBodyName != j2kBodyName)
   {
      Rvector6 cbState, j2kState, delta;
      Real now = ((newEpoch < 0.0) ? epoch : newEpoch);
      cbState = forceOrigin->GetState(now);
      j2kState = j2kBody->GetState(now);

      delta = j2kState - cbState;


      for (Integer i = 0; i < cartesianCount; ++i)
      {
         Integer i6 = cartesianStart + i * 6;
         for (int j = 0; j < 6; ++j)
            rawState[i6+j] = modelState[i6+j] - delta[j];
            #ifdef DEBUG_REORIGIN
               MessageInterface::ShowMessage(
                   wxT("ODEModel::ReturnFromOrigin()\n   Input (model) state: [%lf %lf %lf %lf %lf")
                   wxT(" %lf]\n   j2k state:   [%lf %lf %lf %lf %lf %lf]\n")
                   wxT("   cb state:    [%lf %lf %lf %lf %lf %lf]\n")
                   wxT("   delta:       [%lf %lf %lf %lf %lf %lf]\n")
                   wxT("   raw state: [%lf %lf %lf %lf %lf %lf]\n\n"),
                   modelState[0], modelState[1], modelState[2], modelState[3], modelState[4],
                   modelState[5],
                   j2kState[0], j2kState[1], j2kState[2], j2kState[3], j2kState[4],
                   j2kState[5],
                   cbState[0], cbState[1], cbState[2], cbState[3], cbState[4],
                   cbState[5],
                   delta[0], delta[1], delta[2], delta[3], delta[4], delta[5],
                   rawState[0], rawState[1], rawState[2], rawState[3],
                   rawState[4], rawState[5]);
         #endif
      }
   }
}


//------------------------------------------------------------------------------
// void ReportEpochData()
//------------------------------------------------------------------------------
/**
 * Utility to help debug epoch issues.
 */
//------------------------------------------------------------------------------
void ODEModel::ReportEpochData()
{
//   if (epochFile.is_open())
//   {
//      epochFile.precision(16);
//         epochFile << epoch << wxT(" ") << elapsedTime;
//
//      epochFile << wxT(" Members ");
//      for (std::vector<PhysicalModel*>::iterator i = forceList.begin();
//           i != forceList.end(); ++i)
//      {
//         epochFile.precision(16);
//         epochFile << wxT(" ")
//                   << (*i)->GetRealParameter(satIds[0])
//                   << wxT(" ") << (*i)->GetTime();
//      }
//
//      epochFile << wxT(" Sats ") ;
//      for (std::vector<SpaceObject*>::iterator i = spacecraft.begin();
//           i != spacecraft.end(); ++i)
//      {
//         epochFile.precision(16);
//         epochFile << wxT(" ") << (*i)->GetEpoch();
//      }
//      epochFile << wxT("\n");
//   }
//   else
//      throw ODEModelException(
//         wxT("ODEModel::ReportEpochData: Attempting to write epoch data without ")
//         wxT("opening the data file."));

   // Changed 3/4/2010, DJC
   // Repurposed this method so it is not file based.  For some reason all but
   // the final exception above was commented out, making the method totally
   // useless.

   MessageInterface::ShowMessage(wxT("  Epoch: %.12lf, elapsedTime: %.12lf\n"),
            epoch, elapsedTime);
}


void ODEModel::SetPropStateManager(PropagationStateManager *sm)
{
   psm = sm;
}

void ODEModel::SetState(GmatState *gms)
{
   state = gms;

   #ifdef DEBUG_STATE
      MessageInterface::ShowMessage(wxT("Setting state with dimension %d to\n   ["),
               state->GetSize());
      for (Integer i = 0; i < state->GetSize()-1; ++i)
         MessageInterface::ShowMessage(wxT("%le, "), (*state)[i]);
      MessageInterface::ShowMessage(wxT("%le]\n"), (*state)[state->GetSize()-1]);
   #endif
}


//---------------------------------
// private methods
//---------------------------------

//------------------------------------------------------------------------------
// Integer GetOwnedObjectId(Integer id, GmatBase **owner) const
//------------------------------------------------------------------------------
/*
 * Retrieves owned object property id.
 *
 * @param  id  ODEModel parameter id for for getting owned object parameter id
 * @param  owner  The pointer to owner to set if id provided found from the owned object
 *
 * @return  parameter ID from the owned object
 */
//------------------------------------------------------------------------------
Integer ODEModel::GetOwnedObjectId(Integer id, GmatBase **owner) const
{
   Integer actualId = -1;
   GmatBase *ownedObj = NULL;
   
   if (numForces == 0)
      throw ODEModelException(wxT("ODEModel::GetOwnedObjectId() failed, Has empty force list"));
   
   for (Integer i=0; i<numForces; i++)
   {
      ownedObj = forceList[i];
      try
      {
         actualId = ownedObj->GetParameterID(GetParameterText(id));
         break;
      }
      catch (BaseException &)
      {
         throw;
      }
   }
   
   *owner = ownedObj;
   
   if (owner == NULL)
      throw ODEModelException(wxT("ODEModel::GetOwnedObjectId() failed, Owned force is NULL"));
   
   #ifdef DEBUG_FM_OWNED_OBJECT
   MessageInterface::ShowMessage
      (wxT("ODEModel::GetOwnedObjectId() returning %d, owner=<%p><%s><%s>\n"),
       actualId, *owner, (*owner)->GetTypeName().c_str(), (*owner)->GetName().c_str());
   #endif
   
   return actualId;
}

