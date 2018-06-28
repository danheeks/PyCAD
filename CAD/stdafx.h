
#if wxUSE_UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif

#define _WARNINGS 3

#ifdef WIN32
#pragma warning(disable : 4996)
#endif

#ifdef WIN32
#pragma warning(disable:4100)
#pragma warning(  disable : 4244 )        // Issue warning 4244
#endif

#ifdef WIN32
#pragma warning(  default : 4244 )        // Issue warning 4244
#endif

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif


#ifdef WIN32
#include "windows.h"
#endif


#if wxUSE_UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif

#include <algorithm>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <ctime>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>

#ifdef __WXMSW__
#ifdef _DEBUG
#include <wx/msw/msvcrt.h>      // redefines the new() operator 
#endif
#endif

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include "wx/brush.h"
#include "wx/button.h"
#include <wx/clipbrd.h>
#include <wx/checklst.h>
#include "wx/choice.h"
#include <wx/choicdlg.h>
#include <wx/cmdline.h>
#include "wx/combobox.h"
#include <wx/confbase.h>
#include <wx/config.h>
#include "wx/cursor.h"
#include <wx/dc.h>
#include "wx/dcclient.h"
#include <wx/dcmirror.h>
#include "wx/defs.h"
#include "wx/dirdlg.h"
#include "wx/dnd.h"
#include <wx/dynlib.h>
#include "wx/event.h"
#include <wx/fileconf.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/glcanvas.h>
#include "wx/hash.h"
#include <wx/image.h>
#include <wx/imaglist.h>
#include "wx/intl.h"
#include "wx/layout.h"
#include "wx/log.h"
#include <wx/menuitem.h>
#include "wx/msgdlg.h"
#include "wx/object.h"
#include "wx/panel.h"
#include "wx/pen.h"
#include "wx/popupwin.h"
#include <wx/print.h>
#include <wx/printdlg.h>
#include "wx/settings.h"
#include "wx/scrolwin.h"
#include "wx/sizer.h"
#include "wx/stattext.h"
#include <wx/stdpaths.h>
#include "wx/string.h"
#include <wx/sizer.h>
#include "wx/textctrl.h"
#include "wx/textdlg.h"
#include <wx/toolbar.h>
#include <wx/tooltip.h>
#include <wx/treectrl.h>
#include "wx/window.h"
#include <wx/tokenzr.h>
