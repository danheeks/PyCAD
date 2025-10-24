from Object import Object
import geom
import wx
import cad
import math
from Object import PyProperty
from Object import PyPropertyLength
from Object import PyPropertyDoubleScaled

type = 0

class Gear(Object):
    def __init__(self, mod = 1.0, num_teeth = 12):
        Object.__init__(self, 0)
        self.tm = geom.Matrix()
        self.solid = None
        self.numTeeth = num_teeth
        self.module = mod
        self.addendumOffset = 0.0
        self.addendumMultiplier = 1.0
        self.dedendumMultiplier = 1.0
        self.pressureAngle = 0.34906585039886 # 20 degrees
        self.tipRelief = 0.05
        self.rootClearance = 0.1
        self.rootRoundness = 0.25
        self.numInvoluteFacets = 10
        self.thickness = 5.0
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
        
    def KillGLLists(self):
        self.solid = None
        
    def OnRecalculate(self):
        self.KillGLLists()
        wx.GetApp().Repaint()

    def OnGlCommands(self, select, marked, no_color):
        if self.solid == None:
            self.solid = self.MakeSolid()
        
        self.solid.OnGlCommands(select, marked, no_color)
            
    def Transform(self, mat):
        self.tm.Multiply(mat)
            
    def WriteXml(self):
        cad.SetXmlMatrix('tm', self.tm)
        cad.SetXmlValue('numTeeth', self.numTeeth)
        cad.SetXmlValue('module', self.module)
        cad.SetXmlValue('addendumOffset', self.addendumOffset)
        cad.SetXmlValue('addendumMultiplier', self.addendumMultiplier)
        cad.SetXmlValue('dedendumMultiplier', self.dedendumMultiplier)
        cad.SetXmlValue('pressureAngle', self.pressureAngle)
        cad.SetXmlValue('tipRelief', self.tipRelief)
        cad.SetXmlValue('rootClearance', self.rootClearance)
        cad.SetXmlValue('rootRoundness', self.rootRoundness)        
        cad.SetXmlValue('numInvoluteFacets', self.numInvoluteFacets)
        cad.SetXmlValue('thickness', self.thickness)
        Object.WriteXml(self)

    def ReadXml(self):
        self.tm = cad.GetXmlMatrix('tm')
        self.numTeeth = cad.GetXmlInt('numTeeth', self.numTeeth)
        self.module = cad.GetXmlFloat('module', self.module)
        self.addendumOffset = cad.GetXmlFloat('addendumOffset', self.addendumOffset)
        self.addendumMultiplier = cad.GetXmlFloat('addendumMultiplier', self.addendumMultiplier)
        self.dedendumMultiplier = cad.GetXmlFloat('dedendumMultiplier', self.dedendumMultiplier)
        self.pressureAngle = cad.GetXmlFloat('pressureAngle', self.pressureAngle)
        self.tipRelief = cad.GetXmlFloat('tipRelief', self.tipRelief)
        self.rootClearance = cad.GetXmlFloat('rootClearance', self.rootClearance)
        self.rootRoundness = cad.GetXmlFloat('rootRoundness', self.rootRoundness)
        self.numInvoluteFacets = cad.GetXmlInt('numInvoluteFacets', self.numInvoluteFacets)
        self.thickness = cad.GetXmlFloat('thickness', self.thickness)
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
            pz = geom.Point3D(p.x, p.y, self.thickness)
            p.Transform(self.tm)
            pz.Transform(self.tm)
            box.InsertPoint(p.x, p.y, p.z)
            box.InsertPoint(pz.x, pz.y, pz.z)
            angle += angle_step
        return box
     
    def GetProperties(self):
        properties = []
        properties.append(PyProperty("num teeth", 'numTeeth', self, self.OnRecalculate))
        properties.append(PyProperty("module", 'module', self, self.OnRecalculate))
        properties.append(PyPropertyLength("addendum offset", 'addendumOffset', self, recalculate = self.OnRecalculate))
        properties.append(PyPropertyLength("addendum multiplier", 'addendumMultiplier', self, recalculate = self.OnRecalculate))
        properties.append(PyPropertyLength("dedendum multiplier", 'dedendumMultiplier', self, recalculate = self.OnRecalculate))
        properties.append(PyPropertyDoubleScaled("pressure angle", 'pressureAngle', self, 180.0 / math.pi, self.OnRecalculate))
        properties.append(PyProperty("tip relief", 'tipRelief', self, self.OnRecalculate))
        properties.append(PyProperty("root clearance", 'rootClearance', self, self.OnRecalculate))
        properties.append(PyProperty("root roundness", 'rootRoundness', self, self.OnRecalculate))
        properties.append(PyProperty("num involute facets", 'numInvoluteFacets', self, self.OnRecalculate))
        properties.append(PyPropertyLength("thickness", 'thickness', self, recalculate = self.OnRecalculate))
                                             
        properties += Object.GetProperties(self)

        return properties
    
    def AddSketch(self):
        cad.AddUndoably(self.MakeSketch())
        
    def MakeSketch(self):
        point_list = self.GetPoints()
        import step
        s = cad.NewSketch()
        for pts, spline in point_list:
            if spline:
                # add a spline
                three_d_pts = []
                for p in pts:
                    three_d_pts.append( geom.Point3D(p.x, p.y, 0) )
                s.Add( step.NewSplineFromPoints(three_d_pts) )
            else:
                # add lines
                prev_point = None
                for p in pts:
                    p3d = geom.Point3D(p.x, p.y, 0)
                    if prev_point != None:
                        s.Add(cad.NewLine(prev_point, p3d))
                    prev_point = p3d
        return s

    def AddSolid(self):
        cad.AddUndoably(self.MakeSolid())
        
    def MakeSolid(self):
        import step
        objects = [self.MakeSketch()]        
        new_solids = step.CreateExtrusion(objects, self.thickness, True, False, 0.0, cad.Color(128, 128, 128))
        s = new_solids[0]
        if s != None:
            # subtract cylinder in the middle
            cyl = step.NewCyl()
            cyl.radius = 1
            cyl.height = 20
            cyl.OnApplyProperties()
            objects = [s, cyl]
            s = step.CutShapes(objects)
        return s
        
    def GetPoints(self):
        # returns a list of list of Points, where each list of points could be splined
        point_list = [] # list of tuples ( points - list of Point, spline - to be splined or not )
        
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

        for i in range(0, self.numTeeth):
            tooth_angle = math.pi*2*i/self.numTeeth
            next_tooth_angle = math.pi*2*(i+1)/self.numTeeth
            
            relief_vector = geom.Point(math.cos(tooth_angle), math.sin(tooth_angle))
            root_vector = ~relief_vector
            
            # incremental_angle - to space the middle point at a quarter of a cycle
            incremental_angle = 0.5*math.pi/self.numTeeth - middle_phi_and_angle[1]

            # get the previous tooth's involute points
            prev_involute_points = []
            involute(prev_involute_points, tooth_angle - incremental_angle, True, inside_phi_and_angle, tip_relief_phi_and_angle, base_radius, self.numInvoluteFacets)

            # get the up hill involute points
            involute_points = []
            involute(involute_points, tooth_angle + incremental_angle, False, inside_phi_and_angle, tip_relief_phi_and_angle, base_radius, self.numInvoluteFacets)

            # get the downhill involute points
            involute_points2 = []
            involute(involute_points2, next_tooth_angle - incremental_angle, True, inside_phi_and_angle, tip_relief_phi_and_angle, base_radius, self.numInvoluteFacets)
            
            # root profile
            points = []
            # start with end of involute
            points.append(prev_involute_points[-1])
            # add a point to continue the same direction as the involute
            if math.fabs(self.rootRoundness) > 0.0001:
                end_vector = prev_involute_points[-1] - prev_involute_points[-2]
                end_vector.Normalize()
                points.append(prev_involute_points[-1] + end_vector * (self.rootClearance * self.rootRoundness))
            # add a mid point
            points.append(geom.Point(math.cos(tooth_angle) * inside_radius, math.sin(tooth_angle) * inside_radius) + relief_vector * (-self.rootClearance))
            # add a point the start the direction at the end
            if math.fabs(self.rootRoundness) > 0.0001:
                start_vector = involute_points[0] - involute_points[1]
                start_vector.Normalize()
                points.append(involute_points[0] + start_vector * (self.rootClearance * self.rootRoundness))
            # end with the start of the next involute
            points.append(involute_points[0])
            point_list.append((points, False))
            
            # up hill involute            
            point_list.append((involute_points, True))
            
            # tip relief
            points = []
            points.append(involute_points[-1])
            if math.fabs(self.tipRelief) > 0.00000000001:
                angle3 = tooth_angle + (outside_phi_and_angle[1] + incremental_angle)
                angle4 = next_tooth_angle - (outside_phi_and_angle[1] + incremental_angle)
                points.append(point_at_rad_and_angle(outside_radius, angle3 + (self.tipRelief * 0.5)/outside_radius))
                points.append(point_at_rad_and_angle(outside_radius, angle4 - (self.tipRelief * 0.5)/outside_radius))
            points.append(involute_points2[0])
            point_list.append((points, False))

            # downhill involute
            point_list.append((involute_points2, True))
        
        return point_list

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

   
            