#include <gtk/gtk.h>
#include <glade/glade.h>
#include <stdlib.h>
#include "chkgap_gui_histogram.h"
#include "check_gap_lib.h"
#include "chkgap_gui_callbacks.h"
	
Histogram *blocks_hist, *gaps_hist, *scrap_hist;

int my_freq_table[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7,
		8, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

int my_freq_table2[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7,
		8, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10, 20, 20, 20,
		100, 1024, 1024, 10, 0, 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024,
		2048, 4096, 8192 };

int *my_gaps;
int *my_blocks;
#define TEST_SIZE 6000

void create_test_data(void)
{
		my_gaps = calloc(TEST_SIZE, sizeof(int));
		my_blocks = calloc(TEST_SIZE, sizeof(int));

		//my_blocks[0] = 100;
		my_blocks[63] = 4;
		my_blocks[64] = 4092;
		my_blocks[5000] = 2000;

		my_gaps[9] = 3823;
		my_gaps[11] = 4;
		my_gaps[15] = 7;
		my_gaps[16] = 2;
		my_gaps[17] = 2;
		my_gaps[21] = 2;
		my_gaps[27] = 1;
		my_gaps[30] = 188;
		my_gaps[45] = 56;
		my_gaps[46] = 2;
		my_gaps[47] = 2;
		my_gaps[48] = 3;
		my_gaps[49] = 1;
		my_gaps[52] = 1;
		my_gaps[55] = 1;
		my_gaps[4000] = 1000;
		return;
}

void load_test_data2(void)
{
		histogram_set_frequency_table(blocks_hist, my_blocks, TEST_SIZE);
		histogram_set_frequency_table(gaps_hist, my_gaps, TEST_SIZE);
		return;
}



void load_test_data(void)
{
		/* pass the histogram, the gint *table, and the int max_index */
		int table_len = sizeof(my_freq_table) / sizeof(int);
		histogram_set_frequency_table(blocks_hist, 
						my_freq_table, table_len);
		histogram_set_frequency_table(gaps_hist, 
						my_freq_table, table_len);
		return;
}
						

int main (int argc, char *argv[])
{
	GladeXML *xml;
	//GtkWidget *notebook;
	GtkWidget *blocks_draw_area, *gaps_draw_area;
	CheckGapData *data = calloc(1, sizeof(CheckGapData));
	data->clock_speed = 33000000;
	data->bufsize = 1024*1024;
	data->loops = 1;
	data->bitload = 1;
	data->unitstr = "0";

	//Histogram *blocks_hist;

	gtk_init(&argc, &argv);
	glade_init();

	/* load the interface */
	xml = glade_xml_new("checkgap.glade", NULL, NULL);

	//GtkWidget *scrap_drawable = gtk_drawing_area_new();
	//scrap_hist = histogram_new(GTK_DRAWING_AREA(scrap_drawable));

	gaps_draw_area = glade_xml_get_widget(xml, "gaps_drawingarea");
	gaps_hist = histogram_new(GTK_DRAWING_AREA(gaps_draw_area));
	histogram_set_x_label(gaps_hist, "Gap Size");
	histogram_set_y_label(gaps_hist, "Frequency");
	histogram_set_title(gaps_hist, 
			"Frequency of gap sizes in PCI transfers");
	
	blocks_draw_area = glade_xml_get_widget(xml, "blocks_drawingarea");
	blocks_hist = histogram_new(GTK_DRAWING_AREA(blocks_draw_area));
	histogram_set_x_label(blocks_hist, "Block Size");
	histogram_set_y_label(blocks_hist, "Frequency");
	histogram_set_title(blocks_hist, 
			"Frequency of block sizes in PCI transfers");


	/*** the following may be commeted out so I can develop w/o the EDT board */
#ifdef NO_HARDWARE
	create_test_data();
	load_test_data2(); 
#else /* have hardware: */
	/* run checkgap test and store freq tables in histograms */
	chkgap_parse_options(argc, argv, data); //does very little, but does set unit/dev
	chkgap_initialize (data); /* get the board and program state ready for test */
	chkgap_run_test(data,
					((ResultHandler)(load_new_tables_from_test)) 
					);  /* run the test & save results */
#endif


	/* connect the signals in the interface */
	glade_xml_signal_autoconnect(xml);

	/* start the event loop */
	gtk_main();

	return 0;
}
