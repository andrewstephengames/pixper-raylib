#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <raylib.h>

struct Coord
{
     float x, y;
};

struct Coord w = {
     .x = 800,
     .y = 600
};

struct Coord c = {
     .x = 0,
     .y = 300
};

int main ()
{
     int d = 1;
     InitWindow(w.x, w.y, "Pixper");
     SetTargetFPS(60);
     while (!WindowShouldClose())
     {
          c.x += GetFrameTime()*400*d;
          if (c.x >= w.x-100 || c.x <= 0)
               d *= -1;
          BeginDrawing();
               ClearBackground(BLACK);
               DrawText("Hello World", c.x, c.y, 20, BLUE);
          EndDrawing();
     }
     CloseWindow();
     return 0;
}
