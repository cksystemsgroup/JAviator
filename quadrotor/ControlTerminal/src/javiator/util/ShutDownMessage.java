package javiator.util;

public class ShutDownMessage implements Copyable {
	public boolean flag;
	
	public ShutDownMessage()
	{		
		this(false);		
	}
	
	public ShutDownMessage(boolean flag)
	{		
		this.flag = flag;		
	}
	
	public Object clone()
	{
		return new ShutDownMessage(flag);		
	}
	
	public ShutDownMessage deepClone()
	{
		return (ShutDownMessage)clone();
	}

	public void copyTo(Copyable copy)
	{
		((ShutDownMessage) copy).flag = flag;
	}
}
