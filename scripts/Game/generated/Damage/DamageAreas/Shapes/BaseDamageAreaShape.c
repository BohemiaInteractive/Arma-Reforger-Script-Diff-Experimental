/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Damage_DamageAreas_Shapes
\{
*/

class BaseDamageAreaShape: ScriptAndConfig
{
	proto external vector GetWorldPosition();
	//! Returns the local transform of this shape
	proto external void GetTransform(out vector mat[]);
	//! Sets the local transform for this area damage.
	proto external void SetTransform(vector mat[]);
	//! Returns the damage area that is using this shape
	proto external DamageArea GetDamageArea();
	//! Returns the world transform of this shape.
	proto external void GetWorldTransform(out vector mat[]);
}

/*!
\}
*/
