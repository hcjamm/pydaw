if [ -f ./core ]; then
	rm ./core
fi

ulimit -c unlimited
make clean && make debug && sudo make install && ./pydaw2
