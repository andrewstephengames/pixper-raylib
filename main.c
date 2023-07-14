#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <raylib.h>

struct Coord
{
     short unsigned x, y;
};

typedef struct
{
     Texture2D sprite;
     short unsigned x, y, speed;
} Entity;

struct Coord w = {
     .x = 800,
     .y = 600
};

Entity player, enemy, backgrass, apple, grass, tree, bomb;

void Init (void)
{
     SetConfigFlags(FLAG_WINDOW_RESIZABLE);
     InitWindow(w.x, w.y, "Pixper");
     SetTargetFPS(60);
     SetTraceLogLevel(LOG_ERROR);
     SetRandomSeed(clock());
     player.sprite = LoadTexture ("res/images/player-black.png");
     player.speed = 5;
     apple.sprite = LoadTexture ("res/images/apple.png");
     grass.sprite = LoadTexture ("res/images/grasstile.png");
     tree.sprite = LoadTexture ("res/images/tree.png");
     bomb.sprite = LoadTexture ("res/images/bomb.png");
}

void Movement (void)
{
     bool k[4] = {
          IsKeyDown(KEY_W) || IsKeyDown(KEY_UP),
          IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT),
          IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN),
          IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT),
     };
     if (k[0])
          player.y -= player.speed;
     if (k[1])
          player.x -= player.speed;
     if (k[2])
          player.y += player.speed;
     if (k[3])
          player.x += player.speed;
     if (player.x <= 5)
          player.x = 5;
     if (player.y <= 5)
          player.y = 5;
     if (player.x >= w.x-32)
          player.x = w.x-32;
     if (player.y >= w.y-32)
          player.y = w.y-32;
}

void GenerateEntity (Entity *entity)
{
     entity->x = GetRandomValue (5, w.x-32);
     entity->y = GetRandomValue (5, w.y-32);
}

void DrawEntity (Entity *entity)
{
     DrawTexture (entity->sprite, entity->x, entity->y, WHITE);
}

void DrawBackground (void)
{
     backgrass.sprite = LoadTexture ("res/images/grass.png");
     for (short unsigned i = 0; i < w.x; i += 256)
          for (short unsigned j = 0; j < w.y; j += 256)
               DrawTexture (backgrass.sprite, i, j, WHITE);
}

void LogCoords (void)
{
     printf ("Apple: (%d %d)\n", apple.x, apple.y);
     printf ("Grass: (%d %d)\n", grass.x, grass.y);
     printf ("Tree: (%d %d)\n", tree.x, tree.y);
     printf ("Bomb: (%d %d)\n", bomb.x, bomb.y);
     printf ("Player: (%d %d)\n", player.x, player.y);
}

int main (void)
{
     Init();
     GenerateEntity (&apple);
     GenerateEntity (&grass);
     GenerateEntity (&tree);
     GenerateEntity (&bomb);
     GenerateEntity (&player);
     LogCoords();
     while (!WindowShouldClose())
     {
          w.x = GetScreenWidth();
          w.y = GetScreenHeight();
          Movement();
          BeginDrawing();
               ClearBackground(BLACK);
               DrawBackground();
               DrawEntity (&apple);
               DrawEntity (&grass);
               DrawEntity (&tree);
               DrawEntity (&bomb);
               DrawEntity (&player);
               if (IsWindowResized())
               {
                    GenerateEntity (&apple);
                    GenerateEntity (&grass);
                    GenerateEntity (&tree);
                    GenerateEntity (&bomb);
               }
          EndDrawing();
          if (IsKeyPressed (KEY_Q))
               break;
     }
     UnloadTexture (backgrass.sprite);
     UnloadTexture (player.sprite);
     UnloadTexture (grass.sprite);
     UnloadTexture (apple.sprite);
     UnloadTexture (bomb.sprite);
     CloseWindow();
     return 0;
}
