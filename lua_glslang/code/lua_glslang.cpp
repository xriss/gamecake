/***************************************************************************

(C) Kriss@XIXs.com 2017 and released under the MIT license 
https://opensource.org/licenses/MIT

*/
#include "all.h"


	const TBuiltInResource Resources = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,
    /* .maxDualSourceDrawBuffersEXT = */ 1,

    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }};


/***************************************************************************
--[[#code.glslang.lua_glslang_lint_gles2

	lua_glslang_lint_gles2(lua)

	lua inputs
		vertex code string
		fragment code string
		linker flag strinf

	lua returns
		vertex error string
		fragment error string
		linker error string

Compile a vertex shader and a fragment shader for GLES2, return 
nil,nil,nil for no errors or an error string for either phase if 
something went wrong.

]]*/
static int lua_glslang_lint_gles2(lua_State *l)
{
	ShHandle compilers[2];
	ShHandle linker;
//	TBuiltInResource Resources;    
    
	int count=0;
	int ret;
	const char *s;

	const char *vstr=lua_tostring(l,1);
	const char *fstr=lua_tostring(l,2);
	const char *cstr=lua_tostring(l,3);

	compilers[0]=ShConstructCompiler(EShLangVertex,0);
	compilers[1]=ShConstructCompiler(EShLangFragment,0);
	linker=ShConstructLinker(EShExVertexFragment,0);

	if(vstr)
	{
		ret = ShCompile(compilers[0], &vstr, 1, nullptr, EShOptNone, &Resources, 0, 100 , true, EShMsgDefault);
		s=ShGetInfoLog(compilers[0]);
		if(ret)	{	lua_pushnil(l);			}
		else	{	lua_pushstring(l,s);	}
	}
	else	{	lua_pushnil(l);			}

	if(fstr)
	{
		ret = ShCompile(compilers[1], &fstr, 1, nullptr, EShOptNone, &Resources, 0, 100 , true, EShMsgDefault);
		s=ShGetInfoLog(compilers[1]);
		if(ret)	{	lua_pushnil(l);			}
		else	{	lua_pushstring(l,s);	}
	}
	else	{	lua_pushnil(l);			}

	if(cstr)
	{
		ret = ShLinkExt(linker, compilers, 2);
		s=ShGetInfoLog(linker);					// I think we always get an error here?
		if(ret)	{	lua_pushnil(l);			}
		else	{	lua_pushstring(l,s);	}  // but nothing in the log
	}
	else	{	lua_pushnil(l);			}


	ShDestruct(linker);
	ShDestruct(compilers[1]);
	ShDestruct(compilers[0]);
	
	return 3;
}

/***************************************************************************
--[[#code.glslang.lua_glslang_pp

	lua_glslang_cpp(lua)

	lua inputs
		code string

	lua returns
		preprocesed code string or nil
		error string if code string is nil

Run the preprocesor on the given code string.

]]*/
static int lua_glslang_pp(lua_State *l)
{

	const char* shaderCodeVertex = lua_tostring(l,1);

	const glslang_input_t input =
	{
		.language = GLSLANG_SOURCE_GLSL,
		.stage = GLSLANG_STAGE_VERTEX,
		.client = GLSLANG_CLIENT_VULKAN,
		.client_version = GLSLANG_TARGET_VULKAN_1_1,
		.target_language = GLSLANG_TARGET_SPV,
		.target_language_version = GLSLANG_TARGET_SPV_1_3,
		.code = shaderCodeVertex,
		.default_version = 100,
		.default_profile = GLSLANG_NO_PROFILE,
		.force_default_version_and_profile = false,
		.forward_compatible = false,
		.messages = GLSLANG_MSG_DEFAULT_BIT,
		.resource = reinterpret_cast<const glslang_resource_t*>(&Resources),
	};

	glslang_shader_t* shader = glslang_shader_create( &input );
	
	if(!shader)
	{
		lua_pushnil(l);
		lua_pushstring(l,"failed to glslang_shader_create");
		return 2;
	}

	if ( !glslang_shader_preprocess(shader, &input) )
	{
		lua_pushnil(l);
		lua_pushstring(l,glslang_shader_get_info_debug_log(shader));
		glslang_shader_delete( shader );
		return 2;
	}

	lua_pushstring(l,glslang_shader_get_preprocessed_code(shader));

	glslang_shader_delete( shader );
	return 1;
}


/*

Open the lua library.

*/
extern "C" int luaopen_glslang_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"pp",	lua_glslang_pp},

		{"lint_gles2",	lua_glslang_lint_gles2},

		{0,0}
	};

	glslang_initialize_process();
	ShInitialize();

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

