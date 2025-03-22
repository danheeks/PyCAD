import wx
from   wx.adv import SplashScreen as SplashScreen

class MySplashScreen(SplashScreen):
    """
    Create a splash screen widget.
    """
    def __init__(self, parent=None):

        #------------

        # This is a recipe to a the screen.
        # Modify the following variables as necessary.
        bitmap = wx.Bitmap(name="Splash.png", type=wx.BITMAP_TYPE_PNG)
        splash = wx.adv.SPLASH_CENTRE_ON_SCREEN | wx.adv.SPLASH_TIMEOUT
        duration = 3000 # milliseconds

        # Call the constructor with the above arguments
        # in exactly the following order.
        super(MySplashScreen, self).__init__(bitmap=bitmap,
                                             splashStyle=splash,
                                             milliseconds=duration,
                                             parent=None,
                                             id=-1,
                                             pos=wx.DefaultPosition,
                                             size=wx.DefaultSize,
                                             style=wx.STAY_ON_TOP |
                                                   wx.BORDER_NONE)

        self.Bind(wx.EVT_CLOSE, self.OnExit)

    #-----------------------------------------------------------------------

    def OnExit(self, event):
        """
        ...
        """

        # The program will freeze without this line.
        event.Skip()  # Make sure the default handler runs too...
        self.Hide()

app = wx.App()
MySplash = MySplashScreen()
MySplash.CenterOnScreen(wx.BOTH)
MySplash.Show(True)
app.MainLoop()
