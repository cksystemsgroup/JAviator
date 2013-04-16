/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   UIManagerColor.java    Provides specific UIManager colors.              */
/*                                                                           */
/*   Copyright (c) 2006-2013 Rainer Trummer <rainer.trummer@gmail.com>       */
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

import java.awt.Color;
import javax.swing.UIManager;

/*****************************************************************************/
/*                                                                           */
/*   Class UIManagerColor                                                    */
/*                                                                           */
/*****************************************************************************/

public class UIManagerColor
{
    public static final long serialVersionUID = 1;

    public UIManagerColor( )
    {
    	initColors( );
    }

    public static Color getButtonBackground( )
    {
        return( buttonBackground );
    }

    public static Color getButtonDarkShadow( )
    {
        return( buttonDarkShadow );
    }

    public static Color getButtonForeground( )
    {
        return( buttonForeground );
    }

    public static Color getButtonHighlight( )
    {
        return( buttonHighlight );
    }

    public static Color getButtonLight( )
    {
        return( buttonLight );
    }

    public static Color getButtonShadow( )
    {
        return( buttonShadow );
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Private Section                                                     */
    /*                                                                       */
    /*************************************************************************/

    private static Color buttonBackground = null;
    private static Color buttonDarkShadow = null;
    private static Color buttonForeground = null;
    private static Color buttonHighlight  = null;
    private static Color buttonLight      = null;
    private static Color buttonShadow     = null;

    private void initColors( )
    {
        try
        {
            UIManager.setLookAndFeel( UIManager.getSystemLookAndFeelClassName( ) );
		}
		catch( Exception e )
		{
		}

        buttonBackground = UIManager.getColor( "Button.background" );
        buttonDarkShadow = UIManager.getColor( "Button.darkShadow" );
        buttonForeground = UIManager.getColor( "Button.foreground" );
        buttonHighlight  = UIManager.getColor( "Button.highlight" );
        buttonLight      = UIManager.getColor( "Button.light" );
        buttonShadow     = UIManager.getColor( "Button.shadow" );
    }
}

/* End of file */