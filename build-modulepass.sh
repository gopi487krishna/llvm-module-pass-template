# Script directory
SCRIPTDIR=$(dirname "$0")

if [ "$1" == "clean" ]; then
	cmake --build $SCRIPTDIR/build/  --target clean
else

	cmake -B $SCRIPTDIR/build -S $SCRIPTDIR -G Ninja
	cmake --build $SCRIPTDIR/build --config Debug --target all
fi
