/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup SaveGame
\{
*/

class SaveGameOperationCb: Managed
{
	//! Callbacks can be inherited to implement OnResult or constructed by passing a function to handle it with the same parameters.
	proto void SaveGameOperationCb(Managed context = null, SaveGameOperationDelegate handler = null);
	/*!
	Will use the handler function passed to constructor if not implemented
	\param context The context instance passed to constructor.
	\param statusCode The result status. OK as expected result, anything else needs handling / logging / throwing errors.
	*/
	event protected void OnCompleted(Managed context, bool success);

	//! Invoke the handler function assigned to this callback. Returns false if there is none. To be used from OnResult.
	proto external bool InvokeHandler(Managed context, bool success);
}

/*!
\}
*/
