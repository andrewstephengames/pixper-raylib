/*
     sql = "CREATE TABLE IF NOT EXISTS Players (Name TEXT PRIMARY KEY, Score INTEGER);" //Hard BOOLEAN)"
           "CREATE TABLE IF NOT EXISTS Obstacles (Playername TEXT, RNG REAL PRIMARY KEY);";
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <raylib.h>

typedef struct 
{
     int x, y;
} Window;

typedef struct
{
     Texture2D sprite;
     short x, y, speed, num, score;
} Entity;

Window w = {
     .x = 800,
     .y = 600
};

Entity a[50], b, cc[1], dd[2];
int health = 10;

Window CenterText (const char *s, int size)
{
     return (Window) {
          .x = w.x/2 - MeasureText (s, size)/2,
          .y = w.y/2 - size/2,
     };
}

int main (void)
{
     SetConfigFlags(FLAG_WINDOW_RESIZABLE);
     InitWindow(w.x, w.y, "Pixper");
     SetTargetFPS(60);
     while (!WindowShouldClose())
     {
          w.x = GetScreenWidth ();
          w.y = GetScreenHeight ();
          char s[] = "Hello World\0";
          int size = w.x / 20;
          Window mid = CenterText (s, size);
          
          BeginDrawing();
               ClearBackground(BLACK);
               //if (IsWindowMaximized())
               if (IsWindowResized ())
                    DrawText(s, mid.x, mid.y, size, BLUE);
               if (IsWindowResized() && IsWindowMaximized())
               {
                    printf ("(%d %d)\n", mid.x, mid.y);
                    DrawText(strchr(s, ' ')+1, mid.x, mid.y, size, BLUE);
               }
          EndDrawing();
          if (IsKeyPressed(KEY_Q))
               break;
     }
     CloseWindow();
     return 0;
}
