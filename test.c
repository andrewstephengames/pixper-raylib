#include <stdio.h>
#include <string.h>
#include <raylib.h>
#include <raymath.h>

#define NAME 1
#define PLAY 2
#define OPTIONS 3
#define STATS 4
#define QUIT 5

typedef struct
{
     Vector2 a, b;
} Vector2Pair;

Vector2 w = {
     .x = 1280,
     .y = 720,
};

typedef struct
{
     bool close, won;
     int score;
} Game;

Vector2Pair buttons[20];
Game game;

Vector2Pair DrawTextButton (const char *s, int textsize, Vector2 pos, int offset, Color bg, Color fg);
Vector2 CenterText (const char *s, int size, Vector2 pos);
void DrawBackground (float alpha);
Rectangle AddRect (Rectangle a, Rectangle b);
void DrawMenu (void);
void InitMenu (void);
void EndMenu (void);

int main (void)
{
     SetConfigFlags (FLAG_WINDOW_RESIZABLE);
     InitWindow (w.x, w.y, "Menu");
     SetTargetFPS(60);
     SetTraceLogLevel(LOG_ERROR);
     while (!WindowShouldClose() && !game.close)
     {
          w.x = GetRenderWidth();
          w.y = GetRenderHeight();
          BeginDrawing();
               ClearBackground (BLACK);
               DrawBackground (150);
               //InitMenu();
               EndMenu();
          EndDrawing();
          if (IsKeyPressed(KEY_Q))
               break;
          if (IsKeyPressed(KEY_F))
          {
               Vector2 mouse = GetMousePosition();
               printf ("Mouse: (%.f %.f)\n", mouse.x, mouse.y);
          }
     }
     CloseWindow();
     return 0;
}

Vector2 CenterText (const char *s, int size, Vector2 pos)
{
     Vector2 newpos = {
          .x = pos.x/2 - MeasureText (s, size)/2,
          .y = pos.y/2 - size/2,
     };
     return newpos;
}

Rectangle AddRect (Rectangle a, Rectangle b)
{
     Rectangle rect = {
          a.x + b.x,
          a.y + b.y,
          a.width + b.width,
          a.height + b.height,
     };
     return rect;
}

Vector2Pair DrawTextButton (const char *s, int textsize, Vector2 pos, int offset, Color bg, Color fg)
{
     Vector2 center = CenterText (s, textsize, pos);
     Font font = GetFontDefault();
     Vector2 rectsize = MeasureTextEx (font, s, textsize, w.x/200);
     DrawRectangle (center.x, center.y+offset, rectsize.x, rectsize.y, bg);
     DrawText (s, center.x, center.y+offset, textsize, fg);
     center.y += offset;
     Vector2Pair v = {
          center,
          Vector2Add(center, rectsize),
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

void DrawMenu (void)
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
     buttons[OPTIONS] = DrawTextButton (s, size, w, w.x*0.06f, bg, fg);
     if (input.x >= buttons[OPTIONS].a.x && input.x <= buttons[OPTIONS].b.x &&
          input.y >= buttons[OPTIONS].a.y && input.y <= buttons[OPTIONS].b.y)
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               //TODO: options menu
               printf ("Options\n");
     strcpy (s, "Stats");
     buttons[STATS] = DrawTextButton (s, size, w, w.x*0.12f, bg, fg);
     if (input.x >= buttons[STATS].a.x && input.x <= buttons[STATS].b.x &&
          input.y >= buttons[STATS].a.y && input.y <= buttons[STATS].b.y)
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               //TODO: stats menu
               printf ("Stats\n");
     strcpy (s, "Quit");
     buttons[QUIT] = DrawTextButton (s, size, w, w.x*0.18f, bg, fg);
     if (input.x >= buttons[QUIT].a.x && input.x <= buttons[QUIT].b.x &&
          input.y >= buttons[QUIT].a.y && input.y <= buttons[QUIT].b.y)
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               game.close = 1;
}

void InitMenu (void)
{
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
                    game.close = 1;
               if (game.close)
                    break;
     }
}

void EndMenu (void)
{
     int size = w.x/20;
     char s[30];
     game.won = 1;
     game.score = 0xFF;
     if (game.won)
     {
          strcpy (s, "You Won!");
          Vector2 mid = CenterText (s, size, w);
          DrawText (s, mid.x, mid.y/2, size, YELLOW);
          sprintf (s, "Score: %d", game.score);
          size = w.x/30;
          mid = CenterText (s, size, w);
          DrawText (s, mid.x, (mid.y/2)*1.5f, size, YELLOW);
     }
     else
     {
          strcpy (s, "You Lost!");
          Vector2 mid = CenterText (s, size, w);
          DrawText (s, mid.x, mid.y/2, size, RED);
          size = w.x/30;
          if (game.score == 0)
               strcpy (s, "The Enemy stole no apples!");
          if (game.score == 1)
               strcpy (s, "The Enemy stole one apple!");
          if (game.score >= 2)
               sprintf (s, "The Enemy stole %d apples!", game.score);
          mid = CenterText (s, size, w);
          DrawText (s, mid.x, (mid.y/2)*1.5f, size, RED);
     }
     Vector2 half = {
          w.x/2,
          w.y,
     };
     Vector2 most = {
          w.x + w.x/2,
          w.y,
     };
     size = w.x/20;
     Vector2 input = GetMousePosition();
     Color fg = YELLOW, bg = { 40, 40, 40, 120 };
     strcpy (s, "Play Again");
     buttons[PLAY] = DrawTextButton (s, size, half, w.x*0.10f, bg, fg);
     if (input.x >= buttons[PLAY].a.x && input.x <= buttons[PLAY].b.x &&
          input.y >= buttons[PLAY].a.y && input.y <= buttons[PLAY].b.y)
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               printf ("Play\n");
     strcpy (s, "Quit to Menu");
     buttons[QUIT] = DrawTextButton (s, size, most, w.x*0.10f, bg, fg);
     if (input.x >= buttons[QUIT].a.x && input.x <= buttons[QUIT].b.x &&
          input.y >= buttons[QUIT].a.y && input.y <= buttons[QUIT].b.y)
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               game.close = 1;
}
