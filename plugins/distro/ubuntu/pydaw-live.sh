if [ -d /home/ubuntu ]; then
	cp /usr/share/applications/pydaw* /home/ubuntu/Desktop
	cp /usr/share/applications/qjackctl* /home/ubuntu/Desktop
	cp /usr/share/applications/audacity* /home/ubuntu/Desktop
else
	echo "No /home/ubuntu folder found, not copying shortcut(s)"
fi
