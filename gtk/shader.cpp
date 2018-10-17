#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <GL/glew.h>



#ifdef GDK_WINDOWING_QUARTZ
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#define TIMEOUT_INTERVAL 10

static guint idle_id = 0;
static gboolean is_sync = true;
static gboolean is_inited = false;


GLuint quad_vertexbuffer;

static void
print_hello (GtkWidget *widget,
             gpointer   data)
{
	g_print ("Hello World\n");
}

/**
   Creates the plane that will be used to render everything on
*/
void create_render_quad() {
	GLuint quad_vertex_array_id;
	// Create a quad
	glGenVertexArrays(1, &quad_vertex_array_id);
	/*glBindVertexArray(quad_vertex_array_id);
	
	static const GLfloat quad_vertex_buffer_data[] = {
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f,  -1.0f, 0.0f,
		1.0f,  -1.0f, 0.0f
	};
	
	// Put the data in buffers
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER,
				 sizeof(quad_vertex_buffer_data),
				 quad_vertex_buffer_data,
				 GL_STATIC_DRAW);*/
}

void init ()
{
	// Only in sourceview v4
	// gtk_source_init ();
	GLenum err;
	
	err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "GLEW initialization failed: %s\n",
				glewGetErrorString(err));
	}
	
	create_render_quad();
	is_inited = true;
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
visible (GtkWidget          *widget,
	 GdkEventVisibility *event,
	 gpointer            data)
{
	if (event->state == GDK_VISIBILITY_FULLY_OBSCURED){
		idle_remove (widget);
	}
	else
	{
		if (!is_inited){
			init();
		}
		idle_add (widget);
		
	}

  return TRUE;
}

/* new window size or exposure */
static gboolean
reshape (GtkWidget         *widget,
	 GdkEventConfigure *event,
	 gpointer           data)
{
  GtkAllocation allocation;

  GLfloat h;

  gtk_widget_get_allocation (widget, &allocation);
  h = (GLfloat) (allocation.height) / (GLfloat) (allocation.width);

  /*** OpenGL BEGIN ***/
  if (!gtk_widget_begin_gl (widget))
    return FALSE;

  glViewport (0, 0, allocation.width, allocation.height);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glFrustum (-1.0, 1.0, -h, h, 5.0, 60.0);
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glTranslatef (0.0, 0.0, -40.0);

  gtk_widget_end_gl (widget, FALSE);
  /*** OpenGL END ***/

  return TRUE;
}

/* change view angle, exit upon ESC */
static gboolean
key (GtkWidget   *widget,
     GdkEventKey *event,
     gpointer     data)
{
  GtkAllocation allocation;

  gtk_widget_get_allocation (widget, &allocation);
  gdk_window_invalidate_rect (gtk_widget_get_window (widget), &allocation, FALSE);

  return TRUE;
}

static gboolean
render (GtkWidget *widget,
	    cairo_t   *cr,
	    gpointer   data)
{
	if (!is_inited){
		return TRUE;
	}

	glClearColor (0.5, 0.5, 0.8, 1.0);
	glClearDepth (1.0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	g_print ("DRAW\n");

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(0);
	
	glFlush();
	
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
		
	gtk_widget_add_events (drawing_area,
						   GDK_VISIBILITY_NOTIFY_MASK);
	
	g_signal_connect (G_OBJECT (drawing_area), "configure_event",
					  G_CALLBACK (reshape), NULL);
	g_signal_connect (G_OBJECT (drawing_area), "draw",
					  G_CALLBACK (render), NULL);
	g_signal_connect (G_OBJECT (drawing_area), "map_event",
					  G_CALLBACK (map), NULL);
	g_signal_connect (G_OBJECT (drawing_area), "unmap_event",
					  G_CALLBACK (unmap), NULL);
	g_signal_connect (G_OBJECT (drawing_area), "visibility_notify_event",
					  G_CALLBACK (visible), NULL);
	
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
