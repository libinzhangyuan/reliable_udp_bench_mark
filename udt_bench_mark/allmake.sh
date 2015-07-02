OLD_PWD="$( pwd )"

#OS="LINUX"
OS="OSX"

ARCH="IA32"

echo "" && echo "" && echo "" && echo "" && echo ""
echo "============================================================"
echo "        Compiling                                     =====" 
echo "========================================================="

echo "" && echo "" && echo "[-------------------------------]" && echo "   udt4" && echo "[-------------------------------]" && \
    cd ./udt4/ && make -e os=$OS arch=$ARCH && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   udt_stream_server" && echo "[-------------------------------]" && \
    cd ../udt_stream_server/ && make -e os=$OS arch=$ARCH && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   udt_stream_client" && echo "[-------------------------------]" && \
    cd ../udt_stream_client/ && make -e os=$OS arch=$ARCH && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   udt_dgram_server" && echo "[-------------------------------]" && \
    cd ../udt_dgram_server/ && make -e os=$OS arch=$ARCH && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   udt_dgram_client" && echo "[-------------------------------]" && \
    cd ../udt_dgram_client/ && make -e os=$OS arch=$ARCH && \
echo ""

# restore old path.
cd $OLD_PWD
