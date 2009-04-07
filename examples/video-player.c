#include <stdlib.h>

#include <clutter/clutter.h>
#include <clutter-vlc/clutter-vlc.h>

#define SEEK_H 20
#define SEEK_W 690

typedef struct VideoApp
{
  ClutterActor *vtexture;

  ClutterActor    *control, *control_bg, *control_play, *control_pause,
    *control_seek1, *control_seek2, *control_seekbar, *control_label;
  gboolean         controls_showing, paused;
  guint            controls_timeout;
  ClutterTimeline *controls_tl, *effect1_tl;
}
VideoApp;

static void
show_controls (VideoApp *app, gboolean vis);

void
control_tl_cb (ClutterTimeline *timeline,
	       gint             frame_num,
	       VideoApp        *app)
{
  guint8 opacity;

  clutter_actor_show_all (app->control);
  clutter_actor_hide (app->paused ? app->control_pause : app->control_play);
  clutter_actor_show (app->paused ? app->control_play : app->control_pause);

  opacity = ( frame_num * 0xde ) / clutter_timeline_get_n_frames (timeline);

  if (!app->controls_showing)
    opacity = 0xde - opacity;

  clutter_actor_set_opacity (app->control, opacity);
}

void
control_tl_complete_cb (ClutterTimeline *timeline,
			VideoApp        *app)
{
  if (!app->controls_showing)
      clutter_actor_hide_all (app->control);

  app->controls_timeout = 0;
}

static gboolean
controls_timeout_cb (VideoApp *app)
{
  show_controls (app, FALSE);
  return FALSE;
}

static void
show_controls (VideoApp *app, gboolean vis)
{
  if (clutter_timeline_is_playing (app->controls_tl))
    return;

  if (vis == TRUE && app->controls_showing == FALSE)
    {
      app->controls_showing = TRUE;
      clutter_timeline_start (app->controls_tl);

      app->controls_timeout =
        g_timeout_add_seconds (5, (GSourceFunc) controls_timeout_cb, app);
      return;
    }

  if (vis == TRUE && app->controls_showing == TRUE)
    {
      if (app->controls_timeout)
	{
	  g_source_remove (app->controls_timeout);

	  app->controls_timeout =
	    g_timeout_add_seconds (5, (GSourceFunc) controls_timeout_cb, app);
	}
      return;
    }

  if (vis == FALSE && app->controls_showing == TRUE)
    {
      app->controls_showing = FALSE;
      clutter_timeline_start (app->controls_tl);
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

	  if (actor == app->control_seek1
	      || actor == app->control_seek2
	      || actor == app->control_seekbar)
	    {
	      gint x, y, dist, pos;

	      clutter_actor_get_transformed_position (app->control_seekbar,
                                                      &x, &y);

	      dist = bev->x - x;

	      CLAMP(dist, 0, SEEK_W);

	      pos = (dist * clutter_media_get_duration
                               (CLUTTER_MEDIA(app->vtexture))) / SEEK_W;

	      clutter_media_set_position (CLUTTER_MEDIA(app->vtexture),
					      pos);
	    }
	}
      break;
    case CLUTTER_KEY_RELEASE:
      {
	ClutterKeyEvent* kev = (ClutterKeyEvent *) event;

	switch (clutter_key_event_symbol (kev))
	  {
	  case CLUTTER_q:
	  case CLUTTER_Escape:
	    clutter_main_quit ();
	    break;
	  case CLUTTER_e:
	    if (!clutter_timeline_is_playing (app->effect1_tl))
	      clutter_timeline_start (app->effect1_tl);
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

void
size_change (ClutterTexture *texture,
	     gint            width,
	     gint            height,
	     gpointer        user_data)
{
  ClutterActor  *stage;
  gint           new_x, new_y, new_width, new_height;

  new_height = ( height * CLUTTER_STAGE_WIDTH() ) / width;
  if (new_height <= CLUTTER_STAGE_HEIGHT())
    {
      new_width = CLUTTER_STAGE_WIDTH();

      new_x = 0;
      new_y = (CLUTTER_STAGE_HEIGHT() - new_height) / 2;
    }
  else
    {
      new_width  = ( width * CLUTTER_STAGE_HEIGHT() ) / height;
      new_height = CLUTTER_STAGE_HEIGHT();

      new_x = (CLUTTER_STAGE_WIDTH() - new_width) / 2;
      new_y = 0;
    }

  clutter_actor_set_position (CLUTTER_ACTOR (texture), new_x, new_y);

  clutter_actor_set_size (CLUTTER_ACTOR (texture),
			  new_width,
			  new_height);
}

void
tick (GObject      *object,
      GParamSpec   *pspec,
      VideoApp     *app)
{
  ClutterMedia* media;
  gint                    w, h, position, duration, seek_w;
  gchar                   buf[256];

  media = CLUTTER_MEDIA(object);

  position = clutter_media_get_position (media);
  duration = clutter_media_get_duration (media);

  if (duration == 0 || position == 0)
    return;

  clutter_actor_set_size (app->control_seekbar,
			  (position * SEEK_W) / duration,
			  SEEK_H);
}

int
effect1_tl_cb (ClutterTimeline *timeline,
	       gint             frame_num,
	       VideoApp        *app)
{
  clutter_actor_set_rotation (app->vtexture, CLUTTER_Y_AXIS,
                              frame_num * 12,
                              CLUTTER_STAGE_WIDTH() / 2,
                              0,
                              0);
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

  app->vtexture = clutter_vlc_video_texture_new ();

  if (app->vtexture == NULL)
    {
      g_error("failed to create vtexture");
    }

  /* Dont let the underlying pixbuf dictate size */
  g_object_set (G_OBJECT(app->vtexture), "sync-size", FALSE, NULL);

  /* Handle it ourselves so can scale up for fullscreen better */
  g_signal_connect (CLUTTER_TEXTURE(app->vtexture),
		    "size-change",
		    G_CALLBACK (size_change), NULL);

  /* Load up out video texture */
  clutter_media_set_uri(CLUTTER_MEDIA(app->vtexture), argv[1]);

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

  app->control_label
    = clutter_label_new_with_text("Sans Bold 24",
				  g_path_get_basename(argv[1]));
  clutter_label_set_color (CLUTTER_LABEL(app->control_label), &control_color1);

  clutter_group_add_many (CLUTTER_GROUP (app->control),
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

  clutter_actor_hide (app->control);

  /* Add control UI to stage */
  clutter_group_add_many (CLUTTER_GROUP (stage),
			  app->vtexture, app->control, NULL);

  printf("start\n");

  x = (CLUTTER_STAGE_WIDTH() - clutter_actor_get_width(app->control))/2;
  y = CLUTTER_STAGE_HEIGHT() - (CLUTTER_STAGE_HEIGHT()/3);

  printf("setting x = %i, y = %i, width = %i\n", x, y, clutter_actor_get_width(app->control));

  clutter_actor_set_position (app->control, x, y);

  printf("stop\n");

  /* hook up a time line for fading controls */
  app->controls_tl = clutter_timeline_new (10, 30);
  g_signal_connect (app->controls_tl, "new-frame",
		    G_CALLBACK (control_tl_cb), app);
  g_signal_connect (app->controls_tl, "completed",
		    G_CALLBACK (control_tl_complete_cb), app);


  app->effect1_tl = clutter_timeline_new (30, 90);
  g_signal_connect (app->effect1_tl, "new-frame",
		    G_CALLBACK (effect1_tl_cb), app);

  /* Hook up other events */
  g_signal_connect (stage, "event",
		    G_CALLBACK (input_cb),
		    app);

  g_signal_connect (app->vtexture,
		    "notify::position",
		    G_CALLBACK (tick),
		    app);

  clutter_media_set_playing (CLUTTER_MEDIA (app->vtexture), TRUE);

  clutter_actor_show (stage);

  clutter_main();

  return 0;
}
