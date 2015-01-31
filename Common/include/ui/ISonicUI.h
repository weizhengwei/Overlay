#pragma once


#ifdef SONICUI_EXPORTS
#define SONICUI_API _declspec(dllexport)
#else
#define SONICUI_API
#endif

#ifndef ARGB
#define ARGB(a, r, g, b) ((a) << 24 | (b) << 16 | (g) << 8 | (r))
#endif
#define CONVERT_RGB(rgb) ((rgb) >> 16 & 0xff | (rgb) << 16 & 0xff0000 | (rgb) & 0xff00 | (rgb) & 0xff000000)
#define DEFAULT_COLOR_KEY	RGB(128, 128, 128)
#define SONICUI_DESTROY(pBase) if(pBase){ GetSonicUI()->DestroyObject(pBase); pBase = NULL; }

//////////////////////////////////////////////////////////////////////////
//					SonicUI GUI engine
//
//	Author:			Sonic Guan
//	Date:			2014-10-29
//	Version:		v1.3
//	Introduction:	This is a GUI engine based on nake gdi APIs. It offers several UI components to
//					accomplish kinds of complex effects, such as irregular windows, URLs on windows, self-draw
//					buttons, animaiton and image operation methods, etc. The main purpose is to use the least
//					code to achieve the best effect so you can release yourself from repeated 
//					and flavourless UI work, and concentrate on the more meaningful part.
//	Statement:		You can use this code freely for any purpose(including commercial purpose), and modify or add components at your pleasure, 
//					but please don't erase this statement. 
//					Source code of SonicUI is under the license LGPL.
//////////////////////////////////////////////////////////////////////////

namespace sonic_ui
{
	class SONICUI_API CSafeCrtBuffer
	{
	public:
		virtual ~CSafeCrtBuffer(){}
		LPVOID operator new(size_t sz);
		LPVOID operator new(size_t sz, LPCSTR lpszFileName, int nLineNum);		// compatible with mfc
		LPVOID operator new[](size_t sz);
		void operator delete(LPVOID p);
		void operator delete[](LPVOID p);
	};

	class ISonicInputFilter
	{
	public:
		// get a chance to handle all input(mouse and keyboard) messages.
		// return true to pass through the message, and return false to eat the message
		virtual BOOL OnInputMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	};

// used to disable the position changing of button text when being clicked
#define SATTR_STRING_BTNTEXTSTATIC	_T("SAttrStringBtnTextStatic")
// used to make button with alpha channel, the transparent area of the button will not respond message
// transparent level: 
// 1 means any pixels with alpha less than 255 will not respond messages
// 2 means only pixels with alpha zero will not respond messages
#define SATTR_STRING_BTNALPHA		_T("SAttrStringBtnAlpha")
// used to control the padding width for the button text of a ISonicString
#define SATTR_STRING_LEFT_PADDING	_T("SAttrStringLeftPadding")
#define SATTR_STRING_RIGHT_PADDING	_T("SAttrStringRightPadding")
// used to control the alignment for the button text of a ISonicString
#define SATTR_STRING_BTNTEXT_ALIGN	_T("SAttrStringBtnTextAlign")
// used to control the horizontal space between tab buttons or icons in ISonicIconPanel
#define SATTR_HSPACE				_T("SAttrHSpace")
// used to control the vertical(line) space between icons in ISonicIconPanel
#define SATTR_VSPACE				_T("SAttrVSpace")
// used to control tile divider when tile render an image
#define SATTR_TILE_CXDIVIDER		_T("SAttrTileCxDivider")
#define SATTR_TILE_CYDIVIDER		_T("SAttrTileCyDivider")
// indicate the image should be scaled when vertically tile
#define SATTR_TILE_VERSCALE			_T("SAttrTileVerscale")
// the value of this attribute is considered as a CSafeCrtBuffer * pointer, which will be auto deleted when the host ISonic object is cleared
#define SATTR_AUTO_DELETE			_T("SAttrAutoDelete")
// used to control the horizontal and vertical margin in ISonic objects. the meaning of this attribute varies for different ISonic objects
#define SATTR_HMARGIN				_T("SAttrHMargin")
#define SATTR_VMARGIN				_T("SAttrVMargin")
// set this attribute for ISonicBaseWnd objects to activate the tracking mode of tooltip, that means the tooltip will go after the cursor
#define SATTR_TOOLTIP_TRACK			_T("SAttrTooltipTrack")
// set this attribute for ISonicBaseWnd objects to set a maximum width for its tooltip, default as 260, set 0 to remove the limit
#define SATTR_TOOLTIP_MAXWIDTH		_T("SAttrTooltipMaxWidth")
// if set this attribute for ISonicSkin, the whole window will be redrawn with a fixed interval, just like games doing
#define SATTR_SKIN_GAME_RENDERING	_T("SAttrGameRendering")
// if set this attribute for ISonicSkin, the window will receive keyboard messages when off-screen rendering
// if set this attribute for any ISonicBaseWnd(except ISonicString and ISonicPaint), the host window of the ISonicBaseWnd will get the focus  when the object is being clicked
#define SATTR_ACCEPT_FOCUS		_T("SAttrAcceptFocus")
// set border color of ISonicTab, default as the color of the border of the top tab image
#define SATTR_TAB_BORDER_COLOR		_T("SAttrTabBorderColor")
// remove the border of left, right and bottom edge
#define SATTR_TAB_SIMPLE_BORDER		_T("SAttrTabSimpleBorder")
// set this attribute to control the drag level of ISonicTab
// 0 means cannot drag tab items
// 1 means tab items can be dragged within the area of ISonicTab
// 2 means tab items can be dragged outside the boundary of ISonicTab
#define SATTR_TAB_DRAG_LEVEL		_T("SAttrTabDragLevel")
// this attribute must be set before any loading operation. if this attributes is set to true, ISonicImage will keep the original data
// without adjusting its RGB value with alpha
#define SATTR_IMAGE_RAWDATA			_T("SAttrImageRawData")
// this attribute is used to adjust offset between the real window and its shadow border, not commonly used.
#define SATTR_IMAGE_OFFSET			_T("SAttrImageOffset")
// disable default effect of ISonicParticleEffect
// for water wave effect, the default effect is a water wave following the mouse when being hovered
#define SATTR_PE_NODEFAULT_EFFECT	_T("SAttrPENoDefaultEffect")
// set the damping level of water wave, the higher this value, the faster wave resume to peace
// valid interval of this value is 1 to 8, default as 5
#define SATTR_PE_WATERWAVE_DAMPING	_T("SAttrPEWaterWaveDamping")
// set this attribute to an Item of ISonicIconPanel or ISonicTab to disable the drag feature
#define SATTR_ICON_NODRAG			_T("SAttrIconNoDrag")
// set this attribute to ISonicIconPanel to hide all partial tab items
#define SATTR_ICON_NOPARTIALITEM	_T("SAttrTabNoPartialItem")
// set this attribute to ISonicImage when it's holding a gif, to control the interval between frames. the final interval is the original interval multiple this value, default as 10
#define SATTR_GIF_INTERVAL_MULTIPLE	_T("SAttrGifIntervalMultiple")
// set this attribute to any objects with a vertical scroll bar to show or hide it
#define SATTR_ENABLE_VSCROLLBAR		_T("SAttrEnableVScrollBar")
// set this attribute to any objects with a horizontal scroll bar to show or hide it
#define SATTR_ENABLE_HSCROLLBAR		_T("SAttrEnableHScrollBar")
// set this attribute to ISonicTreeView or its item to enable drag operation, if an ISonicTreeView has this attribute, all items are considered automatically having this
// this attribute, unless you separately turned off this attribute for an item.
#define SATTR_ENABLE_DRAG			_T("SAttrEnableDrag")
// set this attribute to ISonicTreeView to specify the color to identify an item which is the destination of a drag&drop
#define SATTR_DRAG_HOTITEMCOLOR		_T("SAttrDragHotItemColor")
// set this attribute to an item of ISonicTreeView to make a prompt item, which could be visible even it's scrolled to an invisible area but some of its child items are visible.
#define SATTR_TREE_PROMPTITEM		_T("SAttrTreePromptItem")
// set this attribute to an item of ISonicTreeView to set a personal select image
#define SATTR_TREE_ITEMSELECTIMAGE	_T("SAttrTreeItemSelectImage")
// set this attribute to an ISonicString being added to an ISonicTreeView, to make it as a sign of a expanded item
#define SATTR_TREE_EXPANDSIGN		_T("SAttrTreeExpandSign")
// set this attribute to an item of ISonicTreeView to control how it expands
// 0 means don't expand at all
// 1 means expanding by a single click
// 2 means expanding by a double click
// the default value of this attribute of every item is 2
#define SATTR_TREE_ITEMEXPAND		_T("SAttrTreeItemExpand")
// set this attribute to an item of ISonicTreeView to indicate if the item is selectable, default as true
#define SATTR_TREE_ITEMSELECTABLE	_T("SAttrTreeItemSelectable")
// set this attribute to a ISonicTreeView to disable automatically updating layout. this is often used when you want to quickly add or delete a mass of items
#define SATTR_TREE_NOAUTO_UPDATELAYOUT	_T("SAttrTreeNoAutoUpdateLayout")
	// Delegable events
	enum enDelegateEvent
	{
		DELEGATE_EVENT_NONE = 0,	
		DELEGATE_EVENT_CLICK,			// left button click
		DELEGATE_EVENT_RCLICK,			// right button click
		DELEGATE_EVENT_DBLCLICK,		// left button double click
		DELEGATE_EVENT_DBLRCLICK,		// right button double click
		DELEGATE_EVENT_MOUSEWHEEL,		// mouse wheel
		DELEGATE_EVENT_MOUSEENTER,		// mouse enter
		DELEGATE_EVENT_MOUSEHOVER,		// mouse hover
		DELEGATE_EVENT_MOUSELEAVE,		// mouse leave
		DELEGATE_EVENT_LBUTTONDOWN,		// left button down
		DELEGATE_EVENT_LBUTTONUP,		// left button up
		DELEGATE_EVENT_MBUTTONDOWN,		// middle button down
		DELEGATE_EVENT_MBUTTONUP,		// middle button up
		DELEGATE_EVENT_RBUTTONDOWN,		// right button down
		DELEGATE_EVENT_RBUTTONUP,		// right button up
		DELEGATE_EVENT_RAWMOUSEMOVE,	// raw mouse move, even the cursor is not in the region of the object
		DELEGATE_EVENT_RAWLBUTTONUP,	// raw left button up, even the cursor is not in the region of the object
		DELEGATE_EVENT_GIF_OVER,		// just after a gif complete playing all its frames
		DELEGATE_EVENT_SPARKLE_OVER,	// after sparkle
		DELEGATE_EVENT_SHOWGENTLY_OVER,	// after fadeout
		DELEGATE_EVENT_MOVEGENTLY_OVER,	// after move gently
		DELEGATE_EVENT_TRANSFORM_OVER,	// after transform
		DELEGATE_EVENT_ANI_NUM_OVER,	// after a number animation 
		DELEGATE_EVENT_PAINT,			// delegate a procedure to this event of ISonicPaint, which will be called while the Paint is drawing
		DELEGATE_EVENT_SLIDE_OVER,		// after slide
		DELEGATE_EVENT_SHUTTER_OVER,	// after shutter animation
		DELEGATE_EVENT_TIMER,			// timer for ISonicBaseWnd, callback param is the timer id
		DELEGATE_EVENT_SEL_CHANGED,		// notify that selection is changed in a container which could select a single item at a time, the callback param is a pointer to the old selected item
		DELEGATE_EVENT_TAB_DELETING,	// notify that a tab is being deleted in ISonicTab, callback param is the index
		DELEGATE_EVENT_POSCHANGED,		// notify that a ISonicBaseWnd has changed its position
		DELEGATE_EVENT_SIZECHANGED,		// notify that a ISonicBaseWnd has changed its size
		DELEGATE_EVENT_ENUM,			// notify that the ISonicBaseContainer is enumerating its children, the callback param is the child object being enumerated
		DELEGATE_EVENT_ICON_REORDER,	// notify that an Icon in ISonicIconPanel has changed its order, the callback param is the item(not the index of it) triggering the event
		DELEGATE_EVENT_ATTR_CHANGED,	// notify that an attribute has been modified, the callback param is the name of the attribute which was just changed
		DELEGATE_EVENT_DRAGOVER,		// notify that an item is dragging over the host object, the callback param is a pointer to a DROP_PARAM data object
		DELEGATE_EVENT_DROP,			// notify that a drop operation just occurred in the host object, the callback param is a pointer to a DROP_PARAM data object
	};

	// Optional Draw Param Mask used by ISonicImage::Draw
	enum enDrawParam
	{
		DP_NONE				= 0x0,	// none
		DP_ALPHA			= 0x1,	// total transparency
		DP_SRC_CLIP			= 0x2,	// source clip
		DP_SCALE			= 0x4,	// scale, when combined with DP_TILE, fScaleX and fScaleY will be ignored, but the middle part of image will be stretched instead of being tiled vertically
		DP_DEST_LIMIT		= 0x8,	// auto scale to fit the destination region, cannot be combined with DP_TILE
		DP_COLOR_KEY		= 0x10,	// color key, cannot be combined with DP_ALPHA
		DP_TILE				= 0x20,	// tile draw. cx and cy indicate the tile region, if set to zero, the original width and height will be used
		DP_NO_PIXELALPHA	= 0x40,	// disable alpha rendering for images with source alpha channel, such as png
	};

	// work with DROP_PARAM to inquire where I should put the dragging item
	enum enDropPostion
	{
		DROP_POS_NONE		= 0,	// not able to be dropped
		DROP_POS_BEFORE,			// drop before the hot item
		DROP_POS_AFTER,				// drop after the hot item
		DROP_POS_WITHIN,			// drop within the hot item, makes it as a parent item
	};

	// Components type
	enum enBaseType
	{
		BASE_TYPE_NONE = 0x1457,
		BASE_TYPE_IMAGE,			// ISonicImage
		BASE_TYPE_WND_EFFECT,		// ISonicWndEffect
		BASE_TYPE_STRING,			// ISonicString
		BASE_TYPE_TEXT_SCROLL_BAR,	// ISonicTextScrollBar
		BASE_TYPE_PAINT,			// ISonicPaint
		BASE_TYPE_ANIMATION,		// ISonicAnimation
		BASE_TYPE_SKIN,				// ISonicSkin
		BASE_TYPE_PROGRESSBAR,		// ISonicProgressBar
		BASE_TYPE_TAB,				// ISonicTab
		BASE_TYPE_MENUBAR,			// ISonicMenuBar
		BASE_TYPE_PARTICLE_EFFECT,	// ISonicParticleEffect
		BASE_TYPE_SCROLL_VIEW,		// ISonicScrollView
		BASE_TYPE_ICON_PANEL,		// ISonicIconPanel
		BASE_TYPE_TREE_VIEW,		// ISonicTreeView
		BASE_TYPE_END,
	};

	// attributes used by ISonicUI::HandleRawString 
	enum enRawStringChangeType
	{
		RSCT_DOUBLE = 0x1,		// convert all "/" to "//" in a raw string to avoid keyword confusion
		RSCT_REVERSE = 0x2,		// convert all "/" to "\\" in a raw string to avoid keyword confusion
		RSCT_URL = 0x4,			// auto identify and add URL mark in a raw string
	};

	// Image types that ISonicImage can support
	enum enImageType
	{
		IMAGE_TYPE_BMP = 0,
		IMAGE_TYPE_JPEG,
		IMAGE_TYPE_PNG,
		IMAGE_TYPE_TGA,
		IMAGE_TYPE_ICO,
	};

	// Optional attributes used by ISonicTextScrollBar
	enum enTextScrollBarOpt
	{
		TSBO_SINGLE_NO_SCROLL = 0x1,		// the scroll bar will not scroll text if there is only one item in it
		TSBO_LEFT_ALIGN	= 0x2,				// scroll alignment
		TSBO_FIRST_NO_SCROLL = 0x4,			// don't scroll the first item
	};

	// windows effects used by ISonicWndEffect
	enum enWndEffectMask
	{
		WND_EFFECT_NORMAL = 0,
		WND_EFFECT_WHOLE_DRAG = 0x4,	// full window drag
		WND_EFFECT_TRANSFORM = 0x20,	// transform:rotate and scale
	};

	// animations status
	enum enAnimationStatus
	{
		ANI_STATUS_NONE = 0,
		ANI_STATUS_MOVE_GENTLY,		// moving
		ANI_STATUS_SHOW_GENTYLE,	// fading out
		ANI_STATUS_TRANSFROM,		// rotating or scaling
		ANI_STATUS_SHUTTER,			// shuttering
		ANI_STATUS_SLIDE,			// slide show
	};

	// particle effect type
	enum enParticleEffectType
	{
		PETYPE_NONE = 0,
		PETYPE_WATERWAVE				// water wave,
	};

	// animation type when scrolling an ISonicScrollView
	enum enScrollAniType
	{
		SATYPE_NONE = 0,
		SATYPE_CONSTANT_SPEED,
		SATYPE_ALTERABLE_SPEED,			
	};

	// the constants below are used in ISonicBaseContainer::AddObject only, to make alignment for the new object being added
	enum enAddObjectAlignment
	{
		AOA_CENTER = -10000008,			// set this value to x to center horizontally, and to y to center vertically
		AOA_RIGHT,
		AOA_BOTTOM = AOA_RIGHT,
	};

	enum enIconPanelStyle
	{
		IPS_TABLIKE = 0x1,
		IPS_HASPAGE = 0x2,
	};
	
	// Optional Draw Param Mask used by ISonicImage::Draw
	typedef struct tagDrawParam
	{
		DWORD dwMask;			// flag mask, indicates which member is used
		float fScaleX;			// width scale rate
		float fScaleY;			// height scale rate	
		int cx;					// destination region width, go width DP_DEST_LIMIT or DP_TILE
		int cy;					// destination region height, go width DP_DEST_LIMIT or DP_TILE
		short cxDiv;			// divide the original width into n blocks and take the width of one block as edge width
		short cyDiv;			// divide the original height into n blocks and take the height of one block as edge height
		unsigned char cAlpha;	// transparent rate
		RECT rtSrc;				// source region clip
		DWORD dwColorKey;		// color key
	}DRAW_PARAM;

	// can be used in procedure delegated to DELEGATE_EVENT_PAINT of ISonicPaint
	typedef struct tagCurrentPaint
	{
		HDC hdc;				// current hdc of paint, a memory dc or a virtual one
		int left;				// if the paint is a virtual paint this value is the left coordinate of the paint in the window, if the paint has a memory dc, it is 0
		int top;				// if the paint is a virtual paint this value is the top coordinate of the paint in the window, if the paint has a memory dc, it is 0
		RECT rtMem;				// rtMem is used for calculating the right offset in the window, to assure objects could respond mouse messages correctly
	}CURRENT_PAINT;

	class ISonicBase;
	class ISonicBaseWnd;
	class ISonicBaseContainer;
	class ISonicPaint;
	// Paint Items
	typedef struct tagPaintItem
	{
		ISonicBase * pBase;		// a Item
		int x;					// x coordinate of the item
		int y;					// y coordinate of the item
		BYTE bVisible;			// visibility of the item
		BYTE bDestroyedOnClear;	// item object will be destroyed when containers clear themselves if bDestroyedOnClear set to true
	}PAINT_ITEM;

	// this data object is used when a drop operation occurred in a container, currently supporting ISonicTreeView only.
	// it accompanies with DELEGATE_EVENT_DROP
	enum enDropType
	{
		DROP_TYPE_NONE = 0,
		DROP_TYPE_SELF,
		DROP_TYPE_OTHERS,
		DROP_TYPE_FILE,
	};
	typedef struct tagDropParam
	{
		UINT uDropPos;					// tell the container if the drop operation is allowed
		UINT uDropType;					// drop type, identify what the dragging source is, from itself, or from file system, or from other compatible containers
		ISonicBaseWnd * pHotItem;		// the item that is going to be the parent of the dragging item, if any
		union
		{
			ISonicBaseWnd * pDraggingItem;	// the dragging item when drop type is DROP_TYPE_SELF or DROP_TYPE_OTHERS, ignore this param when dragging files
			TCHAR * szFilePath;			// file path with a terminating null character, only worked with DROP_TYPE_FILE, currently doesn't support multiple files drag&drop
		};
	}DROP_PARAM;

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicBase
	// intro:		base class type contain common attributes and methods
	//////////////////////////////////////////////////////////////////////////
	class ISonicBase
	{
	public:
		// virtual destructor
		virtual ~ISonicBase(){}

		// get the base object id
		virtual DWORD GetObjectId() = 0;

		// get the validity of current object
		virtual BOOL IsValid() = 0;

		// clear this object
		virtual void Clear() = 0;

		// get base type
		virtual DWORD GetType() = 0;

		// set a new name for this object, the new name must be unique and cannot conflict with name of others
		virtual BOOL SetName(LPCTSTR szName) = 0;

		// get the name of this object
		virtual LPCTSTR GetName() = 0;

		// delegate various procedure to events
		// 'message' stands for enDelegateEvent
		// 'pReserve' stands for a delegated data used by callback function
		// 'pClass' is used only when the delegated procedure is a member function of a class
		// the last param is the delegated procedure. here use the volatile parameter trick to avoid C++ compliling check when the proc is a member function
		//////////////////////////////////////////////////////////////////////////
		// notice: delegated proc can be only declared as following styles, either is ok
		// global function: void WINAPI Func(ISonicBase *, LPVOID pReserve);
		// member function: void Cxxx::Func(ISonicBase *, LPVOID pReserve);
		//////////////////////////////////////////////////////////////////////////
		// the ISonicBase * is a pointer to the object which was delegated previously
		virtual void Delegate(UINT message, LPVOID pParam, LPVOID pClass, ...) = 0;

		// set int attribute, never use names with "Sonic" prefix, such as "SonicValue", which are preserved for UI engine.
		virtual void SetAttr(LPCTSTR szName, int nValue) = 0;

		// set void * attribute, never use names with "Sonic" prefix, such as "SonicValue", which are preserved for UI engine.
		virtual void SetAttr(LPCTSTR szName, LPVOID pData) = 0;

		// remove specified attribute
		virtual BOOL RemoveAttr(LPCTSTR szName) = 0;

		// get int attribute, return FALSE if the attribute not exists
		virtual BOOL GetAttr(LPCTSTR szName, int & nValue, int nDefault = 0) = 0;

		// get void * attribute, return null if not exists
		virtual LPVOID GetAttr(LPCTSTR szName) = 0;

		// get width
		virtual int GetWidth() = 0;

		// get height
		virtual int GetHeight() = 0;

		// get handle of window which was attached by this object, no sense for ISonicImage
		virtual HWND GetSafeHwnd() = 0;

		friend class ISonicUI;
	protected:
		DWORD m_dwType;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicImage
	// intro:		image operation interface.Loading and saving procedure depends on CxImage lib internally. Supports bmp, jpeg, png, tga, gif, ico.
	//				Supports rotation, scale and hsl adjustment, etc.
	// remarks:		ISonicImage does not response to windows message so it cannot be delegated.
	//////////////////////////////////////////////////////////////////////////
	class ISonicImage : public ISonicBase
	{
	public:
		enum { BT = BASE_TYPE_IMAGE };
	public:
		// load image from memory view of an image file
		virtual BOOL Load(LPVOID pViewOfFile, DWORD dwFileSize) = 0;

		// load from specified path in disk
		// if this function is called between the ISonicUI::BeginImageFromZip and ISonicUI::EndImageFromZip pairs, 
		// lpszFileName will be considered as a relative path in the zip file instead of a real path in disk
		virtual BOOL Load(LPCTSTR lpszFileName) = 0;

		// load from resource
		// nResId:			resource id
		// hResModule:		module contain resource
		// szResourceType:	resource type name, default as "IMAGE"
		virtual BOOL Load(UINT nResId, HMODULE hResModule = NULL, LPCTSTR szResourceType = NULL) = 0;

		// load from hdc
		// bAlphaChannel:	image will add alpha channel if this param is set 
		virtual BOOL Load(HDC hdc, int x, int y, int cx, int cy) = 0;

		// load from a hbitmap, make a copy of it
		// bAlphaChannel:	image will add alpha channel if this param is set
		virtual BOOL Load(HBITMAP hBmp) = 0;

		// create a image with specified attributes
		virtual BOOL Create(int cx, int cy, DWORD dwColor = 0, BOOL bWithAlpha = FALSE) = 0;

		// fill the image with specified color 
		virtual BOOL Flush(DWORD dwColor) = 0;

		// fill the alpha channel with 0
		virtual BOOL DestroyAlphaChannel() = 0;

		// create alpha channel for non-alpha buffer, initial alpha is 0xff
		virtual BOOL CreateAlphaChannel() = 0;

		// draw to hdc with optional params
		virtual BOOL Draw(HDC hdc, int x = 0, int y = 0, DRAW_PARAM * pParam = NULL) = 0;

		// draw without any effects, even the alpha channel
		virtual BOOL SetBitsToDevice(HDC hdc, int x = 0, int y = 0) = 0;

		// get the frame count of this image
		virtual UINT GetFrameCount() = 0;

		// switch to specified frame, nIndex = -1 stands for next frame.
		virtual void SetCurrentFrame(int nIndex = -1) = 0;

		// get current frame index
		virtual UINT GetCurrentFrame() = 0;

		// get delay of specified frame, -1 stands for current
		virtual UINT GetFrameDelay(int nIndex = -1) = 0;

		// get width
		virtual int GetWidth() = 0;

		// get height
		virtual int GetHeight() = 0;

		// copy from
		virtual BOOL CloneFrom(const ISonicImage *) = 0;

		// rotate
		// pScout is a scout point, set a scout point in and the out value is the converted coordinates after rotation
		virtual BOOL Rotate(int nAngle, POINT * pScout = NULL) = 0;

		// grayness
		virtual BOOL Gray() = 0;

		// save as file, imgType is between 0 and 100, only available when imgType is IMAGE_TYPE_JPEG
		virtual BOOL SaveAsFile(LPCTSTR lpszFileName, enImageType imgType, int nQuality = 0) = 0;

		// make a rgn with specified color key. remember to destroy the rgn object when not used
		// x, y presents the destination left and top coordinates in CreateRectRgn
		// pRtSrc: uses the source clip to make rgn
		// bReverse: if set to TRUE, rgn equals to color key region, otherwise color key as tranparent region
		virtual HRGN CreateRgn(DWORD dwColorKey = DEFAULT_COLOR_KEY, int x = 0, int y = 0, RECT * pRtSrc = NULL, BOOL bReverse = FALSE) = 0;

		// if alpha channel is valid
		virtual BOOL IsAlphaChannelValid() = 0;

		// get the ARGB of specified coordinates
		virtual DWORD GetPixel(int x, int y) = 0;

		// set the ARGB of specified coordinates
		virtual BOOL SetPixel(int x, int y, DWORD dwColor) = 0;

		// get data buffer
		virtual BYTE * GetBits() = 0;

		// get size of data buffer
		virtual int GetSize() = 0;

		// calculate the render rect by draw param
		// pSrc is a optional param to get the new source region
		virtual RECT CalculateRectByDrawParam(int x, int y, const DRAW_PARAM * pDp, RECT * pSrc = NULL) = 0;

		// set color key. the whole image will own alpha channel after this operation
		virtual BOOL SetColorKey(DWORD dwColor) = 0;

		// set hue, saturation, and luminance. -1 stands for unchanged.
		// h [0, 360], s [0, 255], l [0, 255]
		virtual BOOL SetHsl(int h = -1, int s = -1, int l = -1) = 0;

		// mirror by x
		virtual BOOL MirrorX() = 0;

		// mirror by y
		virtual BOOL MirrorY() = 0;

		// stretch
		virtual BOOL Stretch(int cx, int cy) = 0;

		// just extend without stretch
		virtual BOOL Extend(int cx = 0, int cy = 0) = 0;

		// get internal dc of image directly
		virtual HDC GetDC() = 0;

		// get internal HBITMAP of image
		virtual HBITMAP GetHBitmap() = 0;

		// box blur
		// nRadius must be a positive value, the bigger, the slower
		virtual BOOL BoxBlur(int nRadius) = 0;

		// set tile status. tile status is used internally in ISonicString during the Format procedure
		// ISonicString will check this value for automatic source image clip.a positive nTileStatus means the image is horizontally tiled, 
		// a negative nTileStatus means it's vertically tiled. zero means it's a normal image.
		// the two usages following are actually the same
		// Example1:	pSonicImage->SetTileStatus(-4);
		//				pSonicString->Format("/a, p=%d/", pSonicImage->GetObjectId());
		// Example2:	// nTileStatus of pSonicImage is zero
		//				pSonicString->Format("/a, p4=%d, ptile=1/", pSonicImage->GetObjectId());
		virtual void SetTileStatus(int nTileStatus) = 0;

		// get tile status
		virtual int GetTileStatus() = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicBaseWnd
	// intro:		objects inherit from this interface act like windows, they can be showed or hided,
	//				work on client region of a real window and obtain a fixed region. most objects inherit
	//				from this interface could respond windows message except ISonicPaint.
	//////////////////////////////////////////////////////////////////////////
	// preposition declaration
	class ISonicBaseWnd : public ISonicBase
	{
	public:
		// move the object to new position, set cx or cy to zero to preserve the original width and height
		// no sense for ISonicString
		virtual void Move(int x, int y, int cx = 0, int cy = 0, BOOL bRedraw = FALSE) = 0;

		// manually render this object during customer painting procedure.
		// no sense for ISonicString
		// note: except ISonicPaint and ISonicString, all other objects inherit from ISonicBaseWnd will be automatically rendered at the end of 
		// WM_PAINT internally. but you can also manually call this render method in your own painting procedure, by doing which, the automatic render
		// will no be triggered.
		virtual BOOL Render(HDC hdc, HWND hWnd = NULL, const RECT * pMemDCRect = NULL) = 0;

		// show or hide the object
		virtual BOOL Show(BOOL b, BOOL bRedraw = TRUE) = 0;

		// test if it's visible
		// when an ISonicBaseWnd is a child  item of other sonic objects, its visibility relies on its parent. so a child item is actually not visible
		// when its parent hides, even when its own visibility value is TRUE
		// set bTestSelfOnly to TRUE to inquiry its own visibility value, default as true, to inquiry its actual visibility
		virtual BOOL IsVisible(BOOL bTestSelfOnly = FALSE) = 0;

		// enable or disable the object
		virtual BOOL Enable(BOOL b, BOOL bRedraw = TRUE) = 0;

		// test if it's enabled
		virtual BOOL IsEnabled() = 0;

		// get current region, if this object is manually rendered on a memory dc, this rect is in the relative coordinates system of the memory dc
		virtual const RECT * GetRect() = 0;

		// get message responding region of this object on the host window. if rendered on a memory dc, this region is internally fixed and clipped by 
		// the argument "pMemDCRect" passed in when rendering.
		virtual const RECT * GetMsgRect() = 0;

		// redraw itself
		virtual BOOL Redraw(BOOL bEraseBkground = TRUE) = 0;

		// set tooltip for this object
		virtual BOOL SetTooltip(LPCTSTR szTipText) = 0;

		// get tooltip text of this object
		virtual LPCTSTR GetTooltip() = 0;

		// set the background color, note that if this value is set, ISonicBaseWnd will not let mouse messages pass through, except ISonicPaint
		virtual BOOL SetBkColor(DWORD dwColor) = 0;

		// clear the background color to make the object transparent
		virtual BOOL RemoveBkColor() = 0;

		// set custom timer, only the following eight customer ID are allowed
		// 0x1000,0x2000,0x4000,0x8000,0x10000,0x20000,0x40000,0x80000
		// uElapse must be a multiple of 10
		virtual void SetTimer(DWORD dwTimerID, UINT uElapse, BOOL bOnce = FALSE) = 0;

		// kill custom timer
		virtual void KillTimer(DWORD dwTimerID) = 0;

		// set a background image, note that if this value is set, ISonicBaseWnd will not let mouse messages pass through, except ISonicPaint
		virtual BOOL SetBkImage(ISonicImage * pImgBg) = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicBaseContainer
	// intro:		common container interfaces. a container can hold plenty of object as items
	//////////////////////////////////////////////////////////////////////////
	class ISonicBaseContainer : public ISonicBaseWnd
	{
	public:
		// judge whether the item is containable in this container
		virtual BOOL IsItemContainable(ISonicBase * pBase) = 0;

		// add an object to a specified position and index, return value is the index of the new object, return -1 if failed
		// if nIndex is -1, the object is add to the tail of the item list, otherwise insert to the order of nIndex
		virtual int AddObject(DWORD dwObjectId, int x = 0, int y = 0, BOOL bDestroyedOnClear = FALSE, int nIndex = -1) = 0;

		// delete objects
		virtual BOOL DelObject(DWORD dwObjectId, BOOL bByIndex = FALSE) = 0;

		// change the render order of an object
		virtual BOOL Reorder(int nOldIndex, int nNewIndex) = 0;

		// delete all objects
		virtual BOOL DelAllObject() = 0;

		// get the count of current objects
		virtual int GetCount() = 0;

		// get specified PAINT_ITEM of a object by object id or index
		virtual PAINT_ITEM * GetPaintItem(DWORD dwObjectId, BOOL bByIndex = FALSE) = 0;

		// show or hide a specified object by object id or index
		virtual BOOL ShowObject(BOOL b, BOOL bRedraw, DWORD dwObjectId, BOOL bByIndex = FALSE) = 0;

		// enumerate the children to do something, the callback function should be specified by delegation
		virtual BOOL Enumerate() = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicPaint
	// intro:		paint can hold some objects. can accomplish dirty update mechanism easily
	//////////////////////////////////////////////////////////////////////////
	class ISonicPaint : public ISonicBaseContainer
	{
	public:
		enum { BT = BASE_TYPE_PAINT };
	public:
		// create
		// hMemDib:	if the paint obtain its own mem dc
		// cx, cy:	set to 0 to make a auto adjustment by objects added
		virtual BOOL Create(BOOL bMemDib, int cx = 0, int cy = 0) = 0;

		// resize
		// cx, cy:	set to 0 to make a auto adjustment by objects added
		virtual BOOL Resize(int cx = 0, int cy = 0) = 0;

		// get mem dc
		virtual HDC GetDC() = 0;

		// get mem data buffer
		virtual BYTE * GetBits() = 0;

		// get size of data buffer, width * height * 4
		virtual int GetSize() = 0;

		// get HBitmap of memdc
		virtual HBITMAP GetHBitmap() = 0;

		// return current paint params
		virtual CURRENT_PAINT * GetCurrentPaint() = 0;

		// return rect in window which responses to message
		virtual const RECT * GetMsgRect() = 0;

		// copy from
		virtual BOOL CloneFrom(ISonicPaint *) = 0;

		// turn on/off auto background backup mechanism when drawing
		virtual BOOL EnableDrawingBackup(BOOL b) = 0;

		// fill the paint with specified color 
		virtual BOOL Flush(DWORD dwColor) = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicAnimation
	// intro:		animation operation methods, such as rotation, stretch, tec
	//////////////////////////////////////////////////////////////////////////
	class ISonicAnimation : public ISonicBaseContainer
	{
	public:
		enum { BT = BASE_TYPE_ANIMATION };
	public:
		// create an animation with specified region
		virtual BOOL Create(HWND hWnd, int x, int y, int cx, int cy) = 0;

		// move animation
		// nFrame: animation frames
		// bEvenSpeed: movement will be at an even speed if this is set to TRUE, otherwise will slow down smoothly
		virtual BOOL MoveGently(int x, int y, int nFrame, BOOL bEvenSpeed = FALSE) = 0;

		// if the animation is doing a moving animation, get the destination postion
		virtual BOOL GetMovingDestination(LPPOINT ppt) = 0;

		// fading out animation
		// nFrame: animation frames
		virtual BOOL ShowGently(BOOL bShow, int nFrame) = 0;

		// transform animation
		virtual BOOL Transform(BOOL b, int nAngle = 0, RECT * pRect = NULL, int nFrame = 0) = 0;

		// get current animation Status with enum enAnimationStatus
		virtual DWORD GetAnimationStatus() = 0;

		// force ISonicAnimation to finish its animation and move to the final position immediately
		virtual BOOL EndAni() = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicString
	// intro:		format a string with key words, make the string act as a URL, a button, an image with message response ability, just like what the HTML does
	// keywords
	//			a		usage:	make a link
	//					exp:	/a=www.xxx.com/hello
	//
	//			c		usage:	set font color
	//					exp:	/c=0xffffff/hello
	//
	//			linkl	usage:	only available with 'a' keyword. 0 no underline under a url; 1 uderline when hover; 2 underline always
	//					exp:	/a=www.xxx.com, linkl=1/hello
	//
	//			linkc	usage:	only available with 'a' keyword. 0 cursor will not change when hover; 1 cursor will change to "HAND" style when hover
	//					exp:	/a=www.xxx.com, linkc=0/hello
	//
	//			linkh	usage:	only available with 'a' keyword. set color when hover on a url
	//					exp:	/a=www.xxx.com, linkh=0xff00ff/hello
	//
	//		btn_type	usage:	only available with 'a' keyword. 0 normal button(acts like system buttons), 1 check box(toggle between 2 status, 
	//							like the system check box), 2 quick button(no capture, mouse events will be fired when handling WM_XBUTTONDOWN, instead of WM_XBUTTONUP)
	//
	//		pass_msg	usage:	mouse message will pass through the object to the window beneath
	//					exp:	/a, pass_msg/你好
	//
	//			p		usage:	display a picture, auto switch frame if a gif is set. set a ISonicImage object id to it 
	//					exp:	Format("/p=%d//c=0xff/你好啊", pImg->GetObjectId());
	//
	//		btn_text	usage:	only available with 'a', 'p', and 'p%d' keywords. output text on the button, if the length of text exceeds the suitable length of the button,
	//							auto button length extention would be made internally. ISonicString syntax can be used within the single quotes.
	//					exp:	/a, p3=%d, btn_text='/c=%x/确定'/pButton->GetObjectId(), RGB(255, 0, 255)
	//
	//		btn_width	usage:	only available with 'a', 'p', and 'p%d' keywords. force button width to the specified value.
	//					exp:	/a, p3=%d, btn_width=40/pButton->GetObjectId()
	//			
	//		btn_height	usage:	only available with 'a', 'p', and 'p%d' keywords. force button height to the specified value.
	//					exp:	/a, p3=%d, btn_height=30/pButton->GetObjectId()
	//
	//		animation	usage:	only available with 'a', 'p', 'ph' and 'pc' keywords. make a animation button with specified speed
	//					exp:	Format("/a, p=%d, ph=%d, pc=%d, animation=30/", pNormal->GetObjectId(), pHover->GetObjectId(), pClick->GetObjectId());
	//
	//			key		usage:	only available with 'p' keyword. set color key of a image
	//					exp:	Format("/key=0x808080, p=%d/", pImg->GetObjectId());
	//
	//			pclip	usage:	only available with 'p' keyword. source image clip, the arguments order is: left|top|right|bottom
	//					exp:	Format("/p=%d, pclip=0|0|15|15/", pImg->GetObjectId());
	//	
	//		pstretch	usage:	only available with 'p' keyword. stretch the source image to fit specified width and height, the arguments order is: width|height
	//					exp:	Format("/p=%d, pstretch=100|80/", pImg->GetObjectId());
	//
	//		palpha		usage:	only avaible with 'p' keyword. set an total alpha value when drawing this image. a valid alpha value should between 0 and 255
	//					exp:	Format("/p=%d, palpha=120/", pImg->GetObjectId());
	//
	//			def		usage:	use a default font color and style
	//					exp:	/def/你好
	//
	//		line_width	usage:	set line width, text will make a newline while exceeding the width
	//					exp:	/line_width=50//a=www.xxx.com/你好吗，你好吗
	//
	//		line_space	usage:	space between lines
	//					exp:	/line_space=5/你好吗
	//
	//		single_line	usage:	only available with 'line_width' keyword. force a single line format. 0 disable single line. 
	//							1 clip when exceeding; 2 clip when exceeding, and add "..." to make an end ellipsis, and display whole info when hover
	//					exp:	/line_width=30, single_line=2//a=www.xxx.com/我们都有一个家，名字叫中国
	//	height_limit	usage:	only available with 'line_width' keyword. force the string to be formated with the specified height. 
	//							0 indicates no limit; a positive value indicates the text block will be truncated when when exceeding the specified height,
	//							and a "..." will be added as ending. while being hovered, a tooltip with whole text will pop up.
	//							a negative value indicates the text block will be truncated when exceeding without further process
	//					exp:	/line_width=300, height_limit=60/hello my friends, take your time and have a rest
	//
	//		word_break	usage:	specify if the word should be broken when encountering a new line, default set to 1
	//					exp:	/line_width=30, word_break=0/hello, buddy, how are you
	//
	//			align	usage:	line aligning mode. 0x0 for top alignment, 0x1 for vertical middle , 0x2 for bottom, 0x10 for horizontal middle, 0x20 for horizontal right
	//					exp:	Format("/align=2/你好吗/p=%d/", pImg->GetObjectId());
	//
	//			br		usage:	make a newline, same with "\n". because \n is an escape character only in VC, so if you generate a SonicString 
	//							from a string defined in a config file, \n will be considered as normal ASCII characters, in which case you should 
	//							use /br/ to make a newline. in other words, \n should only be used in VC.
	//					exp:	Format("123/br/hello");
	//
	//		br_offset	usage:	offset of a newline beginning
	//					exp:	/br_offset=30/你好\n你好
	//
	//			font	usage:	set font style
	//		font_height	usage:	only available with 'font' keyword. change font size
	//		font_bold	usage:	only available with 'font' keyword. set bold
	//		font_italic	usage:	only available with 'font' keyword. set italic
	//	font_strikeout	usage:	only available with 'font' keyword. set strikeout
	//		font_face	usage:	only available with 'font' keyword. set font face.
	//					exp:	/font, font_height=16, font_italic=1, font_face=隶书/你好/def/你好/a=www.xxx.com/你好
	//
	//			sparkle	usage:	sparkle effect
	//	sparkle_color	usage:	only available with 'sparkle' keyword. sparkle color. maximum of 4 colors
	// sparkle_interval	usage:	only available with 'sparkle' keyword. sparkle color. sparkle interval, default as 500 ms
	//	sparkle_timeout	usage:	only available with 'sparkle' keyword. set sparkle timeout.
	//					exp:	/sparkle, sparkle_color=0xff|0xff00|0xff0000, sparkle_timeout=40/你好吗
	//
	//			fadeout	usage:	fadeout effect
	//		fadeout_max	usage:	only available with 'fadeout' keyword. fadeout max value
	//		fadeout_min	usage:	only available with 'fadeout' keyword. fadeout min value
	//	fadeout_speed	usage:	only available with 'fadeout' keyword. fadeout speed
	//					exp:	/fadeout, fadeout_speed=10/你好吗
	//
	//			global	usage:	global attributes. when '/def/' is used, global attributes will be adopted
	//					exp:	/global, c=0xff, font, font_height=15/你好
	//
	//			ani_num	usage:	make a animation number effect
	//	ani_num_from	usage:	only available with 'ani_num' keyword. number from
	//		ani_num_to	usage:	only available with 'ani_num' keyword. number to
	//	ani_num_frame	usage:	only available with 'ani_num' keyword. animation frames
	//					exp:	/ani_num, ani_num_from=0, ani_num_to=3000, ani_num_frame=30/
	//
	//		init_normal	usage:	don't act special effect until calling AnimateXXX() manually
	//					exp:	/fadeout, init_normal/hello world!
	//
	//	frame_interval	usage:	only available with 'p' keyword. used for making animation. the source image must have more than one tiling frame.
	//					exp:	Format("/p=%d, frame_interval=20/", pImg->GetObjectId());
	//////////////////////////////////////////////////////////////////////////
	class ISonicString : public ISonicBaseWnd
	{
	public:
		enum { BT = BASE_TYPE_STRING };
	public:		
		// format like the CString, the length of this string cannot exceed 1024
		virtual BOOL Format(LPCTSTR lpszArg, ...) = 0;

		// render the text, please remember to specify a hWnd if the hdc is a memory one
		virtual BOOL TextOut(HDC hdc, int x, int y, HWND hWnd = NULL, const RECT * pMemDCRect = NULL) = 0;

		// turn on/off gif animation
		virtual BOOL AnimateGif(BOOL bBegin = TRUE) = 0;

		// turn on/off sparkle
		virtual BOOL AnimateSparkle(BOOL b = TRUE) = 0;

		// get raw string
		virtual LPCTSTR GetStr() = 0;

		// get text without control characters
		virtual LPCTSTR GetTextWithoutCtrl() = 0;

		// turn on/off fadeout animation
		virtual BOOL AnimateFadeout(BOOL b = TRUE) = 0;

		// turn on/off number animation
		virtual BOOL AnimateAniNum(BOOL b = TRUE) = 0;

		// force a text color
		virtual BOOL ForceColor(BOOL bForce, DWORD dwColor = 0, BOOL bRedraw = TRUE) = 0;

		// test if is forcing color
		virtual BOOL IsForcingColor() = 0;

		// test if is pushed, 2-state button only, such as check box
		virtual BOOL IsPushed() = 0;

		// set the button to pushed or normal state, mainly for check box
		virtual BOOL Push(BOOL b) = 0;

		// add new attribute or modify existing attributes, cannot remove any attributes
		// only the first section of attributes can be modified, the length of this string cannot exceed 1024
		virtual BOOL ModifyAttributes(LPCTSTR lpszArg, ...) = 0;

		// test if this is a check box
		virtual BOOL IsCheckBox() = 0;

		// change the string to a highlight status just like when it's being hovered 
		virtual BOOL Highlight(BOOL b, BOOL bRedraw = TRUE) = 0;

		// get the internal button text if exists
		virtual ISonicString * GetBtnText() = 0;

	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicTextScrollBar
	// intro:		Text strcoll bar control, used for scroll text or images
	// remarks:		text strcoll bar is not a real window
	//////////////////////////////////////////////////////////////////////////
	class ISonicTextScrollBar : public ISonicBaseWnd
	{
	public:
		enum { BT = BASE_TYPE_TEXT_SCROLL_BAR };
	public:
		// create a text scroll bar
		// nSpeed:		scroll speed, intervals between every scroll frame
		// nMidPause:	pause a specified time if the text is in center of the bar
		virtual BOOL Create(HWND hWnd, int x, int y, int cx, int cy, BOOL bVerScroll = TRUE, int nSpeed = 50, int nMidPause = 0) = 0;

		// add an object automatically destroyed on clear
		virtual BOOL AddText(DWORD dwObjectId, BOOL bDestroyedOnClear = FALSE) = 0;

		// add raw text to text scroll bar, ISonicString object will be generated internally and be destroyed on clear
		virtual BOOL AddText(LPCTSTR lpszText) = 0;

		// delete an item with specified index
		virtual BOOL DelText(int nIndex) = 0;

		// turn on/off scroll 
        virtual BOOL DoScroll(BOOL bScroll = TRUE) = 0;

		// enable extra option
		virtual BOOL EnableOpt(DWORD dwOpt) = 0;

		// get total count of items
		virtual int GetCount() = 0;
	};

	// this interface is merely used for off-screen rendering. the off-screen rendering system will call these interfaces to notify the 
	// host window to change its UI appearance
	class IOffscreenUICallBack
	{
	public:
		// return the window handle which will be the render target
		virtual void osGetRenderTarget(HWND hOSWindow, LPRECT pRectScreen) = 0;

		// notify the host to change its cursor shape, the nCursorResID is system defined
		virtual void osChangeCursor(int nCursorResID) = 0;

		// notify the host the canvas should be repaint with a specified dirty rectangle
		// if bNullImage is true, the off-screen paint is now completely transparent,
		// which means all off-screen windows are invisible
		virtual void osDraw(ISonicPaint * pPaint, const RECT * pRectDirty, BOOL bNullImage = FALSE) = 0;

		// notify the host to set or release capture(releasing if hOSWindow is null)
		virtual void osChangeCapture(HWND hOSWindow) = 0;

		// notify the host some off-screen is destroying
		virtual void osDestroy(HWND hOSWindow) = 0;

		// SonicUI will call this function to get the off-screen panel, that is considered as the bottom and main window
		virtual HWND osGetPanel() = 0;

		// notify the host focus is changing from hLosingFocus to hFocus
		virtual void osChangeFocus(HWND hFocus, HWND hLosingFocus) = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicSkin
	// intro:		subclass mechanism to change skin
	// remarks:		while formating lpszValue, use colon to set value and semicolon to separate different values
	//				it's highly recommended that attach ISonicSkin a window before it's visible for more smooth UI sense.
	//				it's free to attach ISonicSkin to normal windows with kinds of style, but when face to dialog, you should 
	//				modify dialog style in the template editor. style modification after dialogs are created will cause weird
	//				action, especially in modal dialogs. for example, if modify style to WS_MINIMIZE or WS_SIZEBOX within WM_INITDIALOG
	//				process, modification could be confirmed with spy++ but the new style will not function at all.
	//				so, if attach ISonicSkin to a dialog, you should prepare a none-border dialog with the template editor. if more optional
	//				styles wanted, you will have to edit the .rc file manually
	// note:		you will obtain automatic double buffer mechanism after attaching a ISonicSkin to a window if set bDoubleBuffer to TRUE while attaching.
	//				after that , you just normally call BeginPaint and EndPaint during your WM_PAINT responding procedure without worrying about the flicker.
	//				the WM_ERASEBKGND message will be filtered  by ISonicSkin so the application won't receive this message after attaching
	//////////////////////////////////////////////////////////////////////////
	class ISonicSkin : public ISonicBase
	{
	public:
		enum { BT = BASE_TYPE_SKIN };
	public:
		// make sure pImg passed in is valid while working time
		// skin type description:
		//////////////////////////////////////////////////////////////////////////
		// Skin Name: bg
		// Description: background render
		// Value:	image: specify background image
		//			shadow_border: if the background image has a shadow border, set this value to true.
		//			fit_image: if set to 1, the window is resized to fit the size of background image, and cannot change its size any more, default set zero
		//			bg_color: specify background color if no background image specified
		//			text_color: specify static text color
		//			title_height: specify the title area in the background image, if not specified, auto judged internally
		//			title_text:	specify the title text
		//			enable_drag: set to false to disable window drag, default to TRUE
		//			color_key: specify the transparent color to make window rgn	
		//			close: system close button
		//			min: system minimize button
		//			max: system maximize button
		//			restore: system restore button
		//			icon: system icon, image
		//			btn_space: width between system buttons
		//			btn_top_margin: top margin of system buttons
		//			btn_right_margin: right margin of system buttons
		// Skin Name: filter
		// Description: a visual filter
		// Value:	src: specify a filter source, if not set, a filter source is a pointer of ISonicSkin
		//			mask: an ISonicImage used as a filter mask
		//			type: 0 for no filter; 1 for transparent; 2 for blur
		//			blur_radius: only valid when using a blur filter, specify a radius for doing blur. the bigger the radius, the more blurry the filter is.
		//			blur_times:only valid when using a blur filter, specify how much times the filter will blur the source image.
		//////////////////////////////////////////////////////////////////////////
		// Exp:		SetSkin("bg", "image:%d;title_height:1;close:%d", pBG->GetObjectId(), pClose->GetObjectId());
		//////////////////////////////////////////////////////////////////////////
		virtual BOOL SetSkin(LPCTSTR lpszSkinName, LPCTSTR lpszValue, ...) = 0;

		// attach skin to a specified window, SetSkin cannot be called before attaching.
		// set bDoubleBuffer to TRUE to use a internal double buffer to avoid flicker.
		virtual BOOL Attach(HWND hWnd, BOOL bDoubleBuffer = TRUE) = 0;

		// get the internal paint used by ISonicSkin, you can add custom objects or delegate custom painting procedure freely.
		virtual ISonicPaint * GetPaint() = 0;

		// get skin value
		virtual DWORD GetSkin(LPCTSTR lpszSkinName, LPCTSTR lpszValue) = 0;

		// a panel window of an ISonicSkin may be used for two purpose. 
		// if the host of this skin was attached by an ISonicWndEffect with bAlphaPerPexel set to true,then the host will not show any of its 
		// child windows due to the limitation of layered windows. so this panel window will be created as a container for child windows and move along with the host.
		// if the background image specified has a shadow border, a panel window is used to display the shadow.  
		virtual HWND GetPanelWindow() = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicWndEffect
	// intro:		a effect engine for windows. you can make irregular windows, move or show or close the window with animation, or gray windows, 
	//				or make a snapshot of a window with this interface.
	//////////////////////////////////////////////////////////////////////////
	class ISonicWndEffect : public ISonicBase
	{
	public:
		enum { BT = BASE_TYPE_WND_EFFECT };
	public:
		// attach to a specified window, if already attached, will release the previous attaching relation.
		virtual BOOL Attach(HWND hWnd, BOOL bAlphaPerPixel = FALSE) = 0;

		// shading animation
		virtual BOOL ShowGently(BOOL bShow, BOOL bActivted = FALSE, int nMaxFrame = 10) = 0;

		// make a irregular window with a ISonicImage
		virtual BOOL SetShapeByImage(ISonicImage * pImg) = 0;

		// moving animation
		virtual BOOL MoveGently(int x, int y, int nMaxFrame = 10) = 0;

		// make a snapshot of host window
		virtual ISonicImage * Snapshot() = 0;

		// set color key or transparency
		virtual BOOL SetLayeredAttributes(COLORREF crKey, BYTE bAlpha, DWORD dwFlag) = 0;

		// enable whole window dragment
		virtual BOOL EnableWholeDrag(BOOL b) = 0;

		// direct transformation, if bFadingEffect is TRUE, a fadein or fadeout(depends on the current alpha of this window) effect will be added when doing the transformation
		virtual BOOL DirectTransform(int nAngle, RECT * pDest, int nMaxFrame = 10, BOOL bFadingEffect = FALSE) = 0;

		// sliding the window while showing or hiding
		// using the same control flags with AnimateWindow
		virtual BOOL SlideShow(DWORD dwFlags, int nMaxFrame = 10, BOOL bFadingEffect = FALSE) = 0;

		// ISonicWndEffect internally create a SonicSkin and attach it to the host window
		// call this method to retrieve the internal skin and operate it.
		virtual ISonicSkin * GetSkin() = 0;

		// return if the window is an alpha-per-pixel layered window
		virtual BOOL IsAlphaPerPixel() = 0;

		// return the current alpha value of this window
		virtual BYTE GetCurrentAlpha() = 0;

		// return the animation status if the window is doing an animation, lpRectSrc and lpRectDest are optional params, to retrieve the window area before and after
		// an animation, in screen coordinates
		virtual DWORD GetAnimationStatus(LPRECT lpRectSrc = NULL, LPRECT lpRectDest = NULL) = 0;

		// force ISonicWndEffect to finish its animation and move to the final position immediately(the delegated events will still be fired)
		virtual BOOL EndAni() = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicProgressBar
	// intro:		directUI progress bar control, shining effect and animation can be implemented in 
	//				conjunction with gif and png source images
	//////////////////////////////////////////////////////////////////////////
	class ISonicProgressBar : public ISonicBaseWnd
	{
	public:
		enum { BT = BASE_TYPE_PROGRESSBAR };
	public:
		// create progress bar on specified window, the height of this control is equivalent to the height of pBG, and cannot be modified
		virtual BOOL Create(HWND hWnd, int x, int y, int cx, ISonicImage * pBG, ISonicImage * pBar) = 0;

		// set range, default from 0 to 100
		virtual void SetRange(int nMin, int nMax) = 0;

		// retrieve range values
		virtual void GetRange(int & nMin, int & nMax) = 0;

		// retrieve max range
		virtual int GetMaxRange() = 0;

		// set position, within the range from nMin to nMax
		virtual void SetPos(int nPos) = 0;

		// offset from current position, nPlus can be negative
		virtual void OffsetPos(int nPlus) = 0;

		// return current position
		virtual int GetPos() = 0;

		// change the image of the progress bar
		virtual void SetBarImage(ISonicImage * pBar) = 0;

		// set frame interval to do a frame animation, if the bar image has multiple status
		virtual BOOL SetFrameInterval(int nInterval) = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	IDragDropCallback
	// intro:		a callback interface which could be assigned to ISonicIconPanel or ISonicTab and would be triggered
	//				when performing a drag-and-drop operation
	//////////////////////////////////////////////////////////////////////////
	class IDragDropCallback
	{
	public:
		// this callback is triggered when an item is about to be dragged, return FALSE to prevent this operation, or return TRUE to continue.
		// if pImgOutsideBoundary has no content after return, it means the items are not allowed to be dragged outside the boundary of its parent,
		// otherwise pImgOutsideBoundary will follow the cursor while an item is moving
		virtual BOOL OnBeginDrag(ISonicBaseWnd * pDragFrom, ISonicAnimation * pDraggingItem, ISonicImage * pImgOutsideBoundary){ return TRUE; }

		// this callback is triggered when an item is about to enter the area of an ISonicTab or ISonicIconPanel
		// return FALSE to prevent dragging the item into the target
		virtual BOOL OnTestDropTarget(ISonicBaseWnd * pDragFrom, ISonicBaseWnd * pDragTo, ISonicAnimation * pDraggingItem){ return TRUE; }

		// this callback is triggered when dropping an item, return FALSE to restore the item to its original position, or return TRUE to continue the drop
		// note that if the item is dragged outside the boundary and dropped to an invalid place, the pDragTo will be null on callback
		virtual BOOL OnDrop(ISonicBaseWnd * pDragFrom, ISonicBaseWnd * pDragTo, ISonicAnimation * pDraggingItem){ return TRUE; }
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicTab
	// intro:		directUI tab control
	//////////////////////////////////////////////////////////////////////////
	class ISonicTab : public ISonicBaseWnd
	{
	public:
		enum { BT = BASE_TYPE_TAB };
	public:
		// create tab control
		virtual BOOL Create(HWND hWnd, int x, int y, int cx, int cy, ISonicImage * pTabImage) = 0;

		// select a tab item by index
		virtual void SetSel(int nIndex) = 0;

		// get the current selected item
		virtual int GetSel() = 0;

		// get count of tab items 
		virtual int GetTabCount() = 0;

		// get rect without border and tab buttons
		virtual void GetClientRect(LPRECT pRect) = 0;

		// get hwnd of tab items
		virtual HWND GetTabWnd(ISonicAnimation * pItem) = 0;

		// get hwnd of tab items
		virtual HWND GetTabWnd(int nIndex) = 0;

		// get tab item text
		virtual LPCTSTR GetTabText(int nIndex, BOOL bRawText = FALSE) = 0;

		// get tab item text
		virtual LPCTSTR GetTabText(ISonicAnimation * pItem, BOOL bRawText = FALSE) = 0;

		// set tab item text, support SonicString syntax, without message responding keywords
		virtual void SetTabText(int nIndex, LPCTSTR lpszText) = 0;

		// add a tab item, if nIndex < 0, the new item will be added to the tail of ISonicTab
		virtual int AddTab(HWND hDlg, LPCTSTR lpszTabText, int nIndex = -1) = 0;

		// delete a tab item, if bDestroyWindow is TRUE, the child window attached to this tab item will be destroyed correspondingly, 
		// otherwise it will be hidden instead of being destroyed
		// nMaxFrame is the animation frame when deleting an item, set 0 to disable the animation
		virtual void DeleteTab(int nIndex, int nMaxFrame = 0, BOOL bDestroyWindow = TRUE) = 0;

		// get the tab item with specified index
		virtual ISonicAnimation * GetTabItem(int nIndex) = 0;

		// get the index of a tab item
		virtual int GetTabItemIndex(ISonicAnimation * pItem) = 0;

		// get the paint of ISonicTab
		virtual ISonicPaint * GetPaint() = 0;

		// set width for a specified tab item
		virtual BOOL SetTabWidth(int nIndex, int nWidth) = 0;

		// set right margin for tab panel, this value equals the width of whole ISonicTab subtracts the width of tab panel, 
		// and decides the max visible count of tab items
		virtual BOOL SetTabPanelHorMargin(int nLeftMargin, int nRightMargin) = 0;

		// set the horizontal space between every tab item
		virtual BOOL SetTabBtnSpace(int nSpace) = 0;

		// get the actual width for tab panel, that is a total sum of width of every visible tab item
		virtual int GetVisibleTabsTotalWidth() = 0;

		// if this function is called, a close button will be showed at the top-right corner of every tab item with the specified image.
		virtual BOOL SetCloseButton(ISonicImage * pCloseImg, int nTopMargin = 4, int nRightMargin = 4) = 0;

		// get count of visible tab items, this "visibility" only means there are too many tab items and the tab panel is
		// not big enough to show them all. the following functions with "Visible" tag have the same meaning
		virtual int GetVisibleTabCount() = 0;

		// get index of the first visible tab item
		virtual int GetFirstVisibleIndex() = 0;

		// set the specified tab item to visible
		virtual void SetTabVisible(int nIndex, int nMaxFrame = 0) = 0;

		// set a drag-and-drop callback interface to ISonicTab
		virtual BOOL SetDragDropCallback(IDragDropCallback * pDragDropCallback) = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicMenuBar
	// intro:		directUI menu bar
	//////////////////////////////////////////////////////////////////////////
	class  ISonicMenuBar : public ISonicBaseWnd
	{
	public:
		enum { BT = BASE_TYPE_MENUBAR };
	public:
		// create menu bar
		virtual BOOL Create(HWND hWnd, int x, int y, ISonicImage * pBG = NULL) = 0;

		// add an ISonicString item to the menu bar. if the hPopupMenu is not null, pItem is considered as a button, otherwise a dropdown menu will be displayed
		// when pItem is clicked. note the pItem will be auto destroyed when the menu bar is cleared.
		// set index to -1 to indicate the pItem should be added to the end of the menu item list
		// the return value is the actual index of pItem, and return -1 if failed
		virtual int AddItem(ISonicString * pItem, HMENU hPopupMenu = NULL, int nIndex = -1) = 0;

		// delete specified item with index
		virtual void DeleteItem(int nIndex) = 0;

		// return the item count of menu bar
		virtual int GetItemCount() = 0;

		// set the color of separators between items, note a separators will show itself only if this function is called at least once.
		// set dwColor to -1 to remove the existing separators
		virtual void SetSepColor(DWORD dwColor) = 0;

		// set the background highlight color, set dwColor to -1 to remove remove the background highlight color
		virtual void SetHilightBkColor(DWORD dwColor) = 0;

		// set the highlight text color, set dwColor to -1 to remove remove the highlight text color
		virtual void SetHilightTextColor(DWORD dwColor) = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicParticleEffect
	// intro:		directUI particle effect
	//////////////////////////////////////////////////////////////////////////
	class ISonicParticleEffect : public ISonicBaseContainer
	{
	public:
		enum { BT = BASE_TYPE_PARTICLE_EFFECT };
	public:
		// create a particle effect canvas
		virtual BOOL Create(HWND hWnd, int x, int y, int cx, int cy) = 0;

		// set specified particle effect, multiple effects cannot exist at the same time
		// set dwParticleEffectType to 0 to disable all effects
		virtual BOOL SetCurrentEffect(DWORD dwParticleEffectType) = 0;

		// get the current active particle effect type
		virtual DWORD GetCurrentEffectType() = 0;

		// to generate a water wave by code
		// this function must be called after SetCurrentEffect to water wave
		// x, y is a relative coordinate to this ISonicParticleEffect
		virtual BOOL GenerateWaterWave(int x, int y, int nAmplitude, int nRadius) = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicScrollView
	// intro:		a view control could scroll its content with animation effect
	//////////////////////////////////////////////////////////////////////////
	class ISonicScrollView : public ISonicBaseWnd
	{
	public:
		enum { BT = BASE_TYPE_SCROLL_VIEW };
	public:
		class ICallback
		{
		public:
			// this function will be called when a scroll button is pressed and the the scroll will determine the step length of a single line with the return result
			// pView is the current scroll view
			// nBar identifies if the scroll bar is a vertical bar or a horizontal one, the value can be SB_VERT or SB_HORZ
			// nDirect has the following values: SB_LINEUP, SB_LINELEFT, SB_LINEDOWN, SB_LINERIGHT
			virtual int GetLineStep(ISonicScrollView * pView, int nBar, int nDirection) = 0;

			// this function will be called when the client area is changed, which would happen when the scroll bars show or hide themselves, 
			// or the whole view has changed its size
			virtual void OnClientRectChanged(ISonicScrollView * pView) = 0;

			// this function will be called when the scroll view is about to scroll to some position, x,y is the destination coordinates
			virtual void OnScrollTo(ISonicScrollView * pView, int x, int y) = 0;
		};
	public:
		// create the scroll view with specific size and scroll bar images
		// a scroll bar could be as simple as only one thumb button, which means pChannel,pImgUpArrow and pDownArrow are all optional resources
		virtual BOOL Create(HWND hWnd, int x, int y, int cx, int cy, ISonicImage * pThumb = NULL, ISonicImage * pChannel = NULL,
			ISonicImage * pUpArrow = NULL, ISonicImage * pDownArrow = NULL) = 0;

		// scroll to a specific position, the x, y coordinates can be absolute or relative to the current position
		// dwScrollAniType can be SATYPE_NONE(no animation), SATYPE_CONSTANT_SPEED and SATYPE_ALTERABLE_SPEED
		// nMaxFrame is the total frame number of scrolling animation, 0 means no animation, same as SATYPE_NONE
		virtual BOOL ScrollTo(int x, int y, BOOL bRelative, DWORD dwScrollAniType, int nMaxFrame = 0) = 0;

		// a client area of a scroll view is the rectangle subtracts the area of scroll bars
		virtual BOOL GetClientRect(LPRECT pRect) = 0;

		// the canvas paint the the whole scrolling paint
		virtual ISonicPaint * GetCanvas() = 0;

		// get the view port which actually contains a memory dc and clips the final visual area
		virtual ISonicPaint * GetViewport() = 0;

		// to test if the scroll bar is scrolling, the optional param pptDest could retrieve the destination position where is scrolling to
		virtual BOOL IsScrolling(LPPOINT pptDest = NULL) = 0;

		// set a callback interface to control the behavior of scroll view at specific times
		virtual BOOL SetCallback(ISonicScrollView::ICallback * pCallback) = 0;

		// scroll one line, nAction is one of the following values: SB_LINEUP, SB_LINELEFT, SB_LINEDOWN, SB_LINERIGHT, SB_PAGEUP, SB_PAGEDOWN, SB_PAGELEFT, SB_PAGERIGHT
		virtual void Scroll(int nBar, int nAction, DWORD dwScrollAniType = SATYPE_NONE, int nMaxFrame = 0) = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicIconPanel
	// intro:		an icon panel which supports multiple page and animation effect when reordering its child items
	//////////////////////////////////////////////////////////////////////////
	class ISonicIconPanel : public ISonicBaseWnd
	{
	public:
		enum { BT = BASE_TYPE_ICON_PANEL };
	public:
		virtual BOOL Create(HWND hWnd, int x, int y, int cx, int cy, DWORD dwStyle = IPS_TABLIKE | IPS_HASPAGE) = 0;

		// add an ISonicAnimation item with specified size to this panel in the order of nIndex
		// return a pointer to the new item if succeeded, otherwise return NULL
		// if nIndex is -1, the new item will be added to the tail of the item list
		virtual ISonicAnimation * AddItem(int cx, int cy, int nIndex = -1) = 0;

		virtual ISonicAnimation * GetItem(int nIndex) = 0;

		virtual BOOL DeleteItem(int nIndex, int nMaxFrame = 0) = 0;

		// test if there is an item at the specified position
		// x,y is relative to the top left corner of this icon panel
		virtual ISonicAnimation * HitTest(int x, int y) = 0;

		virtual int GetItemIndex(ISonicAnimation * pAni) = 0;

		virtual int GetPageCount() = 0;

		virtual int GetItemCount() = 0;

		virtual int GetCurrentPage() = 0;

		virtual BOOL ScrollToPage(int nPage, int nMaxFrame = 0) = 0;

		// scroll the ISonicIconPanel to make the specified item visible
		// nMaxFrame is the animation frame, set it to zero to disable the animation
		// if bMakeItFirst is true, the specified item will be the first one from the left, otherwise just scroll to it and make it visible
		// bMakeItFirst makes no sense when IPS_HASPAGE style is specified
		virtual BOOL ScrollToItem(int nIndex, int nMaxFrame = 0, BOOL bMakeItFirst = FALSE) = 0;

		virtual int GetFirstVisibleItem() = 0;

		virtual BOOL IsItemVisible(int nIndex, BOOL bFully = FALSE) = 0;

		virtual int PageFromItem(int nIndex) = 0;

		virtual int GetFirstItemOfPage(int nPage) = 0;

		virtual int GetItemCountOfPage(int nPage) = 0;

		virtual ISonicPaint * GetPaint() = 0;

		virtual ISonicAnimation * GetCurSelect() = 0;

		virtual BOOL SelectItem(int nIndex) = 0;

		// if some item has changed its size or position, call this function to update the position of every item from nIndex
		virtual BOOL UpdateLayout(int nMaxFrame = 0) = 0;

		// generally, client rect of ISonicIconPanel is equal to the result of GetRect, and it's different only when the IPS_HASPAGE is not specified and
		// the SATTR_ICON_NOPARTIALITEM attributes is set
		virtual BOOL GetClientRect(LPRECT pRect) = 0;

		// set a drag-and-drop callback interface to ISonicIconPanel
		virtual BOOL SetDragDropCallback(IDragDropCallback * pDragDropCallback) = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicTreeView
	// intro:		could be used as a tree ctrl or a simple list box
	//////////////////////////////////////////////////////////////////////////
	class ISonicTreeView : public ISonicBaseWnd
	{
	public:
		enum { BT = BASE_TYPE_TREE_VIEW };
	public:
		virtual BOOL Create(HWND hWnd, int x, int y, int cx, int cy, ISonicImage * pThumb = NULL, ISonicImage * pChannel = NULL,
			ISonicImage * pUpArrow = NULL, ISonicImage * pDownArrow = NULL) = 0;

		virtual ISonicAnimation * AddItem(int cy, int nIndex = -1, ISonicAnimation * pParent = NULL) = 0;

		virtual ISonicAnimation * AddItem(DWORD dwObjectId, int x, int y, BOOL bDestroyOnClear = FALSE, int nIndex = -1, ISonicAnimation * pParent = NULL) = 0;

		virtual BOOL GetClientRect(LPRECT pRect) = 0;

		virtual int GetItemCount() = 0;

		virtual BOOL SetItemHeight(ISonicAnimation * pItem, int nHeight) = 0;

		virtual ISonicAnimation * GetItem(int nIndex, ISonicAnimation * pParent = NULL) = 0;

		virtual BOOL DeleteItem(ISonicAnimation * pItem) = 0;

		virtual BOOL DeleteItem(int nIndex, ISonicAnimation * pParent = NULL) = 0;

		virtual BOOL DeleteAllItems() = 0;

		virtual ISonicAnimation * GetItemParent(ISonicAnimation * pItem) = 0;

		virtual ISonicAnimation * GetFirstChildItem(ISonicAnimation * pItem) = 0;

		virtual ISonicAnimation * GetNextSiblingItem(ISonicAnimation * pItem) = 0;

		virtual ISonicAnimation * GetPrevSiblingItem(ISonicAnimation * pItem) = 0;

		virtual ISonicAnimation * GetFirstVisibleItem() = 0;
		
		virtual ISonicAnimation * GetNextVisibleItem(ISonicAnimation * pItem) = 0;

		virtual int GetVisibleItemCount() = 0;

		virtual int GetChildItemCount(ISonicAnimation * pItem, BOOL bVisibleOnly = FALSE) = 0;

		virtual void UpdateLayout() = 0;

		virtual ISonicAnimation * HitTest(POINT pt) = 0;

		virtual BOOL SetSelectBackgroundImage(ISonicImage * pImg) = 0;

		virtual BOOL SelectItem(ISonicAnimation * pItem) = 0;

		virtual BOOL SelectItem(int nIndex, ISonicAnimation * pParent = NULL) = 0;

		virtual ISonicAnimation * GetCurSel() = 0;

		virtual int GetItemIndex(ISonicAnimation * pItem) = 0;

		virtual BOOL ShowItem(ISonicAnimation * pItem, BOOL bShow) = 0;

		virtual BOOL ExpandItem(ISonicAnimation * pItem, BOOL b) = 0;

		virtual BOOL IsItemExpanded(ISonicAnimation * pItem) = 0;

		virtual BOOL MoveItem(ISonicAnimation * pItem, int nIndex, ISonicAnimation * pParent = NULL) = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// interface:	ISonicUI
	// intro:		object factory and manager, arrange the objects
	//////////////////////////////////////////////////////////////////////////
	class ISonicUI
	{
	public:
		template<class T>
		T * CreateObject(LPCTSTR szObjName = NULL)
		{
			return (T *)CreateBaseObject(szObjName, T::BT);
		}

		// get an object with specified name
		virtual ISonicBase * GetObjectByName(LPCTSTR szObjName) = 0;

		// a simple wrapper of GetObjectByName when finding an image, just for convenience
		virtual ISonicImage * GetImage(LPCTSTR szObjName) = 0;

		// create object with specified name and type
		virtual ISonicBase * CreateBaseObject(LPCTSTR szObjName, DWORD dwBaseType) = 0;

		//////////////////////////////////////////////////////////////////////////
		// old interfaces, not recommend to use.
		// create ISonicSkin
		virtual ISonicSkin * CreateSkin() = 0;

		// create ISonicImage
		virtual ISonicImage * CreateImage() = 0;

		// create ISonicWndEffect
		virtual ISonicWndEffect * CreateWndEffect() = 0;

		// create ISonicString, one string can be used only in one window
		virtual ISonicString * CreateString() = 0;

		// create a text scroll bar
		virtual ISonicTextScrollBar * CreateTextScrollBar() = 0;

		// create a progress bar
		virtual ISonicProgressBar * CreateProgressBar() = 0;

		// create a tab control
		virtual ISonicTab * CreateTab() = 0;

		// create a menu bar
		virtual ISonicMenuBar * CreateMenuBar() = 0;

		// create a particle effect
		virtual ISonicParticleEffect * CreateParticleEffect() = 0;
		//////////////////////////////////////////////////////////////////////////

		// destroy object, don't worry about repeated destruction because of internal memory check
		virtual void DestroyObject(ISonicBase *) = 0;

		// destroy all objects attached to a specified window
		virtual void DestroyAttachedObject(HWND hWnd) = 0;

		// judge whether a object is valid
		virtual BOOL IsValidObject(const ISonicBase *) = 0;

		// get a ISonicWndEffect from a specified window
		virtual ISonicWndEffect * EffectFromHwnd(HWND hWnd) = 0;

		// get a ISonicSkin from a specified window
		virtual ISonicSkin * SkinFromHwnd(HWND hWnd) = 0;

		// handle raw strings to avoid keywords conflicts
		virtual LPCTSTR HandleRawString(LPCTSTR lpszStr, int nType, LPCTSTR lpszUrlAttr = NULL) = 0;

		// create a ISonicPaint
		virtual ISonicPaint * CreatePaint() = 0;

		// create a ISonicAnimation
		virtual ISonicAnimation * CreateAnimation() = 0;

		// return a default HFONT with following style:font-family:宋体, font-size:12
		// !!!Don't delete the handle returned!!!
		virtual HFONT GetDefaultFont() = 0;

		// simply draw text
		virtual BOOL DrawText(HDC hDC, int x, int y, LPCTSTR lpszString, DWORD dwColor = 0, HFONT hFont = NULL) = 0;

		// draw a snapshot of window to the specified hdc
		virtual BOOL DrawWindow(HDC hdc, HWND hWnd, BOOL bRecursion = TRUE) = 0;

		// fill solid rect
		virtual BOOL FillSolidRect(HDC hdc, const RECT * pRect, DWORD dwColor) = 0;

		// rectangle
		virtual BOOL Rectangle(HDC hdc, int left, int top, int right, int bottom, DWORD dwColor) = 0;

		// detour the specified function entry with custom one to implement function hook
		// the return value is the original entry backup in a new cache.
		// use assembly code to call the address returned to accomplish the original function call
		virtual LPVOID DetourFunctionEntry(LPVOID pDestFunc, LPVOID pMyFunc) = 0;

		// to test what's the topmost off-screen pop up window in that position, pt is relative to the off-screen panel window
		virtual HWND OffscreenHitTest(POINT pt) = 0;

		// create a 0xff alpha channel for a non-alpha buffer, use SSE instruction optimization internally
		virtual BOOL BufferCreateAlpha(LPBYTE pBits, DWORD dwSize) = 0;

		// multiple a external alpha for a buffer with alpha channel. 255 means every pixel keep the current alpha
		// 0 means completely transparent. SSE optimization
		virtual BOOL BufferMultipleAlpha(BYTE bAlpha, LPBYTE pBits, DWORD dwSize) = 0;

		// return the current cursor position in a specified window, if the window is using off-screen rendering mode,
		// the point will be automatically converted 
		virtual void GetCursorPosInWindow(HWND hWnd, POINT * ppt) = 0;

		// once this function is called, all pop up windows attached with ISonicSkin will be set as fully transparent windows, 
		// and the normal rendering procedure is disabled. and all rendering will be output to a background buffer. 
		// users can acquire the buffer by IOffscreenUICallBack::Draw
		// remark:	you have to relay all inputs to this window in off-screen rendering mode, including (WM_MOUSEFIRST-WM_MOUSEKEYLAST),
		//			(WM_NCMOUSEFIRST-WM_NCMOUSELAST), (WM_KEYFIRST-WM_KEYLAST), note that WM_MOUSELEAVE and WM_NCMOUSELEAVE are not included in the first-last mouse 
		//			messages and you should generate them manually by TrackMouseEvent.
		//			this function must be called at the very beginning of application
		virtual BOOL SetOffscreenUICallback(IOffscreenUICallBack * pCallback) = 0;

		// return the off-screen UI callback. you can use the return value to judge if the engine is doing off-screen rendering
		virtual IOffscreenUICallBack * GetOffscreenUICallback() = 0;

		// make disable color due to the background color
		virtual DWORD MakeDisableColor(DWORD dwBgColor) = 0;

		// declare that the following ISonicImage::Load(LPCTSTR lpszFilePath) will load images from the zip file
		// return FALSE if the zip file doesn't exist
		virtual BOOL BeginImageFromZip(LPCTSTR lpszZipFilePath) = 0;

		// end loading images from zip, after this function, ISonicImage::Load(LPCTSTR lpszFilePath) will resume loading images from disk
		// return FALSE if there is no previous BeginImageFromZip call
		virtual BOOL EndImageFromZip() = 0;

		// add an input filter to monitor all input messages of the main thread, this is a simple way to monitor all input messages in one step
		// remember to remove the filter before you destroy it, or it may cause crash
		virtual BOOL AddInputFilter(ISonicInputFilter * pInputFilter) = 0;

		// remove the specified input filter, set pInputFilter to null to remove all input filters
		virtual BOOL RemoveInputFilter(ISonicInputFilter * pInputFilter) = 0;

		// customize the global tooltip skin
		virtual BOOL SetCustomerTooltip(ISonicImage * pImgBk, DWORD dwTextColor, int nHorMargin = 3, int nVerMargin = 3, int nLineSpace = 3) = 0;

		// return the top ISonicBaseWnd at the specified position of hWnd. if dwBaseType is zero, any ISonicBaseWnd at the top of that position will be returned.
		// otherwise the the object match the specified type will be returned
		// if hWnd is null, hWnd will be the WindowFromPoint of x, y, and at this time x, y is the screen coordinate
		virtual ISonicBaseWnd * HitTest(int x, int y, HWND hWnd = NULL, DWORD dwBaseType = 0) = 0;

		// simulate a modal loop for a specific window
		virtual BOOL DoModalLoop(HWND hWnd) = 0;
	protected:
		virtual const DWORD * GetObjectTypePtr(const ISonicBase *);
	};
};

extern "C" SONICUI_API sonic_ui::ISonicUI * GetSonicUI();
typedef sonic_ui::ISonicUI * (*FUNC_GET_SONIC_UI)();
