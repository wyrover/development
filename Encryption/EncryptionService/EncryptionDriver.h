#pragma once
class CEncryptionDriver
{
public:
    CEncryptionDriver(void);
    ~CEncryptionDriver(void);

    DWORD Initialize(
        );

    DWORD Uninitialize(
        );
};

