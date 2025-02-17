#include <cstdio>

#include "aos/events/shm_event_loop.h"
#include "aos/init.h"
#include "y2020/actors/autonomous_actor.h"
#include "y2020/constants.h"

int main(int argc, char *argv[]) {
  ::aos::InitGoogle(&argc, &argv);

  aos::FlatbufferDetachedBuffer<aos::Configuration> config =
      aos::configuration::ReadConfig("config.json");

  ::aos::ShmEventLoop event_loop(&config.message());
  ::y2020::constants::InitValues();
  ::y2020::actors::AutonomousActor autonomous(&event_loop);

  event_loop.Run();

  return 0;
}
