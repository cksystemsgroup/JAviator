/*--------------------------------------------------------------------------
 *
 * The code is part of JAviator project (http://javiator.cs.uni-salzburg.at)
 *
 *--------------------------------------------------------------------------
 * Date: 11-Apr-2007
 *--------------------------------------------------------------------------
 *
 * Copyright (c) 2006 The University of Salzburg.
 * All rights reserved. Permission is hereby granted, without written 
 * agreement and without license or royalty fees, to use, copy, modify, and 
 * distribute this software and its documentation for any purpose, provided 
 * that the above copyright notice and the following two paragraphs appear 
 * in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF SALZBURG BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 * THE UNIVERSITY OF SALZBURG HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.

 * THE UNIVERSITY OF SALZBURG SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * SALZBURG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 *
 *--------------------------------------------------------------------------
 *Created by Craciunas Silviu (silviu.craciunas@cs.uni-salzburg.at)
 *--------------------------------------------------------------------------
 */

package util;

import java.io.BufferedOutputStream;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import simulation.Constants;

/**
 * Save and load configuration file
 * @author scraciunas
 *
 */
public class Configuration
{

    private static String configFilename   = "config.xml";

    private String        backgroundImage  = "images/sky-29.jpg";
    private float         transparecyValue = 0.1f;
    private int           mouseBehavior    = Constants.MOUSE_BEHAVIOR_ORBITAL;
    private int           fog              = 1;
    private int           axis             = 1;

    public Configuration( )
    {
        loadConfiguration( );
    }

    /**
     * 
     */
    public void loadConfiguration( )
    {
        DocumentBuilderFactory docBuilderFactory = DocumentBuilderFactory.newInstance( );
        DocumentBuilder docBuilder;
        try
        {
            docBuilder = docBuilderFactory.newDocumentBuilder( );
            Document doc = docBuilder.parse( new File( configFilename ) );
            // normalize text representation
            doc.getDocumentElement( ).normalize( );
            System.out.println( "Root element of the doc is " + doc.getDocumentElement( ).getNodeName( ) );

            NodeList listOfItems = doc.getElementsByTagName( "JAviator3DConfiguration" );
            int totalItems = listOfItems.getLength( );
            System.out.println( "Total no of configs : " + totalItems );

            for( int s = 0; s < listOfItems.getLength( ); s++ )
            {

                Node configNode = listOfItems.item( s );
                if( configNode.getNodeType( ) == Node.ELEMENT_NODE )
                {

                    Element configElement = (Element) configNode;

                    //-------------

                    NodeList backgroundImageList = configElement.getElementsByTagName( "backgroundImage" );
                    Element backgroundImageElement = (Element) backgroundImageList.item( 0 );
                    NodeList textFNList = backgroundImageElement.getChildNodes( );
                    System.out.println( "backgroundImage : " + ( (Node) textFNList.item( 0 ) ).getNodeValue( ).trim( ) );
                    this.backgroundImage = ( (Node) textFNList.item( 0 ) ).getNodeValue( ).trim( );

                    //-------------

                    NodeList transparecyValueList = configElement.getElementsByTagName( "transparecyValue" );
                    Element transparecyValueElement = (Element) transparecyValueList.item( 0 );
                    NodeList textTVList = transparecyValueElement.getChildNodes( );
                    System.out.println( "transparencyValue : " + ( (Node) textTVList.item( 0 ) ).getNodeValue( ).trim( ) );
                    this.transparecyValue = Float.parseFloat( ( (Node) textTVList.item( 0 ) ).getNodeValue( ).trim( ) );

                    //-------------

                    NodeList mouseBehaviorList = configElement.getElementsByTagName( "mouseBehavior" );
                    Element mouseBehaviorElement = (Element) mouseBehaviorList.item( 0 );
                    NodeList mouseBehaviorTVList = mouseBehaviorElement.getChildNodes( );
                    System.out.println( "mouseBehavior : " + ( (Node) mouseBehaviorTVList.item( 0 ) ).getNodeValue( ).trim( ) );
                    this.mouseBehavior = Integer.parseInt( ( (Node) mouseBehaviorTVList.item( 0 ) ).getNodeValue( ).trim( ) );

                }
            }
        }
        catch( ParserConfigurationException e )
        {
            // TODO yes .. I am lazy
            System.out.println( e.getMessage( ) );
        }
        catch( SAXException e )
        {
            // TODO Auto-generated catch block
            System.out.println( e.getMessage( ) );
        }
        catch( IOException e )
        {
            // TODO Auto-generated catch block
            System.out.println( e.getMessage( ) );
        }

    }

    /**
     * 
     */
    public void saveConfiguration( )
    {
        try
        {
            OutputStream fout = new FileOutputStream( configFilename );
            OutputStream bout = new BufferedOutputStream( fout );
            OutputStreamWriter out = new OutputStreamWriter( bout, "8859_1" );

            out.write( "<?xml version=\"1.0\" " );
            out.write( "encoding=\"ISO-8859-1\"?>\r\n" );
            out.write( "<JAviator3DConfiguration>\r\n" );

            out.write( "<backgroundImage>" );
            out.write( backgroundImage );
            out.write( "</backgroundImage>\r\n" );
            out.write( "<transparecyValue>" );
            out.write( String.valueOf( transparecyValue ) );
            out.write( "</transparecyValue>\r\n" );
            out.write( "<mouseBehavior>" );
            out.write( String.valueOf( mouseBehavior ) );
            out.write( "</mouseBehavior>\r\n" );
            out.write( "</JAviator3DConfiguration>\r\n" );

            out.flush( );
            out.close( );
        }
        catch( UnsupportedEncodingException e )
        {
            System.out.println( "This VM does not support the Latin-1 character set." );
        }
        catch( IOException e )
        {
            System.out.println( e.getMessage( ) );
        }
    }

    /**
     * @return
     */
    public String getBackgroundImage( )
    {
        return backgroundImage;
    }

    /**
     * @param backgroundImage
     */
    public void setBackgroundImage( String backgroundImage )
    {
        this.backgroundImage = backgroundImage;
    }

    /**
     * @return
     */
    public float getTransparecyValue( )
    {
        return transparecyValue;
    }

    /**
     * @param transparecyValue
     */
    public void setTransparecyValue( float transparecyValue )
    {
        this.transparecyValue = transparecyValue;
    }

    public int getMouseBehavior( )
    {
        return mouseBehavior;
    }

    /**
     * @param mouseBehavior
     */
    public void setMouseBehavior( int mouseBehavior )
    {
        this.mouseBehavior = mouseBehavior;
    }

    /**
     * @return
     */
    public int getAxis( )
    {
        return axis;
    }

    /**
     * @param axis
     */
    public void setAxis( int axis )
    {
        this.axis = axis;
    }

    /**
     * @return
     */
    public int getFog( )
    {
        return fog;
    }

    /**
     * @param fog
     */
    public void setFog( int fog )
    {
        this.fog = fog;
    }

}
