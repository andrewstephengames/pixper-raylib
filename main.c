#include <stdio.h>
#include <stdlib.h>
#include <string.h> //for strcat and strcpy
#include <math.h> //for sqrt and pow (collisions)
#include <time.h> //for clock() (random seed)
#include <raylib.h>

#define PLATFORM_DESKTOP
#define E_SZ 100

typedef struct
{
     int x, y;
} Window;

typedef struct
{
     float x, y, speed;
     Texture2D sprite;
     int num;
     bool used;
} Entity;

typedef struct
{
     bool close, mutemusic, mutesound, help, version, cleardb;
     int score, health;
     float delay;
} Game;

Window w = {
     .x = 800,
     .y = 600,
};

Entity player[2], background, apple[E_SZ], grass[E_SZ], tree[E_SZ], bomb[E_SZ];
Sound sound[4];
Music music[4];
Game game;

bool IsCollision (Entity *a, Entity *b, float c)
{
     float d = sqrt((pow(b->x-a->x, 2) + pow(b->y-a->y, 2)));
     return d < c; //distance has to be smaller than collision coefficient
}

void GenerateEntities (void)
{
     //Designate the first value of obstacle arrays to the number of obstacles
     apple[0].num = GetRandomValue (w.y/25, w.x/25);
     tree[0].num = GetRandomValue (w.y/40, w.x/40);
     grass[0].num = GetRandomValue (w.y/40, w.x/40);
     bomb[0].num = GetRandomValue (w.y/20, w.x/20);
     game.score = 0; // The score must reset for each resize
     for (int i = 1; i <= apple[0].num; i++)
     {
          apple[i].sprite = LoadTexture ("res/images/apple.png");
          apple[i].x = GetRandomValue (5, w.x-32);
          apple[i].y = GetRandomValue (5, w.y-32);
     }
     for (int i = 1; i <= grass[0].num; i++)
     {
          grass[i].sprite = LoadTexture ("res/images/grasstile.png");
          grass[i].x = GetRandomValue (5, w.x-32);
          grass[i].y = GetRandomValue (5, w.y-32);
     }
     for (int i = 1; i <= tree[0].num; i++)
     {
          tree[i].sprite = LoadTexture ("res/images/tree.png");
          tree[i].x = GetRandomValue (5, w.x-32);
          tree[i].y = GetRandomValue (5, w.y-32);
     }
     for (int i = 1; i <= bomb[0].num; i++)
     {
          bomb[i].sprite = LoadTexture ("res/images/bomb.png");
          bomb[i].x = GetRandomValue (5, w.x-32);
          bomb[i].y = GetRandomValue (5, w.y-32);
     }
     // player[0] is the main character
     player[0].sprite = LoadTexture ("res/images/player-black.png");
     player[0].speed = 3.5f;
     player[0].num = 1; //might add support for multiple players
     player[0].x = GetRandomValue (5, w.x-32);
     player[0].y = GetRandomValue (5, w.y-32);
     // player[1] is the enemy
     player[1].sprite = LoadTexture ("res/images/enemy-black.png");
     player[1].speed = 1.5f;
     player[1].num = 1; //might add support for multiple enemies
     player[1].x = GetRandomValue (5, w.x-32);
     player[1].y = GetRandomValue (5, w.y-32);
     // prevent the enemy from spawning on top of, or near the player
     while (IsCollision (&player[0], &player[1], 25))
     {
          player[1].x = GetRandomValue (5, w.x-32);
          player[1].y = GetRandomValue (5, w.y-32);
     }
     // prevent bombs generating on top of, or near the player
     for (int i = 1; i <= bomb[0].num; i++)
     while (IsCollision(&player[0], &bomb[i], 25))
     {
          player[0].x = GetRandomValue (5, w.x-32);
          player[0].y = GetRandomValue (5, w.y-32);
     }
}

void Init (void)
{
     InitWindow(w.x, w.y, "Pixper");
     SetConfigFlags(FLAG_WINDOW_RESIZABLE);
     InitAudioDevice();
     SetTargetFPS(60);
     SetTraceLogLevel(LOG_ERROR);
     SetRandomSeed(clock());

     game.health = 10;
     game.delay = (int) (0x1000*GetFrameTime());
     
     sound[1] = LoadSound ("res/sounds/oof.ogg");
     sound[2] = LoadSound ("res/sounds/eat.ogg");
     sound[3] = LoadSound ("res/sounds/boom.ogg");

     size_t soundsize;
     soundsize = sizeof(sound) / sizeof (Sound);
     for (size_t i = 1; i < soundsize; i++)
          SetSoundVolume(sound[i], 0.10f);

     music[1] = LoadMusicStream("res/music/music1.ogg");     
     music[2] = LoadMusicStream("res/music/music2.ogg");     
     music[3] = LoadMusicStream("res/music/music3.ogg");     

     size_t musicindex, musicsize;
     musicsize = sizeof(music) / sizeof(Music);
     musicindex = GetRandomValue (1, musicsize-1);
     music[0] = music[musicindex];

     PlayMusicStream (music[0]);
     SetMusicVolume (music[0], 0.25f);

     GenerateEntities();
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

void DrawEntities (void)
{
     for (int i = 1; i <= apple[0].num; i++)
          DrawTexture (apple[i].sprite, apple[i].x, apple[i].y, WHITE);
     for (int i = 1; i <= grass[0].num; i++)
          DrawTexture (grass[i].sprite, grass[i].x, grass[i].y, WHITE);
     for (int i = 1; i <= tree[0].num; i++)
          DrawTexture (tree[i].sprite, tree[i].x, tree[i].y, WHITE);
     for (int i = 1; i <= bomb[0].num; i++)
          DrawTexture (bomb[i].sprite, bomb[i].x, bomb[i].y, WHITE);
     DrawTexture (player[0].sprite, player[0].x, player[0].y, WHITE);
     DrawTexture (player[1].sprite, player[1].x, player[1].y, WHITE);
}

void DrawBackground (void)
{
     background.sprite = LoadTexture ("res/images/grass.png");
     for (int i = 0; i < w.x; i += 256)
          for (int j = 0; j < w.y; j += 256)
               DrawTexture (background.sprite, i, j, WHITE);
}


void CalcCollisions (void)
{
     for (int i = 1; i <= apple[0].num; i++)
          //if the player collides with an apple
          if (IsCollision (&player[0], &apple[i], 10) && !apple[i].used)
          {
               game.score++;
               game.health++;
               player[0].speed += 0.05f;
               apple[i].used = 1;
               apple[i].sprite = LoadTexture ("res/images/grasstile.png");
               PlaySound (sound[2]); //eat
          }
     for (int i = 1; i <= bomb[0].num; i++)
     {
          //if the player collides with a bomb
          if (IsCollision (&player[0], &bomb[i], 20) && !bomb[i].used)
          {
               PlaySound (sound[3]); //boom
               PlaySound (sound[1]); //oof
               game.health -= 5;
               player[0].speed -= 0.10f;
               if (player[0].speed < 0.0f)
                    player[0].speed = 0.0f;
               bomb[i].used = 1;
               bomb[i].sprite = LoadTexture ("res/images/bombtile.png");
          }
          //if the enemy collides with a bomb
          if (IsCollision(&player[1], &bomb[i], 30) && !bomb[i].used)
          {
               PlaySound (sound[3]); //boom
               player[1].speed += 0.10f;
               bomb[i].used = 1;
               bomb[i].sprite = LoadTexture ("res/images/bombtile.png");
          }
     }
     if (IsCollision(&player[0], &player[1], 15))
          if (game.delay == 0.0f)
          {
               PlaySound (sound[1]); //oof
               player[0].speed -= 0.001f;
               if (player[0].speed < 0.0f)
                    player[0].speed = 0.0f;
               game.health--;
               game.delay = (int) (0x1000*GetFrameTime());
          }
          else game.delay--;
}

// https://www.geeksforgeeks.org/implement-itoa/

void reverse(char s[])
{
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--)
     {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}

void itoa(int n, char s[])
{
     int i, sign;
 
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}

void DrawHUD (void)
{
     char s[30], h[30], t[30];
     strcpy (s, "Score: ");
     strcpy (h, "Health: ");
     itoa (game.score, t);
     strcat (s, t);
     itoa (game.health, t);
     strcat (h, t);
     DrawText (s, 10, 10, 30, DARKBLUE);
     DrawText (h, 10, 35, 30, RED);
     if (game.health <= 0)
     {
          DrawText ("You Died!", 50, 50, 50, GRAY);
          game.close = 1;
     }
     if (game.score == apple[0].num)
     {
          DrawText ("You Won!", 50, 50, 50, YELLOW);
          game.close = 1;
     }
}

//TODO: Add database to log obstacles
void LogCoords (void)
{
     /*
     printf ("Apple: (%d %d)\n", apple.x, apple.y);
     printf ("Grass: (%d %d)\n", grass.x, grass.y);
     printf ("Tree: (%d %d)\n", tree.x, tree.y);
     printf ("Bomb: (%d %d)\n", bomb.x, bomb.y);
     printf ("Player: (%d %d)\n", player.x, player.y);
     */
     return;
}

int main (int argc, char **argv)
{
     Init();
     // command-line args
     for (int i = 2; i <= argc && argc > 1; i++)
          if (strcmp (argv[i], "mutemusic") == 0)
          {
               printf ("%d: %s\n", argc, argv[i]);
               PauseMusicStream(music[0]);
               game.mutemusic = 1;
          }
     LogCoords();
     while (!WindowShouldClose())
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
               DrawBackground();
               DrawEntities();
               DrawHUD();
          EndDrawing();
          if (IsWindowResized() || IsWindowMaximized()) //TODO: Maximized
          {
               GenerateEntities();
               DrawEntities();
          }
          if (IsKeyPressed (KEY_Q) || game.close)
               break;
          if (IsKeyPressed (KEY_F))
               printf ("(%.3f %.3f)\n", player[0].x, player[0].y);
     }
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
     return 0;
}
