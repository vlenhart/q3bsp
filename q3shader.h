#ifndef Q3SHADER_H

#define Q3SHADER_H

//############################################################################
//##                                                                        ##
//##  Q3SHADER.H                                                            ##
//##                                                                        ##
//##  Reads a Quake3 Shader file.                                           ##
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



#include "stringdict.h"
#include "arglist.h"

// storing info about one blending stage 
class ShaderStage 
{
public :

	ShaderStage() : 
	    animMapFrequency(1.0f),
		isLightMap(false),isAnimMap(false),clamp(false),
		textureBlendMode("ADD")
	{ 
	}
		
	StringRef map;
	
	StringRefVector animMap;
	float animMapFrequency;

	// blending function 
	StringRef blendFuncSrc;
	StringRef blendFuncDst;
	StringRef alphaFunc;
	StringRef depthFunc;


	String tcmodOk;	// tcmod tokens known, can be used in VRML 
	String tcmod;	// tcmod tokens 

	String rgbGen;


	bool isLightMap;
	bool isAnimMap;
	bool clamp;

	// texure blend mode in Multi Texturing 
    StringRef textureBlendMode;	  // 

};

typedef std::vector<ShaderStage > ShaderStageVector;

class QuakeShader
{
public:
  QuakeShader(const StringRef &ref)
  {
    mName = ref;
	mTrans = false;
	mNoLightMap = false;
	mSky = false;
	mLightMapStage=0;
  };
  
  const StringRef& GetName(void) const { return mName; };
  
  void AddTexture(const StringRef& ref)
  {
    String ifoo = ref;
    //?if ( ifoo == "chrome_env" ) return;
    //?if ( ifoo == "tinfx" ) return;

    mTextures.push_back(ref);
  };

  bool GetBaseTexture(StringRef &ref)
  {
    if ( !mTextures.size() ) return false;
    ref = mTextures[0];
    return true;
  }
  
  StringRef mCull;	  // cull property : none

  bool		mTrans;		  // transparent

  bool		mNoLightMap; // no light map
  bool		mSky;

  int		mLightMapStage; // the stage for the lightmap	

  String	skyBox;


  // add an stage 
  void AddStage (ShaderStage *stage) 
  {
	 mStages.push_back(*stage);
  }	

  int GetNumStages() const  { return mStages.size(); }
  const ShaderStage& GetStage(int stage) const {return mStages[stage]; }


private:
  StringRef       mName;		// name of shader.
  StringRefVector mTextures;	// list of textures 
  ShaderStageVector mStages;	// list of stages 
};


typedef std::map< StringRef, QuakeShader *> QuakeShaderMap;
typedef std::map< StringRef, bool > QuakeShaderFileMap;

class QuakeShaderFactory : public ArgList
{
public:
	// where shader files are stored 
  //char const shaderDir[] = "scripts/";
  

  QuakeShaderFactory(void);
  ~QuakeShaderFactory(void);

  QuakeShader * Locate(const String &str);
  QuakeShader * Locate(const StringRef &str);

  bool ShaderFileLoaded(const StringRef &str);


  // add a q3 shader file to add all shaders in file to shader list 
  bool AddShader(const StringRef &sname);


  static QuakeShaderFactory &gQuakeShaderFactory(void)
  {
    if ( !gSingleton )
    {
      gSingleton = new QuakeShaderFactory;
    }
    return *gSingleton;
  }

  static void ExplicitDestroy(void)  // explicitely destroy the global intance
  {
    delete gSingleton;
    gSingleton = 0;
  }

  void Process(const StringVector &args);

  /// get stripped down version of the name
  bool GetName(const String& str,char *stripped);

private:

  int          mBraceCount;

  QuakeShader *mCurrent;
  ShaderStage *mCurrentStage;

  void ShaderString(const char *str);


  QuakeShaderMap mShaders; // all shaders available.

  QuakeShaderFileMap mShaderFiles; // all shader files loades 

  static QuakeShaderFactory *gSingleton; // global instance of data
};

#endif
