#ifndef __ENEMIES_H__
#define __ENEMIES_H__

#include "scene.h"
#include "gen_levels.h"
#include "helpers.h"
#include "body.h"

Body *gen_enemy(double enemy_size, double enemy_mass, Scene *scene, Vector spawn_point);
Body *gen_moving_enemy(double enemy_size, double enemy_mass, Scene *scene, Vector spawn_point);
Body *gen_boss(double boss_size, Scene *scene, Vector spawn_point);

#endif
