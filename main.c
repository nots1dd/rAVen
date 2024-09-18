#include <assert.h>
#include <complex.h>
#include <gtk/gtk.h>
#include <math.h>
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libavformat/avformat.h>
#include <unistd.h>

#define ARRAY_LEN(xs) sizeof(xs) / sizeof(xs[0])
#define N             256
#define pi            3.14159265358979323846f

#define DEFAULT_SONG "/home/s1dd/Downloads/Songs/C418 - Wet Hands.mp3"

typedef struct
{
  float left;
  float right;
} Frame;

typedef enum
{
  STANDARD,
  WAVEFORM,
  STARBURST,
  RADIAL_BARS,
  NUM_MODES = 4
} VisualizationMode;

typedef struct {
    char title[128];
    char artist[128];
    char album[128];
    float duration;  // in seconds
} MusicMetadata;

/********************************************************
 * $GLOBAL VARIABLES DECLARATION
 ********************************************************/

float             freqs[N];
Frame             global_frames[4800] = {0};
size_t            global_frames_count = 0;
float             in[N];
float complex     out[N];
float             max_amp;
char              selected_song[512] = DEFAULT_SONG;
VisualizationMode currentMode        = STANDARD;
const char *helpCommands[] = {
    "f            - Play a media file (GTK file dialog will open)\n",
    "<Space>      - Pause music\n",
    "m            - Toggle mute\n",
    "<UP-ARROW>   - Increase volume by 10%",
    "<DOWN-ARROW> - Decrease volume by 10%\n\n",
    "----------------- VISUAL MODES ---------------------\n\n",
    "v            - Cycle through visual modes (forward)\n",
    "b            - Cycle through visual modes (backward)\n",
    "? - Display the list of available commands"
};

/*************************************************************
 *
 *  @FAST FOURIER TRANSFORM
 *
 *  Check out fft.c for a background on how we got this
 *
 *  Time complexity of FFT is O(nlogn)
 *
 *  $OVERVIEW
 *
 *  Traditional FOURIER TRANSFORM is O(n^2) but thanks to
 *  its symmetrical properties and the concept of divide and
 *  conquer (used in sorting methods like merge/quick sort)
 *
 *  So using divide and conquer to modify DFT, we get FFT,
 *  an insanely fast version of fourier transform that has
 *  complexity of O(n*logn)
 *
 ************************************************************/

void fft(float in[], size_t stride, float complex out[], size_t n)
{
  assert(n > 0);
  if (n == 1)
  {
    out[0] = in[0];
    return;
  }
  /********************************************
   *
   * @STRIDE
   *
   * A concept similar to index slicing in py
   *
   * In python :: arr[i::k] where k is the step
   *
   ********************************************/
  fft(in, stride * 2, out, n / 2);
  fft(in + stride, stride * 2, out + n / 2, n / 2);

  for (size_t k = 0; k < n / 2; k++)
  {
    float         t = (float)k / n;
    float complex v = cexp(-2 * I * pi * t) * out[k + n / 2];
    float complex e = out[k];
    out[k]          = e + v;
    out[k + n / 2]  = e - v;
  }
}

/***************************************
 *
 * $AMPLITUDE
 *
 * -> fabsf :: Calculates the absolute
 *    floating point of given float
 *
 * $NOTE
 *
 * creal(), cimag() by default give
 * doubles so we need to use
 * crealf(), cimagf()
 *
 ***************************************/

float amp(float complex z)
{
  float a = fabsf(crealf(z));
  float b = fabsf(cimagf(z));
  if (a < b)
    return b;
  return a;
}

void SwitchVisualizationModeForward() { currentMode = (currentMode + 1) % NUM_MODES; }

void SwitchVisualizationModeBackward() { currentMode = (currentMode - 1) % NUM_MODES; }

void callback(void* bufferData, unsigned int frames)
{
  if (frames < N)
    return;

  Frame* fs = bufferData;

  for (size_t i = 0; i < frames; i++)
  {
    in[i] = fs[i].left;
  }

  fft(in, 1, out, N);

  max_amp = 0.0f;
  for (size_t i = 0; i < frames; i++)
  {
    float a = amp(out[i]);
    if (max_amp < a)
      max_amp = a;
  }
}

// Function to draw a cool rectangle (reused from earlier)
void DrawCoolRectangle(float x, float y, float width, float height, Color color)
{
  DrawRectangle(x, y, width, height, color);
  DrawRectangleLines(x, y, width, height, ColorAlpha(WHITE, 0.3f));
  DrawCircle(x + width / 2, y, width / 4, ColorAlpha(WHITE, 0.2f));
}

// Function to check if the mouse is hovering over a rectangle (used for the info button)
bool IsMouseOverRectangle(Rectangle rect)
{
  Vector2 mouse = GetMousePosition();
  return CheckCollisionPointRec(mouse, rect);
}

// Function to open a GTK file dialog and update the selected_song path
void OpenFileDialog()
{
  GtkWidget*           dialog;
  GtkFileChooser*      chooser;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;

  // Initialize GTK only once, before the dialog
  if (!gtk_init_check(0, NULL))
  {
    printf("Failed to initialize GTK!\n");
    return;
  }

  dialog = gtk_file_chooser_dialog_new("Open File", NULL, action, "_Cancel", GTK_RESPONSE_CANCEL,
                                       "_Open", GTK_RESPONSE_ACCEPT, NULL);

  chooser = GTK_FILE_CHOOSER(dialog);

  // Process the dialog event in a non-blocking way
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char* file_name = gtk_file_chooser_get_filename(chooser);
    strncpy(selected_song, file_name, sizeof(selected_song) - 1);
    selected_song[sizeof(selected_song) - 1] = '\0'; // Null-terminate string
    g_free(file_name);
  }

  // Destroy the dialog widget after the response
  gtk_widget_destroy(dialog);

  // Ensure any pending GTK events are processed
  while (gtk_events_pending())
  {
    gtk_main_iteration();
  }
}

void handleVisualization(float cell_width, const int screenHeight, const int screenWidth)
{
  if (currentMode == STANDARD)
  {
    for (size_t i = 0; i < N; i++)
    {
      float t        = amp(out[i]) / max_amp;
      Color barColor = ColorFromHSV(i * 360.0f / N, 0.8f, 0.9f);
      DrawCoolRectangle(i * cell_width, screenHeight - screenHeight * t, cell_width,
                        screenHeight * t, barColor);
    }
  }
  else if (currentMode == WAVEFORM)
  {

    Vector2 center = {screenWidth / 2, screenHeight / 2};

    for (size_t i = 0; i < N; i++)
    {
      float amplitude = amp(out[i]) / max_amp;
      float radius    = amplitude * 200.0f;

      float angle = (2 * PI * i) / N;
      float x     = center.x + cosf(angle) * (200 + radius);
      float y     = center.y + sinf(angle) * (200 + radius);

      DrawCircle(x, y, 3.0f, ColorFromHSV(i * 360.0f / N, 0.8f, 0.9f));
    }
  }
  else if (currentMode == STARBURST)
  {
    Vector2 center = {screenWidth / 2, screenHeight / 2};

    for (size_t i = 0; i < N - 1; i++)
    {
      float amplitude1 = amp(out[i]) / max_amp;
      float amplitude2 = amp(out[i + 1]) / max_amp;

      float bar_length1 = amplitude1 * 200.0f;
      float bar_length2 = amplitude2 * 200.0f;

      float angle1 = (2 * PI * i) / N;
      float angle2 = (2 * PI * (i + 1)) / N;

      float x1 = center.x + cosf(angle1) * (50 + bar_length1);
      float y1 = center.y + sinf(angle1) * (50 + bar_length1);

      float x2 = center.x + cosf(angle2) * (50 + bar_length2);
      float y2 = center.y + sinf(angle2) * (50 + bar_length2);

      DrawLineEx((Vector2){x1, y1}, (Vector2){x2, y2}, 2.0f,
                 ColorFromHSV(i * 360.0f / N, 0.8f, 0.9f));
    }
  }
  else if (currentMode == RADIAL_BARS)
  {
    Vector2 center = {screenWidth / 2, screenHeight / 2};

    for (size_t i = 0; i < N; i++)
    {
      float amplitude  = amp(out[i]) / max_amp;
      float bar_length = amplitude * 200.0f;

      float angle = (2 * PI * i) / N;
      float x1    = center.x + cosf(angle) * 50; // Inner radius of 50 pixels
      float y1    = center.y + sinf(angle) * 50;
      float x2    = center.x + cosf(angle) * (50 + bar_length);
      float y2    = center.y + sinf(angle) * (50 + bar_length);

      DrawLineEx((Vector2){x1, y1}, (Vector2){x2, y2}, 3.0f,
                 ColorFromHSV(i * 360.0f / N, 0.8f, 0.9f));
    }
  }
}

void DrawHelpBox(bool showHelp, Font font, const int screenHeight, const int screenWidth) {
    if (showHelp) {
        // Define the dimensions for the larger help box
        int boxWidth = screenWidth/2;
        int boxHeight = screenHeight/2;
        int boxX = (GetScreenWidth() - boxWidth) / 2;  // Center the box horizontally
        int boxY = (GetScreenHeight() - boxHeight) / 2; // Center the box vertically

        // Draw the outer glowing rectangle for space-themed effect
        DrawRectangle(boxX - 10, boxY - 10, boxWidth + 20, boxHeight + 20, DARKBLUE);  // Outer glow effect
        DrawRectangle(boxX, boxY, boxWidth, boxHeight, ColorAlpha(BLACK, 0.85f)); // Main box

        // Draw the title with a futuristic font and glowing effect
        DrawTextEx(font, "Help", (Vector2){boxX + 20, boxY + 20}, 30, 2, SKYBLUE);

        // Create and display the text with commands
        char helpText[1024];  // Increased size to accommodate more text
        snprintf(helpText, sizeof(helpText), "Commands:\n");

        // Append each command to the help text
        for (int i = 0; i < ARRAY_LEN(helpCommands); i++) {
            snprintf(helpText + strlen(helpText), sizeof(helpText) - strlen(helpText),
                     "%s\n", helpCommands[i]);
        }

        // Display help text with a white glow effect
        DrawTextEx(font, helpText, (Vector2){boxX + 20, boxY + 60}, 20, 1, WHITE);
    }
}

void extract_metadata(const char *filename, MusicMetadata *metadata) {
    AVFormatContext *fmt_ctx = NULL;

    // Open the audio file and read its header
    if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) < 0) {
        printf("Could not open file: %s\n", filename);
        return;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        printf("Could not find stream information\n");
        avformat_close_input(&fmt_ctx);
        return;
    }

    AVDictionaryEntry *tag = NULL;

    // Extract metadata (title, artist, album)
    if ((tag = av_dict_get(fmt_ctx->metadata, "title", NULL, 0)))
        strncpy(metadata->title, tag->value, sizeof(metadata->title) - 1);
    if ((tag = av_dict_get(fmt_ctx->metadata, "artist", NULL, 0)))
        strncpy(metadata->artist, tag->value, sizeof(metadata->artist) - 1);
    if ((tag = av_dict_get(fmt_ctx->metadata, "album", NULL, 0)))
        strncpy(metadata->album, tag->value, sizeof(metadata->album) - 1);

    // Get the duration of the track in seconds
    metadata->duration = (float)fmt_ctx->duration / AV_TIME_BASE;

    // Close the input file and free the context
    avformat_close_input(&fmt_ctx);
}


int main()
{
  const int screenWidth  = 1280;
  const int screenHeight = 720;

  InitWindow(screenWidth, screenHeight, "rAVen");
  SetTargetFPS(60);

  InitAudioDevice();
  Music music = LoadMusicStream(selected_song);
  MusicMetadata metadata = {0};
  extract_metadata(selected_song, &metadata);
  assert(music.stream.sampleSize == 32);
  assert(music.stream.channels == 2);

  float currentVolume = 1.0f;          // Volume control (initially set to full)
  float lastVolume    = currentVolume; // Used for toggling mute/unmute state
  bool  isMuted       = false;

  SetMusicVolume(music, currentVolume);
  PlayMusicStream(music);
  AttachAudioStreamProcessor(music.stream, callback);

  Font            font    = LoadFontEx("resources/fonts/monogram.ttf", 24, NULL, 0);
  RenderTexture2D overlay = LoadRenderTexture(screenWidth, screenHeight);

  // Button properties for the info button
  Rectangle infoButton = {screenWidth - 100, 20, 80, 40};
  Rectangle helpButton = {screenWidth - 100, 80, 60, 30};
  bool      showInfo   = false; // Toggle to display info box
  bool      showHelp   = false;

  while (!WindowShouldClose())
  {
    UpdateMusicStream(music);

    if (IsKeyPressed(KEY_SPACE))
    {
      if (IsMusicStreamPlaying(music))
      {
        PauseMusicStream(music);
      }
      else
      {
        ResumeMusicStream(music);
      }
    }

    // Detect if the user clicks on the info button
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && IsMouseOverRectangle(infoButton))
    {
      showInfo = !showInfo;
    }

    // Press 'F' key to open the file chooser
    if (IsKeyPressed(KEY_F))
    {
      PauseMusicStream(music);
      OpenFileDialog();
      UnloadMusicStream(music);
      music = LoadMusicStream(selected_song);
      PlayMusicStream(music);
      extract_metadata(selected_song, &metadata);
      AttachAudioStreamProcessor(music.stream, callback);
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && IsMouseOverRectangle(helpButton))
    {
        showHelp = !showHelp;
    }

    // Volume controls
    if (IsKeyPressed(KEY_UP))
    {
      currentVolume += 0.1f;
      if (currentVolume > 1.0f)
        currentVolume = 1.0f; // Max volume
      SetMusicVolume(music, currentVolume);
      isMuted = false;
    }
    if (IsKeyPressed(KEY_DOWN))
    {
      currentVolume -= 0.1f;
      if (currentVolume < 0.0f)
        currentVolume = 0.0f; // Min volume (mute)
      SetMusicVolume(music, currentVolume);
      isMuted = false;
    }
    if (IsKeyPressed(KEY_V))
    {
      SwitchVisualizationModeForward();
    }
    if (IsKeyPressed(KEY_B))
    {
      SwitchVisualizationModeBackward();
    }
    if (IsKeyPressed(KEY_M))
    {
      if (isMuted)
      {
        currentVolume = lastVolume;
        isMuted       = false;
      }
      else
      {
        lastVolume    = currentVolume;
        currentVolume = 0.0f; // Mute
        isMuted       = true;
      }
      SetMusicVolume(music, currentVolume);
    }

    BeginDrawing();
    ClearBackground(BLACK);

    BeginTextureMode(overlay);
    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(GRAY, 0.2f));
    EndTextureMode();
    DrawTextureRec(overlay.texture, (Rectangle){0, 0, screenWidth, -screenHeight}, (Vector2){0, 0},
                   WHITE);

    float cell_width = (float)screenWidth / N;
    handleVisualization(cell_width, screenHeight, screenWidth);

    // Draw song title
    const char* songTitle = "rAVen";
    Vector2     titleSize = MeasureTextEx(font, songTitle, 40, 2);
    DrawTextEx(font, songTitle, (Vector2){screenWidth / 2 - titleSize.x / 2, 20}, 40, 2, WHITE);

    // Draw song details
    float totalDuration = GetMusicTimeLength(music);
    float currentTime   = GetMusicTimePlayed(music);

    char timeBuffer[100];
    snprintf(timeBuffer, sizeof(timeBuffer), "%.2f / %.2f sec", currentTime, totalDuration);
    Vector2 detailsSize = MeasureTextEx(font, timeBuffer, 20, 1);
    DrawRectangle(0, screenHeight - 40, screenWidth, 40, ColorAlpha(BLACK, 0.7f));
    DrawTextEx(font, timeBuffer, (Vector2){screenWidth / 2 - detailsSize.x / 2, screenHeight - 30},
               20, 1, WHITE);

    // Draw play/pause status
    const char* status = IsMusicStreamPlaying(music) ? "Playing" : "Paused";
    DrawTextEx(font, status, (Vector2){10, 10}, 20, 1, WHITE);

    // Draw volume level
    char volumeBuffer[50];
    snprintf(volumeBuffer, sizeof(volumeBuffer), "Volume: %.0f%%", currentVolume * 100);
    DrawTextEx(font, volumeBuffer, (Vector2){10, 40}, 20, 1, WHITE);

    // Draw info button
    DrawRectangleRec(infoButton, showInfo ? ORANGE : GRAY);
    DrawTextEx(font, "INFO", (Vector2){infoButton.x + 10, infoButton.y + 10}, 20, 1, WHITE);
    DrawRectangleRec(helpButton, showHelp ? ORANGE : GRAY);
    DrawTextEx(font, "?", (Vector2){helpButton.x + 15, helpButton.y + 5}, 20, 1, WHITE);

    // Display info box if the button is toggled
    if (showInfo) {
        // Draw the outer glowing rectangle for space-themed effect
        DrawRectangle(15, 95, 410, 210, DARKBLUE);  // Outer glow effect
        DrawRectangle(20, 100, 400, 200, ColorAlpha(BLACK, 0.85f)); // Main box

        // Draw the title with a futuristic font and glowing effect
        DrawTextEx(font, "Track Info", (Vector2){40, 110}, 24, 2, SKYBLUE);

        // Create and display the text with more metadata
        char infoText[512];
        snprintf(infoText, sizeof(infoText),
                 "Title: %s\nArtist: %s\nAlbum: %s\n\nSample Rate: %d Hz\nChannels: %d\nSample Size: %d-bit\nDuration: %.2f sec",
                 metadata.title[0] ? metadata.title : "Unknown",
                 metadata.artist[0] ? metadata.artist : "Unknown",
                 metadata.album[0] ? metadata.album : "Unknown",
                 music.stream.sampleRate,
                 music.stream.channels,
                 music.stream.sampleSize,
                 metadata.duration);

        // Display metadata text with a white glow effect
        DrawTextEx(font, infoText, (Vector2){40, 150}, 20, 1, WHITE);

        // Add additional space elements (stars, nebula effect, etc.)
        for (int i = 0; i < 50; i++) {
            DrawCircle(rand() % 450 + 20, rand() % 220 + 100, 1, ColorAlpha(WHITE, 0.5f)); // Stars in the background
        }

        // Add a glowing nebula at the bottom-right corner
        DrawCircleGradient(380, 280, 50, ColorAlpha(SKYBLUE, 0.2f), ColorAlpha(PURPLE, 0.0f)); // Nebula glow
    }
    if (showHelp) {
      DrawHelpBox(showHelp, font, screenHeight, screenWidth);
    }

    EndDrawing();
  }

  UnloadMusicStream(music);
  CloseAudioDevice();
  CloseWindow();

  return 0;
}
