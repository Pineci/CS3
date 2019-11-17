#include "polygon.h"
#include "body.h"
#include "scene.h"
#include "gen_levels.h"
#include "gen_forces.h"
#include "sdl_wrapper.h"
#include "shape.h"
#include "forces.h"
#include "helpers.h"
#include "enemies.h"
#include "gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>

const int WINDOW_WIDTH = 3000;
const int WINDOW_HEIGHT = 1000;
const double PLAYER_SIZE = 100;

const double PLAYER_HORIZONTAL_ACCEL = 2E7;

const double JUMP_HEIGHT = 15;
const double ENEMY_SIZE = 100;
const double ENEMY_MASS = 50;

const double BULLET_REGEN_TIME = 3.0;

const char* FONT_REGULAR = "caveat-regular.ttf";
const SDL_Color SDL_BLUE = {0, 0, 255};
const int PTSIZE = 20;

Vector camera_position(void *aux){
    Body *player = (Body*)aux;
    Vector center = vec_subtract(body_get_centroid(player), (Vector){WINDOW_WIDTH/2, WINDOW_HEIGHT/2});
    return center;
}

void player_shoot(Scene *scene){
    Body *player = get_first_body(scene, PLAYER);
    BodyInfo *info = body_get_info(player);
    if(info->bullet_count < info->MAX_BULLETS){
        Body *bullet = gen_bullet(PLAYER_SIZE, scene, BULLET);
        add_bullet_forces(scene, bullet);
        info->bullet_count = info->bullet_count + 1;
    }
}

void regen_bullets(Scene *scene){
    Body *player = get_first_body(scene, PLAYER);
    BodyInfo *info = body_get_info(player);
    if(info->bullet_count > 0){
        double timeDiff = scene_get_time(scene) - info->bullet_time_shot;
        if(timeDiff > BULLET_REGEN_TIME){
            info->bullet_time_shot = scene_get_time(scene);
            info->bullet_count -= 1;
        }
    }
}

void on_key(char key, KeyEventType type, double held_time, void* data){
    Scene *scene = (Scene*) data;
    int *key_data = scene_key_data(scene);
    Body *player = get_first_body(scene, PLAYER);
    BodyInfo *info = body_get_info(player);
    Vector centroid = body_get_centroid(player);
    switch(key){
        case LEFT_ARROW:
            *(key_data + LEFT_ARROW) = type;
            break;
        case RIGHT_ARROW:
            *(key_data + RIGHT_ARROW) = type;
            break;
        case UP_ARROW:
            *(key_data + UP_ARROW) = type;
            break;
        case ' ':
            if(type == KEY_PRESSED){
                info->bullet_time_shot = scene_get_time(scene);
                player_shoot(scene);
            }
            break;
        case 'c':
            printf("(%f, %f)\n", centroid.x, centroid.y);
            break;
        default:
            break;
    }
}

void scene_init_camera(Scene *scene){
    Body *player = get_first_body(scene, PLAYER);
    scene_set_camera(scene, (Vector){body_get_centroid(player).x - WINDOW_WIDTH/2, 0});
}

Scene *gen_level(int level_num){
    Scene *scene = scene_init();

    Body *player = gen_player_sq(PLAYER_SIZE, scene);

    switch(level_num){
        case 0:
            gen_tutorial_level(PLAYER_SIZE, scene, player);
            break;
        case 1:
            gen_first_level(PLAYER_SIZE, scene, player);
            break;
        case 2:
            gen_second_level(PLAYER_SIZE, scene, player);
            break;
        case 3:
            gen_third_level(PLAYER_SIZE, scene, player);
            break;
        case 4:
            gen_boss_level(PLAYER_SIZE, scene, player);
            break;
    }
    gen_forces(scene);
    scene_set_camera_follower(scene, camera_position, get_first_body(scene, PLAYER), NULL);
    sdl_on_key(on_key, scene);
    //spawn_enemy(scene, (Vector){PLAYER_SIZE*5, PLAYER_SIZE*5});
    //scene_init_camera(toret);

    return scene;
}

void show_tutorial_text(Vector player_location, double player_size){
  write_font(FONT_REGULAR, PTSIZE, "Welcome to the tutorial level.", SDL_BLUE,
    (Vector){3 * player_size - player_location.x/3, 1 * player_size + player_location.y/3}, 200, 30);
  write_font(FONT_REGULAR, PTSIZE, "Use the left and right arrow keys to move.", SDL_BLUE,
    (Vector){3 * player_size - player_location.x/3, 1.25 * player_size + player_location.y/3}, 300, 30);
  write_font(FONT_REGULAR, PTSIZE, "Jump over blocks using the up arrow key.", SDL_BLUE,
    (Vector){6.9 * player_size - player_location.x/3, 1 * player_size + player_location.y/3}, 300, 30);
  write_font(FONT_REGULAR, PTSIZE, "Avoid red spikes and red enemies.", SDL_BLUE,
    (Vector){11 * player_size - player_location.x/3, 1 * player_size + player_location.y/3}, 250, 30);
  write_font(FONT_REGULAR, PTSIZE, "You can use space bar to shoot enemies.", SDL_BLUE,
    (Vector){11 * player_size - player_location.x/3, 1.25 * player_size + player_location.y/3}, 300, 30);
  write_font(FONT_REGULAR, PTSIZE, "Powerups are star shaped.", SDL_BLUE,
    (Vector){15 * player_size - player_location.x/3, 1 * player_size + player_location.y/3}, 200, 30);
  write_font(FONT_REGULAR, PTSIZE, "Blue ones give you a bullet upgrade!", SDL_BLUE,
    (Vector){19.5 * player_size - player_location.x/3, 1.125 * player_size + player_location.y/3}, 300, 30);
  write_font(FONT_REGULAR, PTSIZE, "Green ones take you to the next level.", SDL_BLUE,
    (Vector){26.75 * player_size - player_location.x/3, 1.125 * player_size + player_location.y/3}, 300, 30);
}

void show_boss_text(Vector player_location, double player_size){
    write_font(FONT_REGULAR, PTSIZE, "Haha! I am Big Bad Triangle man", SDL_BLUE,
      (Vector){3 * player_size - player_location.x/3, 1 * player_size + player_location.y/3}, 300, 30);
    write_font(FONT_REGULAR, PTSIZE, "I've been controlling the circles all along!", SDL_BLUE,
      (Vector){3 * player_size - player_location.x/3, 1.25 * player_size + player_location.y/3}, 300, 30);
}

void gen_last_text(Vector player_location){
    write_font(FONT_REGULAR, PTSIZE, "Your princess isn't in this castle", SDL_BLUE,
    (Vector){350, 230}, 300, 30);
    write_font(FONT_REGULAR, PTSIZE, "Neither is a credits scene", SDL_BLUE,
    (Vector){350, 250}, 300, 30);
    write_font(FONT_REGULAR, PTSIZE, "bye.", SDL_BLUE,
    (Vector){350, 280}, 300, 30);


}

int main(int argc, const char* argv[]){
    Vector min_corn = {.x = 0, .y = 0};
    Vector max_corn = {.x = WINDOW_WIDTH, .y = WINDOW_HEIGHT};
    sdl_init(min_corn, max_corn);
    srand(time(NULL));

    int curr_level = 1;
    Scene *scene = gen_level(curr_level);

    while(!sdl_is_done()){
        double dt = time_since_last_tick();
        Body *player = get_first_body(scene, PLAYER);
        Vector player_location = body_get_centroid(player);
        BodyInfo *info = body_get_info(player);

        scene_tick(scene, dt);
        sdl_render_scene(scene);
        if(curr_level != 5) add_gui(scene, min_corn, max_corn, info->MAX_BULLETS);
        regen_bullets(scene);
        switch(curr_level){
            case 0:
                show_tutorial_text(player_location, PLAYER_SIZE);
                break;
            case 4:
                show_boss_text(player_location, PLAYER_SIZE);
                break;
            case 5:
                gen_last_text(player_location);
        }
        sdl_show();

        if(scene_check_finished_level(scene)){
          curr_level++;
          scene_set_done(scene, true);
        }
        if(scene_is_done(scene) ||  body_get_centroid(player).y < -100){
            scene_free(scene);
            scene = gen_level(curr_level);
        }
    }
    scene_free(scene);
}
