/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence
\{
*/

class PersistenceWhenAvailableTask: PersistenceCallback
{
	//! Callbacks can be inherited to implement OnResult or constructed by passing a function to handle it with the same parameters.
	proto void PersistenceWhenAvailableTask(Managed context = null, PersistenceWhenAvailableTaskDelegate delegate = null);
	/*!
	Will use the delegate passed to constructor if not implemented
	\param context	The context instance passed to constructor.
	\param instance The instance tracked by the UUID the task was waiting for. ATTENTION: It may be null if it failed to load.
	\param task		The deserialize task optionally passed during creation.
					Can be passed along as strong ref to a later time, or else it will auto complete after this tasks DTOR.
	\param expired	When true the requested UUID did not become available fore the max wait time expired.
	*/
	event protected void OnResult(Managed context, Managed instance, PersistenceDeferredDeserializeTask task = null, bool expired = false);

	//! Invoke the delegate assigned to this callback. Returns false if there is none. To be used from OnResult.
	proto external bool InvokeDelegate(Managed context, Managed instance, PersistenceDeferredDeserializeTask task = null, bool expired = false);
}

/*!
\}
*/
