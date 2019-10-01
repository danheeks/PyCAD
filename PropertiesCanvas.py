
import sys, time, math, os, os.path

import wx
import wx.adv
_ = wx.GetTranslation
import wx.propgrid as wxpg
import cad

class PropertyMapItem:
    def __init__(self, prop = None):
        self.prop = prop
        self.children = {}      # map from key string to PropertyMapItem
        self.properties = []    # list of cad.Property objects, when multiple objects selected, there may be multiple properties with the same name which get represented by a single item in the property grid

    def OnAddProperty(self, prop):
        label = prop.GetLabel()
        is_new = label not in self.children
        if is_new:
            new_map_item = PropertyMapItem(prop)
            self.children[label] = new_map_item
        return self.children[label], is_new
        
class PropertiesObserver(cad.Observer):
    def __init__(self, window):
        cad.Observer.__init__(self)
        self.window = window
        
    def OnAdded(self, added):
        self.window.Refresh()
        
    def OnRemoved(self, removed):
        self.window.Refresh()
        
    def OnModified(self, modified):
        self.window.Refresh()
        self.window.pg.Refresh()
        
    def OnSelectionChanged(self, added, removed):
        self.window.Refresh()
        
    def Freeze(self):
        self.window.Freeze()
        
    def Thaw(self):
        self.window.Thaw()
        
class SampleMultiButtonEditor(wxpg.PGTextCtrlEditor):
    def __init__(self):
        wxpg.PGTextCtrlEditor.__init__(self)

    def CreateControls(self, propGrid, property, pos, sz):
        # Create and populate buttons-subwindow
        buttons = wxpg.PGMultiButton(propGrid, sz)

        # Add two regular buttons
        buttons.AddButton("...")
        buttons.AddButton("A")
        # Add a bitmap button
        buttons.AddBitmapButton(wx.ArtProvider.GetBitmap(wx.ART_FOLDER))

        # Create the 'primary' editor control (textctrl in this case)
        wnd = super(SampleMultiButtonEditor, self).CreateControls(
                                   propGrid,
                                   property,
                                   pos,
                                   buttons.GetPrimarySize())
        wnd = wnd.m_primary

        # Finally, move buttons-subwindow to correct position and make sure
        # returned wxPGWindowList contains our custom button list.
        buttons.Finalize(propGrid, pos);

        # We must maintain a reference to any editor objects we created
        # ourselves. Otherwise they might be freed prematurely. Also,
        # we need it in OnEvent() below, because in Python we cannot "cast"
        # result of wxPropertyGrid.GetEditorControlSecondary() into
        # PGMultiButton instance.
        self.buttons = buttons

        return wxpg.PGWindowList(wnd, buttons)

    def OnEvent(self, propGrid, prop, ctrl, event):
        if event.GetEventType() == wx.wxEVT_COMMAND_BUTTON_CLICKED:
            buttons = self.buttons
            evtId = event.GetId()

            if evtId == buttons.GetButtonId(0):
                # Do something when the first button is pressed
                wx.LogDebug("First button pressed")
                return False  # Return false since value did not change
            if evtId == buttons.GetButtonId(1):
                # Do something when the second button is pressed
                wx.MessageBox("Second button pressed")
                return False  # Return false since value did not change
            if evtId == buttons.GetButtonId(2):
                # Do something when the third button is pressed
                wx.MessageBox("Third button pressed")
                return False  # Return false since value did not change

        return super(SampleMultiButtonEditor, self).OnEvent(propGrid, prop, ctrl, event)
        
class CustomTextCtrl(wx.TextCtrl):
    def __init__(self, parent, id, value, pos, width, style):
        wx.TextCtrl.__init__(self, parent, id, value, pos, width, style)
        self.Bind(wx.EVT_CHAR, self.onCharEvent)
 
    def onCharEvent(self, event):
        keycode = event.GetKeyCode()
        controlDown = event.CmdDown()
        altDown = event.AltDown()
        shiftDown = event.ShiftDown()
 
        if keycode == wx.WXK_RETURN:
            self.WriteText('\n')
        else:
            event.Skip()
        
class TrivialPropertyEditor(wxpg.PGEditor):
    """
    This is a simple re-creation of TextCtrlWithButton. Note that it does
    not take advantage of wx.TextCtrl and wx.Button creation helper functions
    in wx.PropertyGrid.
    """
    def __init__(self):
        wxpg.PGEditor.__init__(self)

    def Txt_Ent(self,event):
       msg1 = (str(self.tc.GetValue()))
       wx.MessageBox(msg1)

    def CreateControls(self, propgrid, property, pos, sz):
        """ Create the actual wxPython controls here for editing the
            property value.
            You must use propgrid.GetPanel() as parent for created controls.
            Return value is either single editor control or tuple of two
            editor controls, of which first is the primary one and second
            is usually a button.
        """
        try:
            x, y = pos
            w, h = sz
            h = 100

            s = property.GetDisplayedString();

            self.tc = CustomTextCtrl(propgrid.GetPanel(), wxpg.PG_SUBID1, s,
                             (x,y), (w ,h),
                             wx.TE_MULTILINE | wx.TE_PROCESS_ENTER)

            return wxpg.PGWindowList(self.tc)#, btn)
        except:
            import traceback
            print(traceback.print_exc())

    def UpdateControl(self, property, ctrl):
        ctrl.SetValue(property.GetDisplayedString())

    def DrawValue(self, dc, rect, property, text):
        if not property.IsValueUnspecified():
            print(property.GetDisplayedString())
            dc.DrawText(property.GetDisplayedString(), rect.x+5, rect.y)

    def OnEvent(self, propgrid, property, ctrl, event):
        """ Return True if modified editor value should be committed to
            the property. To just mark the property value modified, call
            propgrid.EditorsValueWasModified().
        """
        if not ctrl:
            return False

        evtType = event.GetEventType()

        if evtType == wx.wxEVT_COMMAND_TEXT_ENTER:
            self.tc.SetFocus()
            #if propgrid.IsEditorsValueModified():
            #    return True
        elif evtType == wx.wxEVT_COMMAND_TEXT_UPDATED:
            #
            # Pass this event outside wxPropertyGrid so that,
            # if necessary, program can tell when user is editing
            # a textctrl.
            event.Skip()
            event.SetId(propgrid.GetId())

            propgrid.EditorsValueWasModified()
            return False
        elif evtType == wx.wxEVT_COMMAND_BUTTON_CLICKED:
            wx.MessageBox("Button Clicked")

        return False

    def GetValueFromControl(self, property, ctrl):
        """ Return tuple (wasSuccess, newValue), where wasSuccess is True if
            different value was acquired successfully.
        """
        tc = ctrl
        textVal = tc.GetValue()

        if property.UsesAutoUnspecified() and not textVal:
            return (True, None)

        res, value = property.StringToValue(textVal, wxpg.PG_FULL_VALUE)

        # Changing unspecified always causes event (returning
        # True here should be enough to trigger it).
        if not res and value is None:
            res = True

        return (res, value)

    def SetValueToUnspecified(self, property, ctrl):
        ctrl.Remove(0,len(ctrl.GetValue()))

    def SetControlStringValue(self, property, ctrl, text):
        ctrl.SetValue(text)

    def OnFocus(self, property, ctrl):
        ctrl.SetSelection(-1,-1)




############################################################################
#
# MAIN PROPERTY GRID TEST PANEL
#
############################################################################

class PropertiesCanvas( wx.Panel ):

    def __init__( self, parent ):
        wx.Panel.__init__(self, parent, wx.ID_ANY)

        self.panel = wx.Panel(self, wx.ID_ANY)

        # Difference between using PropertyGridManager vs PropertyGrid is that
        # the manager supports multiple pages and a description box.
        self.pg = pg = wxpg.PropertyGrid(self.panel,
                        style=wxpg.PG_DEFAULT_STYLE | wxpg.PG_SPLITTER_AUTO_CENTER)
        
        self.topsizer = wx.BoxSizer(wx.VERTICAL)
        self.topsizer.Add(pg, 1, wx.EXPAND)

        self.ClearProperties() #  initialize data structures
        
        # Show help as tooltips
        pg.SetExtraStyle(wxpg.PG_EX_HELP_AS_TOOLTIPS)

        pg.Bind( wxpg.EVT_PG_CHANGED, self.OnPropGridChange )
        pg.Bind( wxpg.EVT_PG_PAGE_CHANGED, self.OnPropGridPageChange )
        pg.Bind( wxpg.EVT_PG_SELECTED, self.OnPropGridSelect )
        pg.Bind( wxpg.EVT_PG_RIGHT_CLICK, self.OnPropGridRightClick )

        self.panel.SetSizer(self.topsizer)
        self.topsizer.SetSizeHints(self.panel)
        
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.sizer.Add(self.panel, 1, wx.EXPAND)
        self.SetSizer(self.sizer)
        self.SetAutoLayout(True)
        
        self.inAddProperty = 0
        self.in_OnPropGridChange = False
                
        # Register editor class - needs only to be called once
        self.multiButtonEditor = SampleMultiButtonEditor()
        self.pg.RegisterEditor(self.multiButtonEditor)
        self.trivialPropertyEditor = TrivialPropertyEditor()
        self.pg.RegisterEditor(self.trivialPropertyEditor)
        
        self.EXTRA_TOOLBAR_HEIGHT = 7 # todo set to 14 for Linux

    def FindMapItem(self, p):
        if p in self.pmap:
            return self.pmap[p]
        return None

    def Append(self, parent_property, new_prop, property):
        if parent_property:
            item = self.pmap[parent_property]
            new_item, newly_inserted = item.OnAddProperty(new_prop)
            if newly_inserted:
                self.pg.AppendIn(parent_property, new_prop)
        else:
            new_item, newly_inserted = self.map.OnAddProperty(new_prop)
            if newly_inserted:
                self.pg.Append(new_prop)

        new_item.properties.append(property)
        self.pmap[new_prop] = new_item
        self.pset[property] = True

    def AddProperty(self, property, parent_property = None):
        self.inAddProperty += 1
        
        # add a cad.property, optionally to an existing wx.PGProperty
        if property.GetType() == cad.PROPERTY_TYPE_STRING:
            new_prop = wxpg.StringProperty(property.GetTitle(),value=property.GetString())
        elif property.GetType() == cad.PROPERTY_TYPE_LONG_STRING:
            #new_prop = wxpg.LongStringProperty("MultipleButtons", wxpg.PG_LABEL)
            new_prop = wxpg.StringProperty(property.GetTitle(), value=property.GetString())
        elif property.GetType() == cad.PROPERTY_TYPE_DOUBLE:
            new_prop = wxpg.FloatProperty(property.GetTitle(),value=property.GetDouble())
        elif property.GetType() == cad.PROPERTY_TYPE_LENGTH:
            new_prop = wxpg.FloatProperty(property.GetTitle(),value=property.GetDouble() / cad.GetUnits())
        elif property.GetType() == cad.PROPERTY_TYPE_INT:
            new_prop = wxpg.IntProperty(property.GetTitle(),value=property.GetInt())
        elif property.GetType() == cad.PROPERTY_TYPE_COLOR:
            col = property.GetColor()
            wcol = wx.Colour(col.red, col.green, col.blue)
            new_prop = wxpg.ColourProperty(property.GetTitle(),value=wcol)
        elif property.GetType() == cad.PROPERTY_TYPE_CHOICE:
            new_prop = wxpg.EnumProperty(property.GetTitle(), labels = property.GetChoices(), value = property.GetInt())
        elif property.GetType() == cad.PROPERTY_TYPE_CHECK:
            new_prop = wxpg.BoolProperty(property.GetTitle(),value=property.GetBool())
        elif property.GetType() == cad.PROPERTY_TYPE_LIST:
            new_prop = wxpg.StringProperty(property.GetTitle(), value='<composed>')
        elif property.GetType() == cad.PROPERTY_TYPE_FILE:
            new_prop = wxpg.FileProperty(property.GetTitle(),value=property.GetString())
        else:
            wx.MessageBox('invalid property type: ' + str(property.GetType()))
            return
        
        if not property.editable: new_prop.ChangeFlag(wxpg.PG_PROP_READONLY, True)
        self.Append(parent_property, new_prop, property)
        
        if property.GetType() == cad.PROPERTY_TYPE_LIST:
            plist = property.GetProperties()
            for p in plist:
                self.AddProperty(p, new_prop)
            new_prop.ChangeFlag(wxpg.PG_PROP_COLLAPSED, True)
        elif property.GetType() == cad.PROPERTY_TYPE_LONG_STRING:
            # Change property to use editor created in the previous code segment
            #self.pg.SetPropertyEditor("MultipleButtons", self.multiButtonEditor)
            self.pg.SetPropertyEditor(new_prop, "TrivialPropertyEditor")
                
        self.inAddProperty -= 1
       
    def ClearProperties(self):
        self.pg.Clear()
        self.map = PropertyMapItem() # root of a tree of properties
        self.pmap = {} # map of wx.PGProperty to PropertyMapItem
        self.pset = {} # a set of cad.Property objects ( as a dictionary )
        
    def GetProperties(self, p):
        item = self.FindMapItem(p)
        if item == None:
            return None
        return item.properties

    def OnPropGridChange(self, event):
        self.in_OnPropGridChange = True
        p = event.GetProperty()
        properties = self.GetProperties(p)
        if properties == None:
            return

        cad.StartHistory()
        
        for property in properties:
            if property.GetType() == cad.PROPERTY_TYPE_STRING or property.GetType() == cad.PROPERTY_TYPE_LONG_STRING:
                cad.ChangePropertyString(p.GetValue(), property)
            elif property.GetType() == cad.PROPERTY_TYPE_DOUBLE:
                cad.ChangePropertyDouble(p.GetValue(), property)
            elif property.GetType() == cad.PROPERTY_TYPE_LENGTH:
                cad.ChangePropertyLength(p.GetValue() * cad.GetUnits(), property)
            elif property.GetType() == cad.PROPERTY_TYPE_INT:
                cad.ChangePropertyInt(p.GetValue(), property)
            elif property.GetType() == cad.PROPERTY_TYPE_CHOICE:
                cad.ChangePropertyChoice(p.GetValue(), property)
            elif property.GetType() == cad.PROPERTY_TYPE_COLOR:
                c = p.GetValue()
                cad.ChangePropertyColor(cad.Color(c.red, c.green, c.blue), property)
            elif property.GetType() == cad.PROPERTY_TYPE_CHECK:
                cad.ChangePropertyCheck(p.GetValue(), property)
            elif property.GetType() == cad.PROPERTY_TYPE_LIST:
                pass
            elif property.GetType() == cad.PROPERTY_TYPE_FILE:
                cad.ChangePropertyString(p.GetValue(), property)
                
        #for changer in changers:
            #cad.DoUndoable(changer)
            
        cad.EndHistory()
        
        self.in_OnPropGridChange = False

    def OnPropGridSelect(self, event):
        p = event.GetProperty()
        if p:
            pass
            #self.log.write('%s selected\n' % (event.GetProperty().GetName()))
        else:
            pass
            #self.log.write('Nothing selected\n')

    def OnPropGridRightClick(self, event):
        p = event.GetProperty()
        if p:
            pass
            #self.log.write('%s right clicked\n' % (event.GetProperty().GetName()))
        else:
            pass
            #self.log.write('Nothing right clicked\n')

    def OnPropGridPageChange(self, event):
        index = self.pg.GetSelectedPage()
        pass
        #self.log.write('Page Changed to \'%s\'\n' % (self.pg.GetPageName(index)))