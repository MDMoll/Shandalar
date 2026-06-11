package gui;

import java.util.Locale;
import java.util.Objects;

public class gameCard {

    private static final String NO_DECK = "00";
    private static final String DEFAULT_ID = "0000";
    private static final int DEFAULT_QUANTITY = 1;
    private static final int GAME_CODE_LENGTH = 4;
    private static final int DECK_CODE_LENGTH = 2;

    /**
     * When read from the game this contains 8 chars: first 4 for id, then 4 for deck.
     */
    private String id;

    /**
     * No Deck = 00, Deck 1 = 01, Deck 2 = 02, Deck 3 = 04.
     * Add them together because the deck value is stored as bit flags.
     */
    private String deck;

    private String name;
    private int quantity;

    /**
     * Empty constructor initializes the variables.
     */
    public gameCard() {
        this(DEFAULT_ID, "", NO_DECK);
    }

    /**
     * Two String constructor sets the card's id and name to the given
     * arguments. Quantity is set to 1 as cards in the actual game file
     * don't store quantity, we do so we know how many times to loop writing
     * it to the file.
     *
     * @param id The hexadecimal game code of the card.
     * @param name The name of the card.
     */
    public gameCard(String id, String name) {
        this(id, name, NO_DECK);
    }

    /**
     * Three String constructor sets the card's id, name and deck to the given
     * arguments. Quantity is set to 1 as cards in the actual game file
     * don't store quantity, we do so we know how many times to loop writing
     * it to the file.
     *
     * @param id The hexadecimal game code of the card.
     * @param name The name of the card.
     * @param deck Hexadecimal number of the decks this card is in.
     */
    public gameCard(String id, String name, String deck) {
        this.id = normalizeId(id);
        this.name = Objects.requireNonNullElse(name, "");
        this.deck = normalizeDeckCode(deck);
        this.quantity = DEFAULT_QUANTITY;
    }

    /**
     * Get the game card's id.
     *
     * @return String Hexadecimal representation of the card's game code.
     */
    public String getID() {
        return id;
    }

    /**
     * Set the game card's id.
     *
     * @param id String Hexadecimal representation of the card's game code.
     */
    public void setID(String id) {
        this.id = normalizeId(id);
    }

    /**
     * Gets the game card's name.
     *
     * @return String The name of the card.
     */
    public String getName() {
        return name;
    }

    /**
     * Sets the game card's name.
     *
     * @param name String The name of the card.
     */
    public void setName(String name) {
        this.name = Objects.requireNonNullElse(name, "");
    }

    /**
     * Gets the game card's deck.
     *
     * @return String The hexadecimal representation of the decks the card is in.
     */
    public String getDeck() {
        return deck;
    }

    /**
     * Sets the game card's deck.
     *
     * @param deck String The hexadecimal representation of the decks the card is in.
     */
    public void setDeck(String deck) {
        this.deck = normalizeDeckCode(deck);
    }

    /**
     * Sets the deck the card is a part of using an int for the deck number (0, 1, 2 or 3)
     * and a boolean to indicate whether or not the card should be kept a part of
     * the other decks it may be used in.
     *
     * @param deck An int for the actual deck number.
     * @param keepCommitments Boolean indicating if the card should be left in the other
     * decks it is a part of.
     */
    public void setDeck(int deck, boolean keepCommitments) {
        int deckMask = normalizeDeckMask(deck);
        if (!keepCommitments) {
            this.deck = formatDeckMask(deckMask);
            return;
        }

        int currentDeckMask = parseDeckMask(this.deck);
        this.deck = formatDeckMask(currentDeckMask | deckMask);
    }

    /**
     * Removes the card from the given deck.
     *
     * @param deck The deck to remove the card from.
     */
    public void unSetDeck(int deck) {
        int currentDeckMask = parseDeckMask(this.deck);
        int deckMask = normalizeDeckMask(deck);
        this.deck = formatDeckMask(currentDeckMask & ~deckMask);
    }

    /**
     * Gets the game card's quantity. The quantity is used to mark how many times the system
     * should loop round writing the card data to the save.
     *
     * @return int The number of times this card appears in the deck.
     */
    public int getQuantity() {
        return quantity;
    }

    /**
     * Sets the game card's quantity. The quantity is used to mark how many times the system
     * should loop round writing the card data to the save.
     *
     * @param quantity int The number of times this card appears in the deck.
     */
    public void setQuantity(int quantity) {
        if (quantity < 0) {
            throw new IllegalArgumentException("Quantity cannot be negative: " + quantity);
        }
        this.quantity = quantity;
    }

    /**
     * This method is used to convert a game card into a duel card.
     * We do so by taking a map of how the two cards are joined and
     * then getting the codePair that matches the name of the card.
     * From this we can get the new code and then the quantity is
     * just copied across.
     *
     * @param cardMap The mapping of card names to card codes.
     * @return duelCard A duel card representation of the game card, or null if no mapping exists.
     */
    public duelCard convert(allCards cardMap) {
        Objects.requireNonNull(cardMap, "cardMap");

        int newID = cardMap.gameToDuel(gameCode());
        if (newID == -1) {
            return null;
        }

        duelCard outCard = new duelCard();
        outCard.setName(name);
        outCard.setID(newID);
        outCard.setQuantity(quantity);
        return outCard;
    }

    /**
     * Creates a duplicate of this card.
     *
     * @return A duplicate of this card.
     */
    @Override
    public gameCard clone() {
        gameCard gameCard = new gameCard(id, name, deck);
        gameCard.setQuantity(quantity);
        return gameCard;
    }

    /**
     * There is a problem when reading cards that depending on the deck the card
     * is in the code of the card changes. This method eliminates that problem by
     * zeroing out the number that changes.
     *
     * @return The card we just fixed.
     */
    public gameCard fix() {
        if (id.length() < GAME_CODE_LENGTH) {
            throw new IllegalStateException("Cannot fix game card id shorter than 4 characters: " + id);
        }
        id = id.substring(0, 2) + "0" + id.substring(3);
        return this;
    }

    private String gameCode() {
        if (id.length() < GAME_CODE_LENGTH) {
            throw new IllegalStateException("Game card id must be at least 4 characters: " + id);
        }
        return id.substring(0, GAME_CODE_LENGTH);
    }

    private static String normalizeId(String id) {
        return Objects.requireNonNullElse(id, DEFAULT_ID).trim().toUpperCase(Locale.ROOT);
    }

    private static String normalizeDeckCode(String deck) {
        String normalized = Objects.requireNonNullElse(deck, NO_DECK).trim().toUpperCase(Locale.ROOT);
        if (normalized.length() < DECK_CODE_LENGTH) {
            throw new IllegalArgumentException("Deck code must be at least 2 characters: " + deck);
        }
        return normalized;
    }

    private static int normalizeDeckMask(int deck) {
        return deck == 3 ? 4 : deck;
    }

    private static int parseDeckMask(String deck) {
        String normalized = normalizeDeckCode(deck);
        return Integer.parseInt(normalized.substring(0, DECK_CODE_LENGTH), 16);
    }

    private static String formatDeckMask(int deckMask) {
        return "%02X".formatted(deckMask & 0xFF);
    }
}