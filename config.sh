#!/usr/bin/env bash
set -e

ROOT="$(pwd)"

# =========================
# Comprobar certificado CA
# =========================

CONTENT_DIR="$ROOT/content"

CA_URL="https://curl.se/ca/cacert.pem"
CA_FILE="$CONTENT_DIR/certs/ca-bundle.crt"

curl -L --fail "$CA_URL" -o "$CA_FILE"

if [ $? -ne 0 ]; then
    echo
    echo "ERROR: No se ha podido descargar el certificado CA."
    echo
    exit 1
fi

if [ ! -f "$CA_FILE" ]; then
    echo
    echo "ERROR: El certificado CA no existe."
    echo
    exit 1
fi

if [ ! -s "$CA_FILE" ]; then
    echo
    echo "ERROR: El certificado CA está vacío."
    echo
    exit 1
fi

echo
echo "Certificado descargado correctamente:"
echo "$CA_FILE"

chmod +x build.sh
chmod +x build_3ds.sh
chmod +x MakeProjectMaker.sh
chmod +x clear_all.sh

git submodule sync --recursive
git submodule update --init --recursive