/*
 * (c) Copyright IBM Corp. 2007  All Rights Reserved
 */

/*
 * $RCSfile: Copyable.java,v $
 * $Revision: 1.2 $
 * $Date: 2007/08/07 19:39:48 $
 */
package javiator.util;

/**
 * Interface to be implemented by data types that need a shallow-cloning copy
 * without actually cloning
 */
public interface Copyable
{
	public void copyTo(Copyable copy);
	public Object clone();
}
