import cad
import geom

in_here = False
next_object_index = 1

class Object(cad.BaseObject):
    # a wrapper for cad.BaseObject, implementing defaults for the methods
    # derive from this, as cad.BaseObject isn't quite as pythonic as I hoped, yet.
    def __init__(self, type, id_named = False):
        cad.BaseObject.__init__(self, type)
        cad.PyIncref(self)
        self.box = geom.Box3D()
        self.id_named = id_named
        if self.id_named:
            self.title = self.TypeName()
            self.title_made_from_id = True

    def GetTitle(self):
        if self.id_named:
            if self.title_made_from_id:
                return self.TypeName() + ' ' + str(self.GetID())
            else:
                return self.title
        return self.GetTypeString()

    def GetTypeString(self):
        return 'Unknown'

    def GetProperties(self):
        return cad.BaseObject.GetBaseProperties(self)

    def GetBox(self):
        self.box = geom.Box3D()
        for object in self.GetChildren():
            if object.GetVisible():
                self.box.InsertBox(object.GetBox())
        return self.box

    def HasEdit(self):
        return False
    
    def CanAdd(self, object):
        return True
    
    def CanAddTo(self, owner):
        return True
    
    def PreferredPasteTarget(self):
        return None
    
    def WriteXml(self):
        if self.id_named:
            if not self.title_made_from_id:cad.SetXmlValue('title', self.title)
            cad.SetXmlValue('title_from_id', self.title_made_from_id)
        #if self.CallsObjListReadXml():
        cad.ObjList.WriteXml(self)
#        else:
#            cad.Object.WriteObjectXml(self)

    def CallsObjListReadXml(self):
        return False

    def CopyFrom(self, object):
        if self.id_named:
            self.title = object.title
            self.title_made_from_id = object.title_made_from_id
        for obj in object.GetChildren():
            self.Add(obj.MakeACopy())

    def ReadXml(self):
        if self.id_named:
            self.title = cad.GetXmlValue('title', self.title)
            self.title_made_from_id = cad.GetXmlBool('title_from_id', self.title_made_from_id)
        if self.CallsObjListReadXml():
            cad.ObjList.ReadXml(self)
        else:
            self.ReadObjectXml()

    def WriteDefaultValues(self):
        pass # gets called on ok from dialog

class PyChoiceProperty(cad.Property):
    def __init__(self, title, value_name, choices, object, alternative_values = None, recalculate = None):
        self.title = title # to do, remove this GetTitle should use base class method
        self.value_name = value_name
        self.choices = choices
        self.alternative_values = alternative_values
        self.recalc = recalculate
        self.pyobj = object
        cad.Property.__init__(self, cad.PROPERTY_TYPE_CHOICE, title, object)
        cad.PyIncref(self)
        
    def GetType(self):
        return cad.PROPERTY_TYPE_CHOICE

    def GetTitle(self):
        # why is this not using base class?
        return self.title

    def editable(self):
        # why is this not using base class?
        return True

    def SetInt(self, value):
        if self.alternative_values:
            value = self.alternative_values[value]
        setattr(self.pyobj, self.value_name, value)
        if(self.recalc):self.recalc()

    def GetChoices(self):
        return self.choices

    def GetInt(self):
        if self.alternative_values:
            value = getattr(self.pyobj, self.value_name)
            i = 0
            for v in self.alternative_values:
                if v == value:
                    return i
                i += 1
            return 0
        return getattr(self.pyobj, self.value_name)

class PyPropertyLength(cad.Property):
    def __init__(self, title, value_name, pyobj, heeksobj = None, recalculate = None):
        if heeksobj == None:
            heeksobj = pyobj
        t = cad.PROPERTY_TYPE_LENGTH
        cad.Property.__init__(self, t, title, heeksobj)
        self.value_name = value_name
        self.title = title
        self.type = t
        self.recalc = recalculate
        self.pyobj = pyobj
        cad.PyIncref(self)

    def GetType(self):
        # why is this not using base class?
        return self.type

    def GetTitle(self):
        # why is this not using base class?
        return self.title

    def editable(self):
        # why is this not using base class?
        return True

    def SetFloat(self, value):
        setattr(self.pyobj, self.value_name, value)
        if(self.recalc):self.recalc()

    def GetFloat(self):
        return getattr(self.pyobj, self.value_name)

class PyProperty(cad.Property):
    def __init__(self, title, value_name, object, recalculate = None):
        t = cad.PROPERTY_TYPE_INVALID
        a = getattr(object, value_name)
        tt = type(a)
        if tt == bool: t = cad.PROPERTY_TYPE_CHECK
        elif tt == int: t = cad.PROPERTY_TYPE_INT
        elif tt == float: t = cad.PROPERTY_TYPE_DOUBLE
        elif tt == str: t = cad.PROPERTY_TYPE_STRING
        elif tt == geom.Point:
            t = cad.PROPERTY_TYPE_LIST
            self.children = []
            self.children.append(PyPropertyLength('X', 'x', a, object))
            self.children.append(PyPropertyLength('Y', 'y', a, object))
        elif tt == geom.Point3D:
            t = cad.PROPERTY_TYPE_LIST
            self.children = []
            self.children.append(PyPropertyLength('X', 'x', a, object))
            self.children.append(PyPropertyLength('Y', 'y', a, object))
            self.children.append(PyPropertyLength('Z', 'z', a, object))
        cad.Property.__init__(self, t, title, object)
        self.value_name = value_name
        self.title = title
        self.type = t
        self.recalc = recalculate
        self.pyobj = object
        cad.PyIncref(self) # never delete any PyProperty objects. They are needed for undo/redo operations. To do - add these to a list belonging to the App and clear the list when the undo list is cleared.

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

    def SetString(self, value):
        setattr(self.pyobj, self.value_name, value)
        if(self.recalc):self.recalc()

    def GetBool(self):
        return getattr(self.pyobj, self.value_name)

    def GetInt(self):
        return getattr(self.pyobj, self.value_name)

    def GetFloat(self):
        return getattr(self.pyobj, self.value_name)

    def GetString(self):
        return getattr(self.pyobj, self.value_name)

    def MakeACopy(self, o):
        p = PyProperty(self.title, self.value_name, self.pyobj)
        return p

    def GetProperties(self):
        return self.children
