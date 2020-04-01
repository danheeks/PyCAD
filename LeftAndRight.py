class LeftAndRight:
    def __init__(self):
        self.right_up_with_left_down_done = False
        self.left_up_with_right_down_done = False
        
    def LeftAndRightPressed(self, event):
        # returns left_and_right_pressed, event_used
        if event.LeftUp():
            if self.right_up_with_left_down_done:
                self.right_up_with_left_down_done = False
                return True, True
            elif event.rightDown:
                self.left_up_with_right_down_done = True
                return False, True
        elif event.RightUp():
            if self.left_up_with_right_down_done:
                self.left_up_with_right_down_done = False
                return True, True
            elif event.leftDown:
                self.right_up_with_left_down_done = True
                return False, True
        return False, False
