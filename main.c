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
#define N 256
#define pi 3.14159265358979323846f

#define song "samples/Deftones - Be Quiet and Drive (Far Away).mp3"

typedef struct {
    float left;
    float right;
} Frame;

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


// New function to draw a cool rectangle
void DrawCoolRectangle(float x, float y, float width, float height, Color color) {
    DrawRectangle(x, y, width, height, color);
    DrawRectangleLines(x, y, width, height, ColorAlpha(WHITE, 0.3f));
    DrawCircle(x + width / 2, y, width / 4, ColorAlpha(WHITE, 0.2f));
}

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    InitWindow(screenWidth, screenHeight, "Enhanced Music Visualizer");
    SetTargetFPS(60);

    InitAudioDevice();
    Music music = LoadMusicStream(song);
    assert(music.stream.sampleSize == 32);
    assert(music.stream.channels == 2);

    SetMusicVolume(music, 0.5f); 
    PlayMusicStream(music);
    AttachAudioStreamProcessor(music.stream, callback);

    // Load font for text
    Font font = LoadFontEx("resources/fonts/monogram.ttf", 24, NULL, 0);

    // Create a render texture to act as a semi-transparent overlay
    RenderTexture2D overlay = LoadRenderTexture(screenWidth, screenHeight);

    while (!WindowShouldClose()) {
        UpdateMusicStream(music);

        if (IsKeyPressed(KEY_SPACE)) {
            if (IsMusicStreamPlaying(music)) {
                PauseMusicStream(music);
            } else {
                ResumeMusicStream(music);
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);

            // Draw semi-transparent grey background
            BeginTextureMode(overlay);
                DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(GRAY, 0.2f));
            EndTextureMode();
            DrawTextureRec(overlay.texture, (Rectangle){ 0, 0, screenWidth, -screenHeight }, (Vector2){ 0, 0 }, WHITE);

            // Draw visualizer bars
            float cell_width = (float)screenWidth / N;
            for (size_t i = 0; i < N; i++) {
                float t = amp(out[i]) / max_amp;
                Color barColor = ColorFromHSV(i * 360.0f / N, 0.8f, 0.9f);
                DrawCoolRectangle(i * cell_width, screenHeight - screenHeight * t, cell_width, screenHeight * t, barColor);
            }

            // Draw song title
            const char* songTitle = "Deftones - Be Quiet and Drive (Far Away)";
            Vector2 titleSize = MeasureTextEx(font, songTitle, 40, 2);
            DrawTextEx(font, songTitle, (Vector2){screenWidth/2 - titleSize.x/2, 20}, 40, 2, WHITE);

            // Draw song details
            char details[100];
            snprintf(details, sizeof(details), "Sample Rate: %u Hz | Channels: %d", music.stream.sampleRate, music.stream.channels);
            Vector2 detailsSize = MeasureTextEx(font, details, 20, 1);
            DrawRectangle(0, screenHeight - 40, screenWidth, 40, ColorAlpha(BLACK, 0.7f));
            DrawTextEx(font, details, (Vector2){screenWidth/2 - detailsSize.x/2, screenHeight - 30}, 20, 1, WHITE);

            // Draw play/pause status
            const char* status = IsMusicStreamPlaying(music) ? "Playing" : "Paused";
            DrawTextEx(font, status, (Vector2){10, 10}, 20, 1, WHITE);

        EndDrawing();
    }
    
    UnloadFont(font);
    UnloadRenderTexture(overlay);
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
