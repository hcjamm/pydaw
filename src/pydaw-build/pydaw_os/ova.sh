if [ -f pydaw.ova ]; then
	rm -f pydaw.ova
fi

vboxmanage export pydaw -o pydaw.ova

