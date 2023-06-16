cd `dirname $0`
flatpak-builder ../build-flatpack com.wetgenes.gamecake.yaml --force-clean $*

