import cad
import os
import geom

cad_dir = os.path.dirname(os.path.realpath(__file__))

class Gear(cad.BaseObject):
    def __init__(self, mod = 1.0, num_teeth = 12):
        cad.BaseObject.__init__(self)
        self.tm = geom.Matrix()
        self.numTeeth = num_teeth
        self.module = mod
        self.addendumOffset = 0.0
        self.addendumMultiplier = 1.0
        self.dedendumMultiplier = 1.0
        self.pressureAngle = 0.34906585039886 # 20 degrees
        self.tipRelief = 0.05
        self.depth = 0.0
        self.coneHalfAngle = 0.0
        self.angle = 0.0
        self.color = cad.Color(128, 128, 128)
        
    def GetIconFilePath(self):
        return cad_dir + '/icons/gear.png'

    def GetTitle(self):
        return "Gear"

    def GetTypeString(self):
        return "Gear"

    def GetColor(self):
        return self.color
        
    def SetColor(self, col):
        self.color = col

    def OnRenderTriangles(self):
        pass
    
    def GetBox(self):
        return 0,0,0,0,0,0   
     
    def GetProperties(self):
        return []