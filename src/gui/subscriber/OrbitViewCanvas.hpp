//$Id: OrbitViewCanvas.hpp 9682 2011-07-07 16:47:48Z lindajun $
//------------------------------------------------------------------------------
//                              OrbitViewCanvas
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Linda Jun
// Created: 2003/11/25
/**
 * Declares OrbitViewCanvas for opengl plot.
 */
//------------------------------------------------------------------------------
#ifndef OrbitViewCanvas_hpp
#define OrbitViewCanvas_hpp

#include "ViewCanvas.hpp"
#include "gmatwxdefs.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "MdiGlPlotData.hpp"
#include "GuiInterpreter.hpp"
#include "SolarSystem.hpp"
#include "CoordinateSystem.hpp"
#include "CoordinateConverter.hpp"
#include "Rvector3.hpp"
#include "Rvector6.hpp"
#include "GLStars.hpp"

#include <map>

class OrbitViewCanvas: public ViewCanvas
{
public:
   OrbitViewCanvas(wxWindow *parent, const wxWindowID id = -1,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize, 
                   const wxString& name = wxT("OrbitViewCanvas"),
                   long style = 0);
   virtual ~OrbitViewCanvas();
   
   bool GetDrawXyPlane() { return mDrawXyPlane; }
   bool GetDrawEcPlane() { return mDrawEcPlane; }
   bool GetDrawSunLine() { return mDrawSunLine; }
   bool GetDrawAxes() { return mDrawAxes; }
   unsigned int GetXyPlaneColor() { return mXyPlaneColor; }
   unsigned int GetEcPlaneColor() { return mEcPlaneColor; }
   unsigned int GetSunLineColor() { return mSunLineColor; }
   void SetDrawStars(bool flag) { mDrawStars = flag; }
   void SetDrawConstellations(bool flag) { mDrawConstellations = flag; }
   void SetStarCount(int count) { mStarCount = count; }
   void SetDrawXyPlane(bool flag) { mDrawXyPlane = flag; }
   void SetDrawEcPlane(bool flag) { mDrawEcPlane = flag; }
   void SetDrawSunLine(bool flag) { mDrawSunLine = flag; }
   void SetDrawAxes(bool flag) { mDrawAxes = flag; }
   void SetXyPlaneColor(unsigned int color) { mXyPlaneColor = color; }
   void SetEcPlaneColor(unsigned int color) { mEcPlaneColor = color; }
   void SetSunLineColor(unsigned int color) { mSunLineColor = color; }
   
   // actions
   void ClearPlot();
   void RedrawPlot(bool viewAnimation);
   void ShowDefaultView();
   void DrawWireFrame(bool flag);
   void DrawXyPlane(bool flag);
   void DrawEcPlane(bool flag);
   void OnDrawAxes(bool flag);
   void OnDrawGrid(bool flag);
   void DrawInOtherCoordSystem(const wxString &csName);
   void GotoObject(const wxString &objName);
   void GotoOtherBody(const wxString &bodyName);
   void ViewAnimation(int interval, int frameInc = 30);

   void SetGlObject(const StringArray &objNames,
                    const UnsignedIntArray &objOrbitColors,
                    const std::vector<SpacePoint*> &objectArray);
   
   // CoordinateSystem
   void SetGlCoordSystem(CoordinateSystem *internalCs,
                         CoordinateSystem *viewCs,
                         CoordinateSystem *viewUpCs);
   
   // drawing options
   void SetGl3dDrawingOption(bool drawEcPlane, bool drawXyPlane,
                             bool drawWireFrame, bool drawAxes,
                             bool drawGrid, bool drawSunLine,
                             bool usevpInfo, bool drawStars,
                             bool drawConstellations, Integer starCount);
   
   // viewpoint
   void SetGl3dViewOption(SpacePoint *vpRefObj, SpacePoint *vpVecObj,
                          SpacePoint *vdObj, Real vscaleFactor,
                          const Rvector3 &vpRefVec, const Rvector3 &vpVec,
                          const Rvector3 &vdVec, const wxString &upAxis,
                          bool usevpRefVec, bool usevpVec, bool usevdVec);
   
   // user actions   
   void TakeAction(const wxString &action);
      
protected:
   
   // events
   void OnPaint(wxPaintEvent &event);
   void OnSize(wxSizeEvent &event);
   void OnMouse(wxMouseEvent &event);
   void OnKeyDown(wxKeyEvent &event);
   
   DECLARE_EVENT_TABLE();

private:
   
   static const int LAST_STD_BODY_ID;// = 10;
   static const int MAX_COORD_SYS;// = 10;
   static const wxString BODY_NAME[GmatPlot::MAX_BODIES];
   static const Real MAX_ZOOM_IN;// = 3700.0;
   static const Real RADIUS_ZOOM_RATIO;// = 2.2;
   static const Real DEFAULT_DIST;// = 30000.0;
      
   // stars and options
   GLStars *mStars;
   int mStarCount;
   bool mDrawStars;
   bool mDrawConstellations;
      
   // camera
   Camera mCamera;
   
   // light source
   Light mLight;
   
   // Data members used by the mouse
   GLfloat mfStartX, mfStartY;
   
   // Actual params of the window
   GLfloat mfLeftPos, mfRightPos, mfBottomPos, mfTopPos;
   
   // Camera rotations
   GLfloat mfCamRotXAngle, mfCamRotYAngle, mfCamRotZAngle;
   GLfloat mfCamSingleRotAngle, mfCamRotXAxis, mfCamRotYAxis, mfCamRotZAxis;
   
   // Camera translations
   GLfloat mfCamTransX, mfCamTransY, mfCamTransZ;
   
   // DJC added for "Up"   
   GLfloat mfUpAngle, mfUpXAxis, mfUpYAxis, mfUpZAxis;
   
   // view model
   bool mUseGluLookAt;
   
   // drawing options
   float mAxisLength;
   bool mDrawXyPlane;
   bool mDrawEcPlane;
   bool mDrawSunLine;
   bool mDrawAxes;
   
   // color
   unsigned int mXyPlaneColor;
   unsigned int mEcPlaneColor;
   unsigned int mSunLineColor;
   
   // mouse rotating
   float mQuat[4];     // orientation of object
   
   // zooming
   int   mLastMouseX;
   int   mLastMouseY;
   float mZoomAmount;
   float mMaxZoomIn;
   
   // Control Modes
   int mControlMode;  // 0 = rotate around center, 1 = 6DOF movement, 2 = 6DOF movement NASA/Dunn style
   int mInversion;    // 1 is true movement, -1 is inverted
   
   // initial viewpoint
   wxString mViewPointRefObjName;
   wxString mViewUpAxisName;
   
   // passed view definition object pointers
   SpacePoint *pViewPointRefObj;
   SpacePoint *pViewPointVectorObj;
   SpacePoint *pViewDirectionObj;
   
   Rvector3 mViewPointRefVector;
   Rvector3 mViewPointVector;
   Rvector3 mViewDirectionVector;
   Rvector6 mUpState;
   Real mViewScaleFactor;
   
   bool mUseInitialViewPoint;
   bool mUseViewPointRefVector;
   bool mUseViewPointVector;
   bool mUseViewDirectionVector;
   bool mCanRotateAxes;
   
   int mVpRefObjId;
   int mVpVecObjId;
   int mVdirObjId;
   Real mViewObjRadius;
      
   // object rotation
   Real mInitialLongitude;
   Real mInitialMha;
   Real mFinalLongitude;
   Real mFinalMha;
   Real mFinalLst;
   
   // default spacecraft drawing
   GLuint mGlList;
   
   // earth
   float mEarthRadius;
   
   // view
   GLfloat mfViewLeft;
   GLfloat mfViewRight;
   GLfloat mfViewTop;
   GLfloat mfViewBottom;
   GLfloat mfViewNear;
   GLfloat mfViewFar;
   
   float mDefaultRotXAngle;
   float mDefaultRotYAngle;
   float mDefaultRotZAngle;
   float mDefaultViewDist;
   float mCurrRotXAngle;
   float mCurrRotYAngle;
   float mCurrRotZAngle;
   float mCurrViewDist;
   
   // view point
   void SetDefaultViewPoint();
   void InitializeViewPoint();
   
   // texture
   void SetDefaultView();
   
   // view objects
   void SetupProjection();
   void SetupWorld();
   void ChangeView(float viewX, float viewY, float viewZ);
   void ChangeProjection(int width, int height, float axisLength);
   void TransformView();
   
   // light source
   void HandleLightSource();
   
   // drawing objects
   void DrawFrame();
   void DrawPlot();
   void DrawObjectOrbit(int frame);
   void DrawObjectTexture(const wxString &objName, int obj, int objId, int frame);
   void DrawObject(const wxString &objName, int obj);
   void DrawOrbitLines(int i, const wxString &objName, int obj, int objId);
   void DrawEquatorialPlane(UnsignedInt color);
   void DrawEclipticPlane(UnsignedInt color);
   void DrawSunLine();
   void DrawAxes();
   void DrawGridLines(int objId);
   
   // spacecraft model
   void DrawSpacecraft3dModel(Spacecraft *sc, int objId, int frame);
   
   // drawing stars
   void DrawStars();
   
   // for rotation
   void RotateBodyUsingAttitude(const wxString &objName, int objId);
   void RotateBody(const wxString &objName, int frame, int objId);
      
   // for coordinate system
   void UpdateRotateFlags();
   bool ConvertObjectData();
   void ConvertObject(int objId, int index);
   
   // for utility
   void ComputeLongitudeLst(Real time, Real x, Real y, Real *meanHourAngle,
                            Real *longitude, Real *localSiderealTime);
   
   // Linux specific fix
   #ifdef __WXGTK__
      bool hasBeenPainted;
   #endif
};

#endif
