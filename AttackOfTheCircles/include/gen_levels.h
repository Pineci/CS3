#ifndef __GEN_LEVELS_H__
#define __GEN_LEVELS_H__

#include <stdbool.h>
#include "scene.h"
//#include "gen_forces.h"

typedef enum{
  PLAYER,
  FLOOR,
  MOVING_FLOOR,
  ENEMY,
  BULLET_POWERUP,
  FINISHED_LEVEL_POWERUP,
  GRAVITY_BODY,
  SPIKE,
  BULLET,
  BOSS,
  ENEMY_BULLET,
  GUI_BULLET
} BODY_TYPE;

typedef enum{
    FALLING,
    TOUCHING_FLOOR,
    NONE
} BODY_MOVEMENT;

typedef enum{
    TOUCHING_LEFT,
    TOUCHING_RIGHT,
    TOUCHING_NONE
} BODY_WALL_TOUCH;

typedef struct body_info{
    BODY_TYPE type;
    BODY_MOVEMENT movement;
    BODY_WALL_TOUCH touch;
    double num;
    int bullet_count;
    double bullet_time_shot;
    int MAX_BULLETS;
} BodyInfo;

void body_info_free(BodyInfo *body_info);
BODY_TYPE get_body_type(Body *powerup);
BodyInfo *create_body_info(BODY_TYPE body_type, BODY_MOVEMENT body_movement);

// Generates all the floor bodies in the level. Also creates the scene.
void gen_tutorial_level(double player_size, Scene *scene, Body *player);
void gen_first_level(double player_size, Scene *scene, Body *player);
void gen_second_level(double player_size, Scene *scene, Body *player);
void gen_third_level(double player_size, Scene *scene, Body *player);
void gen_boss_level(double player_size, Scene *scene, Body *player);
Body *gen_player_sq(double player_size, Scene *scene);
Body *gen_bullet(double player_size, Scene *scene, BODY_TYPE btype);



#endif // #ifndef __SCENE_H__
