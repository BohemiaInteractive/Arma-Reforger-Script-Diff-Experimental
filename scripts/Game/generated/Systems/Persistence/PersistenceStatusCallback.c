/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence
\{
*/

class PersistenceStatusCallback: PersistenceCallback
{
	//! Callbacks can be inherited to implement OnResult or constructed by passing a function to handle it with the same parameters.
	proto void PersistenceStatusCallback(Managed context = null, PersistenceStatusDelegate delegate = null);
	/*!
	Will use the delegate passed to constructor if not implemented
	\param context The context instance passed to constructor.
	\param statusCode The result status. OK as expected result, anything else needs handling / logging / throwing errors.
	*/
	event protected void OnResult(Managed context, EPersistenceStatusCode statusCode);

	//! Invoke the delegate assigned to this callback. Returns false if there is none. To be used from OnResult.
	proto external bool InvokeDelegate(Managed context, EPersistenceStatusCode statusCode);
}

/*!
\}
*/
