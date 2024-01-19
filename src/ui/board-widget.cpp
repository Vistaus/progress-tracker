#include "board-widget.h"

#include <format>
#include <iostream>

/**
 * TODO: High memory is allocated in setting background, mainly when the
 * background image is high. Should I try to compress it?
 */
ui::BoardWidget::BoardWidget()
    : Gtk::ScrolledWindow{},
      root{Gtk::Orientation::HORIZONTAL},
      add_button{"Add List"},
      cardlist_vector{},
      board{nullptr} {
    set_child(root);
    set_name("board-root");

    root.set_spacing(25);
    root.set_margin(10);

    css_provider_refptr = Gtk::CssProvider::create();
    Gtk::StyleProvider::add_provider_for_display(
        get_display(), css_provider_refptr, GTK_STYLE_PROVIDER_PRIORITY_USER);
    css_provider_refptr->load_from_data(CSS_FORMAT);

    add_button.signal_clicked().connect([this]() {
        add_cardlist(board->add_cardlist(CardList{"New CardList"}));
    });
    add_button.set_halign(Gtk::Align::START);
    add_button.set_valign(Gtk::Align::START);
    add_button.set_hexpand();

    root.append(add_button);
}

ui::BoardWidget::~BoardWidget() {}

void ui::BoardWidget::set(Board* board) {
    if (!board) return;

    clear();
    this->board = board;

    // Code updating cardlists
    for (auto& cardlist : board->get_cardlists()) {
        add_cardlist(cardlist);
    }
    set_background();
}

void ui::BoardWidget::clear() {
    if (cardlist_vector.empty()) return;

    for (auto cardlist_widget : cardlist_vector) {
        root.remove(*cardlist_widget);
    }
    cardlist_vector.clear();
}

bool ui::BoardWidget::save() {
    if (board) return board->save_as_xml();
    return false;
}

void ui::BoardWidget::add_cardlist(std::shared_ptr<CardList> cardlist_refptr) {
    auto new_cardlist = Gtk::make_managed<ui::Cardlist>(*this, cardlist_refptr);
    cardlist_vector.push_back(new_cardlist);

    auto dnd_source_controller = Gtk::DragSource::create();
    Glib::Value<ui::Cardlist*> value_cardlistptr;
    value_cardlistptr.init(Glib::Value<ui::Cardlist*>::value_type());
    value_cardlistptr.set(new_cardlist);
    auto content_provider = Gdk::ContentProvider::create(value_cardlistptr);
    dnd_source_controller->set_actions(Gdk::DragAction::MOVE);
    dnd_source_controller->set_content(content_provider);
    new_cardlist->get_header().add_controller(dnd_source_controller);

    auto dnd_target_controller = Gtk::DropTarget::create(
        Glib::Value<ui::Cardlist*>::value_type(), Gdk::DragAction::MOVE);
    dnd_target_controller->signal_drop().connect(
        [this, new_cardlist](const Glib::ValueBase& value, double x, double y) {
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<ui::Cardlist*>::value_type())) {
                Glib::Value<ui::Cardlist*> dropped_value;
                dropped_value.init(value.gobj());

                ui::Cardlist* dropped_cardlist = dropped_value.get();
                root.reorder_child_after(*dropped_cardlist, *new_cardlist);
                board->reorder_cardlist(dropped_cardlist->get_cardlist_refptr(),
                                        new_cardlist->get_cardlist_refptr());
                return true;
            }
            return false;
        },
        false);
    new_cardlist->get_header().add_controller(dnd_target_controller);

    // The new cardlist should be appended before the add_button
    root.append(*new_cardlist);
    root.reorder_child_after(add_button, *new_cardlist);
}

bool ui::BoardWidget::set_background() {
    if (!board) return false;

    if (board->get_background_type() == "colour") {
        css_provider_refptr->load_from_data(
            std::format(CSS_FORMAT_RGB, board->get_background()));
        std::cout << "Colour background set" << std::endl;
        return true;
    } else if (board->get_background_type() == "file") {
        css_provider_refptr->load_from_data(
            std::format(CSS_FORMAT_FILE, board->get_background()));
        std::cout << "File background set" << std::endl;
        return true;
    }

    return false;
}

bool ui::BoardWidget::remove_cardlist(ui::Cardlist& cardlist) {
    root.remove(cardlist);
    std::erase(cardlist_vector, &cardlist);
    board->remove_cardlist(*(cardlist.get_cardlist_refptr()));
    return true;
}