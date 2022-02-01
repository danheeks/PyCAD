import cad
from Drawing import Drawing

class PointDrawing(Drawing):
    def __init__(self):
        Drawing.__init__(self)
        
    # cad.InputMode's overridden method
    def GetTitle(self):
        return "Point drawing"

    def number_of_steps(self):
        return 1
    
    def is_an_add_level(self, level):
        return True
        
    def calculate_item(self, end):
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