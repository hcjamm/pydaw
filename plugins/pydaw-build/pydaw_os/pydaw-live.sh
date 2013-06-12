if [ -d /home/ubuntu ]; then
	cp /usr/share/applications/pydaw* /home/ubuntu/Desktop
	cp /usr/share/applications/qjackctl* /home/ubuntu/Desktop
	cp /usr/share/applications/audacity* /home/ubuntu/Desktop
	cp /usr/share/applications/mixxx* /home/ubuntu/Desktop
	cp /usr/share/doc/pydaw3/readme.txt /home/ubuntu/Desktop
	if [ -l /dev/disk/by-label/pydaw_data ]; then
		if [ ! -d /media/pydaw_data ]; then
			mkdir /media/pydaw_data
			mount /dev/disk/by-label/pydaw_data /media/pydaw_data
		fi
		if [ ! -d /media/pydaw_data/mixxx ]; then
			mkdir /media/pydaw_data/mixxx
		fi
		ln -s /media/pydaw_data/mixxx /home/ubuntu/.mixxx
	fi
else
	echo "No /home/ubuntu folder found, not copying shortcut(s)"
fi


