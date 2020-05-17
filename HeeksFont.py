import geom
import cad
import os

POSITION_COLOUR = 'ff70ff'

def MakeCurveFromD(d, group_name):
    curve = geom.Curve()
    mode = 'x' # x y h v cx1 cx2 cy1 cy2
    nums = ''
    curp = geom.Point(0,0)
    absolute = True
    move = True
    d += ' '

    for c in d:
        if c == 'm':
            absolute = False
            move = True
            mode = 'x'
        elif c == 'M':
            absolute = True
            move = True
            mode = 'x'
        elif c == 'l':
            absolute = False
            mode = 'x'
        elif c == 'L':
            absolute = True
            mode = 'x'
        elif c == 'h':
            absolute = False
            mode = 'h'
        elif c == 'H':
            mode = 'h'
            absolute = True
        elif c == 'v':
            absolute = False
            mode = 'v'
        elif c == 'V':
            absolute = True
            mode = 'v'
        elif c == 'c':
            mode = 'cx1'
            absolute = False
        elif c == 'C':
            mode = 'cx1'
            absolute = True
        elif c == ' ':
            if len(nums) > 0:
                if mode == 'y':
                    if absolute or move: curp.y = -float(nums)
                    else: curp.y -= float(nums)
                    curve.Append(curp)
                    mode = 'x'
                    move = False
                elif mode == 'x':
                    raise NameError("mode x expecting comma!\n" + d)
                elif mode == 'h':
                    if absolute: curp.x = float(nums)
                    else: curp.x += float(nums)
                    curve.Append(curp)
                elif mode == 'v':
                    if absolute: curp.y = -float(nums)
                    else: curp.y -= float(nums)
                    curve.Append(curp)
                elif mode == 'cy1':
                    mode = 'cx2'
                elif mode == 'cy2':
                    mode = 'x'
                nums = ''
        elif c == ',':
            if mode == 'x':
                if absolute: curp.x = float(nums)
                else: curp.x += float(nums)
                mode = 'y'
            elif mode == 'cx1':
                mode = 'cy1'
            elif mode == 'cx2':
                mode = 'cy2'
            else:
                raise NameError("mode " + mode + " not expecting comma!")
            nums = ''
        elif c == 'z' or c == 'Z':
            # close the curve
            curve.Append(curve.FirstVertex().p)
        else:
            nums += c
    
    return curve

def cchr(i):
    c = chr(i)
    if c == '\\': c = 'backslash'
    return c

class Character:
    def __init__(self, curve_list, pos, width, tris):
        self.curve_list = curve_list
        self.pos = pos
        self.width = width        
        self.tris = tris
        
class Tri:
    def __init__(self, p0,p1,p2):
        self.p0 = p0
        self.p1 = p1
        self.p2 = p2
        
    def Transform(self, mat):
        self.p0.Transform(mat)
        self.p1.Transform(mat)
        self.p2.Transform(mat)

def ConvertHeeksFont():
    chars = {}

    import xml.etree.ElementTree as ET

    this_dir = os.path.dirname(os.path.realpath(__file__))
    tree = ET.parse(this_dir + '/arial font.svg')
    root = tree.getroot()
    
    for child in root:
        if child.attrib['id'] == 'layer1':
            for item in child:
                if item.tag == '{http://www.w3.org/2000/svg}g':
                    expected_label = '{http://www.inkscape.org/namespaces/inkscape}label'
                    if expected_label in item.attrib:
                        label = item.attrib[expected_label]
                        if label[:4] == 'char' and len(label) == 5:
                            c = label[4]
                            curves = []
                            tris = []
                            pos = geom.Point(0,0)
                            width = 10.0
                            for path in item:
                                stroke = None
                                fill = None
                                styles = path.attrib['style'].split(';')
                                for style in styles:
                                    pair = style.split(':')
                                    if len(pair) == 2:
                                        if pair[0] == 'stroke':
                                            if len(pair[1]) == 7:
                                                stroke = pair[1][1:7]
                                            else:
                                                stroke = None
                                        elif pair[0] == 'fill':
                                            if len(pair[1]) == 7:
                                                fill = pair[1][1:7]
                                            else:
                                                fill = None
                                
                                curve = MakeCurveFromD(path.attrib['d'], c)
                                box = curve.GetBox()
                                if stroke == POSITION_COLOUR:
                                    pos = geom.Point(box.MinX(), box.MinY())
                                    width = box.MaxX() - box.MinX()
                                elif stroke != None:
                                    curves.append(curve)
                                elif fill != None:
                                    if curve.NumVertices() == 4:
                                        # triangle curve
                                        vertices = curve.GetVertices()
                                        tris.append(Tri(vertices[0].p, vertices[1].p, vertices[2].p))
                            chars[c] = Character(curves, pos, width, tris)
    
    # now write the c++ file
    f = open(this_dir + '/CAD/HeeksFontData.h', 'w')
    f.write('// HeeksFontData.h written automatically by HeeksFont.py\n\n')
            
    num_curves = []
    start_curves = []
    widths = []
    all_curves = []
    num_triangles = []
    start_tris = []
    all_tris = []
    
    curve_index = 0
    tri_index = 0
    
    for i in range(32, 127):
        c = chr(i)
        pos = geom.Point(0,0)
        width = 10.0
        curves = []
        tris = []
        
        if c in chars:
            character = chars[c]
            curves = character.curve_list
            pos = character.pos
            width = character.width
            tris = character.tris
            
        n = len(curves)
        num_curves.append(n)
        start_curves.append(curve_index)
        nt = len(tris)
        num_triangles.append(nt)
        start_tris.append(tri_index)
        curve_index += n
        tri_index += nt        
            
        mat = geom.Matrix()
        mat.Translate(-pos.x, -pos.y, 0)
        mat.Scale(0.037037037)
            
        for curve in curves:
            curve.Transform(mat)
            all_curves.append(curve)
            
        for tri in tris:
            tri.Transform(mat)
            all_tris.append(tri)

        widths.append(width * 0.037037037)
            
    f.write('static int num_curves[95] =   {')

    i = 32
    for n in num_curves:
        f.write(str(n) + ', //' + cchr(i) + '\n')
        i += 1
        
    f.write('};\n')
    
    f.write('static int start_curves[95] = {')

    i = 32
    for n in start_curves:
        f.write(str(n) + ', //' + cchr(i) + '\n')
        i += 1
        
    f.write('};\n\n\n')
    
    f.write('static float widths[95] = {\n')

    i = 32
    for width in widths:
        f.write(str(width) + 'f, //' + cchr(i) + '\n')
        i += 1
        
    f.write('};\n\n\n')
    
    point_index = 0
    start_points = []
    num_points = []
    points = []
    for curve in all_curves:
        vertices = curve.GetVertices()
        n = len(vertices)
        num_points.append(n)
        start_points.append(point_index)
        point_index += n
        for v in vertices:
            points.append(v.p)
            
    if len(num_points) == 0:
        f.write('static int* num_points;\n')
    else:
        f.write('static int num_points[' + str(len(num_points)) + '] =   {')

        i = 0
        for n in num_points:
            f.write(str(n) + ', //' + str(i) + '\n')
            i += 1
            
        f.write('};\n')
        
    if len(start_points) == 0:
        f.write('static int* start_points;\n')
    else:
        f.write('static int start_points[' + str(len(start_points)) + '] = {')

        i = 0
        for n in start_points:
            f.write(str(n) + ', //' + str(i) + '\n')
            i += 1
            
        f.write('};\n')
    
    if len(points) == 0:
        f.write('static float** points;\n')
    else:
        i = 0
        f.write('static float points[' + str(len(points)) + '][2] = {\n')
        for p in points:
            f.write('       {' + str(float(p.x)) + 'f, ' + str(float(p.y)) + 'f}, //' + str(i) + '\n')
            i += 1
        f.write('};\n')
            
    f.write('static int num_triangles[95] =   {')

    i = 32
    for n in num_triangles:
        f.write(str(n) + ', //' + cchr(i) + '\n')
        i += 1
        
    f.write('};\n')
    
    f.write('static int start_tris[95] = {')

    i = 32
    for n in start_tris:
        f.write(str(n) + ', //' + cchr(i) + '\n')
        i += 1
        
    f.write('};\n\n\n')
    
    i = 0
    f.write('static float tris[' + str(len(all_tris)) + '][6] = {\n')
    for t in all_tris:
        f.write('       {' + str(float(t.p0.x)) + 'f, ' + str(float(t.p0.y)) + 'f, ' + str(float(t.p1.x)) + 'f, ' + str(float(t.p1.y)) + 'f, ' + str(float(t.p2.x)) + 'f, ' + str(float(t.p2.y)) + 'f}, //' + str(i) + '\n')
        i += 1
    f.write('};\n')
    
    
    f.close()
    
    print('successfully created HeeksFont.h')
            
