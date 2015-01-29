#ifndef KEYSTATE_HPP_INCLUDED
#define KEYSTATE_HPP_INCLUDED


// Transforms keyboard state (shift, alt, ctrl) into bitset
unsigned KeyState();


#define KEYSTATE_IS_BLANK(s) (s == 0)
#define KEYSTATE_IS_SHIFT(s) ((s & 1) == 1)
#define KEYSTATE_IS_ALT(s) ((s & 2) == 2)
#define KEYSTATE_IS_CTRL(s) ((s & 4) == 4)
#define KEYSTATE_IS_SHIFT_ALT(s) ((s & 3) == 3)
#define KEYSTATE_IS_SHIFT_CTRL(s) ((s & 5) == 5)
#define KEYSTATE_IS_ALT_CTRL(s) ((s & 6) == 6)
#define KEYSTATE_IS_SHIFT_ALT_CTRL(s) ((s & 7) == 7)


#endif

