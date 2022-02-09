from PropertiesCanvas import PropertiesCanvas
from PropertiesCanvas import PropertiesObserver
import cad

class ObjPropsObserver(PropertiesObserver):
    def __init__(self, window):
        PropertiesObserver.__init__(self, window)
        
    def OnSelectionChanged(self, added, removed):
        self.window.objects = cad.GetSelectedObjects()
        #self.make_initial_properties_in_refresh = True
        self.window.RemoveAndAddAll()
        #self.make_initial_properties_in_refresh = False

    def OnModified(self, modified):
        self.window.RemoveAndAddAll()

# This is a property grid for listing a cad object's properties

class ObjPropsCanvas(PropertiesCanvas):
    def __init__(self, parent):
        PropertiesCanvas.__init__(self, parent)
        self.inRemoveAndAddAll = False
        self.objects = []     
        #self.make_initial_properties_in_refresh = False
        self.observer = ObjPropsObserver(self)
        cad.RegisterObserver(self.observer)
        
    def RemoveAndAddAll(self):
        if self.in_OnPropGridChange:
            return
        
        if self.inRemoveAndAddAll:
            cad.MessageBox('recursion = RemoveAndAddAll')
            return
        self.inRemoveAndAddAll = True
        
        self.ClearProperties()

        for object in self.objects:
            properties = object.GetProperties()
            if properties:
                for property in properties:
                    self.AddProperty(property)
        self.inRemoveAndAddAll = False

        
       
        