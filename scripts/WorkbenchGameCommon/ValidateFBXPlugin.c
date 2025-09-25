[BaseContainerProps(configRoot: true)]
class AssetTypes
{	
	[Attribute(desc: "Array of all possible bloodtypes with randomization weight. Keep combined randomization weight at 100 for ease of use", category: "Data")]
	protected ref array<ref string> m_aAssetTypes;
	
	bool GetData(inout array<string> data)
	{
		foreach(auto assetType : m_aAssetTypes)
		{
			data.Insert(assetType);
		}
		return data;
	}
};


[WorkbenchPluginAttribute(name: "Validate Asset", wbModules: {"ResourceManager"},resourceTypes: {"fbx","xob"} , awesomeFontCode: 0xf0ad, description:"Validates checks on FBX on various issues that can be tedious and time-consuming to verify manually", category:"EBT Validation")]
class ValidateFBXPlugin: WorkbenchPlugin
{	
	[Attribute("0", UIWidgets.ComboBox, "", "", enums: GetAssetTypes(), category: "Asset Type")]
	int Type;
	
	[Attribute("", uiwidget: UIWidgets.FileNamePicker, desc: "Path to folder where your log file will be created", params: "folders FileNameFormat=absolute", category: "Path")]
	string log_file_folder_path;
	
	[Attribute("1", UIWidgets.CheckBox, "", "")]
	bool Validate_materials
	
	static ref array<string> allFbxs = new array<string>;
	
	
	protected static const ref ParamEnum DEFAULT_ENUM = new ParamEnum("GENERIC", "0");
	protected static ref ParamEnumArray s_AssetTypes = {DEFAULT_ENUM};
	
	static ref ParamEnumArray GetAssetTypes()
	{
		return s_AssetTypes;
	}
	
	static void FillAssetTypes()
	{
		s_AssetTypes.Clear();
		s_AssetTypes.Insert(DEFAULT_ENUM);
		
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = {"conf"};
		filter.searchStr = {"AssetTypes"};
		
		array<string> addons = {};
		GameProject.GetLoadedAddons(addons);
		foreach(string addon : addons)
		{
			string addonName = GameProject.GetAddonID(addon);
			
		}
		
		array<string> configFiles = {};
		ResourceDatabase.SearchResources(filter, configFiles.Insert);
		
		if(configFiles.Count() != 0)
		{
			ResourceName config = configFiles[0];
		    Resource container = BaseContainerTools.LoadContainer(config);
			if(container != null)
			{
			    AssetTypes configData = AssetTypes.Cast(BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer()));
				if(configData != null)
				{	
					array<string> configDataArr = {};
				    configData.GetData(configDataArr);
				    for (int i = 1; i <= configDataArr.Count(); i++)
				    {
				        s_AssetTypes.Insert(new ParamEnum(configDataArr[i-1], i.ToString()));
					}
				}
			}
		}
		return;
	}
	
	override void OnResourceContextMenu(notnull array<ResourceName> resources) 
	{
		FillAssetTypes();
		if (Workbench.ScriptDialog("Validate FBXs", "Select one FBX file you want to validate, also possible to select whole folder", this))
		{
			if(resources.Count() == 0)
			{
				Print("No fbx file selected. Please select at least one fbx file in the resource browser.",LogLevel.WARNING);
				return;
			}
			else if(resources.Count() > 1)
			{
				Print("Validating multiple FBXs at once is not supported yet! Please select only one FBX.", LogLevel.WARNING);
				return;
			}
			

			
			// fbxPath
			string fbxPath;
			Workbench.GetAbsolutePath(resources[0].GetPath(),fbxPath);
			
			if (fbxPath.EndsWith(".xob"))
				fbxPath.Replace(".xob", ".fbx");
			// Clean log file from previous report
			string logFilePath = log_file_folder_path + "/MQA_Log.txt";
			
			logFilePath.Replace("\\","/");
			if (FileIO.FileExists(logFilePath))
			{
				FileIO.DeleteFile(logFilePath);
			}

			// Blender Validation
			if (!EBTConfigPlugin.HasBlenderRegistered())
			{
				Print("Blender is not registered in EBT Config, skipping a few checks");
			}
			else
			{
				BlenderOperatorDescription operatorDescription = new BlenderOperatorDescription("modelqa");
				operatorDescription.blIDName = "ebt.mqa_report_background";
				operatorDescription.AddParam("log_file_path",logFilePath);
				operatorDescription.AddParam("fbx_path", fbxPath); 
				operatorDescription.AddParam("asset_type", s_AssetTypes.Get(Type).m_Key);
				StartBlenderWithOperator(operatorDescription, true);	
			}
						
			
			if(Validate_materials)
			{
				string toolPath;
				const string TOOL_NAME = "/ValidateResource.exe";
				string rootPath;
				Workbench.GetCwd(rootPath);
				toolPath = rootPath + TOOL_NAME; 
				string command =  "\"" + toolPath + "\"";
				
				map<string, ResourceName> materials = new map<string, ResourceName>();
				EBTEmatUtils ematUtils = new EBTEmatUtils();
				bool meta = ematUtils.GetMaterials(resources[0].GetPath(), materials);
				
				for(int i = 0; i < materials.Count(); i++)
				{
					ValidateMaterialPlugin.materials.Insert(materials.Get(materials.GetKey(i)));
				}
				
				command += " --matCount " + ValidateMaterialPlugin.materials.Count();
				command += " --slot --dep --res --imp --st --rgb --type --def --uv --uvMax 5 --uvMin 0 --ext --extMax 95 --extMin 5 -pl Material";
				Workbench.RunProcess(command);
			}
		}
	}
	
	override void Run()
	{
		FillAssetTypes();
		if (Workbench.ScriptDialog("Validate FBXs", "Select one or multiple FBX files you want to validate, also possible to select whole folder", this))
		{
			ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
			array<ResourceName> resources = {};
			resourceManager.GetResourceBrowserSelection(resources.Insert, true);
			
			if(resources.Count() == 0)
			{
				Print("No fbx file selected. Please select at least one fbx file in the resource browser.",LogLevel.WARNING);
				return;
			}
			else if(resources.Count() > 1)
			{
				Print("Validating multiple FBXs at once is not supported yet! Please select only one FBX.", LogLevel.WARNING);
				return;
			}
			

			// fbxPath
			string fbxPath;
			Workbench.GetAbsolutePath(resources[0].GetPath(),fbxPath);

			if (fbxPath.EndsWith(".xob"))
				fbxPath.Replace(".xob", ".fbx");
			// Clean log file from previous report
			string logFilePath = log_file_folder_path + "/MQA_Log.txt";
			logFilePath.Replace("\\","/");
			if (FileIO.FileExists(logFilePath))
			{
				FileIO.DeleteFile(logFilePath);
			}
			
			// Blender Validation
			if (!EBTConfigPlugin.HasBlenderRegistered())
			{
				Print("Blender is not registered in EBT Config, skipping a few checks");
			}
			else
			{
				BlenderOperatorDescription operatorDescription = new BlenderOperatorDescription("modelqa");
				operatorDescription.blIDName = "ebt.mqa_report_background";	
					
				operatorDescription.AddParam("log_file_path",logFilePath);
				operatorDescription.AddParam("fbx_path", fbxPath); 
				operatorDescription.AddParam("asset_type", s_AssetTypes.Get(Type).m_Key);
				StartBlenderWithOperator(operatorDescription, true);	
			}
			
			
			if(Validate_materials)
			{
				string toolPath;
				const string TOOL_NAME = "/ValidateResource.exe";
				string rootPath;
				Workbench.GetCwd(rootPath);
				toolPath = rootPath + TOOL_NAME; 
				string command =  "\"" + toolPath + "\"";
				
				map<string, ResourceName> materials = new map<string, ResourceName>();
				EBTEmatUtils ematUtils = new EBTEmatUtils();
				bool meta = ematUtils.GetMaterials(resources[0].GetPath(), materials);
				
				for(int i = 0; i < materials.Count(); i++)
				{
					ValidateMaterialPlugin.materials.Insert(materials.Get(materials.GetKey(i)));
				}
				
				command += " --matCount " + ValidateMaterialPlugin.materials.Count();
				command += " --slot --dep --res --imp --st --rgb --type --def --uv --uvMax 5 --uvMin 0 --ext --extMax 95 --extMin 5 -pl Material";
				Workbench.RunProcess(command);
			}
		}
	}
	
	[ButtonAttribute("Validate")]
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

class FBXReportToolRequest: JsonApiStruct
{
	string logFile;
	
	void FBXReportToolRequest()
	{
		RegV("logFile");
	}
};

class FBXReportToolResponse: JsonApiStruct
{
	string Output;
	
	void FBXReportToolResponse()
	{
		RegV("Output");
	}
};


class FBXReportToolMessage: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new FBXReportToolRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		FBXReportToolRequest req = FBXReportToolRequest.Cast(request);
		FBXReportToolResponse response = new FBXReportToolResponse();
	
		
		FileHandle fRead = FileIO.OpenFile(req.logFile, FileMode.READ);
		
		Print(req.logFile);
	    string fLine;
		while (fRead.ReadLine(fLine) != -1)
		{
			string logLevel = fLine.Substring(0,3);
			LogLevel level;
			string reportLine = fLine.Substring(3,fLine.Length()-3);
			
			switch(logLevel)
			{
				case "[W]":
					level = LogLevel.WARNING;
					break;
				case "[I]":
					level = LogLevel.DEBUG;
					break;
				case "[E]":
					level = LogLevel.ERROR;
					break;
				default:
					level = LogLevel.NORMAL;
					break;
			}

			Print(string.ToString(reportLine),level);
		}
		fRead.Close();
		return response;
	}
}

