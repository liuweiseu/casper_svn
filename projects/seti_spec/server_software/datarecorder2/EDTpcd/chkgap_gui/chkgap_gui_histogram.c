#include <math.h>
#include <gtk/gtk.h>
#include <pango/pango.h>
#include "chkgap_gui_histogram.h"

/*
GapInfo hist_test_data[] = {
	{8,1}, {9,3319}, {11,6}, {13,2}, {14,162}, {15,294},
	{16,5}, {17,17}, {18,27}, {19,4}, {20,1}, {21,2}, {27,1},
	{30,144}, {44,1}, {45,63}, {46,12}, {47,9}, {48,1}, {49,2},
	{51,1}, {54,3}, {55,1}, {56,6}, {57,5}, {58,5}, {59,1},
	{0,0} // end marker
};
*/


//double freq_test[] = { 1.0, .8, .5, .2, .1, .4, .9, .7, .8, .4 };

int freq_table[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 2, 1, 
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 2, 1 };


//HistNode *histogram_array_to_sll ( );
//FreqTableData *convert_freq_table_to_bins (gint *freq_table, gint array_size, 
	//	gint n_bins, gint prev_n_bins);

void convert_freq_table_to_bins (FreqTableData *table_data, 
		gint *freq_table, gint array_size, 
		gint n_bins, gint prev_n_bins);

/* draw the histogram in the specified rectangle of the drawing area */
void draw_histogram(Histogram *hist, GdkRectangle *rect) 
{
	/* draw each column (bin) from the hist's LLL of bins. These 
	 * should be drawn inside the rectangle given as an argument.
	 * Everything will be drawn on hist->pixmap, which must not be NULL */
	GtkWidget *widget = GTK_WIDGET(hist->drawing_area);
	GdkGC *gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
	GdkPixmap *pixmap = hist->pixmap;
	GdkColor bar_color;
	GdkGC *bar_gc = gdk_gc_new(GDK_DRAWABLE(pixmap));
	GdkColor rect_color;
	GdkGC *rect_gc = gdk_gc_new(GDK_DRAWABLE(pixmap));
	GdkColormap *colormap;

	/* TODO: keep track of which frequency is smallest... 
	 * the idea is that if we know what the value of the smallest
	 * bar, we can make that bar very short (mess w/ the y-scale to
	 * give a greater depth of information). For example, if the
	 * data is from 990 to 1000 in frequency for all bars, it would 
	 * be nice if 990 was near the bottom of the histogram drawing area
	 * and 1000 was at the top, so you could make out the differences. 
	 * Hmm, that might not actually be what we want: with the histogram
	 * we might just want to see the general distribution of the data, 
	 * and not care about differences that small.  Hmm.
	 */

	int rect_width, rect_height; /* width and height of rect to paint */
	int xpainted; /* x-axis value of first unpainted pixel */
	int startx, starty; /* start co-ords for rectangles */

	int left_x = rect->x;
	int right_x = rect->x + rect->width;
	int bottom_y = rect->y + rect->height;

	int max_height = rect->height;
	int max_freq = hist->table_data.max_freq_absolute;
	////int max_freq = hist->table_data.max_freq;

	Node *cur_node = hist->table_data.head;

	colormap = gdk_drawable_get_colormap(GDK_DRAWABLE(pixmap));
	if (!gdk_color_parse("firebrick", &bar_color)) {
		g_print("firebrick apparently isn't a color!\n");
	} 
	if (!gdk_colormap_alloc_color(colormap, &bar_color, FALSE, TRUE)) {
		g_print("couln't allocate firebrick color\n");
	}
	if (!gdk_color_parse("sky blue", &rect_color)) {
		g_print("sky blue apparently isn't a color!\n");
	}
	if (!gdk_colormap_alloc_color(colormap, &bar_color, FALSE, TRUE)) {
		g_print("couln't allocate sky blue color\n");
	}
	gdk_gc_copy(bar_gc, gc);
	gdk_gc_set_foreground(bar_gc, &bar_color);

	/* DEBUG: draw blue rectangle around histogram area 
	gdk_gc_copy(rect_gc, gc);
	gdk_gc_set_foreground(rect_gc, &rect_color); 
	gdk_draw_rectangle(pixmap, rect_gc,
		TRUE, rect->x, rect->y, rect->width, rect->height); */

	
	/* get scale: (double) this_freq / max_freq */

	/* draw each column */
	xpainted = left_x;
	rect_width = hist->column_width;
	starty = bottom_y;
	for (; cur_node; cur_node = cur_node->next) {
		g_print("this column freq: %d\n", cur_node->bin.frequency);
		rect_height = (((double)cur_node->bin.frequency) / max_freq) * max_height;
		startx = xpainted;
		if (startx > right_x) {
			g_print("yikes... drawing point x=%d is out of x=%d bounds.\n",
					startx, right_x);
			break; /* cannot draw out of bounds */
		}
		if (startx + rect_width > right_x) {
			g_print("yikes... drawing rect ending at x=%d is out of x=%d bounds.\n",
					startx + rect_width, right_x);
			break; /* cannot draw out of bounds */
		}

		/* draw this column */
		//g_print("drawing rect (x,y,width,height)=(%d, %d, %d, %d)\n",
			//	startx, starty, rect_width, rect_height);
		gdk_draw_rectangle(pixmap, bar_gc, 
			TRUE, startx, starty - rect_height, rect_width, rect_height);

		/* draw little tick mark */
		gdk_draw_line(pixmap, gc, startx + rect_width - 1, starty,
				startx + rect_width - 1, starty - 5);
		
		//gdk_draw_rectangle(pixmap, gc, 
			//FALSE, startx, starty - rect_height, rect_width, rect_height);
		xpainted += rect_width;
	}
}


void draw_axes(Histogram *hist, GdkRectangle *rect) 
{
	//draw the x and y axes. 
	
	/* draw each column (bin) from the hist's LLL of bins. These 
	 * should be drawn inside the rectangle given as an argument.
	 * Everything will be drawn on hist->pixmap, which must not be NULL */
	GtkWidget *widget = GTK_WIDGET(hist->drawing_area);
	GdkGC *gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
	GdkPixmap *pixmap = hist->pixmap;

	int rect_width, rect_height; /* width and height of rect to paint */
	int xpainted; /* x-axis value of first unpainted pixel */
	int startx, starty; /* start co-ords for rectangles */

	int left_x = rect->x;
	int right_x = rect->x + rect->width;
	int bottom_y = rect->y + rect->height;

	int max_height = rect->height;
	int max_freq = hist->table_data.max_freq;

	Node *cur_node = hist->table_data.head;

	
	/* get scale: (double) this_freq / max_freq */

	/* draw each column */
	xpainted = left_x;
	rect_width = hist->column_width;
	starty = bottom_y;
	for (; cur_node; cur_node = cur_node->next) {
		g_print("this column freq: %d\n", cur_node->bin.frequency);
		rect_height = (((double)cur_node->bin.frequency) / max_freq) * max_height;
		startx = xpainted;
		if (startx > right_x) {
			g_print("yikes... drawing point x=%d is out of x=%d bounds.\n",
					startx, right_x);
			break; /* cannot draw out of bounds */
		}
		if (startx + rect_width > right_x) {
			g_print("yikes... drawing rect ending at x=%d is out of x=%d bounds.\n",
					startx + rect_width, right_x);
			break; /* cannot draw out of bounds */
		}

		/* draw this column 
		gdk_draw_rectangle(pixmap, bar_gc, 
			TRUE, startx, starty - rect_height, rect_width, rect_height); */

		/* draw little tick mark */
		gdk_draw_line(pixmap, gc, startx + rect_width - 1, starty,
				startx + rect_width - 1, starty - 5);
		
		//gdk_draw_rectangle(pixmap, gc, 
			//FALSE, startx, starty - rect_height, rect_width, rect_height);
		xpainted += rect_width;
	}
}



/* draws the tick marks and their corresponding labels on the y
 * axis.
 * In order for this to look right, the tick_rect rectangle passed in,
 * in which the tick marks and labels will be drawn in, must be vertically
 * aligned with the rectangle defining the area in which the histogram 
 * is drawn.  Specifically, tick_rect->y must equal hist_rect->y and 
 * tick_rect->height must equal hist_rect->height.
 * Also, the hist_rect should be just to the right of the tick_rect. */
void draw_y_tick_labels(Histogram *hist, GdkRectangle *tick_rect,
				GdkRectangle *hist_rect) 
{ /* BUG: tick_rect s/b called label_rect */
	 /* These 
	 * should be drawn inside the rectangle given as an argument.
	 * Everything will be drawn on hist->pixmap, which must not be NULL */
	GtkWidget *widget = GTK_WIDGET(hist->drawing_area);
	GdkPixmap *pixmap = hist->pixmap;
	GdkGC *gc = gdk_gc_new(GDK_DRAWABLE(pixmap));
	//GdkGC *gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
	GdkGC *bar_gc = gdk_gc_new(GDK_DRAWABLE(pixmap));
	GdkColormap *colormap;
	GdkColor bar_color;
	
	PangoLayout *layout;
	gint layout_width, layout_height; 
	GString *label;
	gint freq;
	gint tick_y, xleft, ytop, ybottom;
	gint max_freq, max_height, word_height;
	gint bottom_boundry = tick_rect->y + tick_rect->height, ypainted, axis_x;
	gint tick_spacing = 5, general_spacing = 5;
	
	label = g_string_new(NULL);

	
	colormap = gdk_drawable_get_colormap(GDK_DRAWABLE(pixmap));
	if (!gdk_color_parse("sky blue", &bar_color)) {
		g_print("blah apparently isn't a color!\n");
	} 
	if (!gdk_colormap_alloc_color(colormap, &bar_color, FALSE, TRUE)) {
		g_print("couln't allocate color\n");
	}


	/* draw blue rect around y tick labels (DEBUG) 
	gdk_gc_copy(bar_gc, gc);
	gdk_gc_set_foreground(bar_gc, &bar_color); 
	
	gdk_draw_rectangle(pixmap, bar_gc, TRUE, 
		 	tick_rect->x, tick_rect->y, tick_rect->width, tick_rect->height);
	*/



	gdk_gc_set_line_attributes(gc, 1, 
			GDK_LINE_ON_OFF_DASH, 
			GDK_CAP_BUTT, 
			GDK_JOIN_BEVEL);

		

	axis_x = hist_rect->x;
	ypainted = tick_rect->y;
	/* draw topmost tickmark and label */
	freq = hist->table_data.max_freq_absolute;
	g_string_printf(label, "%d", freq);
	layout = gtk_widget_create_pango_layout(widget, label->str);
	pango_layout_get_pixel_size(layout, &layout_width, &layout_height);
	gdk_draw_layout(pixmap, gc, 
					axis_x - layout_width - general_spacing, ypainted, layout);
	g_object_unref(layout);
	tick_y = hist_rect->y;
	gdk_draw_line(pixmap, gc, hist_rect->x, tick_y,
			hist_rect->x + hist_rect->width, tick_y);
	/*gdk_draw_line(pixmap, gc, hist_rect->x, tick_y,
			hist_rect->x + 5, tick_y);*/
	ypainted += layout_height;


	/* draw middle ticks/labels */ 

	word_height = hist->x_ticks_height_in_pixels;
	max_freq = hist->table_data.max_freq_absolute;
	max_height = tick_rect->height;

	for (freq = max_freq - 1;
					freq >= 0; --freq) {
		tick_y = bottom_boundry - (((double)freq)/max_freq)*max_height;
		g_string_printf(label, "%d", freq);
		layout = gtk_widget_create_pango_layout(widget, label->str);
		pango_layout_get_pixel_size(layout, &layout_width, &layout_height);

		ytop = tick_y - layout_height/2;
		ybottom = tick_y + layout_height/2;
		if (ytop < ypainted) {
				g_object_unref(layout);
				continue;
		} else if (ybottom > bottom_boundry) {
				g_object_unref(layout);
				break;
		} else {
				xleft = axis_x - layout_width - general_spacing;
				gdk_draw_layout(pixmap, gc, xleft, ytop,
								layout);
				g_object_unref(layout);
				gdk_draw_line(pixmap, gc, axis_x, tick_y, 
								axis_x + hist_rect->width, tick_y);
				ypainted = ytop + layout_height;
		}
	}
	g_string_free(label, TRUE);
	g_object_unref(gc);
	g_object_unref(bar_gc);
	return;
}



void draw_x_tick_labels(Histogram *hist, GdkRectangle *rect) 
{
	 /* These 
	 * should be drawn inside the rectangle given as an argument.
	 * Everything will be drawn on hist->pixmap, which must not be NULL.
	 * TODO: this function should take an extra argument indicating how 
	 * much room to the left of the rectangle is available for drawing the
	 * '0'. 
	 * Note: the leftmost label may not be zero.  If the minimum value
	 * from the frequency table with a non-zero frequency was N, the 
	 * leftmost label will be N, which is not necessarily zero. */
	GtkWidget *widget = GTK_WIDGET(hist->drawing_area);
	GdkGC *gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
	GdkPixmap *pixmap = hist->pixmap;
	GdkGC *bar_gc = gdk_gc_new(GDK_DRAWABLE(pixmap));
	GdkColormap *colormap;
	GdkColor bar_color;
	PangoLayout *layout;
	gint layout_width, layout_height; 
	GString *label;
	gint label_number, i, xpainted, tick_x;
	gint y_top, right_x_bounds;
	gint label_leftside, label_rightside;
#define BTWN_LABEL_SPACE 1 
	
	/* core things this () needs for drawing labels: */
	gint bar_width = hist->column_width;
	gint range = hist->table_data.range;
	gint n_bars = hist->table_data.n_bins;

	label = g_string_new(NULL);
	
	colormap = gdk_drawable_get_colormap(GDK_DRAWABLE(pixmap));
	if (!gdk_color_parse("sky blue", &bar_color)) {
		g_print("blue apparently isn't a color!\n");
	} 
	if (!gdk_colormap_alloc_color(colormap, &bar_color, FALSE, TRUE)) {
		g_print("couln't allocate color\n");
	}
	gdk_gc_copy(bar_gc, gc);
	gdk_gc_set_foreground(bar_gc, &bar_color);

	//gdk_draw_rectangle(pixmap, bar_gc, TRUE, 
		//	rect->x, rect->y, rect->width, rect->height);
	
	// TODO: draw '0' at far left of rectangle.
	
	label_number = hist->table_data.minval; // start labeling at minimum value
	//label_number = 0;
	xpainted = rect->x;
	right_x_bounds = rect->x + rect->width;
	tick_x = xpainted + bar_width;
	y_top = rect->y;
	for (i = 0; i < n_bars; ++i) {
			//for each bar (or its associated tick mark rather),
			//attempt to print a label
			
			label_number += range;
			g_string_printf(label, "%d", label_number);
			layout = gtk_widget_create_pango_layout(widget, label->str);
			pango_layout_get_pixel_size(layout, &layout_width, &layout_height);
			label_leftside = tick_x - (layout_width/2);
			label_rightside = label_leftside + layout_width;
			
			// if the right side of the label would be to the 
			// right of the far right of the rectangle, quit.
			//if (tick_x + (layout_width/2) > right_x_bounds) {
			if (label_rightside > right_x_bounds) {
					g_print("not enough room (right side) for bin %d label\n", i);
					g_object_unref(layout);
					break;
			} else if (label_leftside < xpainted ) {
			//} else if (tick_x - (layout_width/2) < xpainted ) {
				 /*
					g_print("not enough room (left side)  for bin %d label\n", i);
					g_print("methinks %d NOTFITS: xp:%d] [%d   %d]\n", 
									label_number, 
									xpainted, label_leftside, label_rightside); */
				//if the left side of the label would be to the left of 
				//xpainted (a shifting boundry), don't add this label.
					// skip labeling this tick mark so we don't paint over
					// something already drawn.
					g_object_unref(layout);
			} else {
					/* looks like we'll fit. label it. */
					/*
					g_print("methinks %d fits; adding it xp:%d] [%d   %d]\n", 
									label_number, 
									xpainted, label_leftside, label_rightside); */
					gdk_draw_layout(hist->pixmap, gc, 
									label_leftside,
									y_top,
									layout);
					g_object_unref(layout);
					xpainted = label_rightside + BTWN_LABEL_SPACE;
					//xpainted += layout_width + BTWN_LABEL_SPACE;
			}
			tick_x += bar_width;
	}
	g_string_free(label, TRUE);
	return;
}




#define Y_TICK_WIDTH 7
Histogram * histogram_new(GtkDrawingArea *draw_area)
{
	/* create new Histogram structure and fill it in */
	Histogram *hist_widget = g_new0(Histogram, 1);
	PangoLayout *layout;
	gchar strbuf[Y_TICK_WIDTH + 1];
	gint i, y_tick_pix_width, y_tick_pix_height;
	hist_widget->pixmap = NULL; /* not created here */
	hist_widget->drawing_area = draw_area;
	
	hist_widget->column_width = 20;	
	
	// todo: add set_space.. function to set all independently,
	// and also set_generic_space to set all to one value.
	hist_widget->side_spacing = 10;
	hist_widget->title_space_above = 10;
	hist_widget->y_label_space_above = 5;
	hist_widget->histogram_space_above = 5;
	hist_widget->x_label_space_above = 10;
	hist_widget->x_label_space_below = 10;

	hist_widget->x_label = g_strdup("");
	hist_widget->y_label = g_strdup("Frequency");

	hist_widget->y_tick_width = Y_TICK_WIDTH;

	hist_widget->freq_table = NULL;
	hist_widget->freq_table_max_value = 0;
	
	//hist_widget->table_data = g_new(FreqTableData, 1);
	hist_widget->prev_n_bins = -1; 

	hist_widget->config_done = 0;

	//hist_widget->bin_list = NULL; /* this set on call after configure event */
	
	/* create text of y_tick_width chars and find its pixel width */
	for (i = 0; i < hist_widget->y_tick_width; i++) {
		strbuf[i] = '7';
	}
	strbuf[Y_TICK_WIDTH] = '\0';

	layout = gtk_widget_create_pango_layout(GTK_WIDGET(draw_area), strbuf);
	pango_layout_get_pixel_size(layout, &y_tick_pix_width, &y_tick_pix_height);
	hist_widget->y_tick_width_in_pixels = y_tick_pix_width;
	hist_widget->x_ticks_height_in_pixels = y_tick_pix_height;

	//g_print("%s(): y_tick_width_in_pixels: %d, height in pix: %d\n",
		//			__FUNCTION__, y_tick_pix_width, y_tick_pix_height);

	
	/* connect the histogram_* handlers to the various drawing area events */
	/* (this includes configure event when size changes, and 
	 * expose event, when the drawing area (or parts of it?) is shown */
	g_signal_connect(GTK_OBJECT(draw_area), "expose_event", 
			(GtkSignalFunc) histogram_expose, hist_widget);
	g_signal_connect(GTK_OBJECT(draw_area), "configure_event", 
			(GtkSignalFunc) histogram_configure, hist_widget);


	/* return pointer to the Histogram "widget" */
	return hist_widget;
}


/* histogram_paint: 
 *
 * draw the histogram image (including labels, etc) 
 * on the histogram's pixmap */
void histogram_paint (Histogram *hist, GtkWidget *widget)
{
	/* (x,y) variables are like this:
	 *
	 * (orig_x,side_end_y)
	 * |
	 * |
	 * |
	 * |
	 * (orig_x,orig_y) _____________ (bottom_end_x,orig_y)
	 */
	int orig_x, orig_y, bottom_end_x, side_end_y;
	int height, width;
	int variable_top = 0, variable_bottom = 0, variable_side = 0;
	int n_bins;
	GdkRectangle hist_rect_area, x_tick_area, y_tick_area;
	GdkGC *gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
	//PangoContext *pc = gtk_widget_create_pango_context(widget);
	PangoLayout *layout;
	int layout_width, layout_height; 
	//GdkPoint axis_points[3];


	/* If for some reason the actual data doesn't exist (and thus
	 * the pixmap is probably NULL also), we should display an image
	 * which states that the test needs to be run. */
		// if the data doesn't exist, print that message in histogram's place
		//todo: set the pointer to the image to draw to be the ptr to 
		//the "not yet got data" image. 


	if (hist->pixmap)
		gdk_pixmap_unref(hist->pixmap);
	
	height = widget->allocation.height;
	width = widget->allocation.width;
	hist->pixmap = gdk_pixmap_new(widget->window, width, height, -1);
	/* clear the pixmap */
	gdk_draw_rectangle(hist->pixmap, 
			widget->style->white_gc, TRUE, 0,0, width, height);
	

	/* draw the labels */
	//g_object_unref() s/b used on layout unless we store it somewhere.
	
	/* draw title */
	layout = gtk_widget_create_pango_layout(widget, hist->title);
	pango_layout_get_pixel_size(layout, &layout_width, &layout_height);
	variable_top = 0 + hist->title_space_above;
	gdk_draw_layout(hist->pixmap,
			gc, (width-layout_width)/2, 
			variable_top, 
			layout);
	variable_top += layout_height;
	g_object_unref(layout);
	
	/* draw x_axis. */

	/* draw x axis label */
	layout = gtk_widget_create_pango_layout(widget, hist->x_label);
	pango_layout_get_pixel_size(layout, &layout_width, &layout_height);
	variable_bottom = height - hist->x_label_space_below - layout_height;
	gdk_draw_layout(hist->pixmap,
			gc, (width-layout_width)/2, variable_bottom, layout);
	g_object_unref(layout);

	/* draw y_label */
	layout = gtk_widget_create_pango_layout(widget, hist->y_label);
	pango_layout_get_pixel_size(layout, &layout_width, &layout_height);
	variable_top += hist->y_label_space_above;
	//variable_top += hist->y_label_space_above + layout_height;
	gdk_draw_layout(hist->pixmap,
			gc, hist->side_spacing,
			variable_top,
			//0+layout_height+hist->title_space_above, 
			layout);
	variable_top += layout_height;
	g_object_unref(layout);

	
	/* draw the scale and stuff */
	variable_side += hist->side_spacing;

	variable_bottom -= hist->x_label_space_above;
	variable_bottom -= hist->x_ticks_height_in_pixels;

	y_tick_area.x = variable_side;
	y_tick_area.y = variable_top;
	y_tick_area.width = hist->y_tick_width_in_pixels;
	y_tick_area.height = variable_bottom - variable_top;

	//g_print("%s(): y_tick_area.x: %d\n", __FUNCTION__, y_tick_area.x);
	//g_print("%s(): y_tick_area.y: %d\n", __FUNCTION__, y_tick_area.y);
	//g_print("%s(): y_tick_area.width: %d\n", __FUNCTION__, y_tick_area.width);
	//g_print("%s(): y_tick_area.height: %d\n", __FUNCTION__, y_tick_area.height);

	variable_side += hist->y_tick_width_in_pixels;



	
	/* draw the axes */
	/* set up the axes lines */
	orig_x = variable_side;
	orig_y = variable_bottom - 1;
	//orig_y = variable_bottom - hist->x_label_space_above;
	bottom_end_x = width - hist->side_spacing;
	//side_end_y = variable_top + hist->histogram_space_above;
	side_end_y = variable_top; // we don't need room between label and hist

	/* set up the x tick label area */
	x_tick_area.x = variable_side; 
	x_tick_area.y = variable_bottom;
	x_tick_area.width = bottom_end_x - variable_side;
	x_tick_area.height = hist->x_ticks_height_in_pixels;

	GdkPoint axis_points[] = { 
		{ orig_x, side_end_y },
		{ orig_x, orig_y },
		{ bottom_end_x, orig_y }
	};
	gdk_draw_lines(hist->pixmap, gc,  axis_points, 3);
	++variable_side; //left side shifts right 1 pixel, due to x-axis
	--variable_bottom; //bottom shifts up 1 pixel, due to y-axis 
	
	n_bins = (bottom_end_x - orig_x - 1) / hist->column_width;
	g_print("%s: calculations say to do %d bins\n", __FUNCTION__,  n_bins);
	//g_print("hist_configure(): calculations say to do %d bins\n", n_bins);

	if (hist->freq_table) {
		convert_freq_table_to_bins (&hist->table_data, hist->freq_table, 
			hist->freq_table_max_value, n_bins, hist->prev_n_bins);
	} else {
		convert_freq_table_to_bins (&hist->table_data, freq_table, 
			sizeof(freq_table) / sizeof(int), n_bins, hist->prev_n_bins);
	}

	if (! hist->config_done ) {
		hist->table_data.max_freq_absolute = hist->table_data.max_freq;
		hist->config_done = 1;
	}

	if (hist->prev_n_bins == hist->table_data.n_bins) {
		g_print("%s(): bin list not regenerated.\n", __FUNCTION__);
	}
	hist->prev_n_bins = hist->table_data.n_bins;
	if (hist->table_data.n_bins != n_bins) {
		g_print("%s: Whoa.. requested %d bins, but got %d\n", __FUNCTION__,
						n_bins, 
				hist->table_data.n_bins);
	}
	g_print("%s(): convert() gave us %d bins\n", __FUNCTION__,
			hist->table_data.n_bins);
	
	/*hist_rect_area.x = orig_x;
	hist_rect_area.y = side_end_y;
	hist_rect_area.width = bottom_end_x - orig_x;
	hist_rect_area.height = orig_y - side_end_y;*/

	hist_rect_area.x = variable_side;
	hist_rect_area.y = side_end_y;
	hist_rect_area.width = bottom_end_x - variable_side;
	hist_rect_area.height = orig_y - side_end_y;

	//g_print("%s(): hist_rect_area.x: %d\n", __FUNCTION__, hist_rect_area.x);
	//g_print("%s(): hist_rect_area.y: %d\n", __FUNCTION__, hist_rect_area.y);
	//g_print("%s(): hist_rect_area.width: %d\n", 
		//			__FUNCTION__, hist_rect_area.width);
	//g_print("%s(): hist_rect_area.height: %d\n", 
		//			__FUNCTION__, hist_rect_area.height);

	draw_histogram(hist, &hist_rect_area);
	draw_x_tick_labels(hist, &x_tick_area);
	draw_y_tick_labels(hist, &y_tick_area, &hist_rect_area);

	return;
}



/* histogram_configure:
 * The size of the widget has changed. Deal with it. */
void histogram_configure (GtkWidget *widget, GdkEventConfigure *event,
		gpointer *data)
{
	//g_print("entered histogram_configure for \"%s\"\n", ((Histogram *)data)->title);
	histogram_paint ((Histogram *)data, widget);
	return;
}



void histogram_expose (GtkWidget *widget, GdkEventExpose *event, 
	gpointer *data)
{
	Histogram *hist = (Histogram *)data;
	//g_print("entered histogram_expose for \"%s\"\n", hist->title); //DEBUG
	if (! hist->pixmap) { 
			g_print("yikes, pixmap is NULL!\n"); // we better generate it:
			histogram_paint ((Histogram *)data, widget);
	}
	gdk_draw_drawable(widget->window, 
		widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		hist->pixmap, 
		event->area.x, event->area.y,
		event->area.x, event->area.y,
		event->area.width, event->area.height);
	return;
}



void histogram_set_title(Histogram *hist, const gchar *title)
{
	if (hist->title) 
		g_free(hist->title);

	hist->title = g_strdup(title);
	return;
}

void histogram_set_x_label(Histogram *hist, const gchar *x_label)
{
	if (hist->x_label)
		g_free(hist->x_label);

	hist->x_label = g_strdup(x_label);
	return;
}

void histogram_set_y_label(Histogram *hist, const gchar *y_label)
{
	if (hist->y_label)
		g_free(hist->y_label);

	hist->y_label = g_strdup(y_label);
	return;
}

void histogram_set_y_tick_width(Histogram *hist, gint width)
{
	hist->y_tick_width = width;
	return;
}

void histogram_set_frequency_table(Histogram *hist, gint *table, gint maxval)
{
	hist->freq_table = table;
	hist->freq_table_max_value = maxval;
	return;
}

/* convert_freq_table_to_bins: convert the frequency table to 
 * a list of bins, where each bin holds a range of values.
 * Each range will be equal to (maxval - minval)/n_bins, where
 * minval is the minimum index in the table with a value greater
 * than zero. 
 * @prev_n_bins: previous number of bins generated by this function the
 *  time it was called. If prev_n_bins equals the actual # of bins that
 *  will be created, then this function will not recreate the list. 
 *  Setting this value to -1 will force the function to recreate the list.
 * Require: n_bins must be greater than zero. 
 * Returns: FreqTableData, which holds minval, maxval, maxfreq,  
 *  n_bins (the actual # of bins created), and the ptr to head of list. 
 *  Note that the actual number of bins used may be less
 *  than n_bins if the number of bins requested is greater than 
 *  the number of bins we can possibly create (i.e. n_bins is greater
 *  than maxval - minval. */
void convert_freq_table_to_bins (FreqTableData *table_data, 
		gint *freq_table, gint array_size, 
		gint n_bins, gint prev_n_bins)
{
	// only modify table_data if prev_n_bins != actual n bins
	Node *cur_node, *next_node, *list_head;
	gint actual_n_bins;
	gint i, j, cur_value;
	gint cur_bin, target_bin;
	gint minval, maxval, max_freq = 0, cur_freq;
	gint range; /* range is the number of values held per bin. */
	gint total_range; /* total_range is inclusive, so if maxval is 6 and
											minval is 1, the total_range is 6, not 5. */

	g_print("File: %s Line: %d _convert(..., array_size:%d, n_bins:%d, prev_n_bins:%d)\n",
					__FILE__, __LINE__, array_size, n_bins, prev_n_bins);
	
	/* find beginning of non-zero values */
	for (i = 0; *(freq_table + i) == 0 && i < array_size; ++i);
	minval = i;

	/* find end of non-zero values */
	for (j = array_size - 1; *(freq_table + j) == 0 && j >= 0; --j);
	maxval = j;



	//////total_range = maxval - minval;
	total_range = maxval - minval + 1;
	if (n_bins > total_range) {
		range = 1; 
		// we cannot have more bins than elements.
		actual_n_bins = total_range;
	} else {
		// if total_range / n_bins isn't exact (it has a remainder), then
		// the last bin will have to contain the extra elements. WRONG.
		// The reason the method just described is wrong is because in
		// the histogram, all bins must have the *same range*. Instead:
		// If total_range % n_bins != 0, then the range must be increased 
		// to hold all elements in n_bins or less, so 
		// range = (total_range / n_bins) +1.
		if ( total_range % n_bins ) {
			range = (total_range / n_bins) + 1;
			actual_n_bins = ceil(total_range / (double)range);
		} else {
			range = total_range / n_bins;
			actual_n_bins = n_bins;
		}
	}

	if (actual_n_bins == prev_n_bins) {
		return; // don't re-create list when unnecessary
		//BUG? Normally, we don't want to recreate the list
		//if it looks like we've got the right size (n_bins) already.
		//This is done so that if the user resize the window just slightly,
		//the data doesn't need to be re-analyzed/re-packed into bins.
		//But: if somehow the underlying frequency table has changed,
		// the user shouldn't have to change the window size just to
		// get this code to recreate the list.  There should be a 
		// flag variable to tell us re-packing the data into a new
		// bin-list is ok, OR histogram_set_freq... should set the
		// new frequency table and tell some other code to regenerate
		// the list of bins. 
	} 

	/* now that we're recreating the list, we need to free the old one. */
	/* TODO */
	
	table_data->minval = minval;
	table_data->maxval = maxval;
	table_data->n_bins = actual_n_bins;
	table_data->range = range;

	g_print("_convert(): min:%d max:%d bins:%d range:%d total_range:%d\n",
					minval, maxval, actual_n_bins, range, total_range);
	
	list_head = cur_node = g_new(Node, 1);
	cur_node->bin.start_value = minval;
	cur_node->bin.end_value = minval + range;
	cur_node->bin.frequency = cur_node->bin.n_values = 0;
	cur_bin = 0;
	for (cur_value = minval; cur_value <= maxval; ++cur_value) {
		cur_freq = *(freq_table + cur_value);
		if ( cur_freq != 0) {
		//if ( *(freq_table + cur_value) != 0) {
			//compute bin for this index value
			//target_bin = cur_value / range;
			target_bin = (cur_value - minval) / range;
			
			if (target_bin > cur_bin ) { 
			//if (target_bin > cur_bin && cur_bin != 0) { 
					/* add all bins until target_bin (unless we're on first bin:
					 * we skip all bins that would be zero) */
				for (; cur_bin < target_bin; ++cur_bin) {
					//g_print("Adding bin %d to list\n", cur_bin); DEBUG
					next_node = g_new(Node, 1);
					cur_node->next = next_node;
					next_node->bin.start_value = cur_node->bin.end_value + 1;
					next_node->bin.end_value = next_node->bin.start_value + range;
					next_node->bin.frequency = next_node->bin.n_values = 0;
					cur_node = next_node;
				}
			}

			/* if (target_bin == cur_bin)  */
			/* add this value to current bin */
				++ cur_node->bin.n_values;
				cur_node->bin.frequency += cur_freq;
				//cur_node->bin.frequency += *(freq_table + cur_value);
				if (cur_node->bin.frequency > max_freq) {
					max_freq = cur_node->bin.frequency;
				}
		}
	}
		
	cur_node->next = NULL; /* cap the list */
	table_data->head = list_head;	
	table_data->max_freq = max_freq;
	return;
}

