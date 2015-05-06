                             Q3BSP v1.1

                             Q3BSP v1.0

                                by

                           John W. Ratcliff
                          jratcliff@verant.com

                      Open Sourced December 5, 2000
                           Merry Christmas

                           v1.1 / VRML 97
                                by
                           Holger Grahn
                            hg@x79.net


Q3BSP is a cross-platform console application that will convert any valid
Quake 3 BSP file into an organized polygon mesh and export the results
into a standardized ASCII file format, VRML 1.0.  I encourage users of
this software to expand this tool to export in other popular ASCII file
formats, especially ones which support multiple U/V channels.


This utility will convert a Quake3 BSP into a valid polygon mesh and
output the results into two seperate VRML 1.0 files.  The first VRML
file contains all the U/V mapping and texture mapping information for
channel #1, and the second VRML file will contain all of the U/V
mapping and texture names for the second U/V channel, which contains
all lightmap information.  You can then directly import these files
into any number of 3d editing tools, including 3d Studio Max

This tool also extracts the lightmap data and saves it out as a
series of .PNG files.

Contact Id Software about using Quake 3 data files and Quake 3 editing
tools for commercial software development projects.

This tool will parse Quake 3 shader files to find the base texture name
used.  I have not included the Quake 3 shader files with this build, but
you can copy them into the working directory yourself.

This source code makes heavy use of C++ and STL.  It is mostly OS
neutral and should compile fairly easily on any system.


This project was created with Microsoft Developer Studio 6.0 and all
you should have to do is load the workspace and compile it.

The files included in this project are:


arglist.h         Utility class to parse a string into a series of
arglist.cpp       arguments.

fload.h           Utility class to load a file from disk into memory.
fload.cpp

Makefile          You can use this to compile with the make utility.

main.cpp          Main console application.
main.cpp          Some helper functions for cross-platform compatiblity.

patch.h           Converts a Quake 3 Bezier patch into a set of
patch.cpp         triangles.

plane.h           Simple representation of a plane equation.

q3bsp.h           Class to load a Quake 3 BSP file
q3bsp.cpp

q3bsp.dsp         Dev Studio Project file
q3bsp.dsw         Dev Studio Workspace

q3def.h           Data definitions for Quake 3 data structures.

q3effects.wrl     Some Quake 3 shader effects emulation.

q3shader.h        Utility to parse Quake 3 shader files.
q3shader.cpp

qdefs.h           Misc definitions compatible with Q3.

rect.h            Simple template class to represent an axis aligned
                  bounding region.

stable.h          Simple class to maintain a set of ascii strings with
                  no duplications.

stb_image_write.h Cross-platform image reading and writing.
stb_image.h

stl.h             Includes common STL header files.

stringdict.h      Application global string table.
stringdict.cpp

vector.h          Simple template class to represent a 3d data point.

vformat.h         Class to create an organized mesh from a polygon soup.
vformat.cpp       Also saves output into VRML 1.0 & 2


==================================================================================

VRML 97 extension by Holger Grahn hg@x79.net.

Using q3bsp -2 bspfile
Two VRML 97 files are created, the lightmaps are stored in PNG format

Using q3bsp -2m bspfile
One VRML 97 is created using a MultiTexture NODE extensions, the lightmaps are stored in PNG format
This extensions is supported by blaxxun Contact 5 VRML plugin.

In the directory the file q3effects.wrl
is copied into the output file.
The additional option v puts the shader name as string after each Appearance.


The output was tested the following way :

get Quake3 or Quake3 team arena demo from 
http://www.quake3arena.com/tademo/  
http://www.quake3world.com/demo/teamarena.html

rename demoq3/pak0.pk3 to demoq3/pak0.zip

unzip bsp files, textures (jpg, tga) & shaders with path to some directory
you now have :
 dir/maps
 dir/textures
 dir/scripts

 
 cd dir 

 inside dir
 
 run :  
 q3bsp -2m maps\q3dm1
 
 files are created in dir (wrl, light map PNG's)

 referenced TGA files are automatically converted to PNG if not yet existing
 


ToDo: 

translating of more entities
using q3 bsptree / pvs information
Q3 shaders can have n textures,
check & translate  shader properties like cull, blendMode etc

I was trying to emulate some Q3 shader effects with multi texturing.
Faces without a shader use two textures, light map & texture.

Its still a problem to exactly reproduce q3 brightness.
in v1.1 Vertex color are exported,
a problem might be that the rgbgen keyword is not supported.


Effect shaders can use up to 8 textures, with the current set of boards
this effects are not possible to emulate.
This would require Multipath rendering.
For improvements see q3shader.cpp for shader parsing
and vformat.cpp for shader conversion.

New shader files need to manually insertd in q3shader.cpp


q3links

id Software
===========
http://www.idsoftware.com/
http://www.quake3arena.com/


Tools
=====
Map building
q3map

GtkRandiant
http://www.qeradiant.com/
http://quake3.qeradiant.com/

q3ase Shader file editor 
http://www.fileplanet.com/index.asp?file=56919


Sites
=====

http://www.quake3world.com/
http://www.planetquake.com/
http://www.3ddownloads.com/

Info & Resources for builders
=============================
http://www.quake3world.com/editing/
http://www.planetquake.com/polycount/




 
 