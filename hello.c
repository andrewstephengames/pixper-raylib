/*
     GenerateEntity (apple);
     GenerateEntity (grass);
     GenerateEntity (tree);
     GenerateEntity (bomb);
     GenerateEntity (player);
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <raylib.h>

struct Coord
{
     float x, y;
};

struct Coord w = {
     .x = 800,
     .y = 600
};

struct Coord c = {
     .x = 0,
     .y = 300
};

typedef struct
{
     Texture2D sprite;
     short x, y, speed, num, score;
} Entity;

Entity a[50], b, cc[1], dd[2];
short health = 10;

void test (Entity entity[])
{
     size_t n = sizeof(*entity);
     printf ("n == %zu\n", n);
     return;
}

// https://www.geeksforgeeks.org/implement-itoa/

void reverse(char s[]) // "The C Programming Language" implementation
 {
     short i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }

 void itoa(short n, char s[]) // "The C Programming Language" implementation
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
     char s[30];
     strcpy (s, "Health: ");
     char t[30];
     itoa (1234, t);
     strcat (s, t);
     printf ("%s\n", s);
     return;
//     DrawText (s, 10, 10, 20, RED);
}

int main ()
{
     short d = 1;
     //printf ("%s", itoa(1234));
     DrawHUD();
     return 0;
     //printf ("sizeof(a) == %ld\n", sizeof(a)/sizeof(Entity));
     //printf ("sizeof(Entity) == %ld\n", sizeof(Entity));
     //printf ("sizeof(cc) == %ld\n", sizeof(cc)/sizeof(Entity));
     //printf ("sizeof(dd) == %ld\n", sizeof(dd)/sizeof(Entity));
     //test(a);
     //printf ("sizeof(Texture2D) == %zu\n", sizeof(Texture2D));
     //printf ("sizeof(short) == %zu\n", sizeof(short));
     //return 0;
     InitWindow(w.x, w.y, "Pixper");
     SetTargetFPS(60);
     while (!WindowShouldClose())
     {
          c.x += GetFrameTime()*400*d;
          if (c.x >= w.x-100 || c.x <= 0)
               d *= -1;
          BeginDrawing();
               ClearBackground(BLACK);
               DrawText("Hello World", c.x, c.y, 20, BLUE);
          EndDrawing();
          if (IsKeyPressed(KEY_Q))
               break;
     }
     CloseWindow();
     return 0;
}
