#ifdef WORKBENCH
// possible improvements:
// VISUALS
// - highlight vertice's edges with debug shapes
// - remove debug shapes with another button?
// - make arrow debug shapes hidden by terrain/entities?
// PERFORMANCE
// - GetHeightmap done per tile rather than storing the whole heightmap?
// - make an alternative matrix with calculated angles to not calculate them 3 times
[WorkbenchPluginAttribute(
	name: PLUGIN_NAME,
	description: "Detect rough vertices on terrain.",
	shortcut: "",
	wbModules: { "WorldEditor" },
	category: "Terrain",
	awesomeFontCode: 0xF1B3)]
class SCR_TerrainEdgesFinderPlugin : WorldEditorPlugin
{
	[Attribute(defvalue: "75", desc: "Min angle difference between three vertices [deg]", params: "0.01 179.99 0.01")]
	protected float m_fMinAngleDifference;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "GetHeightmap mode - under, over water, or both", enums: SCR_ParamEnumArray.FromString("Everything;Above water only;Underwater only"))]
	protected int m_iMode; //!< 0 = everything, 1 = above water, 2 = underwater

	[Attribute(defvalue: "33", uiwidget: UIWidgets.Slider, desc: "Trace to find if an entity is hiding the bad vertice (0 = no trace)", params: "0 100 0.1")]
	protected float m_fEntityTraceOffset;

	[Attribute(defvalue: "0", desc: "Force reobtaining terrain's elevation - useful after terrain edits")]
	protected bool m_bForceHeightmapRefresh;

	// output possibilities: logs, multiline text window, clipboard, heatmap, html link page
	[Attribute(defvalue: "1", desc: "Output coordinates as Workbench links")]
	protected bool m_bOutputAsWorkbenchLinks;

	[Attribute(defvalue: "1", desc: "Draw arrows pointing at result vertices")]
	protected bool m_bDrawDebugShapes;

	protected ref array<float> m_aHeightmap;
	protected string m_sLastLoadedWorld;
	protected ref SCR_DebugShapeManager m_DebugShapeManager = new SCR_DebugShapeManager();

//	protected static const int MODE_ALL_TERRAIN = 0;
	protected static const int MODE_ABOVE_WATER = 1;
	protected static const int MODE_UNDER_WATER = 2;

	protected static const float DEBUG_SHAPE_HEIGHT = 20;

	protected static const string PLUGIN_NAME = "Terrain Edges Finder";

	//------------------------------------------------------------------------------------------------
	override protected void Run()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		string worldPath;
		worldEditorAPI.GetWorldPath(worldPath);

		if (!worldPath)
		{
			SCR_WorkbenchHelper.PrintDialog("A world must be loaded first.", PLUGIN_NAME, LogLevel.WARNING);
			return;
		}

		if (!SCR_WorldEditorToolHelper.HasTerrainMesh())
		{
			SCR_WorkbenchHelper.PrintDialog("This world does not have a terrain or its heightmap is not generated.", PLUGIN_NAME, LogLevel.WARNING);
			return;
		}

		if (Workbench.ScriptDialog(PLUGIN_NAME, "", this) == 0)
			return;

		if (m_bForceHeightmapRefresh || !m_aHeightmap || m_sLastLoadedWorld != worldPath)
		{
			WBProgressDialog progress = new WBProgressDialog("Gathering terrain heightmap, please wait...", Workbench.GetModule(WorldEditor));
			progress.SetProgress(0.42);
			m_aHeightmap = SCR_WorldEditorToolHelper.GetTerrainHeightmap();
			if (!m_aHeightmap)
			{
				SCR_WorkbenchHelper.PrintDialog("Cannot load terrain's heightmap.", PLUGIN_NAME, LogLevel.ERROR);
				return;
			}

			worldEditorAPI.GetWorldPath(m_sLastLoadedWorld);
		}

		if (Workbench.ScriptDialog(PLUGIN_NAME, "You are about to process " + m_aHeightmap.Count() + " vertices; continue?", new WorkbenchDialog_OKCancel()) == 0)
			return;

		array<vector> resultPositions = {};
		array<float> resultAngles = {};
		GetSuspectVertices(resultPositions, resultAngles);

		int count = resultPositions.Count();
		if (count < 1)
		{
			SCR_WorkbenchHelper.PrintDialog("No suspect vertices found.", caption: PLUGIN_NAME, level: LogLevel.NORMAL);
			return;
		}

		if (count > 1000)
		{
			if (Workbench.ScriptDialog(PLUGIN_NAME, "The tool is about to output " + count + " entries - this may come from an invalid (too small?) angle set.\nContinue (this may take some time)?", new WorkbenchDialog_OKCancel()) == 0)
				return;
		}

		int maxAngleIndex;
		float maxAngle;
		foreach (int i, float angle : resultAngles)
		{
			if (maxAngle < angle)
			{
				maxAngleIndex = i;
				maxAngle = angle;
			}
		}

		float step = worldEditorAPI.GetTerrainUnitScale();

		// output before prompt

		string worldPathNoFS;
		if (m_bOutputAsWorkbenchLinks)
			worldPathNoFS = SCR_AddonTool.StripFileSystem(worldPath);

		m_DebugShapeManager.Clear();
		string resultStr = string.Format(
			"World: %1\n%2 suspect vertices (angle > %3 degrees):",
			FilePath.StripExtension(FilePath.StripPath(worldPathNoFS)),
			count.ToString(),
			SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(m_fMinAngleDifference, 2));

		foreach (int i, vector position : resultPositions)
		{
			resultStr += "\n" + resultAngles[i].ToString(6, 2) + "deg at ";
			if (m_bOutputAsWorkbenchLinks)
				resultStr += string.Format("enfusion:/" + "/WorldEditor/%1;%2,%3,%4;-70.12,315,0", worldPathNoFS, position[0] + 1.808, position[1] + 5, position[2] - 1.808);
			else
				resultStr += position.ToString();

			if (m_bDrawDebugShapes)
				m_DebugShapeManager.AddArrow(position + { 0, DEBUG_SHAPE_HEIGHT, 0 }, position, step * 0.5);
		}

		int printCount;
		if (count > 10)
		{
			printCount = 10;
			PrintFormat("Showing %1 out of %2 entries", printCount, count, level: LogLevel.NORMAL);
		}
		else
		{
			printCount = count;
		}

		PrintFormat(
			"Max angle position: %1 degrees at %2",
			maxAngle,
			Debug.GetPositionLinkString(resultPositions[maxAngleIndex], 0),
			level: LogLevel.NORMAL);

		for (int i; i < printCount; ++i)
		{
			PrintFormat("- %1deg at %2", resultAngles[i].ToString(6, 2), Debug.GetPositionLinkString(resultPositions[i], 0), level: LogLevel.NORMAL);
		}

		Workbench.ScriptDialog(PLUGIN_NAME, "List of " + count + " suspect vertices.", new SCR_TextResultWorkbenchDialog(resultStr));
	}

	//------------------------------------------------------------------------------------------------
	protected void GetSuspectVertices(notnull out array<vector> vertices, out notnull array<float> angles)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		BaseWorld world = worldEditorAPI.GetWorld();

		int mode;
		bool hasOcean = world.IsOcean();
		if (!hasOcean && m_iMode == MODE_UNDER_WATER)
		{
			SCR_WorkbenchHelper.PrintDialog("Temporarily changing mode from underwater to all terrain, provided this terrain has no ocean.", PLUGIN_NAME, LogLevel.NORMAL);
			mode = 0;
		}
		else
		{
			mode = m_iMode;
		}

		float oceanLevel;
		if (hasOcean)
			oceanLevel = world.GetOceanBaseHeight();

		int verticesXMinus1 = worldEditorAPI.GetTerrainResolutionX() - 1;
		int verticesZMinus1 = worldEditorAPI.GetTerrainResolutionY() - 1;
		int verticesXMinus2 = verticesXMinus1 - 1;
		int verticesZMinus2 = verticesZMinus1 - 1;
		float step = worldEditorAPI.GetTerrainUnitScale();

		int totalPoints = verticesXMinus2 * verticesZMinus2;

		TraceParam traceParam;
		if (m_fEntityTraceOffset > 0)
		{
			traceParam = new TraceParam();
			traceParam.Flags = TraceFlags.ENTS;
		}

		vertices.Clear();
		angles.Clear();

		WBProgressDialog progress = new WBProgressDialog("Comparing angles...", worldEditor);

		float minAngleDifferenceRad = m_fMinAngleDifference * Math.DEG2RAD; // work in rad for precision and speed

		int progressStep, progressStepLimit = totalPoints * 0.01;
		Debug.BeginTimeMeasure();

		for (int z = 1; z < verticesZMinus2; ++z)
		{
			for (int x = 1; x < verticesXMinus2; ++x)
			{
				++progressStep;
				if (progressStep >= progressStepLimit) // min 1%
				{
					progress.SetProgress((verticesXMinus1 * z + x) / totalPoints); // expensive
					progressStep = 0;
				}

				int currIndex = verticesZMinus1 * z + x;

				float currY = m_aHeightmap[currIndex];

				if (mode == MODE_ABOVE_WATER)
				{
					if (currY < oceanLevel)
						continue;
				}
				else
				if (mode == MODE_UNDER_WATER)
				{
					if (currY >= oceanLevel)
						continue;
				}

				vector pos = { x, 0, z } * step + { 0, currY, 0 };

				float outAngle;
				// west-east
				if (IsAngleBad(pos, outAngle, m_aHeightmap[currIndex - 1], m_aHeightmap[currIndex + 1], step, minAngleDifferenceRad, traceParam, world))
				{
					vertices.Insert(pos);
					angles.Insert(outAngle);
					continue;
				}

				// south-north
				if (IsAngleBad(pos, outAngle, m_aHeightmap[currIndex - 1], m_aHeightmap[currIndex + 1], step, minAngleDifferenceRad, traceParam, world))
				{
					vertices.Insert(pos);
					angles.Insert(outAngle);
					// continue;
				}
			}
		}

		Debug.EndTimeMeasure("Comparing angles");
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] mode \see m_iMode
	//! \return array of floats grouped by 4 format x, y, z, angle
//	protected array<float> GetSuspectVerticesPerTile(int mode)

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] position currY is position[1]
	//! \param[out] angle in range [0..180[, 0 being same slope as before, 180 being the pointiest vertice ever seen
	//! \param[in] prevY
	//! \param[in] nextY
	//! \param[in] step
	//! \param[in] minAngleDifferenceRad
	//! \param[in] traceParam
	//! \param[in] world
	//! \return
	bool IsAngleBad(
		vector position,
		out float angle,
		float prevY,
		float nextY,
		float step,
		float minAngleDifferenceRad,
		TraceParam traceParam,
		BaseWorld world)
	{
		float anglePrevToCurr = Math.Atan2(position[1] - prevY, step);
		float angleCurrToNext = Math.Atan2(nextY - position[1], step);

		float difference;
		if (anglePrevToCurr > angleCurrToNext)
			difference = anglePrevToCurr - angleCurrToNext;
		else
			difference = angleCurrToNext - anglePrevToCurr;

		if (difference < 0)
			difference = -difference;

		if (difference < minAngleDifferenceRad)
			return false;

		if (traceParam)
		{
			traceParam.Start = position + { 0, m_fEntityTraceOffset, 0 };
			traceParam.End = position;
		}

		if (traceParam && world.TraceMove(traceParam) != 1)
			return false;

		angle = difference * Math.RAD2DEG;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Process", true)]
	protected int ButtonProcess()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}
}
#endif
