import math
import cad
import geom
import wx
from HeeksConfig import HeeksConfig
from HDialog import HDialog
from NiceTextCtrl import LengthCtrl

def RemoveUncopyable():
    uncopyable_objects = []
    for object in cad.GetSelectedObjects():
        if not object.CanBeCopied():
            uncopyable_objects.append(object)
            
    for object in uncopyable_objects:
        cad.Unselect(object)
        
class FromAndToDlg(HDialog):
    def __init__(self, fromp, to, ncopies, title):
        HDialog.__init__(self, title)
        self.ignore_event_functions = True
        sizerMain = wx.BoxSizer(wx.VERTICAL)
        
        self.number_of_copies = ncopies
        self.fromp = fromp
        self.to = to
        
        if ncopies:
            self.txt_num_copies = wx.TextCtrl(self, wx.ID_ANY, str(ncopies))
            self.AddLabelAndControl(sizerMain, 'Number of Copies', self.txt_num_copies)
            
        sizerPos = wx.BoxSizer(wx.VERTICAL)
        sizerMain.Add(sizerPos, 0, wx.EXPAND)
        self.btnFrom = wx.Button(self, label = 'Select')
        self.AddLabelAndControl(sizerPos, 'Position to move from', self.btnFrom)
        self.fromx = LengthCtrl(self)
        self.AddLabelAndControl(sizerPos, 'X', self.fromx)
        self.fromy = LengthCtrl(self)
        self.AddLabelAndControl(sizerPos, 'Y', self.fromy)
        self.fromz = LengthCtrl(self)
        self.AddLabelAndControl(sizerPos, 'Z', self.fromz)
        
        self.fromx.SetValue(fromp.x)
        self.fromy.SetValue(fromp.y)
        self.fromz.SetValue(fromp.z)
            
        sizerPos2 = wx.BoxSizer(wx.VERTICAL)
        sizerMain.Add(sizerPos2, 0, wx.EXPAND)
        self.btnTo = wx.Button(self, label = 'Select')
        self.AddLabelAndControl(sizerPos, 'Position to move to', self.btnTo)
        self.tox = LengthCtrl(self)
        self.AddLabelAndControl(sizerPos, 'X', self.tox)
        self.toy = LengthCtrl(self)
        self.AddLabelAndControl(sizerPos, 'Y', self.toy)
        self.toz = LengthCtrl(self)
        self.AddLabelAndControl(sizerPos, 'Z', self.toz)
        
        self.tox.SetValue(to.x)
        self.toy.SetValue(to.y)
        self.toz.SetValue(to.z)
        
        self.MakeOkAndCancel(wx.HORIZONTAL).AddToSizer(sizerMain)
        
        self.SetSizer(sizerMain)
        sizerMain.SetSizeHints(self)
        sizerMain.Fit(self)
        
        if ncopies:
            self.txt_num_copies.SetFocus()
        else:
            self.fromx.SetFocus()
            
        self.Bind(wx.EVT_BUTTON, self.OnPickFrom, self.btnFrom)
        self.Bind(wx.EVT_BUTTON, self.OnPickTo, self.btnTo)
            
        self.ignore_event_functions = False
        
    def OnPickFrom(self, event):
        self.EndModal(self.btnFrom.GetId())
        
    def OnPickTo(self, event):
        self.EndModal(self.btnTo.GetId())
        
    def GetAllValues(self):
        self.fromp.x = self.fromx.GetValue()
        self.fromp.y = self.fromy.GetValue()
        self.fromp.z = self.fromz.GetValue()
        self.to.x = self.tox.GetValue()
        self.to.y = self.toy.GetValue()
        self.to.z = self.toz.GetValue()
        ncopies = None
        if self.number_of_copies:
            ncopies = int(self.txt_num_copies.GetValue())
        return self.fromp, self.to, ncopies

def InputFromAndTo(fromp, to, ncopies, title):
    while(True):
        dlg = FromAndToDlg(fromp, to, ncopies, title)
        ret = dlg.ShowModal()
        fromp, to, ncopies = dlg.GetAllValues()
        
        if ret == wx.ID_OK:
            return True, fromp, to, ncopies
        if ret == dlg.btnFrom.GetId():
            fromp = wx.GetApp().PickPosition("Pick position to move from")
        elif ret == dlg.btnTo.GetId():
            to = wx.GetApp().PickPosition("Pick position to move to")
        else:
            return False

def Translate(copy = False):
    if cad.GetNumSelected() == 0:
        wx.GetApp().PickObjects('Pick objects to move')
    
    if cad.GetNumSelected() == 0:
        return
    
    config = HeeksConfig()
    ncopies = None
    if copy:
        ncopies = config.ReadInt('TranslateNumCopies', 1);
        RemoveUncopyable()
        if cad.GetNumSelected() == 0:
            return
        
    selected_items = cad.GetSelectedObjects()
    
    cad.ClearSelection(False)
    
    fromp = geom.Point3D()
    to = geom.Point3D()

    fromp.x = config.ReadFloat("TranslateFromX", 0.0)
    fromp.y = config.ReadFloat("TranslateFromY", 0.0)
    fromp.z = config.ReadFloat("TranslateFromZ", 0.0)
    to.x = config.ReadFloat("TranslateToX", 0.0)
    to.y = config.ReadFloat("TranslateToY", 0.0)
    to.z = config.ReadFloat("TranslateToZ", 0.0)
    
    result, fromp, to, ncopies = InputFromAndTo(fromp, to, ncopies, 'Move Translate')
    if not result:
        return
    
    if copy:
        if ncopies < 1:
            return
        config.WriteInt("TranslateNumCopies", ncopies)
        
    config.WriteFloat("TranslateFromX", fromp.x)
    config.WriteFloat("TranslateFromY", fromp.y)
    config.WriteFloat("TranslateFromZ", fromp.z)
    config.WriteFloat("TranslateToX", to.x)
    config.WriteFloat("TranslateToY", to.y)
    config.WriteFloat("TranslateToZ", to.z)
    
    cad.StartHistory()
    
    if copy:
        for i in range(0, ncopies):
            mat = geom.Matrix()
            mat.Translate((to - fromp) * (i + 1))
            for object in selected_items:
                new_object = object.MakeACopy()
                object.GetOwner().Add(new_object)
                cad.TransformUndoably(new_object, mat)
    else:
        mat = geom.Matrix()
        mat.Translate(to - fromp)
        for object in selected_items:
            cad.TransformUndoably(object, mat)
    
    cad.EndHistory()