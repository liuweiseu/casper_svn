#include <stdlib.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "chkgap_gui_callbacks.h"
#include "chkgap_gui_histogram.h"
#include "check_gap_lib.h"

extern Histogram *blocks_hist, *gaps_hist;

int load_new_tables_from_test(CheckGapData *d, int cur_channel)
{
		GapHistory *results = &d->gaps[cur_channel];

		printf("loading new tables for results from test on chan #%d\n", 
						cur_channel);
		printf("last_index: %d\n", results->last_index);

		chkgap_output_gap(d, cur_channel); /* DEBUG: what does data look like? */

		histogram_set_frequency_table(blocks_hist, 
						results->blocks, results->blocks_largest_index + 1);
						//results->blocks, d->maxfreq);
		histogram_set_frequency_table(gaps_hist, 
						results->freqs, results->gaps_largest_index + 1);
						//results->freqs, d->maxfreq);
						//results->freqs, results->last_index);
		return 0;
}

void on_quit1_activate(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}


void on_about1_activate (GtkWidget *menuitem, gpointer data)
{
		GladeXML *xml;
		GtkWidget *main_window, *dialog;
		xml = glade_get_widget_tree(menuitem);
		main_window = glade_xml_get_widget(xml, "window1");

		dialog = gtk_message_dialog_new(
						GTK_WINDOW(main_window), GTK_DIALOG_MODAL,
						GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, 
						"EDT PCI Gaps/Blocks Tool.  Written in 2004 by Peter Welte \
 Contact peter@edt.com with questions, bugs, etc.");
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		return;
}
		

void on_view_gap_histogram1_activate(GtkWidget *menuitem, gpointer data)
{
	GladeXML *xml;
	GtkWidget *widget, *notebook;
	gint page;
	/* prepare to draw */
	xml = glade_get_widget_tree(menuitem);
	notebook = glade_xml_get_widget(xml, "notebook1");
	widget = glade_xml_get_widget(xml, "gaps_drawingarea");
	page = gtk_notebook_page_num(GTK_NOTEBOOK(notebook), widget);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), page);
	return;
}



void on_view_block_histogram1_activate(GtkWidget *menuitem, gpointer data)
{
	GladeXML *xml;
	GtkWidget *widget, *notebook;
	gint page;
	/* prepare to draw */
	xml = glade_get_widget_tree(menuitem);
	notebook = glade_xml_get_widget(xml, "notebook1");
	widget = glade_xml_get_widget(xml, "blocks_drawingarea");
	page = gtk_notebook_page_num(GTK_NOTEBOOK(notebook), widget);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), page);
	return;
}



void on_run_checkgap1_activate(GtkWidget *widget, gpointer data)
{
		/* this function either:
		 * a) calls checkgap() and sets the histograms' data based on
		 * thre returned arrays. or
		 * b) assumes histograms know how to do a) and asks them to do so.
		 *
		 * I think this func() s/ do (a), because checkgap() will return arrays
		 * for gaps and blocks at the same time but it would need to be called
		 * by both the gap-histogram and the block-histogram, which would 
		 * actually get the results of two separate tests (one after the other).
		 */
		g_print("inside on_run_checkgap1_activate\n"); //DEBUG
		CheckGapData *test_data = calloc(1, sizeof(CheckGapData));
		test_data->clock_speed = 33000000;
		test_data->bufsize = 1024*1024;
		test_data->loops = 1;
		test_data->bitload = 1;
		test_data->unitstr = "0";

#ifndef NO_HARDWARE
		/* run checkgap test and store freq tables in histograms */
		//does very little, but does set unit/dev:
		chkgap_parse_options(0, NULL, test_data); 
		/* get the board and program state ready for test: */
		chkgap_initialize (test_data); 
		chkgap_run_test(test_data,
					((ResultHandler)(load_new_tables_from_test)) 
					);  /* run the test & save results */
#endif


}

