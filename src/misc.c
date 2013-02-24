#include "layer.h"
#include "misc.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void
trace (const char *s)
{
    fprintf (stderr, "==> %s\n", s);
}

// SDL function to color a specific pixel on "screen".
void
color_pixel (SDL_Surface * screen, int x, int y, Uint32 target_color)
{
    *((Uint32 *) (screen->pixels) + x + y * screen->w) = target_color;
}

/**
 * Returns a value between 0 and max excluded.
 */
unsigned long
custom_random (unsigned long max)
{
    return (unsigned long) ( ( ((double)rand()) / RAND_MAX) * max);
}

/**
 * Returns a value between 0 and max.
 */
// Fourni une valeur entre 0 et a.
//    * b et m sont premiers entre eux ;
//    * si m est un multiple de 4, alors a%4 = 1 ;
//    * pour tous les nombres premiers p diviseurs de m, on a a%p =1 .

// b doit être "petit" par rapport à a et m ;
// a doit être proche de la racine carrée de m.
unsigned long 
randomgen (unsigned long max, unsigned long seed)
{
    unsigned long i;
    static unsigned long random_number = RANDOMGEN_INIT;
    unsigned long factor = RANDOMGEN_FACTOR, offset = RANDOMGEN_OFFSET;
    for (i = 0; i < seed; i++)
    {
        random_number = (factor * random_number + offset) % max;
    }
    random_number %= max;
    return random_number % max;
}

void
save_bmp (struct layer *c, const char *filename)
{

    SDL_Surface *screen =
        SDL_CreateRGBSurface (SDL_SWSURFACE, c->size, c->size, 32, 0, 0, 0,
                              0);
    if (!screen)
        trace ("SDL error on SDL_CreateRGBSurface");

    int i, j;
    Uint32 u;
    SDL_PixelFormat *fmt = screen->format;
    for (i = 0; i < c->size; i++)
    {
        for (j = 0; j < c->size; j++)
        {
            u = SDL_MapRGB (fmt, c->v[i][j], c->v[i][j], c->v[i][j]);
            color_pixel (screen, i, j, u);
        }
    }

    SDL_SaveBMP (screen, filename);
    SDL_FreeSurface (screen);
}

void
save_bmp_rgb (layer * c,
              const char *filename,
              int threshold_red,
              int threshold_green,
              int threshold_blue, color color1, color color2, color color3)
{

    SDL_Surface *screen =
        SDL_CreateRGBSurface (SDL_SWSURFACE, c->size, c->size, 32, 0, 0, 0,
                              0);
    if (!screen)
        trace ("SDL error on SDL_CreateRGBSurface");

    int i, j;
    int color_init = 0;
    Uint32 u;
    SDL_PixelFormat *fmt = screen->format;
    for (i = 0; i < c->size; i++)
        for (j = 0; j < c->size; j++)
        {

            color_init = c->v[i][j] % 255;

            Uint8 red, green, blue;

            double f;

            if (color_init < threshold_red)
            {
                f = (double) (color_init - 0) / (threshold_red - 0);
                red = color1.red;
                green = color1.green;
                blue = color1.blue;
            }
            else if (color_init < threshold_green)
            {
                f = (double) (color_init - threshold_red) / (threshold_green -
                                                             threshold_red);
                red = (Uint8) (color1.red * (1 - f) + color2.red * (f));
                green = (Uint8) (color1.green * (1 - f) + color2.green * (f));
                blue = (Uint8) (color1.blue * (1 - f) + color2.blue * (f));
            }
            else if (color_init < threshold_blue)
            {
                f = (double) (color_init -
                              threshold_green) / (threshold_blue -
                                                  threshold_green);
                red = (Uint8) (color2.red * (1 - f) + color3.red * (f));
                green = (Uint8) (color2.green * (1 - f) + color3.green * (f));
                blue = (Uint8) (color2.blue * (1 - f) + color3.blue * (f));
            }
            else
            {
                f = (double) (color_init - threshold_blue) / (255 -
                                                              threshold_blue);
                red = color3.red;
                green = color3.green;
                blue = color3.blue;
            }

            u = SDL_MapRGB (fmt, red, green, blue);
            color_pixel (screen, i, j, u);
        }

    SDL_SaveBMP (screen, filename);
    SDL_FreeSurface (screen);
}


void
save_bmp_alt (layer * c,
              const char *filename, int threshold, color color1, color color2)
{

    SDL_Surface *screen =
        SDL_CreateRGBSurface (SDL_SWSURFACE, c->size, c->size, 32, 0, 0, 0,
                              0);
    if (!screen)
        trace ("SDL error on SDL_CreateRGBSurface");

    int i, j;
    int color_init = 0;
    Uint32 u;
    SDL_PixelFormat *fmt = screen->format;
    for (i = 0; i < c->size; i++)
        for (j = 0; j < c->size; j++)
        {

            color_init = c->v[i][j] % 255;

            int red, green, blue;

            double f;

            double value = fmod (color_init, threshold);
            if (value > threshold / 2)
            {
                value = threshold - value;
            }

            f = (1 - cos (3.1415 * value / (threshold / 2))) / 2;
            red = color1.red * (1 - f) + color2.red * f;
            green = color1.green * (1 - f) + color2.green * f;
            blue = color1.blue * (1 - f) + color2.blue * f;

            u = SDL_MapRGB (fmt, red, green, blue);
            color_pixel (screen, i, j, u);
        }

    SDL_SaveBMP (screen, filename);
    SDL_FreeSurface (screen);
}
