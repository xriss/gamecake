
# first install a require script
if ! [[ -x "$(command -v require.sh)" ]] ; then

        echo " we need sudo to install require.sh to /usr/local/bin "
        sudo wget -O /usr/local/bin/require.sh https://raw.githubusercontent.com/xriss/require.sh/main/require.sh
        sudo chmod +x /usr/local/bin/require.sh

fi

# commands

require.sh qemu-system
require.sh /usr/bin/qemu-system-arm
require.sh sshpass
require.sh unzip
