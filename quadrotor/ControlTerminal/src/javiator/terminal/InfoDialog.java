/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   InfoDialog.java    Constructs a non-modal dialog presenting either      */
/*                      key/button assistance or application info.           */
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
import java.awt.Label;
import java.awt.Color;
import java.awt.Point;
import java.awt.GraphicsEnvironment;

import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.KeyListener;
import java.awt.event.KeyEvent;

/*****************************************************************************/
/*                                                                           */
/*   Class InfoDialog                                                        */
/*                                                                           */
/*****************************************************************************/

public class InfoDialog extends Dialog
{
    public static final long serialVersionUID    = 1;
    public static final byte TYPE_KEY_ASSISTANCE = 1;
    public static final byte TYPE_ABOUT_TERMINAL = 2;

    public static InfoDialog createInstance( ControlTerminal parent, String title, byte type )
    {
        if( Instance == null )
        {
            Instance = new InfoDialog( parent, title, type );
        }

        return( Instance );
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Private Section                                                     */
    /*                                                                       */
    /*************************************************************************/

    private static final String NOT_USED  = "(not used)";
    private static final String ACTIVATE_ = "Activate ";

    private static final String[] KEYBOARD_LIST =
    {
        ControlTerminal.ROLL + " left",                                          "Left",
        ControlTerminal.ROLL + " right",                                         "Right",
        ControlTerminal.PITCH + " forward",                                      "Up",
        ControlTerminal.PITCH + " backward",                                     "Down",
        ControlTerminal.YAW + " left",                                           "A",
        ControlTerminal.YAW + " right",                                          "D",
        "Ascend",                                                                "W",
        "Descend",                                                               "S",
        ControlTerminal.KEY_ASSISTANCE,                                          "F1",
        ControlTerminal.ABOUT_TERMINAL,                                          "F2",
        NOT_USED,                                                                "F3",
        "Log" + ControlTerminal._DATA,                                           "F4",
        ControlTerminal.JOYSTICK + ControlTerminal._MODE,                        "F5",
        ControlTerminal.UDP_ + ControlTerminal.CONNECTION,                       "F6",
        ControlTerminal.CONNECT_TO + ControlTerminal._HELI,                      "F7",
        ControlTerminal.SWITCH + ControlTerminal._HELI + ControlTerminal._MODE,  "F8",
        ControlTerminal.SHUT_DOWN + ControlTerminal._HELI,                       "F9",
        ControlTerminal.PORT_SETTINGS,                                           "F10",
        ControlTerminal.SET_PARAMETERS,                                          "F12"
    };

    private static final String[] JOYSTICK_LIST =
    {
    	ACTIVATE_ + "Throttle",                                                  "1",
    	ACTIVATE_ + ControlTerminal.YAW,                                         "2",
        NOT_USED,                                                                "3",
        NOT_USED,                                                                "4",
        NOT_USED,                                                                "5",
        NOT_USED,                                                                "6",
        ControlTerminal.SHUT_DOWN + ControlTerminal._HELI,                       "7",
        NOT_USED,                                                                "8",
        NOT_USED,                                                                "9",
        ControlTerminal.SWITCH + ControlTerminal._HELI + ControlTerminal._MODE,  "10"
    };

    private static final String[] ABOUT_STRINGS =
    {
    	ControlTerminal.NIL,
        "JAviator Control Terminal version 3.2",
        ControlTerminal.NIL,
        "Copyright (c) 2006-2009  Rainer Trummer",
        "Department of Computer Sciences",
        "University of Salzburg, Austria",
        ControlTerminal.NIL,
        "This software is part of the JAviator project",
        "http://javiator.cs.uni-salzburg.at",
        ControlTerminal.NIL
    };

    private static InfoDialog Instance = null;

    private InfoDialog( ControlTerminal parent, String title, byte type )
    {
        super( parent, title, false );

        setBackground( Color.WHITE );

        if( type == TYPE_KEY_ASSISTANCE )
        {
            makeKeyAssistance( );
        }
        else if( type == TYPE_ABOUT_TERMINAL )
        {
            makeAboutTerminal( );
        }

        pack( );

        addWindowListener( new WindowAdapter( )
        {
            public void windowClosing( WindowEvent we )
            {
                closeDialog( );
            }
        } );
        
        addKeyListener( new KeyListener( )
        {
            public void keyPressed( KeyEvent ke )
            {
            	int keyCode = ke.getKeyCode( );

            	if( keyCode == KeyEvent.VK_ESCAPE || keyCode == KeyEvent.VK_ENTER )
            	{
                    closeDialog( );
            	}
            }

            public void keyReleased( KeyEvent ke )
            {
            }

            public void keyTyped( KeyEvent ke )
            {
            }
        } );

        Point center = GraphicsEnvironment.getLocalGraphicsEnvironment( ).getCenterPoint( );
        setLocation( center.x - ( getWidth( ) >> 1 ), center.y - ( getHeight( ) >> 1 ) );
        setResizable( false );
        setVisible( true );
    }

    private void closeDialog( )
    {
        dispose( );
        Instance = null;
    }

    private void makeKeyAssistance()
    {
        int i, items = KEYBOARD_LIST.length;

        Panel keyboardPanel = new Panel( new GridLayout( items >> 1, 2, 50, 0 ) );

        for( i = 0; i < items; ++i )
        {
            keyboardPanel.add( new Label( KEYBOARD_LIST[i], Label.LEFT ) );
        }

        items = JOYSTICK_LIST.length;

        Panel joystickPanel = new Panel( new GridLayout( items >> 1, 2, 50, 0 ) );

        for( i = 0; i < items; ++i )
        {
            joystickPanel.add( new Label( JOYSTICK_LIST[i], Label.LEFT ) );
        }

        Panel northPanel = new Panel( new BorderLayout( ) );
        northPanel.add( new Label( ControlTerminal.KEYBOARD + " Assignments", Label.CENTER ), BorderLayout.NORTH );
        northPanel.add( keyboardPanel, BorderLayout.SOUTH );

        Panel southPanel = new Panel( new BorderLayout( ) );
        southPanel.add( new Label( ControlTerminal.JOYSTICK + " Buttons", Label.CENTER ), BorderLayout.NORTH );
        southPanel.add( joystickPanel, BorderLayout.SOUTH );

        setLayout( new BorderLayout( ) );
        add( northPanel, BorderLayout.NORTH );
        add( southPanel, BorderLayout.SOUTH );
    }

    private void makeAboutTerminal( )
    {
        int items = ABOUT_STRINGS.length;

        setLayout( new GridLayout( items, 1 ) );

        for( int i = 0; i < items; ++i )
        {
            add( new Label( ABOUT_STRINGS[i], Label.CENTER ) );
        }
    }
}

// End of file.