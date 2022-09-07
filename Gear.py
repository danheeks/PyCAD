from Object import Object
import geom
import wx
import cad
import math
from Object import PyProperty
from Object import PyPropertyLength

type = 0

class Gear(Object):
    def __init__(self, mod = 1.0, num_teeth = 12):
        Object.__init__(self, 0)
        self.tm = geom.Matrix()
        self.numTeeth = num_teeth
        self.module = mod
        self.addendumOffset = 0.0
        self.addendumMultiplier = 1.0
        self.dedendumMultiplier = 1.0
        self.pressureAngle = 0.34906585039886 # 20 degrees
        self.tipRelief = 0.05
        self.rootChamfer = 0.03
        self.numInvoluteFacets = 10
        self.color = cad.Color(128, 128, 128)
        
    def GetIconFilePath(self):
        return wx.GetApp().cad_dir + '/icons/gear.png'

    def GetTitle(self):
        teeth_str = str(self.numTeeth) + (' tooth' if self.numTeeth == 1 else ' teeth')
        return "Gear - M" + str(self.module) + ' ' + teeth_str

    def GetType(self):
        return type

    def GetTypeString(self):
        return "Gear"

    def GetColor(self):
        return self.color
        
    def SetColor(self, col):
        self.color = col

    def OnGlCommands(self, select, marked, no_color):
        points = self.GetPoints(0.1)
        if len(points) == 0:
            return
        
        cad.DrawPushMatrix()
        cad.DrawMultMatrix(self.tm)
        
        if not no_color:
            cad.DrawContrastBlackOrWhite()
        cad.BeginLines()
        for point in points:
            cad.GlVertex2D(point)
        cad.GlVertex2D(points[0])
        cad.EndLinesOrTriangles()
        
        a = geom.Area()
        c = geom.Curve()
        for point in points:
            c.Append(point)
        c.Append(points[0])
        a.Append(c)
        
        t_list = a.GetTrianglesList()
        if not no_color:
            cad.DrawColor(self.color)
        for t in t_list:
            cad.DrawTris(t, True)
        cad.DrawPopMatrix()
            
    def Transform(self, mat):
        self.tm.Multiply(mat)
            
    def WriteXml(self):
        cad.SetXmlMatrix('tm', self.tm)
        Object.WriteXml(self)

    def ReadXml(self):
        self.tm = cad.GetXmlMatrix('tm')
        Object.ReadXml(self)
                
    def MakeACopy(self):
        copy = Gear()
        copy.CopyFrom(self)
        return copy
    
    def CopyFrom(self, o):
        self.tm = o.tm
        Object.CopyFrom(self, o)
    
    def GetBox(self):
        box = geom.Box3D()
        pitch_radius = float(self.module) * self.numTeeth * 0.5
        outside_radius = pitch_radius + (self.addendumMultiplier*self.module + self.addendumOffset)
        steps = 20
        angle_step = 2.0 * math.pi / steps
        angle = 0.0
        for i in range(0, 20):
            p = geom.Point3D(outside_radius * math.cos(angle), outside_radius * math.sin(angle), 0.0)
            p.Transform(self.tm)
            box.InsertPoint(p.x, p.y, p.z)
            angle += angle_step
        return box
     
    def GetProperties(self):
        properties = []
        properties.append(PyProperty("num teeth", 'numTeeth', self))
        properties.append(PyProperty("module", 'module', self))
        properties.append(PyPropertyLength("addendum offset", 'addendumOffset', self))
        properties.append(PyPropertyLength("addendum multiplier", 'addendumMultiplier', self))
        properties.append(PyPropertyLength("dedendum multiplier", 'dedendumMultiplier', self))
        properties.append(PyProperty("pressure angle", 'pressureAngle', self))
        properties.append(PyProperty("tip relief", 'tipRelief', self))
        properties.append(PyProperty("root chamfer", 'rootChamfer', self))
        properties.append(PyProperty("num involute facets", 'numInvoluteFacets', self))        
                                             
        properties += Object.GetProperties(self)

        return properties
        
    def GetPoints(self, tolerance):
        pitch_radius = float(self.module) * self.numTeeth * 0.5
        inside_radius = pitch_radius - self.dedendumMultiplier*self.module
        outside_radius = pitch_radius + (self.addendumMultiplier*self.module + self.addendumOffset)
        base_radius = pitch_radius * math.cos(self.pressureAngle)

        if inside_radius < base_radius:
             inside_radius = base_radius
        
        inside_phi_and_angle = involute_intersect(inside_radius, base_radius)
        outside_phi_and_angle = involute_intersect(outside_radius, base_radius)
        tip_relief_phi_and_angle = involute_intersect(outside_radius - self.tipRelief, base_radius)
        middle_phi_and_angle = involute_intersect(pitch_radius, base_radius)

        points = []
        clearance = math.fabs(self.GetClearanceMM())
        
        for i in range(0, self.numTeeth):
            tooth_angle = math.pi*2*i/self.numTeeth
            next_tooth_angle = math.pi*2*(i+1)/self.numTeeth
            
            relief_vector = geom.Point(math.cos(tooth_angle), math.sin(tooth_angle))
            root_vector = ~relief_vector
            
            # incremental_angle - to space the middle point at a quarter of a cycle
            incremental_angle = 0.5*math.pi/self.numTeeth - middle_phi_and_angle[1]
            angle1 = tooth_angle - (inside_phi_and_angle[1] + incremental_angle)
            angle2 = tooth_angle + (inside_phi_and_angle[1] + incremental_angle);
            angle3 = tooth_angle + (outside_phi_and_angle[1] + incremental_angle)
            angle4 = next_tooth_angle - (outside_phi_and_angle[1] + incremental_angle)
            
            if clearance > 0.0000000001:
                p1 = geom.Point(math.cos(angle1) * inside_radius, math.sin(angle1) * inside_radius) + relief_vector * (-clearance)
                p2 = geom.Point(math.cos(angle2) * inside_radius, math.sin(angle2) * inside_radius) + relief_vector * (-clearance)

                if self.rootChamfer > 0.000000001:
                    points.append(p1 + relief_vector * self.rootChamfer)
                    points.append(p1 + root_vector * self.rootChamfer)
                    points.append(p2 + root_vector * (-self.rootChamfer))
                    points.append(p2 + relief_vector * self.rootChamfer)
                else:
                    points.append(p1 + v_in * clearance)
                    points.append(p2 + v_in * clearance)

            involute(points, tooth_angle + incremental_angle, False, inside_phi_and_angle, tip_relief_phi_and_angle, base_radius, self.numInvoluteFacets)
            
            if math.fabs(self.tipRelief) > 0.00000000001:
                points.append(point_at_rad_and_angle(outside_radius, angle3 + (self.tipRelief * 0.5)/outside_radius))
                points.append(point_at_rad_and_angle(outside_radius, angle4 - (self.tipRelief * 0.5)/outside_radius))

            involute(points, next_tooth_angle - incremental_angle, True, inside_phi_and_angle, tip_relief_phi_and_angle, base_radius, self.numInvoluteFacets)
        
        return points
                              
    def GetClearanceMM(self):
        # 12 teeth clearance 0.8
        # 20 teeth clearance 0.55
        # 52 teeth clearance 0.4
        # 100000 teeth clearance 0.1

        return (8.4 / ( 7.2 + self.numTeeth/2.5 )) * self.module


def point_at_phi(phi, base_radius):
    x = base_radius * phi
    sx = math.cos(phi) * base_radius
    sy = math.sin(phi) * base_radius
    return geom.Point(sx + math.sin(phi) * x, sy - math.cos(phi) * x)

def point_at_rad_and_angle(r, angle):
    return geom.Point(r * math.cos(angle), r * math.sin(angle))

def involute_intersect1(r, base_radius, phi, phi_step):
    while True:
        p = point_at_phi(phi, base_radius)
        if p.Length() > r:
            if phi_step > 0.0000000001:
                return involute_intersect1(r, base_radius, phi-phi_step, phi_step * 0.5)
            return (phi, math.atan2(p.y, p.x))
        phi = phi + phi_step
            
def involute_intersect(r, base_radius):
    return involute_intersect1(r, base_radius, 1.0, 1.0)
    
    
def involute(points, tooth_angle, do_reverse, inside_phi_and_angle, tip_relief_phi_and_angle, base_radius, steps):
    first = True
    
    start = steps if do_reverse else 0
    stop = -1 if do_reverse else (steps + 1)
    step = -1 if do_reverse else 1
    for i in range(start, stop, step):
        phi = inside_phi_and_angle[0] + (tip_relief_phi_and_angle[0] - inside_phi_and_angle[0])*i/steps

        # calculate point on the first tooth
        p = point_at_phi(phi, base_radius)
        if do_reverse:
            p.y = -p.y # mirror for reverse

        # rotate by tooth angle
        x = p.x * math.cos(tooth_angle) -p.y*math.sin(tooth_angle)
        y = p.y * math.cos(tooth_angle) + p.x * math.sin(tooth_angle)
        points.append( geom.Point(x, y))

   
            