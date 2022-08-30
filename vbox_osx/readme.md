This is partially working again...

Auto key setup seems to fail so, need to enable dss in ,ssh/config

	Host *
		HostkeyAlgorithms +ssh-dss
		PubkeyAcceptedKeyTypes +ssh-dss

and then

	vagrant up

	ssh-copy-id ssh://vagrant@127.0.0.1:2222


then we can use ssh to autologin

	ssh ssh://vagrant@127.0.0.1:2222


