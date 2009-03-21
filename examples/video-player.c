#include <clutter/clutter.h>
#include <clutter-vlc/clutter-vlc.h>

static ClutterColor color_light_blue = { 0xaa, 0xaa, 0xff, 0xff };

int
main(int ac, char** av)
{
  ClutterActor* stage;
  ClutterActor* texture;

  if (ac < 2)
    return -1;

  clutter_init(&ac, &av);
  clutter_vlc_init();

  stage = clutter_stage_get_default();
  clutter_stage_set_color(CLUTTER_STAGE(stage), &color_light_blue);

  texture = clutter_vlc_video_texture_new();
  clutter_media_set_uri(CLUTTER_MEDIA(texture), av[1]);
  clutter_media_set_playing(CLUTTER_MEDIA(texture), TRUE);

  clutter_actor_set_size(texture, 320, 240);
  clutter_actor_set_position(texture,
			     clutter_actor_get_width(stage) / 2 - clutter_actor_get_width(texture) / 2,
			     clutter_actor_get_height(stage) / 2 - clutter_actor_get_height(texture) / 2);

  clutter_container_add_actor(CLUTTER_CONTAINER(stage), texture);

  clutter_actor_show(stage);

  clutter_main();

  clutter_vlc_cleanup();

  return 0;
}
