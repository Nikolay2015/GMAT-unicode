//$Id: DecoratedTree.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                                  DecoratedTree
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Author: Darrel J. Conway
// Created: 2003/10/12
//
// Developed by Thinking Systems, Inc. to support the internal Unit Test 
// framework.
//
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// 
/**
 * Implementation for the custom extension to the wxTree class.
 */
//------------------------------------------------------------------------------


// Class automatically generated by Dev-C++ New Class wizard


#include "DecoratedTree.hpp" // class's header file

#include "MessageInterface.hpp"


BEGIN_EVENT_TABLE(DecoratedTree, wxTreeCtrl)
   EVT_PAINT(DecoratedTree::OnPaint)
END_EVENT_TABLE()


/** 
 * Class constructor
 * 
 * The parameters specified for DecoratedTree uses the same parameters as 
 * wxTreeCtrl, its base class.  This allows users to treat the DecoratedTree as
 * a wxTreeCtrl with additional properties the programmer can access.
 * 
 * @param parent Parent window for the component
 * @param id     ID used for the component
 * @param pos    Location of the component in the parent window
 * @param size   Size of the component
 * @param style  style for the wxTreeCtrl base class
 * @param validator wxValidator used by the base class
 * @param name   wxWindow name for the control
 */
DecoratedTree::DecoratedTree(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                   const wxSize& size, long style,
                   const wxValidator& validator, const wxString& name) :
    wxTreeCtrl              (parent, id, pos, size, style, validator, name),
    initialized             (false),
    drawOutline             (false),
    drawBoxes               (true),
    boxCount                (0),
    boxWidth                (20),
    rowHeight               (14)
{
    offset = 5;
    int w, h;
    GetSize(&w, &h);
    SetSize(w-80, h);
}


/**
 *  class destructor
 */
DecoratedTree::~DecoratedTree()
{
}


/**
 * Method used to prepare the tree for addition of nodes
 */
void DecoratedTree::Initialize(void)
{
    if (!initialized)
    {
        AddRoot("Root");
        initialized = true;        
    }
}


/**
 * Dummy tree fill for testing the OnPaint command.  
 * 
 * This method builds a tree with 2 columns on the right, like used in GMAT
 */
void DecoratedTree::SetNodes(void)
{
    if (!initialized)
        Initialize();
    wxTreeItemId root = GetRootItem();
    AppendItem(root, "Propagate");
    AppendItem(root, "Maneuver");
    AppendItem(root, "Propagate");
    wxTreeItemId subroot = AppendItem(root, "Target");
    AppendItem(subroot, "Maneuver");
    AppendItem(subroot, "Propagate");
    AppendItem(subroot, "Maneuver");
    AppendItem(root, "Propagate");
    
 /*
    SetString( 1, " 0");
    SetString( 3, " 0");
    SetString( 5, " 0");
    SetString( 7, " 2");
    SetString( 9, " 0");
    SetString(11, " 0");
    SetString(13, " 2");
    SetString(15, " 0");

    SetString( 0, " 0");
    SetString( 2, " 0");
    SetString( 4, " 0");
    SetString( 6, " 2");
    SetString( 8, " 1");
    SetString(10, " 0");
    SetString(12, " 1");
    SetString(14, " 0");
*/
    SetString(-1, " 0");
    SetString(-1, " 0");
    SetString(-1, " 0");
    SetString(-1, " 0");
    SetString(-1, " 0");
    SetString(-1, " 0");
    SetString(-1, " 2");
    SetString(-1, " 2");
    SetString(-1, " 0");
    SetString(-1, " 1");
    SetString(-1, " 0");
    SetString(-1, " 0");
    SetString(-1, " 2");
    SetString(-1, " 1");
    SetString(-1, " 0");
    SetString(-1, " 0");
}


/**
 * Method to expand all of the tree nodes
 */
void DecoratedTree::ExpandAll(void)
{
    wxTreeItemId root = GetRootItem();
    ExpandAll(root);
}



/**
 * Method to expand all of the nodes beneath the input node.
 * 
 * @param root  Base node for the expansion.
 */
void DecoratedTree::ExpandAll(wxTreeItemId root)
{
   //long cookie;
   wxTreeItemIdValue  cookie;
   wxTreeItemId current = GetFirstChild(root, cookie);
    
   while (current > (wxTreeItemId)0) {
      Expand(current);
      ExpandAll(current);
      current = GetNextChild(root, cookie);
   }
}


/**
 * Method to find the node with the input text
 * 
 * @param str   The text of the desired node
 */
wxTreeItemId DecoratedTree::Find(wxString str)
{
    wxTreeItemId root = GetRootItem();
    return Find(str, root);
}


/**
 * Method to find the node with the input text, given a base node
 * 
 * @param str   The text of the desired node
 * @param root  The base node
 */
wxTreeItemId DecoratedTree::Find(wxString str, wxTreeItemId root)
{
   //long cookie;
   wxTreeItemIdValue  cookie;
   wxTreeItemId current = GetFirstChild(root, cookie), item;

   while (current > (wxTreeItemId)0) {
      if (str == GetItemText(current))
         return current;
        
      item = Find(str, current);
      if (item > (wxTreeItemId)0)
         return item;
        
      current = GetNextChild(root, cookie);
   }

   return current;
}


/**
 * Method to add a node to the tree
 * 
 * @param txt       The text of the new node
 * @param parent    Text of the node above the new node
 */
wxTreeItemId DecoratedTree::AddItem(wxString txt, wxString parent)
{
    if (!initialized)
        Initialize();
    wxTreeItemId root = GetRootItem();

    if (parent != "")
        root = Find(parent, root);

    return AppendItem(root, txt);
}


/**
 * Method used to make the repaints behave correctly on all 3 platforms
 */
void DecoratedTree::OnPaint(wxPaintEvent& ev)
{
    // wxGetApp().Yield();
    ev.Skip();
    
    wxTreeItemId id = GetRootItem();
    
    if (drawOutline) {
        lineNumber = 0;
        DrawOutline(id);
    }
        
    if (drawBoxes) {
        lineNumber = 0;
        DrawBoxes(id);
    }
}


// Protected methods

/**
 * Method used to draw the box around a specified node
 * 
 * @param id    The node that needs the box
 */
void DecoratedTree::DrawOutline(wxTreeItemId id)
{
    // Add the new decorations
    wxRect bound;
    int w, h;
    bool visible;
    wxTreeItemIdValue cookie;
    wxTreeItemId current = GetFirstChild(id, cookie);
    
    GetSize(&w, &h);
    wxClientDC dc(this);

    //wxWidgets-2.6.3:deprecated->dc.BeginDrawing();
    
    while (current > (wxTreeItemId)0) {
        visible = GetBoundingRect(current, bound, TRUE);
        dc.SetPen(wxPen(wxColour(0xaf, 0xaf, 0xaf), 1, wxSOLID));

        // Top and bottom
        dc.DrawLine(bound.x, bound.y-1, w-offset, bound.y-1);
        dc.DrawLine(bound.x, bound.y+rowHeight+1, w-offset, bound.y+rowHeight+1);
        dc.DrawLine(bound.x, bound.y-1, bound.x, bound.y+rowHeight+1);
        dc.DrawLine(w-offset, bound.y+1, w-offset, bound.y+rowHeight+1);

//          dc.DrawLine(4, bound.y-1, w-20, bound.y-1);
//          dc.DrawLine(4, bound.y+rowHeight+1, w-20, bound.y+rowHeight+1);
//          dc.DrawLine(4, bound.y-1, 4, bound.y+rowHeight+1);
//          dc.DrawLine(w-20, bound.y+1, w-20, bound.y+rowHeight+1);
        
        DrawOutline(current);
        current = GetNextChild(id, cookie);
    }
    
    //wxWidgets-2.6.3:deprecated->dc.EndDrawing();
}


/**
 * Method used to draw the box around the decorations
 * 
 * @param id    The node that needs the decoration box(es)
 */
void DecoratedTree::DrawBoxes(wxTreeItemId id)
{
//    MessageInterface::ShowMessage("DecoratedTree::DrawBoxes() entered\n");

    // Add the new decorations
    wxRect bound;
    int w, h;
    int lft, rt, top, btm;
    bool visible;
    wxTreeItemIdValue cookie;
    wxTreeItemId current = GetFirstChild(id, cookie);
    
    GetSize(&w, &h);

    wxClientDC dc(this);

    //wxWidgets-2.6.3:deprecated->dc.BeginDrawing();
    
    while (current > (wxTreeItemId)0) {
       visible = GetBoundingRect(current, bound, TRUE);
       dc.SetPen(wxPen(wxColour(0xaf, 0xaf, 0xaf), 1, wxSOLID));

       if ((visible)) {                // Draw decorations on visible nodes
          for (int i = 0; i < boxCount; ++i) {
             //lft = w-20-boxWidth*(i+1);
             //rt  = w-20-boxWidth*i;
             lft = w-offset-boxWidth*(i+1);
             rt  = w-offset-boxWidth*i;
             top = bound.y;
             btm = bound.y + rowHeight;
                
             dc.DrawLine(lft, top, lft, btm);
             dc.DrawLine(rt, top, rt, btm);
             dc.DrawLine(lft, top, rt, top);
             dc.DrawLine(lft, btm, rt, btm);
             if (!boxData.empty())
             {
                //MessageInterface::ShowMessage("Box not empty\n");
                // Should be DrawText, but the wx build for Dev-C++ has a
                // bug with that method
                //dc.DrawRotatedText("Ready", lft+2, top, 0);
                int boxDataNumber = i+lineNumber*boxCount;
                //MessageInterface::ShowMessage("Box data number %d\n", boxDataNumber);
                    
                if (boxDataNumber < (int)boxData.size())
                {
                   if (boxData[boxDataNumber])
                   {
                      dc.DrawRotatedText(boxData[i+lineNumber*boxCount]->c_str(),
                                         lft+2, top, 0);
                      //MessageInterface::ShowMessage
                      //("Drew text\nline number=%d\nboxcount=%d\ni=%d\n", lineNumber, boxCount, i);
                   }
                }
             }
          }
       }
       
       if (GetChildrenCount(current) > 0) {
          ++lineNumber;
          DrawBoxes(current);
          --lineNumber;
       }
       
        current = GetNextChild(id, cookie);
        ++lineNumber;
    }

    //MessageInterface::ShowMessage("end drawing\n");
    //wxWidgets-2.6.3:deprecated->dc.EndDrawing();
}


/**
 * Generic method used to change the properties of the decorations
 * 
 * @param id    ID for the parameter being set
 * @param value The new value for the parameter
 */
void DecoratedTree::SetParameter(int id, int value)
{
    switch (id) {
        case DRAWOUTLINE:
            if (value != 0)
                drawOutline = true;
            else
                drawOutline = false;
            break;
            
        case DRAWBOXES:
            if (value != 0)
                drawBoxes = true;
            else
                drawBoxes = false;
            break;
            
        case BOXCOUNT:
            if (value > 0)
                boxCount = value;
            break;
            
        case BOXWIDTH:
            if (value > 1)
                boxWidth = value;
            break;
            
        default:
            break;
    }
}


/**
 * Generic method used to access the properties of the decorations
 * 
 * @param id    ID for the parameter being set
 * 
 * @return      The (integer) value for the parameter
 */
int DecoratedTree::GetParameter(int id)
{
    switch (id) {
        case DRAWOUTLINE:
            if (drawOutline)
                return 1;
            else
                return 0;
            
        case DRAWBOXES:
            if (drawBoxes)
                return 1;
            else
                return 0;

        case BOXCOUNT:
            return boxCount;
            
        case BOXWIDTH:
            return boxWidth;
            
        default:
            break;
    }
    
    return -1;
}


/**
 * Method used to set the text for a decoration
 * 
 * @param line  ID for the decoration being set
 * @param value The new text for the decoration
 * 
 * @bug Use of the line parameter is confusing when there are more tha one 
 * column of decorations; the control fills the data a column at a time, but it
 * may be better to provide a SetString method that is easier to use.
 */
void DecoratedTree::SetString(int line, std::string value)
{
//    MessageInterface::ShowMessage("DecoratedTree::SetString() entered\n");

    if (line < 0)
        boxData.push_back(new std::string(value));
    else {
        std::string *str = boxData[line];
        *str = value;
    }
//   MessageInterface::ShowMessage("DecoratedTree::SetString() exit\n");

}

