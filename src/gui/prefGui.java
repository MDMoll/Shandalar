package gui;

import java.awt.BorderLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.WindowConstants;

/**
 * For saving the settings to the file.
 * @author Ryan Russell
 * @since 08-Jul-2012
 */
public class prefGui extends JFrame {

    private static final Path SETTINGS_FILE = Path.of("settings.cfg");
    private static final String SHANDALAR_DIR_PREFIX = "shandDir:";
    private static final String PLAYDECK_DIR_PREFIX = "deckDir:";
    private static final int FIELD_COLUMNS = 36;
    private static final Insets DEFAULT_INSETS = new Insets(6, 6, 6, 6);

    private final JButton btnSave = new JButton("Save");
    private final JButton btnCancel = new JButton("Cancel");
    private final JButton btnBrowseShandDir = new JButton("Browse...");
    private final JButton btnBrowsePlayDir = new JButton("Browse...");
    private final JLabel lblShandDir = new JLabel("Shandalar directory:");
    private final JLabel lblPlayDir = new JLabel("Playdeck directory:");
    private final JTextField txtShandDir = new JTextField(FIELD_COLUMNS);
    private final JTextField txtPlayDir = new JTextField(FIELD_COLUMNS);

    /** Creates new form prefGui */
    public prefGui(String[] dirs) {
        initComponents();
        setInitialDirectories(dirs);
    }

    private void initComponents() {
        setTitle("Preferences");
        setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);

        lblShandDir.setLabelFor(txtShandDir);
        lblPlayDir.setLabelFor(txtPlayDir);

        txtShandDir.setText("C:\\Games\\Shandalar");
        txtPlayDir.setText("C:\\Games\\Shandalar\\Playdeck");

        btnSave.addActionListener(this::btnSaveActionPerformed);
        btnCancel.addActionListener(event -> dispose());
        btnBrowseShandDir.addActionListener(event -> chooseDirectory(txtShandDir, "Choose Shandalar directory"));
        btnBrowsePlayDir.addActionListener(event -> chooseDirectory(txtPlayDir, "Choose Playdeck directory"));

        getRootPane().setDefaultButton(btnSave);

        JPanel formPanel = new JPanel(new GridBagLayout());
        formPanel.add(lblShandDir, constraints(0, 0, 0.0, GridBagConstraints.NONE));
        formPanel.add(txtShandDir, constraints(1, 0, 1.0, GridBagConstraints.HORIZONTAL));
        formPanel.add(btnBrowseShandDir, constraints(2, 0, 0.0, GridBagConstraints.NONE));
        formPanel.add(lblPlayDir, constraints(0, 1, 0.0, GridBagConstraints.NONE));
        formPanel.add(txtPlayDir, constraints(1, 1, 1.0, GridBagConstraints.HORIZONTAL));
        formPanel.add(btnBrowsePlayDir, constraints(2, 1, 0.0, GridBagConstraints.NONE));

        JPanel buttonPanel = new JPanel();
        buttonPanel.add(btnSave);
        buttonPanel.add(btnCancel);

        JPanel contentPanel = new JPanel(new BorderLayout(6, 6));
        contentPanel.add(formPanel, BorderLayout.CENTER);
        contentPanel.add(buttonPanel, BorderLayout.SOUTH);
        setContentPane(contentPanel);

        pack();
        setLocationRelativeTo(getOwner());
    }

    private static GridBagConstraints constraints(int x, int y, double weightX, int fill) {
        GridBagConstraints constraints = new GridBagConstraints();
        constraints.gridx = x;
        constraints.gridy = y;
        constraints.weightx = weightX;
        constraints.fill = fill;
        constraints.anchor = GridBagConstraints.WEST;
        constraints.insets = DEFAULT_INSETS;
        return constraints;
    }

    private void btnSaveActionPerformed(ActionEvent evt) {
        try {
            savePreferences();
            dispose();
        } catch (IllegalArgumentException e) {
            showWarning(e.getMessage());
        } catch (IOException e) {
            showWarning("Error saving preferences: " + e.getMessage());
        }
    }

    private void chooseDirectory(JTextField targetField, String dialogTitle) {
        JFileChooser chooser = new JFileChooser();
        chooser.setDialogTitle(dialogTitle);
        chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        chooser.setAcceptAllFileFilterUsed(false);

        Path initialDirectory = findInitialDirectory(targetField.getText());
        if (initialDirectory != null) {
            chooser.setCurrentDirectory(initialDirectory.toFile());
        }

        if (chooser.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
            targetField.setText(chooser.getSelectedFile().getAbsolutePath());
        }
    }

    private static Path findInitialDirectory(String value) {
        if (value == null || value.isBlank()) {
            return null;
        }

        try {
            Path path = Path.of(value.trim());
            if (Files.isDirectory(path)) {
                return path;
            }
            Path parent = path.getParent();
            return parent != null && Files.isDirectory(parent) ? parent : null;
        } catch (InvalidPathException e) {
            return null;
        }
    }

    private void setInitialDirectories(String[] dirs) {
        if (dirs == null || dirs.length < 2) {
            return;
        }

        txtShandDir.setText(dirs[0]);
        txtPlayDir.setText(dirs[1]);
    }

    private void savePreferences() throws IOException {
        String gameDir = requireText(txtShandDir.getText(), "Shandalar directory");
        String playDir = requireText(txtPlayDir.getText(), "Playdeck directory");

        String contents = SHANDALAR_DIR_PREFIX + gameDir + System.lineSeparator()
                + PLAYDECK_DIR_PREFIX + playDir + System.lineSeparator();

        Files.writeString(
                SETTINGS_FILE,
                contents,
                StandardCharsets.UTF_8,
                StandardOpenOption.CREATE,
                StandardOpenOption.TRUNCATE_EXISTING,
                StandardOpenOption.WRITE
        );
    }

    private static String requireText(String value, String fieldName) {
        String trimmed = value == null ? "" : value.trim();
        if (trimmed.isEmpty()) {
            throw new IllegalArgumentException(fieldName + " is required.");
        }
        return trimmed;
    }

    private void showWarning(String message) {
        JOptionPane.showMessageDialog(this, message, "Warning", JOptionPane.WARNING_MESSAGE);
    }
}
