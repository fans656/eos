#ifndef COLOR_H
#define COLOR_H

constexpr uint rgb_red(uint rgb) { return (rgb >> 16) & 0xff; }
constexpr uint rgb_green(uint rgb) { return (rgb >> 8) & 0xff; }
constexpr uint rgb_blue(uint rgb) { return (rgb >> 0) & 0xff; }

constexpr uint RGB2BGR(uint rgb) {
    return (rgb_blue(rgb) << 16) | (rgb_green(rgb) << 8) | rgb_red(rgb);
}

// http://www.html-color-names.com/color-chart.php
constexpr uint AliceBlue =         0xf0f8ff;
constexpr uint RoyalBlue =         0x4169e1;
constexpr uint DarkSlateBlue =     0x483d8b; 
constexpr uint SteelBlue =         0x4682b4; 
constexpr uint LightSteelBlue =    0xb0c4de; 
constexpr uint GhostWhite =        0xf8f8ff; 

constexpr uint White =        0xffffff; 

#endif
