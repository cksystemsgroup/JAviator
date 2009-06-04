package javiator.signals;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;

import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;

/**
 * @author Dani
 *
 */
public class SignalsDialog extends JDialog {
    public static final long serialVersionUID = 1;

    private boolean closed = false;
    
    public boolean isClosed() { return closed; }
    
    public void open() { setVisible(true); z.reset(); closed = false; }
    
	//diagrams
	private DiagramPanel zDiag;
	private DiagramPanel rollDiag;
	private DiagramPanel pitchDiag;
	private DiagramPanel yawDiag;
	private DiagramPanel T1Diag;
	private DiagramPanel T2Diag;
	private DiagramPanel T3Diag;
	private DiagramPanel T4Diag;
	
//	signals
	public SignalModel front;
	public SignalModel right;
	public SignalModel rear;
	public SignalModel left;
	public SignalModel z;
	public SignalModel roll;
	public SignalModel pitch;
	public SignalModel yaw;
	public SignalModel droll;
	public SignalModel dpitch;
	public SignalModel dyaw;
	public SignalModel ddroll;
	public SignalModel ddpitch;
	public SignalModel ddyaw;
	public SignalModel ddx;
	public SignalModel ddy;
	public SignalModel ddz;
	public SignalModel vx;
	public SignalModel vy;
	public SignalModel vz;
	public SignalModel ref_roll;
	public SignalModel ref_pitch;
	public SignalModel ref_yaw;
	public SignalModel ref_z;
	
	public SignalsDialog(){
		//init signals
		front = new SignalModel("front", "[N]", 1);
		right = new SignalModel("right", "[N]", 1);
		rear = new SignalModel("rear", "[N]", 1);
		left = new SignalModel("left", "[N]", 1);
		z = new SignalModel("Altitude", "[m]", 1);
		roll = new SignalModel("Roll", "[degrees]", 1);
		pitch = new SignalModel("Pitch", "[degrees]", 1);
		yaw = new SignalModel("Yaw", "[degrees]", 1);
		droll = new SignalModel("dRoll", "[degrees/sec]", 1);
		dpitch = new SignalModel("dPitch", "[degrees/sec]", 1);
		dyaw = new SignalModel("dYaw", "[degrees/sec]", 1);
		ddroll = new SignalModel("ddRoll", "[degrees/sec^2]", 1);
		ddpitch = new SignalModel("ddPitch", "[degrees/sec^2]", 1);
		ddyaw = new SignalModel("ddYaw", "[degrees/sec^2]", 1);
		ddx = new SignalModel("ddX", "[m/sec^2]", 1);
		ddy = new SignalModel("ddY", "[m/sec^2]", 1);
		ddz = new SignalModel("ddZ", "[m/sec^2]", 1);
		vx = new SignalModel("vx", "[m/sec]", 1);
		vy = new SignalModel("vy", "[m/sec]", 1);
		vz = new SignalModel("vz", "[m/sec]", 1);
		ref_roll = new SignalModel("Ref Roll", "[degrees]", 1);
		ref_pitch = new SignalModel("Ref Pitch", "[degrees]", 1);
		ref_yaw = new SignalModel("Ref Yaw", "[degrees]", 1);
		ref_z = new SignalModel("Ref Z", "[m]", 1);
		closed = false;
		
		//diagrams panel
		JTabbedPane diagTabPanel = new JTabbedPane();
		diagTabPanel.addTab("Controlled Signals", createSignalsPanel());
		diagTabPanel.addTab("Commands", createThrustsPanel());
		diagTabPanel.addTab("Angle Rates", createAngleRatesPanel());
		diagTabPanel.addTab("Angle Acceleration", createAngleAccelerationPanel());
		diagTabPanel.addTab("Velocities", createVelocitiesPanel());
		diagTabPanel.addTab("Accelerations", createAccelerationPanel());
		diagTabPanel.addTab("Referances", createRefPanel());
		
		JPanel content = new JPanel();
		content.setLayout(new BorderLayout());
		content.add(diagTabPanel, BorderLayout.CENTER);
		setContentPane(content);
		
		addWindowListener(new WindowListener(){

			public void windowActivated(WindowEvent arg0) {
				// TODO Auto-generated method stub
				
			}

			public void windowClosed(WindowEvent arg0) {
				// TODO Auto-generated method stub
				
			}

			public void windowClosing(WindowEvent arg0) {
				// TODO Auto-generated method stub
				setVisible(false);
				closed = true;
			}

			public void windowDeactivated(WindowEvent arg0) {
				// TODO Auto-generated method stub
				
			}

			public void windowDeiconified(WindowEvent arg0) {
				// TODO Auto-generated method stub
				
			}

			public void windowIconified(WindowEvent arg0) {
				// TODO Auto-generated method stub
				
			}

			public void windowOpened(WindowEvent arg0) {
				// TODO Auto-generated method stub
				
			}
			
		});
		
		setSize(new Dimension(800, 600));
		setVisible(true);
	}
	
	private JPanel createSignalsPanel(){
		JPanel diagOutPanel = new JPanel(new GridLayout(2, 2 , 2 ,2));
		
		//new DiagramDialog()
		//create signal diagram panels
		zDiag = new DiagramPanel(z, Color.BLUE);//, true, true, new DiagramDialog(this, z, true, Color.BLUE));
		zDiag.addSignals(ref_z, Color.RED);
		rollDiag = new DiagramPanel(roll, Color.BLUE);
		rollDiag.addSignals(ref_roll, Color.RED);
		pitchDiag = new DiagramPanel(pitch, Color.BLUE);
		pitchDiag.addSignals(ref_pitch, Color.RED);
		yawDiag = new DiagramPanel(yaw, Color.BLUE);
		yawDiag.addSignals(ref_yaw, Color.RED);
		
		diagOutPanel.add(rollDiag);
		diagOutPanel.add(pitchDiag);
		diagOutPanel.add(yawDiag);
		diagOutPanel.add(zDiag);
		return diagOutPanel;
	}
	
	private JPanel createThrustsPanel(){
		JPanel diagInPanel = new JPanel(new GridLayout(2, 2 , 2 ,2));
		T1Diag = new DiagramPanel(front, Color.RED);
		T2Diag = new DiagramPanel(right, Color.RED);
		T3Diag = new DiagramPanel(rear, Color.RED);
		T4Diag = new DiagramPanel(left, Color.RED);
		diagInPanel.add(T1Diag);
		diagInPanel.add(T2Diag);
		diagInPanel.add(T3Diag);
		diagInPanel.add(T4Diag);
		return diagInPanel;
	}
	
	private JPanel createAngleRatesPanel(){
		JPanel diagRatesPanel = new JPanel(new GridLayout(2, 2 , 2 ,2));
		
		//create signal diagram panels
		DiagramPanel diag = new DiagramPanel(droll, Color.BLUE);
		diagRatesPanel.add(diag);
		diag = new DiagramPanel(dpitch, Color.BLUE);
		diagRatesPanel.add(diag);
		diag = new DiagramPanel(dyaw, Color.BLUE);
		diagRatesPanel.add(diag);
		
		return diagRatesPanel;
	}
	
	private JPanel createAngleAccelerationPanel(){
		JPanel diagRatesPanel = new JPanel(new GridLayout(2, 2 , 2 ,2));
		
		//create signal diagram panels
		DiagramPanel diag = new DiagramPanel(ddroll, Color.BLUE);
		diagRatesPanel.add(diag);
		diag = new DiagramPanel(ddpitch, Color.BLUE);
		diagRatesPanel.add(diag);
		diag = new DiagramPanel(ddyaw, Color.BLUE);
		diagRatesPanel.add(diag);
		
		return diagRatesPanel;
	}
	
	private JPanel createVelocitiesPanel(){
		JPanel diagVelocitiesPanel = new JPanel(new GridLayout(2, 2 , 2 ,2));
		
		//create signal diagram panels
		DiagramPanel diag = new DiagramPanel(vx, Color.BLUE);
		diagVelocitiesPanel.add(diag);
		diag = new DiagramPanel(vy, Color.BLUE);
		diagVelocitiesPanel.add(diag);
		diag = new DiagramPanel(vz, Color.BLUE);
		diagVelocitiesPanel.add(diag);
		
		return diagVelocitiesPanel;
	}
	
	private JPanel createAccelerationPanel(){
		JPanel diagAccPanel = new JPanel(new GridLayout(2, 2 , 2 ,2));
		
		//create signal diagram panels
		DiagramPanel diag = new DiagramPanel(ddx, Color.BLUE);
		diagAccPanel.add(diag);
		diag = new DiagramPanel(ddy, Color.BLUE);
		diagAccPanel.add(diag);
		diag = new DiagramPanel(ddz, Color.BLUE);
		diagAccPanel.add(diag);
		
		return diagAccPanel;
	}
	
	private JPanel createRefPanel(){
		JPanel diagRefPanel = new JPanel(new GridLayout(2, 2 , 2 ,2));
		
		//create signal diagram panels
		DiagramPanel diag = new DiagramPanel(ref_roll, Color.BLUE);
		diagRefPanel.add(diag);
		diag = new DiagramPanel(ref_pitch, Color.BLUE);
		diagRefPanel.add(diag);
		diag = new DiagramPanel(ref_yaw, Color.BLUE);
		diagRefPanel.add(diag);
		diag = new DiagramPanel(ref_z, Color.BLUE);
		diagRefPanel.add(diag);
		
		return diagRefPanel;
	}
}
