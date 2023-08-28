#pragma once
//输入按键的模板来自GLFW，所以属于直接兼容GLFW.
//其他窗口输入方式则需要重新映射一下。

#include <vector>
#include <map>
#include "VulkanRenderer.h"
enum class Action : int
{
	RELEASE = 0,
	PRESS   = 1,
	REPEAT  = 2,
};

enum class JoystickHat : int
{
	Centered = 0,
	Up = 1,
	Right = 2,
	Down = 4,
	Left = 8,
	Right_Up = (Right | Up),
	Right_Down = (Right | Down),
	Left_Up = (Left | Up),
	Left_Down = (Left | Down),
};

enum class Joystick : int
{
	Joystick_1             =0 ,
	Joystick_2             =1 ,
	Joystick_3             =2 ,
	Joystick_4             =3 ,
	Joystick_5             =4 ,
	Joystick_6             =5 ,
	Joystick_7             =6 ,
	Joystick_8             =7 ,
	Joystick_9             =8 ,
	Joystick_10            =9 ,
	Joystick_11            =10,
	Joystick_12            =11,
	Joystick_13            =12,
	Joystick_14            =13,
	Joystick_15            =14,
	Joystick_16            =15,
	Joystick_LAST          =Joystick_16,
};

enum class GamePadButton : int
{
	A					=0	,
	B					=1	,
	X					=2	,
	Y					=3	,
	Left_Bumper			=4	,
	Right_Bumper		=5	,
	Back				=6	,
	Start				=7	,
	Guide				=8	,
	Left_Thumb			=9	,
	Right_Thumb			=10	,
	Dpad_Up				=11	,
	Dpad_Right			=12	,
	Dpad_Down			=13	,
	Dpad_Left			=14	,
	Last				=Dpad_Left,
														 
	Cross				=A		 ,
	Circle				=B		 ,
	Square				=X		 ,
	Triangle			=Y		 ,
};

enum class GamePadAxis : int
{
	Left_X			=0,
	Left_Y			=1,
	Right_X			=2,
	Right_Y			=3,
	Left_Trigger	=4,
	Right_Trigger	=5,
	LAST			= Right_Trigger,
};

enum class KeyCode : int
{
	None				   =-1 ,
	Space				   =32 ,  
	Apostrophe             =39 ,   /* ' */
	Comma                  =44 ,   /* , */
	Minus                  =45 ,   /* - */
	Period                 =46 ,   /* . */
	Slash                  =47 ,   /* / */
	Num0                   =48 ,
	Num1                   =49 ,
	Num2                   =50 ,
	Num3                   =51 ,
	Num4                   =52 ,
	Num5                   =53 ,
	Num6                   =54 ,
	Num7                   =55 ,
	Num8                   =56 ,
	Num9                   =57 ,
	Semicolon              =59 ,   /* ; */
	Equal                  =61 ,   /* = */
	A                      =65 ,  
	B                      =66 ,  
	C                      =67 ,  
	D                      =68 ,  
	E                      =69 ,  
	F                      =70 ,  
	G                      =71 ,  
	H                      =72 ,  
	I                      =73 ,  
	J                      =74 ,  
	K                      =75 ,  
	L                      =76 ,  
	M                      =77 ,  
	N                      =78 ,  
	O                      =79 ,  
	P                      =80 ,  
	Q                      =81 ,  
	R                      =82 ,  
	S                      =83 ,  
	T                      =84 ,  
	U                      =85 ,  
	V                      =86 ,  
	W                      =87 ,  
	X                      =88 ,  
	Y                      =89 ,  
	Z                      =90 ,  
	LeftBracket            =91 ,    	/* [ */
	Backslash              =92 ,    	/* \ */
	RightBracket           =93 ,    	/* ] */
	GraveAccent            =96 ,    	/* ` */
	World1                =161,     	/* non-US #1 */
	World2                =162,     	/* non-US #2 */
						    		    
	/* Function keys */    		    
	Escape                 =256,    
	Enter                  =257,  
	Tab                    =258,  
	BackSpace              =259,  
	Insert                 =260,  
	Delete                 =261,  
	Right                  =262,  
	Left                   =263,  
	Down                   =264,  
	Up                     =265,  
	PageUp                 =266,  
	PageDown               =267,  
	Home                   =268,  
	End                    =269,  
	CapsLock               =280,  
	ScrollLock             =281,  
	NumLock                =282,  
	PrintScreen            =283,  
	Pause                  =284,  
	F1                     =290,  
	F2                     =291,  
	F3                     =292,  
	F4                     =293,  
	F5                     =294,  
	F6                     =295,  
	F7                     =296,  
	F8                     =297,  
	F9                     =298,  
	F10                    =299,  
	F11                    =300,  
	F12                    =301,  
	F13                    =302,  
	F14                    =303,  
	F15                    =304,  
	F16                    =305,  
	F17                    =306,  
	F18                    =307,  
	F19                    =308,  
	F20                    =309,  
	F21                    =310,  
	F22                    =311,  
	F23                    =312,  
	F24                    =313,  
	F25                    =314,  
	KP_0                   =320,  
	KP_1                   =321,  
	KP_2                   =322,  
	KP_3                   =323,  
	KP_4                   =324,  
	KP_5                   =325,  
	KP_6                   =326,  
	KP_7                   =327,  
	KP_8                   =328,  
	KP_9                   =329,  
	KP_Decimal             =330,  
	KP_Divide              =331,  
	KP_Multiply            =332,  
	KP_Subtract            =333,  
	KP_Add                 =334,  
	KP_Enter               =335,  
	KP_Equal               =336,  
	LeftShift              =340,  
	LeftControl            =341,
	LeftAlt                =342,
	LeftSuper              =343,
	RightShift             =344,  
	RightControl           =345,  
	RightAlt               =346,
	RightSuper             =347,
	MENU                   =348,  
};

enum class KeyMod : int
{
	Shift          = 0x0001,
	Contorl        = 0x0002,
	Alt            = 0x0004,
	Super          = 0x0008,
	Caps_Lock      = 0x0010,
	NumLock       = 0x0020,
};

enum class MouseButton : int
{
	Button_1 = 0,
	Button_2 = 1,
	Button_3 = 2,
	Button_4 = 3,
	Button_5 = 4,
	Button_6 = 5,
	Button_7 = 6,
	Button_8 = 7,
	Button_Last = Button_8,
	Button_Left = Button_1,
	Button_Right = Button_2,
	Button_Middle = Button_3,
};

struct KeyCallBack
{
	KeyCode key;
	Action  action;
	KeyMod  mod;
	void* focusWindowHandle = NULL; 
};

struct MouseCallBack
{
	MouseButton button;
	Action  action;
	KeyMod  mod;
	void* focusWindowHandle = NULL;
};

class HInput
{
	friend class VulkanApp;
public:
	//Keyboard	注册当前帧反馈按键

	static inline bool GetKey(KeyCode key , class VulkanRenderer* renderer = NULL) {
		if (!HasFocus(renderer))
			return false;
		return FindRepeatKey(key) != NULL;
	}

	static inline bool GetKeyDown(KeyCode key, class VulkanRenderer* renderer = NULL) {
		if (!HasFocus(renderer))
			return false;
		return FindDefaultKey(key, Action::PRESS) != NULL;
	}

	static inline bool GetKeyUp(KeyCode key, class VulkanRenderer* renderer = NULL) {
		if (!HasFocus(renderer))
			return false;
		return FindDefaultKey(key, Action::RELEASE) != NULL;
	}

	//
	static inline bool GetMouse(MouseButton button, class VulkanRenderer* renderer = NULL) {
		if (!HasFocus(renderer))
			return false;
		return FindRepeatMouse(button) != NULL;
	}

	static inline bool GetMouseDown(MouseButton button, class VulkanRenderer* renderer = NULL) {
		if (!HasFocus(renderer))
			return false;
		return FindDefaultMouse(button, Action::PRESS) != NULL;
	}

	static inline bool GetMouseUp(MouseButton button, class VulkanRenderer* renderer = NULL) {
		if (!HasFocus(renderer))
			return false;
		return FindDefaultMouse(button, Action::RELEASE) != NULL;
	}

	static inline glm::vec2 GetMousePos()
	{
		return _mousePos;
	}

	//键盘输入的回调函数中调用,记录当前帧按下的按键,其他时候不要主动调用该函数!
	static void KeyProcess(void* focusWindowHandle , KeyCode key, KeyMod mod ,Action action);

	//鼠标输入的回调函数中调用,记录当前帧按下的按键,其他时候不要主动调用该函数!
	static void MouseProcess(void* focusWindowHandle, MouseButton mouse, KeyMod mod, Action action);

private:

	static bool HasFocus(class VulkanRenderer* renderer)
	{
		if (renderer == NULL)
		{
			if (!VulkanApp::GetMainForm() || !VulkanApp::GetMainForm()->renderer)
			{
				return false;
			}
			renderer = VulkanApp::GetMainForm()->renderer;
		}
		return renderer->HasFocus();
	}

	static inline KeyCallBack* FindDefaultKey(KeyCode key , Action action)
	{
		auto it = std::find_if(_keyRegisterDefault.begin(), _keyRegisterDefault.end(), [key, action](KeyCallBack& callback) {
			return callback.key == key && callback.action == action;
			});
		if (it != _keyRegisterDefault.end())
			return &(*it);
		else
			return NULL;
	}

	static inline KeyCallBack* FindRepeatKey(KeyCode key)
	{
		auto it = std::find_if(_keyRegisterRepeat.begin(), _keyRegisterRepeat.end(), [key](KeyCallBack& callback) {
			return callback.key == key;
			});
		if (it != _keyRegisterRepeat.end())
			return &(*it);
		else
			return NULL;
	}

	//
	static inline MouseCallBack* FindDefaultMouse(MouseButton mouse, Action action)
	{
		auto it = std::find_if(_mouseRegisterDefault.begin(), _mouseRegisterDefault.end(), [mouse, action](MouseCallBack& callback) {
			return callback.button == mouse && callback.action == action;
			});
		if (it != _mouseRegisterDefault.end())
			return &(*it);
		else
			return NULL;
	}

	static inline MouseCallBack* FindRepeatMouse(MouseButton mouse)
	{
		auto it = std::find_if(_mouseRegisterRepeat.begin(), _mouseRegisterRepeat.end(), [mouse](MouseCallBack& callback) {
			return callback.button == mouse;
			});
		if (it != _mouseRegisterRepeat.end())
			return &(*it);
		else
			return NULL;
	}

	static inline void SetMousePos(glm::vec2 pos)
	{
		_mousePos = pos;
	}

	//清空当前帧的输入缓存,其他时候不要主动调用该函数!
	static void ClearInput()
	{
		_keyRegisterDefault.clear();
	}

	static std::vector<KeyCallBack> _keyRegisterDefault;
	static std::vector<KeyCallBack> _keyRegisterRepeat;
	//
	static std::vector<MouseCallBack> _mouseRegisterDefault;
	static std::vector<MouseCallBack> _mouseRegisterRepeat;
	//
	static glm::vec2 _mousePos;
};