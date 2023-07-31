//TODO: add feature to buy an apple reveal using the score acquired
#include <stdio.h>
#include <stdlib.h>
#include <string.h> //for strcat, strcpy, strlen and strcmp
#include <math.h> //for sqrt and pow (collisions)
#include <time.h> //for clock() (random seed)
#include <sqlite3.h> //for db
#include <raylib.h>
#include <raymath.h> // for Vector2Add

#define PLATFORM_DESKTOP
#define E_SZ 100
#define PLAY 1
#define OPTIONS 2
#define STATS 3
#define QUIT 4
#define SFX 5
#define MUS 6
#define DIFF 7
#define BACK 8

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
     float x, y, speed;
     Texture2D sprite;
     int num;
     bool used;
     char name[30];
} Entity;

typedef struct
{
     bool close, mutemusic, mutesound, difficulty, won, start;
     int score, health, delay, pausenum;
     float sfx, music;
} Game;

typedef struct
{
     int apple, bomb, score;
     struct
     {
          struct
          {
               int player, enemy;
          } inc, dec;
     } health;
     struct
     {
          struct
          {
               float inc, dec;
          } player, enemy;
     } speed;
     struct
     {
          float a, b;
     } reveal;
} Difficulty;

typedef struct
{
     int player, enemy, apple;
     struct
     {
          int p, e;
     } bomb;
} Collision;

Entity player[2], background, apple[E_SZ], grass[E_SZ], tree[E_SZ], bomb[E_SZ];
Sound sound[4];
Music music[4];
Game game = {
     .difficulty = 0,
     .health = 10
};
Difficulty diff;
Collision col;
Rectangle buttons[10];


sqlite3 *db;
char *err_msg, sql[500], entries[5][3][50];
int rc, entrynum;

// all functions used
// FIXME: refactoring
int callback(void *NotUsed, int argc, char **argv, char **azColName);
void Commandline (int argc, char **argv);
bool IsCollision (Entity *a, Entity *b, float c);
void SetDifficulty (bool difficulty);
Vector2 CenterText (const char *s, int size, Vector2 pos);
Rectangle DrawTextButton (const char *s, int size, Vector2 pos, int offset, Color bg, Color fg);
void DrawBackground (float alpha, bool usesprite);
bool IsAnyKeyPressed (void);
void DrawMenu (void);
void InitMenu (void);
void UpdateAudio (void);
void DrawPauseButton (void);
void StatsMenu (void);
void PauseMenu (void);
void OptionsMenu (void);
void EndMenu (void);
void PlayerMovement (void);
void EnemyMovement (void);
void InitEntities (void);
void GenerateEntities (void);
void DrawEntities (void);
void CalcCollisions (void);
void DrawHUD (void);
void Gameplay (void);

int main (int argc, char **argv)
{
     SetConfigFlags(FLAG_WINDOW_RESIZABLE);
     InitWindow(w.x, w.y, "Pixper");
     InitAudioDevice();
     SetTargetFPS(60);
     SetTraceLogLevel(LOG_ERROR);
     SetRandomSeed(clock());
     game.sfx = 0.10f;
     game.music = 0.25f;
     strcpy (player[0].name, "Player");

     rc = sqlite3_open ("./res/db/stats.db", &db);
     strcpy (sql, "CREATE TABLE IF NOT EXISTS Players (Name TEXT PRIMARY KEY, Score INTEGER, Difficulty BOOLEAN);"
           "CREATE TABLE IF NOT EXISTS Obstacles (Playername TEXT, RNG INTEGER PRIMARY KEY, Name TEXT);");
     
     rc = sqlite3_exec(db, sql, 0, 0, &err_msg); //update db
     if (rc != SQLITE_OK)
     {
          fprintf (stderr, "Cannot open the database: %s\n", sqlite3_errmsg(db));
          sqlite3_close(db);
          return 1;
     }
     Commandline(argc, argv);
     InitMenu();
     UnloadTexture (player[0].sprite);
     UnloadTexture (background.sprite);
     for (int i = 1; i < 100; i++)
          if (grass[i].num || apple[i].num || bomb[i].num || tree[i].num)
          {
               UnloadTexture (grass[i].sprite);
               UnloadTexture (apple[i].sprite);
               UnloadTexture (bomb[i].sprite);
               UnloadTexture (tree[i].sprite);
          }
     StopMusicStream(music[0]);
     CloseAudioDevice();
     CloseWindow();
     if (rc != SQLITE_OK)
     {
          fprintf (stderr, "Failed to select data\n");
          fprintf (stderr, "SQL Error: %s\n", err_msg);
          sqlite3_close(db);
          return 1;
     }
     sqlite3_close(db);
     return 0;
}

int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
     (void) NotUsed;
     int k = 0;
     for (int i = 0; i < argc; i++)
     {
          printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
          if (argv[i])
               strcpy (entries[entrynum][k++], argv[i]);
     }
     for (int i = 0; i < k; i++)
          printf ("entries[%d][%d] = %s\n", entrynum, i, entries[entrynum][i]);
     printf("\n");
     entrynum++;
     return 0;
}

void Commandline (int argc, char **argv)
{
     // command-line args
     for (int i = 1; i < argc && argc > 1; i++)
     {
          if (strcmp (argv[i], "mutemusic") == 0)
          {
               printf ("arg[%d] == %s\n", argc, argv[i]);
               PauseMusicStream(music[0]);
               game.mutemusic = 1;
          }
          if (strcmp (argv[i], "cleardb") == 0)
          {
               printf ("arg[%d] == %s\n", argc, argv[i]);
               // truncate the res/db/stats.db file
               FILE *f = fopen ("./res/db/stats.db", "w");
               fclose(f);
          }
     }
}

bool IsCollision (Entity *a, Entity *b, float c)
{
     float d = sqrt((pow(b->x-a->x, 2) + pow(b->y-a->y, 2)));
     return d < c; //distance has to be smaller than collision coefficient
}

void SetDifficulty (bool difficulty)
{
     int a, t, b;
     if (difficulty)
     {
          a = 25;
          t = 40;
          b = 20;
          player[0].speed = 2.5f;
          player[1].speed = 2.5f;
          game.health = 5;
          // Interval of apple percentage consumed for the hidden apples to be revealed
          diff.reveal.a = 0.75f;
          diff.reveal.b = 0.85f;
          // the score increase determined by the difficulty
          diff.score = 1;
          // health increase when the player encounters an apple
          diff.health.inc.player = 1;
          // health decrease when the player hits the enemy
          diff.health.dec.enemy = 5;
          // health decrease when the player hits a bomb
          diff.health.dec.player = 10;
          // speed decrease when the player encounters the enemy
          diff.speed.player.dec = 0.005f;
          // speed increase when the enemy encounters a bomb
          diff.speed.enemy.inc = 0.20f;
          // speed decrease when the player encounters a bomb
          diff.speed.player.dec = 0.20f;
          // speed increase when the player encounters an apple
          diff.speed.player.inc = 0.025f;
          // Collision with an apple
          col.apple = 5;
          // Player collision with a bomb
          col.bomb.p = 30;
          // Enemy collision with a bomb
          col.bomb.e = 30;
          // Player collision with the enemy
          col.enemy = 30;
     }
     else
     {
          a = 20;
          t = 50;
          b = 30;
          player[0].speed = 3.5f;
          player[1].speed = 1.5f;
          game.health = 10;
          // Interval of apple percentage consumed for the hidden apples to be revealed
          diff.reveal.a = 0.60f;
          diff.reveal.b = 0.90f;
          // the score increase determined by the difficulty
          diff.score = 1;
          // health increase when the player encounters an apple
          diff.health.inc.player = 2;
          // health decrease when the player hits the enemy
          diff.health.dec.enemy = 2;
          // health decrease when the player hits a bomb
          diff.health.dec.player = 3;
          // speed decrease when the player encounters the enemy
          diff.speed.player.dec = 0.001f;
          // speed increase when the enemy encounters a bomb
          diff.speed.enemy.inc = 0.10f;
          // speed decrease when the player encounters a bomb
          diff.speed.player.dec = 0.10f;
          // speed increase when the player encounters an apple
          diff.speed.player.inc = 0.05f;
          // Collision with an apple
          col.apple = 15;
          // Player collision with a bomb
          col.bomb.p = 15;
          // Enemy collision with a bomb
          col.bomb.e = 15;
          // Player collision with the enemy
          col.enemy = 15;
     }
     apple[0].num = GetRandomValue (w.y/a, w.x/a);
     tree[0].num = GetRandomValue (w.y/t, w.x/t);
     grass[0].num = GetRandomValue (w.y/t, w.x/t);
     bomb[0].num = GetRandomValue (w.y/b, w.x/b);
}

Vector2 CenterText (const char *s, int size, Vector2 pos)
{
     Vector2 newpos = {
          .x = pos.x/2 - MeasureText (s, size)/2,
          .y = pos.y/2 - size/2,
     };
     return newpos;
}

Rectangle DrawTextButton (const char *s, int textsize, Vector2 pos, int offset, Color bg, Color fg)
{
     Vector2 center = CenterText (s, textsize, pos);
     Font font = GetFontDefault();
     Vector2 rectsize = MeasureTextEx (font, s, textsize, w.x/200);
     DrawRectangle (center.x, center.y+offset, rectsize.x, rectsize.y, bg);
     DrawText (s, center.x, center.y+offset, textsize, fg);
     center.y += offset;
     Rectangle rect = { center.x, center.y, rectsize.x, rectsize.y };
     return rect;
}

void DrawBackground (float alpha, bool usesprite)
{
     Texture2D sprite = LoadTexture ("./res/images/grass.png");
     Color c = WHITE;
     c.a = alpha;
     Rectangle rect = {
          .width = w.x,
          .height = w.y,
     };
     if (usesprite)
          SetShapesTexture (sprite, rect);
     DrawRectangle (0, 0, w.x, w.y, c);
}

bool IsAnyKeyPressed(void)
{
    bool keyPressed = false;
    int key = GetKeyPressed();

    if ((key >= 32) && (key <= 126)) keyPressed = true;

    return keyPressed;
}

void DrawMenu (void)
{
     int size = w.x/10;
     char s[30];
     strcpy (s, "Pixper");
     Vector2 mid = CenterText (s, size, w);
     DrawText (s, mid.x, mid.y/2-w.x/24, size, YELLOW);
     size = w.x/20;
     Vector2 input = GetMousePosition();
     Color fg = YELLOW, bg = { 40, 40, 40, 120 };
     strcpy (s, "Play");
     buttons[PLAY] = DrawTextButton (s, size, w, 0, bg, fg);
     if (CheckCollisionPointRec (input, buttons[PLAY]))
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               Gameplay ();
     strcpy (s, "Options");
     buttons[OPTIONS] = DrawTextButton (s, size, w, w.x*0.06f, bg, fg);
     if (CheckCollisionPointRec (input, buttons[OPTIONS]))
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               OptionsMenu();
     strcpy (s, "Stats");
     buttons[STATS] = DrawTextButton (s, size, w, w.x*0.12f, bg, fg);
     if (CheckCollisionPointRec (input, buttons[STATS]))
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               StatsMenu();
     strcpy (s, "Quit");
     buttons[QUIT] = DrawTextButton (s, size, w, w.x*0.18f, bg, fg);
     if (CheckCollisionPointRec (input, buttons[QUIT]))
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
               game.close = 1;
}

void InitMenu (void)
{
     strcpy (sql, "SELECT Name, Score, Difficulty FROM Players ORDER BY Score DESC LIMIT 5");
     rc = sqlite3_exec (db, sql, callback, 0, &err_msg); 
     int lc = 0, fc = 0;
     bool mouse = false;
     char name[20] = "\0";
     Rectangle textbox = {
          w.x/2 - w.x/12,
          w.y/2 - w.x/12,
          w.x/6,
          w.x/24,
     };
     Color fg = YELLOW, bg = { 40, 40, 40, 120 };
     while (!WindowShouldClose())
     {
          w.x = GetRenderWidth();
          w.y = GetRenderHeight();
          textbox = (Rectangle) {
               w.x/2 - w.x/12,
               w.y/2 - w.x/12,
               w.x/6,
               w.x/24,
          };
          Vector2 input = GetMousePosition();
          mouse = CheckCollisionPointRec (input, textbox);
          if (mouse)
          {
               SetMouseCursor (MOUSE_CURSOR_IBEAM);
               int key = GetCharPressed();
               while (key > 0)
               {
                    if (key >= 32 && key <= 125 && lc < (int)sizeof (name)-1)
                    {
                         name[lc] = (char) key;
                         name[++lc] = '\0';
                    }
                    key = GetCharPressed();
               }
               if (IsKeyPressed (KEY_BACKSPACE))
               {
                    lc--;
                    if (lc < 0) lc = 0;
                    name[lc] = '\0';
               }
          }
          else SetMouseCursor (MOUSE_CURSOR_DEFAULT);
          if (mouse) fc++;
          else fc = 0;
          BeginDrawing();
               ClearBackground (BLACK);
               DrawBackground (180, 1);
               DrawRectangleRec (textbox, bg);
               //if (mouse) DrawRectangleLinesEx (textbox, w.x/500, RED);
               //else DrawRectangleLinesEx (textbox, w.x/500, DARKGRAY);
               if (mouse) bg.a = 60;
               else bg.a = 120;
               DrawText (name, (int) textbox.x+w.x/200, textbox.y, w.x/25, fg);
               if (mouse)
                    if (lc < (int) sizeof (name))
                         if ((fc/20)%2 == 0)
                              DrawText ("|", textbox.x+w.x/200+MeasureText(name, w.x/25), textbox.y, w.x/25, fg);
               if (IsKeyPressed (KEY_ENTER))
                   strcpy (player[0].name, name);
               DrawMenu();
          EndDrawing();
               if (IsKeyPressed(KEY_Q))
                    game.close = 1;
               if (game.close)
                    break;
     }
}

void DrawPauseButton (void)
{
     Texture2D sprite = LoadTexture ("./res/images/pause-button");
     Rectangle rect = {
          .width = 32,
          .height = 32,
     };
     Color c = WHITE;
     c.a = 0;
     SetShapesTexture (sprite, rect);
     DrawRectangle (w.x-33, 1, 32, 32, c);
     Vector2 input = GetMousePosition();
     if (CheckCollisionPointRec (input, rect))
          PauseMenu();
}

void UpdateAudio (void)
{
     size_t soundsize = sizeof(sound) / sizeof (Sound) - 1;
     for (size_t i = 1; i <= soundsize; i++)
          SetSoundVolume(sound[i], game.sfx);
     size_t musicindex, musicsize;
     musicsize = sizeof(music) / sizeof(Music);
     musicindex = GetRandomValue (1, musicsize-1);
     music[0] = music[musicindex];
     SetMusicVolume (music[0], game.music);
     PlayMusicStream (music[0]);
}

void StatsMenu (void)
{
     while (!WindowShouldClose())
     {
          w.x = GetRenderWidth();
          w.y = GetRenderHeight();
          int size = w.x/20;
          char s[120], d[10];
          Vector2 half = { w.x/2, w.y/2 };
          Color fg = YELLOW, bg = { 40, 40, 40, 150 };
          BeginDrawing();
               ClearBackground (BLACK);
               DrawBackground (180, 1);
               strcpy (s, "Stats"); // menu title
               Vector2 mid = CenterText (s, size, w);
               DrawText (s, mid.x, w.y*0.10f, size, YELLOW);
               strcpy (s, "Latest Matches");
               buttons[PLAY] = DrawTextButton (s, size/2, half, 0, bg, fg);
               int i = 0;  
               for (i = 0; i < entrynum; i++)
               {
                    if (atoi(entries[i][2]) == 0)
                         strcpy (d, "easy");
                    else strcpy (d, "hard");
                    if (strcmp (entries[i][0], "Enemy") != 0)
                         sprintf (s, "%d. %s collected %d apples on %s difficulty.",
                                  i+1, entries[i][0], atoi(entries[i][1]), d);
                    else
                         sprintf (s, "%d. %s stole %d apples on %s difficulty.",
                                   i+1, entries[i][0], atoi(entries[i][1]), d);
                    DrawText (s, half.x/2, w.y/5+w.y*0.10f*(i+1), size/2, YELLOW);
                    //w.y/5 is the offset, w.y*0.10f*(i+1) is the distance between each entry
               }
               if (i == 0)
               {
                    strcpy (s, "No matches were found in the database.");
                    DrawText (s, half.x/2, w.y/5+w.y*0.10f, size/2, YELLOW);
                    strcpy (s, "Play some more to see your matches here!");
                    DrawText (s, half.x/2, w.y/5+w.y*0.20f, size/2, YELLOW);
               }
               Vector2 input = GetMousePosition();
               strcpy (s, "Back");
               buttons[BACK] = DrawTextButton (s, size, w, w.x*0.18f, bg, fg);
               if (CheckCollisionPointRec (input, buttons[BACK]))
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                         break;
          EndDrawing();
          if (IsKeyPressed (KEY_Q))
               game.close = 1;
          if (game.close) 
               break;
     }
}

void PauseMenu (void)
{
     game.pausenum++;
     while (!WindowShouldClose())
     {
          w.x = GetRenderWidth();
          w.y = GetRenderHeight();
          int size = w.x/20;
          char s[30];
          Vector2 input = GetMousePosition();
          Color fg = YELLOW, bg = { 40, 40, 40, 2 };
          BeginDrawing();
               if (game.pausenum % 2 == 0)
               {
                    ClearBackground (BLACK);
                    bg.a = 150;
                    DrawBackground (180, 0);
               }
               
               strcpy (s, "Paused!");
               Vector2 mid = CenterText (s, size, w);
               DrawText (s, mid.x, mid.y/2, size, YELLOW);
               strcpy (s, "Resume");
               buttons[PLAY] = DrawTextButton (s, size, w, 0, bg, fg);
               if (CheckCollisionPointRec (input, buttons[PLAY]))
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                         break;
               strcpy (s, "Options");
               buttons[OPTIONS] = DrawTextButton (s, size, w, w.x*0.06f, bg, fg);
               if (CheckCollisionPointRec (input, buttons[OPTIONS]))
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                    {
                         OptionsMenu();
                         game.pausenum++;
                         UpdateAudio(); // audio must be updated when the game is resumed
                    }
               strcpy (s, "Quit to Menu");
               buttons[QUIT] = DrawTextButton (s, size, w, w.x*0.12f, bg, fg);
               if (CheckCollisionPointRec (input, buttons[QUIT]))
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                         InitMenu();
               
          EndDrawing();
          if (IsKeyPressed(KEY_Q))
               game.close = 1;
          if (game.close)
               break;
     }
}

void OptionsMenu (void)
{
     while (!WindowShouldClose())
     {
          w.x = GetRenderWidth();
          w.y = GetRenderHeight();
          int size = w.x/10;
          char s[30];
          size = w.x/20;
          Vector2 input = GetMousePosition();
          Color fg = YELLOW, bg = { 40, 40, 40, 150 };
          BeginDrawing();
               ClearBackground (BLACK);
               DrawBackground (180, 1);
               
               strcpy (s, "Options");
               Vector2 mid = CenterText (s, size, w);
               DrawText (s, mid.x, mid.y/2, size, YELLOW);
               strcpy (s, "Sfx: ");
               Vector2 half = { w.x/2, w.y }, slider[10][2];
               buttons[SFX] = DrawTextButton (s, size, half, 0, bg, fg);
               if (CheckCollisionPointRec (input, buttons[SFX]))
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                         printf ("SFX\n");
               slider[SFX][0] = (Vector2) { 0.35f * w.x, w.y/2 };
               slider[SFX][1] = (Vector2) { 0.80f * w.x, w.y/2 };
               DrawLineEx (slider[SFX][0], slider[SFX][1], w.x/100, YELLOW);
               int sliderlen = slider[SFX][1].x - slider[SFX][0].x + 1;
               int offset = slider[SFX][0].x;
               Vector2 circle[2];
               if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                    if (CheckCollisionPointLine (input, slider[SFX][0], slider[SFX][1], w.x/100))
                    {
                         circle[0].x = input.x;
                         game.sfx = (input.x - offset) / sliderlen;
                    }
               circle[0].x = offset + game.sfx * sliderlen;
               circle[0].y = slider[SFX][0].y;
               DrawCircleV (circle[0], w.x/75, YELLOW);
               input = GetMousePosition();
               strcpy (s, "Music: ");
               buttons[MUS] = DrawTextButton (s, size, half, w.x*0.06f, bg, fg);
               if (CheckCollisionPointRec (input, buttons[MUS]))
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                         printf ("Music\n");
               slider[MUS][0] = (Vector2) { 0.35f * w.x, w.y/2+w.x*0.06f };
               slider[MUS][1] = (Vector2) { 0.80f * w.x, w.y/2+w.x*0.06f };
               DrawLineEx (slider[MUS][0], slider[MUS][1], w.x/100, YELLOW);
               sliderlen = slider[MUS][1].x - slider[MUS][0].x + 1;
               offset = slider[MUS][0].x;
               if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
               if (CheckCollisionPointLine (input, slider[MUS][0], slider[MUS][1], w.x/100))
                    {
                         circle[1].x = input.x;
                         game.music = (input.x - offset) / sliderlen;
                    }
               circle[1].x = offset + game.music * sliderlen;
               circle[1].y = slider[MUS][0].y;
               DrawCircleV (circle[1], w.x/75, YELLOW);
               strcpy (s, "Difficulty: ");
               if (game.difficulty)
                    strcat (s, "Hard");
               else strcat (s, "Easy");
               buttons[DIFF] = DrawTextButton (s, size, w, w.x*0.12f, bg, fg);
               if (CheckCollisionPointRec (input, buttons[DIFF]))
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                         //change difficulty so long as the game hasn't started
                         if (!game.start)
                              game.difficulty = !game.difficulty;
                         
               strcpy (s, "Back");
               buttons[BACK] = DrawTextButton (s, size, w, w.x*0.18f, bg, fg);
               if (CheckCollisionPointRec (input, buttons[BACK]))
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                         break;
          EndDrawing();
          if (IsKeyPressed(KEY_Q))
               game.close = 1;
          if (IsKeyPressed(KEY_F))
               printf ("Mouse: (%.f %.f)\n", GetMousePosition().x, GetMousePosition().y);
          if (game.close)
               break;
     }
}

void EndMenu (void)
{
     while (!WindowShouldClose())
     {
          w.x = GetRenderWidth();
          w.y = GetRenderHeight();
          int size = w.x/20;
          char s[30];
          Vector2 half = {
               w.x/2,
               w.y,
          };
          Vector2 most = {
               w.x + w.x/2,
               w.y,
          };
          Vector2 input = GetMousePosition();
          Color fg = YELLOW, bg = { 40, 40, 40, 120 };
          BeginDrawing();
               //ClearBackground (BLACK);
               //DrawBackground (255, 0);
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
               strcpy (s, "Play Again");
               buttons[PLAY] = DrawTextButton (s, size, half, w.x*0.10f, bg, fg);
               if (CheckCollisionPointRec (input, buttons[PLAY]))
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                         Gameplay();
               strcpy (s, "Quit to Menu");
               buttons[QUIT] = DrawTextButton (s, size, most, w.x*0.10f, bg, fg);
               if (CheckCollisionPointRec (input, buttons[QUIT]))
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                         InitMenu();
               
          EndDrawing();
          if (IsKeyPressed(KEY_Q))
               game.close = 1;
          if (game.close)
               break;
     }
}

void PlayerMovement (void)
{
     bool k[4] = {
          IsKeyDown(KEY_W) || IsKeyDown(KEY_UP),
          IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT),
          IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN),
          IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT),
     };
     if (k[0])
          player[0].y -= player[0].speed;
     if (k[1])
          player[0].x -= player[0].speed;
     if (k[2])
          player[0].y += player[0].speed;
     if (k[3])
          player[0].x += player[0].speed;
     if (player[0].x <= 5 && player[0].x != -1)
          player[0].x = 5;
     if (player[0].y <= 5 && player[0].y != -1)
          player[0].y = 5;
     if (player[0].x >= w.x-32)
          player[0].x = w.x-32;
     if (player[0].y >= w.y-32)
          player[0].y = w.y-32;
}

void EnemyMovement (void)
{
     if (player[1].x < player[0].x)
          player[1].x += player[1].speed;
     else player[1].x -= player[1].speed;

     if (player[1].y < player[0].y)
          player[1].y += player[1].speed;
     else player[1].y -= player[1].speed;

     if (player[1].x <= 5 && player[1].x != -1)
          player[1].x = 5;
     if (player[1].y <= 5 && player[1].y != -1)
          player[1].y = 5;
     if (player[1].x >= w.x-32)
          player[1].x = w.x-32;
     if (player[1].y >= w.y-32)
          player[1].y = w.y-32;
}

void InitEntities (void)
{
     for (int i = 1; i <= apple[0].num; i++)
     {
          apple[i].sprite = LoadTexture ("./res/images/reset.png");
          apple[i].x = -5;
          apple[i].y = -5;
     }
     for (int i = 1; i <= grass[0].num; i++)
     {
          grass[i].sprite = LoadTexture ("./res/images/reset.png");
          grass[i].x = -5;
          grass[i].y = -5;
     }
     for (int i = 1; i <= tree[0].num; i++)
     {
          tree[i].sprite = LoadTexture ("./res/images/reset.png");
          tree[i].x = -5;
          tree[i].y = -5;
     }
     for (int i = 1; i <= bomb[0].num; i++)
     {
          bomb[i].sprite = LoadTexture ("./res/images/reset.png");
          bomb[i].x = -5;
          bomb[i].y = -5;
     }
     // player[0] is the main character
     UnloadTexture(player[0].sprite);
     // player[1] is the enemy
     UnloadTexture(player[1].sprite);
}

void GenerateEntities (void)
{
     //Designate the first value of obstacle arrays to the number of obstacles
     game.score = 0; // The score must reset for each resize
     SetDifficulty (game.difficulty);
     for (int i = 1; i <= apple[0].num; i++)
     {
          apple[i].sprite = LoadTexture ("./res/images/apple.png");
          apple[i].x = GetRandomValue (5, w.x-32);
          apple[i].y = GetRandomValue (5, w.y-32);
     }
     for (int i = 1; i <= grass[0].num; i++)
     {
          grass[i].sprite = LoadTexture ("./res/images/grasstile.png");
          grass[i].x = GetRandomValue (5, w.x-32);
          grass[i].y = GetRandomValue (5, w.y-32);
     }
     for (int i = 1; i <= tree[0].num; i++)
     {
          tree[i].sprite = LoadTexture ("./res/images/tree.png");
          tree[i].x = GetRandomValue (5, w.x-32);
          tree[i].y = GetRandomValue (5, w.y-32);
     }
     for (int i = 1; i <= bomb[0].num; i++)
     {
          bomb[i].sprite = LoadTexture ("./res/images/bomb.png");
          bomb[i].x = GetRandomValue (5, w.x-32);
          bomb[i].y = GetRandomValue (5, w.y-32);
     }
     // player[0] is the main character
     player[0].sprite = LoadTexture ("./res/images/player-black.png");
     //player[0].speed = 3.5f;
     player[0].num = 1; //might add support for multiple players
     player[0].x = GetRandomValue (5, w.x-32);
     player[0].y = GetRandomValue (5, w.y-32);
     // player[1] is the enemy
     player[1].sprite = LoadTexture ("./res/images/enemy-black.png");
     //player[1].speed = 1.5f;
     player[1].num = 1; //might add support for multiple enemies
     player[1].x = GetRandomValue (5, w.x-32);
     player[1].y = GetRandomValue (5, w.y-32);
     strcpy (player[1].name, "Enemy");
     // prevent the enemy from spawning on top of, or near the player
     while (IsCollision (&player[0], &player[1], 50))
     {
          player[1].x = GetRandomValue (5, w.x-32);
          player[1].y = GetRandomValue (5, w.y-32);
     }
     // prevent bombs generating on top of, or near the player
     for (int i = 1; i <= bomb[0].num; i++)
     while (IsCollision(&player[0], &bomb[i], 50))
     {
          player[0].x = GetRandomValue (5, w.x-32);
          player[0].y = GetRandomValue (5, w.y-32);
     }
}

void DrawEntities (void)
{
     for (int i = 1; i <= apple[0].num; i++)
          if (apple[i].x && apple[i].y)
               DrawTexture (apple[i].sprite, apple[i].x, apple[i].y, WHITE);
     for (int i = 1; i <= grass[0].num; i++)
          if (grass[i].x && grass[i].y)
               DrawTexture (grass[i].sprite, grass[i].x, grass[i].y, WHITE);
     for (int i = 1; i <= tree[0].num; i++)
          if (tree[i].x && tree[i].y)
               DrawTexture (tree[i].sprite, tree[i].x, tree[i].y, WHITE);
     for (int i = 1; i <= bomb[0].num; i++)
          if (bomb[i].x && bomb[i].y)
               DrawTexture (bomb[i].sprite, bomb[i].x, bomb[i].y, WHITE);
     if (game.score > apple[0].num * diff.reveal.a && game.score < apple[0].num * diff.reveal.b)
     {
          char s[] = "Hidden apples have been revealed!\0";
          int size = w.x/20;
          Vector2 mid = CenterText (s, size, w);
          DrawText (s, mid.x, mid.y, size, YELLOW);
          for (int i = 1; i <= apple[0].num; i++)
               if (apple[i].x && apple[i].y)
                    DrawTexture (apple[i].sprite, apple[i].x, apple[i].y, WHITE);
     }
     DrawTexture (player[0].sprite, player[0].x, player[0].y, WHITE);
     DrawTexture (player[1].sprite, player[1].x, player[1].y, WHITE);
}

void CalcCollisions (void)
{
     char buffer[101];
     for (int i = 1; i <= apple[0].num; i++)
          //if the player collides with an apple
          if (IsCollision (&player[0], &apple[i], col.apple) && !apple[i].used)
          {
               game.score += diff.score;
               game.health += diff.health.inc.player;
               player[0].speed += diff.speed.player.inc;
               apple[i].used = 1;
               apple[i].sprite = LoadTexture ("./res/images/grasstile.png");
               PlaySound (sound[2]); //eat
               int rng = (apple[i].x + apple[i].y)/2;
               sprintf (buffer, "INSERT OR REPLACE INTO Obstacles VALUES ('%s', '%d', '%s');", player[0].name, rng, "Apple");
               strcpy (sql, buffer);
               rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
          }
     for (int i = 1; i <= bomb[0].num; i++)
     {
          //if the player collides with a bomb
          if (IsCollision (&player[0], &bomb[i], col.bomb.p) && !bomb[i].used)
          {
               PlaySound (sound[3]); //boom
               PlaySound (sound[1]); //oof
               game.health -= diff.health.dec.player;
               player[0].speed -= diff.speed.player.dec;
               if (player[0].speed < 0.0f)
                    player[0].speed = 0.0f;
               bomb[i].used = 1;
               bomb[i].sprite = LoadTexture ("./res/images/bombtile.png");
               int rng = (bomb[i].x + bomb[i].y)/2;
               sprintf (buffer, "INSERT OR REPLACE INTO Obstacles VALUES ('%s', '%d', '%s');", player[0].name, rng, "Bomb");
               strcpy (sql, buffer);
               rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
          }
          //if the enemy collides with a bomb
          if (IsCollision(&player[1], &bomb[i], col.bomb.e) && !bomb[i].used)
          {
               PlaySound (sound[3]); //boom
               player[1].speed += diff.speed.enemy.inc;
               bomb[i].used = 1;
               bomb[i].sprite = LoadTexture ("./res/images/bombtile.png");
               int rng = (bomb[i].x + bomb[i].y)/2;
               sprintf (buffer, "INSERT OR REPLACE INTO Obstacles VALUES ('%s', '%d', '%s');", player[1].name, rng, "Bomb");
               strcpy (sql, buffer);
               rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
          }
     }
     if (IsCollision(&player[0], &player[1], col.enemy))
     {
          if (game.delay == 0.0f)
          {
               PlaySound (sound[1]); //oof
               player[0].speed -= diff.speed.player.dec;
               if (player[0].speed < 0.0f)
                    player[0].speed = 0.0f;
               game.health -= diff.health.dec.enemy;
               game.delay = 0x1000*GetFrameTime();
          }
          else game.delay--;
     }
}

void DrawHUD (void)
{
     char buffer[101]; 
     snprintf (buffer, 30, "%s: %d", "Score", game.score);
     DrawText (buffer, 10, 10, 30, DARKBLUE);
     snprintf (buffer, 30, "%s: %d", "Health", game.health);
     DrawText (buffer, 10, 35, 30, RED);
     if (game.health <= 0)
     {
          game.health = 0;
          sprintf (buffer, "INSERT OR REPLACE INTO Players VALUES ('%s', '%d', '%d');", player[1].name, game.score, game.difficulty);
          strcpy (sql, buffer);
          rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
          game.won = 0;
          EndMenu();
     }
     if (game.score == apple[0].num)
     {
          sprintf (buffer, "INSERT OR REPLACE INTO Players VALUES ('%s', '%d', '%d');", player[0].name, game.score, game.difficulty);
          strcpy (sql, buffer);
          rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
          game.won = 1;
          EndMenu();
     }
}

void Gameplay (void)
{
     game.delay = 0x1000*GetFrameTime();
     game.start = 1;
     
     sound[1] = LoadSound ("./res/sounds/oof.ogg");
     sound[2] = LoadSound ("./res/sounds/eat.ogg");
     sound[3] = LoadSound ("./res/sounds/boom.ogg");
     
     music[1] = LoadMusicStream("./res/music/music1.ogg");     
     music[2] = LoadMusicStream("./res/music/music2.ogg");     
     music[3] = LoadMusicStream("./res/music/music3.ogg");     

     UpdateAudio();
     PlayMusicStream (music[0]);

     //InitEntities();
     GenerateEntities();
     while (!WindowShouldClose() && !game.close)
     {
          w.x = GetScreenWidth();
          w.y = GetScreenHeight();
          if (!game.mutemusic)
               UpdateMusicStream(music[0]);
          PlayerMovement();
          EnemyMovement();
          CalcCollisions();
          BeginDrawing();
               ClearBackground(BLACK);
               DrawBackground(255, 1);
               DrawEntities();
               DrawHUD();
               if (IsKeyPressed (KEY_P))
                    PauseMenu();
               DrawPauseButton();
          EndDrawing();
          if (IsWindowResized()) //|| IsWindowMaximized()) //TODO: Maximized
          {
               GenerateEntities();
               DrawEntities();
               DrawHUD();
               DrawPauseButton();
          }
          if (IsKeyPressed (KEY_Q))
               game.close = 1;
     }
     if (rc != SQLITE_OK)
     {
          fprintf (stderr, "Failed to select data\n");
          fprintf (stderr, "SQL Error: %s\n", err_msg);
          sqlite3_close(db);
          return;
     }
}
