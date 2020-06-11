import math
import cad
import geom
import wx
from HeeksConfig import HeeksConfig
from HDialog import HDialog
from HDialog import control_border
from HDialog import HControl
from NiceTextCtrl import DoubleCtrl
from NiceTextCtrl import LengthCtrl
from PictureFrame import PictureWindow

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
        
        self.ncopies = ncopies
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
        if self.ncopies:
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
            pos = wx.GetApp().PickPosition("Pick position to move from")
            if pos != None: fromp = pos
        elif ret == dlg.btnTo.GetId():
            pos = wx.GetApp().PickPosition("Pick position to move to")
            if pos != None: to = pos
        else:
            return False, None, None, None

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
    
class AngleAndPlaneDlg(HDialog):
    def __init__(self, angle, axis, axis_type, pos, ncopies, axial_shift, title):
        HDialog.__init__(self, title)
        self.ignore_event_functions = True
        sizerMain = wx.BoxSizer(wx.VERTICAL)
        
        self.ncopies = ncopies
        self.axis = axis
        self.axis_type = axis_type
        self.pos = pos
        self.btnPick = None
        self.btnVectorPick = None
        
        if ncopies:
            self.txt_num_copies = wx.TextCtrl(self, wx.ID_ANY, str(ncopies))
            self.AddLabelAndControl(sizerMain, 'Number of Copies', self.txt_num_copies)
            
        sizerPosAndAxis = wx.BoxSizer(wx.HORIZONTAL)
        sizerMain.Add(sizerPosAndAxis, 0, wx.EXPAND)
            
        sizerPos = wx.BoxSizer(wx.VERTICAL)
        sizerPosAndAxis.Add(sizerPos, 0, wx.EXPAND)
        if axis_type == 0:
            sizerPos.Add( wx.StaticText(self, label = 'Position to Rotate About'), 0, wx.EXPAND | wx.ALL, control_border)
        else:
            self.btnPick = wx.Button(self, label = 'Select')
            self.AddLabelAndControl(sizerPos, 'Position to Rotate About', self.btnPick)
        self.posx = LengthCtrl(self)
        self.AddLabelAndControl(sizerPos, 'X', self.posx )
        self.posy = LengthCtrl(self)
        self.AddLabelAndControl(sizerPos, 'Y', self.posy )
        self.posz = LengthCtrl(self)
        self.AddLabelAndControl(sizerPos, 'Z', self.posz )
        if axis_type == 0:
            self.btnPick = wx.Button(self, label = 'Select')
            sizerPos.Add( self.btnPick, wx.EXPAND | wx.ALL, control_border )
        self.posx.SetValue(self.pos.x)
        self.posy.SetValue(self.pos.y)
        self.posz.SetValue(self.pos.z)                        
                         
        # add picture
        self.picture = PictureWindow(self, wx.Size(150, 100))
        pictureSizer = wx.BoxSizer(wx.HORIZONTAL)
        pictureSizer.Add( self.picture, 1, wx.EXPAND )
        sizerMain.Add( pictureSizer, 0, wx.ALL, control_border )
        sizerPlane = wx.BoxSizer(wx.VERTICAL)
        pictureSizer.Add(sizerPlane, 0, wx.EXPAND )
        
        if axis_type == 0:
            # non standard axis
            sizerAxis = wx.BoxSizer(wx.VERTICAL)
            sizerPosAndAxis.Add(sizerAxis, 0, wx.EXPAND)
            
            sizerAxis.Add( wx.StaticText(self, label = 'Axis to Rotate About'), 0, wx.EXPAND | wx.ALL, control_border )
            self.vectorx = DoubleCtrl(self)
            self.AddLabelAndControl( sizerAxis, 'X', self.vectorx )
            self.vectory = DoubleCtrl(self)
            self.AddLabelAndControl( sizerAxis, 'Y', self.vectory )
            self.vectorz = DoubleCtrl(self)
            self.AddLabelAndControl( sizerAxis, 'Z', self.vectorz )
            self.btnVectorPick = wx.Button(self, label = 'Select a line')
            sizerAxis.Add( self.btnVectorPick, 0, wx.EXPAND | wx.ALL, control_border )
            self.vectorx.SetValue( axis.x )
            self.vectory.SetValue( axis.y )
            self.vectorz.SetValue( axis.z )
        else:
            self.vectorx = None        
        
        self.rbXy = wx.RadioButton( self, label = 'XY', style = wx.RB_GROUP)
        self.rbXz = wx.RadioButton( self, label = 'XZ')
        self.rbYz = wx.RadioButton( self, label = 'YZ')
        self.rbOther = wx.RadioButton( self, label = 'Other')
        sizerPlane.Add( self.rbXy, 0, wx.ALL, control_border )
        sizerPlane.Add( self.rbXz, 0, wx.ALL, control_border )
        sizerPlane.Add( self.rbYz, 0, wx.ALL, control_border )
        sizerPlane.Add( self.rbOther, 0, wx.ALL, control_border )
        
        self.rbXy.SetValue( axis_type == 1 )
        self.rbXz.SetValue( axis_type == 2 )
        self.rbYz.SetValue( axis_type == 3 )
        self.rbOther.SetValue( axis_type == 0 )
        
        self.angle_ctrl = DoubleCtrl(self)
        self.AddLabelAndControl( sizerMain, 'Angle', self.angle_ctrl )
        self.angle_ctrl.SetValue( angle )
        
        self.axial_shift_ctrl = LengthCtrl(self)
        self.AddLabelAndControl( sizerMain, 'Axial Shift', self.axial_shift_ctrl)
        self.axial_shift_ctrl.SetValue(axial_shift)        
        
        self.MakeOkAndCancel(wx.HORIZONTAL).AddToSizer(sizerMain)
        
        self.SetSizer(sizerMain)
        sizerMain.SetSizeHints(self)
        sizerMain.Fit(self)
        
        if ncopies:
            self.txt_num_copies.SetFocus()
        else:
            self.angle_ctrl.SetFocus()
            
        self.SetPicture()
            
        self.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton, self.rbXy)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton, self.rbXz)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton, self.rbYz)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton, self.rbOther)
        if self.btnPick: self.Bind(wx.EVT_BUTTON, self.OnPick, self.btnPick)
        if self.btnVectorPick: self.Bind(wx.EVT_BUTTON, self.OnPickVector, self.btnVectorPick)
            
        self.ignore_event_functions = False
        
    def OnPick(self, event):
        self.EndModal(self.btnPick.GetId())
        
    def OnPickVector(self, event):
        self.EndModal(self.btnVectorPick.GetId())
        
    def OnRadioButton(self, event):
        if self.ignore_event_functions:
            return
        
        if event.GetId() == self.rbXy.GetId() or event.GetId() == self.rbXz.GetId() or event.GetId() == self.rbYz.GetId():
            if self.axis_type == 0:
                self.EndModal(event.GetId())
                
        if event.GetId() == self.rbOther.GetId() and self.axis_type != 0:
            self.EndModal(event.GetId())
            
        self.SetPicture()
        
    def SetPictureByName(self, name):
        self.picture.SetPicture(wx.GetApp().cad_dir + '/bitmaps/angle/' + name + '.png')

    def SetPicture(self):
        if self.rbXy.GetValue():self.SetPictureByName('xy')
        elif self.rbXz.GetValue():self.SetPictureByName('xz')
        elif self.rbYz.GetValue():self.SetPictureByName('yz')
        elif self.rbOther.GetValue():self.SetPictureByName('line')
        
    def GetAllValues(self):
        angle = self.angle_ctrl.GetValue()
        if self.rbXy.GetValue():
            axis = geom.Point3D(0,0,1)
            axis_type = 1
        elif self.rbXz.GetValue():
            axis = geom.Point3D(0,-1,0)
            axis_type = 2
        elif self.rbYz.GetValue():
            axis = geom.Point3D(1,0,0)
            axis_type = 3
        else:
            if self.vectorx == None:
                axis = self.axis
            else:
                axis = geom.Point3D(self.vectorx.GetValue(), self.vectory.GetValue(), self.vectorz.GetValue())
            axis_type = 0
        pos = geom.Point3D(self.posx.GetValue(), self.posy.GetValue(), self.posz.GetValue())
            
        ncopies = None
        if self.ncopies:
            ncopies = int(self.txt_num_copies.GetValue())
        
        axial_shift = self.axial_shift_ctrl.GetValue()
        
        return angle, axis, axis_type, pos, ncopies, axial_shift

def InputAngleWithPlane(angle, axis, pos, ncopies, axial_shift, title):
    axis_type = 0 # not one of the three
    if axis == geom.Point3D(0,0,1):axis_type = 1
    elif axis == geom.Point3D(0,-1,0):axis_type = 2
    elif axis == geom.Point3D(1,0,0):axis_type = 3

    while(True):
        dlg = AngleAndPlaneDlg(angle, axis, axis_type, pos, ncopies, axial_shift, title)
        ret = dlg.ShowModal()
        angle, axis, axis_type, pos, ncopies, axial_shift = dlg.GetAllValues()
        
        if ret == wx.ID_OK:
            return True, angle, axis, pos, ncopies, axial_shift
        elif ret == dlg.rbXy.GetId():
            continue
        elif ret == dlg.rbXz.GetId():
            continue
        elif ret == dlg.rbYz.GetId():
            continue
        elif ret == dlg.rbOther.GetId():
            continue
        elif ret == dlg.btnPick.GetId():
            p = wx.GetApp().PickPosition("Pick Position To Rotate About")
            if p != None: pos = p
        elif ret == dlg.btnVectorPick.GetId():
            p = wx.GetApp().PickPosition("Pick Position On Rotation Axis");
            if p != None:
                v = p - pos
                if v.Length() < 0.01:
                    wx.MessageBox('Axis Point Too Close to Position')
            else:
                axis = v.Normalized()
        else:
            return False, None, None, None, None, None
    

def Rotate(copy = False):
    #rotation axis - Z axis by default
    axis_Dir = geom.Point3D(0,0,1)
    line_Pos = geom.Point3D(0,0,0)
    
    if cad.GetNumSelected() == 0:
        wx.GetApp().PickObjects('Pick objects to rotate')
    
    if cad.GetNumSelected() == 0:
        return
    
    config = HeeksConfig()
    ncopies = None
    if copy:
        ncopies = config.ReadInt('RotateNumCopies', 1);
        RemoveUncopyable()
        if cad.GetNumSelected() == 0:
            return
        
    selected_items = cad.GetSelectedObjects()
    
    cad.ClearSelection(False)

    angle = config.ReadFloat('RotateAngle', 90.0)

    # enter angle, plane and position

    axis = geom.Point3D(0,0,1)
    pos = geom.Point3D(0,0,0)
    axis.x = config.ReadFloat("RotateAxisX", 0.0)
    axis.y = config.ReadFloat("RotateAxisY", 0.0)
    axis.z = config.ReadFloat("RotateAxisZ", 1.0)
    pos.x = config.ReadFloat("RotatePosX", 0.0)
    pos.y = config.ReadFloat("RotatePosY", 0.0)
    pos.z = config.ReadFloat("RotatePosZ", 0.0)
    
    axial_shift = config.ReadFloat('RotateAxialShift', 0.0)
    
    result, angle, axis, pos, ncopies, axial_shift = InputAngleWithPlane(angle, axis, pos, ncopies, axial_shift, 'Rotate')
    if not result:
        return
    
    if copy:
        if ncopies < 1:
            return
        config.WriteInt("RotateNumCopies", ncopies)
        
    config.WriteFloat("RotateAngle", angle)
    config.WriteFloat("RotateAxialShift", axial_shift)
    config.WriteFloat("RotateAxisX", axis.x)
    config.WriteFloat("RotateAxisY", axis.y)
    config.WriteFloat("RotateAxisZ", axis.z)
    config.WriteFloat("RotatePosX", pos.x)
    config.WriteFloat("RotatePosY", pos.y)
    config.WriteFloat("RotatePosZ", pos.z)
    
    cad.StartHistory()
    
    if copy:
        for i in range(0, ncopies):
            mat = geom.Matrix()
            mat.Translate(-pos)
            mat.RotateAxis(angle * 0.0174532925199433 *  (i + 1), axis)
            mat.Translate(pos)
            for object in selected_items:
                new_object = object.MakeACopy()
                object.GetOwner().Add(new_object)
                cad.TransformUndoably(new_object, mat)
    else:
        mat = geom.Matrix()
        mat.Translate(-pos)
        mat.RotateAxis(angle * 0.0174532925199433, axis)
        mat.Translate(pos)
        for object in selected_items:
            cad.TransformUndoably(object, mat)
    
    cad.EndHistory()    
            
class MirrorDlg(HDialog):
    def __init__(self, axis, pos, axis_type, copy, title):
        HDialog.__init__(self, title)
        self.ignore_event_functions = True
        sizerMain = wx.BoxSizer(wx.VERTICAL)
        
        self.copy = copy
        self.axis = axis
        self.axis_type = axis_type
        self.pos = pos
        self.btnPick = None
        self.btnVectorPick = None
        
        self.chkCopy = wx.CheckBox(self, wx.ID_ANY, 'Keep Original')
        HControl(wx.ALL, self.chkCopy).AddToSizer(sizerMain)
        self.chkCopy.SetValue(copy)
            
        sizerPosAndAxis = wx.BoxSizer(wx.HORIZONTAL)
        sizerMain.Add(sizerPosAndAxis, 0, wx.EXPAND)
            
        sizerPos = wx.BoxSizer(wx.VERTICAL)
        sizerPosAndAxis.Add(sizerPos, 0, wx.EXPAND)
        if axis_type == 0:
            sizerPos.Add( wx.StaticText(self, label = 'Position to Mirror About'), 0, wx.EXPAND | wx.ALL, control_border)
        else:
            self.btnPick = wx.Button(self, label = 'Select')
            self.AddLabelAndControl(sizerPos, 'Position to Mirror About', self.btnPick)
        self.posx = LengthCtrl(self)
        self.AddLabelAndControl(sizerPos, 'X', self.posx )
        self.posy = LengthCtrl(self)
        self.AddLabelAndControl(sizerPos, 'Y', self.posy )
        self.posz = LengthCtrl(self)
        self.AddLabelAndControl(sizerPos, 'Z', self.posz )
        if axis_type == 0:
            self.btnPick = wx.Button(self, label = 'Select')
            sizerPos.Add( self.btnPick, wx.EXPAND | wx.ALL, control_border )
        self.posx.SetValue(self.pos.x)
        self.posy.SetValue(self.pos.y)
        self.posz.SetValue(self.pos.z) 
        if axis_type != 0:
            self.posx.Enable( axis_type == 1 )
            self.posy.Enable( axis_type == 2 )
            self.posz.Enable( axis_type == 3 )        
        
        if axis_type == 0:
            # non standard axis
            sizerAxis = wx.BoxSizer(wx.VERTICAL)
            sizerPosAndAxis.Add(sizerAxis, 0, wx.EXPAND)
            
            sizerAxis.Add( wx.StaticText(self, label = 'Axis to Mirror Through'), 0, wx.EXPAND | wx.ALL, control_border )
            self.vectorx = DoubleCtrl(self)
            self.AddLabelAndControl( sizerAxis, 'X', self.vectorx )
            self.vectory = DoubleCtrl(self)
            self.AddLabelAndControl( sizerAxis, 'Y', self.vectory )
            self.vectorz = DoubleCtrl(self)
            self.AddLabelAndControl( sizerAxis, 'Z', self.vectorz )
            self.btnVectorPick = wx.Button(self, label = 'Select Axis Point')
            sizerAxis.Add( self.btnVectorPick, 0, wx.EXPAND | wx.ALL, control_border )
            self.vectorx.SetValue( axis.x )
            self.vectory.SetValue( axis.y )
            self.vectorz.SetValue( axis.z )
        else:
            self.vectorx = None        
                         
        # add picture
        self.picture = PictureWindow(self, wx.Size(150, 100))
        pictureSizer = wx.BoxSizer(wx.HORIZONTAL)
        pictureSizer.Add( self.picture, 1, wx.EXPAND )
        sizerMain.Add( pictureSizer, 0, wx.ALL, control_border )
        sizerPlane = wx.BoxSizer(wx.VERTICAL)
        pictureSizer.Add(sizerPlane, 0, wx.EXPAND )

        
        self.rbH = wx.RadioButton( self, label = 'Horizontal', style = wx.RB_GROUP)
        self.rbV = wx.RadioButton( self, label = 'Vertical')
        self.rbZ = wx.RadioButton( self, label = 'Z')
        self.rbOther = wx.RadioButton( self, label = 'Other')
        sizerPlane.Add( self.rbH, 0, wx.ALL, control_border )
        sizerPlane.Add( self.rbV, 0, wx.ALL, control_border )
        sizerPlane.Add( self.rbZ, 0, wx.ALL, control_border )
        sizerPlane.Add( self.rbOther, 0, wx.ALL, control_border )
        
        self.rbH.SetValue( axis_type == 1 )
        self.rbV.SetValue( axis_type == 2 )
        self.rbZ.SetValue( axis_type == 3 )
        self.rbOther.SetValue( axis_type == 0 )
        
        self.MakeOkAndCancel(wx.HORIZONTAL).AddToSizer(sizerMain)

        self.SetPicture()
            
        self.SetSizer(sizerMain)
        sizerMain.SetSizeHints(self)
        sizerMain.Fit(self)
        
        self.chkCopy.SetFocus()
            
        self.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton, self.rbH)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton, self.rbV)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton, self.rbZ)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton, self.rbOther)
        if self.btnPick: self.Bind(wx.EVT_BUTTON, self.OnPick, self.btnPick)
        if self.btnVectorPick: self.Bind(wx.EVT_BUTTON, self.OnPickVector, self.btnVectorPick)
            
        self.ignore_event_functions = False
        
    def OnPick(self, event):
        self.EndModal(self.btnPick.GetId())
        
    def OnPickVector(self, event):
        self.EndModal(self.btnVectorPick.GetId())
        
    def OnRadioButton(self, event):
        if self.ignore_event_functions:
            return
        
        if event.GetId() == self.rbH.GetId() or event.GetId() == self.rbV.GetId() or event.GetId() == self.rbZ.GetId():
            if self.axis_type == 0:
                self.EndModal(event.GetId())
                
        if event.GetId() == self.rbOther.GetId() and self.axis_type != 0:
            self.EndModal(event.GetId())
            
        self.SetPicture()
        
    def SetPictureByName(self, name):
        self.picture.SetPicture(wx.GetApp().cad_dir + '/bitmaps/mirror/' + name + '.png')

    def SetPicture(self):
        if self.rbH.GetValue():self.SetPictureByName('horiz')
        elif self.rbV.GetValue():self.SetPictureByName('vert')
        elif self.rbZ.GetValue():self.SetPictureByName('z')
        elif self.rbOther.GetValue():self.SetPictureByName('other')
        
    def GetAllValues(self):
        if self.rbH.GetValue():
            axis = geom.Point3D(1,0,0)
            axis_type = 1
        elif self.rbV.GetValue():
            axis = geom.Point3D(0,1,0)
            axis_type = 2
        elif self.rbZ.GetValue():
            axis = geom.Point3D(0,0,1)
            axis_type = 3
        else:
            if self.vectorx == None:
                axis = self.axis
            else:
                axis = geom.Point3D(self.vectorx.GetValue(), self.vectory.GetValue(), self.vectorz.GetValue())
            axis_type = 0
        pos = geom.Point3D(self.posx.GetValue(), self.posy.GetValue(), self.posz.GetValue())
            
        copy = self.chkCopy.GetValue()
        
        return axis, pos, axis_type, copy

def InputMirrorPlane(axis, pos, copy, title):
    axis_type = 0 # not one of the three
    if axis == geom.Point3D(1,0,0):axis_type = 1
    elif axis == geom.Point3D(0,1,0):axis_type = 2
    elif axis == geom.Point3D(0,0,1):axis_type = 3

    while(True):
        dlg = MirrorDlg(axis, pos, axis_type, copy, title)
        ret = dlg.ShowModal()
        axis, pos, axis_type, copy = dlg.GetAllValues()
        
        if ret == wx.ID_OK:
            return True, axis, pos, copy
        elif ret == dlg.rbH.GetId():
            continue
        elif ret == dlg.rbV.GetId():
            continue
        elif ret == dlg.rbZ.GetId():
            continue
        elif ret == dlg.rbOther.GetId():
            continue
        elif ret == dlg.btnPick.GetId():
            p = wx.GetApp().PickPosition("Pick Position To Mirror About")
            if p != None: pos = p
        elif (dlg.btnVectorPick != None) and (ret == dlg.btnVectorPick.GetId()):
            p = wx.GetApp().PickPosition("Pick Position On Mirror Axis")
            if p != None:
                v = p - pos
                if v.Length() < 0.01:
                    wx.MessageBox('Axis Point Too Close to Position')
                else:
                    axis = v.Normalized()
        else:
            return False, None, None, None

def Mirror():
    if cad.GetNumSelected() == 0:
        wx.GetApp().PickObjects('Pick objects to mirror')
    
    if cad.GetNumSelected() == 0:
        return
    
    config = HeeksConfig()
        
    selected_items = cad.GetSelectedObjects()
    
    cad.ClearSelection(False)
    
    copy = config.ReadBool("MirrorCopy", False)
    axis = geom.Point3D(0,0,1)
    pos = geom.Point3D(0,0,0)
    axis.x = config.ReadFloat("MirrorAxisX", 1.0)
    axis.y = config.ReadFloat("MirrorAxisY", 0.0)
    axis.z = config.ReadFloat("MirrorAxisZ", 0.0)
    pos.x = config.ReadFloat("MirrorPosX", 0.0)
    pos.y = config.ReadFloat("MirrorPosY", 0.0)
    pos.z = config.ReadFloat("MirrorPosZ", 0.0)
    
    result, axis, pos, copy = InputMirrorPlane(axis, pos, copy, 'Mirror')
    if not result:
        return
    
    config.WriteBool("MirrorCopy", copy)
    config.WriteFloat("MirrorAxisX", axis.x)
    config.WriteFloat("MirrorAxisY", axis.y)
    config.WriteFloat("MirrorAxisZ", axis.z)
    config.WriteFloat("MirrorPosX", pos.x)
    config.WriteFloat("MirrorPosY", pos.y)
    config.WriteFloat("MirrorPosZ", pos.z)
    
    cad.StartHistory()

    mat = geom.Matrix()
    x, y = axis.ArbitraryAxes()
    axis_mat = geom.Matrix(pos, x, y)
    inv_mat = axis_mat.Inverse()
    mat.Multiply(inv_mat) # transform so axis goes to z axis
    mat.Scale3(1.0, 1.0, -1.0) # mirror in z axis
    mat.Multiply(axis_mat) # transform back to where it was
    for object in selected_items:
        if copy:
            if object.CanBeCopied():
                object = object.MakeACopy()
                object.Transform(mat)
                cad.AddUndoably(object)
        else:
            cad.TransformUndoably(object, mat)
    
    cad.EndHistory()   
    
def PickOrigin(prompt):
    wx.GetApp().PickObjects(prompt)
    origin = None
    for object in cad.GetSelectedObjects():
        if object.GetType() == cad.OBJECT_TYPE_COORD_SYS:
            origin = object
            break
    return origin

def OriTransform():
    if cad.GetNumSelected() == 0:
        wx.GetApp().PickObjects('Pick objects to transform')
    
    if cad.GetNumSelected() == 0:
        return
    
    config = HeeksConfig()
        
    selected_items = cad.GetSelectedObjects()
    
    cad.ClearSelection(False)
    
    o1 = PickOrigin('Pick From Origin')
    if o1 == None:
        wx.MessageBox("You didn't pick an origin\nTransform Cancelled!")
        return
    
    o2 = PickOrigin('Pick To Origin')
    if o2 == None:
        wx.MessageBox("You didn't pick an origin\nTransform Cancelled!")
        return
    
    cad.StartHistory()

    m1 = o1.GetOrigin()
    mat = m1.Inverse()
    m2 = o2.GetOrigin()
    mat.Multiply(m2)
    for object in selected_items:
        cad.TransformUndoably(object, mat)
    
    cad.EndHistory()   
   