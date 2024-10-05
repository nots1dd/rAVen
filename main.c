#include <assert.h>
#include <complex.h>
#include <gtk/gtk.h>
#include <libavformat/avformat.h>
#include <magic.h>
#include <math.h>
#include <signal.h>
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ARRAY_LEN(xs) sizeof(xs) / sizeof(xs[0])
#define N             (1 << 13)
#define pi            3.14159265358979323846f

/**************************************************
 * @COLOR PALETTE
 **************************************************/

#define GRUVBOX_BG \
  (Color) { 40, 40, 40, 255 } // #282828
#define GRUVBOX_FG \
  (Color) { 235, 219, 178, 255 } // #ebdbb2
#define GRUVBOX_YELLOW \
  (Color) { 250, 189, 47, 255 } // #fabd2f
#define GRUVBOX_BLUE \
  (Color) { 131, 165, 152, 255 } // #83a598
#define GRUVBOX_GREEN \
  (Color) { 184, 187, 38, 255 } // #b8bb26
#define GRUVBOX_RED \
  (Color) { 251, 73, 52, 255 } // #fb4934
#define GRUVBOX_ORANGE \
  (Color) { 254, 128, 25, 255 } // #fe8019
#define GRUVBOX_AQUA \
  (Color) { 142, 192, 124, 255 } // #8ec07c
#define GRUVBOX_PURPLE \
  (Color) { 211, 134, 155, 255 } // #d3869b

/*typedef struct*/
/*{*/
/*  float left;*/
/*  float right;*/
/*} Frame;*/

typedef enum
{
  STANDARD,
  PIXEL,
  WAVEFORM,
  STARBURST,
  RADIAL_BARS,
  NUM_MODES = 5
} VisualizationMode;

typedef struct
{
  char  title[128];
  char  artist[128];
  char  album[128];
  float duration; // returns in seconds
} MusicMetadata;

/********************************************************
 * $GLOBAL VARIABLES DECLARATION
 ********************************************************/

float             freqs[N];
float             global_frames[4800] = {0};
size_t            global_frames_count = 0;
float             in[N];
float complex     out[N];
float             max_amp;
float global_sample_rate;
char              selected_song[512];
VisualizationMode currentMode = STANDARD;
const char* helpCommands[]    = {"f            - Play a media file (GTK file dialog will open)\n",
                                 "<Space>      - Pause music\n",
                                 "m            - Toggle mute\n",
                                 "<UP-ARROW>   - Increase volume by 10%\n",
                                 "<DOWN-ARROW> - Decrease volume by 10%\n\n",
                                 "----------------- VISUAL MODES ---------------------\n\n",
                                 "v            - Cycle through visual modes (forward)\n",
                                 "b            - Cycle through visual modes (backward)\n",
                                 "? - Display the list of available commands"};

/*************************************************************
 *
 * @SIGNAL HANDLING 
 *
 * For graceful exits and better error handling of the cases:
 *
 * -> IOT Instruction / Core Dump 
 * -> Segmentation Fault 
 * 
 * The above the are the most common signals to look after in 
 * the case for rAVen
 *
 ************************************************************/

void rAVen_sig_abrt(int signal_num) {
    printf("[rAVen] Received exit signal.\n Goodbye!", signal_num);
    exit(signal_num);  // Exit gracefully
}

/***************************************
 *
 * $LOW_PASS_FILTER
 *
 * -> apply_low_pass_filter :: Applies a low-pass filter
 *    by zeroing out frequencies above the cutoff frequency.
 *
 * $NOTE
 *
 * The cutoff bin is calculated based on
 * the ratio of the cutoff frequency to the
 * sample rate, ensuring that only frequencies
 * below the cutoff remain.
 *
 ***************************************/

void apply_low_pass_filter(float complex out[], size_t n, float sample_rate, float cutoff_frequency)
{
  size_t cutoff_bin = (size_t)(cutoff_frequency / (sample_rate / (float)n));

  for (size_t i = cutoff_bin; i < n; i++)
  {
    out[i] = 0;
  }
}

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
 *  $WHY FFT?
 *
 *  We use FFT to break down audio signals into their frequency components, which is a more
 *  intuitive and informative way to visualize sound than looking at a time-domain waveform alone
 *
 ************************************************************/

void fft(float in[], size_t stride, float complex out[], size_t n, float sample_rate)
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
  fft(in, stride * 2, out, n / 2, sample_rate);
  fft(in + stride, stride * 2, out + n / 2, n / 2, sample_rate);

  for (size_t k = 0; k < n / 2; k++)
  {
    float         t = (float)k / n;
    float complex v = cexp(-2 * I * pi * t) * out[k + n / 2];
    float complex e = out[k];
    out[k]          = e + v;
    out[k + n / 2]  = e - v;
  }
  
  apply_low_pass_filter(out, n, sample_rate, 99999.0f);
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

  float(*fs)[2] = bufferData;

  for (size_t i = 0; i < frames; i++)
  {
    memmove(in, in + 1, (N - 1) * sizeof(in[0]));
    in[N - 1] = (fs[i][0] + fs[i][1]) / 2;
  }

  fft(in, 1, out, N, global_sample_rate);

  max_amp = 0.0f;
  for (size_t i = 0; i < frames; i++)
  {
    float a = amp(out[i]);
    if (max_amp < a)
      max_amp = a;
  }
}

int is_song_file(const char* filename)
{
  const char* extensions[] = {".mp3", ".wav", ".flac", ".aac", ".ogg", NULL};
  for (int i = 0; extensions[i] != NULL; i++)
  {
    if (strstr(filename, extensions[i]) != NULL)
    {
      return 1; // It is a song file
    }
  }
  return 0; // Not a song file
}

// Function to draw a cool rectangle (reused from earlier)
void DrawCoolRectangle(float x, float y, float width, float height, Color color)
{
  DrawRectangle(x, y, width, height, color);                        // Use passed color
  DrawRectangleLines(x, y, width, height, ColorAlpha(color, 0.3f)); // Gruvbox foreground
  DrawCircle(x + width / 2, y, width / 4, ColorAlpha(color, 0.2f)); // Aqua as an accent
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

void handleVisualization(float cell_width, const int screenHeight, const int screenWidth, size_t m)
{
  Vector2 center = {screenWidth / 2, screenHeight / 2}; // Calculate the center point for drawing
  float   step   = 0.4f;                                // [0.01 - 0.06 looks good ig]
  float   maxAmplitude = max_amp > 0 ? max_amp : 1;

  // Calculate amplitude for all points once
  float amplitudes[N];
  for (size_t i = 0; i < N; i++)
  {
    amplitudes[i] = amp(out[i]) / maxAmplitude; // Normalize amplitude
  }

  // Store previous amplitudes for smoothing
  static float previousAmplitudes[N] = {0};

  for (size_t i = 0; i < N - 1; i++)
  {
    if (amplitudes[i] > 0.01f)
    {
      switch (currentMode)
      {
        /*******************************************************
         *
         * @STANDARD MODE
         *
         * Draws rectangles representing amplitudes as bars.
         * The height of each rectangle corresponds to the amplitude.
         *
         *******************************************************/
        case STANDARD:
        {
          DrawCoolRectangle(i * cell_width, screenHeight - screenHeight * amplitudes[i],
                            cell_width * step, screenHeight * amplitudes[i], GRUVBOX_RED);
          break;
        }

        /*******************************************************
         *
         * @PIXEL MODE
         *
         * Similar to STANDARD, but with a larger step to create
         * a pixelated effect. This enhances the blocky appearance.
         *
         *******************************************************/
        case PIXEL:
        {
          step = 1.06f; // Adjust step for pixelated effect
          DrawCoolRectangle(i * cell_width, screenHeight - screenHeight * amplitudes[i],
                            cell_width * step, screenHeight * amplitudes[i], GRUVBOX_PURPLE);
          break;
        }

        /*******************************************************
         *
         * @WAVEFORM MODE
         *
         * Visualizes the amplitudes as a line waveform.
         * Connects the amplitude points with lines for smooth transitions.
         *
         *******************************************************/
        case WAVEFORM:
        {
          Vector2 start = {i * cell_width, center.y + (screenHeight / 2) * amplitudes[i]};
          Vector2 end   = {(i + 1) * cell_width, center.y + (screenHeight / 2) * amplitudes[i + 1]};
          DrawLineEx(start, end, 2.0f, GRUVBOX_BLUE);
          break;
        }

        /*******************************************************
         *
         * @STARBURST MODE
         *
         * Draws radial lines (rays) emanating from the center.
         * Each ray's length is determined by the amplitude, and
         * color varies based on the index.
         *
         *******************************************************/
        case STARBURST:
        {
          float   angle = i * 360.0f / m; // Calculate angle for each ray WRT freq range
          Vector2 end   = {center.x + cos(angle * DEG2RAD) * amplitudes[i] * (screenHeight / 2),
                           center.y + sin(angle * DEG2RAD) * amplitudes[i] * (screenHeight / 2)};

          // Select a color based on the index
          Color rayColor;
          switch (i % 6)
          {
            case 0:
              rayColor = GRUVBOX_YELLOW;
              break;
            case 1:
              rayColor = GRUVBOX_BLUE;
              break;
            case 2:
              rayColor = GRUVBOX_GREEN;
              break;
            case 3:
              rayColor = GRUVBOX_RED;
              break;
            case 4:
              rayColor = GRUVBOX_ORANGE;
              break;
            case 5:
              rayColor = GRUVBOX_PURPLE;
              break;
          }

          DrawLineEx(center, end, 2.0f, rayColor); // Draw the ray
          break;
        }

        /*******************************************************
         *
         * @RADIAL BARS MODE
         *
         * Draws bars radiating from the center, similar to STARBURST,
         * but using circular sections. The length of each bar is
         * based on the amplitude, and bars have a color scheme.
         *
         *******************************************************/
        case RADIAL_BARS:
        {
          float angle       = i * 360.0f / m;   // Calculate angle for each bar WRT audio freq range
          float innerRadius = screenHeight / 8; // Radius for inner circle
          float outerRadius = screenHeight / 4; // Base radius for bars
          float amplitudeScale = screenHeight / 4; // Scaling factor for amplitude

          // Draw the inner circle
          DrawCircle(center.x, center.y, innerRadius, GRUVBOX_FG);
          DrawCircleLines(center.x, center.y, innerRadius, GRUVBOX_FG);

          Vector2 start = {center.x + cos(angle * DEG2RAD) * outerRadius,
                           center.y + sin(angle * DEG2RAD) * outerRadius};

          // Use a smoothed amplitude value [by taking average of prev and current amps]
          float smoothedAmplitude =
            (previousAmplitudes[i] + amplitudes[i]) * 0.5; // Simple averaging
          previousAmplitudes[i] = smoothedAmplitude;       // Store for next frame

          Vector2 end = {
            center.x + cos(angle * DEG2RAD) * (outerRadius + smoothedAmplitude * amplitudeScale),
            center.y + sin(angle * DEG2RAD) * (outerRadius + smoothedAmplitude * amplitudeScale)};

          // Color selection
          Color barColor;
          switch (i % 6)
          {
            case 0:
              barColor = GRUVBOX_YELLOW;
              break;
            case 1:
              barColor = GRUVBOX_BLUE;
              break;
            case 2:
              barColor = GRUVBOX_GREEN;
              break;
            case 3:
              barColor = GRUVBOX_ORANGE;
              break;
            case 4:
              barColor = GRUVBOX_AQUA;
              break;
            case 5:
              barColor = GRUVBOX_PURPLE;
              break;
          }

          DrawLineEx(start, end, cell_width * step, barColor); // Draw the radial bar
          break;
        }
      }
    }
  }
}

void DrawHelpBox(bool showHelp, Font font, const int screenHeight, const int screenWidth)
{
  if (showHelp)
  {
    int boxWidth  = screenWidth / 2;
    int boxHeight = screenHeight / 2 + 7; // some padding
    int boxX      = (GetScreenWidth() - boxWidth) / 2;
    int boxY      = (GetScreenHeight() - boxHeight) / 2;

    // Draw the outer glowing rectangle using Gruvbox red for the border
    DrawRectangle(boxX - 10, boxY - 10, boxWidth + 20, boxHeight + 20, GRUVBOX_RED);
    DrawRectangle(boxX, boxY, boxWidth, boxHeight,
                  ColorAlpha(GRUVBOX_BG, 0.95f)); // Gruvbox background

    // Title with Gruvbox yellow
    DrawTextEx(font, "Help for rAVen:", (Vector2){boxX + 20, boxY + 20}, 30, 2, GRUVBOX_YELLOW);

    // Display help commands in Gruvbox foreground
    char helpText[1024];
    snprintf(helpText, sizeof(helpText), "Commands:\n");
    for (int i = 0; i < ARRAY_LEN(helpCommands); i++)
    {
      snprintf(helpText + strlen(helpText), sizeof(helpText) - strlen(helpText), "%s\n",
               helpCommands[i]);
    }
    DrawTextEx(font, helpText, (Vector2){boxX + 20, boxY + 60}, 20, 1, GRUVBOX_FG);
  }
}

void LimitText(char* dest, const char* src, int maxLength)
{
  if (strlen(src) > maxLength)
  {
    strncpy(dest, src, maxLength - 3); // Copy up to maxLength - 3 characters
    dest[maxLength - 3] = '\0';        // Ensure null termination
    strcat(dest, "...");               // Append "..." to indicate truncation
  }
  else
  {
    strcpy(dest, src); // Copy the full string if it fits within the limit
  }
}

void DrawSpaceTheme(Font font, Music music, MusicMetadata* metadata)
{
  // Constants
  int boxWidth  = 400; // Width of the text box
  int padding   = 40;  // Padding for the text inside the box
  int charLimit = 30;  // Maximum character limit for each line (adjust as needed)

  // Draw the outer glowing rectangle for space-themed effect
  DrawRectangle(15, 95, 410, 210, GRUVBOX_BLUE); // Outer glow effect
  DrawRectangle(20, 100, boxWidth, 200,
                ColorAlpha(GRUVBOX_BG, 0.85f)); // Main box with Gruvbox background

  // Draw the title with a Gruvbox-style glowing effect
  DrawTextEx(font, "Track Info", (Vector2){padding, 110}, 24, 2,
             GRUVBOX_YELLOW); // Gruvbox yellow for title

  // Create and display the text with metadata
  char title[128], artist[128], album[128];
  LimitText(title, metadata->title[0] ? metadata->title : "Unknown", charLimit);
  LimitText(artist, metadata->artist[0] ? metadata->artist : "Unknown", charLimit);
  LimitText(album, metadata->album[0] ? metadata->album : "Unknown", charLimit);

  char infoText[512];
  snprintf(infoText, sizeof(infoText),
           "Title: %s\nArtist: %s\nAlbum: %s\nSample Rate: %d Hz\nChannels: %d\nSample Size: "
           "%d-bit\nDuration: %.2f sec",
           title, artist, album, music.stream.sampleRate, music.stream.channels,
           music.stream.sampleSize, metadata->duration);

  // Display the metadata text with Gruvbox foreground color
  DrawTextEx(font, infoText, (Vector2){padding, 150}, 20, 1, GRUVBOX_FG);

  // Add additional space elements (stars, nebula effect, etc.)
  for (int i = 0; i < 50; i++)
  {
    DrawCircle(rand() % 450 + 20, rand() % 220 + 100, 1,
               ColorAlpha(GRUVBOX_FG, 0.5f)); // Stars using Gruvbox foreground color
  }

  // Add a glowing nebula at the bottom-right corner using Gruvbox colors
  DrawCircleGradient(380, 280, 50, ColorAlpha(GRUVBOX_AQUA, 0.2f),
                     ColorAlpha(GRUVBOX_PURPLE, 0.0f)); // Nebula glow using Gruvbox aqua and purple
}

void extract_metadata(const char* filename, MusicMetadata* metadata)
{
  AVFormatContext* fmt_ctx = NULL;

  // Open the audio file and read its header
  if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) < 0)
  {
    printf("Could not open file: %s\n", filename);
    return;
  }

  // Retrieve stream information
  if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
  {
    printf("Could not find stream information\n");
    avformat_close_input(&fmt_ctx);
    return;
  }

  AVDictionaryEntry* tag = NULL;

  // Extract metadata (title, artist, album)
  if ((tag = av_dict_get(fmt_ctx->metadata, "title", NULL, 0)))
  {
    strncpy(metadata->title, tag->value, sizeof(metadata->title) - 1);
    metadata->title[sizeof(metadata->title) - 1] = '\0';
  }
  else
  {
    strncpy(metadata->title, "Unknown Title", sizeof(metadata->title) - 1);
    metadata->title[sizeof(metadata->title) - 1] = '\0';
  }

  if ((tag = av_dict_get(fmt_ctx->metadata, "artist", NULL, 0)))
  {
    strncpy(metadata->artist, tag->value, sizeof(metadata->artist) - 1);
    metadata->artist[sizeof(metadata->artist) - 1] = '\0';
  }
  else
  {
    strncpy(metadata->artist, "Unknown Artist", sizeof(metadata->artist) - 1);
    metadata->artist[sizeof(metadata->artist) - 1] = '\0';
  }

  if ((tag = av_dict_get(fmt_ctx->metadata, "album", NULL, 0)))
  {
    strncpy(metadata->album, tag->value, sizeof(metadata->album) - 1);
    metadata->album[sizeof(metadata->album) - 1] = '\0';
  }
  else
  {
    strncpy(metadata->album, "Unknown Album", sizeof(metadata->album) - 1);
    metadata->album[sizeof(metadata->album) - 1] = '\0';
  }

  // Extract duration in seconds
  metadata->duration = fmt_ctx->duration / (float)AV_TIME_BASE;

  // Close the input context
  avformat_close_input(&fmt_ctx);
}

int main(int argc, char* argv[])
{
  /******************************
   * @SIGNAL DECLS
   ******************************/

  signal(SIGABRT, rAVen_sig_abrt);

  const int screenWidth  = 1280;
  const int screenHeight = 720;

  if (argc > 1)
  {
    if (is_song_file(argv[1]))
    {
      strcpy(selected_song, argv[1]);
      printf("Selected song: %s\n", selected_song);
    }
    else
    {
      printf("Error: %s is not a valid audio file.\n", argv[1]);
      return 1;
    }
  }
  else
  {
    printf(" [rAVen]\nNo arguments provided.\n");
    return 1;
  }

  InitWindow(screenWidth, screenHeight, "rAVen");
  SetTargetFPS(60);

  InitAudioDevice();
  Music         music    = LoadMusicStream(selected_song);
  MusicMetadata metadata = {0};
  extract_metadata(selected_song, &metadata);
  global_sample_rate = (float)music.stream.sampleRate;
  assert(music.stream.sampleSize == 32);
  assert(music.stream.channels == 2);

  float currentVolume = 0.8f;          // Volume control (initially set to full)
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
    if (IsKeyPressed(KEY_Q))
    {
      break;
      UnloadMusicStream(music);
      CloseAudioDevice();
      CloseWindow();
      return 0;
    }

    if (IsFileDropped())
    {
      PauseMusicStream(music);
      FilePathList droppedFiles = LoadDroppedFiles();
      printf("File dropped\n");
      if (droppedFiles.count > 0)
      {
        const char* file_path = droppedFiles.paths[0];
        printf("%s", droppedFiles.paths[0]);
        StopMusicStream(music);
        UnloadMusicStream(music);
        music = LoadMusicStream(file_path);
        PlayMusicStream(music);
        SetMusicVolume(music, currentVolume);
        extract_metadata(file_path, &metadata);
        AttachAudioStreamProcessor(music.stream, callback);
      }
      UnloadDroppedFiles(droppedFiles);
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
      if (is_song_file(selected_song))
      {
        UnloadMusicStream(music);
        music = LoadMusicStream(selected_song);
        PlayMusicStream(music);
        SetMusicVolume(music, currentVolume);
        extract_metadata(selected_song, &metadata);
        AttachAudioStreamProcessor(music.stream, callback);
      }
      else
      {
        printf("NOT A VALID SONG FILE\n");
        ResumeMusicStream(music);
      }
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
      isMuted = !isMuted;
      if (isMuted)
      {
        SetMusicVolume(music, 0.0);
      }
      else
      {
        SetMusicVolume(music, currentVolume);
      }
    }

    BeginDrawing();
    ClearBackground(BLACK);

    BeginTextureMode(overlay);
    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(GRAY, 0.2f));
    EndTextureMode();
    DrawTextureRec(overlay.texture, (Rectangle){0, 0, screenWidth, -screenHeight}, (Vector2){0, 0},
                   WHITE);

    /****************************************************************************
     *
     * @What is N?
     *
     * It is the total number of frequency bins we need for audio analysis
     *
     * I took 1<<13 (2 to the power 13) as it was the greatest power with the
     * optimal performance and actually looked very cool, overall this gave
     * the rAVen an actual AV experience.
     *
     * size_t m represents the frequency bands that will be visualized, so instead
     * of iterating over N (which is a large number), we visualize an audio freq
     * range instead
     *
     * I got step = 1.06f from an article online on conversion of frequencies to
     * visualizable audio
     *
     ****************************************************************************/
    size_t m    = 0;
    float  step = 1.06f;
    for (float f = 20.0f; f < N; f *= step)
    {
      m++;
    }
    float cell_width = (float)screenWidth / m;
    handleVisualization(cell_width, screenHeight, screenWidth, m);

    // Draw song title
    const char* mainTitle = "rAVen";
    Vector2     titleSize = MeasureTextEx(font, mainTitle, 40, 2);
    DrawTextEx(font, mainTitle, (Vector2){screenWidth / 2 - titleSize.x / 2, 20}, 40, 2,
               GRUVBOX_BLUE);

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
    DrawTextEx(font, status, (Vector2){10, 10}, 20, 1,
               IsMusicStreamPlaying(music) ? GRUVBOX_GREEN : GRUVBOX_RED);

    // Draw volume level
    char volumeBuffer[50];
    snprintf(volumeBuffer, sizeof(volumeBuffer), "Volume: %.0f%% %s", currentVolume * 100, isMuted ? "!" : "");
    DrawTextEx(font, volumeBuffer, (Vector2){10, 40}, 20, 1, GRUVBOX_AQUA);

    // Draw info button
    DrawRectangleRec(infoButton, showInfo ? GRUVBOX_ORANGE : GRUVBOX_PURPLE);
    DrawTextEx(font, "INFO", (Vector2){infoButton.x + 10, infoButton.y + 10}, 20, 1, WHITE);
    DrawRectangleRec(helpButton, showHelp ? GRUVBOX_ORANGE : GRUVBOX_PURPLE);
    DrawTextEx(font, "?", (Vector2){helpButton.x + 15, helpButton.y + 5}, 20, 1, WHITE);

    // Display info box if the button is toggled
    if (showInfo)
    {
      DrawSpaceTheme(font, music, &metadata);
    }
    if (showHelp)
    {
      DrawHelpBox(showHelp, font, screenHeight, screenWidth);
    }

    EndDrawing();
  }

  UnloadMusicStream(music);
  CloseAudioDevice();
  CloseWindow();

  return 0;
}
