import wx
import cad

class DoubleCtrl(wx.TextCtrl):
    def __init__(self, parent, id = wx.ID_ANY, factor = 1.0):
        wx.TextCtrl.__init__(self, parent, id)
        self.factor = factor
        
    def GetValue(self):
        try:
            return float(wx.TextCtrl.GetValue(self))/self.factor
        except:
            return 0.0

    def DoubleToString(self, value):
        return str(value * self.factor)
    
    def SetValue(self, value):
        wx.TextCtrl.SetValue(self, self.DoubleToString(value))
        
class LengthCtrl(DoubleCtrl):
    '''
    Use in a dialog
    '''
    def __init__(self, parent, id = wx.ID_ANY):
        '''
        more doc
        '''
        factor = 1.0/cad.GetViewUnits()
        DoubleCtrl.__init__(self, parent, id, factor)
        
class ObjectIdsCtrl(wx.TextCtrl):
    def __init__(self, parent):
        wx.TextCtrl.__init__(self, parent, wx.ID_ANY)
        
    def GetIdList(self):
        id_list = []
        strs = wx.TextCtrl.GetValue(self).split()
        for s in strs:
            id_list.append(int(s))
        return id_list
    
    def SetFromIdList(self, id_list):
        first = True
        s = ""
        for id in id_list:
            if first == False:
                s = s + " "
            else:
                first = False
            s += str(id)
        wx.TextCtrl.SetValue(self, s)
        
class GeomCtrl(wx.TextCtrl):
    # for now this is just a list of profile names with quotes around them and spaces between them, but later it might have a diagram showing the geometry
    def __init__(self, parent, id):
        wx.TextCtrl.__init__(self, parent, id)
        
    def GetGeomList(self):
        str = wx.TextCtrl.GetValue(self)
        str = str.replace('\\', '/')
        s = ""
        geom_list = []
        length = len(str)
        name_started = False
        for i in range(0, length):
            if str[i] == '"':
                if name_started:
                    geom_list.append(s)
                    s = ""
                    name_started = False
                else:
                    name_started = True
            elif str[i] == " " and (name_started == False):
                if len(s)>0:
                    geom_list.append(s)
                    s = ""
            else:
                s += str[i]
        if len(s)>0:
            geom_list.append(s)
            s = ""
        return geom_list
    
    def SetFromGeomList(self, geom_list):
        first = True
        str = ""
        for geom in geom_list:
            if first == False:
                str = str + " "
            else:
                first = False
            str += geom
        wx.TextCtrl.SetValue(self, str)
        
