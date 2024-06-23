import cad
import step

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
    