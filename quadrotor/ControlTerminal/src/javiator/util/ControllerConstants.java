/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   Copyright (c) 2006-2010  Harald Roeck, Rainer Trummer                   */
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

package javiator.util;

public class ControllerConstants
{
    /* Altitude modes */
    public static final int ALT_MODE_GROUND   = 0x00;
    public static final int ALT_MODE_FLYING   = 0x01;
    public static final int ALT_MODE_SHUTDOWN = 0x02;

    /* State flags */
    public static final int STATE_TEST_MODE   = 0x01;

    /* Max motor signal */
    public static final int MOTOR_MAX         = 16000;
}