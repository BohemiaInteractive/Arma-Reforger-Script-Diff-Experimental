/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class EnvironmentProbeEntityClass: VolumeReceiverEntityClass
{
}

class EnvironmentProbeEntity: VolumeReceiverEntity
{
	/*!
	Set new probe position
	\param position
	\param invalidate 	invalidate probe data to capture them again
	*/
	proto external void SetProbePosition(vector position, bool invalidate);
	/*!
	Set EV for reflection
	\param EV	 0 is original probe value
	*/
	proto external void SetReflectionEV(float EV);
	/*!
	Set EV for difuse part
	\param EV		0 is original probe value
	*/
	proto external void SetDiffuseEV(float EV);
	/*!
	Get number of probe's volumes
	*/
	proto external int GetVolumesCount();
	/*!
	Get index of volume according the given name, negative value if it doesn't exist
	\param volumeName
	*/
	proto external int GetVolumeIndex(string volumeName);
	/*!
	Get name of volume according the given index, empty string if it doesn't exist
	\param index
	*/
	proto external string GetVolumeName(int index);
	/*!
	enable volume
	\param index
	\param On			true = switch it on, false = switch it off
	*/
	proto external void EnableVolume(int index, bool On);
	/*!
	Is volume enabled ?
	\param index
	*/
	proto external bool IsVolumeEnabled(int index);
}

/*!
\}
*/
