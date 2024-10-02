#include <stdio.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../lib/libraven.h"

/****************************************
 * main.c
 *
 * This is the main program. It dynamically loads the library (libraven.so)
 * at runtime using , retrieves the function symbols using ,
 * and can reload the library during runtime, enabling "hot reloading".
 *
 * Key functions:
 * - reload_func: Loads/reloads the dynamic library and retrieves function symbols.
 * - main: Runs an event loop that listens for user input to either reload
 *   the library or execute the functions dynamically loaded from the library.
 ****************************************/

#define LIBEASC "../lib/libraven.so"
#define RELOAD_SCRIPT "../lib/recompile.sh"  // Script to recompile the library and program

#define HELPER_STRING "Welcome to easC!:\nKEYBINDS:\n\n1. 'r' -- Hot Reload Project\n2. 'c' -- Clear Screen\n3. 'q' -- Quit easC workflow\n\n> "

/****************************************
 * Global Variables:
 * - lib_name: The name of the shared library file.
 * - libplug: Handle for the dynamically loaded library.
 * - Function pointers: All called and defined using X MACROS.
 ****************************************/
const char *lib_name = LIBEASC;
void *libplug = NULL;

/***********************************
 *
 * @X_MACROS DEF 
 *
 * Defining all the typdefs to NULL
 *
 ***********************************/

#ifdef EASC_DYNC
#define EASC(name) name##_t *name = NULL;
#else 
#define EASC(name) name##_t name;
#endif
EASC_FUNC_LIST
#undef EASC

/****************************************
 * reload_func:
 * Dynamically loads (or reloads) the shared library and retrieves the symbols
 * for the functions to be called dynamically. Uses  to open the library
 * and  to locate function symbols. If any errors occur, the function
 * prints an error message and returns false.
 ****************************************/

int is_song_file_main(const char* filename)
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

#ifdef EASC_DYNC
bool reload_func() {
    /* Close the previously opened library if it exists */
    if (libplug != NULL) {
        dlclose(libplug);
    }

    /* Load the shared library (libeasc.so) */
    libplug = dlopen(lib_name, RTLD_NOW);
    if (libplug == NULL) {
        fprintf(stderr, "Couldn't load %s: %s\n", lib_name, dlerror());
        return false;
    }
    
    #define EASC(name) \       
      name = dlsym(libplug, #name); \         
      if (name == NULL) { \
        fprintf(stderr, "Couldn't find %s symbol: %s\n", \
                #name, dlerror()); \
        return false; \
      }
    EASC_FUNC_LIST 
    #undef EASC 

    /* Successfully loaded the library and retrieved all symbols */
    return true;
}
#else 
#define reload_func() true
#endif

/****************************************
 * main:
 * The main function runs an event loop that waits for user input.
 * If the user presses 'r', the reload script (reload.sh) is executed
 * to recompile the library, and the library is reloaded to apply changes.
 * Press 'q' to quit the program.
 ****************************************/
int main(int argc, char* argv[]) {
    /* Load the library and retrieve the function symbols initially */
    if (!reload_func()) {
        return 1;
    }

    /* Initialize the library (call test_init) */
    const int screenWidth  = 1280;
    const int screenHeight = 720;
    char selected_song[512];
    bool easC_break = false;

    if (argc > 1)
    {
      if (is_song_file_main(argv[1]))
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
    easC_init(selected_song);

    /* Event loop: Continue until 'q' is pressed */
    while (!WindowShouldClose()) {
        printf("%s", HELPER_STRING);
        
        /* If 'r' is pressed, reload the library */
        if (IsKeyPressed(KEY_R)) {
            // system(RELOAD_SCRIPT);  // Execute the reload script
            void *state = easC_pre_reload();
            if (!reload_func()) return 1;  // Reload the library and symbols
            easC_post_reload(state);
        }

        /* Call the update function (if 'q' is not pressed) */
        printf("--------------------------------\n\n");
        easC_update();
        printf("\n--------------------------------\n");
        /* Simple clear the screen */

        if (IsKeyPressed(KEY_C)) {
          system("clear");
        }
    } 

    /* Close the library before exiting */
    #ifdef EASC_DYNC
    dlclose(libplug);
    #else 
    printf("Exiting statically...\n");
    #endif
    return 0;
}
