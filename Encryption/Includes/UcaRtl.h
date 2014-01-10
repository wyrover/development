DWORD
EnablePrivilege(
    _In_ HANDLE hToken,
    _In_z_ LPWSTR lpPrivName,
    _In_ BOOL bEnable
    );

DWORD
EnablePrivilegeInCurrentProcess(
    _In_z_ LPWSTR lpPrivName,
    _In_ BOOL bEnable
    );
