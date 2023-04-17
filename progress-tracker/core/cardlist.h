#pragma once

#include <vector>

#include "card.h"
#include "item.h"

/**
 * @class CardList
 *
 * @brief A class representing a single list of cards inside a board.
 */
class CardList : public Item {
   public:
    /**
     * @brief CardList constructor;
     */
    CardList(std::string name);

    /**
     * @brief Adds a \ref Card object to the cardlist.
     *
     * @returns True if the card was added successfully, otherwise false.
     */
    bool add_card(Card& card);

    /**
     * @brief Removes a \ref Card object from the cardlist.
     *
     * @param card \ref Card instance.
     *
     * @returns True if the card was successfully removed from the cardlist.
     *          False may be returned when the requested Card to be removed is
     *          not present in the cardlist.
     */
    bool remove_card(Card& card);

    /**
     * @brief Creates a pointer to a constant vector object.
     *
     * @param card \ref Card instance.
     *
     * @returns the pointer pointing to a constant vector object.
     */
    const std::vector<Card>* get_card_vector() const;

   private:
    std::vector<Card> card_vector;
};