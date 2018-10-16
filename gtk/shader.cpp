#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#ifdef GDK_WINDOWING_QUARTZ
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#define TIMEOUT_INTERVAL 10

static guint idle_id = 0;
static gboolean is_sync = true;

static void
print_hello (GtkWidget *widget,
             gpointer   data)
{
	g_print ("Hello World\n");
}

void init ()
{
	// Only in sourceview v4
	// gtk_source_init ();
}

void close ()
{
	// Only in sourceview v4
	// gtk_source_finalize ();
}

static gboolean
idle (GtkWidget *widget)
{
  GtkAllocation allocation;
  GdkWindow *window;

  window = gtk_widget_get_window (widget);
  gtk_widget_get_allocation (widget, &allocation);

  /* Invalidate the whole window. */
  gdk_window_invalidate_rect (window, &allocation, FALSE);

  /* Update synchronously (fast). */
  if (is_sync)
	  gdk_window_process_updates (window, FALSE);

  return TRUE;
}

static void
idle_add (GtkWidget *widget)
{
  if (idle_id == 0)
    {
      idle_id = g_idle_add_full (GDK_PRIORITY_REDRAW,
                                 (GSourceFunc) idle,
                                 widget,
                                 NULL);
    }
}

static void
idle_remove (GtkWidget *widget)
{
  if (idle_id != 0)
    {
      g_source_remove (idle_id);
      idle_id = 0;
    }
}


static gboolean
visible (GtkWidget          *widget,
	 GdkEventVisibility *event,
	 gpointer            data)
{
  if (event->state == GDK_VISIBILITY_FULLY_OBSCURED)
    idle_remove (widget);
  else
    idle_add (widget);

  return TRUE;
}

static gboolean
map (GtkWidget   *widget,
     GdkEventAny *event,
     gpointer     data)
{
  idle_add (widget);

  return TRUE;
}

static gboolean
unmap (GtkWidget   *widget,
       GdkEventAny *event,
       gpointer     data)
{
  idle_remove (widget);

  return TRUE;
}

static gboolean
render (GtkWidget *widget,
	    cairo_t   *cr,
	    gpointer   data)
{
	glClearColor (0.5, 0.5, 0.8, 1.0);
	glClearDepth (1.0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	g_print ("DRAW\n");
	/*
	// inside this function it's safe to use GL; the given
	// #GdkGLContext has been made current to the drawable
	// surface used by the #GtkGLArea and the viewport has
	// already been set to be the size of the allocation
	
	// we can start by clearing the buffer
	glClearColor (0, 0, 0, 0);
	glClear (GL_COLOR_BUFFER_BIT);
	
	
	// we completed our drawing; the draw commands will be
	// flushed at the end of the signal emission chain, and
	// the buffers will be drawn on the window
	*/
	return TRUE;
}

int
main (int   argc,
      char *argv[])
{
	GdkGLConfig *glconfig;
	GtkBuilder *builder;
	GObject *window;
	GtkGrid *grid;
	GError *error = NULL;
	GtkWidget *view;
	GtkTextBuffer *buffer;
	GtkWidget *drawing_area;
	
	gtk_init (&argc, &argv);
	
	gtk_gl_init (&argc, &argv);
	
	/* Construct a GtkBuilder instance and load our UI description */
	builder = gtk_builder_new ();
	if (gtk_builder_add_from_file (builder, "builder.ui", &error) == 0)
    {
		g_printerr ("Error loading file: %s\n", error->message);
		g_clear_error (&error);
		return 1;
    }
	
	init();
	
	
	view = gtk_text_view_new ();
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_set_text (buffer, "Hello, this is some text\n", -1);
	
	/*
	 * Configure OpenGL-capable visual.
	 */
	
	/* Try double-buffered visual */
	glconfig = gdk_gl_config_new_by_mode (
		static_cast<GdkGLConfigMode>(
			static_cast<int>(GDK_GL_MODE_RGB)    |
			static_cast<int>(GDK_GL_MODE_DEPTH)  |
			static_cast<int>(GDK_GL_MODE_DOUBLE))
		);
	
	if (glconfig == NULL)
    {
		g_print ("*** Cannot find the double-buffered visual.\n");
		g_print ("*** Trying single-buffered visual.\n");
		
		/* Try single-buffered visual */
		glconfig = gdk_gl_config_new_by_mode (
			static_cast<GdkGLConfigMode>(
				static_cast<int>(GDK_GL_MODE_RGB)   |
				static_cast<int>(GDK_GL_MODE_DEPTH))
			);
		
		if (glconfig == NULL)
		{
			g_print ("*** No appropriate OpenGL-capable visual found.\n");
		}
    }
	
	drawing_area = gtk_drawing_area_new ();
	
	gtk_widget_set_gl_capability (drawing_area,
								  glconfig,
								  NULL,
								  TRUE,
								  GDK_GL_RGBA_TYPE);
	
	g_signal_connect (G_OBJECT (drawing_area), "draw",
					  G_CALLBACK (render), NULL);
	g_signal_connect (G_OBJECT (drawing_area), "visibility_notify_event",
					  G_CALLBACK (visible), NULL);
	g_signal_connect (G_OBJECT (drawing_area), "map_event",
					  G_CALLBACK (map), NULL);
	g_signal_connect (G_OBJECT (drawing_area), "unmap_event",
					  G_CALLBACK (unmap), NULL);
	
	grid = GTK_GRID (gtk_builder_get_object (builder, "grid"));
	
	gtk_grid_attach (
		grid,
		drawing_area,
		1,
		0,
		1,
		1);
	
	gtk_grid_attach (
		grid,
		view,
		0,
		0,
		1,
		1);
	
	window = gtk_builder_get_object (builder, "window");
	gtk_widget_show_all (GTK_WIDGET(window));
	
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);


	
	gtk_main ();
	
	return 0;
}
