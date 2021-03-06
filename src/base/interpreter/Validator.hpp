//$Id: Validator.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                                  Validator
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CCA54C
//
// Author: Linda Jun
// Created: 2008/04/01 (Made separate class out of Interpreter::CreateElementWrapper())
//
// Original Wrapper code from the Interpreter.
// Author: Wendy C. Shoan
//
/**
 * Definition for the Validator class.
 * This class validates objects command and builds ElemementWrappers for
 * various data types.
 */
//------------------------------------------------------------------------------
#ifndef Validator_hpp
#define Validator_hpp

#include "gmatdefs.hpp"

// Forward references
class Interpreter;
class Moderator;
class SolarSystem;
class GmatCommand;
class Function;
class Parameter;
class AxisSystem;
class ElementWrapper;

class GMAT_API Validator
{
public:
   
   static Validator* Instance();
   
   void SetInterpreter(Interpreter *interp);
   void SetSolarSystem(SolarSystem *ss);
   void SetObjectMap(ObjectMap *objMap);
   void SetFunction(Function *func);
   
   bool StartMatlabServer(GmatCommand *cmd);
   bool CheckUndefinedReference(GmatBase *obj, bool contOnError = true);
   bool ValidateCommand(GmatCommand *cmd, bool contOnError = true,
                        Integer manage = 1);
   
   void HandleCcsdsEphemerisFile(ObjectMap *objMap, bool deleteOld = false);
   
   ElementWrapper* CreateElementWrapper(const wxString &desc,
                                        bool parametersFirst = false,
                                        Integer manage = 1);
   const StringArray& GetErrorList();
   
   GmatBase* FindObject(const wxString &name, 
                        const wxString &ofType = wxT(""));
   Parameter* GetParameter(const wxString name);
   Parameter* CreateParameter(const wxString &type,
                              const wxString &name,
                              const wxString &ownerName = wxT(""),
                              const wxString &depName = wxT(""),
                              Integer manage = 1);
   Parameter* CreateAutoParameter(const wxString &type,
                                  const wxString &name,
                                  bool &alreadyManaged,
                                  const wxString &ownerName = wxT(""),
                                  const wxString &depName = wxT(""),
                                  Integer manage = 1);
   Parameter* CreateArray( const wxString &arrayStr, Integer manage = 1);   
   Parameter* CreateSystemParameter(bool &paramCreated, const wxString &str,
                                    Integer manage = 1);
   AxisSystem* CreateAxisSystem(wxString type, GmatBase *owner);
   
   bool IsParameterType(const wxString &desc);
   bool ValidateParameter(const StringArray &refNames, GmatBase *obj);
   bool ValidateSaveCommand(GmatBase *obj);
   
private:

   bool CreateAssignmentWrappers(GmatCommand *cmd, Integer manage);
   
   ElementWrapper* CreateSolarSystemWrapper(GmatBase *obj, const wxString &owner,
                                            const wxString &depobj,
                                            const wxString &type,
                                            Integer manage = 1);
   ElementWrapper* CreateForceModelWrapper(GmatBase *obj, const wxString &owner,
                                           const wxString &depobj,
                                           const wxString &type,
                                           Integer manage = 1);
   ElementWrapper* CreateWrapperWithDot(bool parametersFirst, Integer manage = 1);
   ElementWrapper* CreateValidWrapperWithDot(GmatBase *obj,
                                             const wxString &type,
                                             const wxString &owner,
                                             const wxString &depobj,
                                             bool parametersFirst,
                                             Integer manage);
   ElementWrapper* CreateOtherWrapper(Integer manage = 1);
   ElementWrapper* CreateParameterWrapper(Parameter *param,
                                          Gmat::WrapperDataType &itsType);
   ElementWrapper* CreatePropertyWrapper(GmatBase *obj, const wxString &type,
                                         Integer manage, bool checkSubProp = true);
   ElementWrapper* CreateSubPropertyWrapper(GmatBase *obj,
                                            const wxString &type,
                                            Integer manage);
   
   bool ValidateSubCommand(GmatCommand *cmd, Integer level, Integer manage = 1);
   bool CreateCoordSystemProperty(GmatBase *obj, const wxString &prop,
                                  const wxString &value);
   bool CreatePropSetupProperty(GmatBase *obj, const wxString &prop,
                                const wxString &value);
   bool CreateForceModelProperty(GmatBase *obj, const wxString &prop,
                                 const wxString &value);
   bool HandleError(bool addFunction = true);
   void ShowObjectMap(const wxString &label = wxT(""));
   
   Interpreter     *theInterpreter;
   Moderator       *theModerator;
   SolarSystem     *theSolarSystem;
   GmatCommand     *theCommand;
   Function        *theFunction;
   ObjectMap       *theObjectMap;
   StringArray     theParameterList;
   
   wxString     theDescription;
   /// If object not found, it will automatically create StringWrapper
   /// This flag indicates whether or not to create default StringWrapper
   bool            createDefaultStringWrapper;
   
   /// Error handling data
   bool            continueOnError;
   StringArray     theErrorList;
   wxString     theErrorMsg;
   
   static Validator *instance;
   Validator();
   ~Validator();      
};

#endif // Validator_hpp
