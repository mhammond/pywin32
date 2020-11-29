
// This structure is at the very head of the mapped file, and contains
// admin information that needs to be shared between the DLL and this module.
// The perfmon data follows directly.  This data is _always_:
// A PERF_OBJECT_TYPE
// A number of PERF_COUNTER_DEFINITIONs

// dont manage these size better cos I cant be bothered!
const int MMCD_SERVICE_SIZE = 25;
const int MMCD_EVENTSOURCE_SIZE = 25;

enum SupplierStatus {
    SupplierStatusStopped = 0,
    SupplierStatusRunning,
};

struct MappingManagerControlData {
    DWORD ControlSize;  // Size of this structure.
    DWORD TotalSize;    // Total Size allocated in the mapped file.
    SupplierStatus supplierStatus;
    WCHAR ServiceName[MMCD_SERVICE_SIZE];          // The name of the service or application.
    WCHAR EventSourceName[MMCD_EVENTSOURCE_SIZE];  // Source Name that appears in Event Log for errors.
};
