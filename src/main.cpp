#include <gtk/gtk.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <ctime>

// Global pointers for widgets we need to access in callbacks
GtkWidget *calendar_widget;
GtkWidget *text_view;
GtkWidget *date_label;

// Simple map to store events in memory: "YYYY-MM-DD" -> "Note content"
std::map<std::string, std::string> events_db;

// Helper to get the config directory path
std::string get_config_dir() {
    const char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        home = pw->pw_dir;
    }
    std::string path = std::string(home) + "/.config/calendarrr";
    mkdir(path.c_str(), 0755);
    return path;
}

std::string get_events_file() {
    return get_config_dir() + "/events.db";
}

void load_events_from_disk() {
    std::ifstream file(get_events_file());
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('|');
        if (pos != std::string::npos) {
            std::string date = line.substr(0, pos);
            std::string note = line.substr(pos + 1);
            
            // Unescape \n and \|
            std::string unescaped;
            for (size_t i = 0; i < note.length(); ++i) {
                if (note[i] == '\\' && i + 1 < note.length()) {
                    if (note[i+1] == 'n') { unescaped += '\n'; i++; }
                    else if (note[i+1] == '|') { unescaped += '|'; i++; }
                    else if (note[i+1] == '\\') { unescaped += '\\'; i++; }
                    else unescaped += note[i];
                } else {
                    unescaped += note[i];
                }
            }
            events_db[date] = unescaped;
        }
    }
    file.close();
}

void save_events_to_disk() {
    std::ofstream file(get_events_file());
    for (auto const& [date, note] : events_db) {
        if (note.empty()) continue;
        
        std::string escaped;
        for (char c : note) {
            if (c == '\n') escaped += "\\n";
            else if (c == '|') escaped += "\\|";
            else if (c == '\\') escaped += "\\\\";
            else escaped += c;
        }
        file << date << "|" << escaped << std::endl;
    }
    file.close();
}

std::string get_selected_date_string() {
    guint year, month, day;
    gtk_calendar_get_date(GTK_CALENDAR(calendar_widget), &year, &month, &day);
    char buffer[11];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", year, month + 1, day);
    return std::string(buffer);
}

void update_ui_for_date() {
    std::string date_str = get_selected_date_string();
    
    // Update label
    std::string label_text = "Notes for " + date_str + ":";
    gtk_label_set_text(GTK_LABEL(date_label), label_text.c_str());

    // Update text view
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    if (events_db.count(date_str)) {
        gtk_text_buffer_set_text(buffer, events_db[date_str].c_str(), -1);
    } else {
        gtk_text_buffer_set_text(buffer, "", -1);
    }
}

void refresh_calendar_marks() {
    guint year, month, day;
    gtk_calendar_get_date(GTK_CALENDAR(calendar_widget), &year, &month, &day);
    
    gtk_calendar_clear_marks(GTK_CALENDAR(calendar_widget));
    
    char prefix[8]; // YYYY-MM
    snprintf(prefix, sizeof(prefix), "%04d-%02d", year, month + 1);
    std::string s_prefix(prefix);

    for (auto const& [date, note] : events_db) {
        if (!note.empty() && date.substr(0, 7) == s_prefix) {
            int d = std::stoi(date.substr(8, 2));
            gtk_calendar_mark_day(GTK_CALENDAR(calendar_widget), d);
        }
    }
}

// Callbacks
void on_day_selected(GtkCalendar *calendar, gpointer user_data) {
    update_ui_for_date();
}

void on_month_changed(GtkCalendar *calendar, gpointer user_data) {
    refresh_calendar_marks();
}

void on_save_clicked(GtkButton *button, gpointer user_data) {
    std::string date_str = get_selected_date_string();
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    
    events_db[date_str] = std::string(text);
    g_free(text);
    
    save_events_to_disk();
    refresh_calendar_marks();
    
    // Provide a simple visual feedback (optional, maybe status bar later)
    std::cout << "Saved notes for " << date_str << std::endl;
}

void on_today_clicked(GtkButton *button, gpointer user_data) {
    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    gtk_calendar_select_month(GTK_CALENDAR(calendar_widget), now->tm_mon, now->tm_year + 1900);
    gtk_calendar_select_day(GTK_CALENDAR(calendar_widget), now->tm_mday);
    update_ui_for_date();
    refresh_calendar_marks();
}

void on_delete_clicked(GtkButton *button, gpointer user_data) {
    std::string date_str = get_selected_date_string();
    if (events_db.count(date_str) && !events_db[date_str].empty()) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_QUESTION,
                                                 GTK_BUTTONS_YES_NO,
                                                 "Are you sure you want to delete the note for %s?",
                                                 date_str.c_str());
        
        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        if (result == GTK_RESPONSE_YES) {
            events_db.erase(date_str);
            save_events_to_disk();
            update_ui_for_date();
            refresh_calendar_marks();
        }
    }
}

void on_search_changed(GtkSearchEntry *entry, gpointer user_data) {
    const char *query = gtk_entry_get_text(GTK_ENTRY(entry));
    if (strlen(query) < 2) return; // Don't search for tiny strings

    std::string s_query(query);
    for (auto const& [date, note] : events_db) {
        if (note.find(s_query) != std::string::npos) {
            // Found a match, jump to it
            int y = std::stoi(date.substr(0, 4));
            int m = std::stoi(date.substr(5, 2));
            int d = std::stoi(date.substr(8, 2));
            
            gtk_calendar_select_month(GTK_CALENDAR(calendar_widget), m - 1, y);
            gtk_calendar_select_day(GTK_CALENDAR(calendar_widget), d);
            update_ui_for_date();
            refresh_calendar_marks();
            break; // Jump to the first match for now
        }
    }
}

void on_export_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Export Notes",
                                                  GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
                                                  GTK_FILE_CHOOSER_ACTION_SAVE,
                                                  "_Cancel", GTK_RESPONSE_CANCEL,
                                                  "_Save", GTK_RESPONSE_ACCEPT,
                                                  NULL);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::ofstream outfile(filename);
        for (auto const& [date, note] : events_db) {
            outfile << "--- " << date << " ---" << std::endl;
            outfile << note << std::endl << std::endl;
        }
        outfile.close();
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void on_import_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Import Notes (Raw Format)",
                                                  GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
                                                  GTK_FILE_CHOOSER_ACTION_OPEN,
                                                  "_Cancel", GTK_RESPONSE_CANCEL,
                                                  "_Open", GTK_RESPONSE_ACCEPT,
                                                  NULL);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        // For simplicity, we import from the same format we save in (events.db)
        std::ifstream file(filename);
        std::string line;
        while (std::getline(file, line)) {
            size_t pos = line.find('|');
            if (pos != std::string::npos) {
                std::string date = line.substr(0, pos);
                std::string note = line.substr(pos + 1);
                // Basic unescape logic (reusing or calling internal if it was shared)
                std::string unescaped;
                for (size_t i = 0; i < note.length(); ++i) {
                    if (note[i] == '\\' && i + 1 < note.length()) {
                        if (note[i+1] == 'n') { unescaped += '\n'; i++; }
                        else if (note[i+1] == '|') { unescaped += '|'; i++; }
                        else if (note[i+1] == '\\') { unescaped += '\\'; i++; }
                    } else unescaped += note[i];
                }
                events_db[date] = unescaped;
            }
        }
        file.close();
        save_events_to_disk();
        refresh_calendar_marks();
        update_ui_for_date();
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    load_events_from_disk();

    // Main Window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Calendarrr");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 400);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    // Main Vertical Layout
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    // Search Bar at the Top
    GtkWidget *search_entry = gtk_search_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Search notes...");
    gtk_box_pack_start(GTK_BOX(main_vbox), search_entry, FALSE, FALSE, 0);

    // Horizontal box to split Calendar and Notes
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(main_vbox), hbox, TRUE, TRUE, 0);

    // Left Side: Calendar and Navigation
    GtkWidget *vbox_left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox_left, FALSE, FALSE, 0);

    calendar_widget = gtk_calendar_new();
    gtk_box_pack_start(GTK_BOX(vbox_left), calendar_widget, FALSE, FALSE, 0);

    GtkWidget *today_button = gtk_button_new_with_label("Go to Today");
    gtk_box_pack_start(GTK_BOX(vbox_left), today_button, FALSE, FALSE, 0);

    // Export/Import Buttons
    GtkWidget *btn_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox_left), btn_hbox, FALSE, FALSE, 0);

    GtkWidget *export_button = gtk_button_new_with_label("Export");
    gtk_box_pack_start(GTK_BOX(btn_hbox), export_button, TRUE, TRUE, 0);

    GtkWidget *import_button = gtk_button_new_with_label("Import");
    gtk_box_pack_start(GTK_BOX(btn_hbox), import_button, TRUE, TRUE, 0);

    // Right Side: Notes Area
    GtkWidget *vbox_notes = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox_notes, TRUE, TRUE, 0);

    date_label = gtk_label_new("Notes:");
    gtk_label_set_xalign(GTK_LABEL(date_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox_notes), date_label, FALSE, FALSE, 0);

    // Scrolled Window for Text View
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), 
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox_notes), scrolled_window, TRUE, TRUE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    // Action Buttons for Notes
    GtkWidget *actions_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox_notes), actions_hbox, FALSE, FALSE, 0);

    GtkWidget *save_button = gtk_button_new_with_label("Save Note");
    gtk_box_pack_start(GTK_BOX(actions_hbox), save_button, TRUE, TRUE, 0);

    GtkWidget *delete_button = gtk_button_new_with_label("Delete Note");
    gtk_box_pack_start(GTK_BOX(actions_hbox), delete_button, TRUE, TRUE, 0);

    // Signals
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(calendar_widget, "day-selected", G_CALLBACK(on_day_selected), NULL);
    g_signal_connect(calendar_widget, "month-changed", G_CALLBACK(on_month_changed), NULL);
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), NULL);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_clicked), NULL);
    g_signal_connect(today_button, "clicked", G_CALLBACK(on_today_clicked), NULL);
    g_signal_connect(search_entry, "search-changed", G_CALLBACK(on_search_changed), NULL);
    g_signal_connect(export_button, "clicked", G_CALLBACK(on_export_clicked), NULL);
    g_signal_connect(import_button, "clicked", G_CALLBACK(on_import_clicked), NULL);

    // Initial UI update
    update_ui_for_date();
    refresh_calendar_marks();

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
