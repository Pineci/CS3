#include "shape.h"
#include "enemies.h"

const double NUM_VERT = 30;
const double ENEMY_SPEED = 5000.0;

Body *gen_enemy(double enemy_size, double enemy_mass, Scene *scene, Vector spawn_point){
    List *shape = shape_circle(enemy_size, NUM_VERT);
    BodyInfo *info = create_body_info(ENEMY, FALLING);
    Body *enem = body_init_with_info(shape, enemy_mass, (RGBColor){1, 0, 0}, info, (FreeFunc)body_info_free);
    body_set_centroid(enem, spawn_point);
    scene_add_body(scene, enem);
    return enem;
}

Body *gen_boss(double boss_size, Scene *scene, Vector spawn_point){
    List *shape = shape_triangle(boss_size);
    BodyInfo *info = create_body_info(BOSS, FALLING);
    Body *boss = body_init_with_info(shape, 50, (RGBColor){1, .5, .5}, info, (FreeFunc)body_info_free);
    body_set_centroid(boss, spawn_point);
    scene_add_body(scene, boss);
    return boss;
}

Body *gen_moving_enemy(double enemy_size, double enemy_mass, Scene *scene, Vector spawn_point){
    Body *enem = gen_enemy(enemy_size, enemy_mass, scene, spawn_point);
    return enem;
}
