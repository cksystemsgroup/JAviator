package javiator.util;

public class ControllerConstants
{
  /* Altitude modes */
  public static final int    ALT_MODE_GROUND    = 0x00;
  public static final int    ALT_MODE_FLYING    = 0x01;
  public static final int    ALT_MODE_SHUTDOWN  = 0x02;

  /* State flags for reporting to control terminal */
  public static final int    ADJUSTING_ROLL     = 0x01;
  public static final int    ADJUSTING_PITCH    = 0x02;
  public static final int    ADJUSTING_YAW      = 0x04;
  public static final int    ADJUSTING_Z        = 0x08;
}