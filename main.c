#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <raylib.h>

struct Coord
{
     short unsigned x, y;
};

typedef struct
{
     Texture2D sprite;
     short unsigned x, y;
} Entity;

struct Coord w = {
     .x = 800,
     .y = 600
};

Entity player, enemy, backgrass, apple, grass, tree, bomb;

void Init (void)
{
     srand(clock());
     player.sprite = LoadTexture ("res/images/player.png");
     apple.sprite = LoadTexture ("res/images/apple.png");
     grass.sprite = LoadTexture ("res/images/grasstile.png");
     tree.sprite = LoadTexture ("res/images/tree.png");
     bomb.sprite = LoadTexture ("res/images/bomb.png");
}

void Movement (void)
{
     int key;
     key = GetKeyPressed();
     switch (key)
     {
          case KEY_W:
               player.y -= GetFrameTime()*800;
               break;
          case KEY_A:
               player.x -= GetFrameTime()*800;
               break;
          case KEY_S:
               player.y += GetFrameTime()*800;
               break;
          case KEY_D:
               player.x += GetFrameTime()*800;
          default:
               break;
     }
     //if (player.y < 20)
          //fprintf (stderr, "(%f %f)\n", player.x, player.y);
}

void GenerateEntity (Entity *entity)
{
     entity->x = rand() % (w.x-32);
     entity->y = rand() % (w.y-32);
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
     //UnloadTexture (backgrass);
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
     InitWindow(w.x, w.y, "Pixper");
     SetTargetFPS(60);
     Init();
     //DrawBackground();
     GenerateEntity (&apple);
     GenerateEntity (&grass);
     GenerateEntity (&tree);
     GenerateEntity (&bomb);
     GenerateEntity (&player);
     LogCoords();
     while (!WindowShouldClose())
     {
          Movement();
          BeginDrawing();
               ClearBackground(BLACK);
               DrawEntity (&apple);
               DrawEntity (&grass);
               DrawEntity (&tree);
               DrawEntity (&bomb);
               DrawTexture (player.sprite, player.x, player.y, GREEN);
          EndDrawing();
          if (IsKeyPressed (KEY_Q))
               break;
     }
     UnloadTexture (player.sprite);
     UnloadTexture (grass.sprite);
     UnloadTexture (apple.sprite);
     UnloadTexture (bomb.sprite);
     CloseWindow();
     return 0;
}
