#ifndef STYLE_H
#define STYLE_H

// qm_todo: replace strings with hex literals.

// generally, SDL uses 0xAARRGGBB
#define BACKGROUND_COLOR 0xFF24292E
#define LINE_COLOR 0xFF505050
#define AGENT_COLOR 0xFF50A050
#define WALL_COLOR 0xFF5680AD

// but, this one is different to match filledCircleColor(), which for some reason,
// casts Uint32 color like ``Uint8 *c = (Uint8 *)&color.
// I am extatic.
#define FOOD_COLOR 0xCCB224FF 


#endif // !STYLE_H
