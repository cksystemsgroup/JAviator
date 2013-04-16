/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   EditField.java    Constructs an edit field that can be sized/resized    */
/*                     completely independent of its font settings.          */
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

import java.awt.TextField;
import java.awt.Dimension;

/*************************************************************************/
/*                                                                       */
/* Class EditField                                                       */
/*                                                                       */
/*************************************************************************/

public class EditField extends TextField
{
    public static final long serialVersionUID = 1;

    public EditField( String text )
  	{
   		super( );
   		initWindow( text, super.getWidth( ), super.getHeight( ) );
   	}

    public EditField( String text, Dimension size )
  	{
   		super( );
   		initWindow( text, size.width, size.height );
   	}

    public EditField( String text, int width, int height )
  	{
   		super( );
   		initWindow( text, width, height );
   	}

   	public Dimension getPreferredSize( )
   	{
   		return( size );
   	}

   	public Dimension getMaximumSize( )
   	{
   		return( size );
   	}

   	public Dimension getMinimumSize( )
   	{
   		return( size );
   	}

   	public Dimension getSize( )
   	{
   		return( size );
   	}

   	public void setPreferredSize( Dimension size )
   	{
   		this.size.width  = size.width;
   		this.size.height = size.height;
   	}

   	public void setMaximumSize( Dimension size )
   	{
   		this.size.width  = size.width;
   		this.size.height = size.height;
   	}

   	public void setMinimumSize( Dimension size )
   	{
   		this.size.width  = size.width;
   		this.size.height = size.height;
   	}

   	public void setSize( Dimension size )
   	{
   		this.size.width  = size.width;
   		this.size.height = size.height;
   	}

   	public void setSize( int width, int height )
   	{
   		this.size.width  = width;
   		this.size.height = height;
   	}

    /*************************************************************************/
    /*                                                                       */
    /*   Private Section                                                     */
    /*                                                                       */
    /*************************************************************************/

   	private Dimension size = null;

    private void initWindow( String text, int width, int height )
    {
        size = new Dimension( width, height );

        setForeground( UIManagerColor.getButtonForeground( ) );
        setText( text );
    }
}

/* End of file */