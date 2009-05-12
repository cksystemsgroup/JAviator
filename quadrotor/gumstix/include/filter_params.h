/*
 * Copyright (c) Harald Roeck hroeck@cs.uni-salzburg.at
 * Copyright (c) Rainer Trummer rtrummer@cs.uni-salzburg.at
 *
 * University Salzburg, www.uni-salzburg.at
 * Department of Computer Science, cs.uni-salzburg.at
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//ddz butter filter parameters
//this should be adjust so that B1+B2+A2=1, the greater A2 is the 
//stronger the filter is, if A2=0, B1=1 and B2=0 then there is no filtering
#define DDZ_FILTER_A2 0.95
#define DDZ_FILTER_B1 0.025
#define DDZ_FILTER_B2 0.025

//ddz butter filter parameters
//this should be adjust so that B1+B2+A2=1, the greater A2 is the 
//stronger the filter is, if A2=0, B1=1 and B2=0 then there is no filtering
#define Z_FILTER_A2 0.95
#define Z_FILTER_B1 0.025
#define Z_FILTER_B2 0.025

//z_ref butter filter parameters
//this should be adjust so that B1+B2+A2=1, the greater A2 is the 
//stronger th filter is, if A2=0, B1=1 and B2=0 then there is no filtering
#define Z_REF_FILTER_A2 0.9
#define Z_REF_FILTER_B1 0.1
#define Z_REF_FILTER_B2 0.0

/* End of file */
