#include "pgm.h"

static const float KG[5][5] = {
    {  1/273.f,  4/273.f,  7/273.f,  4/273.f,  1/273.f },
    {  4/273.f, 16/273.f, 26/273.f, 16/273.f,  4/273.f },
    {  7/273.f, 26/273.f, 41/273.f, 26/273.f,  7/273.f },
    {  4/273.f, 16/273.f, 26/273.f, 16/273.f,  4/273.f },
    {  1/273.f,  4/273.f,  7/273.f,  4/273.f,  1/273.f }
};

static const float KS[3][3] = {
    { -1, 0, 1 },
    { -2, 0, 2 },
    { -1, 0, 1 }
};

static unsigned char blur_px(const unsigned char *in, int w, int h, int y, int x){
    float suma = 0.0f;
    for (int i = -1; i <= 1; i++){
        int f = y + i;
        if (f < 0) f = 0;
        if (f > h - 1) f = h - 1;
        for (int j = -1; j <= 1; j++){
            int c = x + j;
            if (c < 0) c = 0;
            if (c > w - 1) c = w - 1;
            suma += (1/9.f) * in[f * w + c];
        }
    }
    return (unsigned char)(suma + 0.5f);
}

static unsigned char gauss_px(const unsigned char *in, int w, int h, int y, int x){
    float suma = 0.0f;
    for (int i = -2; i <= 2; i++){
        int f = y + i;
        if (f < 0) f = 0;
        if (f > h - 1) f = h - 1;
        for (int j = -2; j <= 2; j++){
            int c = x + j;
            if (c < 0) c = 0;
            if (c > w - 1) c = w - 1;
            suma += KG[i + 2][j + 2] * in[f * w + c];
        }
    }
    return (unsigned char)(suma + 0.5f);
}

static unsigned char sobel_px(const unsigned char *in, int w, int h, int y, int x){
    float suma = 0.0f;
    for (int i = -1; i <= 1; i++){
        int f = y + i;
        if (f < 0) f = 0;
        if (f > h - 1) f = h - 1;
        for (int j = -1; j <= 1; j++){
            int c = x + j;
            if (c < 0) c = 0;
            if (c > w - 1) c = w - 1;
            suma += KS[i + 1][j + 1] * in[f * w + c];
        }
    }
    if (suma < 0) suma = -suma;
    if (suma > 255) suma = 255;
    return (unsigned char)(suma + 0.5f);
}

static void blur(const unsigned char * restrict in, unsigned char * restrict out, int w, int h){
    for (int y = 0; y < h; y++){
        const unsigned char *r[3];
        for (int i = 0; i < 3; i++){
            int f = y + i - 1;
            if (f < 0) f = 0;
            if (f > h - 1) f = h - 1;
            r[i] = in + f * w;
        }
        unsigned char *o = out + y * w;

        o[0] = blur_px(in, w, h, y, 0);
        for (int x = 1; x < w - 1; x++){
            float suma = 0.0f;
            for (int i = 0; i < 3; i++)
                for (int j = -1; j <= 1; j++)
                    suma += (1/9.f) * r[i][x + j];
            o[x] = (unsigned char)(suma + 0.5f);
        }
        o[w - 1] = blur_px(in, w, h, y, w - 1);
    }
}

static void gauss(const unsigned char * restrict in, unsigned char * restrict out, int w, int h){
    for (int y = 0; y < h; y++){
        const unsigned char *r[5];
        for (int i = 0; i < 5; i++){
            int f = y + i - 2;
            if (f < 0) f = 0;
            if (f > h - 1) f = h - 1;
            r[i] = in + f * w;
        }
        unsigned char *o = out + y * w;

        o[0] = gauss_px(in, w, h, y, 0);
        o[1] = gauss_px(in, w, h, y, 1);
        for (int x = 2; x < w - 2; x++){
            float suma = 0.0f;
            for (int i = 0; i < 5; i++)
                for (int j = -2; j <= 2; j++)
                    suma += KG[i][j + 2] * r[i][x + j];
            o[x] = (unsigned char)(suma + 0.5f);
        }
        o[w - 2] = gauss_px(in, w, h, y, w - 2);
        o[w - 1] = gauss_px(in, w, h, y, w - 1);
    }
}

static void sobel(const unsigned char * restrict in, unsigned char * restrict out, int w, int h){
    for (int y = 0; y < h; y++){
        const unsigned char *r[3];
        for (int i = 0; i < 3; i++){
            int f = y + i - 1;
            if (f < 0) f = 0;
            if (f > h - 1) f = h - 1;
            r[i] = in + f * w;
        }
        unsigned char *o = out + y * w;

        o[0] = sobel_px(in, w, h, y, 0);
        for (int x = 1; x < w - 1; x++){
            float suma = 0.0f;
            for (int i = 0; i < 3; i++)
                for (int j = -1; j <= 1; j++)
                    suma += KS[i][j + 1] * r[i][x + j];
            if (suma < 0) suma = -suma;
            if (suma > 255) suma = 255;
            o[x] = (unsigned char)(suma + 0.5f);
        }
        o[w - 1] = sobel_px(in, w, h, y, w - 1);
    }
}

typedef void (*filtro_fn)(const unsigned char *, unsigned char *, int, int);

int main(int argc, char **argv){
    if (argc < 3){
        fprintf(stderr, "uso: %s <blur|gauss|sobel> <imagen.pgm> [salida.pgm]\n", argv[0]);
        return 1;
    }
    filtro_fn f = NULL;
    if      (!strcmp(argv[1], "blur"))  f = blur;
    else if (!strcmp(argv[1], "gauss")) f = gauss;
    else if (!strcmp(argv[1], "sobel")) f = sobel;
    else { fprintf(stderr, "filtro desconocido: %s\n", argv[1]); return 1; }

    int w, h;
    unsigned char *in  = leer_pgm(argv[2], &w, &h);
    unsigned char *out = (unsigned char *)calloc((size_t)w * h, 1);

    enum { REPS = 5 };
    double t[REPS];
    for (int r = 0; r < REPS; r++){
        double t0 = ahora();
        f(in, out, w, h);
        t[r] = ahora() - t0;
    }

    if (w <= 8 && h <= 8){
        for (int y = 0; y < h; y++){
            for (int x = 0; x < w; x++) printf("%4d", out[y * w + x]);
            printf("\n");
        }
    }
    printf("%s %s %dx%d  mediana %.6f s  checksum %" PRIu64 "\n",
           argv[1], argv[2], w, h, mediana(t, REPS), checksum(out, (size_t)w * h));

    if (argc > 3) escribir_pgm(argv[3], out, w, h);
    free(in); free(out);
    return 0;
}
