import cad

in_here = False

class Object(cad.BaseObject):
    # a wrapper for cad.BaseObject, implementing defaults for the methods
    # derive from this, as cad.BaseObject isn't quite as pythonic as I hoped, yet.
    def __init__(self):
        cad.BaseObject.__init__(self)
        cad.PyIncref(self)
        
    def GetTitle(self):
        return self.GetTypeString()
        
    def OnRenderTriangles(self):
        pass
        
    def GetColor(self):
        return cad.Color(0,0,0)
        
    def GetBox(self):
        return 0,0,0,0,0,0
    
    def GetProperties(self):
        return cad.BaseObject.GetBaseProperties(self)
    
    
class PyProperty(cad.Property):
    def __init__(self, title, value_name, object, recalculate = None):
        t = cad.PROPERTY_TYPE_INVALID
        a = getattr(object, value_name)
        if type(a) == bool: t = cad.PROPERTY_TYPE_CHECK
        elif type(a) == int: t = cad.PROPERTY_TYPE_INT
        elif type(a) == float: t = cad.PROPERTY_TYPE_DOUBLE
        elif type(a) == str: t = cad.PROPERTY_TYPE_STRING
        cad.Property.__init__(self, t, title, object)
        self.value_name = value_name
        self.title = title
        self.type = t
        self.recalc = recalculate
        self.pyobj = object

    def GetType(self):
        # why is this not using base class?
        return self.type

    def GetTitle(self):
        # why is this not using base class?
        return self.title
        
    def editable(self):
        # why is this not using base class?
        return True
        
    def SetBool(self, value):
        setattr(self.pyobj, self.value_name, value)
        if(self.recalc):self.recalc()
        
    def SetInt(self, value):
        setattr(self.pyobj, self.value_name, value)
        if(self.recalc):self.recalc()
        
    def SetFloat(self, value):
        setattr(self.pyobj, self.value_name, value)
        if(self.recalc):self.recalc()
        
    def SetStr(self, value):
        setattr(self.pyobj, self.value_name, value)
        if(self.recalc):self.recalc()
        
    def GetBool(self):
        return getattr(self.pyobj, self.value_name)
    
    def GetInt(self):
        return getattr(self.pyobj, self.value_name)
    
    def GetFloat(self):
        return getattr(self.pyobj, self.value_name)
    
    def GetStr(self):
        return getattr(self.pyobj, self.value_name)
    
    def MakeACopy(self, o):
        p = PyProperty(self.title, self.value_name, self.pyobj)
        return p    