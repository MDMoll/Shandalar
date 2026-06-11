package gui;

import java.util.Objects;

/**
 * The same as the gameCard but for cards from the duel deck.
 *
 * @author Ryan Russell
 * @see gameCard
 */
public class duelCard {

    private static final int DEFAULT_ID = 0;
    private static final int DEFAULT_QUANTITY = 0;

    private int id;
    private int quantity;
    private String name;

    /**
     * Empty constructor initializes the variables.
     */
    public duelCard() {
        this(DEFAULT_ID, DEFAULT_QUANTITY, "");
    }

    /**
     * Three argument constructor sets id, quantity and name.
     *
     * @param id The duel card id.
     * @param quantity The number of copies of this card.
     * @param name The card name.
     */
    public duelCard(int id, int quantity, String name) {
        this.id = id;
        setQuantity(quantity);
        setName(name);
    }

    /**
     * Gets the duel card id.
     *
     * @return The duel card id.
     */
    public int getID() {
        return id;
    }

    /**
     * Sets the duel card id.
     *
     * @param id The duel card id.
     */
    public void setID(int id) {
        this.id = id;
    }

    /**
     * Gets the quantity of this card.
     *
     * @return The number of copies of this card.
     */
    public int getQuantity() {
        return quantity;
    }

    /**
     * Sets the quantity of this card.
     *
     * @param quantity The number of copies of this card.
     */
    public void setQuantity(int quantity) {
        if (quantity < 0) {
            throw new IllegalArgumentException("Quantity cannot be negative: " + quantity);
        }
        this.quantity = quantity;
    }

    /**
     * Gets the card name.
     *
     * @return The card name.
     */
    public String getName() {
        return name;
    }

    /**
     * Sets the card name.
     *
     * @param name The card name.
     */
    public void setName(String name) {
        this.name = Objects.requireNonNullElse(name, "");
    }

    /**
     * This method is used to convert a duel card into a game card.
     * We do so by taking a map of how the two cards are joined and
     * getting the game code that matches this duel id.
     *
     * @param cardMap The mapping of card names to card codes.
     * @return gameCard A game card representation of the duel card, or null if no mapping exists.
     */
    public gameCard convert(allCards cardMap) {
        Objects.requireNonNull(cardMap, "cardMap");

        String newID = cardMap.duelToGame(id);
        if (newID == null) {
            return null;
        }

        gameCard outCard = new gameCard();
        outCard.setName(name);
        outCard.setID(newID);
        outCard.setQuantity(1);
        return outCard;
    }
}
