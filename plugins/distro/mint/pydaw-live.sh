#Run the dns-fix utility so that Linux Mint live use will have working internet
dns-fix

if [ -d /home/mint ]; then
	cp /usr/share/applications/pydaw* /home/mint/Desktop
	cp /usr/share/applications/qjackctl* /home/mint/Desktop
	cp /usr/share/applications/audacity* /home/mint/Desktop
else
	echo "No /home/mint folder found, not copying shortcut(s)"
fi
