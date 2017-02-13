#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <stdint.h>
#include <time.h>

const size_t W = 256;
const size_t H = 382;
const size_t SZ = W * H * 3;

uint8_t monalisa[SZ];

const size_t SPEC_CNT = 300;
const size_t BEST_CNT = 2;

uint8_t specimen[SPEC_CNT][SZ];
size_t best_spec[BEST_CNT][SZ];
size_t best[BEST_CNT];
int step = 0;

void dump_best() {
  if (step % 100 != 0) {
    return;
  }

  printf("   Dumping step %i\r", step); fflush(stdout);
  for (size_t i = 0; i < 1; i++) {
    char fname[256] = {0};
    sprintf(fname, "out\\best_%.6i_%.2i.raw", step, i);
    FILE *f = fopen(fname, "wb");
    fwrite(specimen[best[i]], SZ, 1, f);
    fclose(f);
  }
}

void setpixel(int i, int x, int y, int r, int g, int b) {
  if(x < H && y < W) {
    specimen[i][x * W * 3 + y * 3 + 0] = (specimen[i][x * W * 3 + y * 3 + 0] + r) >> 1;
    specimen[i][x * W * 3 + y * 3 + 1] = (specimen[i][x * W * 3 + y * 3 + 1] + g) >> 1;
    specimen[i][x * W * 3 + y * 3 + 2] = (specimen[i][x * W * 3 + y * 3 + 2] + b) >> 1;
  }
}

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

void mutate() {
  for (size_t i = 0; i < SPEC_CNT; i++) {
    int x = rand() % (H - 1);
    int y = rand() % (H - 1);
    int r = rand() & 0xff;
    int g = rand() & 0xff;
    int b = rand() & 0xff;

    int a = (H*2 - step/32);
    int radius = (rand() % MAX(a, 10)) + 1;
    for(int n = -radius; n <= radius; n++) {
      for(int m = -radius; m <= radius; m++) {
        if(m * m + n * n <= radius * radius)
          setpixel(i, x + m, y + n, r, g, b);
      }
    }
  }
}

double score_me(uint8_t *sp) {
  double sc = 0.0;
  for (size_t j = 0; j < H; j++) {
    for (size_t i = 0; i < W; i++) {
      double r = sp[j * W * 3 + i * 3 + 0] - monalisa[j * W * 3 + i * 3 + 0];
      double g = sp[j * W * 3 + i * 3 + 1] - monalisa[j * W * 3 + i * 3 + 1];
      double b = sp[j * W * 3 + i * 3 + 2] - monalisa[j * W * 3 + i * 3 + 2];
      sc += r * r + g * g + b * b;
    }
  }

  return sc;
}

struct scores_st {
  double score;
  size_t idx;
};

int score_sort_fnc(const void *a, const void *b) {
  const scores_st *sa = (const scores_st*)a;
  const scores_st *sb = (const scores_st*)b;  

  if (sa->score > sb->score) {
    return 1;
  }

  if (sa->score < sb->score) {
    return -1;
  }  

  return 0;
}

void score() {
  static scores_st scores[SPEC_CNT];

  for (size_t i = 0; i < SPEC_CNT; i++) {
    scores[i].idx = i;
    scores[i].score = score_me(specimen[i]);
  }

  qsort(scores, SPEC_CNT, sizeof(scores_st), score_sort_fnc);

  for (size_t i = 0; i < BEST_CNT; i++) {
    best[i] = scores[i].idx;
    //printf("%i %i %f\n", i, best[i], scores[i].score);
  }
}

void cross() {
  for (size_t i = 0; i < BEST_CNT; i++) {
    memcpy(best_spec[i], specimen[best[i]], SZ);
  }

  for (size_t i = BEST_CNT; i < SPEC_CNT; i++) {
    memcpy(specimen[i], best_spec[i % BEST_CNT], SZ);
  }
}

int main(void) {
  //srand(time(NULL));

  FILE *f = fopen("mona_small.raw", "rb");
  fread(monalisa, SZ, 1, f);
  fclose(f);

  for (;; step++) {
    //puts("Stage 1");
    mutate();

    //puts("Stage 2");
    score();

    // Dump best.
    //puts("Stage 3");
    cross();
    dump_best();    

    ///break;
  }

  return 0;
}