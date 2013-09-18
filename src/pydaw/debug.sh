if [ -f ./core ]; then
	rm ./core
fi

if [ ! -d dir_debug ]; then
	mkdir dir_debug
else
	rm -rf dir_debug/*
fi

ulimit -c unlimited
make clean && make debug && make PREFIX=$(pwd)/dir_debug install && ./dir_debug/bin/pydaw3 --debug

