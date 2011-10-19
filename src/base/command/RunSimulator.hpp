//$Id: RunSimulator.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                         ClassName
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
// Author: Darrel J. Conway, Thinking Systems, Inc.
// Created: 2009/ /
//
/**
 * File description here.
 */
//------------------------------------------------------------------------------


#ifndef RunSimulator_hpp
#define RunSimulator_hpp

#include "RunSolver.hpp"

#include "PropSetup.hpp"
#include "Simulator.hpp"

/**
 * Mission Control Sequence Command that runs data simulation
 *
 * This command uses a simulator to generate simulated measurement data that can
 * be processed by GMAT's estimators.  The Simulator class contains the elements
 * that actually manage the simulation process, including a finite state machine
 * that walks the system through the process of data simulation.
 *
 * Simulated data generated by this command is generated through propagation
 * using a pre-configured propagator.  The ODEModel used in this process is set
 * prior to execution of the command.  That means that the simulated data
 * generated in this process does not include any transient forces.
 */
class RunSimulator : public RunSolver
{
public:
   RunSimulator();
   virtual ~RunSimulator();
   RunSimulator(const RunSimulator& rs);
   RunSimulator& operator=(const RunSimulator& rs);

   virtual GmatBase* Clone() const;

   virtual wxString GetRefObjectName(const Gmat::ObjectType type) const;
   virtual bool SetRefObjectName(const Gmat::ObjectType type,
         const wxString &name);
   virtual bool RenameRefObject(const Gmat::ObjectType type,
         const wxString &oldName, const wxString &newName);
   virtual const wxString& GetGeneratingString(Gmat::WriteMode mode,
         const wxString &prefix, const wxString &useName);

   virtual bool Initialize();
   virtual bool Execute();
   virtual void RunComplete();
   virtual bool TakeAction(const wxString &action,
                           const wxString &actionData = wxT(""));

   virtual GmatCommand* GetNext();

protected:
   /// The simulator that drives this process
   Simulator      *theSimulator;
   /// Flag indicating if command execution is started
   bool commandRunning;
//   /// Flag indicating if propagation is running an needs reentrance
//   bool commandPropagating;
   /// Flag indicating if command execution is done
   bool commandComplete;

   void PrepareToSimulate();
   void Propagate();
   void Calculate();
   void LocateEvent();
   void Simulate();
   void Finalize();
};

#endif /* RunSimulator_hpp */
