import cad

class Object(cad.BaseObject):
    # a wrapper for cad.BaseObject, implementing defaults for the methods
    # derive from this, as cad.BaseObject isn't quite as pythonic as I hoped, yet.
    def __init__(self):
        cad.BaseObject.__init__(self)
        #cad.PyIncref(self)
        
    def GetTitle(self):
        return self.GetTypeString()
        
    def OnRenderTriangles(self):
        pass
    
    def GetProperties(self):
        return []
        
    def GetColor(self):
        return cad.Color(0,0,0)
        
    def GetBox(self):
        return 0,0,0,0,0,0