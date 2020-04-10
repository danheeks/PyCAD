from App import App
from HeeksConfig import HeeksConfig
import step
import cad

class SolidApp(App):
    def __init__(self):
        App.__init__(self)

    def GetAppName(self):
        return 'Solid CAD ( Computer Aided Design )'

    def RegisterObjectTypes(self):
        step.SetResPath(self.cad_dir)
        step.SetApp(cad.GetApp())
        App.RegisterObjectTypes(self)
        step.SetStepFileObjectType(cad.RegisterObjectType("STEP_file", step.CreateStepFileObject))

    def GetObjectTools(self, object, control_pressed, from_tree_canvas = False):
        tools = App.GetObjectTools(self, object, control_pressed, from_tree_canvas)
#        if object.GetType() == Profile.type:
#            tools.append(CamContextTool.CamObjectContextTool(object, "Add Tags", "addtag", self.AddTags))
        return tools
