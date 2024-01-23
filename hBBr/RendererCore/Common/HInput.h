#pragma once
//根据SDL2的输入结构进行兼容

#include <vector>
#include <map>
#include "VulkanRenderer.h"
#include "FormMain.h"
enum class Action : int
{
	RELEASE = SDL_RELEASED,
	PRESS   = SDL_PRESSED,
	REPEAT  = 2,
};

typedef enum
{
    Key_UNKNOWN = 0,

    Key_Return = '\r',
    Key_Escape = '\x1B',
    Key_Backspace = '\b',
    Key_Tab = '\t',
    Key_Space = ' ',
    Key_Exclaim = '!',
    Key_Quotedbl = '"',
    Key_Hash = '#',
    Key_Percent = '%',
    Key_Dollar = '$',
    Key_Ampersand= '&',
    Key_Quote = '\'',
    Key_LeftParen = '(',
    Key_RightParen = ')',
    Key_Asterisk = '*',
    Key_Plus = '+',
    Key_Comma = ',',
    Key_Minus = '-',
    Key_Period = '.',
    Key_Slash = '/',
    Key_0 = '0',
    Key_1 = '1',
    Key_2 = '2',
    Key_3 = '3',
    Key_4 = '4',
    Key_5 = '5',
    Key_6 = '6',
    Key_7 = '7',
    Key_8 = '8',
    Key_9 = '9',
    Key_Colon = ':',
    Key_Semicolon = ';',
    Key_Less = '<',
    Key_Equals = '=',
    Key_Greater = '>',
    Key_Question= '?',
    Key_At = '@',

    /*
       Skip uppercase letters
     */

     Key_LeftBracket = '[',
     Key_BackSlash = '\\',
     Key_RightBracket = ']',
     Key_Caret = '^',
     Key_Underscore = '_',
     Key_BackQuote = '`',
     Key_A = 'a',
     Key_B = 'b',
     Key_C = 'c',
     Key_D = 'd',
     Key_E = 'e',
     Key_F = 'f',
     Key_G = 'g',
     Key_H = 'h',
     Key_I = 'i',
     Key_J = 'j',
     Key_K = 'k',
     Key_L = 'l',
     Key_M = 'm',
     Key_N = 'n',
     Key_O = 'o',
     Key_P = 'p',
     Key_Q = 'q',
     Key_R = 'r',
     Key_S = 's',
     Key_T = 't',
     Key_U = 'u',
     Key_V = 'v',
     Key_W = 'w',
     Key_X = 'x',
     Key_Y = 'y',
     Key_Z = 'z',

     Key_CapsLock = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK),

     Key_F1 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1),
     Key_F2 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2),
     Key_F3 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3),
     Key_F4 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4),
     Key_F5 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5),
     Key_F6 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6),
     Key_F7 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7),
     Key_F8 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8),
     Key_F9 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9),
     Key_F10 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10),
     Key_F11 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11),
     Key_F12 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12),

     Key_Printscreen = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN),
     Key_ScrollLock = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK),
     Key_Pause = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE),
     Key_Insert = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT),
     Key_Home = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME),
     Key_Pageup = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP),
     Key_Delete = '\x7F',
     Key_End = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_END),
     Key_PageDown = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN),
     Key_Right = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT),
     Key_Left = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT),
     Key_Down = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN),
     Key_Up = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP),

     //小键盘
     Key_NumLockClear = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR),
     Key_KP_Divide = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE),
     Key_KP_Multiply = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY),
     Key_KP_Minus = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS),
     Key_KP_Plus = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS),
     Key_KP_Enter = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER),
     Key_KP_1 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1),
     Key_KP_2 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2),
     Key_KP_3 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3),
     Key_KP_4 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4),
     Key_KP_5 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5),
     Key_KP_6 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6),
     Key_KP_7 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7),
     Key_KP_8 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8),
     Key_KP_9 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9),
     Key_KP_0 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0),
     Key_KP_PERIOD = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD),

     Key_Application = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APPLICATION),
     Key_Power = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_POWER),
     Key_KP_Equals = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALS),
     Key_F13 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F13),
     Key_F14 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F14),
     Key_F15 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F15),
     Key_F16 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F16),
     Key_F17 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F17),
     Key_F18 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F18),
     Key_F19 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F19),
     Key_F20 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F20),
     Key_F21 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F21),
     Key_F22 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F22),
     Key_F23 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F23),
     Key_F24 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F24),
     Key_Execute = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXECUTE),
     Key_Help = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HELP),
     Key_Menu = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MENU),
     Key_Select = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SELECT),
     Key_Stop = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_STOP),
     Key_Again = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AGAIN),
     Key_Uudo = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UNDO),
     Key_Cut = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CUT),
     Key_Copy = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COPY),
     Key_Paste = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PASTE),
     Key_Find = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_FIND),
     Key_Mute = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MUTE),
     Key_VolumeUp = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEUP),
     Key_VolumeDown = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEDOWN),
     Key_KP_Comma = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COMMA),
     Key_KP_EqualsAS400 =
     SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALSAS400),

    Key_AltErase = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ALTERASE),
    Key_Sysreq = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SYSREQ),
    Key_Cancel = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CANCEL),
    Key_Clear = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEAR),
    Key_Prior = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRIOR),
    Key_Return2 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RETURN2),
    Key_Separator = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SEPARATOR),
    Key_Out = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OUT),
    Key_Oper = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OPER),
    Key_ClearAgain = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEARAGAIN),
    Key_Crsel = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CRSEL),
    Key_Exsel = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXSEL),

    Key_KP_00 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_00),
    Key_KP_000 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_000),
    Key_ThousandsSeparator =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_THOUSANDSSEPARATOR),
    Key_DecimalSeparator =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DECIMALSEPARATOR),
    Key_CurrencyUnit = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYUNIT),
    Key_CurrencSubunit =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYSUBUNIT),
    Key_KP_LeftParen = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTPAREN),
    Key_KP_RightParen = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTPAREN),
    Key_KP_LeftBRACE = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTBRACE),
    Key_KP_RightBrace = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTBRACE),
    Key_KP_Tab = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_TAB),
    Key_KP_Backspace = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BACKSPACE),
    Key_KP_A = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_A),
    Key_KP_B = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_B),
    Key_KP_C = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_C),
    Key_KP_D = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_D),
    Key_KP_E = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_E),
    Key_KP_F = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_F),
    Key_KP_XOR = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_XOR),
    Key_KP_Power = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_POWER),
    Key_KP_Percent = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERCENT),
    Key_KP_Less = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LESS),
    Key_KP_Greater = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_GREATER),
    Key_KP_Ampersand = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AMPERSAND),
    Key_KP_Dblampersand =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLAMPERSAND),
    Key_KP_Verticalbar =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_VERTICALBAR),
    Key_KP_Dblverticalbar =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLVERTICALBAR),
    Key_KP_Colon = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COLON),
    Key_KP_Hash = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HASH),
    Key_KP_Space = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_SPACE),
    Key_KP_At = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AT),
    Key_KP_Exclam = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EXCLAM),
    Key_KP_MemStore = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSTORE),
    Key_KP_MemRecall = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMRECALL),
    Key_KP_MemClear = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMCLEAR),
    Key_KP_MemAdd = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMADD),
    Key_KP_MemSubtract =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSUBTRACT),
    Key_KP_MemMultiply =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMMULTIPLY),
    Key_KP_MemDivide = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMDIVIDE),
    Key_KP_PlusMinus = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUSMINUS),
    Key_KP_Clear = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEAR),
    Key_KP_ClearEntry = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEARENTRY),
    Key_KP_Binary = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BINARY),
    Key_KP_Octal = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_OCTAL),
    Key_KP_Decimal = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DECIMAL),
    Key_KP_Hexadecimal =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HEXADECIMAL),

    Key_LCtrl = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL),
    Key_LShift = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT),
    Key_LAlt = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT),
    Key_LGui = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI),
    Key_RCtrl = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL),
    Key_RShift = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT),
    Key_RAlt = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT),
    Key_RGui = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI),

    Key_Mode = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE),

    Key_AudioNext = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIONEXT),
    Key_AudioPrev = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPREV),
    Key_AudioStop = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOSTOP),
    Key_AudioPlay = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPLAY),
    Key_AudioMute = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOMUTE),
    Key_Mediaselect = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIASELECT),
    Key_WWW = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_WWW),
    Key_Mail = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MAIL),
    Key_Calculator = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALCULATOR),
    Key_Computer = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COMPUTER),
    Key_AC_Search = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SEARCH),
    Key_AC_Home = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_HOME),
    Key_AC_Back = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BACK),
    Key_AC_Forward = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_FORWARD),
    Key_AC_Stop = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_STOP),
    Key_AC_Refresh = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_REFRESH),
    Key_AC_Bookmarks = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BOOKMARKS),

    Key_BrightnessDown =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSDOWN),
    Key_BRightnessUp = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSUP),
    Key_DisplaySwitch = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DISPLAYSWITCH),
    Key_Kbdillumtoggle =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMTOGGLE),
    Key_KbdillumDown = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMDOWN),
    Key_KbdillumUp = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMUP),
    Key_Eject = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EJECT),
    Key_Sleep = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SLEEP),
    Key_App1 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APP1),
    Key_App2 = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APP2),

    Key_AudioRewind = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOREWIND),
    Key_AudioFastForward = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOFASTFORWARD),

    Key_SoftLeft = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTLEFT),
    Key_SoftRight = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTRIGHT),
    Key_Call = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALL),
    Key_EndCall = SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ENDCALL)
} KeyCode;

enum class KeyMod : int
{
    Mod_None = 0x0000,
    Mod_LShift = 0x0001,
    Mod_RShift = 0x0002,
    Mod_LCtrl = 0x0040,
    Mod_RCtrl = 0x0080,
    Mod_LAlt = 0x0100,
    Mod_RAlt = 0x0200,
    Mod_LGui = 0x0400,
    Mod_RGui = 0x0800,
    Mod_Num = 0x1000,
    Mod_Caps = 0x2000,
    Mod_Mode = 0x4000,
    Mod_Scroll = 0x8000,

    Mod_Ctrl = Mod_LCtrl | Mod_RCtrl,
    Mod_Shift = Mod_LShift | Mod_RShift,
    Mod_Alt = Mod_LAlt | Mod_RAlt,
    Mod_Gui = Mod_LGui | Mod_RGui,

    Mod_RESERVED = Mod_Scroll /* This is for source-level compatibility with SDL 2.0.0. */
};

enum MouseButton
{
	Button_Left = 1,
	Button_Middle = 2,
	Button_Right = 3,
	Button_X1 = 4,
	Button_X2 = 5,
	Button_LMask = SDL_BUTTON_LMASK,
	Button_MMask = SDL_BUTTON_MMASK,
	Button_RMask = SDL_BUTTON_RMASK,
	Button_X1Mask = SDL_BUTTON_X1MASK,
	Button_X2Mask = SDL_BUTTON_X2MASK,
};

struct KeyCallBack
{
	KeyCode key;
	Action  action;
	KeyMod  mod;
    VulkanForm* focusWindowHandle = nullptr;
    bool bKeyDown = false;
};

struct MouseCallBack
{
	MouseButton button;
	Action  action;
	VulkanForm* focusWindowHandle = nullptr;
    bool bMouseDown = false;
};

class HInput
{
	friend class VulkanApp;

    static std::vector<KeyCallBack> _keyRegisterDefault;
    static std::vector<KeyCallBack> _keyRegisterRepeat;
    //
    static std::vector<MouseCallBack> _mouseRegisterDefault;
    static std::vector<MouseCallBack> _mouseRegisterRepeat;

public:
	//Keyboard	注册当前帧反馈按键

	static inline bool GetKey(KeyCode key, class VulkanRenderer* renderer = nullptr) {
        if (!renderer)
        {
            renderer = VulkanApp::_mainForm->renderer;
        }
		if (!renderer)
			return false;
		return FindRepeatKey(key, renderer) != nullptr;
	}

	static inline bool GetKeyDown(KeyCode key, class VulkanRenderer* renderer = nullptr) {
        if (!renderer)
        {
            renderer = VulkanApp::_mainForm->renderer;
        }
        if (!renderer)
            return false;
		return FindDefaultKeyDown(key, Action::PRESS, renderer) != nullptr;
	}

	static inline bool GetKeyUp(KeyCode key, class VulkanRenderer* renderer = nullptr) {
        if (!renderer)
        {
            renderer = VulkanApp::_mainForm->renderer;
        }
        if (!renderer)
            return false;
		return FindDefaultKey(key, Action::RELEASE, renderer) != nullptr;
	}

	static inline bool GetMouse(MouseButton button, class VulkanRenderer* renderer = nullptr) {
        if (!renderer)
        {
            renderer = VulkanApp::_mainForm->renderer;
        }
        if (!renderer)
            return false;
		return FindRepeatMouse(button, renderer) != nullptr;
	}

	static inline bool GetMouseDown(MouseButton button, class VulkanRenderer* renderer = nullptr) {
        if (!renderer)
        {
            renderer = VulkanApp::_mainForm->renderer;
        }
        if (!renderer)
            return false;
		return FindDefaultMouseDown(button, Action::PRESS, renderer) != nullptr;
	}

	static inline bool GetMouseUp(MouseButton button, class VulkanRenderer* renderer = nullptr) {
        if (!renderer)
        {
            renderer = VulkanApp::_mainForm->renderer;
        }
        if (!renderer)
            return false;
		return FindDefaultMouse(button, Action::RELEASE, renderer) != nullptr;
	}

    static inline void SetCursorPos(glm::vec2 pos)
    {
        SDL_WarpMouseGlobal(pos.x, pos.y);
    }

    static inline void SetCursorPosClient(glm::vec2 pos)
    {
        if (VulkanApp::GetFocusForm() && VulkanApp::GetFocusForm()->window)
        {
            SDL_WarpMouseInWindow(VulkanApp::GetFocusForm()->window, pos.x, pos.y);
        }     
    }

    static inline void ShowCursor(bool bShow)
    {
        if (bShow)
            SDL_ShowCursor();
        else
            SDL_HideCursor();
    }

	static inline glm::vec2 GetMousePos()
	{
        glm::vec2 result(0);
        SDL_GetGlobalMouseState(&result.x, &result.y);
        //return _mousePos;
        return result;
	}

    static inline glm::vec2 GetMousePosClient()
    {
        glm::vec2 result(0);
        SDL_GetMouseState(&result.x, &result.y);
        //return _mousePosInWindow;
        return result;
    }

	//键盘输入的回调函数中调用,记录当前帧按下的按键,其他时候不要主动调用该函数!
	static void KeyProcess(VulkanForm* focusWindowHandle , KeyCode key, KeyMod mod ,Action action);

	//鼠标输入的回调函数中调用,记录当前帧按下的按键,其他时候不要主动调用该函数!
	static void MouseProcess(VulkanForm* focusWindowHandle, MouseButton mouse, Action action);

private:

	static inline KeyCallBack* FindDefaultKey(KeyCode key , Action action, class VulkanRenderer* renderer)
	{
		auto it = std::find_if(_keyRegisterDefault.begin(), _keyRegisterDefault.end(), [key, action, renderer](KeyCallBack& callback) {
			return callback.key == key && callback.action == action && callback.focusWindowHandle && callback.focusWindowHandle->renderer == renderer;
			});
		if (it != _keyRegisterDefault.end())
			return &(*it);
		else
			return nullptr;
	}

    static inline KeyCallBack* FindDefaultKeyDown(KeyCode key, Action action, class VulkanRenderer* renderer)
    {
        auto it = std::find_if(_keyRegisterRepeat.begin(), _keyRegisterRepeat.end(), [key, action, renderer](KeyCallBack& callback) {
            return callback.key == key && callback.action == action && callback.focusWindowHandle && callback.focusWindowHandle->renderer == renderer && callback.bKeyDown ==true;
            });
        if (it != _keyRegisterRepeat.end())
            return &(*it); 
        else
            return nullptr;
    }

	static inline KeyCallBack* FindRepeatKey(KeyCode key, class VulkanRenderer* renderer)
	{
		auto it = std::find_if(_keyRegisterRepeat.begin(), _keyRegisterRepeat.end(), [renderer,key](KeyCallBack& callback) {
            return callback.key == key && callback.focusWindowHandle && callback.focusWindowHandle->renderer == renderer;
			});
		if (it != _keyRegisterRepeat.end())
			return &(*it);
		else
			return nullptr;
	}

	//
	static inline MouseCallBack* FindDefaultMouse(MouseButton mouse, Action action, class VulkanRenderer* renderer)
	{
		auto it = std::find_if(_mouseRegisterDefault.begin(), _mouseRegisterDefault.end(), [mouse, action, renderer](MouseCallBack& callback) {
			return callback.button == mouse && callback.action == action && callback.focusWindowHandle && callback.focusWindowHandle->renderer == renderer;
			});
		if (it != _mouseRegisterDefault.end())
			return &(*it);
		else
			return nullptr;
	}

    static inline MouseCallBack* FindDefaultMouseDown(MouseButton mouse, Action action, class VulkanRenderer* renderer)
    {
        auto it = std::find_if(_mouseRegisterRepeat.begin(), _mouseRegisterRepeat.end(), [mouse, action, renderer](MouseCallBack& callback) {
            return callback.button == mouse && callback.action == action && callback.focusWindowHandle && callback.focusWindowHandle->renderer == renderer && callback.bMouseDown == true;
            });
        if (it != _mouseRegisterRepeat.end())
            return &(*it);
        else
            return nullptr;
    }

	static inline MouseCallBack* FindRepeatMouse(MouseButton mouse, class VulkanRenderer* renderer)
	{
		auto it = std::find_if(_mouseRegisterRepeat.begin(), _mouseRegisterRepeat.end(), [mouse, renderer](MouseCallBack& callback) {
			return callback.button == mouse && callback.focusWindowHandle && callback.focusWindowHandle->renderer == renderer;
			});
        if (it != _mouseRegisterRepeat.end())
        {
            return &(*it);
        }
		else
			return nullptr;
	}

    //清空当前帧的输入缓存,其他时候不要主动调用该函数!
    static void ClearInput()
    {
        _keyRegisterDefault.clear();
        //_keyRegisterRepeat.clear();
        _mouseRegisterDefault.clear();
        //_mouseRegisterRepeat.clear();
        for (auto& i : _keyRegisterRepeat)
        {
            i.bKeyDown = false;
        }
        for (auto& i : _mouseRegisterRepeat)
        {
            i.bMouseDown = false;
        }
    }

};