import cad
import step
import geom
import wx

DEFAULT_COLOR = 0x808080

# scaling; I am writing these .wrl files for use in KiCAD, which assumes one unit is 0.1 inches
MM_TO_WRL_SCALE = 0.3937007874015748

def GetFileName(path):
    p2 = path.replace('\\', '/')
    slash = p2.rfind('/')
    if slash == -1:
        return path
    return p2[slash+1:]
    
def color_red(color):
    return float((color & 0xff) / 255)
    
def color_green(color):
    return float(((color >> 8) & 0xff) / 255)
    
def color_blue(color):
    return float(((color >>16) & 0xff) / 255)
    
class WrlFace:
    def __init__(self, color, face):
        self.color = color
        self.index = []
        self.points = []
        
        t = face.GetTris(0.01)
        mesh = t.GetMesh()
        self.vlist, self.flist = mesh.GetFaces()
        
class Shape:
    def __init__(self):
        self.tris = geom.Stl()
        self.points = []
        self.coordIndex = []
        
    def AddStls(self):
        temp_file = wx.StandardPaths.Get().GetTempDir() + '/wrl.stl'
        if len(self.coordIndex) > 0:
            stl = geom.Stl()
            for coords in self.coordIndex:
                if len(coords) == 3:
                    stl.Add(self.points[coords[0]], self.points[coords[1]], self.points[coords[2]])            
            stl.WriteStl(temp_file)
            cad.Import(temp_file)
        
NUMBER_MODE_NOT_SET = 0
NUMBER_MODE_POINT = 1
NUMBER_MODE_COORD_INDEX = 2

class WrlImporter:
    
    def __init__(self):
        self.curlys = 0 # counting + for { and - for }
        self.squares = 0 # counting + for [ and - for ]
        self.number = ''
        self.e_found = False
        self.line_number = 0
        self.word = ''
        self.path = ''
        self.number_mode = NUMBER_MODE_NOT_SET
        self.shape = None
        self.point = []
        self.coords = []
        
    def add_number(self):
        if len(self.number) == 0:
            return
        
        if self.e_found:
            f_number = 0.0
        else:
            f_number = float(self.number)
        
        if self.number_mode == NUMBER_MODE_POINT:
            self.point.append(f_number)
            if len(self.point)== 3:
                self.shape.points.append(geom.Point3D(self.point[0], self.point[1], self.point[2]))
                self.point = []
        elif self.number_mode == NUMBER_MODE_COORD_INDEX:
            i = int(self.number)
            if i == -1:
                self.shape.coordIndex.append(self.coords)
                self.coords = []
            else:
                self.coords.append(i)
            
        self.number = ''
        self.e_found = False
    
        
    def Import(self, path):
        # returns 1 if all good, else 0
        self.path = path
        
        f = open(path, 'r')
        while True:
            line = f.readline()
            if not line:
                break
            self.line_number += 1
            
            for c in line:
                word_added = False
                number_added = False
                if len(self.word) == 0 and (ord(c) >= 48 and ord(c) <= 57 or c == '.' or c == '-'):
                    self.number += c
                    number_added = True
                elif c == 'e' and len(self.number) > 0:
                    self.e_found = True
                    number_added = True
                elif c.isspace():
                    pass
                elif c == '{':
                    self.curlys += 1
                elif c == '}':
                    self.curlys -= 1
                    if self.curlys < 0:
                        print('mismatched } on line ' + str(self.line_number) + ' reading file: ' + self.path)
                        return 0
                    if (self.curlys == 0) and (self.shape != None):
                        # end of shape definition, add the stl object to CAD
                        self.shape.AddStls()
                        self.number_mode = NUMBER_MODE_NOT_SET
                        self.shape = None
                elif c == '[':
                    self.squares += 1
                elif c == ']':
                    self.add_number()
                    self.squares -= 1
                    if self.squares < 0:
                        print('mismatched ] on line ' + str(self.line_number) + ' reading file: ' + self.path)
                        return 0
                elif c == ',':
                    self.add_number()
                else:
                    self.word += c
                    word_added = True
                    
                if not number_added:
                    #number has ended
                    self.add_number()
                    
                if not word_added and len(self.word) > 0:
                    #word has ended, process it
                    if self.word.lower() == 'shape':
                        self.shape = Shape() # start a new shape
                    elif self.word.lower() == 'point':
                        self.number_mode = NUMBER_MODE_POINT
                    elif self.word.lower() == 'coordindex':
                        self.number_mode = NUMBER_MODE_COORD_INDEX
                    self.word = ''
        return 1
        
def Import(path):
    importer = WrlImporter()
    return importer.Import(path)
                
            
             
    
def Export(path):
    SOLID_TYPE = step.GetSolidType()
    f = open(path, 'w')
    
    f.write("#VRML V2.0 utf8\n\n")
    doc = cad.GetApp()
    object = doc.GetFirstChild()
    
    color_set = set()
    face_list = []
        
    while object:
        if object.GetType() == SOLID_TYPE:
            solid = object
            solid.__class__ = step.Solid
            faces = solid.GetFaces()
            for face in faces:
                color = face.GetColor().ref()
                color_set.add(color)
                face_list.append(WrlFace(color, face))
                
        else:
            get_tris = getattr(object, "GetTris", None)
            if callable(get_tris):
                color = object.GetColor().ref()
                color_set.add(color)
                face_list.append(WrlFace(color, object))
                
        object = doc.GetNextChild()

    # write the materials
    for color in color_set:
        f.write('Shape {\n\tappearance Appearance {\n\t\tmaterial DEF M' + hex(color) + ' Material {\n\t\t\tdiffuseColor ' + str(color_red(color)) + ' ' + str(color_green(color)) + ' ' + str(color_blue(color))
                + '\n\t\t}\n\t}\n}\n\n')
        
    # write the faces
    for face in face_list:
        f.write('Shape { geometry IndexedFaceSet \n{ creaseAngle 0.50 coordIndex \n[')
        i = 0
        for t in face.flist:
            for v in t:
                if i != 0:
                    f.write(',')
                f.write(str(v))
                i = i + 1
            f.write(',-1')
        f.write(']\ncoord Coordinate { point [')
        i = 0
        for vt in face.vlist:
            if i != 0:
                f.write(',')
            j = 0
            for coord in vt:
                if j != 0:
                    f.write(' ')
                f.write(str(coord * MM_TO_WRL_SCALE))
                j = j + 1
            i = i + 1
        f.write(']\n}}\nappearance Appearance{material USE M' + hex(face.color) + ' }\n}\n')
    
    f.close()
    