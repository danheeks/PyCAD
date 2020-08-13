from Object import Object
import geom
import wx
import cad

type = 0

class HImage(Object):
    def __init__(self, filepath = None):
        Object.__init__(self, 0)
        self.filepath = filepath
        self.texture_number = None
        self.width = None
        self.height = None
        self.textureWidth = None
        self.textureHeight = None
        self.bottom_left = None
        self.bottom_right = None
        self.top_right = None
        self.top_left = None
        
    def GetIconFilePath(self):
        return wx.GetApp().cad_dir + '/icons/picture.png'

    def GetTypeString(self):
        return "Picture"
    
    def GetTitle(self):
        return 'Picture - ' + self.filepath

    def GetType(self):
        return type
    
    def GetBox(self):
        box = geom.Box3D()
        if self.bottom_left != None:
            box.InsertPoint(self.bottom_left.x, self.bottom_left.y, self.bottom_left.z)
            box.InsertPoint(self.bottom_right.x, self.bottom_right.y, self.bottom_right.z)
            box.InsertPoint(self.top_right.x, self.top_right.y, self.top_right.z)
            box.InsertPoint(self.top_left.x, self.top_left.y, self.top_left.z)
        return box
     
    def GetProperties(self):
        return []
    
    def OnGlCommands(self, select, marked, no_color):
        if self.texture_number == None:
            image = wx.Image(self.filepath)
            if not image.IsOk():
                self.texture_number = 0
                return
 
            self.width = image.GetWidth()
            self.height = image.GetHeight()
            if self.bottom_left == None:
                self.bottom_left = geom.Point3D(0,0,0)
                self.bottom_right = geom.Point3D(self.width,0,0)
                self.top_right = geom.Point3D(self.width,self.height,0)
                self.top_left = geom.Point3D(0,self.height,0)
            self.texture_number, self.textureWidth = cad.GenTexture(image.GetData(), None, self.width, self.height, image.HasAlpha())
        
        if self.texture_number == 0:
            return

        if self.bottom_left != None:
            cad.DrawImageQuads(self.width, self.height, self.textureWidth, self.texture_number, self.bottom_left, self.bottom_right, self.top_right, self.top_left, no_color, marked)
            
    def GetGrippers(self, just_for_endof):
        if self.bottom_left != None:
            cad.AddGripper(cad.GripData(self.bottom_left, cad.GripperType.Stretch, 0))
            cad.AddGripper(cad.GripData(self.bottom_right, cad.GripperType.Stretch, 0))
            cad.AddGripper(cad.GripData(self.top_right, cad.GripperType.Stretch, 0))
            cad.AddGripper(cad.GripData(self.top_left, cad.GripperType.Stretch, 0))
            
    def Stretch(self):
        p = cad.GetStretchPoint()
        shift = cad.GetStretchShift()
        if p == self.bottom_left:
            self.bottom_left = self.bottom_left + shift
        if p == self.bottom_right:
            self.bottom_right = self.bottom_right + shift
        if p == self.top_right:
            self.top_right = self.top_right + shift
        if p == self.top_left:
            self.top_left = self.top_left + shift
            
    def Transform(self, mat):
        self.bottom_left.Transform(mat)
        self.bottom_right.Transform(mat)
        self.top_right.Transform(mat)
        self.top_left.Transform(mat)
            
    def WriteXml(self):
        cad.SetXmlValue('filepath', self.filepath)
        cad.SetXmlValue('x00', self.bottom_left.x)
        cad.SetXmlValue('x01', self.bottom_left.y)
        cad.SetXmlValue('x02', self.bottom_left.z)
        cad.SetXmlValue('x10', self.bottom_right.x)
        cad.SetXmlValue('x11', self.bottom_right.y)
        cad.SetXmlValue('x12', self.bottom_right.z)
        cad.SetXmlValue('x20', self.top_right.x)
        cad.SetXmlValue('x21', self.top_right.y)
        cad.SetXmlValue('x22', self.top_right.z)
        cad.SetXmlValue('x30', self.top_left.x)
        cad.SetXmlValue('x31', self.top_left.y)
        cad.SetXmlValue('x32', self.top_left.z)
        Object.WriteXml(self)

    def ReadXml(self):
        self.filepath = cad.GetXmlValue('filepath')
        if self.bottom_left == None:
            self.bottom_left = geom.Point3D(0,0,0)            
            self.bottom_right = geom.Point3D(0,0,0)            
            self.top_right = geom.Point3D(0,0,0)            
            self.top_left = geom.Point3D(0,0,0)            
        self.bottom_left.x = float(cad.GetXmlValue('x00'))
        self.bottom_left.y = float(cad.GetXmlValue('x01'))
        self.bottom_left.z = float(cad.GetXmlValue('x02'))
        self.bottom_right.x = float(cad.GetXmlValue('x10'))
        self.bottom_right.y = float(cad.GetXmlValue('x11'))
        self.bottom_right.z = float(cad.GetXmlValue('x12'))
        self.top_right.x = float(cad.GetXmlValue('x20'))
        self.top_right.y = float(cad.GetXmlValue('x21'))
        self.top_right.z = float(cad.GetXmlValue('x22'))
        self.top_left.x = float(cad.GetXmlValue('x30'))
        self.top_left.y = float(cad.GetXmlValue('x31'))
        self.top_left.z = float(cad.GetXmlValue('x32'))
        Object.ReadXml(self)
                
    def MakeACopy(self):
        copy = HImage()
        copy.CopyFrom(self)
        return copy
    
    def CopyFrom(self, o):
        self.filepath = o.filepath
        self.bottom_left = geom.Point3D(o.bottom_left)
        self.bottom_right = geom.Point3D(o.bottom_right)
        self.top_right = geom.Point3D(o.top_right)
        self.top_left = geom.Point3D(o.top_left)
        Object.CopyFrom(self, o)
            
        
        
