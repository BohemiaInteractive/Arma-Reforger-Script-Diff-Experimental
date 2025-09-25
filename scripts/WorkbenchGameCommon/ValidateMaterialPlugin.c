enum ValidationType
{
	All = 0,
	Edds = 1,
	Emats = 2
};

[WorkbenchPluginAttribute(name: "Validate Material", wbModules: {"ResourceManager"},resourceTypes: {"emat","edds"}, awesomeFontCode: 0xf0ad, description:"Validates material and texture issues that can easily be overlooked and identifies elements that violate our conventions", category:"EBT Validation")]
class ValidateMaterialPlugin: WorkbenchPlugin
{	
	
	[Attribute("0", UIWidgets.ComboBox, "All - All selected Emats and Edds \nEdds - Only selected edds files \nEmats - Only selected emats and textures assigned to them",category: "General", enumType: ValidationType)]
	ValidationType m_ePerformChecksOn;
	
	[Attribute("true", UIWidgets.CheckBox, "Use UV Tiling limits that can be set below",category: "Material")]
	bool m_bCheckUVTilingLimits;	
	[Attribute("5", UIWidgets.Slider, "Maximal number for UV Tiling that won't be reported", "-100 100 0",category: "Material")]
	float m_fMaxUVTiling;
	[Attribute("0", UIWidgets.Slider, "Minimal number for UV Tiling that won't be reported", "-100 100 0",category: "Material")]
	float m_fMinUVTiling;
	[Attribute("true", UIWidgets.CheckBox, "Check extreme values of properties",category: "Material")]
	bool m_bCheckExtremeValues;	
	[Attribute("95", UIWidgets.Slider, "Maximal property value in percent","0 100 0",category: "Material")]
	float m_fMaxPropertyValue;
	[Attribute("5", UIWidgets.Slider, "Minimal property value in percent","0 100 0",category: "Material")]
	float m_fMinPropertyValue;
	[Attribute("true", UIWidgets.CheckBox, "Check if material has default settings in General tab (Used more like a reminder)", category:"Material")]
	bool m_bCheckDefaultValues;
	[Attribute("true", UIWidgets.CheckBox, "Check if suffix is in the right map slot(_BCR in BCRMap)", category:"Material")]
	bool m_bCheckSlots;
	[Attribute("true", UIWidgets.Slider, "Check properties that have a mutual dependence",category: "Material")]
	bool m_bDependencies;
	
	[Attribute("true", UIWidgets.CheckBox, "Check textures that are not .tiff or texture types that should have 4 channels file format but does not.",category: "Texture")]
	bool m_bCheckExtension;
	[Attribute("true", UIWidgets.CheckBox, "Check textures with ST_ prefix that are not in SharedData and vice versa",category: "Texture")]
	bool m_bSTPrefix;
	[Attribute("true", UIWidgets.CheckBox, "Check textures that has different resolution than 2^n",category: "Texture")]
	bool m_bResolution;
	[Attribute("true", UIWidgets.CheckBox, "Check textures that do not have matching import settings with its suffix",category: "Texture")]
	bool m_bImportDefaultValues;
	[Attribute("true", UIWidgets.CheckBox, "Check textures that has something wrong with RGB channels for its prefix",category: "Texture")]
	bool m_bCheckTexturesRGBs;
	
	static ref array<ResourceName> materials = new array<ResourceName>;
	static ref array<ResourceName> textures = new array<ResourceName>;
	
	// creating the command with the right settings
	string GetCommand(int matCount)
	{
		string toolPath;
		const string TOOL_NAME = "/ValidateResource.exe";
		string rootPath;
		Workbench.GetCwd(rootPath);
		toolPath = rootPath + TOOL_NAME; 
		string command =  "\"" + toolPath + "\"";
		
		command += " --matCount " + matCount.ToString();
		
		map<string,bool> settings = new map<string,bool>();
		settings[" --uv"] = m_bCheckUVTilingLimits;
		settings[" --ext"] = m_bCheckExtremeValues;
		settings[" --def"] = m_bCheckDefaultValues;
		settings[" --slot"] = m_bCheckSlots;
		settings[" --dep"] = m_bDependencies;
		settings[" --type"] = m_bCheckExtension;
		settings[" --st"] = m_bSTPrefix;
		settings[" --res"] = m_bResolution;
		settings[" --imp"] = m_bImportDefaultValues;
		settings[" --rgb"] = m_bCheckTexturesRGBs;
		

		for(int i = 0; i < settings.Count(); i++)
		{
			if(settings.Get(settings.GetKey(i)))
			{
				bool value = settings.Get(settings.GetKey(i));
				command += settings.GetKey(i);
				if(settings.GetKey(i) == " --uv")
				{
					command += " --uvMax " + m_fMaxUVTiling;
					command += " --uvMin " + m_fMinUVTiling;
				}
				if(settings.GetKey(i) == " --ext")
				{
					command += " --extMax " + m_fMaxPropertyValue;
					command += " --extMin " + m_fMinPropertyValue;
				}

			}
		}
		
		command += " -pl Material";
		return command;
	}
	
	// filling the arrays of Edds and Emats to be checked
	int FillResourceArrays(array<ResourceName> resources)
	{
		materials.Clear();
		textures.Clear();
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MaterialValidatorUtils matValid = new MaterialValidatorUtils();
		TextureValidatorUtils texValid = new TextureValidatorUtils();
		int matCount = 0;
		foreach(ResourceName path: resources)
		{
			// rel path for emat since emats are validated only in WB
			if(path.EndsWith(".emat") && (m_ePerformChecksOn != 1))
			{
				matCount++;
				materials.Insert(path);
			}
			// absolute path for edds since textures are validated also in python
			else if(path.EndsWith(".edds") && (m_ePerformChecksOn != 2))
			{
				string absPath;
				MetaFile meta = resourceManager.GetMetaFile(path.GetPath());
				BaseContainerList configurations = meta.GetObjectArray("Configurations");
				BaseContainer cfg = configurations.Get(0);
				
				string extension = cfg.GetClassName().Substring(0,3);
				Workbench.GetAbsolutePath(path.GetPath(),absPath);
				extension.ToLower();
				absPath.Replace("edds",extension);
				
				textures.Insert(absPath);
			}
		}
		return matCount;
	}
	
	override void OnResourceContextMenu(notnull array<ResourceName> resources) 
	{
		if (Workbench.ScriptDialog("Validate Material", "", this))
		{	
			int matCount = FillResourceArrays(resources);
			if(materials.Count() + textures.Count() == 0)
			{
				Print("No Emat or Edds file selected. Please select at least one emat or edds file in the Resource Browser and make sure you have \"Perform Checks On\" set to the right value",LogLevel.WARNING);
			}
			else
			{
				Print("Validating " + matCount.ToString() + " materials");
				Workbench.RunProcess(GetCommand(matCount));
			}
		}
	}
	
	override void Run()
	{
		if (Workbench.ScriptDialog("Validate Material", "", this))
		{	
			ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
			array<ResourceName> selection = {};
			resourceManager.GetResourceBrowserSelection(selection.Insert, true);
			int matCount = FillResourceArrays(selection);
			if(materials.Count() + textures.Count() == 0)
			{
				Print("No Emat or Edds file selected. Please select at least one emat or edds file in the Resource Browser and make sure you have \"Perform Checks On\" set to right value",LogLevel.WARNING);
			}
			else
			{
				Print("Validating " + matCount.ToString() + " materials");
				Workbench.RunProcess(GetCommand(matCount));
			}
		}
	}
	
	[ButtonAttribute("OK")]
	bool OK()
	{
		return true;
	}
	
	[ButtonAttribute("Cancel")]
	bool Cancel()
	{
		return false;
	}

};


class MaterialValidatorUtils
{				
	// These dependencies are not using RGB colors
	void CheckDependencies(BaseContainer mat, out bool issue)
	{
		// dictionary Key(Map) -> Value(Params) that depends on that map
		map<string,ref array<string>> dependencies = new map<string, ref array<string>>();
		dependencies["NMOMap"] = {"NormalPower"};
		dependencies["DetailNormalMap"] = {"DetailNormalPower"};
		dependencies["GlobalNMOMap"] = {"NormalPower"};
		dependencies["MudNMOMap"] = {"MudDetNormal"};
		dependencies["NMO_4"] = {"DetNormal_4"};
		dependencies["NMO_3"] = {"DetNormal_3"};
		dependencies["NMO_2"] = {"DetNormal_2"};
		dependencies["NMO_1"] = {"DetNormal_1"};
		dependencies["MaskMap"] = {"MaskRedScale","MaskGreenScale","MaskBlueScale"};
		dependencies["NTCMap"] = {"NormalPower"};
		dependencies["NMOMap2"] = {"NormalPower2"};
		dependencies["DetailNormalMap2"] = {"DetailNormalPower2"};
		
		
		for(int i = 0; i < dependencies.Count(); i++)
		{
			string mapVarname = dependencies.GetKey(i);
			// if map exists in the shader
			if(mat.GetVarIndex(mapVarname) != -1)
			{
				string mapValue;
				string paramValue;
				string defaultValue;
				array<string> depParam = dependencies.Get(mapVarname);
				// iterate through all params of that map
				foreach(string paramVarname: depParam)
				{
					// get values from the material
					mat.Get(mapVarname, mapValue);
					mat.Get(paramVarname, paramValue);
					mat.GetDefaultAsString(paramVarname, defaultValue);
					// Map is Null and Param is set with some value that is not zero
					if(mapValue == "" && (paramValue != "" && paramValue != "0" && paramValue != defaultValue))
					{
						issue = true;
						Print("    - " + paramVarname + " was directly set but " + mapVarname + " is missing",LogLevel.WARNING);
					}
					// Map is set
					else if(mapValue != "")
					{
						// get the value of the param even if default
						GetValue(mat,paramVarname, paramValue);
						// param is 0 and map is set
						if(paramValue == "0")
						{
							issue = true;
							Print("    - " + mapVarname + " was directly set but " + paramVarname + " is 0",LogLevel.WARNING);
						}
					}
				}
			}
		}
		return;
	}
	// Dictionary what Slots can be used for each suffix
	map<string,ref array<string>> GetTextureConnections()
	{
		map<string,ref array<string>> suffixToSlots = new map<string,ref array<string>>();
		suffixToSlots["BCR"] = {"BCRMap","BCR_1","BCR_2","BCR_3","BCR_4","MudBCRMap","DirtBCRMap","BCRMap2","EmissiveMap"};
		suffixToSlots["NMO"] = {"NMOMap","NMO_1","NMO_2","NMO_3","NMO_4","MudNMOMap","NMOMap2","GlobalNMOMap","DetailNormalMap","DetailNormalMap2"};
		suffixToSlots["N"] = suffixToSlots.Get("NMO");
		suffixToSlots["MCR"] = {"MCRMap","GlobalMCRMap","GlobalMCMap","MacroMap"};
		suffixToSlots["NHO"] = suffixToSlots.Get("NMO");
		suffixToSlots["MC"] = suffixToSlots.Get("MCR");
		suffixToSlots["MC"].Insert("GlobalMCMap");
		suffixToSlots["NTC"] = {"NTCMap"};
		suffixToSlots["MLOD"] = {"BCRMap"};
		suffixToSlots["VFX"] = {"VFX","VFXMap"};
		suffixToSlots["MASK"] = {"MaskMap","Mask_2","Mask_3","Mask_4","DirtMaskMap","MudMaskMap","SubsurfaceMask"};
		suffixToSlots["A"] = {"OpacityMap","HeightMap"};
		suffixToSlots["CRM"] = {"CamoCRMMap"};
		suffixToSlots["EM"] = {"EmissiveMap","ProjectionMap"};
		suffixToSlots["H"] = {"HeightMap","PuddleHeightMap"};
		return suffixToSlots;
	}
	
	// Checks if the right map is in the right slot (ex. _BCR/_MLOD in BCR Map)
	void CheckSlots(string slot, string suffix)
	{				
		// Dictionary SUFFIX:[SLOT,SLOT,SLOT,..]
		map<string,ref array<string>> connections = GetTextureConnections();

				
		// get value of the suffix key (What slots can be used for the suffix)
		array<string> slots = connections.Get(suffix);
		// If array not null => Suffix is supported
		// and if it doesnt contain the slot, it is wrong
		if(slots && !slots.Contains(slot))
		{
			Print("    - Texture with suffix " + suffix + " is in " + slot,LogLevel.ERROR);
		}
		else if(!slots)
		{
			Print("    - " + suffix + " suffix is not supported for checking its slot");
		}
	}
	
	
	// Checks parameters that should stay as default values(General tab, Wetness, etc..)
	void CheckDefaults(BaseContainer cont, out bool issue)
	{
		//https://confluence.bistudio.com/display/~phammac/Typos+in+Workbanch+parametrs
		//DisableUserAphaInShadow
		// Doesnt need to clear the array, since its initialized everytime the function is called
		array<string> DefaultProperties = {"ZEnable", "ZBias", "Cull", "Sort", "SortBias", "CastShadow", "ReceiveShadow", "OccludeRain", 
		"OccludeWetness", "AddWetness", "DisableFog", "DisableInteriorProbes", "TCModFuncs", "NoDecals", "EnableClutter", "ClutterThresholdAngle", 
		"CrossFade", "WetnessScale", "PorosityScale", "CustomWetnessChannel", "SlidingDropsVisibility" };
		
		array<string> allDefaultProperties = {"ZEnable","ZBias","Cull","Sort","SortBias","CastShadow","ReceiveShadow","OccludeRain","OccludeWetness",
		"AddWetness","DisableFog","DisableInteriorProbes","TCModFuncs","NoDecals","EnableClutter","ClutterThresholdAngle","CrossFade",
		"WetnessScale","PorosityScale","CustomWetnessChannel","SlidingDropsVisibility","ZWrite","AlphaTest","AlphaMul","BlendMode","AllowUserAlphaBias","BisableUserAphaInShadow","ManualCascadesShadows"};
		string matClass = cont.GetClassName();
		for(int i = 0; i < cont.GetNumVars(); i++)
		{
			string var = cont.GetVarName(i);
			// if the prop is in General props that should be checked, check if it's set directly
			if(allDefaultProperties.Contains(var) && cont.IsVariableSetDirectly(var))
			{
				issue = true;
				Print("    - " + var + " doesn't match the default settings of " + matClass);
			}
		}
	}
	
	// Cont.Get but it if it's not set it will return default, not empty string
	void GetValue(BaseContainer cont, string varName, out string value)
	{
		// get value the normal way
		cont.Get(varName, value);
		// if the value is empty and variable is not set get default
		if(!cont.IsVariableSetDirectly(varName) && value == "")
		{
			cont.GetDefaultAsString(varName, value);
		}
		return;
	}
	
		
	// Checking all validations for all textures on material
	void CheckTextures(BaseContainer cont, out array<string> matTextures, out array<string> slots, bool checkSlot, out bool issue)
	{
		TextureValidatorUtils textValid = new TextureValidatorUtils();
		string absPath;
		// going through properties of emat
		for(int i = 0; i < cont.GetNumVars(); i++)
		{
			string var = cont.GetVarName(i);
			ResourceName value;
			cont.Get(var, value);
			// get edds in emat
			if(value.Contains(".edds"))
			{
				// texture value will be with anim, removing that
				if(value.EndsWith(" 0"))
				{
					value = value.Substring(0, value.Length() - 2);
				}	
				
				// checking GUID
				string guid = value.Substring(0,18);
				string guidtest = Workbench.GetResourceName(guid);
				if(guid == guidtest)
				{
					issue = true;
					Print("    - " + cont.GetResourceName() + " has a wrong guid!",LogLevel.ERROR);
					return;
				}
				
				// meta of the edds to get the right extesion
				ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
				MetaFile meta = resourceManager.GetMetaFile(value.GetPath());
				if(!meta)
				{
					Print("    - Metafile for " + value + "(" + var +  ") couldn't be found!",LogLevel.ERROR);
					continue;
				}
				BaseContainerList configurations = meta.GetObjectArray("Configurations");
				BaseContainer cfg = configurations.Get(0);
				
				// getting extension from edds
				string extension = cfg.GetClassName().Substring(0,3);
				// need to get relative path for texture validator class	
				Workbench.GetAbsolutePath(value.GetPath(),absPath);
				extension.ToLower();
				absPath.Replace("edds",extension);
				
				// if texture was selected and also in a material that was selected, the texture errors will be reported twice
				// this removes the texture from validating textures itself and validates it only once with the material
				if(ValidateMaterialPlugin.textures.Contains(absPath))
				{
					ValidateMaterialPlugin.textures.RemoveItem(absPath);
				}
				
				
				// absolute path of texture with right extension(.tif, etc..) for python
				matTextures.Insert(absPath);
				// texture suffix and its slot
				string suffix = textValid.GetSuffix(value);
				string slot = var;
				slots.Insert(slot);	
				// this is removing int from Mask1 => Mask. But I can add Mask1/2 into the dict..
				if(suffix[suffix.Length() - 1].ToInt())
				{
					suffix = suffix.Substring(0,suffix.Length()-1);
				}
				
				// checking the right slot
				if(checkSlot)
				{
					CheckSlots(slot,suffix);
				}
						
				// from here it continues to Python and to TextureValidator
			}
		}
		return;
	}
		
	// Check extreme values of properties (NormalPower,etc...)
	void CheckExtremes(BaseContainer cont, float min, float max, out bool issue)
	{
		array<string> properties = {"NormalPower","MudDetNormal","DetNormal_1","DetNormal_2","DetNormal_3","DetNormal_4","DetailNormalPower","DetailNormalPower2",
		"DetailMaskSharpness","DetailMaskOffset","DetailMaskUVScale","NormalPower2","RoughnessScale2","MetalnessScale2","RoughnessScale","MetalnessScale",
		"MudRoughness","MudMetalness","Roughness1","Metalness_1","Roughness_2","Metalness_2","Roughness_3","Metalness_3","Roughness_4","Metalness_4",
		"AO_1","AO_2","AO_3","AO_4",};
		foreach(string property: properties)
		{
			string value;
			// have to check if the property exists in the shader
			if(cont.GetVarIndex(property) != -1)
			{
				cont.Get(property,value);
				if(value != "" || cont.IsVariableSetDirectly(property))
				{
					float limitMin;
					float limitMax;
					float step;
					cont.GetLimits(cont.GetVarIndex(property),limitMin,limitMax, step);
					float newMin = limitMax * (min / 100);
					float newMax = limitMax * (max / 100);
					if((value.ToFloat() < newMin || value.ToFloat() > newMax) && (value.ToFloat() != 0))
					{
						issue = true;
						Print("    - " + property + " is out of limitation(" + newMin + "-" + newMax + ")",LogLevel.WARNING);
					}	
				}
			}
		}
	}
	
	// Validates material UVs (Unified, Extremes)
	void ValidateUVs(BaseContainer cont, float MinTiling, float MaxTiling, out bool issue)
	{
		BaseContainer empty;
		
		for(int i = 0; i < cont.GetNumVars(); i++)
		{
			string var = cont.GetVarName(i);
			// getting object
			if(cont.GetObject(var) != empty)
			{
				BaseContainer uvTiling = cont.GetObject(var);
				// checking if the object is UV Transforms
				if(uvTiling.GetClassName() == "MatUVTransform")
				{
					string tilingU, tilingV; 
					GetValue(uvTiling, "TilingU", tilingU);
					GetValue(uvTiling, "TilingV", tilingV);
					// UV tiling uniform
					if(tilingU != tilingV)
					{
						issue = true;
						Print("    - UV Tiling on " + var + " is not uniform.",LogLevel.ERROR);
					}
					// UV tiling limits
					if(MinTiling > tilingU.ToFloat() || MaxTiling < Math.AbsFloat(tilingU.ToFloat()))
					{
						issue = true;
						Print("    - UV Tiling on " + var + " is out of limitation, U = " + tilingU,LogLevel.WARNING);
					}
					if(MinTiling > tilingV.ToFloat() || MaxTiling < Math.AbsFloat(tilingV.ToFloat()))
					{
						issue = true;
						Print("    - UV Tiling on " + var + " is out of limitation, V = " + tilingV,LogLevel.WARNING);
					}
					
				}
			}
			string value; 
			cont.Get(var, value);
		}
		return;
	}
}

class TextureValidatorUtils
{
	// When texture has ST_ prefix, must be in SharedData and vice versa
	void CheckSTConvention(TextureCheck txt,out bool issue)
	{	
		ResourceName texture = txt.m_path;
		string filename = FilePath.StripExtension(FilePath.StripPath(texture));
		string path = FilePath.StripExtension(FilePath.StripFileName(texture));
		if(filename.StartsWith("ST_") && !path.Contains("_SharedData"))
		{
			issue = true;
			Print("	- contains ST_ prefix but it's not located in _SharedData",LogLevel.ERROR);
		}
		else if(!filename.StartsWith("ST_") && path.Contains("_SharedData"))
		{
			issue = true;
			Print("	- doesn't contain ST_ prefix but it's located in _SharedData",LogLevel.ERROR);
		}
		
	}
	
	// Texture should have the same import settings per texture type
	void TextureImportSettings(ResourceName path, out bool issue)
	{
		// Get config (From metafile) and check if something is set directly
		// If that won't work check if something is different from parent conf.
		
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(path.GetPath());
		
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer cfg = configurations.Get(0);
		
		for(int i = 0; i < cfg.GetNumVars(); i++)
		{
			string var = cfg.GetVarName(i);
			if(cfg.IsVariableSetDirectly(var))
			{
				issue = true;
				Print("	- " + var + " doesn't match the Import settings config!", LogLevel.WARNING);
			}
		}
		
		TextureTypes textureTypes = new TextureTypes();
		string absPath;
		Workbench.GetAbsolutePath(path.GetPath(),absPath);
		TextureType type = textureTypes.FindTextureType(absPath);
		//Somehow check if texture has valid suffix
		if(type.m_PostFix == string.Empty)
		{
			issue = true;
			Print("	- doesn't have any valid suffix!", LogLevel.ERROR);
		}
		return;
	}
	
	// get suffix of texture resource name
	string GetSuffix(ResourceName texture)
	{
		array<string> splitted = new array<string>;
		texture.Split("_",splitted,true);
		string suffix = splitted[splitted.Count() - 1];
		suffix.ToUpper();
		suffix.Replace(".EDDS","");
		return suffix;
	}
		
	// report resolution that is not 2^n
	void CheckResolution(TextureCheck txt,out bool issue)
	{
		if(!(txt.m_width & (txt.m_width - 1)) == 0 && txt.m_width != 0 || !(txt.m_height & (txt.m_height - 1)) == 0 && txt.m_height != 0)
		{
			issue = true;
			Print("	- doesn't have proper resolution!",LogLevel.ERROR);
		}
	}
	// if the extension is not .tif
	void CheckExtension(TextureCheck txt,out bool issue)
	{
		if(txt.m_fileType != "tif" && txt.m_numChannels == 4)
		{
			issue = true;
			//
			Print("	- is a\'" + txt.m_fileType + "\'format not\'tif\'",LogLevel.WARNING);			
		}
	}
	
	void CheckChannelCount(TextureCheck txt, out bool issue)
	{
		array<string> suffixChannels4 = {"BCR","BCA","NMO","MCR","NTC","NHO"};
		foreach(string suffix: suffixChannels4)
		{
			string fileName = FilePath.StripPath(txt.m_path);
			if(fileName.Contains(suffix) && txt.m_numChannels != 4 && txt.m_fileType != "tif")
			{
				issue = true;
				Print("	- is " + txt.m_fileType + " with " + suffix + " suffix but file uses only " + txt.m_numChannels + " channels",LogLevel.ERROR);
			}
		}
	}

	// NMO always should have R and G channel.
	// Maybe add here more checks
	void CheckRGBDependencies(TextureCheck txt,out bool issue)
	{
		string fileName = FilePath.StripPath(txt.m_path);
		fileName.ToUpper();
		// For suffix -> No matter in what slot the texture is, checks RG for NMO suffix
		if(GetSuffix(txt.m_path) == "NMO" && (txt.m_relRGB[0] == 0 || txt.m_relRGB[1] == 0))
		{
			issue = true;
			Print("	- has NMO suffix but it has either Red or Green channel empty",LogLevel.WARNING);
		}

		MaterialValidatorUtils matValid = new MaterialValidatorUtils();
		
		// For slots -> If texture in NMO it will check RG channels no matter of suffix
		map<string,ref array<string>> connections = matValid.GetTextureConnections();
		if(connections["NMO"].Contains(txt.m_slot) && (txt.m_relRGB[0] == 0 || txt.m_relRGB[1] == 0))
		{
			issue = true;
			Print("	- is in " + txt.m_slot + " slot but it has either Red or Green channel empty",LogLevel.WARNING);
		}
		float rgbPercentage = txt.m_absRGB[0] + txt.m_absRGB[1] + txt.m_absRGB[2];
		if(rgbPercentage < 0.1)
		{
			Print("	- has almost no values!",LogLevel.WARNING);
		}
	
	}
}

class MaterialValidatorRequest: JsonApiStruct
{
	bool uvTiling;
	float uvMax;
	float uvMin;
	
	bool extremeValues;
	float extMax;
	float extMin;
	
	bool defaultValues;
	bool slots;
	bool dependencies;
	
	bool extension;
	bool stPrefix;
	bool resolution;
	bool importDef;
	bool rgb;
		
	bool getOnlyTextures;
	int matIndex;
	
	int index;
	void MaterialValidatorRequest()
	{
		RegV("uvTiling");
		RegV("uvMax");
		RegV("uvMin");
		
		RegV("extremeValues");
		RegV("extMax");
		RegV("extMin");
		
		RegV("defaultValues");
		RegV("slots");
		RegV("dependencies");
		
		RegV("matIndex");
		RegV("getOnlyTextures");
		//Textures
		RegV("extension");
		RegV("stPrefix");
		RegV("resolution");
		RegV("importDef");
		RegV("rgb");
		RegV("index");
					
	}
};


class MaterialValidatorResponse: JsonApiStruct
{
	ref array<string> paths = new array<string>();
	ref array<string> slots = new array<string>();
	ref array<ResourceName> textures = new array<ResourceName>();
	int count;
	
	void MaterialValidatorResponse()
	{
		RegV("paths");
		RegV("slots");
		RegV("textures");
		RegV("count");
	}
};

class MaterialValidator: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new MaterialValidatorRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		bool issue = false;
		const int MAX_TEXTURES_SEND = 4500;
		MaterialValidatorRequest req = MaterialValidatorRequest.Cast(request);
		MaterialValidatorResponse response = new MaterialValidatorResponse();
		MaterialValidatorUtils matValid = new MaterialValidatorUtils();
		TextureValidatorUtils textValid = new TextureValidatorUtils();
		// need to call this to get array of selected textures
		if(req.getOnlyTextures)
		{
			response.count = ValidateMaterialPlugin.textures.Count();
			if(ValidateMaterialPlugin.textures.Count() != 0)
			{
				for(int i = req.index-1; i < ValidateMaterialPlugin.textures.Count(); i++)
				{
					response.textures.Insert(ValidateMaterialPlugin.textures[i]);	
					if(response.textures.Count() >= MAX_TEXTURES_SEND)
					{
						Print("Validating " + response.textures.Count() + " textures");
						return response;
					}
				}
			}
			Print("Validating " + response.textures.Count() + " textures");
			return response;
		}
		Resource resource = Resource.Load(ValidateMaterialPlugin.materials[req.matIndex]);
		BaseContainer material = resource.GetResource().ToBaseContainer();
		PrintFormat("Material - @\"%1\" :",material.GetResourceName(),LogLevel.DEBUG);
		
		if(req.uvTiling)
		{
			matValid.ValidateUVs(material, req.uvMin, req.uvMax, issue);
		}
		if(req.defaultValues)
		{
			matValid.CheckDefaults(material, issue);
		}
		if(req.extremeValues)
		{
			matValid.CheckExtremes(material,req.extMin,req.extMax, issue);
		}
		if(req.dependencies)
		{
			matValid.CheckDependencies(material, issue);
		}
		matValid.CheckTextures(material, response.paths, response.slots, req.slots, issue);
		
		if(!issue)
		{
			Print("    - Material has 0 issues!");
		}
		return response;
	}
}



// only material, abspath, relpath or index
// in constructor set material as BaseContainer and set m_slot by finding the varName by the its value(m_path) in the material(m_mat)
class TextureCheck
{
	// File extension
	string m_fileType;
	// resolution
	int m_width;
	int m_height;
	// rel path
	string m_path;
	// Relative RGB
	// how many pixels channel takes to other channels(Addition is 100%)
	// when there is 1 pixel red and everything else black RGB = 100,0,0
	vector m_relRGB;
	// Slot in material
	string m_slot;
	// Absolute RGB
	// how many pixels channel takes in percentage
	// when there is 1 pixel red and everything else black RGB = 0.001,0,0
	vector m_absRGB;
	int m_numChannels;
	
	bool m_nonPBRValues;
	
	
	void TextureCheck(string fileType, int width, int height, string path, vector rgb, vector absRgb ,string slot, int numChannels, bool nonPBRValues)
	{
		m_fileType = fileType;
		m_width = width;
		m_height = height;
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(path);
		m_path = meta.GetResourceID();
		m_relRGB = rgb;
		m_slot = slot;
		m_absRGB = absRgb;
		m_numChannels = numChannels;
		m_nonPBRValues = nonPBRValues;
	}
}

			
class TextureValidatorRequest: JsonApiStruct
{
	bool extension;
	bool stPrefix;
	bool resolution;
	bool importDef;
	bool rgb;
	
	
	ref array<bool> brokenImage;
	ref array<string> fileType;
	ref array<int> resolutions;	
	ref array<float> rgbs;
	ref array<ResourceName> relTextures;
	ref array<string> slots;					
	ref array<float> absRgb;
	ref array<int> numChannels;
	ref array<bool> nonPBRValues;
	
	 
	bool finished;
	void TextureValidatorRequest()
	{
		// settings
		RegV("extension");
		RegV("stPrefix");
		RegV("resolution");
		RegV("importDef");
		RegV("rgb");
		
		// texture values
		RegV("brokenImage");
		RegV("fileType");
		RegV("resolutions");
		RegV("rgbs");
		RegV("relTextures");
		RegV("slots");
		RegV("absRgb");
		RegV("numChannels");
		RegV("nonPBRValues");
		
		RegV("finished");
	}
};

class TextureValidatorResponse: JsonApiStruct
{
	ref array<string> paths = new array<string>();
	
	void TextureValidatorResponse()
	{
		RegV("paths");
	}
};

class TextureValidator: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new TextureValidatorRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		TextureValidatorRequest req = TextureValidatorRequest.Cast(request);
		TextureValidatorResponse response = new TextureValidatorResponse();
		TextureValidatorUtils textValid = new TextureValidatorUtils();
		if(!req.relTextures)
		{
			return response;
		}
		for(int i = 0; i < req.relTextures.Count(); i++)
		{
			// rgb values
			vector rgb = {req.rgbs[i*3], req.rgbs[i*3+1], req.rgbs[i*3+2]};
			vector absRgb = {req.absRgb[i*3],req.absRgb[i*3+1], req.absRgb[i*3+2]};
			TextureCheck texture = new TextureCheck(req.fileType[i], req.resolutions[i*2],req.resolutions[i*2+1],req.relTextures[i], rgb,absRgb, req.slots[i], req.numChannels[i], req.nonPBRValues[i]);
			

			if(texture.m_slot != "")
			{
				PrintFormat("@\"%1\"(\"%2\") :",texture.m_path,texture.m_slot,LogLevel.DEBUG);
			}
			else
			{
				PrintFormat("@\"%1\" :",texture.m_path,LogLevel.DEBUG);
			}

			// Default import settings
			bool issue = false;
			if(req.importDef)
			{
				textValid.TextureImportSettings(texture.m_path, issue);
			}
			// ST_ prefix
			if(req.stPrefix)
			{
				textValid.CheckSTConvention(texture, issue);
			}
			// texture resolution
			if(!req.brokenImage[i])
			{
				if(req.resolution)
				{
					textValid.CheckResolution(texture, issue);	
				}
				
				TextureTypes textureTypes = new TextureTypes();
				string absPath;
				ResourceName textPath = texture.m_path;
				Workbench.GetAbsolutePath(textPath.GetPath(),absPath);
				TextureType type = textureTypes.FindTextureType(absPath);
				if(texture.m_nonPBRValues)
				{
					if(type.m_PostFix == "_bcr")
					{
						Print("	- has some values outside PBR range Albedo - (30-240)",LogLevel.WARNING);
					}
					else
					{
						Print("	- has some values outside PBR range Metalness - (155-255)",LogLevel.WARNING);
					}
				}
				// file extension and channel count
				if(req.extension)
				{
					textValid.CheckExtension(texture, issue);
					textValid.CheckChannelCount(texture, issue);
				}
				// texture rgb
				if(req.rgb)
				{
					textValid.CheckRGBDependencies(texture, issue);
				}	
			}
			else
			{
				Print("	- Texture couldn't be opened!",LogLevel.WARNING);
				continue;	
			}
			if(!issue)
			{
				Print("	- Texture has 0 issues!");
			}
		}
		if(req.finished)
		{
			Print("------------------Validation is completed------------------",LogLevel.DEBUG);
		}
		return response;
	}
}