#include "cardlist.h"

#include <format>
#include <iostream>

#include "board-widget.h"
#include "card.h"

ui::CardListHeader::CardListHeader(std::shared_ptr<CardList>& cardlist_refptr)
    : EditableLabelHeader{}, cardlist_refptr{cardlist_refptr} {
    set_label(cardlist_refptr->get_name());
}

void ui::CardListHeader::set_label(std::string new_label) {
    label.set_markup(std::format(CARDLIST_TITLE_FORMAT, new_label));
}

void ui::CardListHeader::on_confirm_changes() {
    cardlist_refptr->set_name(label.get_text());
}

/**
 * TODO: Implement code that reorders cards within the Cardlist
*/
ui::Cardlist::Cardlist(BoardWidget& board,
                       std::shared_ptr<CardList> cardlist_refptr)
    : Gtk::ListBox{},
      add_card_button{"Add card"},
      root{Gtk::Orientation::VERTICAL},
      cards_tracker{},
      board{board},
      cardlist_refptr{cardlist_refptr},
      cardlist_header{cardlist_refptr} {
    add_css_class("rich-list");
    set_valign(Gtk::Align::START);
    set_vexpand(true);
    set_halign(Gtk::Align::START);
    set_size_request(CARDLIST_SIZE, CARDLIST_SIZE * 2);
    set_selection_mode(Gtk::SelectionMode::NONE);

    cardlist_header.add_option("remove", "Remove",
                               sigc::mem_fun(*this, &ui::Cardlist::remove));

    add_card_button.set_valign(Gtk::Align::CENTER);
    add_card_button.set_halign(Gtk::Align::START);
    add_card_button.set_hexpand(false);
    add_card_button.signal_clicked().connect([this]() { add_card(); });
    root.append(add_card_button);

    append(cardlist_header);
    for (auto& card : cardlist_refptr->get_card_vector()) {
        add_card(card);
    }

    root.set_size_request(CARDLIST_SIZE, CARDLIST_SIZE);
    root.set_valign(Gtk::Align::FILL);
    root.set_vexpand();
    root.set_spacing(4);
    root.set_margin_top(4);

    Gtk::ScrolledWindow scr_window{};
    scr_window.set_child(root);
    scr_window.set_size_request(CARDLIST_SIZE, CARDLIST_SIZE * 2);
    scr_window.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    append(scr_window);

    // Makes the header and the list itself non-selectable
    get_row_at_index(0)->set_activatable(false);
    get_row_at_index(1)->set_activatable(false);
}

void ui::Cardlist::remove() { board.remove_cardlist(*this); }

void ui::Cardlist::reorder_card(ui::CardWidget& next, ui::CardWidget& sibling) {
    root.reorder_child_after(next, sibling);

    // Code reordering the data models goes here
    cardlist_refptr->reorder_card(next.get_card(), sibling.get_card());
}

void ui::Cardlist::append_card(ui::CardWidget* card) {
    root.append(*card);
    root.reorder_child_after(add_card_button, *card);
    card->set_cardlist(this);

    // DragSource Settings
    auto drag_source_c = Gtk::DragSource::create();
    drag_source_c->set_actions(Gdk::DragAction::MOVE);
    Glib::Value<ui::CardWidget*> value_new_cardptr;
    value_new_cardptr.init(Glib::Value<ui::CardWidget*>::value_type());
    value_new_cardptr.set(card);
    drag_source_c->set_content(
        Gdk::ContentProvider::create(value_new_cardptr));
    card->add_controller(drag_source_c);
    
    // DropTarget Settings
    auto drop_target_c = Gtk::DropTarget::create(
        Glib::Value<ui::CardWidget*>::value_type(),
        Gdk::DragAction::MOVE);
    drop_target_c->signal_drop().connect(
        [this, card](const Glib::ValueBase& value, double x, double y) {
            if (G_VALUE_HOLDS(value.gobj(), Glib::Value<ui::CardWidget*>::value_type())) {
                Glib::Value<ui::CardWidget*> dropped_value;
                dropped_value.init(value.gobj());
                auto dropped_card = dropped_value.get();
                if (is_child(dropped_card)) {
                    reorder_card(*dropped_card, *card);
                } else {
                    // Dropped card's parent is the dropped-on card's parent
                    auto card_refptr = dropped_card->get_card();
                    cardlist_refptr->add_card(*card_refptr);
                    dropped_card->remove();
                    auto card_from_dropped = add_card(card_refptr);
                    cards_tracker.push_back(card_from_dropped);
                    reorder_card(*card_from_dropped, *card);
                }
                return true;
            }
            return false;
        }, false);
    card->add_controller(drop_target_c);
    cards_tracker.push_back(card);
}

void ui::Cardlist::remove_card(ui::CardWidget* card) {
    root.remove(*card);

    for (size_t i = 0; i < cards_tracker.size(); i++) {
        if (cards_tracker[i] == card) {
            cards_tracker.erase(cards_tracker.begin()+i);
        }
    }

    cardlist_refptr->remove_card(*card->get_card());
}

ui::CardWidget* ui::Cardlist::add_card(std::shared_ptr<Card> card) {
    auto new_card = Gtk::make_managed<ui::CardWidget>(card);
    append_card(new_card);

    return new_card;
}

ui::CardWidget* ui::Cardlist::add_card() {
    return add_card(cardlist_refptr->add_card(Card{"New Card"}));
}

std::shared_ptr<CardList>& ui::Cardlist::get_cardlist_refptr() {
    return cardlist_refptr;
}

bool ui::Cardlist::is_child(ui::CardWidget* card) {
    for (auto& card_: cards_tracker) {
        if (card == card_) return true;
    }
    return false;
}

ui::CardListHeader& ui::Cardlist::get_header() {
    return cardlist_header;
}