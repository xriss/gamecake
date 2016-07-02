
#vagrant winrm -c "net user vagrant vagrant"
#vagrant winrm -c "echo \"Compression no\" >> '/Program Files/OpenSSH-Win32/sshd_config' "


vagrant winrm -c "iex ((new-object net.webclient).DownloadString('https://chocolatey.org/install.ps1'))"
vagrant winrm -c "chocolatey feature enable -n allowGlobalConfirmation"


#vagrant winrm -c "choco install win32-openssh -params '\"/SSHServerFeature\"' "
#vagrant winrm -c "net start sshd "


vagrant winrm -c "choco install visualstudio2015community"
vagrant winrm -c "choco install nuget.commandline"

