#ifndef __GEN_FORCES_H__
#define __GEN_FORCES_H__

#include "helpers.h"
#include "gen_levels.h"
#include "forces.h"

void gen_forces(Scene *scene);
void add_enemy_forces(Scene *scene, Body *enemy, Body *player);
void add_bullet_forces(Scene *scene, Body *bullet);
void add_boss_forces(Scene *scene, Body *boss);

#endif
