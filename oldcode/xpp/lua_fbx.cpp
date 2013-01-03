/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"

#ifndef WET_DISABLE_FBX


using namespace FBXSDK_NAMESPACE;

static KFbxSdkManager *pfbx;




#define UPVALUE_LIB 1
#define UPVALUE_PTR 2

void lua_fbx_tab_openlib (lua_State *l, int upvalues);


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Base helper funcs 
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

KFbxImporter::EFileFormat GetFileFormat(const char* pFilename)
{
	KFbxImporter::EFileFormat lFileFormat = KFbxImporter::eFBX_BINARY;
	KString lFileName(pFilename);
	int     lIndex;

	// Find the '.' separating the file extension
	if ((lIndex = lFileName.ReverseFind('.')) > -1)
	{
		// extension found
		KString ext = lFileName.Right(lFileName.GetLen() - lIndex - 1);
		if (ext.CompareNoCase("obj") == 0)
		{
			lFileFormat = KFbxImporter::eALIAS_OBJ;
		}
		else
		if (ext.CompareNoCase("3ds") == 0)
		{
			lFileFormat = KFbxImporter::e3D_STUDIO_3DS;
		}
		else
		if (ext.CompareNoCase("dxf") == 0)
		{
			lFileFormat = KFbxImporter::eAUTOCAD_DXF;
		}
#ifdef ARUBA
		else
		if (ext.CompareNoCase("apf") == 0)
		{
			lFileFormat = KFbxImporter::eALIAS_APF;
		}
#endif
	}

	return lFileFormat;
}









bool LoadScene(KFbxSdkManager* pSdkManager, KFbxScene* pScene, const char* pFilename)
{
	int lMajor, lMinor, lRevision;
	int i, lTakeCount;
	KString lCurrentTakeName;
	bool lStatus;
//	char lPassword[1024];

	// Create an importer.
    KFbxImporter* lImporter = pSdkManager->CreateKFbxImporter();
	lImporter->SetFileFormat(GetFileFormat(pFilename));

    // Initialize the importer by providing a filename.
	if(lImporter->Initialize(pFilename) == false)
	{
		printf("Call to KFbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetLastErrorString());

		if (lImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_YET ||
			lImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_ANYMORE)
		{
			KFbxIO::GetCurrentVersion(lMajor, lMinor, lRevision);
			printf("FBX version number for this version of the FBX SDK is %d.%d.%d\n", lMajor, lMinor, lRevision);

			lImporter->GetFileVersion(lMajor, lMinor, lRevision);
			printf("FBX version number for file %s is %d.%d.%d\n\n", pFilename, lMajor, lMinor, lRevision);
		}

		return false;
	}

	KFbxIO::GetCurrentVersion(lMajor, lMinor, lRevision);
//	printf("FBX version number for this version of the FBX SDK is %d.%d.%d\n", lMajor, lMinor, lRevision);

	bool isFbx = lImporter->GetFileFormat() == KFbxImporter::eFBX_ENCRYPTED ||
			     lImporter->GetFileFormat() == KFbxImporter::eFBX_ASCII ||
				 lImporter->GetFileFormat() == KFbxImporter::eFBX_BINARY;

	if (isFbx)
	{
		lImporter->GetFileVersion(lMajor, lMinor, lRevision);
//		printf("FBX version number for file %s is %d.%d.%d\n\n", pFilename, lMajor, lMinor, lRevision);

		// From this point, it is possible to access take information without
		// the expense of loading the entire file.

//		printf("Take Information\n");

		lTakeCount = lImporter->GetTakeCount();

//		printf("    Number of takes: %d\n", lTakeCount);
//		printf("    Current take: \"%s\"\n", lImporter->GetCurrentTakeName());
//		printf("\n");

		for(i = 0; i < lTakeCount; i++)
		{
			KFbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

//			printf("    Take %d\n", i);
//			printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
//			printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

			// Change the value of the import name if the take should be imported 
			// under a different name.
//			printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

			// Set the value of the import state to false if the take should be not
			// be imported. 
//			printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
//			printf("\n");
		}

		// Set the import states. By default, the import states are always set to 
		// true. The code below shows how to change these states.
		lImporter->SetState(KFbxImporter::eIMPORT_MATERIAL, true);
		lImporter->SetState(KFbxImporter::eIMPORT_TEXTURE, true);
		lImporter->SetState(KFbxImporter::eIMPORT_LINK, true);
		lImporter->SetState(KFbxImporter::eIMPORT_SHAPE, true);
		lImporter->SetState(KFbxImporter::eIMPORT_GOBO, true);
		lImporter->SetState(KFbxImporter::eIMPORT_ANIMATION, true);
		lImporter->SetState(KFbxImporter::eIMPORT_GLOBAL_SETTINGS, true);
	}

	// Import the scene.
	lStatus = lImporter->Import(*pScene);

/*
	if(lStatus == false && lImporter->GetLastErrorID() == KFbxIO::ePASSWORD_ERROR)
	{
		printf("Please enter password: ");
		
		lPassword[0] = '\0';

		scanf("%s", lPassword);
		
		lImporter->SetPassword(lPassword);

		lStatus = lImporter->Import(*pScene);

		if(lStatus == false && lImporter->GetLastErrorID() == KFbxIO::ePASSWORD_ERROR)
		{
			printf("\nPassword is wrong, import aborted.\n");
		}
	}
*/

	// Destroy the importer.
	pSdkManager->DestroyKFbxImporter(lImporter);	

	return lStatus;
}




class FBXSDK_NAMESPACE::KFbxVector2;
class FBXSDK_NAMESPACE::KFbxVector4;
class FBXSDK_NAMESPACE::KFbxColor;

void DisplayString(char* pHeader, char* pValue  = "", char* pSuffix  = "");
void DisplayBool(char* pHeader, bool pValue, char* pSuffix  = "");
void DisplayInt(char* pHeader, int pValue, char* pSuffix  = "");
void DisplayDouble(char* pHeader, double pValue, char* pSuffix  = "");
void Display2DVector(char* pHeader, FBXSDK_NAMESPACE::KFbxVector2 pValue, char* pSuffix  = "");
void Display3DVector(char* pHeader, FBXSDK_NAMESPACE::KFbxVector4 pValue, char* pSuffix  = "");
void DisplayColor(char* pHeader, FBXSDK_NAMESPACE::KFbxColor pValue, char* pSuffix  = "");


void DisplayString(char* pHeader, char* pValue /* = "" */, char* pSuffix /* = "" */)
{
	KString lString;

	lString = pHeader;
	lString += pValue;
	lString += pSuffix;
	lString += "\n";
	printf(lString);
}


void DisplayBool(char* pHeader, bool pValue, char* pSuffix /* = "" */)
{
	KString lString;

	lString = pHeader;
	lString += pValue ? "true" : "false";
	lString += pSuffix;
	lString += "\n";
	printf(lString);
}


void DisplayInt(char* pHeader, int pValue, char* pSuffix /* = "" */)
{
	KString lString;

	lString = pHeader;
	lString += pValue;
	lString += pSuffix;
	lString += "\n";
	printf(lString);
}


void DisplayDouble(char* pHeader, double pValue, char* pSuffix /* = "" */)
{
	KString lString;
	KString lFloatValue = (float) pValue;
	
	lFloatValue = pValue <= -HUGE_VAL ? "-INFINITY" : lFloatValue.Buffer();
	lFloatValue = pValue >=  HUGE_VAL ?  "INFINITY" : lFloatValue.Buffer();

	lString = pHeader;
	lString += lFloatValue;
	lString += pSuffix;
	lString += "\n";
	printf(lString);
}

void lua_push_fbx_v2(lua_State *l, KFbxVector2 pValue)
{
	lua_newtable(l);
	lua_pushliteral(l,"x");	lua_pushnumber(l,pValue[0]);	lua_rawset(l,-3);
	lua_pushliteral(l,"y");	lua_pushnumber(l,pValue[1]);	lua_rawset(l,-3);
}

void Display2DVector(char* pHeader, KFbxVector2 pValue, char* pSuffix  /* = "" */)
{
	KString lString;
	KString lFloatValue1 = (float)pValue[0];
	KString lFloatValue2 = (float)pValue[1];

	lFloatValue1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
	lFloatValue1 = pValue[0] >=  HUGE_VAL ?  "INFINITY" : lFloatValue1.Buffer();
	lFloatValue2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
	lFloatValue2 = pValue[1] >=  HUGE_VAL ?  "INFINITY" : lFloatValue2.Buffer();

	lString = pHeader;
	lString += lFloatValue1;
	lString += ", ";
	lString += lFloatValue2;
	lString += pSuffix;
	lString += "\n";
	printf(lString);
}

void lua_push_fbx_v3(lua_State *l, KFbxVector4 pValue)
{
	lua_newtable(l);
	lua_pushliteral(l,"x");	lua_pushnumber(l,pValue[0]);	lua_rawset(l,-3);
	lua_pushliteral(l,"y");	lua_pushnumber(l,pValue[1]);	lua_rawset(l,-3);
	lua_pushliteral(l,"z");	lua_pushnumber(l,pValue[2]);	lua_rawset(l,-3);
}

void Display3DVector(char* pHeader, KFbxVector4 pValue, char* pSuffix /* = "" */)
{
	KString lString;
	KString lFloatValue1 = (float)pValue[0];
	KString lFloatValue2 = (float)pValue[1];
	KString lFloatValue3 = (float)pValue[2];

	lFloatValue1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
	lFloatValue1 = pValue[0] >=  HUGE_VAL ?  "INFINITY" : lFloatValue1.Buffer();
	lFloatValue2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
	lFloatValue2 = pValue[1] >=  HUGE_VAL ?  "INFINITY" : lFloatValue2.Buffer();
	lFloatValue3 = pValue[2] <= -HUGE_VAL ? "-INFINITY" : lFloatValue3.Buffer();
	lFloatValue3 = pValue[2] >=  HUGE_VAL ?  "INFINITY" : lFloatValue3.Buffer();

	lString = pHeader;
	lString += lFloatValue1;
	lString += ", ";
	lString += lFloatValue2;
	lString += ", ";
	lString += lFloatValue3;
	lString += pSuffix;
	lString += "\n";
	printf(lString);
}

void lua_push_fbx_color(lua_State *l, KFbxColor pValue)
{
	lua_newtable(l);
	lua_pushliteral(l,"red");	lua_pushnumber(l,pValue.mRed);		lua_rawset(l,-3);
	lua_pushliteral(l,"green");	lua_pushnumber(l,pValue.mGreen);	lua_rawset(l,-3);
	lua_pushliteral(l,"blue");	lua_pushnumber(l,pValue.mBlue);		lua_rawset(l,-3);
}


void DisplayColor(char* pHeader, KFbxColor pValue, char* pSuffix /* = "" */)
{
	KString lString;

	lString = pHeader;
	lString += (float) pValue.mRed;
	lString += " (red), ";
	lString += (float) pValue.mGreen;
	lString += " (green), ";
	lString += (float) pValue.mBlue;
	lString += " (blue)";
	lString += pSuffix;
	lString += "\n";
	printf(lString);
}






/*
int lua_fbx_getGlobalCameraSettings(KFbxGlobalCameraSettings* pGlobalCameraSettings)
{
    DisplayString("Default Camera: ", pGlobalCameraSettings->GetDefaultCamera());
  
	char* lDefaultViewingModes[] = { "Standard", "X-Ray", "Models Only" };
	
	DisplayString("Default Viewing Mode: ", lDefaultViewingModes[pGlobalCameraSettings->GetDefaultViewingMode()]);

	DisplayCamera(&pGlobalCameraSettings->GetCameraProducerPerspective(), PRODUCER_PERSPECTIVE);
	DisplayCamera(&pGlobalCameraSettings->GetCameraProducerTop(), PRODUCER_TOP);
	DisplayCamera(&pGlobalCameraSettings->GetCameraProducerFront(), PRODUCER_FRONT);
	DisplayCamera(&pGlobalCameraSettings->GetCameraProducerRight(), PRODUCER_RIGHT);

	DisplayString("");
}



*/










//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_fbx_ptr_name="fbx*ptr";


// the data pointer we are using

typedef KFbxScene * part_ptr ;

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get userdata from upvalue, no need to test for type
// just error on null
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_fbx_get_ptr (lua_State *l)
{
part_ptr p;

	p=(part_ptr )(*(void **)lua_touserdata(l,lua_upvalueindex(UPVALUE_PTR)));

	if (p == NULL)
	{
		luaL_error(l, "null pointer in fbx usedata" );
	}

	return p;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// alloc an item, returns table that you can modify and associate extra data with
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_fbx_create (lua_State *l)
{
part_ptr *p;
const char *s;

int idx_ud;

	p = (part_ptr *)lua_newuserdata(l, sizeof(part_ptr));
	
	idx_ud=lua_gettop(l);

	(*p)=0;

	luaL_getmetatable(l, lua_fbx_ptr_name);
	lua_setmetatable(l, -2);

	lua_newtable(l);

// main lib and userdata are stored as upvalues in the function calls for easy/fast access

	lua_pushvalue(l, lua_upvalueindex(UPVALUE_LIB) ); // get our base table
	lua_pushvalue(l, idx_ud ); // get our userdata,

	lua_fbx_tab_openlib(l,2);

// remember the userdata in the table as well as the upvalue

	lua_pushstring(l, lua_fbx_ptr_name );
	lua_pushvalue(l, idx_ud ); // get our userdata,
	lua_rawset(l,-3);


	(*p)=pfbx->CreateKFbxScene();

	if(*p) // allocated data, fill in lua special stuff
	{
		if(lua_isstring(l,1))
		{
			s=lua_tostring(l,1);
			LoadScene(pfbx,(*p),s);
		}

//		(*p)->lua_back_ptr=p;

// force this object to persist as long as this library is open
// unref can be used to allow this item to get GCd later otherwise it lives until the lib is closed

//		lua_pushlightuserdata(l, *p);
//		lua_pushvalue(l,-2);
//		lua_rawset(l, lua_upvalueindex(UPVALUE_LIB));

	}

	lua_remove(l, idx_ud );
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_fbx_destroy_ptr (lua_State *l)
{
part_ptr *p;
	
	p = (part_ptr *)luaL_checkudata(l, 1, lua_fbx_ptr_name);

	if(*p)
	{
		if(pfbx)
		{
//			delete (p);
		}
	}
	(*p)=0;

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// delete the pointer data and set pointer to 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_fbx_destroy (lua_State *l)
{
part_ptr *p;
	
	p = (part_ptr *)luaL_checkudata(l, lua_upvalueindex(UPVALUE_PTR), lua_fbx_ptr_name);

	if(*p)
	{
		if(pfbx)
		{
//			delete (p);
		}
	}
	(*p)=0;

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// delete the global keep alive reference to this object
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
//int lua_fbx_unref (lua_State *l)
//{
//part_ptr p;
//
//	p=lua_fbx_get_ptr(l);
//
//	lua_pushlightuserdata(l, p);
//	lua_pushnil(l);
//	lua_rawset(l, lua_upvalueindex(UPVALUE_LIB));
//
//	return 0;
//}




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get/set GlobalLightSettings , pass table to set, returns table of current values
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

int lua_fbx_GlobalLightSettings(lua_State *l)
{
part_ptr p;
//int i, lCount;
KFbxGlobalLightSettings* pGlobalLightSettings;


	p=lua_fbx_get_ptr(l);

	pGlobalLightSettings = &p->GetGlobalLightSettings();


	lua_newtable(l);

	lua_pushliteral(l,"AmbientColor");			lua_push_fbx_color(l,pGlobalLightSettings->GetAmbientColor());
	lua_rawset(l,-3);

/*
    DisplayColor("Ambient Color: ", pGlobalLightSettings->GetAmbientColor());
	DisplayString("Fog Options");
	DisplayBool("    Fog Enable: ", pGlobalLightSettings->GetFogEnable());

	char* lFogModes[] = { "Linear", "Exponential", "Squareroot Exponential" };
    
	DisplayString("    Fog Mode: ", lFogModes[pGlobalLightSettings->GetFogMode()]);

	if (pGlobalLightSettings->GetFogMode() == KFbxGlobalLightSettings::eLINEAR)
	{
		DisplayDouble("    Fog Start: ", pGlobalLightSettings->GetFogStart());
		DisplayDouble("    Fog End: ", pGlobalLightSettings->GetFogEnd());
	}
	else
	{
		DisplayDouble("    Fog Density: ", pGlobalLightSettings->GetFogDensity());
	}

	DisplayColor("    Fog Color: ", pGlobalLightSettings->GetFogColor());

    lCount = pGlobalLightSettings->GetShadowPlaneCount();

	if (lCount)
	{
		DisplayString("    Shadow Planes");
		DisplayBool("        Enable: ", pGlobalLightSettings->GetShadowEnable());
		DisplayDouble("        Intensity: ", pGlobalLightSettings->GetShadowIntensity());

		for (i = 0; i < lCount; i++)
		{
	        DisplayInt("        Shadow Plane ", i);
			DisplayBool("            Enable: ", pGlobalLightSettings->GetShadowPlane(i)->mEnable);
			Display3DVector("            Origin: ", pGlobalLightSettings->GetShadowPlane(i)->mOrigin);
			Display3DVector("            Normal: ", pGlobalLightSettings->GetShadowPlane(i)->mNormal);
        }
	}

	DisplayString("");
*/

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get/set GlobalTimeSettings , pass table to set, returns table of current values
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

int lua_fbx_GlobalTimeSettings(lua_State *l)
{
part_ptr p=lua_fbx_get_ptr(l);
KFbxGlobalTimeSettings* pGlobalTimeSettings;

	pGlobalTimeSettings = &p->GetGlobalTimeSettings();


	lua_newtable(l);


	char lTimeString[256];
	int i, lCount;

	char* lTimeModes[] = { "Default Mode", "Cinema", "PAL", "Frames 30", 
			                  "NTSC Drop Frame", "Frames 50", "Frames 60",
							  "Frames 100", "Frames 120", "NTSC Full Frame", 
							  "Frames 30 Drop", "Frames 1000" }; 


//    DisplayString("Time Mode: ", lTimeModes[pGlobalTimeSettings->GetTimeMode()]);

	lua_pushliteral(l,"TimeMode");			lua_pushstring(l,lTimeModes[pGlobalTimeSettings->GetTimeMode()]);
	lua_rawset(l,-3);


	char* lTimeProtocols[] = { "SMPTE", "Frame", "Default Protocol" };

//	DisplayString("Time Protocol: ", lTimeProtocols[pGlobalTimeSettings->GetTimeProtocol()]);
	lua_pushliteral(l,"TimeProtocol");			lua_pushstring(l,lTimeProtocols[pGlobalTimeSettings->GetTimeProtocol()]);
	lua_rawset(l,-3);

//	DisplayBool("Snap On Frame: ", pGlobalTimeSettings->GetSnapOnFrame());
	lua_pushliteral(l,"SnapOnFrame");			lua_pushboolean(l,pGlobalTimeSettings->GetSnapOnFrame());
	lua_rawset(l,-3);

    lCount = pGlobalTimeSettings->GetTimeMarkerCount();

	if (lCount)
	{
//		DisplayString("Time Markers");
//		DisplayInt("    Current Time Marker: ", pGlobalTimeSettings->GetCurrentTimeMarker());

		lua_pushliteral(l,"CurrentTimeMarker");			lua_pushnumber(l,pGlobalTimeSettings->GetCurrentTimeMarker());
		lua_rawset(l,-3);

		for (i = 0; i < lCount; i++)
		{

			lua_pushliteral(l,"TimeMarker");
			lua_newtable(l);


//	        DisplayInt("    Time Marker ", i);
			lua_pushliteral(l,"ID");	lua_pushnumber(l,i);
			lua_rawset(l,-3);

//			DisplayString("        Name: ", pGlobalTimeSettings->GetTimeMarker(i)->mName.Buffer());
			lua_pushliteral(l,"Name");	lua_pushstring(l,pGlobalTimeSettings->GetTimeMarker(i)->mName.Buffer());
			lua_rawset(l,-3);

//			DisplayString("        Time: ", pGlobalTimeSettings->GetTimeMarker(i)->mTime.GetTimeString(lTimeString));
			lua_pushliteral(l,"Time");	lua_pushstring(l,pGlobalTimeSettings->GetTimeMarker(i)->mTime.GetTimeString(lTimeString));
			lua_rawset(l,-3);

//			DisplayBool("        Loop: ", pGlobalTimeSettings->GetTimeMarker(i)->mLoop);
			lua_pushliteral(l,"Loop");	lua_pushboolean(l,pGlobalTimeSettings->GetTimeMarker(i)->mLoop);
			lua_rawset(l,-3);


			lua_rawset(l,-3);
        }
	}

//	DisplayString("");

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// find and return a node of the given name, use names to reference all data nodes, names are unique and human readable
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

KFbxNode* lua_fbx_find_node(lua_State *l,KFbxNode* pNode,const char *name)
{
int i;
KFbxNode* ret;

	ret=0;

	if(pNode==0) // first level
	{
		pNode=lua_fbx_get_ptr(l)->GetRootNode();
	}

	if( strcmp( pNode->GetName() , name )==0 )
	{
		return pNode;
	}

	for(i = 0; i < pNode->GetChildCount(); i++)
	{
		ret=lua_fbx_find_node(l,pNode->GetChild(i),name);

		if(ret) break;
	}

	return ret;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get/set Hierarchy , pass table to set, returns table of current values
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

void lua_fbx_Hierarchy_node(lua_State *l,KFbxNode* pNode)
{
int i;
class fbxsdk_2005_08::KFbxNodeAttribute *attr;

const char *type_names[]=
{
		"UNIDENTIFIED",
		"NULL",
		"MARKER",
	    "SKELETON", 
	    "MESH", 
	    "NURB", 
	    "PATCH", 
	    "CAMERA", 
		"CAMERA_SWITCHER",
	    "LIGHT",
		"OPTICAL_REFERENCE",
		"OPTICAL_MARKER",
		"CONSTRAINT"
};

	lua_newtable(l);

	lua_pushliteral(l,"Name");		lua_pushstring(l,pNode->GetName());
	lua_rawset(l,-3);



	attr=pNode->GetNodeAttribute();
	if(attr)
	{
//		lua_pushliteral(l,"Attribute");
//		lua_newtable(l);
		{
			lua_pushliteral(l,"Type");		lua_pushstring(l, type_names[ attr->GetAttributeType() ] );
			lua_rawset(l,-3);
		}
//		lua_rawset(l,-3);
	}

	lua_checkstack(l,4);

	for(i = 0; i < pNode->GetChildCount(); i++)
	{
		lua_pushnumber(l,i+1);
		lua_fbx_Hierarchy_node(l,pNode->GetChild(i));
		lua_rawset(l,-3);
	}
}

int lua_fbx_Hierarchy(lua_State *l)
{
part_ptr p=lua_fbx_get_ptr(l);

	lua_fbx_Hierarchy_node(l,p->GetRootNode());

	return 1;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get/set mesh of the node of the given name , pass table(with name in it) to set or pass name to get,
// returns table of current values
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_fbx_Mesh(lua_State *l)
{
const char *s;

	s=lua_tostring(l,2);	// must always give name of node we are talking about

KFbxMesh* pMesh;
KFbxNode* pNode;
KFbxLink* pLink;

	int numof_mats=0;

	pNode=lua_fbx_find_node(l,0,s);

	if(!pNode) // couldnt find, so error
	{
		luaL_error(l, "couldn't find fbx node" );
	}

	pMesh=(KFbxMesh*) pNode->GetNodeAttribute ();


	pMesh->ComputeVertexNormals(true);


	lua_newtable(l);
	lua_pushliteral(l,"Name");		lua_pushstring(l,pNode->GetName());
	lua_rawset(l,-3);



	int i, j, lPolygonCount = pMesh->GetPolygonCount();

	int LinkCount;

	KFbxVector4* lControlPoints = pMesh->GetControlPoints(); 
	int* mat_idxs=pMesh->GetMaterialIndices();


	int vertex_count=pMesh->GetControlPointsCount();

	int UV_count=pMesh->GetTextureUVCount();
	KFbxVector2* UVs = pMesh->GetTextureUV (); 

	lua_pushliteral(l,"Points");
	lua_newtable(l);

	for( i=0 ; i<vertex_count ; i++ )
	{
		lua_pushnumber(l,i+1);
		lua_newtable(l);

		lua_pushliteral(l,"X");		lua_pushnumber(l,lControlPoints[i][0]);
		lua_rawset(l,-3);
		lua_pushliteral(l,"Y");		lua_pushnumber(l,lControlPoints[i][1]);
		lua_rawset(l,-3);
		lua_pushliteral(l,"Z");		lua_pushnumber(l,lControlPoints[i][2]);
		lua_rawset(l,-3);

		lua_rawset(l,-3);
	}

	lua_rawset(l,-3);

	lua_pushliteral(l,"UVs");
	lua_newtable(l);

	for( i=0 ; i<UV_count ; i++ )
	{
		lua_pushnumber(l,i+1);
		lua_newtable(l);

		lua_pushliteral(l,"X");		lua_pushnumber(l,UVs[i][0]);
		lua_rawset(l,-3);
		lua_pushliteral(l,"Y");		lua_pushnumber(l,UVs[i][1]);
		lua_rawset(l,-3);

		lua_rawset(l,-3);
	}

	lua_rawset(l,-3);



	
	lua_pushliteral(l,"Mats");
	lua_newtable(l);

KFbxGeometry* pGeometry=pMesh;

	for (int ll = 0; ll < 1/*pGeometry->GetLayerCount()*/; ll++)
	{
		KFbxLayerElementMaterial* leMat = pGeometry->GetLayer(ll)->GetMaterials();
		if (leMat)
		{
			if (leMat->GetReferenceMode() == KFbxLayerElement::eINDEX)
				// materials are in an undefined external table
				continue;

			if (leMat->GetDirectArray().GetCount())
			{
//				char header[100];
//				sprintf(header, "    Materials on layer %d: ", l); 
//				DisplayString(header);


				numof_mats=leMat->GetDirectArray().GetCount();

				for (int lCount = 0; lCount < leMat->GetDirectArray().GetCount(); lCount ++)
				{
				KFbxMaterial* lMaterial = leMat->GetDirectArray().GetAt(lCount);

					lua_pushnumber(l,lCount+1);
					lua_newtable(l);


					lua_pushliteral(l,"Name");		lua_pushstring(l,lMaterial->GetName());
					lua_rawset(l,-3);

					lua_pushliteral(l,"Ambient");		lua_push_fbx_color(l,lMaterial->GetAmbient());
					lua_rawset(l,-3);
					lua_pushliteral(l,"Diffuse");		lua_push_fbx_color(l,lMaterial->GetDiffuse());
					lua_rawset(l,-3);
					lua_pushliteral(l,"Specular");		lua_push_fbx_color(l,lMaterial->GetSpecular());
					lua_rawset(l,-3);
					lua_pushliteral(l,"Emissive");		lua_push_fbx_color(l,lMaterial->GetEmissive());
					lua_rawset(l,-3);
					lua_pushliteral(l,"Opacity");		lua_pushnumber(l,lMaterial->GetOpacity());
					lua_rawset(l,-3);
					lua_pushliteral(l,"Shininess");		lua_pushnumber(l,lMaterial->GetShininess());
					lua_rawset(l,-3);
					lua_pushliteral(l,"Reflectivity");		lua_pushnumber(l,lMaterial->GetReflectivity());
					lua_rawset(l,-3);

					lua_pushliteral(l,"ShadingModel");		lua_pushstring(l,lMaterial->GetShadingModel());
					lua_rawset(l,-3);


					lua_rawset(l,-3);
				}
			}
		}
	}

	lua_rawset(l,-3);



	lua_pushliteral(l,"Polys");
	lua_newtable(l);

	for( i=0 ; i<lPolygonCount ; i++ )
	{
		lua_pushnumber(l,i+1);
		lua_newtable(l);

		int lPolygonSize = pMesh->GetPolygonSize(i);

		for (j = 0; j < lPolygonSize; j++)
		{
			int lControlPointIndex = pMesh->GetPolygonVertex(i, j);

			lua_pushnumber(l,j+1);
			lua_pushnumber(l,lControlPointIndex+1);
			lua_rawset(l,-3);

		}

		lua_pushliteral(l,"UVs");
		lua_newtable(l);
		for (j = 0; j < lPolygonSize; j++)
		{
			int UVidx = pMesh->GetTextureUVIndex(i, j);

			lua_pushnumber(l,j+1);
			lua_pushnumber(l,UVidx+1);
			lua_rawset(l,-3);

		}
		lua_rawset(l,-3);


		lua_pushliteral(l,"Normals");
		lua_newtable(l);
		for (j = 0; j < lPolygonSize; j++)
		{
		KFbxVector4 nrm;

			nrm[0]=0;
			nrm[1]=0;
			nrm[2]=0;

			pMesh->GetPolygonVertexNormal(i, j,nrm);

			lua_pushnumber(l,j+1);
			lua_newtable(l);

			lua_pushliteral(l,"X");		lua_pushnumber(l,nrm[0]);
			lua_rawset(l,-3);
			lua_pushliteral(l,"Y");		lua_pushnumber(l,nrm[1]);
			lua_rawset(l,-3);
			lua_pushliteral(l,"Z");		lua_pushnumber(l,nrm[2]);
			lua_rawset(l,-3);

			lua_rawset(l,-3);

		}
		lua_rawset(l,-3);



		lua_pushliteral(l,"Mat");
		if((mat_idxs) && (numof_mats>1)) // mat_idxs seems to be missing if we only have one material, which is kind of sensible
		{
			lua_pushnumber(l,mat_idxs[i]+1);
		}
		else
		{
			lua_pushnumber(l,1);
		}
		lua_rawset(l,-3);

		lua_pushliteral(l,"Group");
		lua_pushnumber(l,pMesh->GetPolygonGroup(i));
		lua_rawset(l,-3);


		lua_rawset(l,-3);
	}

	lua_rawset(l,-3);


	lua_pushliteral(l,"Links");
	lua_newtable(l);


	LinkCount=pMesh->GetLinkCount();

	for( i=0 ; i<LinkCount ; i++ )
	{
	KFbxNode*p;
	int count;
	int *ip;
	double *dp;

	const char * modes[]={
	    "NORMALIZE", 
        "ADDITIVE", 
		"TOTAL1"};

		pLink=pMesh->GetLink(i);
		p=pLink->GetLink();

		lua_pushnumber(l,i+1);
		lua_newtable(l);

			lua_pushliteral(l,"mode");		lua_pushstring(l,modes[pLink->GetLinkMode()] );
			lua_rawset(l,-3);

			lua_pushliteral(l,"linkname");		lua_pushstring(l,p->GetName());
			lua_rawset(l,-3);

			count=pLink->GetControlPointIndicesCount();
			ip=pLink->GetControlPointIndices();
			dp=pLink->GetControlPointWeights();

			for(j=0;j<count;j++)
			{
				lua_pushnumber(l,j+1);
				lua_newtable(l);

				lua_pushliteral(l,"index");		lua_pushnumber(l,ip[j]+1 );
				lua_rawset(l,-3);
				lua_pushliteral(l,"weight");	lua_pushnumber(l,dp[j] );
				lua_rawset(l,-3);

				lua_rawset(l,-3);
			}

		lua_rawset(l,-3);
	}


	lua_rawset(l,-3);




	return 1;
}








/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// mesh to internal object data
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_object * fbx_mesh_into_t3d_object(KFbxNode* pNode, int layer)
{

s32 i,j;
s32 ii,iii;
//s32 tri;

t3d_object	*obj;

//t3d_point   *pm;
t3d_point   *point;
//t3d_point   *pointparent;
t3d_poly    *poly;
t3d_surface *surface;
//t3d_morph	*morph;

//s32 *ip;
//f32 **fpp;

//f32 len;


KFbxMesh* fbx_mesh;

int numof_mats;
int numof_links;


int* mat_idxs;	 


KFbxLayerElementMaterial* fbx_mats=0;
KFbxVector4* vertexs; 
int numof_vertexs;
int numof_polys;

int numof_UVs;
KFbxVector2* UVs;


	obj=0;


	if(!(obj=T3D->AllocObject()))
	{
		goto bogus;
	}




	fbx_mesh=(KFbxMesh*) pNode->GetNodeAttribute ();


	

	obj->numof_surfaces=0;

	fbx_mats = fbx_mesh->GetLayer(layer)->GetMaterials();

	if (fbx_mats)
	{
		if (fbx_mats->GetReferenceMode() != KFbxLayerElement::eINDEX)		// materials are available?
		{


			if(!(surface=T3D->AllocSurface()))
			{
				DBG_Error("Failed to allocate surface.\n");
				goto bogus;
			}
			DLIST_CUTPASTE(obj->surfaces->last,surface,0);
			obj->numof_surfaces++;




			if (fbx_mats->GetDirectArray().GetCount())
			{


				numof_mats=fbx_mats->GetDirectArray().GetCount();

				for (i = 0; i < numof_mats; i++)
				{
				KFbxMaterial* mat = fbx_mats->GetDirectArray().GetAt(i);

					surface->id=i;

					surface->a=(float)mat->GetOpacity();
					surface->r=(float)mat->GetDiffuse().mRed;
					surface->g=(float)mat->GetDiffuse().mGreen;
					surface->b=(float)mat->GetDiffuse().mBlue;

					surface->a*=255.0f;
					surface->r*=255.0f;
					surface->g*=255.0f;
					surface->b*=255.0f;

					CLAMPVAL(surface->a,0,255);
					CLAMPVAL(surface->r,0,255);
					CLAMPVAL(surface->g,0,255);
					CLAMPVAL(surface->b,0,255);

					surface->sa=1.0f;
					surface->sr=(float)mat->GetSpecular().mRed;
					surface->sg=(float)mat->GetSpecular().mGreen;
					surface->sb=(float)mat->GetSpecular().mBlue;

					surface->sa*=255.0f;
					surface->sr*=255.0f;
					surface->sg*=255.0f;
					surface->sb*=255.0f;

					CLAMPVAL(surface->sa,0,255);
					CLAMPVAL(surface->sr,0,255);
					CLAMPVAL(surface->sg,0,255);
					CLAMPVAL(surface->sb,0,255);

					surface->gloss=(float)mat->GetShininess();
					CLAMPVAL(surface->gloss,0,1);

					strncpy( surface->name , mat->GetName() , sizeof( surface->name ) ); surface->name[ sizeof( surface->name ) -1 ]=0;

				}
			}
		}
	}

	vertexs = fbx_mesh->GetControlPoints(); 
	numof_vertexs=fbx_mesh->GetControlPointsCount();

	numof_links=fbx_mesh->GetLinkCount();


	obj->numof_points=0;
	for( i=0 ; i<numof_vertexs ; i++ )
	{

		if(!(point=T3D->AllocPoint()))
		{
			DBG_Error("Failed to allocate point.\n");
			goto bogus;
		}
		DLIST_CUTPASTE(obj->points->last,point,0);
		obj->numof_points++;

		point->id=i;
		point->x=(f32)vertexs[i][0];
		point->y=(f32)vertexs[i][1];
		point->z=(f32)vertexs[i][2];

// scan all bones and add any that effect this point index

		{
		const char * b_name[2];
		f32 b_weight[2];
		int bone_idx;

			b_name[0]="";
			b_weight[0]=0;
			b_name[1]="";
			b_weight[1]=0;

			bone_idx=0;

			for( ii=0 ; ii<numof_links ; ii++ )
			{
			KFbxLink*fbx_link;
			KFbxNode*p;
			int count;
			int *ip;
			double *dp;
			s32 m;
			const char *s;

				fbx_link=fbx_mesh->GetLink(ii);
				p=fbx_link->GetLink();

				m=fbx_link->GetLinkMode();
				s=p->GetName();

				count=fbx_link->GetControlPointIndicesCount();
				ip=fbx_link->GetControlPointIndices();
				dp=fbx_link->GetControlPointWeights();

				for(iii=0;iii<count;iii++)
				{
					if(ip[iii]==i)
					{
						if(bone_idx>=2)
						{
							DBG_Error("more than %d bones per vertex.\n",2);
							goto bogus;
						}
						else
						{
							b_name[bone_idx]=s;
							b_weight[bone_idx]=((f32)(dp[iii]));
							bone_idx++;
							break;
						}
					}
				}
			}

			point->bone=obj->ref_bone(b_name[0],b_weight[0],b_name[1],b_weight[1]);
		}

	}

	numof_UVs=fbx_mesh->GetTextureUVCount();
	UVs = fbx_mesh->GetTextureUV();


	obj->numof_maps=0;
	for( i=0 ; i<numof_UVs ; i++ )
	{
		if(!(point=T3D->AllocPoint()))
		{
			DBG_Error("Failed to allocate point.\n");
			goto bogus;
		}
		DLIST_CUTPASTE(obj->maps->last,point,0);
		obj->numof_maps++;

		point->id=i;
		point->x=(f32)UVs[i][0];
		point->y=(f32)UVs[i][1];
	}


	numof_polys = fbx_mesh->GetPolygonCount();
	mat_idxs=fbx_mesh->GetMaterialIndices();


	obj->numof_polys=0;
	for( i=0 ; i<numof_polys ; i++ )
	{
	int poly_points;

		poly_points= fbx_mesh->GetPolygonSize(i);

		for (j = 0; (j+2) < poly_points; j++)
		{
			if(!(poly=T3D->AllocPoly()))
			{
				DBG_Error("Failed to allocate poly.\n");
				goto bogus;
			}
			DLIST_CUTPASTE(obj->polys->last,poly,0);
			obj->numof_polys++;


			poly->id=i;
			poly->points[0]=obj->findpoint(fbx_mesh->GetPolygonVertex(i, 0));
			poly->points[1]=obj->findpoint(fbx_mesh->GetPolygonVertex(i, j+1));
			poly->points[2]=obj->findpoint(fbx_mesh->GetPolygonVertex(i, j+2));

			poly->maps[0]=obj->findmap(fbx_mesh->GetTextureUVIndex(i, 0));
			poly->maps[1]=obj->findmap(fbx_mesh->GetTextureUVIndex(i, j+1));
			poly->maps[2]=obj->findmap(fbx_mesh->GetTextureUVIndex(i, j+2));


			if((mat_idxs) && (numof_mats>1)) // mat_idxs seems to be missing if we only have one material, which is kind of sensible
			{
				poly->surface=obj->findsurface(mat_idxs[i]);
			}
			else
			{
				poly->surface=obj->findsurface(0);
			}

		}

	}




	return obj;

bogus:

	if(obj)
	{
		T3D->FreeObject(obj);
	}

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill an items data from an fbx node
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/

void fbx_node_into_t3d_item(KFbxNode* pNode , t3d_scene *scene , t3d_item *item)
{
class fbxsdk_2005_08::KFbxNodeAttribute *attr;
const char *s;
s32 i;
t3d_item *new_item;
t3d_stream *new_stream;

const char *type_names[]=
{
		"UNIDENTIFIED",
		"NULL",
		"MARKER",
	    "SKELETON", 
	    "MESH", 
	    "NURB", 
	    "PATCH", 
	    "CAMERA", 
		"CAMERA_SWITCHER",
	    "LIGHT",
		"OPTICAL_REFERENCE",
		"OPTICAL_MARKER",
		"CONSTRAINT"
};


	s=pNode->GetName();
	strncpy(item->name,s,sizeof(item->name)); item->name[sizeof(item->name)-1]=0;

	attr=pNode->GetNodeAttribute();
	if(attr)
	{
	s32 type;
	
		type=attr->GetAttributeType();
		s=type_names[type];


		strncpy(item->type,s,sizeof(item->type)); item->type[sizeof(item->type)-1]=0;
	}

// add the 9 standard default streams, but ignore any animation in the fbx

	for(i = 0; i < 9; i++)
	{
		if( ! ( new_stream = scene->master->AllocStream() ) ) { goto bogus; }
		DLIST_CUTPASTE(item->streams,new_stream,0);
		item->numof_streams++;

	}

	for(i = 0; i < pNode->GetChildCount(); i++)
	{
		if( ! ( new_item = scene->master->AllocItem() ) ) { goto bogus; }
		DLIST_CUTPASTE(scene->items->last,new_item,0);
		scene->numof_items++;

		new_item->parent=item;

		fbx_node_into_t3d_item(pNode->GetChild(i),scene,new_item);
	}

bogus:

	return;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// scene to internal object data
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_scene * fbx_scene_into_t3d_scene(KFbxScene* kscene, thunk3d *T3D)
{
t3d_scene *scene;
t3d_item *item;
s32 i;

	scene=T3D->AllocScene();
	if(scene)
	{
		if( ! ( item = T3D->AllocItem() ) ) { goto bogus; }
		DLIST_CUTPASTE(scene->items->last,item,0);
		scene->numof_items++;
			
		fbx_node_into_t3d_item(kscene->GetRootNode(),scene,item);
	}

	scene->sort_items();

	for( item=scene->items->first , i=0 ; item->next ; item=item->next , i++)
	{
		item->id=i;
	}


	return scene;

bogus:

	if(scene)
	{
		T3D->FreeScene(scene);
	}
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_fbx_openlib (lua_State *l, int upvalues)
{
	const luaL_reg lib[] =
	{
		{	"create"		,	lua_fbx_create	},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
};


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our ptr functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_fbx_ptr_openlib (lua_State *l, int upvalues)
{
const luaL_reg lib[] =
	{
		{"__gc",			lua_fbx_destroy_ptr},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our tab functions
//
// all functions expect the self table to be passed in as arg1
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_fbx_tab_openlib (lua_State *l, int upvalues)
{
const luaL_reg lib[] =
	{

//		{	"unref"					,	lua_fbx_unref},

		{	"GlobalLightSettings"	,	lua_fbx_GlobalLightSettings	},
		{	"GlobalTimeSettings"	,	lua_fbx_GlobalTimeSettings	},
		{	"Hierarchy"				,	lua_fbx_Hierarchy			},
		{	"Mesh"					,	lua_fbx_Mesh				},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


int luaopen_fbx (lua_State *l)
{

    pfbx = KFbxSdkManager::CreateKFbxSdkManager();

	if (!pfbx)
	{
		return(0);
	}


	luaL_newmetatable(l, lua_fbx_ptr_name);
	lua_fbx_ptr_openlib(l,0);
	lua_pop(l,1);

	lua_pushstring(l, LUA_fbx_LIB_NAME );
	lua_newtable(l);
	lua_pushvalue(l, -1); // have this table as the first up value
	lua_fbx_openlib(l,1);
	lua_rawset(l, LUA_GLOBALSINDEX);



	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// close library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

int luaclose_fbx (lua_State *l)
{
	lua_pushstring(l, LUA_fbx_LIB_NAME);
	lua_pushnil(l);
	lua_rawset(l, LUA_GLOBALSINDEX);


    delete pfbx;
	pfbx=0;


	return 0;
}


#endif
