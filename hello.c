#include <stdio.h>
#include <raylib.h>
#include <string.h>
#include <math.h>

typedef struct
{
     int x, y;
} Window;

Window w = {
     .x = 1280,
     .y = 720,
};

Window CenterText (const char *s, int size);

int main ()
{
     SetConfigFlags (FLAG_WINDOW_RESIZABLE);
     InitWindow (w.x, w.y, "Menu");
     SetTargetFPS(60);
     while (!WindowShouldClose())
     {
          w.x = GetRenderWidth();
          w.y = GetRenderHeight();
          char s[] = "Pixper";
          int size = w.x/10, rectsize;
          Window mid = CenterText (s, size);
          BeginDrawing();
               ClearBackground (BLACK);
               DrawText (s, mid.x, mid.y/2, size, GREEN);
               size = w.x/30;
               strcpy (s, "Play");
               Color c = {
                    .r = 51,
                    .g = 51,
                    .b = 51,
                    .a = 230,
               };
               rectsize = w.x/2 - MeasureText (s, size);
               DrawRectangle (rectsize, w.y/2-size, 200, 100, c);
               DrawText (s, w.x/2 - MeasureText (s, size)+strlen(s)*10, w.y/2-size+100/4, size, YELLOW);
               strcpy (s, "Options");
               DrawRectangle (rectsize, w.y/2-size+120, 200, 100, c);
               DrawText (s, w.x/2 - MeasureText (s, size)+strlen(s)*10, w.y/2-size+100/4+120, size, YELLOW);
               if (IsKeyPressed(KEY_Q))
                    break;
               //if (IsKeyPressed(KEY_F))
                    //printf ("\nx: %d ~ %d\ny: %d ~ %d", GetMouseX(), play.x/4, GetMouseY(), play.y/4);
          EndDrawing();
     }
     CloseWindow();
     return 0;
}

Window CenterText (const char *s, int size)
{
     return (Window) {
          .x = w.x/2 - MeasureText (s, size)/2,
          .y = w.y/2 - size/2,
     };
}
