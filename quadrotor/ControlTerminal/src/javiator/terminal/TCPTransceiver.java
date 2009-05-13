/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   JControlTransceiver.java    Uses a Socket to connect to JControl        */
/*                                                                           */
/*   Copyright (c) 2006-2009  Harald Roeck, Rainer Trummer                   */
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

import java.io.IOException;

import java.net.Socket;
import java.net.UnknownHostException;

/*****************************************************************************/
/*                                                                           */
/*   Class TCPTransceiver                                                    */
/*                                                                           */
/*****************************************************************************/

public class TCPTransceiver extends Transceiver
{
    protected TCPTransceiver( ControlTerminal parent, String host, int port )
    {
        super( parent );

        this.host = host;
        this.port = port;
    }

    public synchronized void connect( )
    {
        Socket  socket;
        boolean linked = false;

        try
        {
            socket = new Socket( host, port );
            socket.setTcpNoDelay( true );
            input  = socket.getInputStream( );
            output = socket.getOutputStream( );
            linked = true;
        }
        catch( UnknownHostException e )
        {
            System.err.println( "TCPTransceiver.connect: " + e.getMessage( ) );
        }
        catch( IOException e )
        {
            System.err.println( "TCPTransceiver.connect: " + e.getMessage( ) );
        }

        setConnected( linked );
        setLinked( linked );
    }

    public void disconnect( )
    {
        try
        {
            input  .close( );
            output .close( );
        }
        catch( IOException e )
        {
            System.err.println( "TCPTransceiver.disconnect: " + e.getMessage( ) );
        }
        catch( Exception e )
        {
            System.err.println( "TCPTransceiver.disconnect: " + e.getMessage( ) );
        }

        input  = null;
        output = null;
        setConnected( false );
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Private Section                                                     */
    /*                                                                       */
    /*************************************************************************/

    private String host;
    private int    port;
}

//End of file.