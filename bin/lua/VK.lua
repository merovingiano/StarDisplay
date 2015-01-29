print("  sourcing VK.lua")
--
-- Virtual Keys, Standard Set, ASCII
--

local VK = {
  LBUTTON = 1, 
  RBUTTON = 2, 
  CANCEL = 3, 
  MBUTTON = 4,  -- NOT contiguous with L & RBUTTON 
  XBUTTON1 = 5, 
  XBUTTON2 = 6, 
  BACK = 8, 
  BACKSPACE = 8, 
  TAB = 9, 
  CLEAR = 12, 
  RETURN = 13, 
  SHIFT = 16, 
  CONTROL = 17, 
  CTRL = 17, 
  MENU = 18, 
  ALT = 18, 
  PAUSE = 19, 
  CAPITAL = 20, 
  CAPSLOCK = 20, 
  KANA = 21, 
  HANGUL = 21, 
  JUNJA = 23, 
  FINAL = 24, 
  HANJA = 25, 
  KANJI = 25, 
  CONVERT = 28, 
  NONCONVERT = 29, 
  ACCEPT = 30, 
  MODECHANGE = 31, 
  ESCAPE = 27, 
  SPACE = 32, 
  PRIOR = 33, 
  PGUP = 33, 
  NEXT = 34, 
  PGDN = 34, 
  END = 35, 
  HOME = 36, 
  LEFT = 37, 
  UP = 38, 
  RIGHT = 39, 
  DOWN = 40, 
  SELECT = 41, 
  PRINT = 42, 
  EXECUTE = 43, 
  SNAPSHOT = 44, 
  INSERT = 45, 
  DELETE = 46, 
  HELP = 47, 
-- VK["0"] - VK["9"] are the same as ASCII '0' - '9' (0x30 - 0x39)
  -- 0x40 : unassigned
  -- VK.A - VK.Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
  LWIN = 91, 
  RWIN = 92, 
  APPS = 93, 
  SLEEP = 95, 
  NUMPAD0 = 96, 
  NUMPAD1 = 97, 
  NUMPAD2 = 98, 
  NUMPAD3 = 99, 
  NUMPAD4 = 100, 
  NUMPAD5 = 101, 
  NUMPAD6 = 102, 
  NUMPAD7 = 103, 
  NUMPAD8 = 104, 
  NUMPAD9 = 105, 
  MULTIPLY = 106, 
  ADD = 107, 
  SEPARATOR = 108, 
  SUBTRACT = 109, 
  DECIMAL = 110, 
  DIVIDE = 111, 
  F1 = 112, 
  F2 = 113, 
  F3 = 114, 
  F4 = 115, 
  F5 = 116, 
  F6 = 117, 
  F7 = 118, 
  F8 = 119, 
  F9 = 120, 
  F10 = 121, 
  F11 = 122, 
  F12 = 123, 
  F13 = 124, 
  F14 = 125, 
  F15 = 126, 
  F16 = 127, 
  F17 = 128, 
  F18 = 129, 
  F19 = 130, 
  F20 = 131, 
  F21 = 132, 
  F22 = 133, 
  F23 = 134, 
  F24 = 135, 
  NUMLOCK = 144, 
  SCROLL = 145, 
  -- VK_L & VK_R - left and right Alt, Ctrl and Shift virtual keys. 
  -- Used only as parameters to GetAsyncKeyState() and GetKeyState(). 
  -- No other API or message will distinguish left and right keys in this way. 
  LSHIFT = 160, 
  RSHIFT = 161, 
  LCONTROL = 162, 
  RCONTROL = 163, 
  LMENU = 164, 
  RMENU = 165, 
  BROWSER_BACK = 166, 
  BROWSER_FORWARD = 167, 
  BROWSER_REFRESH = 168, 
  BROWSER_STOP = 169, 
  BROWSER_SEARCH = 170, 
  BROWSER_FAVORITES = 171, 
  BROWSER_HOME = 172, 
  VOLUME_MUTE = 173, 
  VOLUME_DOWN = 174, 
  VOLUME_UP = 175, 
  MEDIA_NEXT_TRACK = 176, 
  MEDIA_PREV_TRACK = 177, 
  MEDIA_STOP = 178, 
  MEDIA_PLAY_PAUSE = 179, 
  LAUNCH_MAIL = 180, 
  LAUNCH_MEDIA_SELECT = 181, 
  LAUNCH_APP1 = 182, 
  LAUNCH_APP2 = 183, 
  OEM_1 = 186, 
  OEM_PLUS = 187, 
  OEM_COMMA = 188, 
  OEM_MINUS = 189, 
  OEM_PERIOD = 190, 
  OEM_2 = 191, 
  OEM_3 = 192, 
  OEM_4 = 219, 
  OEM_5 = 220, 
  OEM_6 = 221, 
  OEM_7 = 222, 
  OEM_8 = 223, 
  OEM_102 = 226, 
  PACKET = 231, 
  PROCESSKEY = 229, 
  ATTN = 246, 
  CRSEL = 247, 
  EXSEL = 248, 
  EREOF = 249, 
  PLAY = 250, 
  ZOOM = 251, 
  NONAME = 252, 
  PA1 = 253, 
  OEM_CLEAR = 254, 
}

-- assign ASCII
for i = string.byte("0"), string.byte("9") do
  VK[string.char(i)] = i
end
for i = string.byte("A"), string.byte("Z") do
  VK[string.char(i)] = i
end
  
local band = bit.band
local bor = bit.bor
local lshift = bit.lshift


-- Check for exlusive state
function VK.KEYSTATE_IS_BLANK(s) return s == 0 end
function VK.KEYSTATE_IS_SHIFT(s) return band(s, 1) == 1 end
function VK.KEYSTATE_IS_ALT(s) return band(s, 2) == 2 end
function VK.KEYSTATE_IS_CTRL(s) return band(s, 4) == 4 end
function VK.KEYSTATE_IS_SHIFT_ALT(s) return band(s, 3) == 3 end
function VK.KEYSTATE_IS_SHIFT_CTRL(s) return band(s, 5) == 5 end
function VK.KEYSTATE_IS_ALT_CTRL(s) return band(s, 6) == 6 end
function VK.KEYSTATE_IS_SHIFT_ALT_CTRL(s) return band(s, 7) == 7 end

-- Check for inclusive state
function VK.KEYSTATE_SHIFT(s) return band(s, 1) end
function VK.KEYSTATE_ALT(s) return band(s, 2) end
function VK.KEYSTATE_CTRL(s) return band(s, 4) end


function VK.Encode(key, shift, alt, ctrl, dbl)
  if shift then shift = 1 else shift = 0 end
  if alt then alt = 2 else alt = 0 end
  if ctrl then ctrl = 4 else ctrl = 0 end
  if dbl then dbl = 8 else dbl = 0 end
  return bor(lshift(key, 8), shift, alt, ctrl, dbl)
end


return VK



