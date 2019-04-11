
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
        new_map_item = PropertyMapItem(prop)
        
        self.children[label] = new_map_item
        return new_map_item, is_new
        
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
            if newly_inserted: self.pg.Append(new_prop)

        new_item.properties.append(property)
        self.pmap[new_prop] = new_item
        self.pset[property] = True
        

    def AddProperty(self, property, parent_property = None):
        self.inAddProperty += 1
        
        # add a cad.property, optionally to an existing wx.PGProperty
        if property.GetType() == cad.PROPERTY_TYPE_STRING:
            new_prop = wxpg.StringProperty(property.GetTitle(),value=property.GetString())
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
        p = event.GetProperty()
        properties = self.GetProperties(p)
        if properties == None:
            return

        cad.StartHistory()
        
        for property in properties:
            if property.GetType() == cad.PROPERTY_TYPE_STRING:
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