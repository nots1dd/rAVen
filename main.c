#include <stdio.h>
#include <raylib.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <complex.h>
#include <string.h>

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])
//#define song "C418 - Mice On Venus.mp3"
#define N 256
#define pi 3.14159265358979323846f

#define song "Deftones - Be Quiet and Drive (Far Away).mp3"

/*
 * @RAYLIB Samples 
 * According to the example given in their repo,
 * SAMPLES are stored internally as a float
 */

typedef struct {
    float left;
    float right;
} Frame;

/*
 * @GLOBAL FRAMES DECLARATION
 */

float freqs[N];
Frame global_frames[4800] = {0};
size_t global_frames_count = 0;
float in[N];
float complex out[N];
float max_amp;

void fft(float in[], size_t stride, float complex out[], size_t n) {
  /*
   * @STRIDE => Similar to python's index slicing [Ex: in[0::2]]
   */

  assert(n > 0);
  
  /*
   * $BASE CASE
   */
  if (n == 1) {
    out[0] = in[0];
    return;
  }

  fft(in, stride*2, out, n/2);
  fft(in + stride, stride*2, out + n/2, n/2);

  for (size_t k=0;k<n/2;k++) {
    float t = (float)k/n;
    float complex v = cexp(-2*I*pi*t)*out[k+n/2];
    float complex e = out[k];
    out[k]     = e + v;
    out[k+n/2] = e - v;
  }
}

void callback_wave(void *bufferData, unsigned int frames) {
  size_t capacity = ARRAY_LEN(global_frames);
  /*
   * @OFFSETTING FRAMES
   */
  /*
   * @ 1. Append global_frames
   *   2. Ran out of capacity
   *   3. ELSE
   */
  if (frames <= capacity - global_frames_count) {
    memcpy(global_frames+global_frames_count, bufferData, sizeof(Frame)*frames);
    global_frames_count += frames;
  } else if (frames <= capacity) {
    memmove(global_frames, global_frames + frames, sizeof(Frame)*(capacity-frames));
    memcpy(global_frames + (capacity - frames), bufferData, sizeof(Frame)*frames);
    // global_frames_count stays the same
  } else {
    /*
     * @ELSE 
     * Copy all global frames into the bufferData as per the capacity 
     * (entirety of global_frames will NOT be copied into bufferData as bufferData is smaller in size)
     */
    memcpy(global_frames, bufferData, sizeof(Frame)*capacity);
    global_frames_count = capacity;
  } 
}

float amp(float complex z) {
  float a = fabsf(crealf(z)); // crealf returns float by default creal returns double
  float b = fabsf(crealf(z));
  if (a < b) return b;
  return a;
}

void callback(void *bufferData, unsigned int frames) {
  if (frames < N) return;
  
  Frame *fs  = bufferData;

  for (size_t i=0;i<frames;i++) {
    in[i] = fs[i].left;
  }

  fft(in, 1, out, N);
  
  max_amp = 0.0f;
  for (size_t i=0;i<frames;i++) {
    float a = amp(out[i]);
    if (max_amp < a) max_amp = a;
  }
}

int main() {
    
    InitWindow(800, 600, "Testing");
    SetTargetFPS(60);

    InitAudioDevice();
    Music music = LoadMusicStream(song);
    assert(music.stream.sampleSize == 32);
    assert(music.stream.channels == 2);
    printf("framecount = %u\n", music.frameCount);
    printf("samplerate = %u\n", music.stream.sampleRate);
    printf("samplesize = %u\n", music.stream.sampleSize);
    printf("channels = %u\n", music.stream.channels);
    /*
     * @INFO
     * PlayMusicStream(Music music) does not load the entire song to the memory
     * You will need to keep updating the streams in an event loop
     */
    SetMusicVolume(music, 0.5f); 
    PlayMusicStream(music);
    AttachAudioStreamProcessor(music.stream, callback);
    /*
     * @Event loop
     */
    while (!WindowShouldClose())
    {
        UpdateMusicStream(music);

        if (IsKeyPressed(KEY_SPACE)) {
            if (IsMusicStreamPlaying(music)) {
                PauseMusicStream(music);
            } else {
                ResumeMusicStream(music);
            }
        }
        int w = GetRenderWidth();
        int h = GetRenderHeight();
        BeginDrawing();
        ClearBackground(GREEN);
        float cell_width = (float)w/N;
        for (size_t i=0;i<N;i++) {
          float t = amp(out[i])/max_amp;
          DrawRectangle(i*cell_width, h/2 - h/2*t, cell_width, h/2*t, BLUE); 
        } 
        /*if (IsMusicStreamPlaying(music)) {*/
        /*    ClearBackground(GREEN);*/
        /*} else {*/
        /*  ClearBackground(RED);*/
        /*}*/
        EndDrawing();
    }
    
    return 0;
}
