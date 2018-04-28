//-----------------------------------------------------------------------------
//
//	MONOGRAM Multimedia FLV Mux
//
//
//-----------------------------------------------------------------------------

#include <afxwin.h>
#include <afxtempl.h>
#include <atlbase.h>

#include <streams.h>
#include <initguid.h>

#include <mmsystem.h>
#include <mmreg.h>
#include <ks.h>
#include <ksmedia.h>
#include <dvdmedia.h>

#define _MATH_DEFINES
#include <math.h>

#include <windowsx.h>

#include <list>
#include <vector>

using namespace std;

#include "..\resource.h"

#include "as_objects.h"

/******************************************************************************
**
**	use libMonoDShow to build this filter 
**
*******************************************************************************
*/
#include "mediasample_ex.h"
#include "basemux.h"


#include "flv_types.h"
#include "flv_writer.h"


#include "flv_filter.h"
#include "flv_prop.h"
