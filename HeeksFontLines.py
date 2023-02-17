import geom
import cad
import os

def cchr(i):
    c = chr(i)
    if c == '\\': c = 'backslash'
    return c

class Character:
    def __init__(self, c):
        self.c = c
        self.pos = None
        self.width = 0.0        
        self.line_strips = []
        
    def AddSketch(self, s):
        a = s.GetArea()
        c = a.GetCurves()[0]
        ls = LineStrip()
        for v in c.GetVertices():
            ls.pts.append(v.p)
        self.line_strips.append(ls)
    
class LineStrip:
    def __init__(self):
        self.pts = []
        
    def Transform(self, mat):
        for p in self.pts:
            p.Transform(mat)

def ConvertHeeksLines():
    this_dir = os.path.dirname(os.path.realpath(__file__))
    chars = {}
    chars[' '] = Character(' ')

    for object in cad.GetObjects():
        if object.GetType() == cad.OBJECT_TYPE_SKETCH:
            title = object.GetTitle()
            if len(title) == 2 and title[0] == '_':
                box = object.GetBox()
                chars[title[1]].pos = geom.Point(box.MinX(), box.MinY())
                chars[title[1]].width = box.Width()
            else:
                c = title[0]
                if c not in chars:
                    chars[c] = Character(c)
                sketch = object
                sketch.__class__ = cad.Sketch
                chars[c].AddSketch(sketch)
                
    # now write the c++ file
    f = open(this_dir + '/CAD/HeeksFontLineData.h', 'w')
    f.write('// HeeksFontLineData.h written automatically by HeeksFontLines.py\n\n')
    f.write('double RenderCharLines(char c)\n{\n\tswitch(c){\n')
    
    scale = 0.0133333
            
    for i in range(32, 127):
        c = chr(i)
        
        if c in chars:
            f.write('\t\tcase ' + str(i) + ':\n')
            ch = chars[c]
            for line_strip in ch.line_strips:
                f.write('\t\t\tglBegin(GL_LINE_STRIP);\n')
                for pt in line_strip.pts:
                    f.write('\t\t\tglVertex2d(' + str((pt.x - ch.pos.x) * scale) + ', ' + str((pt.y - ch.pos.y) * scale) + ');\n')
                f.write('\t\t\tglEnd();\n')
            f.write('\t\t\treturn ' + str(ch.width * scale) + ';\n')
        else:
            print('missing char: ' + c)
    f.write('\t\tdefault:\n\t\t\treturn 0.0;')
    f.write('\t}\n}\n')
    
    f.write('\n\n')
    f.write('double GetLinesCharWidth(char c)\n{\n\tswitch(c){\n')
    
    for i in range(32, 127):
        c = chr(i)
        
        if c in chars:
            f.write('\t\tcase ' + str(i) + ':\n')
            ch = chars[c]
            f.write('\t\t\treturn ' + str(ch.width * scale) + ';\n')
        else:
            print('missing char: ' + c)
    f.write('\t\tdefault:\n\t\t\treturn 0.0;')
    f.write('\t}\n}\n')
    
    f.close()
    
    print('successfully created HeeksFontLineData.h')
            
