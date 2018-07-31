from PropertiesCanvas import PropertiesCanvas
from PropertiesCanvas import PropertiesObserver
import cad

class ObjPropsObserver(PropertiesObserver):
    def __init__(self, window):
        PropertiesObserver.__init__(self, window)
        
    def OnSelectionChanged(self, added, removed):
        print('sel')
        self.window.objects = cad.GetSelectedObjects()
        self.window.RemoveAndAddAll()

    def OnModified(self, modified):
        self.window.RemoveAndAddAll()

# This is a property grid for listing a cad object's properties

class ObjPropsCanvas(PropertiesCanvas):
    def __init__(self, parent):
        PropertiesCanvas.__init__(self, parent)
        self.inRemoveAndAddAll = False
        self.objects = []     
        self.observer = ObjPropsObserver(self)
        cad.RegisterObserver(self.observer)
        cad.MessageBox('ObjPropsCanvas')
        
    def RemoveAndAddAll(self):
        print('RemoveAndAddAll')
        if self.inRemoveAndAddAll:
            cad.MessageBox('recursion = RemoveAndAddAll')
            return
        self.inRemoveAndAddAll = True
        self.ClearProperties()
        for object in self.objects:
            properties = object.GetProperties()
            for property in properties:
                self.AddProperty(property)
        self.inRemoveAndAddAll = False
        
       
        