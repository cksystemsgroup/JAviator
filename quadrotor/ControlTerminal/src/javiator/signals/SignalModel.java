package javiator.signals;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Iterator;



/**
 * @author Daniel Iercan, daniel.iercan@cs.uni-salzburg.at
 *  
 */
public final class SignalModel extends MyPropertyChangeSupport {

    public final static String PROPERTY_VALUE_ADDED = "value_added";

    private final ArrayList<Double> array;
    
    private final String name;
    private final String unit;
    private final double scaleFactor;
    private double maxValue;
    private double minValue;
    private double secondMaxValue;
    
    private int maxIndex;
    private int minIndex;
    private int secondMaxIndex;

    public void reset()
    {
    	minValue=0.0;
        maxValue=100.0;
        secondMaxValue=0.0;
        
        maxIndex=0;
        secondMaxIndex=0;
        minIndex=0;
    }
    
    public SignalModel(String pName, String pUnit, double pScaleFactor) {
    	
    	array=new ArrayList<Double>();
        name=pName;
        scaleFactor=pScaleFactor;
        unit=pUnit;
        
        minValue=0.0;
        maxValue=100.0;
        secondMaxValue=0.0;
        
        maxIndex=0;
        secondMaxIndex=0;
        minIndex=0;
    }
    
    public String getName(){
        return name;
    }
    
    public String getUnit(){
        return unit;
    }
    
    public double getScaleFactor(){
        return scaleFactor;
    }

    public synchronized void add(double pValue) {
        array.add(new Double(pValue));
        if(pValue>maxValue){
            maxValue=pValue;
            maxIndex=array.size()-1;
        }
        if(pValue<minValue){
            minValue=pValue;
            minIndex=array.size()-1;
            secondMaxValue=0;
            secondMaxIndex=0;
        }
        
        if(maxIndex<minIndex){
            if(pValue>secondMaxValue){
                secondMaxValue=pValue;
                secondMaxIndex=array.size()-1;
            }
        }
        
        firePropertyChangeEvent(PROPERTY_VALUE_ADDED, null, new Double(pValue));
    }

    public int getMinIndex(){
        return minIndex;
    }

    public int getMaxIndex(){
        return maxIndex;
    }
    
    public int getSecondMaxIndex(){
        return secondMaxIndex;
    }
    
    /**
     * If the idex is out of bounds return 0
     * 
     * @param index
     * @return
     */
    public synchronized double get(int index) {
        if (index >= 0 && index < array.size()) {
            return ((Double) array.get(index)).doubleValue();
        } else
            return 0.0;
    }
    
    public synchronized double getScaledValue(int index) {
        if (index >= 0 && index < array.size()) {
            return ((Double) array.get(index)).doubleValue()*scaleFactor;
        } else
            return 0.0;
    }
    
    public double getMaxValue(){
        return maxValue;
    }
    
    public double getMinValue(){
        return minValue;
    }
    
    public double getSecondMaxValue(){
        return secondMaxValue;
    }
    
    public void clear(){
        array.clear();
    }

    public synchronized int getSize() {
        return array.size();
    }

    public double[] toArray() {
        final double[] tmpArr = new double[array.size()];
        int i = 0;

        Iterator it = array.iterator();
        while (it.hasNext()) {
            tmpArr[i++] = ((Double) it.next()).doubleValue();
        }

        return tmpArr;
    }

    public Iterator iterator() {
        return array.iterator();
    }

    public synchronized void writeToFile(String pFileName) {
        try {
            PrintWriter out = new PrintWriter(new FileOutputStream(pFileName));
            Iterator it = iterator();
            while (it.hasNext()) {
                out.write("" + ((Double) it.next()).doubleValue());
                if (it.hasNext())
                    out.write(";");
            }
            out.flush();
            out.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }
    
    public boolean equals(Object signal){
        return name.compareTo(((SignalModel)signal).name)==0;
    }
    
    public int hashCode(){
        return name.hashCode();
    }
    
    public String toString(){
        String str="";
        Iterator it = iterator();
        while (it.hasNext()) {
            str+="" + ((Double) it.next()).doubleValue();
            if (it.hasNext())
                str+=";";
        }
        
        str=name+" {"+str+"}";
        
        return str;
    }
}
