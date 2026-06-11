package gui;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;

/**
 * A store for game cards.
 *
 * <p>This class keeps the legacy ArrayList shape used by the rest of the GUI,
 * but provides card-aware helpers for Shandalar deck operations.</p>
 *
 * @author Ryan
 */
public class gameDeck extends ArrayList<gameCard> {

    private static final String LINE_SEPARATOR = System.lineSeparator();

    /**
     * Returns the normal, non-debug representation of this deck.
     *
     * @return String representation of this deck.
     */
    @Override
    public String toString() {
        return toString(false);
    }

    /**
     * A String representation of the values contained by the fields
     * of this object. If we are in debug mode we return more information.
     *
     * @param debug Boolean indicating if we should display debugging information
     * @return String representation of this deck.
     */
    public String toString(boolean debug) {
        if (isEmpty()) {
            return "Deck Empty!";
        }

        StringBuilder out = new StringBuilder();
        for (gameCard card : this) {
            out.append(card.getQuantity()).append(' ');
            if (debug) {
                out.append(card.getID()).append(' ')
                        .append(card.getDeck()).append(' ');
            }
            out.append(card.getName()).append(LINE_SEPARATOR);
        }
        return out.toString();
    }

    /**
     * Returns the amount of cards in this deck. This is different from size()
     * because each entity in the list also contains a quantity.
     *
     * @return int The amount of cards contained by this deck.
     */
    public int amountOfCards() {
        return stream()
                .mapToInt(gameCard::getQuantity)
                .sum();
    }

    /**
     * When we first retrieve the game deck from Shandalar we have a deck made up
     * of lots of individual cards. This method groups all matching cards together,
     * making it the same as the format used by Duel decks.
     *
     * <p>This method preserves the first encountered card instance for each name,
     * then updates its quantity to the total quantity found.</p>
     *
     * @return A new grouped game deck.
     */
    public gameDeck group() {
        try {
            Map<String, gameCard> groupedCards = new LinkedHashMap<>();
            for (gameCard card : this) {
                gameCard groupedCard = groupedCards.get(card.getName());
                if (groupedCard == null) {
                    groupedCard = card.clone();
                    groupedCard.setQuantity(card.getQuantity());
                    groupedCards.put(groupedCard.getName(), groupedCard);
                } else {
                    groupedCard.setQuantity(groupedCard.getQuantity() + card.getQuantity());
                }
            }

            gameDeck groupedDeck = new gameDeck();
            groupedDeck.addAll(groupedCards.values());
            return groupedDeck;
        } catch (Exception e) {
            throw new IllegalStateException("Error while grouping cards in gameDeck.", e);
        }
    }

    /**
     * Creates a new deck containing only the cards that are stored in the given
     * deck number.
     *
     * @param number The deck number of the stored cards to return.
     * @return A new deck containing cards assigned to the requested deck number.
     */
    public gameDeck getSubDeck(int number) {
        int deckNumber = normalizeDeckNumber(number);
        gameDeck tempDeck = new gameDeck();
        for (gameCard card : this) {
            int deckMask = deckMaskOf(card);
            // bit comparison for deck bitflags
            if ((deckMask & deckNumber) == deckNumber) {
                tempDeck.add(card);
            }
        }
        return tempDeck;
    }

    /**
     * Converts this game deck into a duel deck as used in Duel.
     * This is done by calling the convert method for each card in the
     * deck and adding them to a duelDeck object which is then returned.
     *
     * <p>We lose the deck information of each card by doing this because it isn't
     * stored in the duel deck format.</p>
     *
     * @param cardMap The mapping of the cards' name to the cards' ids.
     * @return duelDeck
     */
    public duelDeck convert(allCards cardMap) {
        Objects.requireNonNull(cardMap, "cardMap");

        duelDeck outDeck = new duelDeck();
        for (gameCard card : this) {
            duelCard convertedCard = card.convert(cardMap);
            if (convertedCard != null) {
                outDeck.add(convertedCard);
            }
        }
        return outDeck;
    }

    /**
     * Creates the given game deck in this game deck using the following rules:
     * Any cards that are needed and are already in the game deck are used.
     * Any cards that are needed and are not already in the game deck are added to it then
     * used.
     * Cards that make up part of another deck have their other commitments preserved.
     * Any cards that aren't needed by any deck are kept in hand.
     *
     * @param deckNumber The slot to inject this deck into.
     * @param newSubDeck The game deck to create in this game deck.
     * @return A new game deck with the requested deck slot updated.
     */
    public gameDeck setDeck(int deckNumber, gameDeck newSubDeck) {
        Objects.requireNonNull(newSubDeck, "newSubDeck");

        int normalizedDeckNumber = normalizeDeckNumber(deckNumber);
        gameDeck newGameDeck = new gameDeck();

        // We need to use iterators here as we are manipulating the lists as we travel.
        for (Iterator<gameCard> heldIterator = iterator(); heldIterator.hasNext();) {
            gameCard heldCard = heldIterator.next();
            boolean found = false;

            for (Iterator<gameCard> neededIterator = newSubDeck.iterator(); neededIterator.hasNext();) {
                gameCard neededCard = neededIterator.next();
                if (Objects.equals(heldCard.getID(), neededCard.getID())) {
                    heldCard.setDeck(normalizedDeckNumber, true);
                    newGameDeck.add(heldCard);
                    heldIterator.remove();
                    neededIterator.remove();
                    found = true;
                    break;
                }
            }

            // We've been around all the cards we need and this one isn't needed.
            if (!found) {
                heldCard.unSetDeck(normalizedDeckNumber);
                newGameDeck.add(heldCard);
                heldIterator.remove();
            }
        }

        // At this point we've used all the held cards we could, but we might still
        // need more cards, so add them in.
        for (gameCard neededCard : newSubDeck) {
            neededCard.setDeck(normalizedDeckNumber, true);
            newGameDeck.add(neededCard);
        }
        return newGameDeck;
    }

    /**
     * Checks if this deck contains the given card based solely on ID.
     *
     * @param gameCard The card to check if this deck contains.
     * @return true if this deck contains a card with the same ID.
     */
    public boolean contains(gameCard gameCard) {
        return containsById(gameCard);
    }

    /**
     * Checks if this deck contains the given card based solely on ID.
     *
     * @param candidate The object to check if this deck contains.
     * @return true if this deck contains a card with the same ID.
     */
    @Override
    public boolean contains(Object candidate) {
        return candidate instanceof gameCard gameCard && containsById(gameCard);
    }

    /**
     * Returns the first instance of the given card from the deck, matching by ID.
     *
     * @param gameCard The card to return.
     * @return The matching card, or null if no matching card exists.
     */
    public gameCard get(gameCard gameCard) {
        if (gameCard == null) {
            return null;
        }

        for (gameCard card : this) {
            if (Objects.equals(card.getID(), gameCard.getID())) {
                return card;
            }
        }
        return null;
    }

    private boolean containsById(gameCard gameCard) {
        return get(gameCard) != null;
    }

    private static int normalizeDeckNumber(int deckNumber) {
        return deckNumber == 3 ? 4 : deckNumber;
    }

    private static int deckMaskOf(gameCard card) {
        String deck = Objects.requireNonNull(card.getDeck(), "card.deck");
        if (deck.length() < 2) {
            throw new IllegalArgumentException("Invalid deck code for " + card.getName() + ": " + deck);
        }
        return Integer.parseInt(deck.substring(0, 2), 16);
    }
}