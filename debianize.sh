#!/bin/bash

VERSION="$1"

HELP="Exemplo de uso: $(basename $0) 0.1"

[ -z "$VERSION" ] && {
	echo "Erro: Nenhuma versão do pacote foi passada como parâmetro ao script"
	echo "$HELP"
	exit 1
}

[ "$VERSION" == "-h" -o "$VERSION" == "--help" ] && {
	echo "$HELP"
	exit 0
}

mkdir -p seplib_${VERSION}_all/DEBIAN

CONTROL="Package: seplib
Version: ${VERSION}
Architecture: all
Priority: optional
Essential: no
Maintainer: Rodolfo A C Neves (Dirack) <https://dirack.github.io>
Original-Maintainer: GPGEOF <https://github.com/gpgeof>
Bugs: https://github.com/Dirack/Shellinclude/issues/new?assignees=Dirack&labels=bug&template=bug_report.md&title=%5BBUG%5D
Homepage: https://github.com/Dirack/Shellinclude/wiki
Depends: bash
Description: Programas da biblioteca SEPlib
"

echo "$CONTROL" > seplib_${VERSION}_all/DEBIAN/control

mkdir -p seplib_${VERSION}_all/usr/share

mkdir -p seplib_${VERSION}_all/usr/local

cp -r ./bin seplib_${VERSION}_all/usr/local

cp -r ./man seplib_${VERSION}_all/usr/share

mv seplib_${VERSION}_all build

cd build

dpkg-deb -b seplib_${VERSION}_all
