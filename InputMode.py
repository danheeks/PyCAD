class InputMode:
    def __init__(self):
        pass
        
    def GetTitle(self):
        return ''
    
    def GetHelpText(self):
        return ''
        
    def OnRender(self):
        pass
            
    def OnModeChange(self):
        pass

    def OnFrontRender(self):
        pass
                
    def OnMouse(self, event):
        pass

    def GetProperties(self):
        return []
    
    def OnKeyDown(self, key_code):
        return False 
        
    def OnKeyUp(self, key_code):
        return False 