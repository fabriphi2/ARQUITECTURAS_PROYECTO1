#include "pgm.h"   /* siempre antes que cualquier otro #include */

/* ===== V0: version simple, sin optimizaciones =====
 * Cada funcion recibe la imagen de entrada y escribe la de salida (w*h bytes).
 * Recuerde (enunciado, seccion 2):
 *   - coeficientes y suma en float, sumando por filas (arriba->abajo, izq->der)
 *   - orillas: clampear cada indice por separado a [0, H-1] y [0, W-1]
 *   - salida = (unsigned char)(suma + 0.5f), despues de |suma| y min(255)
 */

static void blur(const unsigned char *in, unsigned char *out, int w, int h){
    for (int y = 0; y < h; y++){
        for (int x = 0; x < w; x++){
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
            out[y * w + x] = (unsigned char)(suma + 0.5f);
        }
    }
}

static void gauss(const unsigned char *in, unsigned char *out, int w, int h){
    const float K[5][5] = {
        {  1/273.f,  4/273.f,  7/273.f,  4/273.f,  1/273.f },
        {  4/273.f, 16/273.f, 26/273.f, 16/273.f,  4/273.f },
        {  7/273.f, 26/273.f, 41/273.f, 26/273.f,  7/273.f },
        {  4/273.f, 16/273.f, 26/273.f, 16/273.f,  4/273.f },
        {  1/273.f,  4/273.f,  7/273.f,  4/273.f,  1/273.f }
    };
    for (int y = 0; y < h; y++){
        for (int x = 0; x < w; x++){
            float suma = 0.0f;
            for (int i = -2; i <= 2; i++){
                int f = y + i;
                if (f < 0) f = 0;
                if (f > h - 1) f = h - 1;
                for (int j = -2; j <= 2; j++){
                    int c = x + j;
                    if (c < 0) c = 0;
                    if (c > w - 1) c = w - 1;
                    suma += K[i + 2][j + 2] * in[f * w + c];
                }
            }
            out[y * w + x] = (unsigned char)(suma + 0.5f);
        }
    }
}

static void sobel(const unsigned char *in, unsigned char *out, int w, int h){
    /* TODO: ventana 3x3 {-1 0 1; -2 0 2; -1 0 1}; primero valor absoluto, luego recorte a 255 */
    (void)in; (void)out; (void)w; (void)h;
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

    if (w <= 8 && h <= 8){               /* imagen de prueba: imprime la matriz */
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