import cad

class RefreshObserver(cad.Observer):
    def __init__(self, window):
        cad.Observer.__init__(self)
        self.window = window
        
    def OnRemoved(self, removed):
        self.window.Refresh()
        
    def OnAdded(self, added):
        self.window.Refresh()
        
    def OnModified(self, modified):
        self.window.Refresh()
            
    def OnSelectionChanged(self, added, removed):
        self.window.Refresh()