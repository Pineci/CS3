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
#include <string.h>
#include <stdio.h>


const int WINDOW_WIDTH = 1500;
const int WINDOW_HEIGHT = 500;
const int NUM_ROWS = 3;
const int BRICK_WIDTH = 100;
const int BRICK_HEIGHT = 30;
const int ROW_HEIGHT = 40;
const int CIRCLE_POINTS = 100;
const double MARGIN = 70;
const double SPACING = 30;
const double BRICK_MASS = 150;
const RGBColor GRAY = (RGBColor){.5, .5, .5};
const RGBColor WHITE = (RGBColor){1, 1, 1};
const double PLAYER_BLOCK_SPEED = 500;
const int PLAYER_BLOCK_WIDTH = 300;
const int PLAYER_BLOCK_HEIGHT = 50;
const double BALL_SIZE = 15;
const double BALL_MASS = 10;
const double BALL_SPEED = 600;
const double ELASTICITY = 1.0;
const double POWERUP_SPAWN_CHANCE = 0.75;
const int  POWERUP_SIZE = 15;

typedef enum {
    POWERUP,
    BOTTOM_WALL,
    WALL,
    BALL,
    BRICK,
    PLAYER_BLOCK
} BODY_TYPE_BLOCKS;

typedef struct body_info_blocks{
    BODY_TYPE_BLOCKS type;
    int num;
    bool boolean;
} BodyInfoBlocks;

BodyInfoBlocks *body_info_init(BODY_TYPE_BLOCKS type, int num, bool boolean){
    BodyInfoBlocks *body_info = malloc(sizeof(BodyInfoBlocks));
    body_info->type = type;
    body_info->num = num;
    body_info->boolean = boolean;
    return body_info;
}

void body_info_free_blocks(BodyInfoBlocks *body_info){
    free(body_info);
}

void gen_player(Scene *scene, Vector min_corn){
    List *shape = shape_rectangle(PLAYER_BLOCK_WIDTH, PLAYER_BLOCK_HEIGHT);
    BodyInfoBlocks *body_info = body_info_init(PLAYER_BLOCK, 0, false);
    Body *player = body_init_with_info(shape, INFINITY, gen_color(), body_info, (FreeFunc)body_info_free_blocks);
    body_set_centroid(player, (Vector){0, min_corn.y});
    scene_add_body(scene, player);
}

double *rainbow_maker(int i, double *prev_color, double num_stuff){
  double delta_color = 6 / (double) num_stuff;
  double flip = prev_color[3];
  int color_index = prev_color[4];
  if ((int)fmod(i, num_stuff / 6) == 0 && i != 0){
    if (flip == -1){
      prev_color[color_index] = 0;
    }
    else{
      prev_color[color_index] = 1;
    }
    prev_color[3] *= -1;
    prev_color[4] = fmod(color_index + 2, 3);
    flip = prev_color[3];
    color_index = prev_color[4];
  }
  prev_color[color_index] += (delta_color * flip);
  return prev_color;
}

void gen_powerup(Scene *scene, RGBColor color, Vector center){
    List *shape = shape_estrella(POWERUP_SIZE);
    BodyInfoBlocks *body_info = body_info_init(POWERUP, 0, false);
    Body *powerup = body_init_with_info(shape, INFINITY, color, body_info, (FreeFunc)body_info_free_blocks);
    body_set_centroid(powerup, center);
    scene_add_body(scene, powerup);
}

void add_brick_row(Scene *scene, double height, double min_x, double max_x){
    double prev_color[5] = {1, 0, 1, -1, 2};
    for(int i = min_x + MARGIN; i < max_x; i += BRICK_WIDTH+SPACING){
        List *brick = shape_rectangle(BRICK_WIDTH, BRICK_HEIGHT);
        BodyInfoBlocks *body_info = body_info_init(BRICK, 0, false);
        RGBColor rainbow = (RGBColor){prev_color[0], prev_color[1], prev_color[2]};
        Body *brick_body = body_init_with_info(brick, BRICK_MASS, rainbow, body_info, (FreeFunc)body_info_free_blocks);
        memmove(prev_color, rainbow_maker(i, prev_color, BRICK_WIDTH+SPACING - (min_x+MARGIN)), sizeof(int *));
        Vector center = (Vector){i, height};
        body_set_centroid(brick_body, center);
        if((double)rand()/RAND_MAX < POWERUP_SPAWN_CHANCE){
            gen_powerup(scene, GRAY, center);
        }
        scene_add_body(scene, brick_body);
    }
}

void gen_bricks(Scene *scene, Vector min_corn, Vector max_corn){
    for(int i = 0; i < NUM_ROWS; i++){
        add_brick_row(scene, max_corn.y - i*(BRICK_HEIGHT + ROW_HEIGHT), min_corn.x, max_corn.x);
    }
}

void gen_walls(Scene *scene, Vector min_corn, Vector max_corn){
    List *wall1 = shape_rectangle(100, max_corn.y - min_corn.y);
    List *wall2 = shape_rectangle(100, max_corn.y - min_corn.y);
    List *wall3 = shape_rectangle(max_corn.x - min_corn.x, 100);
    List *wall4 = shape_rectangle(max_corn.x - min_corn.x, 100);
    BodyInfoBlocks *body_info1 = body_info_init(WALL, 0, false);
    BodyInfoBlocks *body_info2 = body_info_init(WALL, 0, false);
    BodyInfoBlocks *body_info3 = body_info_init(BOTTOM_WALL, 1, false);
    BodyInfoBlocks *body_info4 = body_info_init(WALL, 0, false);
    Body *leftWall = body_init_with_info(wall1, INFINITY, GRAY, body_info1, (FreeFunc)body_info_free_blocks);
    Body *rightWall = body_init_with_info(wall2, INFINITY, GRAY, body_info2, (FreeFunc)body_info_free_blocks);
    Body *bottomWall = body_init_with_info(wall3, INFINITY, WHITE, body_info3, (FreeFunc)body_info_free_blocks);
    Body *topWall = body_init_with_info(wall4, INFINITY, GRAY, body_info4, (FreeFunc)body_info_free_blocks);
    body_set_centroid(leftWall, (Vector){min_corn.x-50, 0});
    body_set_centroid(rightWall, (Vector){max_corn.x+50, 0});
    body_set_centroid(bottomWall, (Vector){0, min_corn.y - 50});
    body_set_centroid(topWall, (Vector){0, max_corn.y + 50});
    scene_add_body(scene, leftWall);
    scene_add_body(scene, rightWall);
    scene_add_body(scene, bottomWall);
    //scene_add_body(scene, topWall);
}

void gen_ball(Scene *scene){
    List *shape = shape_circle(BALL_SIZE, CIRCLE_POINTS);
    BodyInfoBlocks *body_info = body_info_init(BALL, 1, false);
    Body *ball = body_init_with_info(shape, BALL_MASS, GRAY, body_info, (FreeFunc)body_info_free_blocks);
    body_set_velocity(ball, (Vector){((rand() % 2)*2 - 1) * BALL_SPEED/2, BALL_SPEED});
    scene_add_body(scene, ball);
}

List *get_bodies(Scene *scene, BODY_TYPE_BLOCKS type){
    List *list = list_init(1, NULL);
    for(int i = 0; i < scene_bodies(scene); i++){
        Body *curr_body = scene_get_body(scene, i);
        BodyInfoBlocks *body_info = (BodyInfoBlocks*)body_get_info(curr_body);
        if(body_info->type == type){
            list_add(list, curr_body);
        }
    }
    return list;
}

Body *get_first_body_type(Scene *scene, BODY_TYPE_BLOCKS type){
    List *list = get_bodies(scene, type);
    Body *body = list_get(list, 0);
    list_free(list);
    return body;
}

void on_key(char key, KeyEventType type, double held_time, void* data){
    Scene *scene = (Scene*)data;
    Body *player = get_first_body_type(scene, PLAYER_BLOCK);
    BodyInfoBlocks *body_info = (BodyInfoBlocks*)body_get_info(player);
    double speed_to_set = 0;
    switch(key){
        case LEFT_ARROW:
            //if(!body_info->boolean || body_info->num > 0){
                speed_to_set = -1 * PLAYER_BLOCK_SPEED;
            //}
            break;
        case RIGHT_ARROW:
            //if(!body_info->boolean || body_info->num < 0){
                speed_to_set = PLAYER_BLOCK_SPEED;

            //}
            break;
        default:
            break;
    }
    if(type == KEY_PRESSED){
        body_set_velocity(player, (Vector){speed_to_set, 0});
        body_info->boolean = false;
        body_info->num = 0;
    }
    else{
        body_set_velocity(player, VEC_ZERO);
    }
}

void add_brick_bounce(Body *ball, Body *brick, Vector axis, void *aux, CollisionEventType type){
    if(type == COLLISION_START){
        add_bounce(ball, brick, axis, aux, type);
        body_remove(brick);
    }
}

void add_bottom_wall_collision(Body *ball, Body *wall, Vector axis, void *aux, CollisionEventType type){
    if(type == COLLISION_START){
        Scene *scene = (Scene*)aux;
        BodyInfoBlocks *body_info = (BodyInfoBlocks*) body_get_info(wall);
        body_info->num--;
        body_remove(ball);
        if(body_info->num <= 0){
            scene_set_done(scene, true);
        }
    }
}

void add_player_wall_collision(Body *player, Body *wall, Vector axis, void *aux, CollisionEventType type){
    if(type == COLLISION_START){
        BodyInfoBlocks *body_info = (BodyInfoBlocks*)body_get_info(player);
        body_info->num = axis.x > 0 ? 1 : -1;
        body_info->boolean = true;
        printf("(%f, %f)\n", axis.x, axis.y);
        //body_set_velocity(player, VEC_ZERO);
    }
}

void gen_ball_collisions(Scene *scene, Body *ball);

void add_powerup_ball_collision(Body *ball, Body *powerup, Vector axis, void *aux){
    Scene *scene = (Scene*)aux;
    Body* bottom_wall = get_first_body_type(scene, BOTTOM_WALL);
    BodyInfoBlocks *body_info = (BodyInfoBlocks*)body_get_info(bottom_wall);
    body_info->num++;
    gen_ball(scene);
    List *balls = get_bodies(scene, BALL);
    Body *new_ball = list_get(balls, body_info->num-1);
    list_free(balls);
    gen_ball_collisions(scene, new_ball);
    body_remove(powerup);
}

void gen_ball_collisions(Scene *scene, Body *ball){
    for(int i = 0; i < scene_bodies(scene); i++){
        Body *curr_body = scene_get_body(scene, i);
        BodyInfoBlocks *body_info = (BodyInfoBlocks*)body_get_info(curr_body);
        switch(body_info->type){
            case WALL:
                create_physics_collision(scene, ELASTICITY, ball, curr_body);
                break;
            case BRICK:
                create_collision(scene, ball, curr_body, add_brick_bounce, (void *)&ELASTICITY, NULL);
                break;
            case PLAYER_BLOCK:
                create_physics_collision(scene, ELASTICITY, ball, curr_body);
                break;
            case BALL:
                break;
            case BOTTOM_WALL:
                create_collision(scene, ball, curr_body, add_bottom_wall_collision, (void*)scene, NULL);
                break;
            case POWERUP:
                create_collision(scene, ball, curr_body, add_powerup_ball_collision, (void*)scene, NULL);
                break;
        }
    }
}

void gen_player_collisions(Scene *scene){
    Body *player = get_first_body_type(scene, PLAYER_BLOCK);
    for(int i = 0; i < scene_bodies(scene); i++){
        Body *curr_body = scene_get_body(scene, i);
        BodyInfoBlocks *body_info = (BodyInfoBlocks*)body_get_info(curr_body);
        if(body_info->type == WALL){
            create_collision(scene, player, curr_body, add_player_wall_collision, NULL, NULL);
        }
    }
}

void gen_bodies(Scene *scene, Vector min_corn, Vector max_corn){
    gen_player(scene, min_corn);
    gen_bricks(scene, min_corn, max_corn);
    gen_walls(scene, min_corn, max_corn);
    gen_ball(scene);
}

void gen_collisions(Scene *scene){
    gen_player_collisions(scene);
    gen_ball_collisions(scene, get_first_body_type(scene, BALL));
}

Scene *gen_game(Vector min_corn, Vector max_corn, KeyHandler on_key){
    Scene *scene = scene_init();
    gen_bodies(scene, min_corn, max_corn);
    gen_collisions(scene);
    sdl_on_key(on_key, scene);
    return scene;
}


int main(int argc, const char* argv[]){
    srand(time(NULL));
    Vector min_corn = {.x = -1 * WINDOW_WIDTH, .y = -1 * WINDOW_HEIGHT};
    Vector max_corn = {.x = WINDOW_WIDTH, .y = WINDOW_HEIGHT};
    sdl_init(min_corn, max_corn);
    Scene *my_scene = gen_game(min_corn, max_corn, on_key);

    while(!sdl_is_done()){
        double dt = time_since_last_tick();
        scene_tick(my_scene, dt);
        sdl_render_scene(my_scene);

        if(scene_is_done(my_scene)){
            scene_free(my_scene);
            my_scene = gen_game(min_corn, max_corn, on_key);
        }
    }
    scene_free(my_scene);
}
