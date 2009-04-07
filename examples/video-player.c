#include <stdlib.h>

#include <clutter/clutter.h>
#include <clutter-vlc/clutter-vlc.h>

#define SEEK_H 20
#define SEEK_W 690

typedef struct _VideoApp
{
  ClutterActor *stage;

  ClutterActor *vtexture;

  ClutterActor *control;
  ClutterActor *control_bg;
  ClutterActor *control_label;

  ClutterActor *control_play, *control_pause;

  ClutterActor *control_seek1, *control_seek2, *control_seekbar;

  gboolean      controls_showing, paused;

  guint         controls_timeout;
} VideoApp;

static void show_controls (VideoApp *app, gboolean vis);

static gboolean
controls_timeout_cb (gpointer data)
{
  VideoApp *app = data;

  app->controls_timeout = 0;
  show_controls (app, FALSE);

  return FALSE;
}

static void
show_controls (VideoApp *app, gboolean vis)
{
  if (vis == TRUE && app->controls_showing == TRUE)
    {
      if (app->controls_timeout == 0)
        {
          app->controls_timeout =
            g_timeout_add_seconds (5, controls_timeout_cb, app);
        }

      return;
    }

  if (vis == TRUE && app->controls_showing == FALSE)
    {
      app->controls_showing = TRUE;

      clutter_stage_show_cursor (CLUTTER_STAGE (app->stage));
      clutter_actor_animate (app->control, CLUTTER_EASE_OUT_QUINT, 250,
                             "opacity", 224,
                             NULL);

      return;
    }

  if (vis == FALSE && app->controls_showing == TRUE)
    {
      app->controls_showing = FALSE;

      clutter_stage_hide_cursor (CLUTTER_STAGE (app->stage));
      clutter_actor_animate (app->control, CLUTTER_EASE_OUT_QUINT, 250,
                             "opacity", 0,
                             NULL);
      return;
    }
}

void
toggle_pause_state (VideoApp *app)
{
  if (app->paused)
    {
      clutter_media_set_playing (CLUTTER_MEDIA(app->vtexture), 
				     TRUE);
      app->paused = FALSE;
      clutter_actor_hide (app->control_play);
      clutter_actor_show (app->control_pause);
    }
  else
    {
      clutter_media_set_playing (CLUTTER_MEDIA(app->vtexture), 
				     FALSE);
      app->paused = TRUE;
      clutter_actor_hide (app->control_pause);
      clutter_actor_show (app->control_play);
    }
}

static void
reset_animation (ClutterAnimation *animation,
                 VideoApp         *app)
{
  clutter_actor_set_rotation (app->vtexture, CLUTTER_Y_AXIS, 0.0, 0, 0, 0);
}

static gboolean
input_cb (ClutterStage *stage, 
	  ClutterEvent *event,
	  gpointer      user_data)
{
  VideoApp *app = (VideoApp*)user_data;

  switch (event->type)
    {
    case CLUTTER_MOTION:
      show_controls (app, TRUE);
      break;

    case CLUTTER_BUTTON_PRESS:
      if (app->controls_showing)
	{
	  ClutterActor       *actor;
	  ClutterButtonEvent *bev = (ClutterButtonEvent *) event;

	  actor = clutter_stage_get_actor_at_pos (stage, bev->x, bev->y);

	  printf("got actor %p at pos %ix%i\n", actor, bev->x, bev->y);

	  if (actor == app->control_pause || actor == app->control_play)
	    {
	      toggle_pause_state (app);
	      return;
	    }

	  if (actor == app->control_seek1 ||
              actor == app->control_seek2 ||
              actor == app->control_seekbar)
	    {
	      gint x, y, dist;
              gdouble progress;

	      clutter_actor_get_transformed_position (app->control_seekbar,
                                                      &x, &y);

	      dist = bev->x - x;

	      CLAMP (dist, 0, SEEK_W);

	      progress = (gdouble) dist / SEEK_W;

	      clutter_media_set_progress (CLUTTER_MEDIA (app->vtexture),
                                          progress);
	    }
	}
      break;

    case CLUTTER_KEY_PRESS:
      {
	ClutterKeyEvent* kev = (ClutterKeyEvent *) event;
        ClutterVertex center = { 0, };
        ClutterAnimation *animation = NULL;

        center.x = clutter_actor_get_widthu (app->vtexture) / 2;
	
	switch (clutter_key_event_symbol (kev))
	  {
	  case CLUTTER_q:
	  case CLUTTER_Escape:
	    clutter_main_quit ();
	    break;

	  case CLUTTER_e:
            animation =
              clutter_actor_animate (app->vtexture, CLUTTER_LINEAR, 500,
                                     "rotation-angle-y", 360.0,
                                     "fixed::rotation-center-y", &center,
                                     NULL);
            g_signal_connect (animation, "completed",
                              G_CALLBACK (reset_animation),
                              app);
	    break;

	  default:
	    toggle_pause_state (app);
	    break;
	  }
      }
    default:
      break;
    }

  return FALSE;
}

static void
size_change (ClutterTexture *texture, 
	     gint            width,
	     gint            height,
	     VideoApp       *app)
{
  ClutterActor *stage = app->stage;
  gint new_x, new_y, new_width, new_height;
  guint stage_width, stage_height;

  clutter_actor_get_size (stage, &stage_width, &stage_height);

  new_height = (height * stage_width) / width;
  if (new_height <= stage_height)
    {
      new_width = stage_width;

      new_x = 0;
      new_y = (stage_height - new_height) / 2;
    }
  else
    {
      new_width  = (width * stage_height) / height;
      new_height = stage_height;

      new_x = (stage_width - new_width) / 2;
      new_y = 0;
    }

  clutter_actor_set_position (CLUTTER_ACTOR (texture), new_x, new_y);

  clutter_actor_set_size (CLUTTER_ACTOR (texture),
			  new_width,
			  new_height);
}

static void
tick (GObject      *object,
      GParamSpec   *pspec,
      VideoApp     *app)
{
  ClutterMedia *video_texture = CLUTTER_MEDIA (object);
  gdouble progress = clutter_media_get_progress (video_texture);

  clutter_actor_set_size (app->control_seekbar,
                          progress * SEEK_W,
			  SEEK_H);
}

int
main (int argc, char *argv[])
{
  VideoApp            *app = NULL;
  ClutterActor        *stage;
  ClutterColor         stage_color = { 0x00, 0x00, 0x00, 0x00 };
  ClutterColor         control_color1 = { 73, 74, 77, 0xee };
  ClutterColor         control_color2 = { 0xcc, 0xcc, 0xcc, 0xff };
  gint                 x,y;

  if (argc < 2)
    g_error("%s <video file>", argv[0]);

  clutter_init (&argc, &argv);
  clutter_vlc_init ();

  stage = clutter_stage_get_default ();
  g_object_set (stage, "fullscreen", TRUE, NULL);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color); 

  app = g_new0(VideoApp, 1);
  app->stage = stage;
  app->vtexture = clutter_vlc_video_texture_new();
  
  if (app->vtexture == NULL)
    g_error("failed to create vtexture");

  /* Dont let the underlying pixbuf dictate size */
  g_object_set (G_OBJECT(app->vtexture), "sync-size", FALSE, NULL);

  /* Handle it ourselves so can scale up for fullscreen better */
  g_signal_connect (CLUTTER_TEXTURE (app->vtexture),
		    "size-change",
		    G_CALLBACK (size_change), app);

  /* Load up out video texture */
  clutter_media_set_uri (CLUTTER_MEDIA (app->vtexture), argv[1]);
  clutter_media_set_audio_volume (CLUTTER_MEDIA (app->vtexture), 0.5);

  /* Create the control UI */
  app->control = clutter_group_new ();

  app->control_bg =
    clutter_texture_new_from_file ("vid-panel.png", NULL);
  app->control_play =
    clutter_texture_new_from_file ("media-actions-start.png", NULL);
  app->control_pause =
    clutter_texture_new_from_file ("media-actions-pause.png", NULL);

  g_assert (app->control_bg && app->control_play && app->control_pause);

  app->control_seek1   = clutter_rectangle_new_with_color (&control_color1);
  app->control_seek2   = clutter_rectangle_new_with_color (&control_color2);
  app->control_seekbar = clutter_rectangle_new_with_color (&control_color1);
  clutter_actor_set_opacity (app->control_seekbar, 0x99);

  app->control_label =
    clutter_text_new_full ("Sans Bold 24",
                           g_path_get_basename (argv[1]),
                           &control_color1);

  clutter_actor_hide (app->control_play);

  clutter_container_add (CLUTTER_CONTAINER (app->control),
			 app->control_bg,
			 app->control_play,
			 app->control_pause,
			 app->control_seek1,
			 app->control_seek2,
			 app->control_seekbar,
			 app->control_label,
			 NULL);

  clutter_actor_set_opacity (app->control, 0xee);

  clutter_actor_set_position (app->control_play, 30, 30);
  clutter_actor_set_position (app->control_pause, 30, 30);

  clutter_actor_set_size (app->control_seek1, SEEK_W+10, SEEK_H+10);
  clutter_actor_set_position (app->control_seek1, 200, 100);
  clutter_actor_set_size (app->control_seek2, SEEK_W, SEEK_H);
  clutter_actor_set_position (app->control_seek2, 205, 105);
  clutter_actor_set_size (app->control_seekbar, 0, SEEK_H);
  clutter_actor_set_position (app->control_seekbar, 205, 105);

  clutter_actor_set_position (app->control_label, 200, 40);

  /* Add control UI to stage */
  clutter_container_add (CLUTTER_CONTAINER (stage),
                         app->vtexture,
                         app->control,
                         NULL);

  g_print ("start\n");

  {
    guint stage_width, stage_height;

    clutter_actor_get_size (stage, &stage_width, &stage_height);

    x = (stage_width - clutter_actor_get_width (app->control)) / 2;
    y = stage_height - (stage_height / 3);
  }

  g_print ("setting x = %i, y = %i, width = %i\n",
           x, y, clutter_actor_get_width (app->control));

  clutter_actor_set_position (app->control, x, y);

  g_print ("stop\n");

  clutter_stage_hide_cursor (CLUTTER_STAGE (stage));
  clutter_actor_animate (app->control, CLUTTER_EASE_OUT_QUINT, 1000,
                         "opacity", 0,
                         NULL);

  /* Hook up other events */
  g_signal_connect (stage, "event", G_CALLBACK (input_cb), app);

  g_signal_connect (app->vtexture,
		    "notify::progress", G_CALLBACK (tick),
		    app);

  clutter_media_set_playing (CLUTTER_MEDIA (app->vtexture), TRUE);

  clutter_actor_show (stage);

  clutter_main ();

  clutter_vlc_cleanup();

  return 0;
}
