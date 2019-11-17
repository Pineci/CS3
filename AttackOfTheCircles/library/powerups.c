#include "powerups.h"

void add_random_velocity(Body *player){
  double factor = (double)(rand() % 4) + 2;
  Vector new_velocity = vec_multiply(factor, body_get_velocity(player));
  body_add_impulse(player, new_velocity);
}
