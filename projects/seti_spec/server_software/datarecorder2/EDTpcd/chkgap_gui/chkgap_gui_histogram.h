#ifndef _Histogram_h_
#define _Histogram_h_
#include <gtk/gtk.h>

/* Histogram: a psuedo-gtk widget.  The histogram "widget" actually uses
 * a GtkDrawingArea to handle the lowest level GTK stuff.  This "widget"
 * holds the data the histogram represents (a set of bins) and has code
 * which responds to the users actions, such as resizing the histogram
 * image when the window it is in is resized. */
/* TODO: the set of bins comes from a table of values and frequencies...
 *   there is a function to convert the table to a list of bins, but
 *   it should probably be part of the histogram's code if it isn't already,
 *   and the histogram should keep a reference to the frequency table 
 *   so it can recreate the bin list if neccessary. 
 *   Also, there should be a way to tell the histogram to retrieve a new
 *   frequency table. (Histogram s/ have a func() which takes a *func(),
 *   which it can call and which returns a freq. table and its size ) */
/*
typedef struct {
	int size;
	int frequency;
} GapInfo; //OLD
*/

typedef struct _Bin Bin;
struct _Bin {
	int start_value, end_value; /* specifies the range of values held */
	int frequency; /* total of all frequencies for this range. */
	int n_values; /* number of values contributing frequencies to this bin. */
};

typedef struct _Node Node;
struct _Node {
	Bin bin;
	Node *next;
};

typedef struct _FreqTableData FreqTableData;
struct _FreqTableData {
	/* The smallest and largest values with nonzero frequencies. */
	int minval, maxval;
	int range; /* range of values held in each bin */
	/* largest frequency of all bins, when data is squished most: */
	int max_freq_absolute; 
	int max_freq; /* largest frequency of all the bins. */
	int min_freq; /* smallest frequency of all the bins. */
	int n_bins; 
	Node *head; /* head of linked list of bins */
};

/* draw test histogram on widget */
void histogram_draw_test(GtkDrawingArea *widget);

struct _Histogram {
	gint config_done; /* whether or not config() has been called yet (0=no). */
	GdkPixmap *pixmap;
	GtkDrawingArea *drawing_area; /* the widget to draw on */
	/* number of pixels btwn each side of histogram and each side of draw area: */
	gint side_spacing; 

	gint column_width; /* width of columns in pixels */

	gint title_space_above; /* space between title and top of draw area */
	gint y_label_space_above; /* space btwn. y_label and title */
	gint histogram_space_above; /* space btwn. histogram and y axis label */
	gint x_label_space_above; /* space btwn. x label and scale of hist. */
	gint x_label_space_below; /* space btwn. x label and bottom of draw area */
	gint y_tick_width; /* max width in digets of y axis tick labels. */
	gint y_tick_width_in_pixels; /* max width of y axis tick labels */
	gint x_ticks_height_in_pixels; /* max height of x-axis tick labels */

	gint *freq_table; /* for each N, N is the value and freq_table[N] is 
											 the frequency the value occurs */
	gint freq_table_max_value; /* actually, this is freq_table_len */

	FreqTableData table_data; /* holds info about the set of bins. */
	gint prev_n_bins; /* previous number of bins created by convert() */
	
	gchar *title; /* histogram title */
	gchar *x_label; /* label for the x axis */
	gchar *y_label; /* label for the y axis */
};
typedef struct _Histogram Histogram;


void histogram_expose (GtkWidget *widget, GdkEventExpose *event, gpointer *data);

void histogram_configure (GtkWidget *widget, GdkEventConfigure *event,
		gpointer *data);

Histogram * histogram_new(GtkDrawingArea *draw_area);

void histogram_set_title(Histogram *hist, const gchar *title);
void histogram_set_x_label(Histogram *hist, const gchar *x_label);
void histogram_set_y_label(Histogram *hist, const gchar *y_label);
/* histogram_set_y_tick_width: set the maximum width of the tick labels
 * on the Y axis in terms of digets (e.g. if max # is 900, width is 3). 
 * This is only a suggestion, and if the histogram finds the guess to
 * be wrong, it will use the correct value. 
 * Defaults to 7. */
void histogram_set_y_tick_width(Histogram *hist, gint width);
/* histogram_set_frequency_table: 
 * set the table to create the histogram from. The table
 * is expected to run from n = 0 to table_len-1, where 
 * n is the value, and table[n] is the frequency that the value occurs.
 *
 * table: the array of ints which represents values and their frequencies 
 * table_len: length of the array.  table[table_len-1] is last element of
 * table. */
void histogram_set_frequency_table(Histogram *hist, 
		gint *table, gint table_len);


#endif /* _Histogram_h_ */
