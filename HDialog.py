import wx
from NiceTextCtrl import LengthCtrl

control_border = 3

class HControl:
    def __init__(self, flag, window = None, sizer = None):
        self.flag = flag
        self.window = window
        self.sizer = sizer
        
    def AddToSizer(self, s):
        if self.window:
            return s.Add(self.window, 0, self.flag, control_border)
        return s.Add(self.sizer, 0, self.flag, control_border)
        
class HTypeObjectDropDown(wx.ComboBox):
    def __init__(self, parent, object_type, obj_list, OnComboOrCheck = None):
        choices = self.GetObjectChoices(object_type, obj_list)
        wx.ComboBox.__init__(self, parent, choices = choices)
        self.object_type = object_type
        self.obj_list = obj_list
        if OnComboOrCheck:
            parent.Bind(wx.EVT_COMBOBOX, OnComboOrCheck, self)
        
    def Recreate(self):
        self.Clear()
        self.Append(self.GetObjectChoices(self.object_type, self.obj_list))
        
    def GetObjectChoices(self, object_type, obj_list):
        choices = ['None']
        object = obj_list.GetFirstChild()
        while object:
            if object.GetIDGroupType() == object_type:
                number = object.GetID()
                choices.append(object.GetTitle())
            object = obj_list.GetNextChild()
        return choices
            
    def GetSelectedId(self):
        sel = self.GetSelection()
        if sel < 0:
            return 0
        return self.obj_list[sel].GetId()
        
    def SelectById(self, id):
        # set the combo to the correct item
        i = 0
        object = self.obj_list.GetFirstChild()
        while object:
            if object.GetId() == id:
                self.SetSelection(i)
                return
            i += 1
            object = self.obj_list.GetNextChild()
        self.SetSelection(0)
     
class XYZBoxes(wx.StaticBoxSizer):
    def __init__(self, dlg, label, xstr, ystr, zstr):
        self.lgthX = LengthCtrl(dlg)
        self.sttcX = dlg.AddLabelAndControl(self, xstr, self.lgthX)
        self.lgthY = LengthCtrl(dlg)
        self.sttcY = dlg.AddLabelAndControl(self, ystr, self.lgthY)
        self.lgthZ = LengthCtrl(dlg)
        self.sttcZ = dlg.AddLabelAndControl(self, zstr, self.lgthZ)
    
class HDialog(wx.Dialog):
    def __init__(self, title):
        wx.Dialog.__init__(self, wx.GetApp().frame, wx.ID_ANY, title)
        self.ignore_event_functions = False
        self.button_id_txt_map = {}
        
    def AddLabelAndControl(self, sizer, label, control):
        sizer_horizontal = wx.BoxSizer(wx.HORIZONTAL)
        static_label = wx.StaticText(self, wx.ID_ANY, label)
        sizer_horizontal.Add( static_label, 0, wx.RIGHT + wx.ALIGN_LEFT + wx.ALIGN_CENTER_VERTICAL, control_border )
        sizer_horizontal.Add( control, 1, wx.LEFT + wx.ALIGN_RIGHT + wx.ALIGN_CENTER_VERTICAL, control_border )
        sizer.Add( sizer_horizontal, 0, wx.EXPAND + wx.ALL, control_border )
        return static_label
        
    def AddFileNameControl(self, sizer, label, text_control):
        sizer_horizontal = wx.BoxSizer(wx.HORIZONTAL)
        static_label = wx.StaticText(self, wx.ID_ANY, label)
        sizer_horizontal.Add( static_label, 0, wx.RIGHT + wx.ALIGN_LEFT + wx.ALIGN_CENTER_VERTICAL, control_border )
        sizer_horizontal.Add( text_control, 1, wx.LEFT + wx.RIGHT + wx.ALIGN_CENTER_VERTICAL, control_border )
        button_control = wx.Button(self, label = "...")
        sizer_horizontal.Add( button_control, 0, wx.LEFT + wx.ALIGN_RIGHT + wx.ALIGN_CENTER_VERTICAL, control_border )
        sizer.Add( sizer_horizontal, 0, wx.EXPAND + wx.ALL, control_border )
        self.button_id_txt_map[button_control.GetId()] = text_control
        self.Bind(wx.EVT_BUTTON, self.OnFileBrowseButton, button_control)
        return static_label, button_control
    
    def MakeControlUsingStaticText(self, static_text, control1, control2 = None):
        sizer_horizontal = wx.BoxSizer(wx.HORIZONTAL)
        sizer_horizontal.Add(static_text, 0, wx.RIGHT | wx.ALIGN_LEFT | wx.ALIGN_CENTER_VERTICAL, control_border)
        if control2:
            sizer_horizontal.Add(control1, 1, wx.LEFT | wx.RIGHT | wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL, control_border)
            sizer_horizontal.Add(control2, 0, wx.LEFT | wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL, control_border)
        else:
            sizer_horizontal.Add(control1, 1, wx.LEFT | wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL, control_border)

        return HControl(wx.EXPAND | wx.ALL, sizer = sizer_horizontal)
        
    def MakeLabelAndControl(self, label, control1, control2 = None):
        static_label = wx.StaticText(self, label = label)
        return self.MakeControlUsingStaticText(static_label, control1, control2)
        
    def MakeOkAndCancel(self, orient):
        sizerOKCancel = wx.BoxSizer(orient)
        buttonOK = wx.Button(self, wx.ID_OK, "OK")
        if orient == wx.RIGHT:
            ok_flag = wx.LEFT
            cancel_flag = wx.UP
        else:
            ok_flag = wx.DOWN
            cancel_flag = wx.UP
        ok_flag = 0
        cancel_flag = 0
        sizerOKCancel.Add( buttonOK, 0, wx.ALL + ok_flag, control_border )
        buttonCancel = wx.Button(self, wx.ID_CANCEL, "Cancel")
        sizerOKCancel.Add( buttonCancel, 0, wx.ALL + cancel_flag, control_border )
        buttonOK.SetDefault()
        return HControl(wx.ALL | wx.ALIGN_RIGHT | wx.ALIGN_BOTTOM, sizerOKCancel)
    
    def OnFileBrowseButton(self, event):
        dialog = wx.FileDialog(wx.GetApp().frame, "Choose File", wildcard = "All files" + " |*.*")
        dialog.CentreOnParent()
        
        if dialog.ShowModal() == wx.ID_OK:
            text_control = self.button_id_txt_map[event.GetId()]
            text_control.SetValue(dialog.GetPath())
            
class ComboBoxBinded(wx.ComboBox):
    def __init__(self, parent, choices):
        wx.ComboBox.__init__(self, parent, choices = choices)
        parent.Bind(wx.EVT_COMBOBOX, parent.OnComboOrCheck, self)
