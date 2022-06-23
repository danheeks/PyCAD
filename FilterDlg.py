import wx
import cad
from HDialog import HDialog
from HDialog import control_border

class FilterDlg(HDialog):
    def __init__(self):
        HDialog.__init__(self, title = 'Filter')

        self.names_and_types = cad.GetObjectNamesAndTypes()
        self.check_boxes = []

        sizerMain = wx.BoxSizer(wx.VERTICAL)
        for name, type in cad.GetObjectNamesAndTypes():
            check_box = wx.CheckBox(self, wx.ID_ANY, name)
            self.check_boxes.append((check_box, type))
            if wx.GetApp().select_mode.filter.IsTypeInFilter(type):
                check_box.SetValue(True)
            sizerMain.Add(check_box, 0, wx.ALL, 2)
            
        ok_cancel = self.MakeOkAndCancel(wx.HORIZONTAL)
        buttonSetAll = wx.Button(self, wx.ID_ANY, "Set All")
        ok_cancel.sizer.Add( buttonSetAll, 0, wx.ALL, control_border )
        self.Bind(wx.EVT_BUTTON, self.OnSetAll, buttonSetAll)
        ok_cancel.AddToSizer(sizerMain)
        
        self.SetSizer( sizerMain )
        sizerMain.SetSizeHints( self )
        sizerMain.Fit( self )
        
    def OnSetAll(self, event):
        for check_box, type in self.check_boxes:
            check_box.SetValue(True)
        
    def SetFilterFromCheckBoxes(self):
        checked_types = []
        for check_box, type in self.check_boxes:
            if check_box.IsChecked():
                checked_types.append(type)
        wx.GetApp().select_mode.filter.Clear()
        if len(checked_types) < len(self.names_and_types):
            for checked_type in checked_types:
                wx.GetApp().select_mode.filter.AddType(checked_type)
