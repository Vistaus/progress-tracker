#ifndef WINDOW_H
#define WINDOW_H

#include <gtkmm/aboutdialog.h>
#include <gtkmm/button.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/label.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/window.h>

namespace ui {
/**
 * Progress app about dialog.
 */
class ProgressAboutDialog : public Gtk::AboutDialog {
   public:
    ProgressAboutDialog(Gtk::Window& parent);
    ~ProgressAboutDialog() override;
};

/**
 * Progress application window.
 */
class ProgressWindow : public Gtk::Window {
   public:
    ProgressWindow();
    ~ProgressWindow() override;

   private:
    Gtk::HeaderBar header_bar;
    Gtk::Button add_board_button;
    Gtk::MenuButton menu_button;
    Gtk::Label label;

    ui::ProgressAboutDialog dialog;

    void setup_menu_button();
    void show_about();
};
}  // namespace ui

#endif  // WINDOW_h