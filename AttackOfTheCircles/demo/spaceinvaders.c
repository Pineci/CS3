#include "polygon.h"
#include "body.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "shape.h"
#include "forces.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>


const int WINDOW_WIDTH = 1500;
const int WINDOW_HEIGHT = 500 ;
const int NUM_ROWS = 3;
const int INVADER_RADIUS = 100;
const int ROW_HEIGHT = 40;
const int CIRCLE_POINTS = 100;
const double UWU_BULGE = 2*M_PI/3;
const double MARGIN = 70;
const double SPACING = 30;
const double INVADER_MASS = 150;
const RGBColor GRAY = (RGBColor){.5, .5, .5};
const double INVADER_SPEED = 150;
const double PLAYER_INVADERS_SPEED = 400;
const double BULLET_SIZE = 15;
const double BULLET_SPEED = 2000;
const double BULLET_CHANCE = 10000;


typedef enum {
    I_BULLET,
    P_BULLET,
    INVADER,
    PLAYER_INVADERS
} BODY_TYPE_INVADERS;


void gen_player(Scene *scene, Vector min_corn){
    List *shape = shape_partial_circle(INVADER_RADIUS, CIRCLE_POINTS, UWU_BULGE/(2*M_PI));
    Vector *bulge = malloc(sizeof(Vector));
    *bulge = VEC_ZERO;
    list_add(shape, bulge);
    polygon_rotate(shape, UWU_BULGE, VEC_ZERO);
    polygon_translate(shape, (Vector){0, min_corn.y + ROW_HEIGHT});
    Body *player = body_init_with_info(shape, INVADER_MASS, gen_color(), (BODY_TYPE_INVADERS*)PLAYER_INVADERS, NULL);
    scene_add_body(scene, player);
}

void add_invader_row(Scene *scene, double height, double min_x, double max_x){
    for(int i = min_x + MARGIN; i < max_x; i += INVADER_RADIUS*2+SPACING){
        List *invader = shape_partial_circle(INVADER_RADIUS, CIRCLE_POINTS, UWU_BULGE/(2*M_PI));
        Vector *bulge = malloc(sizeof(Vector));
        *bulge = VEC_ZERO;
        list_add(invader, bulge);
        polygon_rotate(invader, -UWU_BULGE/2, VEC_ZERO);
        polygon_translate(invader, (Vector){i, height});
        Body *invader_body = body_init_with_info(invader, INVADER_MASS, GRAY, (BODY_TYPE_INVADERS*)INVADER, NULL);
        body_set_velocity(invader_body, (Vector){INVADER_SPEED, 0});
        scene_add_body(scene, invader_body);
        create_destructive_collision(scene, invader_body, scene_get_body(scene, 0));
    }
}

void gen_invaders(Scene *scene, Vector min_corn, Vector max_corn){
    for(int i = 0; i < NUM_ROWS; i++){
        add_invader_row(scene, max_corn.y - i*(INVADER_RADIUS + ROW_HEIGHT), min_corn.x, max_corn.x);
    }
}

void next_row(Body* invader){
    Vector center = body_get_centroid(invader);
    body_set_centroid(invader, (Vector){center.x, center.y - (INVADER_RADIUS + ROW_HEIGHT)*NUM_ROWS});
    body_set_velocity(invader, (Vector){body_get_velocity(invader).x * -1, 0});

}

void check_edge(Body *invader, Vector max_corn, Vector min_corn){
    Vector center = body_get_centroid(invader);
    if(body_get_velocity(invader).x > 0){
        if(center.x + INVADER_RADIUS >= max_corn.x){
            next_row(invader);
        }
    }
    else{
        if(center.x - INVADER_RADIUS <= min_corn.x){
            next_row(invader);
        }
    }
}

void spawn_bullet( Scene *scene, BODY_TYPE_INVADERS player, Vector spawn){
    List *bull = shape_regular_star(4, BULLET_SIZE);
    polygon_translate(bull, spawn);
    Body *bullet = body_init_with_info(bull, 10, GRAY, (BODY_TYPE_INVADERS *)player, NULL);
    if(player){
        body_set_velocity(bullet, (Vector){0, BULLET_SPEED});
        for(int i = 0; i < scene_bodies(scene); i++){
            Body *curr_body = scene_get_body(scene, i);
            if((BODY_TYPE_INVADERS)body_get_info(curr_body) == INVADER){
                create_destructive_collision(scene, bullet, curr_body);
            }
        }
    }
    else{
        body_set_velocity(bullet, (Vector){0, BULLET_SPEED * -1});
        create_destructive_collision(scene, bullet, scene_get_body(scene, 0));
    }
    scene_add_body(scene, bullet);

}

void on_key(char key, KeyEventType type, double held_time, void* data){
    Scene *scene = (Scene*)data;
    Body *player = scene_get_body(scene, 0);
    double speed_to_set = 0;
    bool shoot = false;
    switch(key){
        case LEFT_ARROW:
        speed_to_set = -1 * PLAYER_INVADERS_SPEED;
        break;

        case RIGHT_ARROW:
        speed_to_set = PLAYER_INVADERS_SPEED;
        break;

        case ' ':
        shoot = true;
        break;
    }
    if(type == KEY_PRESSED){
        body_set_velocity(player, (Vector){speed_to_set, 0});
        if(shoot){
            spawn_bullet(scene, P_BULLET, body_get_centroid(player));
            shoot = false;
        }
    }
    else{
        body_set_velocity(player, VEC_ZERO);
    }
}


int main(int argc, const char* argv[]){
    Vector min_corn = {.x = -1 * WINDOW_WIDTH, .y = -1 * WINDOW_HEIGHT};
    Vector max_corn = {.x = WINDOW_WIDTH, .y = WINDOW_HEIGHT};
    sdl_init(min_corn, max_corn);

    Scene *my_scene = scene_init();

    gen_player(my_scene, min_corn);
    gen_invaders(my_scene, min_corn, max_corn);
    sdl_on_key(on_key, my_scene);
    srand(time(NULL));
    while(!sdl_is_done()){
        double dt = time_since_last_tick();

        size_t num_bodies = scene_bodies(my_scene);
        for(int i = 0; i < num_bodies; i++){
            Body *curr_body = scene_get_body(my_scene, i);
            if((int)body_get_info(curr_body) == INVADER){
                check_edge(curr_body, max_corn, min_corn);
                if(fmod(rand(), BULLET_CHANCE) == 1){
                    spawn_bullet(my_scene, I_BULLET, body_get_centroid(curr_body));
                }
            }
        }

        Body *player = scene_get_body(my_scene, 0);
        if((BODY_TYPE_INVADERS)body_get_info(player) != PLAYER_INVADERS){
            exit(0);
        }

        scene_tick(my_scene, dt);
        sdl_render_scene(my_scene);
    }
    scene_free(my_scene);
}
