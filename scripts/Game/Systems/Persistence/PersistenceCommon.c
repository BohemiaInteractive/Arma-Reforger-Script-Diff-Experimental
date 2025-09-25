//! Create/Update/Delete callback delegate
void PersistenceStatusDelegate(Managed context, EPersistenceStatusCode statusCode);
typedef func PersistenceStatusDelegate;

//! Read callback delegate
void PersistenceResultDelegate(Managed context, EPersistenceStatusCode statusCode, Managed result, bool isLast);
typedef func PersistenceResultDelegate;

//! WhenAvailable callback delegate
void PersistenceWhenAvailableTaskDelegate(Managed context, Managed instance, PersistenceDeferredDeserializeTask task, bool expired);
typedef func PersistenceWhenAvailableTaskDelegate;
