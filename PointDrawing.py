import cad

class PointDrawing(cad.Drawing):
    def __init__(self):
        cad.Drawing.__init__(self)
        
    # cad.InputMode's overridden method
    def GetTitle(self):
        return "Point drawing"
    
    def IsAnAddLevel(self, level):
        return True
    
    def NumberOfSteps(self):
        return 1
        
    def CalculateItem(self, end):
        if end.type == cad.DigitizeType.DIGITIZE_NO_ITEM_TYPE:
            return False

        if (self.TempObject() != None) and (self.TempObject().GetType() != cad.OBJECT_TYPE_POINT):
            self.ClearObjectsMade()

        if self.TempObject() == None:
            self.AddToTempObjects(cad.NewPoint(end.point))
        else:
            self.TempObject().SetStartPoint(end.point)
            
        return True
        
point_drawing = PointDrawing()