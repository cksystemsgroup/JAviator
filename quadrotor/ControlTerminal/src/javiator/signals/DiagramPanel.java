/**
 * @author Daniel Iercan, daniel.iercan@gmail.com
 *  
 */

package javiator.signals;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.geom.GeneralPath;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;

import javax.imageio.ImageIO;
import javax.imageio.ImageWriter;
import javax.imageio.stream.ImageOutputStream;
import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;

public final class DiagramPanel extends JPanel implements
		PropertyChangeListener {

	/**
     * 
     */
	private static final long serialVersionUID = 1L;

	private final SignalModel signal[];

	private final Diagram diagram;

	private final JLabel lblTitle;

	private final JButton btnSave;

	private final Color foreColor[];

	private final boolean showIndicators;

	private boolean enabled;

	private final DiagramDialog diagramDialog;

	private final boolean fancy;

	private boolean isImage;

	private int drawSamples = 1000; // -1 means none

	public DiagramPanel(SignalModel pSignale, Color pColor) {
		this(pSignale, pColor, false, false, null);
	}

	public DiagramPanel(SignalModel pSignale, Color pColor,
			boolean pShowIndicators, boolean pFancy,
			DiagramDialog pDiagramDialog) {
		signal = new SignalModel[2];
		foreColor = new Color[2];
		signal[0] = pSignale;
		signal[0].addPropertyChangeListener(SignalModel.PROPERTY_VALUE_ADDED,
				this);

		isImage = false;

		enabled = true;

		fancy = pFancy;

		diagramDialog = pDiagramDialog;

		foreColor[0] = pColor;

		showIndicators = pShowIndicators;

		setLayout(new BorderLayout(5, 5));

		setBorder(BorderFactory.createRaisedBevelBorder());

		diagram = new Diagram();

		lblTitle = new JLabel(signal[0].getName() + signal[0].getUnit(),
				JLabel.CENTER);
		lblTitle.setFont(new Font(lblTitle.getFont().getName(), Font.BOLD,
				lblTitle.getFont().getSize()));

		btnSave = new JButton("Save");
		btnSave.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				saveToFile(signal[0].getName() + "-fancy-" + ".jpg");
			}

		});
		final JPanel titlePane = new JPanel();
		titlePane.setBorder(BorderFactory.createRaisedBevelBorder());
		titlePane.add(lblTitle);
		if (fancy) {
			titlePane.add(btnSave);
		}

		add(titlePane, BorderLayout.NORTH);
		add(diagram, BorderLayout.CENTER);
	}

	public void addSignals(SignalModel pSignale, Color pColor) {
		signal[1] = pSignale;
		signal[1].addPropertyChangeListener(SignalModel.PROPERTY_VALUE_ADDED,
				this);
		foreColor[1] = pColor;
	}

	public void setAuxLines(double pLines[][]) {
		diagram.repaint();
	}

	public boolean isEnabled() {
		return enabled;
	}

	public void setEnabled(boolean pEnabled) {
		enabled = pEnabled;
	}

	public boolean isShowIndicatorsEnabled() {
		return showIndicators;
	}

	private void doubleClick() {
		if (diagramDialog != null)
			diagramDialog.setVisible(true);
	}

	private class Diagram extends JPanel {

		public Diagram() {

			addMouseListener(new MouseAdapter() {
				public void mouseClicked(java.awt.event.MouseEvent arg0) {
					if (arg0.getClickCount() == 2 && enabled)
						doubleClick();
				}
			});
		}

		/**
         * 
         */
		private static final long serialVersionUID = 1L;

		public void paint(Graphics g) {
			// super.paint(g);
			if (signal == null)
				return;
			Graphics2D g2d = (Graphics2D) g;

			final double h = isImage ? g2d.getDeviceConfiguration().getBounds()
					.getHeight() : getHeight();
			final int w = isImage ? (int) g2d.getDeviceConfiguration()
					.getBounds().getWidth() : getWidth();
			double increment = 1;
			int start = 0;

			final Rectangle2D.Double bounds = new Rectangle2D.Double(0, 0, w, h);

			g2d.setPaint(Color.WHITE);// getBackground());
			g2d.fill(bounds);

			int pointsCount = signal[0].getSize();
			int offset = 0;
			int signalSize = signal[0].getSize();
			if (drawSamples > 0) {
				pointsCount = drawSamples;
				if (signalSize > pointsCount)
					offset = signalSize - pointsCount;
			}

			double max = signal[0].getMaxValue();
			double min = signal[0].getMinValue();

			if (signal[1] != null) {
				if (max < signal[1].getMaxValue())
					max = signal[1].getMaxValue();

				if (min > signal[1].getMinValue())
					min = signal[1].getMinValue();
			}
			
			double unit = (max - min) / h;
			if (unit == 0)
				unit = 1;
			double y;
			double x;
			double length;

			if (pointsCount == 0)
				return;

			if (pointsCount <= w) {
				increment = w / pointsCount;
				length = pointsCount;
			} else {
				start = 0;
				increment = pointsCount / (double) (w + 1);
				length = pointsCount / (double) increment;
			}

			for (int j = 0; j < 2; ++j) {
				final GeneralPath path = new GeneralPath();
				if (signal[j] == null)
					continue;
				for (int i = start; i < length; i++) {

					if (pointsCount > w) {
						if (signalSize < i * increment + offset)
							break;
						x = i - start;
						y = h
								- (signal[j]
										.get((int) (i * increment + offset)) - min)
								/ unit;
					} else {
						if (signalSize < i + offset)
							break;

						x = (i - start) * increment;
						y = h - (signal[j].get(i + offset) - min) / unit;
					}
					// System.out.print("("+x+";"+y+")");
					if (y < 1.0) {
						y = 1.0;
					}
					if (i == start)
						path.moveTo((float) x, (float) y);
					else {
						path.lineTo((float) x, (float) y);
					}
				}
				// draw the grahpic
				g2d.setColor(foreColor[j]);
				g2d.draw(path);

			}

		}

	}

	public void saveToFile(String pFileName) {
		BufferedImage buff = new BufferedImage(800, 600,
				BufferedImage.TYPE_3BYTE_BGR);
		isImage = true;
		diagram.paint(buff.getGraphics());
		isImage = false;
		try {
			Iterator<ImageWriter> writers = ImageIO.getImageWritersByFormatName("jpg");
			ImageWriter writer = (ImageWriter) writers.next();

			File f = new File(pFileName);
			ImageOutputStream ios = ImageIO.createImageOutputStream(f);
			writer.setOutput(ios);

			writer.write(buff);

		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @seejava.beans.PropertyChangeListener#propertyChange(java.beans.
	 * PropertyChangeEvent)
	 */
	public void propertyChange(PropertyChangeEvent arg0) {
		if (arg0.getPropertyName().compareTo(SignalModel.PROPERTY_VALUE_ADDED) == 0
				&& enabled) {
			diagram.repaint();
		}

	}

	public ImageIcon getIcon(String iconFile) {
		InputStream pngStream = getClass().getResourceAsStream(iconFile);
		if (pngStream == null) {
			System.out.println("Image " + iconFile + ", not found.");
			return null;
		}

		ImageIcon icon = null;
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		try {

			int c;
			while ((c = pngStream.read()) >= 0)
				baos.write(c);

			icon = new ImageIcon(getToolkit().createImage(baos.toByteArray()));

		} catch (IOException e) {

			e.printStackTrace();
		}
		return icon;
	}
}
