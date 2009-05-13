package javiator.util;

public abstract class NumeratedSendable implements Sendable, Copyable {

	public void copyTo(Copyable copy)
	{
		((NumeratedSendable) copy).sequence = sequence;
	}

	protected long sequence;
	public long getSequence() {
		return sequence;
	}

	public void setSequence(long seq) {
		sequence = seq;
	}

	public Object clone( )  {
		NumeratedSendable copy;
		
		try {
			copy = (NumeratedSendable) super.clone();
		} catch (CloneNotSupportedException e) {
			// TODO Auto-generated catch block
			copy = null;
		}
		
		
		return copy;
	}
}
