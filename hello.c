#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include <string.h>
#include <math.h>

#define PLAY 1
#define OPTIONS 2
#define QUIT 3

typedef struct
{
     Vector2 a, b;
} VectorPair;

Vector2 w = {
     .x = 1280,
     .y = 720,
};
VectorPair buttons[20];

Vector2 CenterText (const char *s, int size, Vector2 pos);
VectorPair DrawTextButton (const char *s, int size, Vector2 pos, int offset, Color bg, Color fg);
void DrawBackground (float alpha);
void InitMenu (void);
void DrawMenu (void);

int main ()
{
     InitMenu();
     return 0;
}

Vector2 CenterText (const char *s, int size, Vector2 pos)
{
     return (Vector2) {
          .x = pos.x/2 - MeasureText (s, size)/2,
          .y = pos.y/2 - size/2,
     };
}


VectorPair DrawTextButton (const char *s, int textsize, Vector2 pos, int offset, Color bg, Color fg)
{
     Vector2 center = CenterText (s, textsize, pos);
     Vector2 rect = {
          .x = pos.x/10,
          .y = pos.y/10,
     };
     size_t num = strlen(s);
     if (num <= 4)
          rect.x += MeasureText ("a", textsize);
     else while (num > strlen(s)/2)
          {
               rect.x += MeasureText ("a", textsize);
               --num;
          }
     DrawRectangle (center.x, center.y+offset, rect.x, rect.y, bg);
     center.x += MeasureText ("a", textsize)/4;
     DrawText (s, center.x, center.y+offset, textsize, fg);
     center.y += offset;
     VectorPair v = {
          .a = center,
          .b = Vector2Add (center, rect),
     };
     return v;
}

void DrawBackground (float alpha)
{
     Texture2D sprite = LoadTexture ("res/images/grass.png");
     Color c = WHITE;
     c.a = alpha;
     Rectangle rect = {
          .width = w.x,
          .height = w.y,
     };
     SetShapesTexture (sprite, rect);
     DrawRectangle (0, 0, w.x, w.y, c);
}

// TODO: highlighting / outlining of menu selection
void InitMenu (void)
{
     int size = w.x/10;
     char s[30];
     strcpy (s, "Pixper");
     Vector2 mid = CenterText (s, size, w);
     DrawText (s, mid.x, mid.y/2, size, YELLOW);
     size = w.x/20;
     Vector2 input = GetMousePosition();
     Color fg = YELLOW, bg = { 40, 40, 40, 120 };
     strcpy (s, "Play");
     buttons[PLAY] = DrawTextButton (s, size, w, 0, bg, fg);
     if (input.x >= buttons[PLAY].a.x && input.x <= buttons[PLAY].b.x &&
          input.y >= buttons[PLAY].a.y && input.y <= buttons[PLAY].b.y)
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               printf ("Play\n");
     strcpy (s, "Options");
     buttons[OPTIONS] = DrawTextButton (s, size, w, w.x*0.08f, bg, fg);
     if (input.x >= buttons[OPTIONS].a.x && input.x <= buttons[OPTIONS].b.x &&
          input.y >= buttons[OPTIONS].a.y && input.y <= buttons[OPTIONS].b.y)
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               printf ("Options\n");
     strcpy (s, "Quit");
     buttons[QUIT] = DrawTextButton (s, size, w, w.x*0.16f, bg, fg);
     if (input.x >= buttons[QUIT].a.x && input.x <= buttons[QUIT].b.x &&
          input.y >= buttons[QUIT].a.y && input.y <= buttons[QUIT].b.y)
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               printf ("Quit\n");
}

void DrawMenu (void)
{
     SetConfigFlags (FLAG_WINDOW_RESIZABLE);
     InitWindow (w.x, w.y, "Menu");
     SetTargetFPS(60);
     SetTraceLogLevel(LOG_ERROR);
     while (!WindowShouldClose())
     {
          w.x = GetRenderWidth();
          w.y = GetRenderHeight();
          BeginDrawing();
               ClearBackground (BLACK);
               DrawBackground (150);
               DrawMenu();
          EndDrawing();
               if (IsKeyPressed(KEY_Q))
                    break;
     }
     CloseWindow();
}
