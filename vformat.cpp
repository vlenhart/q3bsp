#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


//############################################################################
//##                                                                        ##
//##  VFORMAT.CPP                                                           ##
//##                                                                        ##
//##  Defines a LightMapVertex, which is a vertex with two U/V channels.    ##
//##  Also defines a class to organize a triangle soup into an ordered mesh ##
//##                                                                        ##
//##  OpenSourced 12/5/2000 by John W. Ratcliff                             ##
//##                                                                        ##
//##  No warranty expressed or implied.                                     ##
//##                                                                        ##
//##  Part of the Q3BSP project, which converts a Quake 3 BSP file into a   ##
//##  polygon mesh.                                                         ##
//############################################################################
//##                                                                        ##
//##  Contact John W. Ratcliff at jratcliff@verant.com                      ##
//############################################################################


#include "q3shader.h"


#include "vformat.h"
#include "stb_image.h"
#include "stb_image_write.h"

// does the file exists ?
bool ExistsFile(const char *path) 
{
	FILE *f = fopen(path,"rb");
	if (!f) 
		return false;
	fclose(f);
	return true;

}

// check basename for texture, and try to return jpg or png file 
void CheckTexture(const char *baseName, String &textureFileName)
{

	const char *ext = strrchr(baseName,'.');

	// check if jpg exists
	textureFileName=baseName;
	if (!ext) {
		textureFileName+=".jpg";

		if (ExistsFile(textureFileName.c_str()))
			return;
	}


	// check if png exists
	textureFileName=baseName;
	if (ext)  // erase ext 
		textureFileName.erase(textureFileName.size()-strlen(ext),strlen(ext));

	textureFileName+=".png";

	if (ExistsFile(textureFileName.c_str()))
		return;

	// check if tga exists
	textureFileName=baseName;
	if (ext)  // erase ext 
		textureFileName.erase(textureFileName.size()-strlen(ext),strlen(ext));

	textureFileName+=".tga";

	if (ExistsFile(textureFileName.c_str())) {
		// convert tga to png
		String outFileName=baseName;
		if (ext)  // erase ext 
			outFileName.erase(outFileName.size()-strlen(ext),strlen(ext));

		outFileName+=".png";

		// bmp.TGAToPNG(textureFileName.c_str(),outFileName.c_str());
		int width, height, comp;
		unsigned char *data = stbi_load(textureFileName.c_str(), &width, &height, &comp	, 0);
		stbi_write_png(outFileName.c_str(), width, height, comp, data, width*comp);
		stbi_image_free(data);

		textureFileName = outFileName;
		return;
	
	}

	// check if jpg exists
	textureFileName=baseName;
	if (ext)  // erase ext 
		textureFileName.erase(textureFileName.size()-strlen(ext),strlen(ext));

	textureFileName+=".jpg";

	if (ExistsFile(textureFileName.c_str()))
		return;

	printf("Texture file not found:'%s'\n",baseName);

}




LightMapVertex VertexLess::mFind;
VertexVector * VertexLess::mList=0;

bool VertexLess::operator()(int v1,int v2) const
{

  const LightMapVertex& a = Get(v1);
  const LightMapVertex& b = Get(v2);

  if ( a.GetX() < b.GetX() ) return true;
  if ( a.GetX() > b.GetX() ) return false;

  if ( a.GetY() < b.GetY() ) return true;
  if ( a.GetY() > b.GetY() ) return false;

  if ( a.GetZ() < b.GetZ() ) return true;
  if ( a.GetZ() > b.GetZ() ) return false;

  if ( a.mTexel1.x < b.mTexel1.x ) return true;
  if ( a.mTexel1.x > b.mTexel1.x ) return false;

  if ( a.mTexel1.y < b.mTexel1.y ) return true;
  if ( a.mTexel1.y > b.mTexel1.y ) return false;

  if ( a.mTexel2.x < b.mTexel2.x ) return true;
  if ( a.mTexel2.x > b.mTexel2.x ) return false;

  if ( a.mTexel2.y < b.mTexel2.y ) return true;
  if ( a.mTexel2.y > b.mTexel2.y ) return false;


  return false;
};


VertexMesh::~VertexMesh(void)
{
  VertexSectionMap::iterator i;
  for (i=mSections.begin(); i!=mSections.end(); ++i)
  {
    VertexSection *section = (*i).second;
    delete section;
  }
}


void VertexMesh::AddTri(const StringRef &name,const LightMapVertex &v1,const LightMapVertex &v2,const LightMapVertex &v3)
{
  VertexSection *section;

  if ( name == mLastName )
  {
    section = mLastSection;
  }
  else
  {
    VertexSectionMap::iterator found;
    found = mSections.find( name );
    if ( found != mSections.end() )
    {
      section = mLastSection = (*found).second;
      mLastName = name;
    }
    else
    {
      mLastSection = section = new VertexSection( name );
      mSections[name] = section;
      mLastName = name;
    }
  }

  assert( section );

  section->AddTri( v1, v2, v3 );

  mBound.MinMax( v1.mPos );
  mBound.MinMax( v2.mPos );
  mBound.MinMax( v3.mPos );

}


void VertexSection::AddTri(const LightMapVertex &v1,
            const LightMapVertex &v2,
            const LightMapVertex &v3)
{
  mBound.MinMax(v1.mPos);
  mBound.MinMax(v2.mPos);
  mBound.MinMax(v3.mPos);

  AddPoint(v1);
  AddPoint(v2);
  AddPoint(v3);

}


void VertexSection::AddPoint(const LightMapVertex &p)
{
  unsigned short idx = (unsigned short)mPoints.GetVertex(p);
  mIndices.push_back(idx);
};


void VertexMesh::SaveVRML2(
			FILE *fph,
            VFormatOptions &options) const 
{
	if ( mSections.size() )
	{
		
		if ( fph )
		{
			//fprintf(fph,"#VRML V2.0 utf8 generated by QBSP \n");
			if (mSections.size() >0) {
				fprintf(fph,"Group {\n");
				fprintf(fph,"children [\n");
			}	
			VertexSectionMap::const_iterator i;
			
			for (i=mSections.begin(); i!=mSections.end(); ++i)
			{
				(*i).second->SaveVRML2(fph,options);
			}
			if (mSections.size() >0) {
				
				fprintf(fph,"\n]\n}\n");
			}
			
		}
	}
}

void VertexMesh::SaveVRML(const String &name,  // base file name
              bool tex1) const           // texture channel 1=(true)
{
  if ( mSections.size() )
  {
    String oname = name+".wrl";
    FILE *fph = fopen(oname.c_str(),"wb");

    if ( fph )
    {
      fprintf(fph,"#VRML V1.0 ascii\n");
      fprintf(fph,"Separator {\n");
      fprintf(fph,"  ShapeHints {\n");
      fprintf(fph,"    shapeType SOLID\n");
      fprintf(fph,"    vertexOrdering COUNTERCLOCKWISE\n");
      fprintf(fph,"    faceType CONVEX\n");
      fprintf(fph,"  }\n");

      VertexSectionMap::const_iterator i;

      for (i=mSections.begin(); i!=mSections.end(); ++i)
      {
        (*i).second->SaveVRML(fph,tex1);
      }

      fprintf(fph,"}\n");

      fclose(fph);
    }
  }
}

void VertexSection::SaveVRML(FILE *fph,bool tex1)
{
  // save it into a VRML file!
  static int itemcount=1;

  fprintf(fph,"DEF item%d Separator {\n",itemcount++);
  fprintf(fph,"Translation { translation 0 0 0 }\n");
  fprintf(fph,"Material {\n");
  fprintf(fph,"  ambientColor 0.1791 0.06536 0.06536\n");
  fprintf(fph,"  diffuseColor 0.5373 0.1961 0.1961\n");
  fprintf(fph,"  specularColor 0.9 0.9 0.9\n");
  fprintf(fph,"  shininess 0.25\n");
  fprintf(fph,"  transparency 0\n");
  fprintf(fph,"}\n");
  fprintf(fph,"Texture2 {\n");


  const char *foo = mName;
  char scratch[256];

  if ( !tex1 )
  {
    while ( *foo && *foo != '+' ) foo++;
    char *dest = scratch;
    if ( *foo == '+' )
    {
      foo++;
      while ( *foo ) *dest++ = *foo++;
    }
    *dest = 0;
  }
  else
  {
    char *dest = scratch;
    while ( *foo && *foo != '+' ) *dest++ = *foo++;
    *dest = 0;
  }

  if ( tex1 )
    fprintf(fph,"  filename %c%s.tga%c\n",0x22,scratch,0x22);
  else
    fprintf(fph,"  filename %c%s.bmp%c\n",0x22,scratch,0x22);

  fprintf(fph,"}\n");

  mPoints.SaveVRML(fph,tex1);

  int tcount = mIndices.size()/3;

  if ( 1 )
  {
    fprintf(fph,"  IndexedFaceSet {\ncoordIndex [\n");
    UShortVector::iterator j= mIndices.begin();
    for (int i=0; i<tcount; i++)
    {
      int i1 = *j;
      j++;
      int i2 = *j;
      j++;
      int i3 = *j;
      j++;
      if ( i == (tcount-1) )
        fprintf(fph,"  %d, %d, %d, -1]\n",i1,i2,i3);
      else
        fprintf(fph,"  %d, %d, %d, -1,\n",i1,i2,i3);
    }
  }

  if ( 1 )
  {
    fprintf(fph,"  textureCoordIndex [\n");
    UShortVector::iterator j= mIndices.begin();
    for (int i=0; i<tcount; i++)
    {
      int i1 = *j;
      j++;
      int i2 = *j;
      j++;
      int i3 = *j;
      j++;
      if ( i == (tcount-1) )
        fprintf(fph,"  %d, %d, %d, -1]\n",i1,i2,i3);
      else
        fprintf(fph,"  %d, %d, %d, -1,\n",i1,i2,i3);
    }
  }
  fprintf(fph,"  }\n");
  fprintf(fph,"}\n");
};


void VertexPool::SaveVRML(FILE *fph,bool tex1)
{
  if ( 1 )
  {
    fprintf(fph,"  Coordinate3 {\npoint [\n");
    int count = mVtxs.size();
    for (int i=0; i<count; i++)
    {
      const LightMapVertex &vtx = mVtxs[i];
      if ( i == (count-1) )
        fprintf(fph,"  %f %f %f\n]\n",vtx.mPos.x,vtx.mPos.y,vtx.mPos.z);
      else
        fprintf(fph,"  %f %f %f,\n",vtx.mPos.x,vtx.mPos.y,vtx.mPos.z);
    }
    fprintf(fph,"   }\n");
  }
  if ( 1 )
  {
    fprintf(fph,"  TextureCoordinate2 {\npoint [\n");
    int count = mVtxs.size();

    for (int i=0; i<count; i++)
    {
      const LightMapVertex &vtx = mVtxs[i];

      if ( !tex1 ) // if saving second U/V channel.
      {
        if ( i == (count-1) )
          fprintf(fph,"  %f %f\n]\n",vtx.mTexel2.x,1.0f-vtx.mTexel2.y);
        else
          fprintf(fph,"  %f %f,\n",vtx.mTexel2.x,1.0f-vtx.mTexel2.y);
      }
      else
      {
        if ( i == (count-1) )
          fprintf(fph,"  %f %f\n]\n",vtx.mTexel1.x,1.0f-vtx.mTexel1.y);
        else
          fprintf(fph,"  %f %f,\n",vtx.mTexel1.x,1.0f-vtx.mTexel1.y);
      }
    }

    fprintf(fph,"   }\n");
  }
}

// write a single VRML ImageTexture node

void WriteImageTexture(FILE *fph,const StringRef &name,
					   VFormatOptions &options,bool lightmap, bool clamp= false)
{

  // texture node already defined ?
	  TextureDefMap::iterator found;
	  found = options.textureDefMap.find(name);
	  if ( found != options.textureDefMap.end()) { // simply use it 
		  fprintf(fph,"USE %s\n",(const char *)(*found).second );
	  } 
	  else 
	  { // need to define new imageTexture node 

		  char buf[60];
		  sprintf(buf,"_T%d",options.textureCount);
		  options.textureCount++;

		  fprintf(fph,"DEF %s ImageTexture {\n",(const char *)buf);

		  if (clamp) fprintf(fph,"\trepeatS FALSE repeatT FALSE\n");		  
		  const char *ext="bmp";
		  
		  if (options.usePng) 
			  ext = "png";
		  
		  if (!lightmap) {
				String textureFileName;
				// check and return jpg, png ..
				CheckTexture(name, textureFileName);
				fprintf(fph,"  url %c%s%c\n",0x22,textureFileName.c_str(),0x22);
		  } else
			  fprintf(fph,"  url %c%s.%s%c\n",0x22,name.Get(),ext,0x22);
		  
		  fprintf(fph,"}\n");
		  
		  // insert new node into map 
		  options.textureDefMap.insert(TextureDefMap::value_type(name,buf));
		  
	  }
}

// save it into a VRML 2 file

void VertexSection::SaveVRML2(FILE *fph,VFormatOptions &options)
{

  static int itemcount=1;

  bool hasLightMap = true;
  int numStages=2;
  int lightMapStage = 0; // place lightmap coord into this texture stage 
  bool vertexColor = true;

  //lightMapStage=1;

  // name of lightmap texture 
  StringRef lightMap;


  // parse mName into base texture + light map

  const char *foo = mName;
  char scratch[256];

  char *dest = scratch;
  while ( *foo && *foo != '+' ) *dest++ = *foo++;
  if (*foo == '+' && foo[1]) {
	  lightMap = foo+1;
	  hasLightMap = true;
  }	
  else  { 
	  hasLightMap = false;
	  numStages = 1;
	  lightMapStage = -1;
  }	
  *dest = 0;

  StringRef name(scratch);

  if (mShader) {
	  numStages =mShader->GetNumStages();
	  if (mShader->mNoLightMap) {
		  hasLightMap = false;
		  lightMapStage = -1;
	  }	else {
		  lightMapStage=mShader->mLightMapStage;
	  }	
	 //if (rgbGen == Identity)
	 //	 vertexColor = false;
  }	else {

  }	

  //fprintf(fph,"DEF item%d Shape {\n",itemcount++);
  fprintf(fph,"Shape {\n");
  fprintf(fph,"appearance ");


  // appearance node already defined ?
  AppearanceDefMap::iterator found;
  found = options.appearanceDefMap.find(mName);
  if ( found != options.appearanceDefMap.end()) { // simply use it 
	  //found.r
	  fprintf(fph,"USE %s\n",(const char *)(*found).second);
  } 
  else 
  {  // need to define new Appearance node 
	  if (options.verbose) {
		fprintf(fph," #%s\n",(const char *) mName);
		if (mShader) {
			  fprintf(fph," #shader %s\n",(const char *) mShader->GetName());
		}		
	  }	
	  
	  fprintf(fph,"DEF A%d Appearance {\n",options.appearanceCount);
	  char buf[60];
	  sprintf(buf,"_A%d",options.appearanceCount);

	  // insert new node into map 
	  options.textureDefMap.insert(AppearanceDefMap::value_type(StringRef(mName),StringRef(buf)));
	  
	  options.appearanceCount++;
		  
		  if (options.useLighting) {
			  fprintf(fph,"material Material {\n");
			  fprintf(fph,"  ambientIntensity 0.06536 \n");
			  fprintf(fph,"  diffuseColor 0.5373 0.1961 0.1961\n");
			  fprintf(fph,"  specularColor 0.9 0.9 0.9\n");
			  fprintf(fph,"  shininess 0.25\n");
			  //fprintf(fph,"  transparency 0\n");
			  fprintf(fph,"}\n");
		  }
		  else {  
		  if (options.useMat) {
			  if (!options.matDefined) {
				fprintf(fph,"material DEF MAT Material {\n");
				//fprintf(fph,"  diffuseColor 0 0 0\n");
				fprintf(fph,"  diffuseColor 1 1 1\n");
				//fprintf(fph,"  emissiveColor 0.5 0.5 0.5\n"); // overall brightness 
				fprintf(fph,"  emissiveColor 0.2 0.2 0.2\n"); // overall brightness 
				fprintf(fph,"}\n");
				options.matDefined = true;
			  } else 	
				fprintf(fph,"material USE MAT \n");

		  }
		  }
		  
		  
		  fprintf(fph,"texture ");
		  
		  bool tex1 = options.tex1;
		
		  
		  if (options.useMultiTexturing) {
			  fprintf(fph,"MultiTexture {\nmaterialColor TRUE texture [\n");
			  tex1 = true;
		
		  if (mShader) {
		    hasLightMap = false;
			numStages =mShader->GetNumStages();
			for (int i=0; i<numStages; i++) {
				
				//if (i>0 && numStages >2) // we can do only 2 stages 
				//	i=numStages-1;

				const ShaderStage& stage = mShader->GetStage(i);

				if (stage.isAnimMap) {
					if (options.useEffects) {
						fprintf(fph,"DEF MAP AnimMap {\nfrequency %f textures [\n",stage.animMapFrequency);
						for (int i=0; i<stage.animMap.size(); i++)
							WriteImageTexture(fph,stage.animMap[i],options,false,stage.clamp);

						//fprintf(fph,"] ROUTE TIMER.fraction_changed TO MAP.set_fraction \n");
					    fprintf(fph,"]ROUTE TIMER.time_changed TO MAP.set_time\n");

						fprintf(fph,"}");
					}else
						WriteImageTexture(fph,stage.animMap[0],options,false,stage.clamp);


				} else 
				if (stage.isLightMap) {
					WriteImageTexture(fph,lightMap,options,true,stage.clamp);
					hasLightMap = true;
					mShader->mLightMapStage=lightMapStage=i;
				} else 
					WriteImageTexture(fph,stage.map,options,false,stage.clamp);
			}
		  }	
		  else 
		  { // no shader
			//WriteImageTexture(fph,name,options,false);
			if (options.useMultiTexturing) {
				if (hasLightMap) {
					WriteImageTexture(fph,lightMap,options,true,false);
				}
			}
			WriteImageTexture(fph,name,options,false);

		  }	
		  } 
		  else {
			  // not useMultiTexturing
			  if (tex1) {
					if (mShader) 
					    mShader ->GetBaseTexture(name);

					WriteImageTexture(fph,name,options,false);

			  } else {	
				if (hasLightMap) {
					WriteImageTexture(fph,lightMap,options,true,false);
				}
				else WriteImageTexture(fph,"nomap",options,true,false);
			  }	

		  }	
		  if (options.useMultiTexturing) {
			  fprintf(fph,"]");
			  // add makes it to bright
			  // with modulate currently to dark
			  //if (numStages>1) 
			  if (mShader) {
				fprintf(fph,"mode [ ");
				bool hasTcMod = false;

				for (int i=0; i<numStages; i++) {
				
					//if (i>0 && numStages >2) // we can do only 2 stages 
					//	i=numStages-1;
					const ShaderStage& stage = mShader->GetStage(i);
					const char *mode = stage.textureBlendMode;
					
					if (stage.tcmod.length()>0) hasTcMod = true;
					if (stage.tcmodOk.length()>0) hasTcMod = true;

					if (i==0 && stage.textureBlendMode == String("ADD") && !stage.isLightMap)
						mode = "MODULATE";

					if (!mode) {
						if (i==0 && !options.useMat) 
							mode = "REPLACE";
					}	

					fprintf(fph,"\"%s\" ", mode ? mode : "ADD");
					fprintf(fph,"# blend %s %s \n",(const char *) stage.blendFuncSrc,(const char *) stage.blendFuncDst);

				}
				fprintf(fph,"]\n");

				// tcmod 
				if (hasTcMod) {
				fprintf(fph,"textureTransform [ ");

				for (int i=0; i<numStages; i++) {
				
					//if (i>0 && numStages >2) // we can do only 2 stages 
					//	i=numStages-1;
					const ShaderStage& stage = mShader->GetStage(i);
					const char *mode = stage.tcmod.c_str();
					const char *modeOk = stage.tcmodOk.c_str();

					if (stage.tcmod.length()>0 || stage.tcmodOk.length()>0 ) {
					   fprintf(fph,"DEF TC%d TcMod {\n",i);

					   if (stage.tcmodOk.length()>0)
						   fprintf(fph,"\t%s\n", modeOk);

					   if (stage.tcmod.length()>0) 
						   fprintf(fph,"\tmode \"%s\" \n",mode ? mode : "" );

					   //fprintf(fph,"ROUTE TIMER.fraction_changed TO TC%d.set_fraction \n",i);
					   fprintf(fph,"ROUTE TIMER.time_changed TO TC%d.set_time\n",i);
					   fprintf(fph,"}\n");
					}
					else  fprintf(fph,"NULL\n");

				}
				fprintf(fph,"]\n");
				}

			  } 
			  else {
				  if (hasLightMap)
					  fprintf(fph, "%s", options.blendMode);
				  else ; // MODULATE 
			  }					
			  fprintf(fph,"}");
		  }	// multi texture 
		  
		  fprintf(fph,"}\n");
  }	

  int tcount = mIndices.size()/3;

  // write the indexed face set 
  if ( 1 )
  {
    fprintf(fph,"geometry  IndexedFaceSet {\n");

    fprintf(fph,"\tccw FALSE creaseAngle 3.14\n");

	if (mShader && (mShader->mCull == StringRef("none") ||  mShader->mCull == StringRef("disable")))  {
	    fprintf(fph,"\tsolid FALSE\n");
	}
	if (mShader && (mShader->mCull == StringRef("back")))  {
	    fprintf(fph,"\tccw TRUE\n");
	}
    fprintf(fph,"\tcoordIndex [\n");

    UShortVector::iterator j= mIndices.begin();
    for (int i=0; i<tcount; i++)
    {
      int i1 = *j;
      j++;
      int i2 = *j;
      j++;
      int i3 = *j;
      j++;
      if ( i == (tcount-1) )
        fprintf(fph,"\t%d,%d,%d,-1]\n",i1,i2,i3);
      else
        fprintf(fph,"\t%d,%d,%d,-1,\n",i1,i2,i3);

    }
  }

  if ( 0 ) // if texCoord index == ccordIndex no need to export
  {
    fprintf(fph,"  textureCoordIndex [\n");
    UShortVector::iterator j= mIndices.begin();
    for (int i=0; i<tcount; i++)
    {
      int i1 = *j;
      j++;
      int i2 = *j;
      j++;
      int i3 = *j;
      j++;
      if ( i == (tcount-1) )
        fprintf(fph,"\t%d,%d,%d,-1]\n",i1,i2,i3);
      else
        fprintf(fph,"\t%d,%d,%d,-1,\n",i1,i2,i3);
    }
  }

  mPoints.SaveVRML2(fph,lightMapStage,options);

  fprintf(fph,"  }\n");
  fprintf(fph,"}\n");
};


void VertexPool::SaveVRML2(FILE *fph, int lightMapStage, VFormatOptions &options)
{
  
  if ( 1 )
  {
    fprintf(fph,"coord Coordinate {\npoint [\n\t");
    int count = mVtxs.size();
    for (int i=0; i<count; i++)
    {
      const LightMapVertex &vtx = mVtxs[i];
	  
	  if (i>0) { 
			fprintf(fph,",\n\t");
	  }		
	   
      if (options.yzFlip)
		  fprintf(fph,options.VFORMAT,vtx.mPos.x,vtx.mPos.z,vtx.mPos.y);
      else fprintf(fph,options.VFORMAT,vtx.mPos.x,vtx.mPos.y,vtx.mPos.z);

    }
    fprintf(fph,"\n]\n}\n");
  }

  if (options.useMultiTexturing) { // PROPOSAL MultiTextureCoodinate 
    fprintf(fph,"texCoord  MultiTextureCoordinate {\ncoord [\n\t");
    int count = mVtxs.size();

	// channel 0
    fprintf(fph,"\tTextureCoordinate {\npoint [\n\t");


    for (int i=0; i<count; i++)
    {
      const LightMapVertex &vtx = mVtxs[i];

	  if (i>0) { 
			if ( (i%4) == 0) fprintf(fph,",\n\t");
			else fprintf(fph,",");
	  }		
      if (lightMapStage == 0)
		  fprintf(fph,options.TFORMAT,vtx.mTexel2.x,1.0f-vtx.mTexel2.y);
      else fprintf(fph,options.TFORMAT,vtx.mTexel1.x,1.0f-vtx.mTexel1.y);

    }
    fprintf(fph,"\n]\n}\n");
	
	//if (lightMapStage == 1) 
	{

	// channel 1
	fprintf(fph,"\tTextureCoordinate {\npoint [\n\t");

    for (int i=0; i<count; i++)
    {
      const LightMapVertex &vtx = mVtxs[i];

	  if (i>0) { 
			if ( (i%4) == 0) fprintf(fph,",\n\t");
			else fprintf(fph,",");
	  }		
      if (lightMapStage == 1)
		  fprintf(fph,options.TFORMAT,vtx.mTexel2.x,1.0f-vtx.mTexel2.y);
      else fprintf(fph,options.TFORMAT,vtx.mTexel1.x,1.0f-vtx.mTexel1.y);
	}
    fprintf(fph,"\n\t]\n}\n");
	
	}
    fprintf(fph,"\n]\n}\n");


  } else	
  if ( options.noTextureCoordinates )
  {
    fprintf(fph,"texCoord  TextureCoordinate {\npoint [\n\t");
    int count = mVtxs.size();

    for (int i=0; i<count; i++)
    {
      const LightMapVertex &vtx = mVtxs[i];

	  if (i>0) {
			if ( (i%4) == 0) fprintf(fph,",\n\t");
			else fprintf(fph,",");
	  }		


      if ( !options.tex1 ) // if saving second U/V channel.
      {
         fprintf(fph,options.TFORMAT,vtx.mTexel2.x,1.0f-vtx.mTexel2.y);
      }
      else
      {
          fprintf(fph,options.TFORMAT,vtx.mTexel1.x,1.0f-vtx.mTexel1.y);
      }
    }
    fprintf(fph,"\n]\n}\n");

  }

   //if (options.useVertexColor) 
   {
    int count = mVtxs.size();
	// channel 0
    fprintf(fph,"\tcolor Color {\ncolor [\n\t");


    for (int i=0; i<count; i++)
    {
      const LightMapVertex &vtx = mVtxs[i];

	  if (i>0) { 
			if ( (i%4) == 0) fprintf(fph,",\n\t");
			else fprintf(fph,",");
	  }		
      fprintf(fph,options.CFORMAT,vtx.mColor.x,vtx.mColor.y,vtx.mColor.z);

    }
    fprintf(fph,"\n]\n}\n");
   }
}
