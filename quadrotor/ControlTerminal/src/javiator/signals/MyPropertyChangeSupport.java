package javiator.signals;

import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;

/**
 * @author Daniel Iercan, daniel.iercan@cs.uni-salzburg.at
 *
 */
public abstract class MyPropertyChangeSupport {
    private PropertyChangeSupport propertyChangeSupport;
    
    public MyPropertyChangeSupport(){
        propertyChangeSupport=new PropertyChangeSupport(this);
    }
    
    /**
     * Add a new listener for the specified property
     * @param pPropertyName
     * @param pListener
     */
    public void addPropertyChangeListener(String pPropertyName, PropertyChangeListener pListener){
        propertyChangeSupport.addPropertyChangeListener(pPropertyName, pListener);
    }
    
    /**
     * Remove a listener for the specified property
     * @param pPropertyName
     * @param pListener
     */
    public void removePropertyChangeListener(String pPropertyName, PropertyChangeListener pListener){
        propertyChangeSupport.removePropertyChangeListener(pPropertyName, pListener);
    }
    
    /**
     * Fire the property changed event for the specified property
     * @param pPropertyName
     * @param pOldValue
     * @param pNewValue
     */
    protected void firePropertyChangeEvent(String pPropertyName, Object pOldValue, Object pNewValue){
        propertyChangeSupport.firePropertyChange(pPropertyName, pOldValue, pNewValue);
    }
    
    /**
     * Fire the property changed event for the specified property
     * @param pPropertyName
     * @param pOldValue
     * @param pNewValue
     */
    protected void firePropertyChangeEvent(String pPropertyName, int pOldValue, int pNewValue){
        propertyChangeSupport.firePropertyChange(pPropertyName, pOldValue, pNewValue);
    }
    
    /**
     * Fire the property changed event for the specified property
     * @param pPropertyName
     * @param pOldValue
     * @param pNewValue
     */
    protected void firePropertyChangeEvent(String pPropertyName, boolean pOldValue, boolean pNewValue){
        propertyChangeSupport.firePropertyChange(pPropertyName, pOldValue, pNewValue);
    }
}
