#include <stdio.h>
#include <stdlib.h>
#include <string.h> //for strcat, strcpy and strlen
#include <math.h> //for sqrt and pow (collisions)
#include <time.h> //for clock() (random seed)
#include <sqlite3.h> //for db
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
     char name[30];
} Entity;

typedef struct
{
     bool close, mutemusic, mutesound, help, version, cleardb, difficulty;
     int score, health, delay;
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

Window w = {
     .x = 1280,
     .y = 720,
};

Entity player[2], background, apple[E_SZ], grass[E_SZ], tree[E_SZ], bomb[E_SZ];
Sound sound[4];
Music music[4];
Game game;
Difficulty diff;
Collision col;

sqlite3 *db;
char *err_msg, sql[500];
int rc;

int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
     NotUsed = 0;
     for (int i = 0; i < argc; i++)
          printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
     printf("\n");
     return 0;
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

void GenerateEntities (void)
{
     //Designate the first value of obstacle arrays to the number of obstacles
     game.score = 0; // The score must reset for each resize
     SetDifficulty (game.difficulty);
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
     //player[0].speed = 3.5f;
     player[0].num = 1; //might add support for multiple players
     player[0].x = GetRandomValue (5, w.x-32);
     player[0].y = GetRandomValue (5, w.y-32);
     strcpy (player[0].name, "Player");
     // player[1] is the enemy
     player[1].sprite = LoadTexture ("res/images/enemy-black.png");
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

void Init (void)
{
     SetConfigFlags(FLAG_WINDOW_RESIZABLE);
     InitWindow(w.x, w.y, "Pixper");
     InitAudioDevice();
     SetTargetFPS(60);
     SetTraceLogLevel(LOG_ERROR);
     SetRandomSeed(clock());
     rc = sqlite3_open ("res/db/stats.db", &db);
     strcpy (sql, "CREATE TABLE IF NOT EXISTS Players (Name TEXT PRIMARY KEY, Score INTEGER, Difficulty BOOLEAN);"
           "CREATE TABLE IF NOT EXISTS Obstacles (Playername TEXT, RNG INTEGER PRIMARY KEY, Name TEXT);");
     
     rc = sqlite3_exec(db, sql, 0, 0, &err_msg); //update db

     game.health = 10;
     game.delay = 0x1000*GetFrameTime();
     game.difficulty = 0;
     
     sound[1] = LoadSound ("res/sounds/oof.ogg");
     sound[2] = LoadSound ("res/sounds/eat.ogg");
     sound[3] = LoadSound ("res/sounds/boom.ogg");

     size_t soundsize = sizeof(sound) / sizeof (Sound) - 1;
     for (size_t i = 1; i <= soundsize; i++)
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

Window CenterText (const char *s, int size)
{
     return (Window) {
          .x = w.x/2 - MeasureText (s, size)/2,
          .y = w.y/2 - size/2,
     };
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
     if (game.score > apple[0].num * diff.reveal.a && game.score < apple[0].num * diff.reveal.b)
     {
          char s[] = "Hidden apples have been revealed!\0";
          int size = w.x/20;
          Window mid = CenterText (s, size);
          DrawText (s, mid.x, mid.y, size, YELLOW);
          for (int i = 1; i <= apple[0].num; i++)
               DrawTexture (apple[i].sprite, apple[i].x, apple[i].y, WHITE);
     }
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
     char buffer[101];
     for (int i = 1; i <= apple[0].num; i++)
          //if the player collides with an apple
          if (IsCollision (&player[0], &apple[i], col.apple) && !apple[i].used)
          {
               game.score += diff.score;
               game.health += diff.health.inc.player;
               player[0].speed += diff.speed.player.inc;
               apple[i].used = 1;
               apple[i].sprite = LoadTexture ("res/images/grasstile.png");
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
               bomb[i].sprite = LoadTexture ("res/images/bombtile.png");
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
               bomb[i].sprite = LoadTexture ("res/images/bombtile.png");
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
     char buffer[101], s[30];
     snprintf (buffer, 30, "%s: %d", "Score", game.score);
     DrawText (buffer, 10, 10, 30, DARKBLUE);
     snprintf (buffer, 30, "%s: %d", "Health", game.health);
     DrawText (buffer, 10, 35, 30, RED);
     if (game.health <= 0)
     {
          game.health = 0;
          strcpy (s, "You Died!");
          int size = 50;
          Window mid = CenterText (s, size);
          DrawText (s, mid.x, mid.y, size, GRAY);
          game.close = 1;
          sprintf (buffer, "INSERT OR REPLACE INTO Players VALUES ('%s', '%d', '%d');", player[1].name, game.score, game.difficulty);
          strcpy (sql, buffer);
          rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
          //TODO: make end game menu
     }
     if (game.score == apple[0].num)
     {
          strcpy (s, "You won!");
          int size = 50;
          Window mid = CenterText (s, size);
          DrawText (s, mid.x, mid.y, size, YELLOW);
          game.close = 1;
          sprintf (buffer, "INSERT OR REPLACE INTO Players VALUES ('%s', '%d', '%d');", player[0].name, game.score, game.difficulty);
          strcpy (sql, buffer);
          rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
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
     if (rc != SQLITE_OK)
     {
          fprintf (stderr, "Cannot open the database: %s\n", sqlite3_errmsg(db));
          sqlite3_close(db);
          return 1;
     }
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
               game.cleardb = 1;
          }
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
          if (IsWindowResized()) //|| IsWindowMaximized()) //TODO: Maximized
          {
               GenerateEntities();
               DrawEntities();
               DrawHUD();
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
     strcpy (sql, "SELECT Name, Score, Difficulty FROM Players ORDER BY Score DESC");
     rc = sqlite3_exec (db, sql, callback, 0, &err_msg); 
     strcpy (sql, "SELECT Playername, RNG, Name FROM Obstacles ORDER BY RNG DESC LIMIT 3");
     rc = sqlite3_exec (db, sql, callback, 0, &err_msg); 
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
