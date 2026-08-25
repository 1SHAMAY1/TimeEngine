#pragma once

#include "Core/PreRequisites.h"
#include "SpriteScriptRuntime.hpp"


struct ScriptPresetTemplate
{
    TEString Name;
    TEString Category;
    TEString Description;
    TEString Code;
};

class SpriteModeLibrary
{
public:
    static TEArray<ScriptPresetTemplate> GetPresetTemplates()
    {
        TEArray<ScriptPresetTemplate> presets;

        // 1. Animated Vector Star
        presets.Add({
            "Animated Vector Star",
            "Vector",
            "A smooth rotating vector star with glowing background ring.",
            "// Animated Vector Star\n"
            "var cx = get_width() * 0.5\n"
            "var cy = get_height() * 0.5\n"
            "var t = time()\n"
            "\n"
            "// Outer Pulsing Ring\n"
            "var ring_r = 44.0 + sin(t * 3.0) * 4.0\n"
            "draw_circle_outline(cx, cy, ring_r, rgba(0.2, 0.6, 1.0, 0.4), 2.0)\n"
            "draw_circle_filled(cx, cy, ring_r * 0.85, rgba(0.05, 0.1, 0.25, 0.9))\n"
            "\n"
            "// 5-Spike Rotating Star\n"
            "var angle = t * 1.5\n"
            "var outer = 36.0 + sin(t * 5.0) * 3.0\n"
            "var inner = 16.0\n"
            "draw_star(cx, cy, 5, outer, inner, rgba(1.0, 0.85, 0.2, 1.0), angle)\n"
            "\n"
            "// Center core\n"
            "draw_circle_filled(cx, cy, 6.0, WHITE)\n"
            "draw_text_outlined(cx - 20, cy + 46, \"STAR\", YELLOW, BLACK, 1.0)\n"
        });

        // 2. Pixel Art Bouncing Slime
        presets.Add({
            "Pixel Bouncing Slime",
            "Pixel",
            "A multi-frame animated pixel art slime with eyes and bouncy squash.",
            "// Pixel Bouncing Slime Animation\n"
            "set_grid_size(16, 16)\n"
            "clear_pixels(rgba(0, 0, 0, 0))\n"
            "\n"
            "var f = frame()\n"
            "var bounce = sin(f * 0.785) * 2.0\n"
            "var squash = cos(f * 0.785) * 1.0\n"
            "\n"
            "var sx = 8\n"
            "var sy = 10 + bounce\n"
            "var w = 5 - squash\n"
            "var h = 4 + squash\n"
            "\n"
            "// Slime body\n"
            "var green = rgba(0.2, 0.85, 0.3, 1.0)\n"
            "var dark_green = rgba(0.1, 0.55, 0.2, 1.0)\n"
            "draw_pixel_rect(sx - w, sy - h, w * 2, h * 2, green)\n"
            "draw_pixel_rect(sx - w + 1, sy + h - 1, (w - 1) * 2, 1, dark_green)\n"
            "\n"
            "// Eyes\n"
            "set_pixel(sx - 2, sy - 1, BLACK)\n"
            "set_pixel(sx + 2, sy - 1, BLACK)\n"
            "set_pixel(sx - 2, sy - 2, WHITE)\n"
            "set_pixel(sx + 2, sy - 2, WHITE)\n"
            "\n"
            "// Cheek blush\n"
            "set_pixel(sx - 3, sy, rgba(1.0, 0.4, 0.6, 0.8))\n"
            "set_pixel(sx + 3, sy, rgba(1.0, 0.4, 0.6, 0.8))\n"
        });

        // 3. Neon Sci-Fi Shield HUD
        presets.Add({
            "Neon Shield HUD",
            "Vector",
            "Futuristic neon radar HUD with rotating scanners and status text.",
            "// Neon Shield HUD\n"
            "var cx = get_width() * 0.5\n"
            "var cy = get_height() * 0.5\n"
            "var t = time()\n"
            "\n"
            "// Outer Hexagon Shield\n"
            "var shield_col = rgba(0.1, 0.9, 0.8, 0.8)\n"
            "draw_polygon(cx, cy, 48.0, 6, shield_col, 0.0, 2.0)\n"
            "draw_polygon(cx, cy, 40.0, 6, rgba(0.1, 0.9, 0.8, 0.15), t * 0.2, 0.0)\n"
            "\n"
            "// Inner Rotating Radar Sweep\n"
            "var sweep_angle = t * 3.0\n"
            "var lx = cx + cos(sweep_angle) * 36.0\n"
            "var ly = cy + sin(sweep_angle) * 36.0\n"
            "draw_line(cx, cy, lx, ly, CYAN, 2.0)\n"
            "draw_circle_filled(lx, ly, 3.0, WHITE)\n"
            "\n"
            "// Center crosshair\n"
            "draw_line(cx - 8, cy, cx + 8, cy, CYAN, 1.0)\n"
            "draw_line(cx, cy - 8, cx, cy + 8, CYAN, 1.0)\n"
            "\n"
            "// Outlined Status Text\n"
            "draw_text_shadowed(cx - 30, cy + 52, \"ONLINE\", CYAN, BLACK, 1.0, 1.0)\n"
        });

        // 4. Pixel Character Walk Cycle
        presets.Add({
            "Pixel Walk Cycle",
            "Pixel",
            "A smooth 8-frame pixel art character walking animation.",
            "// Pixel Character Walk Cycle (8 Frames)\n"
            "set_grid_size(16, 16)\n"
            "clear_pixels(rgba(0, 0, 0, 0))\n"
            "\n"
            "var f = frame()\n"
            "var leg_offset = sin(f * 0.785) * 2.0\n"
            "var arm_offset = cos(f * 0.785) * 2.0\n"
            "var bob = abs(sin(f * 0.785)) * 1.0\n"
            "\n"
            "var skin = rgba(0.95, 0.8, 0.65, 1.0)\n"
            "var hair = rgba(0.4, 0.25, 0.15, 1.0)\n"
            "var shirt = rgba(0.2, 0.5, 0.9, 1.0)\n"
            "var pants = rgba(0.2, 0.2, 0.3, 1.0)\n"
            "\n"
            "// Head & Hair\n"
            "draw_pixel_rect(6, 2 - bob, 4, 2, hair)\n"
            "draw_pixel_rect(6, 4 - bob, 4, 3, skin)\n"
            "set_pixel(8, 5 - bob, BLACK) // Eye\n"
            "\n"
            "// Torso / Shirt\n"
            "draw_pixel_rect(6, 7 - bob, 4, 4, shirt)\n"
            "\n"
            "// Arms\n"
            "set_pixel(5, 8 - bob + arm_offset, shirt)\n"
            "set_pixel(10, 8 - bob - arm_offset, shirt)\n"
            "\n"
            "// Legs & Shoes\n"
            "draw_pixel_line(6, 11, 6 - leg_offset, 14, pants)\n"
            "draw_pixel_line(9, 11, 9 + leg_offset, 14, pants)\n"
            "set_pixel(6 - leg_offset, 15, BLACK)\n"
            "set_pixel(9 + leg_offset, 15, BLACK)\n"
        });

        // 5. Procedural Magic Fire Ball
        presets.Add({
            "Magic Fireball",
            "Vector",
            "Multi-layered glowing magical fireball with ember particles.",
            "// Magic Fireball with procedural aura\n"
            "var cx = get_width() * 0.5\n"
            "var cy = get_height() * 0.5\n"
            "var t = time()\n"
            "\n"
            "// Outer Flame Glow\n"
            "var outer_r = 38.0 + sin(t * 8.0) * 4.0\n"
            "draw_circle_filled(cx, cy, outer_r, rgba(1.0, 0.2, 0.05, 0.4))\n"
            "\n"
            "// Mid Orange Core\n"
            "var mid_r = 26.0 + cos(t * 6.0) * 3.0\n"
            "draw_circle_filled(cx, cy, mid_r, rgba(1.0, 0.6, 0.1, 0.7))\n"
            "\n"
            "// Inner Bright Core\n"
            "var inner_r = 14.0 + sin(t * 12.0) * 2.0\n"
            "draw_circle_filled(cx, cy, inner_r, rgba(1.0, 0.95, 0.6, 0.95))\n"
            "\n"
            "// Orbiting Embers\n"
            "for i = 0 to 4 {\n"
            "    var a = t * 4.0 + i * 1.256\n"
            "    var dist = 32.0 + sin(t * 5.0 + i) * 6.0\n"
            "    var ex = cx + cos(a) * dist\n"
            "    var ey = cy + sin(a) * dist\n"
            "    draw_circle_filled(ex, ey, 2.5, YELLOW)\n"
            "}\n"
        });

        return presets;
    }
};
