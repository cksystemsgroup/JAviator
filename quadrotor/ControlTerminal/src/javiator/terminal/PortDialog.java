/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   PortDialog.java    Constructs a non-modal dialog that allows to change  */
/*                      the port and host required for relay connection.     */
/*                                                                           */
/*   Copyright (c) 2006-2009  Rainer Trummer                                 */
/*                                                                           */
/*   This program is free software; you can redistribute it and/or modify    */
/*   it under the terms of the GNU General Public License as published by    */
/*   the Free Software Foundation; either version 2 of the License, or       */
/*   (at your option) any later version.                                     */
/*                                                                           */
/*   This program is distributed in the hope that it will be useful,         */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*   GNU General Public License for more details.                            */
/*                                                                           */
/*   You should have received a copy of the GNU General Public License       */
/*   along with this program; if not, write to the Free Software Foundation, */
/*   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.      */
/*                                                                           */
/*****************************************************************************/

package javiator.terminal;

import java.awt.Dialog;
import java.awt.Panel;
import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.Button;
import java.awt.Label;
import java.awt.Font;
import java.awt.Color;
import java.awt.Point;
import java.awt.GraphicsEnvironment;

import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.KeyListener;
import java.awt.event.KeyEvent;

/*****************************************************************************/
/*                                                                           */
/*   Class PortDialog                                                        */
/*                                                                           */
/*****************************************************************************/

public class PortDialog extends Dialog
{
    public static final long serialVersionUID = 1;

    public static PortDialog createInstance( ControlTerminal parent, String title )
    {
        if( Instance == null )
        {
            Instance = new PortDialog( parent, title );
        }

        return( Instance );
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Private Section                                                     */
    /*                                                                       */
    /*************************************************************************/

    private static PortDialog      Instance  = null;
    private        ControlTerminal parent    = null;
    private        EditField       hostField = null;
    private        EditField       portField = null;

    private PortDialog( ControlTerminal parent, String title )
    {
        super( parent, title, false );

        this.parent = parent;
        hostField   = new EditField( parent.relayHost, 130, 10 );
        portField   = new EditField( ControlTerminal.NIL + parent.relayPort, 50, 10 );

        setBackground( Color.WHITE );
        makePanel( );
        pack( );

        addWindowListener( new WindowAdapter( )
        {
            public void windowClosing( WindowEvent we )
            {
                closeDialog( false );
            }
        } );

        Point center = GraphicsEnvironment.getLocalGraphicsEnvironment( ).getCenterPoint( );
        setLocation( center.x - ( getWidth( ) >> 1 ), center.y - ( getHeight( ) >> 1 ) );
        setResizable( false );
        setVisible( true );
    }

    private void makePanel( )
    {
        Label colonLabel = new Label( ":", Label.RIGHT );
        colonLabel.setFont( new Font( "monospace", Font.BOLD, 12 ) );

        Panel hostPanel = new Panel( new BorderLayout( ) );
        hostPanel.add( new Label( "Host", Label.CENTER ), BorderLayout.NORTH );
        hostPanel.add( hostField, BorderLayout.CENTER );
        hostPanel.add( new Label( ), BorderLayout.SOUTH );
        hostPanel.add( new Label( ), BorderLayout.WEST );
        hostPanel.add( colonLabel, BorderLayout.EAST );

        Panel portPanel = new Panel( new BorderLayout( ) );
        portPanel.add( new Label( "Port", Label.CENTER ), BorderLayout.NORTH );
        portPanel.add( portField, BorderLayout.CENTER );
        portPanel.add( new Label( ), BorderLayout.SOUTH );
        portPanel.add( new Label( ), BorderLayout.WEST );
        portPanel.add( new Label( ), BorderLayout.EAST );

        Panel northPanel = new Panel( new BorderLayout( ) );
        northPanel.add( hostPanel, BorderLayout.WEST );
        northPanel.add( portPanel, BorderLayout.EAST );

        Button okButton = new Button( "Ok" );
        okButton.addActionListener( new ActionListener( )
        {
            public void actionPerformed( ActionEvent ae )
            {
                closeDialog( true );
            }
        } );
        okButton.addKeyListener( new KeyListener( )
        {
            public void keyPressed( KeyEvent ke )
            {
            	if( ke.getKeyCode( ) == KeyEvent.VK_ENTER )
            	{
                    closeDialog( true );
            	}
            }

            public void keyReleased( KeyEvent ke )
            {
            }

            public void keyTyped( KeyEvent ke )
            {
            }
        } );

        Button cancelButton = new Button( "Cancel" );
        cancelButton.addActionListener( new ActionListener( )
        {
            public void actionPerformed( ActionEvent ae )
            {
                closeDialog( false );
            }
        } );
        cancelButton.addKeyListener( new KeyListener( )
        {
            public void keyPressed( KeyEvent ke )
            {
            	if( ke.getKeyCode( ) == KeyEvent.VK_ENTER )
            	{
                    closeDialog( false );
            	}
            }

            public void keyReleased( KeyEvent ke )
            {
            }

            public void keyTyped( KeyEvent ke )
            {
            }
        } );

        Panel buttonPanel = new Panel( new GridLayout( 1, 3 ) );
        buttonPanel.add( okButton );
        buttonPanel.add( new Label( ) );
        buttonPanel.add( cancelButton );

        Panel southPanel = new Panel( new BorderLayout( ) );
        southPanel.add( new Label( ), BorderLayout.WEST );
        southPanel.add( buttonPanel, BorderLayout.CENTER );
        southPanel.add( new Label( ), BorderLayout.EAST );

        setLayout( new BorderLayout( ) );
        add( northPanel, BorderLayout.NORTH );
        add( southPanel, BorderLayout.SOUTH );
    }

    private void closeDialog( boolean assume )
    {
        if( assume )
        {
            parent.relayHost = hostField.getText( );

            try
            {
                parent.relayPort = Integer.parseInt( portField.getText( ) );
            }
            catch( Exception e )
            {
                parent.relayPort = 0;
                System.err.println( "PortDialog.closeDialog: " + e.getMessage( ) );
            }
        }

        dispose( );
        Instance = null;
    }
}

/* End of file */