package gui;

import java.util.Locale;
import java.util.Objects;

/**
 * Used to store both the Duel and Shandalar code for a card.
 * The Duel code is stored as an int since it is stored as one in game.
 * The Game code is stored as a String since it is a 2 byte hexadecimal number.
 *
 * @author Ryan Russell
 */
public final class codePair {

    private static final int DEFAULT_DUEL_CODE = 0;
    private static final String DEFAULT_GAME_CODE = "0000";

    private int duel;
    private String game;

    /**
     * Empty constructor creates a default code pair.
     */
    public codePair() {
        this(DEFAULT_DUEL_CODE, DEFAULT_GAME_CODE);
    }

    /**
     * Creates a codePair object and sets the fields to the arguments.
     *
     * @param duelCode An int representing the duel card code.
     * @param gameCode A String representing the game card code.
     */
    public codePair(int duelCode, String gameCode) {
        this.duel = duelCode;
        this.game = normalizeGameCode(gameCode);
    }

    /**
     * Returns a string representation of the codePair.
     *
     * @return String The values held in the fields.
     */
    @Override
    public String toString() {
        return "(" + duel + ", " + game + ")";
    }

    /**
     * Gets the duel code int in the pair.
     *
     * @return Returns the duel int.
     */
    public int getDuelCode() {
        return duel;
    }

    /**
     * Sets the duel code int in the pair.
     *
     * @param duelCode The new duel code int.
     */
    public void setDuelCode(int duelCode) {
        this.duel = duelCode;
    }

    /**
     * Gets the game code string in the pair.
     *
     * @return Returns the game string.
     */
    public String getGameCode() {
        return game;
    }

    /**
     * Sets the game code string in the pair.
     *
     * @param gameCode The new game code string.
     */
    public void setGameCode(String gameCode) {
        this.game = normalizeGameCode(gameCode);
    }

    /**
     * Legacy accessor retained for older callers.
     *
     * @return Returns the duel int.
     */
    public int getduelCode() {
        return getDuelCode();
    }

    /**
     * Legacy mutator retained for older callers.
     *
     * @param duelCode The new duel code int.
     */
    public void setduelCode(int duelCode) {
        setDuelCode(duelCode);
    }

    /**
     * Legacy accessor retained for older callers.
     *
     * @return Returns the game string.
     */
    public String getgameCode() {
        return getGameCode();
    }

    /**
     * Legacy mutator retained for older callers.
     *
     * @param gameCode The new game code string.
     */
    public void setgameCode(String gameCode) {
        setGameCode(gameCode);
    }

    private static String normalizeGameCode(String gameCode) {
        return Objects.requireNonNullElse(gameCode, DEFAULT_GAME_CODE)
                .trim()
                .toUpperCase(Locale.ROOT);
    }
}
