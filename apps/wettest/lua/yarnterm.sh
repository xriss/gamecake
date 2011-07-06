#lua cant set cbreak modes so do it in this script, 

stty cbreak

lua yarnterm.lua || true

stty sane
