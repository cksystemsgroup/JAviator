package javiator.terminal;

import simulation.JAviator3D;
import javiator.util.SensorData;
import javiator.util.MotorSignals;

/**
 * @author scraciunas
 *
 */
public class JAviator3DControlPanel implements JAviator3DControl {
    
    private JAviator3D javiator3D;

    public JAviator3DControlPanel(){
        javiator3D = new JAviator3D();
    }

    /* (non-Javadoc)
     * @see javiator.ui.JAviator3DControl#createModel()
     */
    public void createModel() {
        javiator3D.createModel();
    }

    /* (non-Javadoc)
     * @see javiator.ui.JAviator3DControl#resetModel()
     */
    public void resetModel() {
        javiator3D.resetModel();
    }

    /* (non-Javadoc)
     * @see javiator.ui.JAviator3DControl#sendSensorData(javiator.util.SensorData)
     */
    public void sendSensorData(Object data, Object desiredData) {
        SensorData sensorData = (SensorData) data;
        SensorData actuatorData = (SensorData) desiredData;
        javiator3D.sendSensorData((float)sensorData.roll,
        		(float)sensorData.pitch, 
        		(float)sensorData.yaw,
        		(float)sensorData.x,
        		(float)sensorData.y,
        		(float)sensorData.z,
        		(float)actuatorData.roll, 
        		(float)actuatorData.pitch, 
        		(float)actuatorData.yaw, 
        		(float)actuatorData.z+1f);
    }

    /* (non-Javadoc)
     * @see javiator.ui.JAviator3DControl#setRotorSpeed(javiator.util.ActuatorData)
     */
    public void setRotorSpeed(Object data) {
        MotorSignals actuatorData = (MotorSignals) data;
        javiator3D.setRotorSpeed(actuatorData.front, actuatorData.rear, actuatorData.right, actuatorData.left);
    }
}
