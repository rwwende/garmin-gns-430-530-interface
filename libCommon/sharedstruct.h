#ifndef __SHARED_STRUCT_H__
#define __SHARED_STRUCT_H__

#include <windows.h>
#include <aclapi.h>
#include "log.h"

template<typename T>
class SharedStruct
{
public:
    explicit SharedStruct(char* mapName, bool create, unsigned long extra_size):
        pMapFile(0),
        handle(0)

    {
        if(create)
        {
            logMessageEx("--- Shared %d+%d %d",  sizeof(T), extra_size , sizeof(T) + extra_size);

            CreateSharedMemory(mapName, sizeof(T) + extra_size, 0);
        }else
        {
            OpenSharedMemory(mapName);
        }
    }


    // ctor to create the initial file mapping
    void CreateSharedMemory(char* mapName,
        DWORD maxSizeLow,
        DWORD maxSizeHigh)
    {
        handle = NULL;


        DWORD dwRes;
        PSID pEveryoneSID = NULL, pAdminSID = NULL;
        PACL pACL = NULL;
        PSECURITY_DESCRIPTOR pSD = NULL;
        EXPLICIT_ACCESS ea[2];
        SID_IDENTIFIER_AUTHORITY SIDAuthWorld =
            SECURITY_WORLD_SID_AUTHORITY;
        SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
        SECURITY_ATTRIBUTES sa;

        // Create a well-known SID for the Everyone group.
        if(!AllocateAndInitializeSid(&SIDAuthWorld, 1,
            SECURITY_WORLD_RID,
            0, 0, 0, 0, 0, 0, 0,
            &pEveryoneSID))
        {
            logMessageEx("??? AllocateAndInitializeSid Error %u", GetLastError());
            return;
        }

        // Initialize an EXPLICIT_ACCESS structure for an ACE.
        // The ACE will allow Everyone read access to the key.
        ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));
        ea[0].grfAccessPermissions = KEY_ALL_ACCESS;//KEY_READ;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance= NO_INHERITANCE;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[0].Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

        // Create a SID for the BUILTIN\Administrators group.
        if(! AllocateAndInitializeSid(&SIDAuthNT, 2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &pAdminSID)) 
        {
            logMessageEx("??? AllocateAndInitializeSid Error %u", GetLastError());
            return; 
        }

        // Initialize an EXPLICIT_ACCESS structure for an ACE.
        // The ACE will allow the Administrators group full access to
        // the key.
        ea[1].grfAccessPermissions = KEY_ALL_ACCESS;
        ea[1].grfAccessMode = SET_ACCESS;
        ea[1].grfInheritance= NO_INHERITANCE;
        ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ea[1].Trustee.ptstrName  = (LPTSTR) pAdminSID;

        // Create a new ACL that contains the new ACEs.
        dwRes = SetEntriesInAcl(2, ea, NULL, &pACL);
        if (ERROR_SUCCESS != dwRes) 
        {
            logMessageEx("??? SetEntriesInAcl Error %u", GetLastError());
            return;
        }

        // Initialize a security descriptor.  
        pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
            SECURITY_DESCRIPTOR_MIN_LENGTH); 
        if (NULL == pSD) 
        { 
            logMessageEx("??? LocalAlloc Error %u", GetLastError());
            return; 
        } 

        if (!InitializeSecurityDescriptor(pSD,
            SECURITY_DESCRIPTOR_REVISION)) 
        {  
            logMessageEx("??? InitializeSecurityDescriptor Error %u", GetLastError());
            return; 
        } 

        // Add the ACL to the security descriptor. 
        if (!SetSecurityDescriptorDacl(pSD, 
            TRUE,     // bDaclPresent flag   
            pACL, 
            FALSE))   // not a default DACL 
        {  
            logMessageEx("??? SetSecurityDescriptorDacl Error %u", GetLastError());
            return; 
        } 

        // Initialize a security attributes structure.
        sa.nLength = sizeof (SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = pSD;
        sa.bInheritHandle = FALSE;



        // create new file mapping
        handle =
            ::CreateFileMapping(reinterpret_cast<HANDLE>(0xffffffff),
            &sa, //NULL,
            PAGE_READWRITE,
            maxSizeHigh,
            maxSizeLow,
            mapName);

        if (handle == NULL)
        {
            //throw int(::GetLastError()); // your error code here
            logMessageEx("??? Shared handle = NULL");

            return;//
        }

        pMapFile = static_cast<T *>(::MapViewOfFile(handle,
            FILE_MAP_ALL_ACCESS,
            0, // high offset
            0, // low offset
            0)); // num bytes to map (all)
        if (pMapFile == NULL)
        {
            //throw int(::GetLastError());
            logMessageEx("??? Shared map = NULL");
            return;//
        }

        memset(pMapFile, 0x00, sizeof(T));

    }

    // ctor to open an existing file mapping
    void OpenSharedMemory(char* mapName)
    {

        bool inherit = true;



        // open existing file mapping
        handle = ::OpenFileMapping(FILE_MAP_ALL_ACCESS,
            inherit,
            mapName);
        if (handle == NULL)
        {
            logMessageEx("??? OpenSharedMemory handle == NULL %s ::GetLastError()=%d", mapName, ::GetLastError());    

            return;//
        }

        pMapFile = static_cast<T *>(::MapViewOfFile(handle,
            FILE_MAP_ALL_ACCESS,
            0, // high offset
            0, // low offset
            0)); // num bytes to
        // map
        if (pMapFile == NULL)
        {
            logMessageEx("??? OpenSharedMemory pMapFile == NULL");    
            //throw int(::GetLastError());
            return;
        }

    }

    ~SharedStruct()
    {
        if(NULL != pMapFile)
        {
            ::UnmapViewOfFile(pMapFile);
        }
        pMapFile = NULL;
        if(NULL != handle)
        {
            ::CloseHandle(handle);
        }
    }

    T *get() { return pMapFile; }

    HANDLE getHandle() { return handle;}

private:
    // do not allow
    SharedStruct();
    SharedStruct(const SharedStruct &rhs);
    SharedStruct &operator=(const SharedStruct &rhs);

    T *pMapFile; // the data, I own
    HANDLE handle;
};

#endif //#ifndef __SHARED_STRUCT_H__