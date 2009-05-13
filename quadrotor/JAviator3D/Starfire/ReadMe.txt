StarfireExt.jar - version 2.20

History:
Version 2.20:
	Improved file buffering
	Added option to disable loading of textures
	Permanent Texture Cache utilized so only one copy of a particular texture is loaded.
Version 2.00:
	Added support for loading via URL
	Added ability to set crease angle for loading round shapes (humanoids)
Version 1.31:
	Reporting in log, actual color values for material chunks.
Version 1.30:
	Fixed problem where a material was applied to zero triangles causing a problem in Java 3D 1.2.1 beta 1 (didn't cause a problem in Java 3D 1.2 release).
Version 1.20:
	Inspector3DS changed to Loader3DS (standardized to other loaders)
	Inspector3DS left as shell (uses Loader3DS) for backward compatiblity and ease of use.
	Grouping is now supported so that named groups result in a TransformGroup with their objects under them.
	IMPORTANT-> Package name changed from com.starfireresearch to com.mnstarfire
Version 1.12b:
	Now opening file as read only so that files from CD can be used
Version 1.11b:
	Texture lighting now optional
Version 1.10b:
	Two sided materials now handled by doubling the vertices (with reversing the order of the triangles for the back face)
Version 1.05a:
	Better error handling for invalid filenames
Version 1.04a:
	Put in check when applying a material that the number of faces (triangles) isn't zero
Version 1.03a:
	Texture file names converted to lowercase to improve ease of use on Unix/Linux
	Alternate Texture path can be set
	Pathing fixed for Linux/Unix


This should be placed in your library extensions directory.
(default would be: C:\Program Files\JavaSoft\Jre\1.3\lib\ext\)

You can then develop Java 3D applications using our 3DS loader "Inspector3DS" (for ease of use).
Or "Loader3DS" (compatible with Sun's Loader model using "Scenes").

This is a class to load a 3D model in 3DS format into Java 3D.

License:
This code is being provided as a service to the Java 3D community.  It may be used and improved royalty free.  It may not be modified and sold.  All rights are 
reserved by the author, John Wright, and Starfire Research.  We are asking for a "donation" of $10 to help defray the cost of development and distribution.

Please send checks to:
	Starfire Research
	5235 West 139th Street
	Savage, MN 55378

Email Contact: wright@mnstarfire.com


Minimal usage code snippet:

import com.mnstarfire.loaders3d.Inspector3DS;
...
Inspector3DS loader = new Inspector3DS("model.3ds"); // constructor
loader.parseIt(); // process the file
TransformGroup theModel = loader.getModel(); // get the resulting 3D model as a Transform Group with Shape3Ds as children

Detailed usage code snippet (using Inspector3DS):

  import com.mnstarfire.loaders3d.Inspector3DS;
  ...
  Inspector3DS loader = new Inspector3DS("D:\ThreeD\models\model.3ds");
  loader.setLogging(true); // turns on logging to a disk file "log3ds.txt"
  loader.setDetail(6); // sets the level of detail to be logged
  loader.setTextureLightingOn(); // turns on modulate mode for textures (lighting)
  // loader.setTexturePath("D:\MyTextures"); // optional alternative path to find texture files
  loader.parseIt();
  TransformGroup theModel = loader.getModel();

or (using Loader3DS):

// setup a file name "fileName"
try {
  Loader3DS loader = new Loader3DS();
	// optional options to be used
	// loader.setLogging(true); // turns on writing a log file
	// loader.setDetail(7); // sets level of detail of report log
	// loader.setTextureLightingOn(); // turns on texture modulate mode
	// loader.setTexturePath("D:\MyTextures"); // optional alternate path to find texture files
	// loader.noTextures(); // if you don't want to load textures
  Scene theScene = loader.load(fileName);
}
catch(FileNotFoundException fnf) {
	// Couldn't find the file you requested - deal with it!
}
// Use the scene as per Sun's documentation of SceneBase


Minimal usage code snippet (for using via URL):

import com.mnstarfire.loaders3d.Inspector3DS;
...
URL url = new URL("http://www.company.com/models/sample.3ds");
String urlBase = "http://www.company.com/models/";
...
Inspector3DS loader = new Inspector3DS(url); // constructor
loader.setURLBase(urlBase);
loader.parseIt(); // process the file
TransformGroup theModel = loader.getModel(); // get the resulting 3D model as a Transform Group with Shape3Ds as children


Features:

     Textures Cached
     Loading from URL
     Reads geometry and materials (including textures)
     Calculates normals
     Relatively fast file processing & high quality resultant images
     Materials include diffuse, ambient and specular colors; transparency is supported
     Can generate a debug log file detailing the parsing of the 3DS file and the values read.
     Named groups are supported

Weaknesses:

     3DS format supports only 8.3 filenames for textures
     multiple textures assigned to a single object are not properly read
     Debug Log files are generated as Windows standard text files (not sure how readable they will be on other platforms)

Performance Notes:
	Texture blending supports lighting on textures but also creates a performance hit
	Using models with double sided materials creates a big performance hit (doubles the number of vertices!)

Other comments:

     This design is intended to load 3D models not "scenes".
     Lights are ignored (assumed to be provided by the Java 3D scene)
     Animation information is not currently imported or used

We recommend that you "install" this jar file to your "jre\lib\ext\" directory (both for the runtime and SDK if you are a developer).  This should automatically place it in the classpath and require no special setting of a classpath environment variable.
