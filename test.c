#include <stdio.h>
#include <string.h>
#include <raylib.h>
#include <raymath.h>

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

typedef struct
{
     bool close, won;
     int score;
} Game;

VectorPair buttons[20];
Game game;

VectorPair DrawTextButton (const char *s, int textsize, Vector2 pos, int offset, Color bg, Color fg);
Vector2 CenterText (const char *s, int size, Vector2 pos);
void DrawBackground (float alpha);
void InitMenu (void);

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
               InitMenu();
          EndDrawing();
               if (IsKeyPressed(KEY_Q))
                    break;
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

void InitMenu (void)
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
     strcpy (s, "Play");
     buttons[PLAY] = DrawTextButton (s, size, half, w.x*0.10f, bg, fg);
     if (input.x >= buttons[PLAY].a.x && input.x <= buttons[PLAY].b.x &&
          input.y >= buttons[PLAY].a.y && input.y <= buttons[PLAY].b.y)
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               printf ("Play\n");
     strcpy (s, "Quit");
     buttons[QUIT] = DrawTextButton (s, size, most, w.x*0.10f, bg, fg);
     if (input.x >= buttons[QUIT].a.x && input.x <= buttons[QUIT].b.x &&
          input.y >= buttons[QUIT].a.y && input.y <= buttons[QUIT].b.y)
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               game.close = 1;
}