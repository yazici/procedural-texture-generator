#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <math.h>

#include "proctext.h"
#include "config.h"

typedef struct layer
{
    Uint8 **v;
    long size;
    double persistence;
} layer;


/******************************************************************************/

void
usage ()
{
    puts ("procedural-textures [OPTIONS] FILE\n\n"
          "Option list:\n" "  -h: show this help.\n");
}

void
trace (const char *s)
{
    fprintf (stderr, "==> %s\n", s);
}

/******************************************************************************/

/**
 * Returns a value between 0 and max inclusive.
 */
unsigned long
randomgen (unsigned long max)
{
    return (unsigned long) ( ( ((double)rand()) / RAND_MAX) * max);
}

/**
 * Returns a value between 0 and max inclusive. Must be run the first time with
 * a seed, then without a seed.
 *
 * In this home-made random, we use the following recursive sequence to generate
 * random values:
 *
 *  random_number = ( factor * random_number + offset ) % max
 *
 * We need to match the following properties to have a decent RNG.
 *
 *  - Offset and max must be coprime
 *  - If 4 divides max , then factor % 4 == 1
 *  - For all p dividing max, factor % p == 1
 *
 * Besides,
 *
 *  - offset must be "small" compared to max;
 *  - factor must be close to the square root of max.
 */
unsigned long 
custom_randomgen (unsigned long max, unsigned long seed)
{
    static unsigned long random_number = 0;
    if (seed != 0)
    {
        random_number = seed;
        return random_number;
    }

    unsigned long factor = RANDOMGEN_FACTOR, offset = RANDOMGEN_OFFSET;

    random_number = (factor * random_number + offset) % max;
    return random_number;
}


/* SDL function to color a specific pixel on "screen". */
void
color_pixel (SDL_Surface * screen, int x, int y, Uint32 target_color)
{
    *((Uint32 *) (screen->pixels) + x + y * screen->w) = target_color;
}

/* Grayscale bitmap. */
void
save_bmp (struct layer *current_layer, const char *filename)
{

    SDL_Surface *screen =
        SDL_CreateRGBSurface (SDL_SWSURFACE, current_layer->size, current_layer->size, 32, 0, 0, 0,
                              0);
    if (!screen)
        trace ("SDL error on SDL_CreateRGBSurface");

    long i, j;
    Uint32 map;
    SDL_PixelFormat *fmt = screen->format;
    for (i = 0; i < current_layer->size; i++)
    {
        for (j = 0; j < current_layer->size; j++)
        {
            map = SDL_MapRGB (fmt, current_layer->v[i][j], current_layer->v[i][j], current_layer->v[i][j]);
            color_pixel (screen, i, j, map);
        }
    }

    SDL_SaveBMP (screen, filename);
    SDL_FreeSurface (screen);
}

void
save_bmp_rgb (layer * current_layer, const char *filename,
              Uint8 threshold_red, Uint8 threshold_green, Uint8 threshold_blue,
              color color1, color color2, color color3)
{

    SDL_Surface *screen = SDL_CreateRGBSurface (SDL_SWSURFACE, current_layer->size,
                              current_layer->size, 32, 0, 0, 0, 0);
    if (!screen)
        trace ("SDL error on SDL_CreateRGBSurface");

    long i, j;
    Uint8 color_init = 0;
    Uint32 map;
    SDL_PixelFormat *fmt = screen->format;

    for (i = 0; i < current_layer->size; i++)
        for (j = 0; j < current_layer->size; j++)
        {
            color_init = current_layer->v[i][j];

            Uint8 red, green, blue;
            double f;

            if (color_init < threshold_red)
            {
                /* f = (double) (color_init - 0) / (threshold_red - 0); */
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

            map = SDL_MapRGB (fmt, red, green, blue);
            color_pixel (screen, i, j, map);
        }

    SDL_SaveBMP (screen, filename);
    SDL_FreeSurface (screen);
}


void
save_bmp_alt (layer * current_layer,
              const char *filename, Uint8 threshold, color color1, color color2)
{

    SDL_Surface *screen =
        SDL_CreateRGBSurface (SDL_SWSURFACE, current_layer->size, current_layer->size, 32, 0, 0, 0,
                              0);
    if (!screen)
        trace ("SDL error on SDL_CreateRGBSurface");

    long i, j;
    Uint8 color_init = 0;
    Uint32 map;
    SDL_PixelFormat *fmt = screen->format;
    for (i = 0; i < current_layer->size; i++)
        for (j = 0; j < current_layer->size; j++)
        {

            color_init = current_layer->v[i][j];

            Uint8 red, green, blue;
            double f;

            double value = fmod (color_init, threshold);
            if (value > threshold / 2)
                value = threshold - value;

            f = (1 - cos (M_PI * value / (threshold / 2))) / 2;
            red = color1.red * (1 - f) + color2.red * f;
            green = color1.green * (1 - f) + color2.green * f;
            blue = color1.blue * (1 - f) + color2.blue * f;

            map = SDL_MapRGB (fmt, red, green, blue);
            color_pixel (screen, i, j, map);
        }

    SDL_SaveBMP (screen, filename);
    SDL_FreeSurface (screen);
}

// TODO: what are the two spline conditions?
long
interpol (long y1, long y2, long step, long delta)
{
    /* Spline. */
    if (step == 0)
        return y1;
    if (step == 1)
        return y2;

    double a = (double) delta / step;

    double fac1 = 3 * (1-a)*(1-a) - 2 * (1-a)*(1-a)*(1-a);
    double fac2 = 3 * a*a - 2 * a*a*a;

    return y1 * fac1 + y2 * fac2;

    /* Linear interpolation. Unused. */
    /*
      if (n!=0)
       return y1+delta*((double)y2-(double)y1)/(double)n;
       else
       return y1;
    */
}


// TODO: refactor.
long
interpol_val (long i, long j, long frequency, layer * current_layer)
{
    // Bound values.
    long bound1i, bound1j, bound2i, bound2j;
    long buf;

    // TODO: no need for doubles in all this function.
    double step = (double) current_layer->size / frequency;

    buf = (double) i / step;
    bound1i = buf * step;
    bound2i = bound1i + step;

    if (bound2i >= current_layer->size)
        bound2i = current_layer->size - 1;

    buf = (double) j / step;
    bound1j = buf * step;
    bound2j = bound1j + step;

    if (bound2j >= current_layer->size)
        bound2j = current_layer->size - 1;

    int b11, b12, b21, b22;
    b11 = current_layer->v[bound1i][bound1j];
    b12 = current_layer->v[bound1i][bound2j];
    b21 = current_layer->v[bound2i][bound1j];
    b22 = current_layer->v[bound2i][bound2j];

    int v1 = interpol (b11, b12, step, j - bound1j);
    int v2 = interpol (b21, b22, step, j - bound1j);
    int result = interpol (v1, v2, step, i - bound1i);

    return result;
}


layer *
init_layer (long size)
{
    layer *current_layer = malloc (sizeof (layer));

    if (!current_layer)
    {
        trace ("Allocation error.");
        return NULL;
    }

    current_layer->v = malloc (size * sizeof (long *));

    if (!current_layer->v)
    {
        trace ("Allocation error.");
        return NULL;
    }

    // TODO: do only one size*size malloc here.
    long i, j;
    for (i = 0; i < size; i++)
    {
        current_layer->v[i] = malloc (size * sizeof (long));
        if (!current_layer->v[i])
        {
            trace ("Allocation error.");
            return NULL;
        }
        for (j = 0; j < size; j++)
            current_layer->v[i][j] = 0;
    }

    current_layer->size = size;

    return current_layer;
}

void
free_layer (struct layer *s)
{
    int j;
    for (j = 0; j < s->size; j++)
        free (s->v[j]);
    free (s->v);
}

// TODO: use constant seed?
/* Gray scale */
layer *
generate_random_layer (struct layer *c, unsigned long seed)
{
    long size = c->size;
    long i, j;

    layer *random_layer;
    random_layer = init_layer (size);
    if (!random_layer)
    {
        trace("Could not init random layer.");
        return c;
    }
    
    /* Init seeds for both std and home-made RNG. */
    srand (seed);
    custom_randomgen (0, seed);
    for (i = 0; i < size; i++)
        for (j = 0; j < size; j++)
            /* random_layer->v[i][j] = custom_randomgen (255, 0); */
            random_layer->v[i][j] = randomgen (255);

    return random_layer;
}

// TODO: remove all int.
// TODO: check mem_leaks
// TODO: remove persistence in args and get it from current.
void
generate_work_layer (long frequency,
                     long octaves,
                     double persistence,
                     layer * current_layer, layer * random_layer)
{
    long size = current_layer->size;
    long i, j, n, f_courante = frequency;
    double sum_persistences = 0;

    layer **work_layers = malloc (octaves * sizeof (struct layer *));
    double *work_persistence = malloc (octaves * sizeof (double));
    for (n = 0; n < octaves; n++)
    {
        work_layers[n] = init_layer (size);
        if (!work_layers[n])
            return;

        for (i = 0; i < size; i++)
        {
            for (j = 0; j < size; j++)
                work_layers[n]->v[i][j] = 
                    interpol_val (i, j, f_courante, random_layer);
        }

        f_courante *= frequency;
        if (n==0)
            work_persistence[n] = persistence;
        else
            work_persistence[n] = work_persistence[n-1] * persistence;
        sum_persistences += work_persistence[n];
    }

    /* Sum of the consecutive layers for every pixel. */
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            for (n = 0; n < octaves; n++)
            {
                current_layer->v[i][j] +=
                    work_layers[n]->v[i][j] * work_persistence[n];
            }
            /* Normalizing. */
            current_layer->v[i][j] =
                current_layer->v[i][j] / sum_persistences;
        }
    }

    /* Clean. */
    free_layer (random_layer);
    for (n = 0; n < octaves; n++)
        free_layer (work_layers[n]);
    free (work_layers);
}


/**
 * We set the x,y pixel to be the mean of all pixels in the k,l square around
 * it. The new pixel value type needs to be higher than traditionnal pixel
 * because we sum pixels and thus it may overflow. The damping factor is the
 * number of pixels in the square. We need to compute it every time when we are
 * close to a border and k,l is no longer a square.
 */
layer *
smooth_layer (long factor, layer * current_layer)
{

    long size = current_layer->size;
    long damping;
    long x, y; 
    long k, l;
    double pixel_val;

    layer *smoothed_layer;
    smoothed_layer = init_layer (size);

    if (!smoothed_layer)
    {
        trace("Could not init smoothed layer.");
        return current_layer;
    }

    for (x = 0; x < size; x++)
        for (y = 0; y < size; y++)
        {
            pixel_val = 0;
            damping = 0;
            for (k = x - factor; k <= x + factor; k++)
            {
                for (l = y - factor; l <= y + factor; l++)
                    if ((k >= 0) && (k < size) && (l >= 0) && (l < size))
                    {
                        damping++;
                        pixel_val += current_layer->v[k][l];
                    }
            }
            smoothed_layer->v[x][y] = (double) pixel_val / damping;
        }

    return smoothed_layer;
}

/******************************************************************************/

int
main (int argc, char **argv)
{
    if (argc < 2)
    {
        usage ();
        return 0;
    }

    FILE *file = NULL;
    file = fopen (argv[1], "rb");
    if (file == NULL)
    {
        trace("Could not open file:");
        trace(argv[1]);
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size != TEXTURE_FILE_SIZE)
    {
        trace("Wrongly formatted texture file.");
        return EXIT_FAILURE;
    }
    
    char * file_buf = malloc (file_size);
    if (file_buf == NULL)
    {
        perror(argv[1]);
        return EXIT_FAILURE;
    }

    fread( file_buf, 1, file_size, file);
    fclose(file);

    // Texture parameters.
    Uint32 seed;
    Uint16 octaves;
    Uint16 frequency;
    double persistence;
    Uint32 width;
    Uint8  threshold_red;
    Uint8  threshold_green;
    Uint8  threshold_blue;
    color  color1;
    color  color2;
    color  color3;
    Uint16 smoothing;

    char * option_ptr = file_buf;
#define READ_OPT(opt,size) memcpy(&(opt), option_ptr, size); option_ptr += size;

    READ_OPT(seed, 4);
    READ_OPT(octaves, 2);
    READ_OPT(frequency, 2);
    READ_OPT(persistence, 8);
    READ_OPT(width, 4);
    READ_OPT(threshold_red, 1);
    READ_OPT(threshold_green, 1);
    READ_OPT(threshold_blue, 1);
    READ_OPT(color1.red, 1);
    READ_OPT(color1.green, 1);
    READ_OPT(color1.blue, 1);
    READ_OPT(color2.red, 1);
    READ_OPT(color2.green, 1);
    READ_OPT(color2.blue, 1);
    READ_OPT(color3.red, 1);
    READ_OPT(color3.green, 1);
    READ_OPT(color3.blue, 1);
    READ_OPT(smoothing, 2);

    /* fprintf (stderr, "%ld\n", seed); */
    /* fprintf (stderr, "%ld\n", octaves); */
    /* fprintf (stderr, "%ld\n", frequency); */
    /* fprintf (stderr, "%lf\n", persistence); */
    /* fprintf (stderr, "%ld\n", width); */
    /* fprintf (stderr, "%ld\n", threshold_red); */
    /* fprintf (stderr, "%ld\n", threshold_green); */
    /* fprintf (stderr, "%ld\n", threshold_blue); */
    /* fprintf (stderr, "%ld\n", color1.red); */
    /* fprintf (stderr, "%ld\n", color1.green); */
    /* fprintf (stderr, "%ld\n", color1.blue); */
    /* fprintf (stderr, "%ld\n", color2.red); */
    /* fprintf (stderr, "%ld\n", color2.green); */
    /* fprintf (stderr, "%ld\n", color2.blue); */
    /* fprintf (stderr, "%ld\n", color3.red); */
    /* fprintf (stderr, "%ld\n", color3.green); */
    /* fprintf (stderr, "%ld\n", color3.blue); */
    /* fprintf (stderr, "%ld\n", smoothing); */


    // Création de layer
    struct layer *base;

    trace("Init.");

    /* The base layer is empty at the beginning. It will be generated upon a */
    /* random layer. */
    base = init_layer (width);
    if (!base)
    {
        trace("Init layer failed.");
        return 1;
    }


    trace("Random layer.");

    // Transformation via l'algorithme de Perlin.
    layer *random_layer = generate_random_layer (base, seed);
    save_bmp (random_layer, OUTPUT_RANDOM);
    generate_work_layer (frequency, octaves, persistence, base,
                         random_layer);

    trace("GS.");

    save_bmp (base, OUTPUT_GS);
    trace("RGB.");
    save_bmp_rgb (base, OUTPUT_RGB, threshold_red, threshold_green,
                  threshold_blue, color1, color2, color3);
    trace("Alt.");
    save_bmp_alt (base, OUTPUT_ALT, threshold_red, color1, color2);


    /* Smoothed version if option is non-zero. */
    if (smoothing != 0)
    {
        layer *layer_smoothed = smooth_layer (smoothing, base);

        save_bmp (layer_smoothed, OUTPUT_GS_SMOOTH);
        save_bmp_rgb (layer_smoothed, OUTPUT_RGB_SMOOTH, threshold_red,
                      threshold_green, threshold_blue, color1, color2,
                      color3);
        save_bmp_alt (layer_smoothed, OUTPUT_ALT_SMOOTH, threshold_red,
                      color1, color2);

        free_layer (layer_smoothed);
    }


    free(file_buf);
    return EXIT_SUCCESS;
}
