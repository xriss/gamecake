
net user vagrant vagrant
echo \"Compression no\" >> '/Program Files/OpenSSH-Win32/sshd_config'


iex ((new-object net.webclient).DownloadString('https://chocolatey.org/install.ps1'))
chocolatey feature enable -n allowGlobalConfirmation


choco install win32-openssh -params '\"/SSHServerFeature\"'
net start sshd 


choco install visualstudio2015community
choco install nuget.commandline

