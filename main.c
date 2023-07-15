#include <stdio.h>
#include <stdlib.h>
#include <string.h> //for strcat and strcpy
#include <math.h> //for sqrt and pow (collisions)
#include <time.h>
#include <raylib.h>

#define PLATFORM_DESKTOP

typedef struct
{
     short x, y;
} Window;

typedef struct
{
     Texture2D sprite;
     short x, y, num;
     float speed;
     bool used;
} Entity;

Window w = {
     .x = 800,
     .y = 600,
};

Entity player[2], background, apple[100], grass[100], tree[100], bomb[100];
short score, health = 10;

bool IsCollision (Entity *a, Entity *b, short c)
{
     short d = sqrt((pow(b->x-a->x, 2) + pow(b->y-a->y, 2)));
     return d < c; //distance has to be smaller than collision coefficient
}

void GenerateEntities (void)
{
     //Designate the first value of obstacle arrays to the number of obstacles
     apple[0].num = GetRandomValue (w.y/25, w.x/25);
     tree[0].num = GetRandomValue (w.y/40, w.x/40);
     grass[0].num = GetRandomValue (w.y/40, w.x/40);
     bomb[0].num = GetRandomValue (w.y/20, w.x/20);
     for (short i = 1; i < apple[0].num; i++)
     {
          apple[i].sprite = LoadTexture ("res/images/apple.png");
          apple[i].x = GetRandomValue (5, w.x-32);
          apple[i].y = GetRandomValue (5, w.y-32);
     }
     for (short i = 1; i < grass[0].num; i++)
     {
          grass[i].sprite = LoadTexture ("res/images/grasstile.png");
          grass[i].x = GetRandomValue (5, w.x-32);
          grass[i].y = GetRandomValue (5, w.y-32);
     }
     for (short i = 1; i < tree[0].num; i++)
     {
          tree[i].sprite = LoadTexture ("res/images/tree.png");
          tree[i].x = GetRandomValue (5, w.x-32);
          tree[i].y = GetRandomValue (5, w.y-32);
     }
     for (short i = 1; i < bomb[0].num; i++)
     {
          bomb[i].sprite = LoadTexture ("res/images/bomb.png");
          bomb[i].x = GetRandomValue (5, w.x-32);
          bomb[i].y = GetRandomValue (5, w.y-32);
     }
     // player[0] is the main character
     player[0].sprite = LoadTexture ("res/images/player-black.png");
     player[0].speed = 5.0f;
     player[0].num = 1; //might add support for multiple players
     player[0].x = GetRandomValue (5, w.x-32);
     player[0].y = GetRandomValue (5, w.y-32);
     // player[1] is the enemy
     player[1].sprite = LoadTexture ("res/images/enemy-black.png");
     player[1].speed = 1.0f;
     player[1].num = 1; //might add support for multiple enemies
     player[1].x = GetRandomValue (5, w.x-32);
     player[1].y = GetRandomValue (5, w.y-32);
     // prevent enemy generating on top of, or near the player
     while (IsCollision (&player[0], &player[1], 25))
     {
          player[1].x = GetRandomValue (5, w.x-32);
          player[1].y = GetRandomValue (5, w.y-32);
     }
     // prevent bombs generating on top of the player
     for (short i = 0; i < bomb[0].num; i++)
     while (IsCollision(&player[0], &bomb[i], 25))
     {
          player[0].x = GetRandomValue (5, w.x-32);
          player[0].y = GetRandomValue (5, w.y-32);
     }
}

void Init (void)
{
     SetConfigFlags(FLAG_WINDOW_RESIZABLE);
     InitWindow(w.x, w.y, "Pixper");
     SetTargetFPS(60);
     SetTraceLogLevel(LOG_ERROR);
     SetRandomSeed(clock());
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
     for (short i = 1; i < apple[0].num; i++)
          DrawTexture (apple[i].sprite, apple[i].x, apple[i].y, WHITE);
     for (short i = 1; i < grass[0].num; i++)
          DrawTexture (grass[i].sprite, grass[i].x, grass[i].y, WHITE);
     for (short i = 1; i < tree[0].num; i++)
          DrawTexture (tree[i].sprite, tree[i].x, tree[i].y, WHITE);
     for (short i = 1; i < bomb[0].num; i++)
          DrawTexture (bomb[i].sprite, bomb[i].x, bomb[i].y, WHITE);
     DrawTexture (player[0].sprite, player[0].x, player[0].y, WHITE);
     DrawTexture (player[1].sprite, player[1].x, player[1].y, WHITE);
}

void DrawBackground (void)
{
     background.sprite = LoadTexture ("res/images/grass.png");
     for (short i = 0; i < w.x; i += 256)
          for (short j = 0; j < w.y; j += 256)
               DrawTexture (background.sprite, i, j, WHITE);
}


void CalcCollisions (void)
{
     for (short i = 1; i < apple[0].num; i++)
          //if the player collides with an apple
          if (IsCollision (&player[0], &apple[i], 10) && !apple[i].used)
          {
               score++;
               health++;
               player[0].speed += 0.25f;
               apple[i].used = 1;
               apple[i].sprite = LoadTexture ("res/images/grasstile.png");
          }
     for (short i = 1; i < bomb[0].num; i++)
     {
          //if the player collides with a bomb
          if (IsCollision (&player[0], &bomb[i], 20) && !bomb[i].used)
          {
               health -= 5;
               player[0].speed -= 0.10f;
               if (player[0].speed < 0.0f)
                    player[0].speed = 0.0f;
               bomb[i].used = 1;
               bomb[i].sprite = LoadTexture ("res/images/bombtile.png");
          }
          //if the enemy collides with a bomb
          if (IsCollision(&player[1], &bomb[i], 20) && !bomb[i].used)
          {
               player[1].speed += 0.10f;
               bomb[i].used = 1;
               bomb[i].sprite = LoadTexture ("res/images/bombtile.png");
          }
     }
     //if the player and enemy collide
     if (IsCollision(&player[0], &player[1], 5))
     {
          player[0].speed -= 0.001f;
          if (player[0].speed < 0.0f)
               player[0].speed = 0.0f;
          health--;
     }
}

// https://www.geeksforgeeks.org/implement-itoa/

void reverse(char s[])
{
     short i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--)
     {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}

void itoa(short n, char s[])
{
     short i, sign;
 
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
     char s[30], t[30];
     strcpy (s, "Score: ");
     itoa (score, t);
     strcat (s, t);
     DrawText (s, 10, 10, 30, DARKBLUE);
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

int main (void)
{
     Init();
     LogCoords();
     while (!WindowShouldClose())
     {
          w.x = GetScreenWidth();
          w.y = GetScreenHeight();
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
          if (IsKeyPressed (KEY_Q))
               break;
          if (IsKeyPressed (KEY_F))
               printf ("(%d %d)\n", player[0].x, player[0].y);
     }
     UnloadTexture (player[0].sprite);
     UnloadTexture (background.sprite);
     for (short i = 0; i < 100; i++)
          if (grass[i].num || apple[i].num || bomb[i].num || tree[i].num)
          {
               UnloadTexture (grass[i].sprite);
               UnloadTexture (apple[i].sprite);
               UnloadTexture (bomb[i].sprite);
               UnloadTexture (tree[i].sprite);
          }
     CloseWindow();
     return 0;
}
