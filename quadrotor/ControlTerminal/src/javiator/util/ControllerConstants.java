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
    /* helicopter states */
    public static final int HELI_STATE_GROUND   = 0x01;
    public static final int HELI_STATE_FLYING   = 0x02;
    public static final int HELI_STATE_SHUTDOWN = 0x04;

    /* helicopter modes */
    public static final int HELI_MODE_MAN_CTRL  = 0x01;
    public static final int HELI_MODE_POS_CTRL  = 0x02;

    /* max motor signal */
    public static final int MOTOR_MAX           = 16000;
}