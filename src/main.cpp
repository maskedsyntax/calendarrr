#include <gtk/gtk.h>

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create a new window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Calendarrr");

    // Create a vertical box to hold the calendar
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create a calendar widget
    GtkWidget *calendar = gtk_calendar_new();
    gtk_widget_set_valign(calendar, GTK_ALIGN_FILL);
    gtk_widget_set_halign(calendar, GTK_ALIGN_FILL);
    
    // Pack the calendar into the box with expand=TRUE and fill=TRUE
    gtk_box_pack_start(GTK_BOX(vbox), calendar, TRUE, TRUE, 0);

    // Connect the window close event
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    return 0;
}