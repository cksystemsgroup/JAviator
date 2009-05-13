package javiator.signals;


import java.awt.Color;
import java.awt.Dimension;
import java.awt.Frame;
import java.util.Hashtable;

import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.table.AbstractTableModel;


/**
 * 
 * @author Daniel Iercan, daniel.iercan@cs.uni-salzburg.at
 *
 * This is a dialog that will plot a signal and it will compute the quality indicators
 */
public class DiagramDialog extends JDialog {
    /**
     * 
     */
    private static final long serialVersionUID = 1L;
    private final SignalModel signal;
    
    private final DiagramPanel diagram;
    
    private final IndicatorsTableModel model;

    private double t1,tc,tm,tr,ts;
    private double sigma;
    private double zmax;
    private double zmin;
    private double zinf;
    
    public DiagramDialog(JFrame pParent, SignalModel pSignal, boolean pShowIndicators, Color pColor){
        super((Frame)pParent,pSignal.getName());
        
        model=new IndicatorsTableModel(pSignal.getName());
        
        signal=pSignal;
        diagram=new DiagramPanel(signal,pColor,pShowIndicators,true,null);
        diagram.setEnabled(false);
        
        final JTable table=new JTable(model);
        table.getSelectionModel().setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
        table.getSelectionModel().addListSelectionListener(new ListSelectionListener(){
            public void valueChanged(ListSelectionEvent arg0) {
                int[] sel=table.getSelectedRows();
                int length=0;
                for(int i=0;i<sel.length;i++){
                    if(sel[i]!=model.INDEX_SIGMA){
                        length++;
                    }
                }
                double[][] auxLines=new double[length][2];
                for(int i=0,j=0;i<sel.length;i++){
                    if(sel[i]!=model.INDEX_SIGMA){
                        if(sel[i]>=0 && sel[i]<model.INDEX_SIGMA){
                            auxLines[j][0]=0;
                            auxLines[j][1]=((Double)model.getValueAt(sel[i],1)).doubleValue();
                        }
                        else{
                            if(sel[i]>model.INDEX_SIGMA){
                                auxLines[j][0]=signal.getMinIndex()+((Double)model.getValueAt(sel[i],1)).doubleValue();
                                auxLines[j][1]=0;
                            }
                        }
                        j++;
                    }
                }
                
                diagram.setAuxLines(auxLines);
            }
        });
        
        final JScrollPane scroll=new JScrollPane(table);
        
        final JSplitPane contentPane=new JSplitPane(JSplitPane.VERTICAL_SPLIT,scroll,diagram);
        contentPane.setOneTouchExpandable(true);
        contentPane.setDividerLocation(165);
        
        setContentPane(contentPane);
        setSize(new Dimension(500,500));
    }
    
    public void setVisible(boolean pVisible){
        super.setVisible(pVisible);
        computeIndicators();
    }
    
    private void computeIndicators(){
        if(diagram.isShowIndicatorsEnabled()){
            int i;
            zmax=signal.getMaxValue();
            if(signal.getMaxIndex()<signal.getMinIndex()){
                zmax=signal.getSecondMaxValue();
            }
            zmin=signal.getMinValue();
            
            //compute zinf
            zinf=0.0;
            for(i=1;i<=100;i++){
                zinf+=signal.get(signal.getSize()-i);
            }
            zinf=zinf/100;
            
            double deltaZ=(zinf-signal.getMinValue());
            
            if(deltaZ<0.001)
                return;
            
            //compute sigma
            sigma=(zmax-zinf)/deltaZ;
            
            double min=0.05*deltaZ;
            double half=0.5*deltaZ;
            double max=0.95*deltaZ;
            
            //compute ts
            ts=0;
            for(i=signal.getMinIndex();i<signal.getSize();i++){
                if((signal.get(i)-signal.getMinValue())<half){
                    ts+=1;
                }
                else{
                    break;
                }
            }
            
            // compute tc
            tc=0;
            for(i=signal.getMinIndex();i<signal.getSize();i++){
                if((signal.get(i)-signal.getMinValue())>max){
                    break;
                }
                if((signal.get(i)-signal.getMinValue())>=min){
                    tc+=1;
                }
            }
            
            //compute tm
            tm=0;
            for(i=signal.getMinIndex();i<signal.getSize();i++){
                if(signal.get(i)<zmax){
                    tm+=1;
                }
                else{
                    tm+=1;
                    break;
                }
            }
            
            // compute t1
            t1=0;
            for(i=signal.getMinIndex();i<signal.getSize();i++){
                if(signal.get(i)<zinf){
                    t1+=1;
                }
                else{
                    break;
                }
            }
            
            //compute tr
            tr=0;
            max=zinf+0.02*deltaZ;
            min=zinf-0.02*deltaZ;
            for(i=signal.getMinIndex();i<signal.getSize();i++){
                if(signal.get(i)<min || signal.get(i)>max){
                    tr=i-signal.getMinIndex();
                }
            }
            
            model.setIndicator(model.INDEX_SIGMA,((int)(sigma*100))/100.0);
            model.setIndicator(model.INDEX_T1,t1);
            model.setIndicator(model.INDEX_TC,tc);
            model.setIndicator(model.INDEX_TM,tm);
            model.setIndicator(model.INDEX_TR,tr);
            model.setIndicator(model.INDEX_TS,ts);
            model.setIndicator(model.INDEX_ZINF,((int)(zinf*100))/100.0);
            model.setIndicator(model.INDEX_ZMAX,((int)(zmax*100))/100.0);
            model.setIndicator(model.INDEX_ZMIN,((int)(zmin*100))/100.0);
        }
        else{
            zmax=signal.getMaxValue();
            zmin=signal.getMinValue();
            
            model.setIndicator(model.INDEX_ZMAX,((int)(zmax*100))/100.0);
            model.setIndicator(model.INDEX_ZMIN,((int)(zmin*100))/100.0);
        }
    }
    
    private class IndicatorsTableModel extends AbstractTableModel{

        /**
         * 
         */
        private static final long serialVersionUID = 1L;

        public final int INDEX_ZINF=0;
        
        public final int INDEX_ZMIN=1;
        
        public final int INDEX_ZMAX=2;
        
        public final int INDEX_SIGMA=3;
        
        public final int INDEX_TS=4;
        
        public final int INDEX_TC=5;
        
        public final int INDEX_T1=6;
        
        public final int INDEX_TM=7;
        
        public final int INDEX_TR=8;
        
        public final String[] indicatorsNames=new String[]{"inf","min","max","sigma","ts/Te","tc/Te","t1/Te","tm/Te","tr/Te"};
        
        public final String[] columnNames=new String[]{"Quality Indicator","Value"};
        
        private Hashtable<String,Double> data;
        
        public IndicatorsTableModel(String name){
            data=new Hashtable<String,Double>();
            indicatorsNames[INDEX_ZINF]=name+indicatorsNames[INDEX_ZINF];
            indicatorsNames[INDEX_ZMIN]=name+indicatorsNames[INDEX_ZMIN];
            indicatorsNames[INDEX_ZMAX]=name+indicatorsNames[INDEX_ZMAX];
            for(int i=0;i<indicatorsNames.length;i++){
                data.put(indicatorsNames[i],new Double(0));
            }
        }
        
        public int getRowCount() {
            return data.size();
        }

        public int getColumnCount() {
            return columnNames.length;
        }
        
        public String getColumnName(int arg0){
            return columnNames[arg0];
        }

        public Object getValueAt(int arg0, int arg1) {
            if(arg1==0){
                return indicatorsNames[arg0];
            }
            else{
                return data.get(indicatorsNames[arg0]);
            }
        }
        
        public void setIndicator(int pIndex, double pValue){
            data.remove(indicatorsNames[pIndex]);
            data.put(indicatorsNames[pIndex], new Double(pValue));
            fireTableDataChanged();
        }
    }
}
