#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <time.h>
#include "sdl_wrapper.h"

#define WINDOW_TITLE "Attack of the Circles"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 500
#define MS_PER_S 1e3

/**
 * The coordinate at the center of the screen.
 */
Vector center;
/**
 * The coordinate difference from the center to the top right corner.
 */
Vector max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
KeyHandler key_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

void *aux_data = NULL;

Vector camera;

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT: return LEFT_ARROW;
        case SDLK_UP: return UP_ARROW;
        case SDLK_RIGHT: return RIGHT_ARROW;
        case SDLK_DOWN: return DOWN_ARROW;
        default:
            // Only process 7-bit ASCII characters
            return key == (SDL_Keycode) (char) key ? key : '\0';
    }
}

void sdl_init(Vector min, Vector max) {
    // Check parameters
    assert(min.x < max.x);
    assert(min.y < max.y);

    camera = VEC_ZERO;
    center = vec_multiply(0.5, vec_add(min, max)),
    max_diff = vec_subtract(max, center);
    SDL_Init(SDL_INIT_EVERYTHING);
    if (TTF_Init() < 0){
      printf("TTF did not init.");
    }
    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );
    renderer = SDL_CreateRenderer(window, -1, 0);
}

void write_font(const char* file, int ptsize, const char* text, SDL_Color color,
  Vector top_left_corner, double width, double height){
    TTF_Font *font = TTF_OpenFont(file, ptsize);
    SDL_Surface *surface_text = TTF_RenderUTF8_Blended(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface_text);
    SDL_Rect message_rect;
    message_rect.x = top_left_corner.x;
    message_rect.y = top_left_corner.y;
    message_rect.w = width;
    message_rect.h = height;
    //SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    //SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &message_rect);
    TTF_CloseFont(font);
    SDL_FreeSurface(surface_text);
    SDL_DestroyTexture(texture);
}

Vector get_min(void){
  return vec_subtract(max_diff, center);
}

Vector get_max(void){
  return vec_subtract(center, max_diff);
}

bool sdl_is_done() {
    SDL_Event *event = malloc(sizeof(*event));
    assert(event);
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_QUIT:
                TTF_Quit();
                free(event);
                return true;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // Skip the keypress if no handler is configured
                // or an unrecognized key was pressed
                if (!key_handler) break;
                char key = get_keycode(event->key.keysym.sym);
                if (!key) break;

                double timestamp = event->key.timestamp;
                if (!event->key.repeat) {
                    key_start_timestamp = timestamp;
                }
                KeyEventType type =
                    event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
                double held_time =
                    (timestamp - key_start_timestamp) / MS_PER_S;
                key_handler(key, type, held_time, aux_data);
                break;
        }
    }
    free(event);
    return false;
}

void sdl_clear(void) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
}

void sdl_draw_polygon(List *points, RGBColor color, bool draw_camera) {
    // Check parameters
    size_t n = list_size(points);
    assert(n >= 3);
    assert(0 <= color.r && color.r <= 1);
    assert(0 <= color.g && color.g <= 1);
    assert(0 <= color.b && color.b <= 1);

    // Scale scene so it fits entirely in the window,
    // with the center of the scene at the center of the window
    int *width = malloc(sizeof(*width)),
        *height = malloc(sizeof(*height));
    assert(width);
    assert(height);
    SDL_GetWindowSize(window, width, height);
    double center_x = *width / 2.0,
           center_y = *height / 2.0;
    free(width);
    free(height);
    double x_scale = center_x / max_diff.x,
           y_scale = center_y / max_diff.y;
    double scale = x_scale < y_scale ? x_scale : y_scale;

    // Convert each vertex to a point on screen
    short *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
    assert(x_points);
    assert(y_points);
    for (size_t i = 0; i < n; i++) {
        Vector *vertex = list_get(points, i);
        Vector displacement = draw_camera ? vec_add(center, camera) : center;
        Vector pos_from_center =
            vec_multiply(scale, vec_subtract(*vertex, displacement));
        // Flip y axis since positive y is down on the screen
        x_points[i] = round(center_x + pos_from_center.x);
        y_points[i] = round(center_y - pos_from_center.y);
    }

    // Draw polygon with the given color
    filledPolygonRGBA(
        renderer,
        x_points, y_points, n,
        color.r * 255, color.g * 255, color.b * 255, 255
    );
    free(x_points);
    free(y_points);
}

void sdl_show(void) {
    SDL_RenderPresent(renderer);
}

void sdl_render_scene(Scene *scene) {
    sdl_clear();
    sdl_set_camera(scene_get_camera(scene));
    size_t body_count = scene_bodies(scene);
    for (size_t i = 0; i < body_count; i++) {
        Body *body = scene_get_body(scene, i);
        List *shape = body_get_shape(body);
        sdl_draw_polygon(shape, body_get_color(body), body_get_camera_attachment(body));
        list_free(shape);
    }
}

void sdl_on_key(KeyHandler handler, void *data) {
    key_handler = handler;
    aux_data = data;
}

void sdl_set_camera(Vector camera_pos){
    camera = camera_pos;
}

double time_since_last_tick(void) {
    clock_t now = clock();
    double difference = last_clock
        ? (double) (now - last_clock) / CLOCKS_PER_SEC
        : 0.0; // return 0 the first time this is called
    last_clock = now;
    return difference;
}
