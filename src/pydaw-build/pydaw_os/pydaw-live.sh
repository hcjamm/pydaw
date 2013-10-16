PYDAW_VERSION=pydaw4

if [ -d /home/ubuntu ]; then
	cp /usr/share/applications/pydaw* /home/ubuntu/Desktop
	cp /usr/share/applications/audacity* /home/ubuntu/Desktop
	cp /usr/share/applications/mixxx* /home/ubuntu/Desktop
	cp /usr/share/doc/$PYDAW_VERSION/readme.txt /home/ubuntu/Desktop	
	cp /usr/lib/$PYDAW_VERSION/mixxx/mixxx.desktop /home/ubuntu/Desktop
else
	echo "No /home/ubuntu folder found, not copying shortcut(s)"
fi

